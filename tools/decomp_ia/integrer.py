#!/usr/bin/env python3
"""Place dans le build un fichier source dont toutes les fonctions correspondent.

  integrer.py src/Bestiary/MonsterList.cpp ov014:0x021842a0 [autre.cpp ADRESSE ...] [--partiel]

Plusieurs fichiers qui s'appellent les uns les autres s'intègrent ensemble, en une fois.

1. compile le fichier et vérifie chaque fonction, dans l'ordre, contre le jeu (comme
   `decomp.py verifier`) ; s'arrête si une seule diffère ;
2. renomme dans symbols.txt les fonctions de la plage avec les noms du fichier compilé
   (noms décorés C++ ou noms C) ;
3. ajoute l'entrée du fichier dans delinks.txt du module, marquée `complete`
   (sauf --partiel), avec la plage .text couverte.

Ensuite : `python tools/configure.py usa && ninja rom check report`.
"""

import argparse
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import decomp  # noqa: E402


def integrer(source: Path, adresse: str, partiel: bool):
    rel = source.resolve().relative_to(decomp.RACINE)
    depart = decomp.trouver(adresse)
    _, _, objet = decomp.compiler(source.resolve(), decomp.IA / "_verifier" / source.stem)
    nos = decomp.fonctions_texte(objet)
    jeu = sorted((x for x in decomp.symboles() if x.module == depart.module and x.genre == "function"
                  and x.adresse >= depart.adresse), key=lambda x: x.adresse)[:len(nos)]
    fin = jeu[-1].adresse + jeu[-1].taille

    dossier = decomp.modules_config()[depart.module]
    fichier_syms = dossier / "symbols.txt"
    texte = fichier_syms.read_text()
    for a, b in zip(jeu, nos):
        if a.nom != b.nom:
            if re.search(rf"^{re.escape(b.nom)} ", texte, re.M):
                sys.exit(f"{b.nom} existe déjà dans {fichier_syms}")
            texte, n = re.subn(rf"^{re.escape(a.nom)} (kind:function)", rf"{b.nom} \1", texte, flags=re.M)
            assert n == 1, a.nom
            print(f"renommé {a.nom} -> {b.nom}")
    fichier_syms.write_text(texte)

    delinks = dossier / "delinks.txt"
    contenu = delinks.read_text()
    if f"{rel}:" in contenu:
        sys.exit(f"{rel} est déjà dans {delinks} : mettre la plage à jour à la main.")
    entree = f"{rel}:\n" + ("" if partiel else "    complete\n") + \
        f"    .text start:{depart.adresse:#010x} end:{fin:#010x}"
    # les fichiers restent rangés par adresse de .text
    blocs = contenu.strip("\n").split("\n\n")
    def debut(bloc):
        m = re.search(r"\.text\s+start:0x([0-9a-f]+)", bloc)
        return int(m.group(1), 16) if m and not bloc.startswith(" ") else -1
    pos = next((i for i, b in enumerate(blocs) if i > 0 and debut(b) > depart.adresse), len(blocs))
    blocs.insert(pos, entree)
    delinks.write_text("\n\n".join(blocs) + "\n")
    print(f"ajouté à {delinks.relative_to(decomp.RACINE)} : .text {depart.adresse:#010x}-{fin:#010x}")


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("paires", nargs="+", metavar="SOURCE ADRESSE",
                   help="un ou plusieurs couples fichier source / adresse de début")
    p.add_argument("--partiel", action="store_true", help="ne pas marquer les fichiers complete")
    args = p.parse_args()
    if len(args.paires) % 2:
        p.error("il faut des couples SOURCE ADRESSE")
    paires = [(Path(args.paires[i]), args.paires[i + 1]) for i in range(0, len(args.paires), 2)]

    # vérification commune : les appels entre fichiers qui s'appellent les uns les autres sont acceptés
    r = subprocess.run([sys.executable, str(Path(__file__).with_name("decomp.py")), "verifier",
                        *args.paires], capture_output=True, text=True)
    print(r.stdout.rstrip())
    if r.returncode != 0:
        sys.exit("Vérification en échec : rien n'est modifié.\n" + r.stderr)
    for source, adresse in paires:
        integrer(source, adresse, args.partiel)


if __name__ == "__main__":
    main()
