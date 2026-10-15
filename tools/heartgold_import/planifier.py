#!/usr/bin/env python3
"""Prépare l'import des fichiers de bibliothèque HeartGold retrouvés dans DQ9.

Usage : planifier.py <obj HG> <resultats.json> <config dsd (config/usa/arm9)> <plan.json>
Pour chaque fichier candidat : sections placées aux adresses DQ9, octets vérifiés,
symboles à nommer, références externes à faire concorder. Écrit plan.json.

mwccarm range chaque fonction et chaque donnée dans sa propre section : l'éditeur de liens
d'origine (-dead) a retiré celles que personne n'appelait. Un fichier retrouvé en partie est
retenu quand les sections présentes se suivent sans trou dans DQ9 : compilé en entier, il
donnera les mêmes octets une fois les sections inutiles retirées par notre propre -dead.
"""
import json, re, struct, sys
from collections import defaultdict
from pathlib import Path

objdir, resfile, cfgdir, sortie = map(Path, sys.argv[1:5])
R_ABS32, CALLS = 2, {1, 10, 28, 29}

class Elf:
    def __init__(self, path):
        d = self.d = path.read_bytes()
        shoff, = struct.unpack_from("<I", d, 0x20)
        shentsize, shnum, shstrndx = struct.unpack_from("<HHH", d, 0x2E)
        self.secs = [struct.unpack_from("<10I", d, shoff + i * shentsize) for i in range(shnum)]
        self.secname = [self.str(shstrndx, s[0]) for s in self.secs]
        self.syms, self.rels = [], defaultdict(list)
        for i, s in enumerate(self.secs):
            if s[1] == 2:
                for o in range(s[4], s[4] + s[5], 16):
                    n, v, sz, info, other, sh = struct.unpack_from("<IIIBBH", d, o)
                    self.syms.append(dict(name=self.str(s[6], n), value=v, size=sz, type=info & 15, bind=info >> 4, shndx=sh))
        for s in self.secs:
            if s[1] in (4, 9):
                ent = 12 if s[1] == 4 else 8
                for o in range(s[4], s[4] + s[5], ent):
                    off, info = struct.unpack_from("<II", d, o)
                    add = struct.unpack_from("<i", d, o + 8)[0] if s[1] == 4 else None
                    self.rels[s[7]].append((off, info & 255, info >> 8, add))
    def str(self, idx, off):
        b = self.secs[idx][4] + off
        return self.d[b:self.d.index(b"\0", b)].decode(errors="replace")
    def data(self, i):
        s = self.secs[i]
        return b"" if s[1] == 8 else self.d[s[4]:s[4] + s[5]]
    def alloc_sections(self):
        return [i for i, s in enumerate(self.secs) if s[2] & 2 and s[5] > 0]

# --- DQ9 ---
cfg = (cfgdir / "config.yaml").read_text()
mods = {}
for m in re.finditer(r"name: (\S+)\n\s+object: '([^']+)'\n(?:.*\n)*?\s+delinks: (\S+)\n\s+symbols: (\S+)\n\s+relocations: (\S+)", cfg):
    name, obj, delinks, syms, rels = m.groups()
    dl = (cfgdir / delinks).read_text()
    head = dl.split("\n\n", 1)[0]
    secs = {mm.group(1): (int(mm.group(2), 16), int(mm.group(3), 16)) for mm in re.finditer(r"^\s+(\.\w+)\s+start:0x([0-9a-f]+) end:0x([0-9a-f]+)", head, re.M)}
    taken = [(int(a, 16), int(b, 16)) for a, b in re.findall(r"^\s+\.\w+ start:0x([0-9a-f]+) end:0x([0-9a-f]+)\s*$", dl.split("\n\n", 1)[1] if "\n\n" in dl else "", re.M)]
    symlines = (cfgdir / syms).read_text().splitlines()
    byaddr = defaultdict(list)
    for ln in symlines:
        mm = re.match(r"^(\S+) (kind:\S+(?: \S+)*?) addr:0x([0-9a-f]+)(.*)$", ln)
        if mm:
            byaddr[int(mm.group(3), 16)].append(mm.group(1))
    relocs = {}
    for ln in (cfgdir / rels).read_text().splitlines():
        mm = re.match(r"from:0x([0-9a-f]+) kind:(\S+) to:0x([0-9a-f]+)(?: add:(-?0x[0-9a-f]+))? module:(\S+)", ln)
        if mm:
            relocs[int(mm.group(1), 16)] = (int(mm.group(3), 16), int(mm.group(4), 16) if mm.group(4) else 0, mm.group(5))
    base = min(a for a, b in secs.values())
    mods[name] = dict(data=(cfgdir / obj).resolve().read_bytes(), base=base, secs=secs, taken=taken, byaddr=byaddr, relocs=relocs, symfile=str(syms), delinks=str(delinks))
modname = {"main": "main", "itcm": "itcm", "dtcm": "dtcm"}
def reloc_module(s):  # "overlay(31)" -> "ov031"
    m = re.match(r"overlay\((\d+)\)", s)
    return f"ov{int(m.group(1)):03d}" if m else s

res = json.load(open(resfile))
cands = {}
for r in res:
    cands[(r["fichier"], r["fonction"])] = [(d["module"], int(d["adresse"], 16)) for d in r["dq9"]]
fichiers = sorted({r["fichier"] for r in res})
# sections que l'éditeur de liens retire quand rien ne les référence
RETIRABLES = {".text", ".data", ".rodata", ".bss", ".sdata", ".sbss"}
EXEC, FORCE_ACTIVE = 4, 0x20000000

plan, refus = [], defaultdict(list)
for f in fichiers:
    elf = Elf(objdir / Path(f).with_suffix(".o"))
    funcs = [s for s in elf.syms if s["type"] == 2 and s["size"] > 0 and s["shndx"] < len(elf.secs)]
    uniques = [s for s in funcs if len(cands.get((f, s["name"]), [])) == 1]
    if not uniques:
        continue
    mod_names = {cands[(f, s["name"])][0][0] for s in uniques}
    if len(mod_names) != 1:
        refus["plusieurs modules"].append(f); continue
    modn = mod_names.pop()
    M = mods[modn]
    bases, conflits = {}, []
    def placer(i, b, d):
        if elf.secs[i][2] & EXEC: b &= ~1
        if bases.setdefault(i, b) != b: conflits.append(f"base incohérente pour {elf.secname[i]} ({d})")
    # sections de code : base déduite des fonctions retrouvées sans ambiguïté
    for s in uniques:
        placer(s["shndx"], cands[(f, s["name"])][0][1] - (s["value"] & ~1), "fonction")
    while True:
        avant = len(bases)
        # sections visées par une section placée : appels et pointeurs donnent leur adresse DQ9
        for sec_i in list(bases):
            for off, typ, si, add in elf.rels.get(sec_i, []):
                sym = elf.syms[si]
                if sym["shndx"] == 0 or sym["shndx"] >= len(elf.secs) or (typ != R_ABS32 and typ not in CALLS):
                    continue
                r = M["relocs"].get(bases[sec_i] + off)
                if not r:
                    conflits.append("relocation absente côté DQ9"); continue
                if reloc_module(r[2]) != modn:
                    conflits.append("relocation vers un autre module"); continue
                if typ == R_ABS32:
                    addend = add if add is not None else struct.unpack_from("<I", elf.data(sec_i), off)[0]
                    placer(sym["shndx"], (r[0] + r[1]) - (sym["value"] + addend), "pointeur")
                else:
                    placer(sym["shndx"], (r[0] + r[1] & ~1) - (sym["value"] & ~1), "appel")
        # fonction présente en plusieurs exemplaires dans DQ9 : celui qui suit la section placée précédente,
        # ou, en tête de fichier, celui qui précède la première section placée
        for s in funcs:
            i = s["shndx"]
            if i in bases or len(cands.get((f, s["name"]), [])) < 2:
                continue
            prec = [j for j in bases if elf.secname[j] == elf.secname[i] and j < i]
            suiv = [j for j in bases if elf.secname[j] == elf.secname[i] and j > i]
            al, taille = max(elf.secs[i][8], 4), elf.secs[i][5]
            cb = sorted({a - (s["value"] & ~1) for m, a in cands[(f, s["name"])] if m == modn})
            if prec:
                fin = bases[max(prec)] + elf.secs[max(prec)][5]
                ok_c = [b for b in cb if b % al == 0 and 0 <= b - fin < al]
            elif suiv:
                debut, al2 = bases[min(suiv)], max(elf.secs[min(suiv)][8], 4)
                ok_c = [b for b in cb if b % al == 0 and 0 <= debut - (b + taille) < al2]
            else:
                ok_c = []
            if len(ok_c) == 1:
                placer(i, ok_c[0], "voisinage")
        if len(bases) == avant:
            break
    if conflits:
        refus[conflits[0].split(" (")[0]].append(f); continue
    ok, why, morceaux, partiel = True, "", [], False
    for i in elf.alloc_sections():
        name = elf.secname[i]
        if i not in bases:
            if name in RETIRABLES:
                partiel = True; continue
            ok, why = False, f"section {name} non placée"; break
        a, size = bases[i], elf.secs[i][5]
        home = [n for n, (lo, hi) in M["secs"].items() if lo <= a and a + size <= hi]
        if home != [name]:
            ok, why = False, f"section {name} tombe dans {home}"; break
        if any(a < hi and lo < a + size for lo, hi in M["taken"]):
            ok, why = False, "chevauche un fichier déjà déclaré"; break
        if elf.secs[i][1] != 8:
            mine, theirs = elf.data(i), M["data"][a - M["base"]:a - M["base"] + size]
            mask = {r[0] + k for r in elf.rels.get(i, []) for k in range(4)}
            if any(x != y for k, (x, y) in enumerate(zip(mine, theirs)) if k not in mask) or len(theirs) != size:
                ok, why = False, f"octets différents dans {name}"; break
        morceaux.append((name, i, a, a + size))
    if ok:
        # une section en morceaux : ordre du fichier, sans trou autre que l'alignement
        fus = {}
        for name, i, a, b in sorted(morceaux, key=lambda x: x[2]):
            prev = fus.get(name)
            if prev:
                al = max(elf.secs[i][8], 4)
                if i < prev["indice"] or not 0 <= a - prev["fin"] < al:
                    ok, why = False, f"section {name} en plusieurs morceaux"; break
                prev["fin"], prev["indice"] = max(prev["fin"], b), i
            else:
                fus[name] = dict(nom=name, debut=a, fin=b, indice=i)
        sections = [dict(nom=s["nom"], debut=s["debut"], fin=s["fin"]) for s in fus.values()]
    if not ok:
        refus[why.split(" pour ")[0]].append(f); continue
    # symboles définis par le fichier (sections présentes dans DQ9 seulement)
    defs, externes = [], {}
    for s in elf.syms:
        if s["shndx"] in bases and s["type"] in (1, 2) and not s["name"].startswith(("@", "$", ".")):
            defs.append(dict(nom=s["name"], adresse=bases[s["shndx"]] + (s["value"] & ~1), local=s["bind"] == 0,
                             fonction=s["type"] == 2, taille=s["size"], section=elf.secname[s["shndx"]]))
    # références vers l'extérieur : le nom HeartGold doit exister à l'adresse visée côté DQ9
    for sec_i, rels in elf.rels.items():
        if sec_i not in bases:
            continue
        for off, typ, si, add in rels:
            sym = elf.syms[si]
            if 0 < sym["shndx"] < len(elf.secs) and sym["shndx"] not in bases:
                ok, why = False, "référence vers une section retirée"; break
            if sym["shndx"] != 0:
                continue
            r = M["relocs"].get(bases[sec_i] + off)
            if not r:
                ok, why = False, "référence externe introuvable"; break
            cible = (reloc_module(r[2]), r[0] + r[1])
            if externes.setdefault(sym["name"], cible) != cible:
                ok, why = False, "référence externe ambiguë"; break
        if not ok:
            break
    if not ok:
        refus[why].append(f); continue
    # sections retirées par l'éditeur de liens d'origine : ce qu'elles référencent décide si l'import
    # doit les couper de la source (référence introuvable dans DQ9, nom global déjà pris)
    fn_de = defaultdict(list)
    for s in elf.syms:
        if s["type"] == 2 and 0 < s["shndx"] < len(elf.secs) and not s["name"].startswith(("@", "$", ".")):
            fn_de[s["shndx"]].append(s["name"])
    retirees = []
    for i in elf.alloc_sections():
        if i in bases:
            continue
        ext, inter = set(), set()
        for off, typ, si, add in elf.rels.get(i, []):
            sym = elf.syms[si]
            if sym["shndx"] == 0:
                ext.add(sym["name"])
            elif sym["shndx"] < len(elf.secs) and sym["shndx"] != i and sym["shndx"] not in bases:
                inter.update(fn_de.get(sym["shndx"], []))
        # section marquée « force_active » (#pragma force_active on) : -dead la garderait
        retirees.append(dict(section=elf.secname[i], noms=fn_de.get(i, []), externes=sorted(ext), internes=sorted(inter),
                             force=bool(elf.secs[i][2] & FORCE_ACTIVE),
                             globaux=[s["name"] for s in elf.syms if s["shndx"] == i and s["bind"] == 1 and s["type"] in (1, 2)]))
    lib = f.split("/")[1]
    plan.append(dict(fichier=f, lib=lib, module=modn, partiel=partiel, retirees=retirees, sections=sections, definis=defs,
                     externes={k: list(v) for k, v in externes.items()}))

json.dump(plan, open(sortie, "w"), indent=1, ensure_ascii=False)
json.dump(refus, open(sortie.with_suffix(".refus.json"), "w"), indent=1, ensure_ascii=False)
nf = lambda ps: sum(len([d for d in p["definis"] if d["fonction"]]) for p in ps)
print(f"Fichiers prêts : {len(plan)} ({nf(plan)} fonctions), dont {sum(p['partiel'] for p in plan)} retrouvés en partie ({nf([p for p in plan if p['partiel']])} fonctions)")
for why, fs in sorted(refus.items(), key=lambda kv: -len(kv[1])):
    print(f"  écartés ({why}) : {len(fs)}  ex. {', '.join(fs[:3])}")
par = defaultdict(int)
for p in plan: par[p["lib"]] += 1
print(dict(par))
