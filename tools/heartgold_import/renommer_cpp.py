#!/usr/bin/env python3
"""Donne leur nom Nintendo (liaison C) à des fonctions du SDK que le jeu a décompilées en C++.

Usage : renommer_cpp.py <worktree DQ9> <correspondances>
Chaque ligne du fichier de correspondances : « _Z20DisableIRQInterruptsv -> OS_DisableInterrupts ».
Le code HeartGold importé appelle ces fonctions sous leur nom C : leur première déclaration
(en-tête du jeu, sinon la source qui les définit) passe en extern "C", le nom C++ est remplacé
partout dans src/ et include/, et symbols.txt (USA et JPN) suit.
"""
import re
import sys
from pathlib import Path

wt, corresp = Path(sys.argv[1]).resolve(), Path(sys.argv[2])
SOURCES = [f for d in ("include", "src") for f in sorted((wt / d).rglob("*"))
           if f.is_file() and f.suffix in (".h", ".hpp", ".c", ".cpp", ".inc")]
SYMBOLES = sorted((wt / "config").rglob("symbols.txt"))


def nom_simple(decore):
    """_Z20DisableIRQInterruptsv -> DisableIRQInterrupts (fonctions libres seulement)."""
    m = re.match(r"_Z(\d+)", decore)
    return decore[m.end():m.end() + int(m.group(1))] if m else None


paires = []
for ln in corresp.read_text().splitlines():
    if ln.strip():
        decore, nouveau = (x.strip() for x in ln.split("->"))
        paires.append((decore, nom_simple(decore), nouveau))

textes = {f: f.read_text(errors="surrogateescape") for f in SOURCES}
for decore, ancien, nouveau in paires:
    # déclaration de premier niveau : type de retour en début de ligne, puis le nom et sa parenthèse
    decl = re.compile(r"^(?!\s*(?:return|else)\b)([A-Za-z_][\w \t*&:<>,]*?[\s*&])" + re.escape(ancien) + r"\s*\(", re.M)
    cible = None
    for f in [f for f in SOURCES if f.suffix in (".h", ".hpp")] + [f for f in SOURCES if f.suffix not in (".h", ".hpp")]:
        m = decl.search(textes[f])
        if m:
            cible = (f, m.start())
            break
    if not cible:
        sys.exit(f"{ancien} : déclaration introuvable")
    f, pos = cible
    if not textes[f][pos:].startswith('extern "C"'):
        textes[f] = textes[f][:pos] + 'extern "C" ' + textes[f][pos:]
    mot = re.compile(r"(?<![\w.>:])" + re.escape(ancien) + r"(?!\w)")
    total = 0
    for g in SOURCES:
        textes[g], n = mot.subn(nouveau, textes[g])
        total += n
    print(f"{ancien} -> {nouveau} : extern \"C\" dans {f.relative_to(wt)}, {total} occurrences")

for f in SOURCES:
    if textes[f] != f.read_text(errors="surrogateescape"):
        f.write_text(textes[f], errors="surrogateescape")
for f in SYMBOLES:
    t = f.read_text()
    t2 = t
    for decore, _, nouveau in paires:
        t2 = re.sub(r"^" + re.escape(decore) + r" ", nouveau + " ", t2, flags=re.M)
    if t2 != t:
        f.write_text(t2)
