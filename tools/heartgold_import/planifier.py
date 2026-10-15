#!/usr/bin/env python3
"""Prépare l'import des fichiers de bibliothèque HeartGold retrouvés entiers dans DQ9.

Usage : planifier.py <obj HG> <resultats.json> <config dsd (config/usa/arm9)> <plan.json>
Pour chaque fichier candidat : sections placées aux adresses DQ9, octets vérifiés,
symboles à nommer, références externes à faire concorder. Écrit plan.json.
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
uniq = {}
for r in res:
    if len(r["dq9"]) == 1:
        uniq[(r["fichier"], r["fonction"])] = (r["dq9"][0]["module"], int(r["dq9"][0]["adresse"], 16))
fichiers = sorted({r["fichier"] for r in res})

plan, refus = [], defaultdict(list)
for f in fichiers:
    elf = Elf(objdir / Path(f).with_suffix(".o"))
    funcs = [s for s in elf.syms if s["type"] == 2 and s["size"] > 0 and s["shndx"] < len(elf.secs)]
    if not funcs or any((f, s["name"]) not in uniq for s in funcs):
        continue
    mod_names = {uniq[(f, s["name"])][0] for s in funcs}
    if len(mod_names) != 1:
        refus["plusieurs modules"].append(f); continue
    M = mods[mod_names.pop()]
    bases, ok, why = {}, True, ""
    # sections de code : base déduite des fonctions
    for s in funcs:
        b = uniq[(f, s["name"])][1] - (s["value"] & ~1)
        if bases.setdefault(s["shndx"], b) != b:
            ok, why = False, "fonctions non contiguës"
    # sections de données : base déduite des relocations du code
    secsym = {i: s for i, s in enumerate(elf.syms)}
    for sec_i, rels in elf.rels.items():
        if sec_i not in bases:
            continue
        for off, typ, si, add in rels:
            sym = elf.syms[si]
            if typ != R_ABS32 or sym["shndx"] == 0 or sym["shndx"] >= len(elf.secs) or sym["shndx"] in bases and elf.secs[sym["shndx"]][2] & 4:
                continue
            addend = add if add is not None else struct.unpack_from("<I", elf.data(sec_i), off)[0]
            r = M["relocs"].get(bases[sec_i] + off)
            if not r:
                ok, why = False, "relocation absente côté DQ9"; continue
            b = (r[0] + r[1]) - (sym["value"] + addend)
            if bases.setdefault(sym["shndx"], b) != b:
                ok, why = False, f"base incohérente pour {elf.secname[sym['shndx']]}"
    sections = []
    for i in elf.alloc_sections():
        name = elf.secname[i]
        if i not in bases:
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
        sections.append(dict(nom=name, debut=a, fin=a + size))
    if ok:
        fus = {}
        for sec in sorted(sections, key=lambda x: x["debut"]):
            prev = fus.get(sec["nom"])
            if prev and sec["debut"] - prev["fin"] > 3:
                ok, why = False, f"section {sec['nom']} en plusieurs morceaux"; break
            if prev: prev["fin"] = max(prev["fin"], sec["fin"])
            else: fus[sec["nom"]] = dict(sec)
        sections = list(fus.values())
    if not ok:
        refus[why.split(" pour ")[0]].append(f); continue
    # symboles définis par le fichier
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
    lib = f.split("/")[1]
    plan.append(dict(fichier=f, lib=lib, module=[k for k, v in mods.items() if v is M][0], sections=sections, definis=defs,
                     externes={k: list(v) for k, v in externes.items()}))

json.dump(plan, open(sortie, "w"), indent=1, ensure_ascii=False)
print(f"Fichiers prêts : {len(plan)} ({sum(len([d for d in p['definis'] if d['fonction']]) for p in plan)} fonctions)")
for why, fs in sorted(refus.items(), key=lambda kv: -len(kv[1])):
    print(f"  écartés ({why}) : {len(fs)}  ex. {', '.join(fs[:3])}")
par = defaultdict(int)
for p in plan: par[p["lib"]] += 1
print(dict(par))
