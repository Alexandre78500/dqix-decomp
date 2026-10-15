#!/usr/bin/env python3
"""Aide à la décompilation « matching » d'une fonction, pensée pour des agents IA.

  decomp.py info FONCTION              assembleur cible + contexte (appels, données, appelants, types)
  decomp.py squelette FONCTION [-o f]  fichier .cpp de départ (déclarations extern prêtes)
  decomp.py essai FONCTION f.cpp       compile f.cpp avec les options du projet et compare avec objdiff
  decomp.py verifier f.cpp ADRESSE [g.cpp ADRESSE ...]
                                       compare toutes les fonctions de chaque fichier, dans l'ordre, avec
                                       celles du jeu à partir de ADRESSE (ex. ov014:0x021842a0) ; les
                                       appels entre fonctions renommées de ces fichiers sont acceptés
  decomp.py journal                    résumé du journal des essais

FONCTION est un nom de symbols.txt (func_ov014_021842a0, nom C++ mangled) ou une adresse
(0x021842a0, avec ov014: devant si plusieurs modules partagent l'adresse).

Tout se passe dans build/ia/<fonction>/ : le build du projet n'est jamais touché.
Le build doit avoir été fait une fois (`ninja`) pour que build/usa/delinks existe.
"""

import argparse
import json
import os
import re
import shlex
import subprocess
import sys
import time
from dataclasses import dataclass
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import elf as elfmod  # noqa: E402

RACINE = Path(__file__).resolve().parents[2]
VERSION = os.environ.get("DQIX_VERSION", "usa")
CONFIG = RACINE / "config" / VERSION / "arm9"
BUILD = RACINE / "build" / VERSION
DELINKS = BUILD / "delinks"
IA = RACINE / "build" / "ia"
OBJDIFF = RACINE / "objdiff-cli"


# --------------------------------------------------------------------------- symboles

@dataclass
class Sym:
    nom: str
    module: str      # main, itcm, dtcm, ov014...
    adresse: int
    taille: int | None
    genre: str       # function, data, bss, label
    detail: str      # arm/thumb, data(any)...


def modules_config() -> dict[str, Path]:
    mods = {"main": CONFIG}
    for p in sorted(CONFIG.iterdir()):
        if p.is_dir() and (p / "symbols.txt").exists():
            mods[p.name] = p
    for p in sorted((CONFIG / "overlays").iterdir()):
        if (p / "symbols.txt").exists():
            mods[p.name] = p
    return mods


_SYMS: list[Sym] | None = None


def symboles() -> list[Sym]:
    global _SYMS
    if _SYMS is None:
        _SYMS = []
        motif = re.compile(r"^(\S+) kind:(\w+)(?:\(([^)]*)\))? addr:0x([0-9a-f]+)")
        for mod, dossier in modules_config().items():
            for ligne in (dossier / "symbols.txt").read_text().splitlines():
                m = motif.match(ligne)
                if not m:
                    continue
                nom, genre, det, adr = m.groups()
                taille = None
                if det and (t := re.search(r"size=0x([0-9a-f]+)", det)):
                    taille = int(t.group(1), 16)
                _SYMS.append(Sym(nom, mod, int(adr, 16), taille, genre, det or ""))
    return _SYMS


def trouver(fonction: str) -> Sym:
    mod = None
    if ":" in fonction and not fonction.startswith("_Z"):
        mod, fonction = fonction.split(":", 1)
    if re.fullmatch(r"(0x)?[0-9a-fA-F]{8}", fonction):
        adr = int(fonction, 16)
        cands = [s for s in symboles() if s.adresse == adr and s.genre == "function"]
    else:
        cands = [s for s in symboles() if s.nom == fonction]
    if mod:
        cands = [s for s in cands if s.module == mod]
    if not cands:
        sys.exit(f"Symbole introuvable : {fonction}")
    if len(cands) > 1:
        sys.exit("Plusieurs candidats, préciser le module (ex. ov014:0x021842a0) : "
                 + ", ".join(f"{s.module}:{s.nom}" for s in cands))
    return cands[0]


def demangle(noms: list[str]) -> dict[str, str]:
    mangles = [n for n in noms if n.startswith("_Z")]
    if not mangles:
        return {}
    try:
        sortie = subprocess.run(["c++filt"], input="\n".join(mangles), capture_output=True,
                                text=True, check=True).stdout.splitlines()
    except (OSError, subprocess.CalledProcessError):
        return {}
    return dict(zip(mangles, sortie))


# --------------------------------------------------------------------------- objets délinkés

def index_delinks() -> dict[str, str]:
    """nom de symbole défini -> objet délinké (cible) qui le contient."""
    objets = sorted(DELINKS.rglob("*.o"))
    if not objets:
        sys.exit("build/usa/delinks est vide : lancer `ninja` d'abord.")
    cache = IA / f"index_{VERSION}.json"
    signature = max(o.stat().st_mtime for o in objets)
    if cache.exists():
        d = json.loads(cache.read_text())
        if d.get("signature") == signature:
            return d["index"]
    index = {}
    for o in objets:
        e = elfmod.lire(str(o), avec_donnees=False)
        for s in e.symboles_definis():
            if s.lien != 0 or s.nom.startswith(("func_", "data_", "_Z")):
                index.setdefault(s.nom, str(o.relative_to(RACINE)))
    IA.mkdir(parents=True, exist_ok=True)
    cache.write_text(json.dumps({"signature": signature, "index": index}))
    return index


def est_decompile(objet: str | None) -> bool:
    """Un symbole est décompilé si son objet cible vient d'un fichier source (src/ ou libs/)."""
    if objet is None:
        return False
    rel = Path(objet).relative_to(DELINKS.relative_to(RACINE))
    return rel.parts[0] in ("src", "libs")


# --------------------------------------------------------------------------- objdiff

def objdiff_json(cible: Path, base: Path | None, symbole: str, correspondance: str | None = None) -> dict:
    IA.mkdir(parents=True, exist_ok=True)
    sortie = IA / f"objdiff_{os.getpid()}.json"
    cmd = [str(OBJDIFF), "diff", "-1", str(cible), "--format", "json", "-o", str(sortie)]
    if base is not None:
        cmd += ["-2", str(base)]
    if correspondance:
        cmd += ["-m", f"{symbole}={correspondance}"]
    cmd.append(symbole)
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode != 0:
        sys.exit("objdiff a échoué :\n" + r.stderr + r.stdout)
    try:
        return json.loads(sortie.read_text())
    finally:
        sortie.unlink(missing_ok=True)


def chercher_symbole(cote: dict, nom: str) -> tuple[dict, dict] | None:
    for sec in cote["sections"]:
        for s in sec.get("symbols", []):
            if s["symbol"]["name"] == nom:
                return sec, s
    return None


def format_instr(entree: dict, demangles: dict[str, str]) -> str:
    ins = entree.get("instruction")
    if ins is None:
        return ""
    morceaux = []
    for arg in ins.get("arguments", []):
        if "plain_text" in arg:
            morceaux.append(arg["plain_text"])
        elif "argument" in arg:
            a = arg["argument"]
            if "opaque" in a:
                morceaux.append(a["opaque"])
            elif "signed" in a:
                v = int(a["signed"])
                morceaux.append(hex(v) if v >= 0 else "-" + hex(-v))
            elif "unsigned" in a:
                morceaux.append(hex(int(a["unsigned"])))
            else:
                morceaux.append(str(next(iter(a.values()))))
        elif "relocation" in arg:
            rel = ins.get("relocation", {}).get("target", {})
            nom = rel.get("symbol", {}).get("name", "?")
            add = int(rel.get("addend", "0"))
            if ins["mnemonic"].startswith(("bl", "b", "blx")) and add in (-8, -4):
                add = 0
            morceaux.append(nom + (f"{add:+#x}" if add else ""))
        elif "branch_dest" in arg:
            morceaux.append(f"->{int(arg['branch_dest']):#x}")
    texte = ins["mnemonic"]
    if morceaux:
        texte += " " + "".join(morceaux).replace(",", ", ").replace(",  ", ", ")
    return texte


def lister_asm(symb: dict, demangles: dict[str, str]) -> list[str]:
    lignes = []
    cibles = set()
    for e in symb.get("instructions", []):
        if "branch_from" in e:
            cibles.add(int(e["instruction"].get("address", "0")))
    for e in symb.get("instructions", []):
        ins = e.get("instruction")
        if ins is None:
            continue
        adr = int(ins.get("address", "0"))
        etiquette = f"{adr:#06x}:" + ("*" if adr in cibles else " ")
        lignes.append(f"{etiquette} {format_instr(e, demangles)}")
    return lignes


def relocs_de(symb: dict) -> list[str]:
    noms = []
    for e in symb.get("instructions", []):
        ins = e.get("instruction") or {}
        nom = ins.get("relocation", {}).get("target", {}).get("symbol", {}).get("name")
        if nom and nom not in noms:
            noms.append(nom)
    return noms


# --------------------------------------------------------------------------- contexte

def donnees_symbole(objet: Path, nom: str, limite: int = 96) -> str:
    """Contenu d'une donnée dans l'objet délinké (octets, chaîne, pointeurs relogés)."""
    e = elfmod.lire(str(objet))
    s = e.symbole(nom)
    if s is None or s.section >= len(e.sections):
        return "  (introuvable dans l'objet cible)"
    sec = e.sections[s.section]
    # taille : jusqu'au symbole suivant de la même section
    suivants = sorted(x.valeur for x in e.symboles_definis() if x.section == s.section and x.valeur > s.valeur)
    fin = suivants[0] if suivants else sec.taille
    taille = s.taille or (fin - s.valeur)
    lignes = [f"  section {sec.nom}, taille ~{taille:#x}"]
    if sec.type == 8:
        return lignes[0] + " (bss, non initialisée)"
    octets = sec.donnees[s.valeur:s.valeur + min(taille, limite)]
    relocs = {r.offset - s.valeur: r.symbole.nom for r in e.relocs.get(s.section, [])
              if s.valeur <= r.offset < s.valeur + taille}
    texte = octets.split(b"\0")[0]
    if len(texte) >= 3 and all(32 <= c < 127 for c in texte):
        lignes.append(f'  chaîne : "{texte.decode()}"')
    mots = []
    for i in range(0, len(octets) - len(octets) % 4, 4):
        if i in relocs:
            mots.append(f"&{relocs[i]}")
        else:
            mots.append(f"{int.from_bytes(octets[i:i + 4], 'little'):#010x}")
    if mots:
        lignes.append("  mots : " + " ".join(mots[:24]) + (" ..." if taille > limite else ""))
    elif octets:
        lignes.append("  octets : " + octets.hex(" "))
    return "\n".join(lignes)


def declarations(nom: str, lisible: str | None) -> list[str]:
    """Où trouver la déclaration d'un symbole déjà nommé : classe C++ dans include/, sinon le nom lui-même."""
    motifs = []
    if lisible and "::" in lisible.split("(")[0]:
        classe = lisible.split("(")[0].split("::")[-2]
        motifs.append(["-E", rf"^\s*(class|struct)\s+{classe}\b"])
    motifs.append(["-w", nom])
    for motif in motifs:
        r = subprocess.run(["grep", "-rn", "--include=*.h", "--include=*.hpp", *motif, "include"],
                           cwd=RACINE, capture_output=True, text=True)
        lignes = ["    " + l.strip()[:160] for l in r.stdout.splitlines()[:3]]
        if lignes:
            return lignes
    return []


def vise_module(dest: str, module: str) -> bool:
    """Le champ module: d'une relocation (main, itcm, overlay(14), overlays(1,2)) désigne-t-il ce module ?"""
    if module.startswith("ov"):
        m = re.fullmatch(r"overlays?\(([\d,]+)\)", dest)
        return bool(m) and int(module[2:]) in [int(x) for x in m.group(1).split(",")]
    return dest == module


def appelants(cible: Sym) -> list[str]:
    res = []
    motif = re.compile(r"from:0x([0-9a-f]+) kind:(\w+) to:0x([0-9a-f]+) module:(\S+)")
    for mod, dossier in modules_config().items():
        f = dossier / "relocs.txt"
        if not f.exists():
            continue
        for l in f.read_text().splitlines():
            m = motif.match(l)
            if not m or int(m.group(3), 16) != cible.adresse:
                continue
            if not vise_module(m.group(4), cible.module):
                continue
            depuis = int(m.group(1), 16)
            fn = fonction_contenant(mod, depuis)
            res.append(f"{mod}:{fn.nom if fn else hex(depuis)} ({m.group(2)})")
    return sorted(set(res))


def fonction_contenant(module: str, adresse: int) -> Sym | None:
    for s in symboles():
        if s.module == module and s.genre == "function" and s.taille and s.adresse <= adresse < s.adresse + s.taille:
            return s
    return None


def voisins(cible: Sym, n: int = 3) -> list[Sym]:
    fns = sorted((s for s in symboles() if s.module == cible.module and s.genre == "function"),
                 key=lambda s: s.adresse)
    i = next(k for k, s in enumerate(fns) if s.nom == cible.nom)
    return fns[max(0, i - n):i] + fns[i + 1:i + 1 + n]


def cmd_info(args):
    cible = trouver(args.fonction)
    index = index_delinks()
    objet = index.get(cible.nom)
    if objet is None:
        sys.exit(f"{cible.nom} n'apparaît dans aucun objet délinké (fonction déjà décompilée ?)")
    print(f"# {cible.nom}  module {cible.module}  adresse {cible.adresse:#010x}  taille {cible.taille:#x}"
          f" ({cible.taille // 4} mots)  {cible.detail}")
    print(f"# objet cible : {objet}")
    d = objdiff_json(RACINE / objet, None, cible.nom)
    _, symb = chercher_symbole(d["left"], cible.nom)
    noms = relocs_de(symb)
    dem = demangle(noms + [cible.nom])
    if cible.nom in dem:
        print(f"# nom C++ : {dem[cible.nom]}")
    print("\n## Assembleur cible (* = cible de branchement)")
    print("\n".join(lister_asm(symb, dem)))

    fonctions = [n for n in noms if any(s.nom == n and s.genre == "function" for s in symboles())]
    donnees = [n for n in noms if n not in fonctions]
    if fonctions:
        print("\n## Fonctions appelées")
        for n in fonctions:
            s = next(s for s in symboles() if s.nom == n)
            etat = "décompilée" if est_decompile(index.get(n)) else "non décompilée"
            print(f"- {n}" + (f"  = {dem[n]}" if n in dem else "")
                  + f"  [{s.module} {s.adresse:#010x}, taille {s.taille or 0:#x}, {etat}]")
            for l in declarations(n, dem.get(n)):
                print(l)
    if donnees:
        print("\n## Données référencées")
        for n in donnees:
            s = next((s for s in symboles() if s.nom == n), None)
            o = index.get(n)
            entete = f"- {n}" + (f"  = {dem[n]}" if n in dem else "")
            if s:
                entete += f"  [{s.module} {s.adresse:#010x} {s.genre}({s.detail})]"
            print(entete)
            if o:
                print(donnees_symbole(RACINE / o, n))
            for l in declarations(n, dem.get(n)):
                print(l)
    print("\n## Appelants")
    print("\n".join("- " + a for a in appelants(cible)) or "- aucun appel direct (pointeur ou table virtuelle ?)")
    print("\n## Voisines dans le module")
    for v in voisins(cible):
        print(f"- {v.nom} {v.adresse:#010x} taille {v.taille or 0:#x}"
              f" {'décompilée' if est_decompile(index.get(v.nom)) else ''}")


def cmd_squelette(args):
    cible = trouver(args.fonction)
    index = index_delinks()
    objet = index[cible.nom]
    d = objdiff_json(RACINE / objet, None, cible.nom)
    _, symb = chercher_symbole(d["left"], cible.nom)
    noms = relocs_de(symb)
    lignes = ['#include "globaldefs.h"', ""]
    if VERSION == "usa":
        pass
    externs_c, autres = [], []
    for n in noms:
        s = next((s for s in symboles() if s.nom == n), None)
        if n.startswith("_Z"):
            autres.append(f"// {n} : déclarer la classe ou la fonction C++ correspondante (voir include/)")
        elif s and s.genre == "function":
            externs_c.append(f"    void {n}(); // TODO signature")
        else:
            externs_c.append(f"    extern unsigned char {n}[]; // TODO type")
    if externs_c:
        lignes += ['extern "C"', "{"] + externs_c + ["}", ""]
    lignes += autres + ([""] if autres else [])
    lignes += [f'extern "C" void {cible.nom}()', "{", "}", ""]
    texte = "\n".join(lignes)
    if args.o:
        Path(args.o).write_text(texte)
        print(f"Écrit {args.o}")
    else:
        print(texte)


# --------------------------------------------------------------------------- compilation

def commande_mwcc() -> str:
    ninja = (RACINE / "build.ninja").read_text()
    m = re.search(r"^rule mwcc\n  command = (.*?)\n  depfile", ninja, re.S | re.M)
    if not m:
        sys.exit("Règle mwcc introuvable dans build.ninja (lancer tools/configure.py).")
    cmd = re.sub(r"\$\n\s*", "", m.group(1))
    cmd = cmd.split(" && ")[0].replace(" -MD", "")
    return cmd


def compiler(source: Path, dossier: Path) -> tuple[bool, str, Path]:
    dossier.mkdir(parents=True, exist_ok=True)
    cmd = (commande_mwcc().replace("$cc_flags", "-lang=c++").replace("$game_version", VERSION)
           .replace("$in", shlex.quote(str(source))).replace("$basedir", shlex.quote(str(dossier))))
    r = subprocess.run(cmd, shell=True, cwd=RACINE, capture_output=True, text=True)
    objet = dossier / (source.stem + ".o")
    return r.returncode == 0 and objet.exists(), (r.stdout + r.stderr).strip(), objet


def fonctions_objet(objet: Path) -> list[str]:
    e = elfmod.lire(str(objet), avec_donnees=False)
    return [s.nom for s in e.symboles if s.type == 2 and s.section not in (0, 0xFFF1) and not s.nom.startswith("$")]


def journaliser(entree: dict):
    IA.mkdir(parents=True, exist_ok=True)
    with open(IA / "journal.jsonl", "a") as f:
        f.write(json.dumps(entree, ensure_ascii=False) + "\n")


# nouveau nom (dans nos sources) -> nom actuel dans symbols.txt, pour les fonctions renommées
RENOMMAGES: dict[str, str] = {}


def faux_positif(a: dict, b: dict) -> bool:
    """objdiff marque différente une relocation vers un symbole défini dans l'objet cible (donnée de
    l'overlay, fonction voisine) quand notre objet le déclare extern. Même instruction et même cible
    de relocation : ce n'est pas une vraie différence (l'éditeur de liens donnera les mêmes octets)."""
    ia, ib = a.get("instruction"), b.get("instruction")
    if not ia or not ib or (a.get("diff_kind") or b.get("diff_kind")) != "DIFF_ARG_MISMATCH":
        return False
    ra, rb = ia.get("relocation", {}).get("target", {}), ib.get("relocation", {}).get("target", {})
    if not ra or not rb:
        return False
    # l'immédiat affiché d'un bl relogé dépend de l'addende implicite : on compare la relocation
    return (ia["mnemonic"] == ib["mnemonic"]
            and (ia.get("formatted") == ib.get("formatted") or ia["mnemonic"].startswith("bl"))
            and ra.get("symbol", {}).get("name") == RENOMMAGES.get(rb.get("symbol", {}).get("name"),
                                                                   rb.get("symbol", {}).get("name"))
            and ra.get("addend", "0") == rb.get("addend", "0"))


def comparer(objet_cible: Path, objet: Path, nom_cible: str, nom_base: str):
    """Renvoie (score corrigé, lignes cible, lignes proposition, marques) ; droite None si non apparié."""
    d = objdiff_json(objet_cible, objet, nom_cible, None if nom_base == nom_cible else nom_base)
    _, gauche = chercher_symbole(d["left"], nom_cible)
    if "target" not in gauche:
        return 0.0, gauche, None, []
    t = gauche["target"]
    droite = d["right"]["sections"][t.get("section_index", 0)]["symbols"][t.get("symbol_index", 0)]
    lg, ld = gauche.get("instructions", []), droite.get("instructions", [])
    marques = []
    for a, b in zip(lg, ld):
        genre = a.get("diff_kind", "") or b.get("diff_kind", "")
        if not genre or faux_positif(a, b):
            marques.append(" ")
        else:
            marques.append({"DIFF_REPLACE": "R", "DIFF_DELETE": "-", "DIFF_INSERT": "+", "DIFF_OP_MISMATCH": "O",
                            "DIFF_ARG_MISMATCH": "A"}.get(genre, "?"))
    total = max(len(lg), len(ld)) or 1
    score = 100.0 * sum(m == " " for m in marques) / total
    if score < 100.0:
        score = min(score, float(gauche.get("match_percent", 0.0)) + 100.0 * (total - len(marques)) / total
                    + 100.0 * sum(1 for a, b, m in zip(lg, ld, marques) if m == " " and a.get("diff_kind")) / total)
    return score, gauche, droite, marques


def cmd_essai(args):
    cible = trouver(args.fonction)
    index = index_delinks()
    objet_cible = RACINE / index[cible.nom]
    source = Path(args.source).resolve()
    dossier = IA / cible.nom
    t0 = time.time()
    ok, messages, objet = compiler(source, dossier)
    entree = {"date": time.strftime("%Y-%m-%dT%H:%M:%S"), "fonction": cible.nom, "module": cible.module,
              "taille": cible.taille, "source": str(source), "agent": os.environ.get("DECOMP_AGENT", "")}
    if not ok:
        print("ÉCHEC DE COMPILATION\n" + messages)
        journaliser(entree | {"compile": False, "score": 0.0})
        sys.exit(1)
    fns = fonctions_objet(objet)
    nom_base = args.symbole
    if nom_base is None:
        if cible.nom in fns:
            nom_base = cible.nom
        elif len(fns) == 1:
            nom_base = fns[0]
        else:
            sys.exit(f"Plusieurs fonctions dans {source.name} ({', '.join(fns)}) : préciser --symbole")
    if nom_base not in fns:
        sys.exit(f"{nom_base} absent de l'objet compilé ; fonctions présentes : {', '.join(fns)}")
    score, gauche, droite, marques = comparer(objet_cible, objet, cible.nom, nom_base)
    taille_base = int(droite["symbol"].get("size", "0")) if droite else 0
    print(f"SCORE {score:.2f} %  ({cible.nom} {cible.taille:#x} octets, proposition {nom_base} {taille_base:#x} octets)"
          f"  compilé en {time.time() - t0:.1f} s")
    if messages and args.verbeux:
        print(messages)
    journaliser(entree | {"compile": True, "score": score, "symbole": nom_base})
    if droite is None:
        print("(objdiff n'a pas apparié les symboles)")
        return
    if score >= 100.0 and not args.tout:
        print("CORRESPONDANCE PARFAITE")
        return
    lg = gauche.get("instructions", [])
    ld = droite.get("instructions", [])
    print(f"\n{'cible (jeu original)':<52} | proposition")
    for a, b, marque in zip(lg, ld, marques):
        ga = format_instr(a, {})
        gb = format_instr(b, {})
        adr = a.get("instruction", {}).get("address") if a.get("instruction") else None
        pre = f"{int(adr):#06x} " if adr is not None else "       "
        if marque != " " or args.tout:
            print(f"{marque} {pre}{ga:<44} | {gb}")
    print(f"\n{sum(m != ' ' for m in marques)} ligne(s) différente(s) sur {max(len(lg), len(ld))} "
          "(R remplacée, O opcode, A argument, - absente de la proposition, + en trop)")


def fonctions_texte(objet: Path) -> list[elfmod.Symbole]:
    """Fonctions de l'objet dans l'ordre où l'éditeur de liens les placera (mwccarm fait une section par fonction)."""
    e = elfmod.lire(str(objet), avec_donnees=False)
    textes = [i for i, sec in enumerate(e.sections) if sec.nom == ".text"]
    return sorted((x for x in e.symboles if x.type == 2 and x.section in textes and not x.nom.startswith("$")),
                  key=lambda x: (x.section, x.valeur))


def cmd_verifier(args):
    """Compile chaque fichier, apparie ses fonctions dans l'ordre avec celles du jeu à partir de son
    adresse, puis compare. Les appels entre fonctions renommées dans ces fichiers sont acceptés."""
    if len(args.paires) % 2:
        sys.exit("Arguments attendus : fichier.cpp ADRESSE [fichier.cpp ADRESSE ...]")
    lots = []
    for source, adresse in zip(args.paires[::2], args.paires[1::2]):
        depart = trouver(adresse)
        source = Path(source).resolve()
        ok, messages, objet = compiler(source, IA / "_verifier" / source.stem)
        if not ok:
            sys.exit(f"ÉCHEC DE COMPILATION de {source.name}\n" + messages)
        nos_fns = fonctions_texte(objet)
        jeu = sorted((x for x in symboles() if x.module == depart.module and x.genre == "function"
                      and x.adresse >= depart.adresse), key=lambda x: x.adresse)[:len(nos_fns)]
        for a, b in zip(jeu, nos_fns):
            if a.nom != b.nom:
                RENOMMAGES[b.nom] = a.nom
        lots.append((source, depart, objet, nos_fns, jeu))
    index = index_delinks()
    echec = False
    for source, depart, objet, nos_fns, jeu in lots:
        total, bons, octets = 0, 0, 0
        fin = depart.adresse
        print(f"== {source.relative_to(RACINE) if source.is_relative_to(RACINE) else source}")
        for a, b in zip(jeu, nos_fns):
            total += 1
            objet_cible = index.get(a.nom)
            score = None
            if objet_cible is not None:
                score, _, _, _ = comparer(RACINE / objet_cible, objet, a.nom, b.nom)
            ok = score is not None and score >= 100.0 and b.taille == a.taille
            if ok:
                bons += 1
                octets += a.taille
            fin = a.adresse + (a.taille or 0)
            print(f"{'OK ' if ok else '-- '}{a.adresse:#010x} {a.taille or 0:#6x} {a.nom:<28} <- {b.nom}  "
                  + (f"{score:.2f} %" if score is not None else "absente des objets délinkés")
                  + ("" if b.taille == a.taille else f" (taille {b.taille:#x})"))
        print(f"{bons}/{total} fonctions identiques, {octets:#x} octets ; plage .text "
              f"{depart.adresse:#010x}-{fin:#010x}")
        if len(nos_fns) > len(jeu):
            print(f"Attention : {len(nos_fns) - len(jeu)} fonction(s) de trop dans {source.name}")
        echec |= bons != total
    if echec:
        sys.exit(1)


def cmd_journal(args):
    f = IA / "journal.jsonl"
    if not f.exists():
        sys.exit("Aucun essai journalisé.")
    par_fn = {}
    for l in f.read_text().splitlines():
        e = json.loads(l)
        par_fn.setdefault(e["fonction"], []).append(e)
    for fn, es in par_fn.items():
        meilleur = max(e["score"] for e in es)
        print(f"{fn:<40} {es[0]['taille'] or 0:#7x} essais {len(es):3d}  meilleur {meilleur:6.2f} %")


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sp = p.add_subparsers(dest="cmd", required=True)
    a = sp.add_parser("info")
    a.add_argument("fonction")
    a.set_defaults(f=cmd_info)
    a = sp.add_parser("squelette")
    a.add_argument("fonction")
    a.add_argument("-o")
    a.set_defaults(f=cmd_squelette)
    a = sp.add_parser("essai")
    a.add_argument("fonction")
    a.add_argument("source")
    a.add_argument("--symbole", help="nom de la fonction dans la proposition, s'il diffère")
    a.add_argument("--tout", action="store_true", help="afficher toutes les lignes, pas seulement les différences")
    a.add_argument("-v", "--verbeux", action="store_true", help="afficher les messages du compilateur")
    a.set_defaults(f=cmd_essai)
    a = sp.add_parser("verifier")
    a.add_argument("paires", nargs="+", metavar="FICHIER ADRESSE")
    a.set_defaults(f=cmd_verifier)
    a = sp.add_parser("journal")
    a.set_defaults(f=cmd_journal)
    args = p.parse_args()
    args.f(args)


if __name__ == "__main__":
    main()
