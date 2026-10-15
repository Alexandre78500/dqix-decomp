#!/usr/bin/env python3
"""Découpe les fonctions non décompilées d'un module en lots, un lot par agent.

  lots.py ov006 [--octets 0x100] [--max 6]

Les petites fonctions voisines sont groupées (au plus --max fonctions et --octets octets par
lot) : un agent qui en traite plusieurs à la suite paie une seule fois son contexte de départ
et profite de ce qu'il a établi sur la première. Une fonction plus grande que --octets forme
un lot à elle seule. Écrit build/ia/lots_<module>.json et affiche un lot par ligne.
"""

import argparse
import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import decomp  # noqa: E402


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("module")
    p.add_argument("--octets", type=lambda x: int(x, 0), default=0x100)
    p.add_argument("--max", type=int, default=6)
    args = p.parse_args()

    fonctions = sorted((s for s in decomp.symboles() if s.module == args.module and s.genre == "function"
                        and s.nom.startswith("func_")), key=lambda s: s.adresse)
    lots, courant = [], []
    for s in fonctions:
        taille = s.taille or 0
        if taille > args.octets:
            lots.append([s])
            continue
        if courant and (len(courant) >= args.max
                        or sum(x.taille or 0 for x in courant) + taille > args.octets):
            lots.append(courant)
            courant = []
        courant.append(s)
    if courant:
        lots.append(courant)
    # les lots sont rangés du plus léger au plus lourd : les gros partent en dernier
    lots.sort(key=lambda l: sum(x.taille or 0 for x in l))

    sortie = [{"lot": i + 1, "fonctions": [s.nom for s in l], "octets": sum(s.taille or 0 for s in l)}
              for i, l in enumerate(lots)]
    decomp.IA.mkdir(parents=True, exist_ok=True)
    (decomp.IA / f"lots_{args.module}.json").write_text(json.dumps(sortie, indent=1) + "\n")
    for l in sortie:
        print(f"{l['lot']:3d} {l['octets']:#7x} {' '.join(l['fonctions'])}")
    print(f"{len(fonctions)} fonctions, {len(sortie)} lots", file=sys.stderr)


if __name__ == "__main__":
    main()
