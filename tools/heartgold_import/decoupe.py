"""Retire des définitions de fonctions d'une source C, sans préprocesseur ni analyseur complet.

Une définition de fonction est un bloc { } de premier niveau précédé d'une liste de paramètres ;
elle commence après la fin de l'élément de premier niveau précédent (« ; », « } » d'un autre corps
de fonction ou ligne de préprocesseur), commentaires d'en-tête compris.
"""
import re

IDENT = re.compile(r"[A-Za-z_]\w*")


def _elements(texte):
    """Rend (début, fin, nom) pour chaque définition de fonction de premier niveau."""
    defs, i, n = [], 0, len(texte)
    debut_elem, prof_acc, prof_par, debut_ligne = 0, 0, 0, True
    nom_cand, corps = None, None
    while i < n:
        c = texte[i]
        if debut_ligne and c in " \t":
            i += 1
            continue
        if debut_ligne and c == "#":
            # ligne de préprocesseur, avec ses continuations
            while i < n and not (texte[i] == "\n" and texte[i - 1] != "\\"):
                i += 1
            if prof_acc == 0 and prof_par == 0:
                debut_elem = i + 1
            continue
        debut_ligne = c == "\n"
        if texte.startswith("//", i):
            i = texte.find("\n", i)
            i = n if i < 0 else i
            continue
        if texte.startswith("/*", i):
            j = texte.find("*/", i + 2)
            i = n if j < 0 else j + 2
            continue
        if c in "\"'":
            j = i + 1
            while j < n and texte[j] != c:
                j += 2 if texte[j] == "\\" else 1
            i = j + 1
            continue
        if c == "(":
            if prof_acc == 0 and prof_par == 0:
                m = None
                for m in IDENT.finditer(texte, debut_elem, i):
                    pass
                # premier identifiant collé à une parenthèse dans l'élément : nom de la fonction
                if nom_cand is None and m and texte[m.end():i].strip() == "":
                    nom_cand = m.group(0)
            prof_par += 1
        elif c == ")":
            prof_par -= 1
        elif c == "{":
            if prof_acc == 0 and prof_par == 0:
                corps = nom_cand if texte[debut_elem:i].rstrip().endswith(")") else None
            prof_acc += 1
        elif c == "}":
            prof_acc -= 1
            if prof_acc == 0 and corps:
                defs.append((debut_elem, i + 1, corps))
                debut_elem, corps, nom_cand = i + 1, None, None
        elif c == ";" and prof_acc == 0 and prof_par == 0:
            debut_elem, nom_cand = i + 1, None
        i += 1
    return defs


def couper(texte, noms):
    """Retire les définitions des fonctions nommées. Rend (texte, noms introuvables)."""
    defs = _elements(texte)
    trouves = {nom for _, _, nom in defs}
    garde, pos = [], 0
    for debut, fin, nom in defs:
        if nom in noms:
            # la fin de ligne de l'élément précédent reste en place
            m = re.match(r"[ \t]*\n", texte[debut:fin])
            if m:
                debut += m.end()
            garde.append(texte[pos:debut])
            pos = fin
            # ligne vide laissée par la définition retirée
            if texte.startswith("\n", pos):
                pos += 1
    garde.append(texte[pos:])
    return "".join(garde), sorted(set(noms) - trouves)
