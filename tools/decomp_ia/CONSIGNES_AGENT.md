# Consignes pour un agent de décompilation (une fonction)

Tu décompiles **une seule fonction** de Dragon Quest IX (DS, USA) en C++ qui, compilé avec
mwccarm 2.0/sp2p2 `-O2` et les options exactes du projet, redonne **les mêmes octets** que le jeu.
Le code de jeu a été écrit en C++ par Level-5 ; il n'existe aucune source de référence.

## Règles

- Travaille uniquement dans `build/ia/<FONCTION>/` (bac à sable ignoré par git). Ne modifie
  **rien** dans `src/`, `include/`, `config/`, `tools/` ; pas de `git`, pas de `ninja`.
- Les commandes se lancent depuis la racine du dépôt, avec `.venv/bin/python`.
- Limite : **40 essais** (`essai`) au maximum. Arrête dès 100 %.

## Outil

```
.venv/bin/python tools/decomp_ia/decomp.py info FONCTION         # assembleur cible + contexte
.venv/bin/python tools/decomp_ia/decomp.py squelette FONCTION -o build/ia/FONCTION/FONCTION.cpp
.venv/bin/python tools/decomp_ia/decomp.py essai FONCTION build/ia/FONCTION/FONCTION.cpp
```

`info` donne l'assembleur (`*` = cible de branchement), les fonctions appelées (avec leur nom
C++ si elles en ont un et l'en-tête où la classe est déclarée), les données référencées (contenu,
chaînes), les appelants et les voisines. `essai` compile, puis affiche le score objdiff et les
seules lignes qui diffèrent (`R` remplacée, `O` opcode, `A` argument/registre, `-`/`+` décalage).
Chaque essai est journalisé automatiquement (c'est ainsi qu'on compte les essais).

## Écrire le C++

- Commence par `#include "globaldefs.h"` ; inclus les en-têtes de `include/` des classes déjà
  connues que la fonction utilise (ex. `Memory/SafeAllocator.h`, `World/Object3D.h`) et appelle
  leurs méthodes normalement : les noms décorés doivent correspondre à ceux de l'assembleur.
- Fonction cible et fonctions non décompilées : `extern "C"` avec leur nom `func_...`
  (ex. `extern "C" void func_0205a198(void*);`). Devine les signatures d'après les registres.
- Données : **jamais** de chaîne littérale ni de tableau constant initialisé dans le code
  (le compilateur les mettrait dans notre objet et le module changerait de taille).
  Déclare-les `extern` sous leur nom `data_...` : `extern "C" char data_ov014_02189661[];`,
  ou `extern "C" const MaStruct data_ov014_02189498;` puis `MaStruct t = data_ov014_02189498;`
  pour une copie sur la pile.
- `this` arrive dans r0. Tant que la classe n'est pas établie, écris une fonction libre
  `extern "C" T func_ov014_xxx(MaClasse* self, ...)` : le code produit est le même.
- Types inconnus : définis des `struct` locales avec les champs aux bons décalages
  (`unk_3c`, ou un nom parlant quand l'usage est clair) et du rembourrage `u8 pad_xx[n]`.
  Taille d'une structure = pas d'un tableau (`mul`/`mla` par 0x14 → `sizeof == 0x14`).
- Pas d'appels virtuels déclarés (`virtual`) : le compilateur émettrait une vtable. Simule un
  appel indirect par un pointeur de fonction lu dans la structure.
- Types C de base disponibles : `int`, `short`, `char` (signé), `unsigned ...`. Définis toi-même
  `typedef unsigned char u8;` etc. si besoin.

## Faire correspondre (astuces mwcc)

- Ordre des comparaisons (`cmp r0, r1` vs `cmp r1, r0`) : inverser les opérandes, passer par une
  variable temporaire, changer signé/non signé.
- Registres différents mais même logique : changer l'ordre de déclaration ou d'utilisation des
  variables, introduire ou supprimer une variable locale, `for` ↔ `while`, pré/post-incrément.
- `and r0, r0, #0xff` / `lsl #0x10; lsr #0x10` : variable `u8` / `u16`. `ldrsh` : `short`.
- `mla`/`mul` par une constante : indexation d'un tableau de structures de cette taille.
- Branches conditionnelles inversées : inverser `if/else`, retour anticipé, `!=` vs `==`.
- `-O2 -inline noauto` : seules les fonctions marquées `inline` sont intégrées ; une petite
  fonction appelée ailleurs peut avoir été `inline` dans la source.
- Constantes de pool (`.word 0x...`) : grandes constantes, adresses de données.
- Un switch dense donne une table de saut `addls pc, pc, r0, lsl #2`.

Appris sur l'overlay 14 (voir `tools/decomp_ia/pilote_ov014/notes_ov014.md`) :

- Registres non volatils : mwcc les attribue dans l'ordre **inverse** de la déclaration ;
  déclarer en tête de bloc, dans un autre ordre, les variables dont les registres sont croisés.
  Sur la pile, la première locale déclarée prend l'adresse la plus haute.
- Un champ relu après un appel (`ldr` répété) : passer l'objet par une locale
  (`BackgroundLoader* l = BackgroundLoader::GetInstance();`) ; à l'inverse, une valeur gardée en
  registre se relit en réécrivant l'expression (`&self->sprites[i]`).
- `a = b = f();` (affectation chaînée) donne le `mov r1, r0` que deux affectations ne donnent pas.
- Drapeaux sur un octet : `tst`/`orr`/`bic` viennent de masques sur l'octet brut (`flags |= 8`),
  les `lsl`/`lsr` d'un champ de bits ; une union octet/bitfield permet les deux.
- `-inline noauto` : seules les `inline` triviales (une expression sans `if`, sans locale)
  sont intégrées ; une `inline` plus grosse devient un vrai appel.
- Un `.word` vers une donnée en deux exemplaires dans le pool alors que le nôtre n'en a qu'un :
  une table `static` locale. Garde la donnée `extern` et déclare-la une deuxième fois dans un
  `namespace` pour l'écriture d'initialisation (voir `BestiaryScreen::Update`).
- `char buf[0x40] = {0};` produit l'appel à `__clear`.

## Partage avec les autres agents

D'autres agents décompilent en parallèle les fonctions voisines du même module.
Lis `build/ia/notes_<module>.md` s'il existe (découvertes des autres : structures, rôles).
À la fin, ajoute-y 1 à 5 lignes utiles (structure et décalages établis, rôle de la fonction)
avec `cat >> build/ia/notes_<module>.md <<'EOF'` … `EOF` (ajout seulement, jamais d'écrasement).

## Rendu

1. Laisse la meilleure version dans `build/ia/<FONCTION>/<FONCTION>.cpp` (relance `essai` dessus
   en dernier pour que le journal ait le score final).
2. Écris `build/ia/<FONCTION>/resultat.json` :
   ```json
   {"fonction": "...", "score": 100.0, "essais": 7,
    "nom_propose": "Bestiary::LoadMonsterList", "role": "une phrase",
    "structures": "ce que tu as établi sur les types", "blocage": "si < 100 % : pourquoi"}
   ```
3. Réponds en une ligne : `FONCTION score essais nom_propose`.
