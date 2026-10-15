#!/usr/bin/env python3
"""Agrège les mesures d'une campagne de décompilation par agents.

Sources (dans build/ia/) :
  journal.jsonl   un enregistrement par `decomp.py essai` (écrit automatiquement)
  agents.jsonl    un enregistrement par agent : fonction, jetons, duree_ms, outils, vague
  <fn>/resultat.json  rendu de l'agent (nom proposé, rôle, blocage)

  mesures.py [--csv sortie.csv] [--md sortie.md]
"""

import argparse
import csv
import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import decomp  # noqa: E402

TRANCHES = [(0, 0x40, "≤ 0x40"), (0x41, 0x100, "0x41–0x100"), (0x101, 0x200, "0x101–0x200"),
            (0x201, 0x400, "0x201–0x400"), (0x401, 1 << 30, "> 0x400")]


def charger():
    ia = decomp.IA
    essais = {}
    for l in (ia / "journal.jsonl").read_text().splitlines():
        e = json.loads(l)
        essais.setdefault(e["fonction"], []).append(e)
    agents = {}
    if (ia / "agents.jsonl").exists():
        lignes_agents = [json.loads(l) for l in (ia / "agents.jsonl").read_text().splitlines()]
        # un agent peut notifier deux fois (il reprend après un arrêt) : son dernier relevé, cumulé, fait foi
        derniere = {a["tache"]: i for i, a in enumerate(lignes_agents) if a.get("tache")}
        lignes_agents = [a for i, a in enumerate(lignes_agents) if not a.get("tache") or derniere[a["tache"]] == i]
        for lot in lignes_agents:
            # un agent qui a traité un lot de fonctions : jetons et durée partagés à parts égales
            noms = lot.get("fonctions") or [lot["fonction"]]
            for nom in noms:
                a = dict(lot, fonction=nom)
                for k in ("jetons", "duree_ms", "outils"):
                    a[k] = lot.get(k, 0) / len(noms)
                # un agent relancé sur la même fonction s'additionne
                prec = agents.get(nom)
                if prec:
                    for k in ("jetons", "duree_ms", "outils"):
                        a[k] = a.get(k, 0) + prec.get(k, 0)
                agents[nom] = a
    lignes = []
    for fn in sorted(set(essais) | set(agents)):
        if fn not in agents:
            continue  # fonction jamais confiée à un agent (essais manuels de mise au point)
        es = essais.get(fn, [])
        a = agents.get(fn, {})
        res = {}
        if (ia / fn / "resultat.json").exists():
            try:
                res = json.loads((ia / fn / "resultat.json").read_text())
            except json.JSONDecodeError:
                pass
        taille = es[0]["taille"] if es else next((s.taille for s in decomp.symboles() if s.nom == fn), 0)
        meilleur = max((e["score"] for e in es), default=0.0)
        # les relances de l'orchestrateur (rescorage après correction de l'outil) ne comptent pas comme essais
        es = [e for e in es if e.get("agent") != "orchestrateur"]
        lignes.append({
            "fonction": fn, "taille": taille, "essais": len(es), "score": meilleur,
            "reussite": meilleur >= 100.0,
            # score relevé après correction de l'outil : tous les essais de l'agent comptent
            "essais_pour_100": next((i + 1 for i, e in enumerate(es) if e["score"] >= 100.0),
                                    len(es) if meilleur >= 100.0 else None),
            "duree_s": round(a.get("duree_ms", 0) / 1000), "jetons": a.get("jetons", 0),
            "nom_propose": res.get("nom_propose", ""), "blocage": res.get("blocage", ""),
            "vague": a.get("vague", ""),
        })
    return lignes


def tableau(lignes):
    out = ["| Tranche (octets) | Fonctions | Réussies (100 %) | Taux | Essais moyens | Durée moyenne | Jetons moyens |",
           "|---|---|---|---|---|---|---|"]
    for lo, hi, nom in TRANCHES + [(0, 1 << 30, "**Total**")]:
        g = [l for l in lignes if lo <= (l["taille"] or 0) <= hi]
        if not g:
            continue
        ok = sum(l["reussite"] for l in g)
        out.append(f"| {nom} | {len(g)} | {ok} | {100 * ok / len(g):.0f} % | "
                   f"{sum(l['essais'] for l in g) / len(g):.1f} | "
                   f"{sum(l['duree_s'] for l in g) / len(g) / 60:.1f} min | "
                   f"{sum(l['jetons'] for l in g) / len(g) / 1000:.0f} k |")
    return "\n".join(out)


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--csv")
    p.add_argument("--md")
    args = p.parse_args()
    lignes = charger()
    if args.csv:
        with open(args.csv, "w", newline="") as f:
            w = csv.DictWriter(f, fieldnames=list(lignes[0]), lineterminator="\n")
            w.writeheader()
            w.writerows(lignes)
    t = tableau(lignes)
    octets_ok = sum(l["taille"] for l in lignes if l["reussite"])
    agent_s = sum(l["duree_s"] for l in lignes)
    resume = (f"{sum(l['reussite'] for l in lignes)}/{len(lignes)} fonctions à 100 %, "
              f"{octets_ok:#x} octets ({octets_ok} o) ; temps agent cumulé {agent_s / 3600:.2f} h ; "
              f"jetons {sum(l['jetons'] for l in lignes) / 1e6:.2f} M")
    print(t + "\n\n" + resume)
    if args.md:
        Path(args.md).write_text(t + "\n\n" + resume + "\n")


if __name__ == "__main__":
    main()
