#!/usr/bin/env python3
"""Intègre dans DQ9 les fichiers de bibliothèque HeartGold validés par planifier.py.

Usage : importer.py <racine pokeheartgold> <plan.json> <worktree DQ9> [--essai] [fichier_a_exclure ...]
Écrit libs/ (sources, en-têtes, réglages de compilation), complète delinks.txt et
renomme les symboles DQ9 avec les noms Nintendo. Rien n'est écrit si le plan est incohérent.
"""
import fnmatch, json, os, re, shutil, subprocess, sys
from collections import defaultdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from decoupe import couper

hg, planf, wt = Path(sys.argv[1]).resolve(), Path(sys.argv[2]), Path(sys.argv[3]).resolve()
essai = "--essai" in sys.argv  # analyse seulement, rien n'est écrit
exclus = set(a for a in sys.argv[4:] if a != "--essai")
cfgdir = wt / "config/usa/arm9"
plan = [p for p in json.load(open(planf)) if p["fichier"] not in exclus]
AUTO = re.compile(r"^(func|data|\.L)_")
RESERVES_DSD = {"AutoloadCallback", "BuildInfo"}
DECORE = re.compile(r"^_Z|^__(ct|dt)__|__(\d+\w+?)?F")

# ---------- symboles DQ9 ----------
cfg = (cfgdir / "config.yaml").read_text()
mods = {}
for m in re.finditer(r"name: (\S+)\n\s+object: '([^']+)'\n(?:.*\n)*?\s+delinks: (\S+)\n\s+symbols: (\S+)\n\s+relocations: (\S+)", cfg):
    name, _, delinks, syms, rels = m.groups()
    lines = (cfgdir / syms).read_text().splitlines()
    head = (cfgdir / delinks).read_text().split("\n\n", 1)[0]
    secs = {mm.group(1): (int(mm.group(2), 16), int(mm.group(3), 16)) for mm in re.finditer(r"^\s+(\.\w+)\s+start:0x([0-9a-f]+) end:0x([0-9a-f]+)", head, re.M)}
    relocs = []
    for ln in (cfgdir / rels).read_text().splitlines():
        mm = re.match(r"from:0x([0-9a-f]+) kind:\S+ to:0x([0-9a-f]+)(?: add:-?0x[0-9a-f]+)? module:(\S+)", ln)
        if mm:
            relocs.append((int(mm.group(1), 16), int(mm.group(2), 16), mm.group(3)))
    corps = (cfgdir / delinks).read_text().split("\n\n", 1)
    pris = [(int(a, 16), int(b, 16)) for a, b in re.findall(r"^\s+\.\w+ start:0x([0-9a-f]+) end:0x([0-9a-f]+)\s*$", corps[1] if len(corps) > 1 else "", re.M)]
    mods[name] = dict(lines=lines, symfile=cfgdir / syms, delinks=cfgdir / delinks, secs=secs, relocs=relocs, pris=pris)

def reloc_module(s):
    m = re.match(r"overlay\((\d+)\)$", s)
    return f"ov{int(m.group(1)):03d}" if m else s

def parse(ln):
    m = re.match(r"^(\S+) (.*?addr:0x([0-9a-f]+))(.*)$", ln)
    return (m.group(1), int(m.group(3), 16), m.group(2), m.group(4)) if m else None

def symbols_at(mod):
    d = defaultdict(list)
    for i, ln in enumerate(mods[mod]["lines"]):
        p = parse(ln)
        if p: d[p[1]].append(i)
    return d

# ---------- cohérence globale, en écartant les fichiers fautifs jusqu'à stabilité ----------
a_renommer = {}  # noms du jeu à passer au nom Nintendo (voir renommer_cpp.py)
while True:
    ranges = {p["fichier"]: [(p["module"], s["debut"], s["fin"]) for s in p["sections"]] for p in plan}
    def owner(mod, a):
        for f, rs in ranges.items():
            if any(m == mod and lo <= a < hi for m, lo, hi in rs): return f
    nom_vers_adr, adr_vers_nom, mauvais = {}, {}, defaultdict(set)
    for p in plan:
        for d in p["definis"]:
            k = (p["module"], d["adresse"])
            if adr_vers_nom.setdefault(k, d["nom"]) != d["nom"]: mauvais[p["fichier"]].add(f"deux noms à 0x{k[1]:x} : {d['nom']}/{adr_vers_nom[k]}")
            if not d["local"] and nom_vers_adr.setdefault(d["nom"], k) != k: mauvais[p["fichier"]].add(f"{d['nom']} défini à deux adresses")
    for p in plan:
        for nom, (mod, adr) in p["externes"].items():
            k = (mod, adr)
            if mod not in mods:
                mauvais[p["fichier"]].add(f"{nom} : module {mod} inconnu")
            elif (adr_vers_nom.setdefault(k, nom), nom_vers_adr.setdefault(nom, k)) != (nom, k):
                mauvais[p["fichier"]].add(f"externe {nom} -> {mod}:0x{adr:x} en conflit ({adr_vers_nom[k]} / {nom_vers_adr[nom][0]}:0x{nom_vers_adr[nom][1]:x})")
    # références venant de l'extérieur d'un fichier : elles doivent viser un symbole global du fichier
    globaux = {(p["module"], d["adresse"]) for p in plan for d in p["definis"] if not d["local"]}
    for mod, M in mods.items():
        for frm, to, tmod in M["relocs"]:
            tmod = reloc_module(tmod)
            f = owner(tmod, to)
            if (tmod, to) not in globaux and to & 1: to &= ~1  # pointeur vers une fonction Thumb
            if f and owner(mod, frm) != f and (tmod, to) not in globaux:
                mauvais[f].add(f"0x{to:x} visé depuis {mod}:0x{frm:x} sans symbole global")
    # adresses visées à l'intérieur d'un fichier sans symbole HeartGold à cet endroit :
    # dsd exige un symbole, que l'objet compilé ne fournira pas (vérification des symboles en échec)
    definis = {(p["module"], d["adresse"]) for p in plan for d in p["definis"]}
    for mod, M in mods.items():
        for frm, to, tmod in M["relocs"]:
            tmod = reloc_module(tmod)
            f = owner(tmod, to)
            if (tmod, to) not in definis and to & 1: to &= ~1
            if f and (tmod, to) not in definis:
                mauvais[f].add(f"adresse interne 0x{to:x} sans symbole (depuis {mod}:0x{frm:x})")
    # anciens noms C++ décorés (CodeWarrior : nom__F..., nom__5ClasseF...) : le code du jeu les appelle
    # sous une déclaration C++, un simple renommage ne suffit pas
    for p in plan:
        cibles_p = [(p["module"], d["adresse"], d["nom"]) for d in p["definis"]]
        cibles_p += [(mod, adr, nom) for nom, (mod, adr) in p["externes"].items() if mod in mods]
        for mod, adr, nom in cibles_p:
            for i in symbols_at(mod).get(adr, []):
                old = parse(mods[mod]["lines"][i])[0]
                if old != nom and DECORE.search(old):
                    mauvais[p["fichier"]].add(f"nom décoré {old} -> {nom}")
                    if re.match(r"_Z\d", old): a_renommer[old] = nom  # fonction libre : renommer_cpp.py
    # noms dont dsd se sert pour construire la ROM (rom config cherche BuildInfo via AutoloadCallback)
    for p in plan:
        cibles_p = [(p["module"], d["adresse"], d["nom"]) for d in p["definis"]]
        cibles_p += [(mod, adr, nom) for nom, (mod, adr) in p["externes"].items() if mod in mods]
        for mod, adr, nom in cibles_p:
            for i in symbols_at(mod).get(adr, []):
                old = parse(mods[mod]["lines"][i])[0]
                if old != nom and old in RESERVES_DSD: mauvais[p["fichier"]].add(f"nom réservé par dsd {old} -> {nom}")
    # appels vers du code déjà décompilé par DQ9 sous un autre nom : on ne peut pas le renommer
    for p in plan:
        for nom, (mod, adr) in p["externes"].items():
            if mod in mods and any(lo <= adr < hi for lo, hi in mods[mod]["pris"]):
                olds = [parse(mods[mod]["lines"][i])[0] for i in symbols_at(mod).get(adr, [])]
                if olds != [nom]:
                    mauvais[p["fichier"]].add(f"appel vers code DQ9 déjà décompilé : {nom} (DQ9 : {olds})")
                    if len(olds) == 1 and (re.match(r"_Z\d", olds[0]) or not DECORE.search(olds[0]) and not AUTO.match(olds[0])):
                        a_renommer[olds[0]] = nom
    # fichier retrouvé en partie : les fonctions absentes de DQ9 sont compilées puis jetées par -dead,
    # sauf celles qui visent un symbole introuvable, dont le nom est déjà pris ou que la source garde
    # (#pragma force_active) : elles sont coupées de la source, avec celles qui les appellent
    connus = {parse(ln)[0] for M in mods.values() for ln in M["lines"] if parse(ln)} | set(nom_vers_adr)
    coupes = {}
    for p in plan:
        coupe = set()
        while True:
            n0 = len(coupe)
            for r in p.get("retirees", []):
                if r["noms"] and set(r["noms"]) <= coupe:
                    continue
                if (r.get("force") or any(e not in connus for e in r["externes"]) or any(g in connus for g in r["globaux"])
                        or set(r["internes"]) & coupe):
                    if not r["noms"]:
                        mauvais[p["fichier"]].add(f"donnée retirée ({r['section']}) à couper")
                    coupe |= set(r["noms"])
            if len(coupe) == n0:
                break
        if coupe:
            texte, manquants = couper((hg / p["fichier"]).read_text(errors="surrogateescape"), coupe)
            if manquants:
                mauvais[p["fichier"]].add(f"découpe impossible : {', '.join(manquants)}")
            coupes[p["fichier"]] = texte
    if not mauvais: break
    print("écartés pour conflit de noms ou de références :", len(mauvais))
    for f in sorted(mauvais):
        print("  ", f, "|", "; ".join(sorted(mauvais[f])))
    plan = [p for p in plan if p["fichier"] not in mauvais]
print(f"fichiers retenus : {len(plan)}, dont {len(coupes)} découpés")
if essai:
    if a_renommer:
        sortie = wt.parent / f"renommages_{wt.name}.txt"
        sortie.write_text("".join(f"{o} -> {n}\n" for o, n in sorted(a_renommer.items())))
        print(f"{len(a_renommer)} noms du jeu à renommer pour débloquer des fichiers : {sortie}")
    sys.exit(0)

# ---------- réglages de compilation repris du Makefile HeartGold ----------
rules = []
for mk in ("common.mk", "Makefile"):
    for line in (hg / mk).read_text().splitlines():
        m = re.match(r"^\$\(BUILD_DIR\)/(\S+?)\.o:\s*private\s+(\w+)\s*:=\s*(.*)$", line.strip())
        if m: rules.append((m.group(1).replace("%", "*"), m.group(2), m.group(3).strip()))
def settings(src):
    s = {"MWCCVER": "2.0/sp2p2", "OPTFLAGS": "-O4,p", "EXCCFLAGS": "-Cpp_exceptions off", "IPA": "-ipa file"}
    stem = str(Path(src).with_suffix(""))
    for exact in (False, True):
        for pat, var, val in rules:
            if ("*" not in pat) == exact and fnmatch.fnmatch(stem, pat):
                if var == "MWCFLAGS" and "filter-out -ipa file" in val: s["IPA"] = ""
                elif var in s: s[var] = val
    return s

ROOTS = {"src": "game_src", "include": "game", "include/library": "game_library", "files": "files", "lib/include": "lib",
         "lib/NitroSDK/include": "sdk", "lib/NitroSDK/include-mw": "sdk_mw", "lib/NitroSDK/include-pokediamond": "sdk_pokediamond"}
HDR = "libs/_headers"
def dest(src):  # lib/NitroSDK/src/os/x.c -> libs/NitroSDK/src/os/x.c ; lib/src/x.c -> libs/misc/src/x.c
    parts = Path(src).parts
    return Path("libs", *(("misc",) + parts[1:] if parts[1] == "src" else parts[1:]))
HG_FLAGS = "-DSDK_ARM9 -DSDK_CODE_ARM -DSDK_FINALROM -sym on -enum int -lang c99 -gccext,on -proc arm946e -msgstyle gcc -gccinc -interworking -inline on,noauto -char signed"
INC_HG = " ".join(("-I" if r == "lib/include" else "-i ") + f"./{r}" for r in ROOTS)

# en-têtes nécessaires : dépendances réelles données par le compilateur
mw = wt / "tools/mwccarm"; wibo = wt / "wibo"
deps = set()
tmp = Path("/tmp/hgdeps"); tmp.mkdir(exist_ok=True)
for p in plan:
    s = settings(p["fichier"])
    cmd = f'{wibo} {mw}/{s["MWCCVER"]}/mwccarm.exe {HG_FLAGS} {s["OPTFLAGS"]} {s["EXCCFLAGS"]} {s["IPA"]} {INC_HG} -MD -c -o {tmp}/x.o {p["fichier"]}'
    subprocess.run(cmd, shell=True, cwd=hg, check=True, capture_output=True, env={**os.environ, "MWCIncludes": "lib/include"})
    dfile = hg / (Path(p["fichier"]).stem + ".d")
    txt = dfile.read_text().replace("\\\n", " ")
    dfile.unlink()
    for tok in txt.split(":", 1)[1].split():
        t = tok.replace("\\", "/")
        t = re.sub(r"^[A-Za-z]:", "", t)
        tp = Path(t)
        if tp.suffix == ".h":
            deps.add(tp.resolve().relative_to(hg) if tp.is_absolute() else tp)
print(f"en-têtes nécessaires : {len(deps)}")

# ---------- écriture ----------
build_settings = {}
for p in plan:
    d = dest(p["fichier"]); (wt / d).parent.mkdir(parents=True, exist_ok=True)
    if p["fichier"] in coupes:
        (wt / d).write_text(coupes[p["fichier"]], errors="surrogateescape")
    else:
        shutil.copy2(hg / p["fichier"], wt / d)
    s = settings(p["fichier"])
    build_settings[str(d)] = {"mwcc": s["MWCCVER"], "flags": " ".join(x for x in (HG_FLAGS, s["OPTFLAGS"], s["EXCCFLAGS"], s["IPA"]) if x)}
for h in sorted(deps):
    hs = str(h)
    roots = [r for r in ROOTS if hs.startswith(r + "/")]
    if roots:
        root = max(roots, key=len)
        target = wt / HDR / ROOTS[root] / hs[len(root) + 1:]
    else:  # en-tête voisin d'une source (#include "x.h") : même place que dans HeartGold
        target = wt / dest(hs)
        print("en-tête local :", hs)
    target.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(hg / h, target)
# un import précédent est complété, pas remplacé
bs_path = wt / "libs/build_settings.json"
ancien = json.loads(bs_path.read_text()) if bs_path.exists() else {}
bs_path.write_text(json.dumps(
    {"includes": [("-I" if r == "lib/include" else "-i ") + f"{HDR}/{ROOTS[r]}" for r in ROOTS],
     # présents dans le jeu même sans appelant : l'éditeur de liens (-dead) ne doit pas les retirer
     "force_active": sorted(set(ancien.get("force_active", [])) | {d["nom"] for p in plan for d in p["definis"] if not d["local"]}),
     "versions": ["usa"],
     "files": dict(sorted({**ancien.get("files", {}), **build_settings}.items()))}, indent=1) + "\n")

# delinks
par_mod = defaultdict(list)
for p in plan: par_mod[p["module"]].append(p)
for mod, ps in par_mod.items():
    blocs = []
    for p in sorted(ps, key=lambda p: min(s["debut"] for s in p["sections"])):
        sl = "\n".join(f"    {s['nom']} start:0x{s['debut']:08x} end:0x{s['fin']:08x}" for s in p["sections"])
        blocs.append(f"{dest(p['fichier'])}:\n    complete\n{sl}")
    path = mods[mod]["delinks"]
    path.write_text(path.read_text().rstrip("\n") + "\n\n" + "\n\n".join(blocs) + "\n")

# symboles : renommer, ajouter, retirer les étiquettes orphelines
renommes = {}
cibles = defaultdict(dict)  # module -> adresse -> (nom, local, kind, donnée définie ici)
for p in plan:
    fins = {s["nom"]: s["fin"] for s in p["sections"]}
    donnees = sorted((d for d in p["definis"] if not d["fonction"]), key=lambda d: d["adresse"])
    for d in p["definis"]:
        if d["fonction"]:
            kind = f"kind:function(arm,size=0x{d['taille']:x})"
        else:
            # taille explicite : sinon dsd étend la donnée jusqu'au symbole suivant, hors du fichier
            suiv = [x["adresse"] for x in donnees if x["section"] == d["section"] and x["adresse"] > d["adresse"]]
            taille = d["taille"] or (min(suiv) if suiv else fins[d["section"]]) - d["adresse"]
            kind = f"kind:bss(size=0x{taille:x})" if d["section"] == ".bss" else f"kind:data(byte[0x{taille:x}])"
        cibles[p["module"]][d["adresse"]] = (d["nom"], d["local"], kind, not d["fonction"])
    for nom, (mod, adr) in p["externes"].items():
        cibles[mod].setdefault(adr, (nom, False, None, False))
for mod, cib in cibles.items():
    M = mods[mod]; lines = M["lines"]
    plages = [(s["debut"], s["fin"]) for p in plan if p["module"] == mod for s in p["sections"]]
    pointes = {to for M2 in mods.values() for _, to, tmod in M2["relocs"] if reloc_module(tmod) == mod}
    nouvelles, vus = [], set()
    for ln in lines:
        q = parse(ln)
        if not q: nouvelles.append(ln); continue
        old, adr, mid, rest = q
        if adr in cib:
            if adr in vus: continue  # un seul nom par adresse
            vus.add(adr)
            nom, local, kind, donnee = cib[adr]
            if donnee: mid = f"{kind} addr:0x{adr:08x}"
            rest = re.sub(r"\s+local\b", "", rest)
            if local: rest = " local" + rest
            if old != nom: renommes[old] = nom
            nouvelles.append(f"{nom} {mid}{rest}")
        elif any(lo <= adr < hi for lo, hi in plages):
            if adr not in pointes:
                continue  # étiquette DQ9 à l'intérieur d'un fichier importé, sans équivalent HeartGold
            # visée par une relocation : dsd en a besoin, on la garde locale et bornée au fichier
            hi = min(h for l, h in plages if l <= adr < h)
            suiv = [a for a in list(cib) + list(pointes) if adr < a < hi]
            taille = (min(suiv) if suiv else hi) - adr
            if "kind:bss" in mid: mid = f"kind:bss(size=0x{taille:x}) addr:0x{adr:08x}"
            elif "kind:data" in mid: mid = f"kind:data(byte[0x{taille:x}]) addr:0x{adr:08x}"
            rest = re.sub(r"\s+local\b", "", rest)
            nouvelles.append(f"{old} {mid} local{rest}")
        else:
            # symbole DQ9 voisin dont la taille déborde sur un fichier importé : borné au début du fichier
            mm = re.search(r"size=0x([0-9a-f]+)|byte\[0x([0-9a-f]+)\]", mid)
            if mm:
                taille = int(mm.group(1) or mm.group(2), 16)
                debuts = [lo for lo, hi in plages if adr < lo < adr + taille]
                if debuts:
                    n = min(debuts) - adr
                    mid = mid[:mm.start()] + (f"size=0x{n:x}" if mm.group(1) else f"byte[0x{n:x}]") + mid[mm.end():]
                    ln = f"{old} {mid}{rest}"
            nouvelles.append(ln)
    for adr, (nom, local, kind, _) in cib.items():
        if adr not in vus:
            if kind is None:
                sec = next((n for n, (lo, hi) in M["secs"].items() if lo <= adr < hi), ".data")
                kind = "kind:bss" if sec == ".bss" else "kind:data(any)"
            nouvelles.append(f"{nom} {kind} addr:0x{adr:08x}" + (" local" if local else ""))
    # tables de données placées à l'intérieur d'une fonction (ex. _fdiv) : une étiquette sans taille,
    # sinon la somme des tailles dépasse la section une fois les fichiers importés découpés autour
    fonctions = []
    for ln in nouvelles:
        q = parse(ln)
        mm = q and re.search(r"kind:function\((arm|thumb)[^)]*size=0x([0-9a-f]+)", q[2])
        if mm: fonctions.append((q[1], q[1] + int(mm.group(2), 16), mm.group(1)))
    for k, ln in enumerate(nouvelles):
        q = parse(ln)
        if q and "kind:data" in q[2]:
            mode = next((m for lo, hi, m in fonctions if lo < q[1] < hi), None)
            if mode:
                nouvelles[k] = f"{q[0]} kind:label({mode}) addr:0x{q[1]:08x}{q[3]}"
    M["symfile"].write_text("\n".join(nouvelles) + "\n")

# anciens noms explicites utilisés dans le code du jeu
remplaces = 0
explicites = {o: n for o, n in renommes.items() if not AUTO.match(o)}
# statiques locales (isInitialized$2443) : pas d'identifiant C possible, le code du jeu ne les vise
# que dans des blocs désactivés (les références compilées sont vérifiées plus haut)
dans_code = {o: n for o, n in renommes.items() if re.fullmatch(r"[A-Za-z_]\w*", n)}
if dans_code:
    pat = re.compile(r"(?<![\w.])(" + "|".join(map(re.escape, sorted(dans_code, key=len, reverse=True))) + r")(?![\w])")
    for f in list((wt / "src").rglob("*")) + list((wt / "include").rglob("*")):
        if f.is_file() and f.suffix in (".c", ".cpp", ".h", ".hpp", ".inc", ".s"):
            t = f.read_text(errors="surrogateescape")
            t2, n = pat.subn(lambda m: dans_code[m.group(1)], t)
            if n: f.write_text(t2, errors="surrogateescape"); remplaces += n
# version japonaise : le code partagé utilise désormais les nouveaux noms ; on renomme aussi
# les symboles JPN qui portent encore l'ancien nom explicite (fix32_Sqrt...). Les noms automatiques
# (func_020ca7e0) désignent une autre fonction côté JPN : le code partagé passe par des #define jpn.
jpn_renommes = 0
if explicites:
    pat_ligne = re.compile(r"^(" + "|".join(map(re.escape, sorted(explicites, key=len, reverse=True))) + r") ", re.M)
    for f in (wt / "config/jpn/arm9").rglob("symbols.txt"):
        t = f.read_text()
        t2, n = pat_ligne.subn(lambda m: explicites[m.group(1)] + " ", t)
        if n: f.write_text(t2); jpn_renommes += n
print(f"symboles JPN renommés : {jpn_renommes}")
json.dump({"fichiers": [p["fichier"] for p in plan], "renommes": renommes}, open(wt.parent / f"import_hg_journal_{wt.name}.json", "w"), indent=1)
print(f"symboles renommés : {len(renommes)} (dont {len(explicites)} noms explicites, {remplaces} occurrences remplacées dans le code du jeu)")
