#!/usr/bin/env python3
"""Contrôles de vraisemblance sur resultats.json : noms concordants, contiguïté, fichiers entiers."""
import json, sys
from collections import defaultdict
res = json.load(open(sys.argv[1]))

# 1. Noms déjà connus dans DQ9 : concordent-ils ?
ok = ko = 0
for r in res:
    if len(r["dq9"]) == 1:
        s = r["dq9"][0]["symbole"]
        if not s.startswith("func_") and not s.startswith("_Z"):
            if s == r["fonction"]: ok += 1
            else:
                ko += 1
                print("  nom différent :", r["fonction"], "<->", s)
print(f"Noms déjà connus dans DQ9 : {ok} concordent, {ko} diffèrent")

# 2. Fichiers HeartGold retrouvés entiers et d'un seul bloc dans DQ9
par_fichier = defaultdict(list)
for r in res:
    par_fichier[r["fichier"]].append(r)
entiers = partiels = 0
fonctions_entiers = octets_entiers = 0
par_lib = defaultdict(lambda: [0, 0])
for f, fs in par_fichier.items():
    lib = fs[0]["lib"]
    par_lib[lib][1] += 1
    if all(len(x["dq9"]) == 1 for x in fs):
        hits = sorted((x["dq9"][0]["module"], int(x["dq9"][0]["adresse"], 16), x["taille"]) for x in fs)
        mods = {h[0] for h in hits}
        span = hits[-1][1] + hits[-1][2] - hits[0][1]
        taille = sum(h[2] for h in hits)
        if len(mods) == 1 and span - taille <= 4 * len(hits):  # contigu, au padding près
            entiers += 1; par_lib[lib][0] += 1
            fonctions_entiers += len(fs); octets_entiers += taille
            continue
    if any(x["dq9"] for x in fs):
        partiels += 1
print(f"Fichiers HeartGold retrouvés entiers et contigus : {entiers} ({fonctions_entiers} fonctions, {octets_entiers} octets) ; partiels : {partiels}")
for lib, (e, n) in sorted(par_lib.items(), key=lambda kv: -kv[1][0]):
    print(f"  {lib:16} {e:4} / {n} fichiers")

# 3. Plages d'adresses couvertes dans main
adrs = sorted(int(r["dq9"][0]["adresse"], 16) for r in res if len(r["dq9"]) == 1 and r["dq9"][0]["module"] == "main")
print(f"main : de {hex(adrs[0])} à {hex(adrs[-1])}")
