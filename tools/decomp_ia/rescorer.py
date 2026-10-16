#!/usr/bin/env python3
"""Recompile la proposition finale de chaque fonction d'un module et donne son score réel.

  rescorer.py ov006 [ov008 ...]

Les rapports des agents peuvent être faux (agent relancé qui écrase un meilleur fichier, ligne de
résultat écrite avant le dernier essai) : seul le fichier build/ia/<fn>/<fn>.cpp compte. Les essais
sont journalisés avec agent=orchestrateur, ils ne comptent pas comme essais d'agent.
"""

import os
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import decomp  # noqa: E402

os.environ["DECOMP_AGENT"] = "orchestrateur"
for module in sys.argv[1:]:
    fonctions = sorted((s for s in decomp.symboles() if s.module == module and s.genre == "function"
                        and s.nom.startswith("func_")), key=lambda s: s.adresse)
    ok = 0
    for s in fonctions:
        src = decomp.IA / s.nom / f"{s.nom}.cpp"
        if not src.exists():
            print(f"{s.nom} {s.taille:#x} ABSENT")
            continue
        r = subprocess.run([sys.executable, str(Path(__file__).with_name("decomp.py")), "essai", s.nom, str(src)],
                           capture_output=True, text=True)
        ligne = next((l for l in r.stdout.splitlines() if l.startswith("SCORE")), "SCORE ? (échec de compilation)")
        score = ligne.split()[1]
        ok += score == "100.00"
        print(f"{s.nom} {s.taille:#x} {score}")
    print(f"{module} : {ok}/{len(fonctions)} à 100 %")
