#!/usr/bin/env python3
"""Cherche les fonctions des bibliothèques HeartGold présentes à l'identique dans DQ9.

Usage : comparer.py <dossier obj HeartGold> <config dsd DQ9 (config/usa/arm9)> <sortie.json>
Les octets couverts par une relocation (appels, pointeurs) sont ignorés des deux côtés.
"""
import json, re, struct, sys
from collections import defaultdict
from pathlib import Path

objdir, cfgdir, sortie = Path(sys.argv[1]), Path(sys.argv[2]), Path(sys.argv[3])
MIN_UTILES = 8  # octets non masqués minimum pour qu'une correspondance compte

def lire_elf(path):
    d = path.read_bytes()
    shoff, = struct.unpack_from("<I", d, 0x20)
    shentsize, shnum, shstrndx = struct.unpack_from("<HHH", d, 0x2E)
    secs = [struct.unpack_from("<10I", d, shoff + i * shentsize) for i in range(shnum)]
    strtab = lambda off, idx: d[secs[idx][4] + off:d.index(b"\0", secs[idx][4] + off)].decode(errors="replace")
    relocs = defaultdict(set)
    for s in secs:
        if s[1] in (9, 4):  # REL / RELA
            ent = 8 if s[1] == 9 else 12
            for o in range(s[4], s[4] + s[5], ent):
                off, = struct.unpack_from("<I", d, o)
                relocs[s[7]].add(off)  # sh_info = section ciblée
    funcs = []
    for s in secs:
        if s[1] != 2:
            continue
        for o in range(s[4], s[4] + s[5], 16):
            name, value, size, info, other, shndx = struct.unpack_from("<IIIBBH", d, o)
            if info & 0xF != 2 or size == 0 or shndx == 0 or shndx >= len(secs):
                continue
            sec = secs[shndx]
            if not sec[2] & 4:  # SHF_EXECINSTR
                continue
            start = value & ~1
            code = d[sec[4] + start:sec[4] + start + size]
            mask = {i for r in relocs[shndx] if start <= r < start + size for i in range(r - start, min(r - start + 4, size))}
            funcs.append((strtab(name, s[6]), code, mask))
    return funcs

FUNC = re.compile(r"^(\S+) kind:function\((arm|thumb)[^)]*?size=0x([0-9a-f]+)[^)]*\) addr:0x([0-9a-f]+)")
def modules():
    cfg = (cfgdir / "config.yaml").read_text()
    for m in re.finditer(r"name: (\S+)\n\s+object: '([^']+)'\n(?:.*\n)*?\s+delinks: (\S+)\n\s+symbols: (\S+)\n\s+relocations: (\S+)", cfg):
        name, obj, delinks, syms, rels = m.groups()
        objp = (cfgdir / obj).resolve()
        base = min(int(x, 16) for x in re.findall(r"^\s+\.\w+\s+start:0x([0-9a-f]+)", (cfgdir / delinks).read_text(), re.M))
        yield name, objp.read_bytes(), base, cfgdir / syms, cfgdir / rels

dq = defaultdict(list)
total_dq = 0
for name, data, base, syms, rels in modules():
    masque = {int(x, 16) for x in re.findall(r"from:0x([0-9a-f]+)", rels.read_text())}
    for line in syms.read_text().splitlines():
        m = FUNC.match(line)
        if not m:
            continue
        sym, mode, size, addr = m.group(1), m.group(2), int(m.group(3), 16), int(m.group(4), 16)
        code = data[addr - base:addr - base + size]
        if len(code) != size:
            continue
        mask = {i for r in masque if addr <= r < addr + size for i in range(r - addr, min(r - addr + 4, size))}
        dq[size].append((name, sym, addr, mode, code, mask))
        total_dq += 1

res, sans_taille, ambigus = [], 0, 0
for o in sorted(objdir.rglob("*.o")):
    lib = str(o.relative_to(objdir)).split("/")[1]
    lib = "NitroCrypto/SPL" if lib == "src" else lib
    for fname, code, mask in lire_elf(o):
        cands = dq.get(len(code), [])
        if not cands:
            sans_taille += 1
        hits = []
        for (mod, sym, addr, mode, c2, m2) in cands:
            ign = mask | m2
            if len(code) - len(ign) < MIN_UTILES:
                continue
            if all(a == b for i, (a, b) in enumerate(zip(code, c2)) if i not in ign):
                hits.append((mod, sym, addr, mode))
        res.append({"lib": lib, "fichier": str(o.relative_to(objdir).with_suffix(".c")), "fonction": fname,
                    "taille": len(code), "dq9": [{"module": h[0], "symbole": h[1], "adresse": hex(h[2]), "mode": h[3]} for h in hits]})

sortie.write_text(json.dumps(res, indent=1, ensure_ascii=False))
par_lib = defaultdict(lambda: [0, 0, 0, 0])
dq_uniques = {}
for r in res:
    st = par_lib[r["lib"]]
    st[0] += 1
    if r["dq9"]:
        st[1] += 1
        if len(r["dq9"]) == 1:
            st[2] += 1
            h = r["dq9"][0]
            dq_uniques[(h["module"], h["adresse"])] = r["taille"]
print(f"Fonctions DQ9 analysées : {total_dq}")
print(f"{'bibliothèque':16} {'fonctions HG':>12} {'trouvées':>9} {'uniques':>8}")
for lib, (n, t, u, _) in sorted(par_lib.items(), key=lambda kv: -kv[1][1]):
    print(f"{lib:16} {n:12} {t:9} {u:8}")
octets = sum(dq_uniques.values())
print(f"\nFonctions DQ9 distinctes identifiées sans ambiguïté : {len(dq_uniques)} ({octets} octets)")
par_mod = defaultdict(int)
for (mod, _), _ in dq_uniques.items():
    par_mod[mod] += 1
print("Par module DQ9 :", dict(sorted(par_mod.items(), key=lambda kv: -kv[1])))
