# Décompilation « matching » par agents IA : rapport du pilote (overlay 14, bestiaire)

15 octobre 2026, version USA.

## En bref

- **66 fonctions sur 68** de l'overlay 14 (le bestiaire) sont décompilées et donnent les mêmes
  octets que le jeu. Elles sont réparties dans **4 fichiers C++ marqués `complete`** et
  organisées en vraies classes nommées (`BestiaryViewer`, `BestiaryScreen`, `PackedTable`,
  `PackedTableLoader`, `ScreenLayout`).
- Progression USA (build/usa/report.json) :

  | | Avant | Après | Écart |
  |---|---|---|---|
  | Code | 10,625 % (314 580 o) | 11,187 % (331 212 o) | +16 632 o |
  | Fonctions | 2 100 (14,20 %) | 2 166 (14,65 %) | +66 |

  `check modules` et `check symbols` passent.
- **Temps réel : environ 30 minutes** du premier agent au dernier commit, avec jusqu'à 12 agents
  en parallèle. Les 64 fonctions qui ont abouti du premier coup ont pris **10 minutes**. Il faut
  compter en plus environ 1 h pour écrire l'outillage.
- **Temps agent cumulé : 2,3 h**, dont 2,06 h pour les fonctions et 0,22 h pour l'intégration, soit
  **environ 29 fonctions par heure d'agent**.
- **Échecs** : deux longues fonctions restent proches du but (99,74 % pour 0xc28 octets, 98,32 %
  pour 0x4a8 octets) après deux passes. Elles sont conservées dans `pilote_ov014/non_matching/`.

## Méthode

### 1. Outillage (`tools/decomp_ia/`, voir README.md)

- `decomp.py info` : à partir d'un nom ou d'une adresse, affiche l'assembleur cible (objdiff, avec
  les relocations nommées). Il donne aussi :
  - les fonctions appelées, avec leur nom C++, leur état (décompilée ou non) et l'en-tête de
    leur classe ;
  - les données lues, avec leur contenu, les chaînes et les pointeurs relogés ;
  - les appelants et les fonctions voisines.
- `decomp.py essai` : compile une proposition avec **exactement** la commande mwcc de
  `build.ninja` (`-O2`, mwcc 2.0/sp2p2, mêmes include). La sortie va dans `build/ia/<fonction>/`
  (bac à sable, le build du projet n'est pas touché). L'outil compare ensuite avec objdiff-cli,
  puis affiche le score et les seules lignes qui diffèrent. Chaque essai est journalisé.
- `decomp.py verifier` : compare toutes les fonctions d'un ou plusieurs fichiers, dans l'ordre,
  avec celles du jeu. Il accepte les appels entre fonctions renommées.
- `integrer.py` : renomme les fonctions dans symbols.txt et ajoute le fichier à delinks.txt
  (`complete`).
- `mesures.py` : produit les tableaux ci-dessous.

Deux pièges de l'outil, corrigés pendant le pilote :

- objdiff compte comme différente une relocation vers un symbole **défini** dans l'objet cible
  (donnée ou fonction de l'overlay) quand la proposition le déclare `extern`. Ce faux écart
  plafonnait des fonctions identiques à 90–98 %. Les six agents de calibration l'ont repéré :
  l'outil l'ignore désormais.
- Les opérateurs implicites (`Vector3i::operator=`) sont émis en copie « multidef » dans chaque
  objet. L'éditeur de liens n'en garde qu'une ; `verifier` les ignore.

### 2. Boucle d'agents

Chaque fonction est confiée à **un sous-agent** (Claude, même modèle que l'orchestrateur), qui
suit `CONSIGNES_AGENT.md` :

- il lance `info` ;
- il écrit une fonction libre `extern "C"` avec des `struct` locales ;
- il enchaîne `essai` jusqu'à 100 %, avec **40 essais au plus** ;
- il rend un `resultat.json` (nom proposé, rôle, structures) ;
- il ajoute ses découvertes au fichier partagé `notes_ov014.md`. Les décalages et les astuces de
  matching profitent ainsi aux agents suivants.

Ordre de passage : une vague de calibration de 6 fonctions de tailles variées, puis les
62 autres de la plus courte à la plus longue, avec 12 agents en parallèle au plus. Les deux
échecs ont eu une **deuxième passe** de 25 essais, avec un nouvel agent et des pistes nouvelles.
Une seule fonction a reçu une aide de l'orchestrateur : pour `02186bc8`, je lui ai signalé la
forme trouvée par un autre agent pour les tables statiques.

### 3. Intégration

Trois agents d'intégration (`CONSIGNES_INTEGRATION.md`) ont fusionné les propositions en
fichiers sources :

- une classe par objet, des champs nommés d'après leur usage et des méthodes ;
- l'ordre des adresses est conservé ;
- chaque fichier est vérifié avec `verifier`.

Les quatre fichiers sont repassés à 100 % du premier coup après le passage en méthodes.

| Fichier | Plage .text | Fonctions |
|---|---|---|
| src/Bestiary/BestiaryViewer.cpp | 0x021842a0–0x02185c90 | 17 |
| src/Bestiary/BestiaryScreen.cpp | 0x021868b8–0x02188b18 | 32 |
| src/Bestiary/PackedTableLoader.cpp | 0x02188b18–0x02188d48 | 7 |
| src/Bestiary/PackedTable.cpp | 0x021891f0–0x02189468 | 10 |

Les deux fonctions manquantes coupent la plage en quatre fichiers au lieu de deux. Les données
(.data, .rodata, chaînes) restent dans l'assembleur et sont référencées en `extern`.

## Mesures

Données brutes dans `pilote_ov014/` :

- `journal.jsonl` : un enregistrement par essai ;
- `agents.jsonl` : jetons, durée et nombre d'outils par agent ;
- `mesures.csv` : une ligne par fonction.

| Tranche (octets) | Fonctions | Réussies (100 %) | Taux | Essais moyens | Durée moyenne | Jetons moyens* |
|---|---|---|---|---|---|---|
| ≤ 0x40 | 19 | 19 | 100 % | 3,4 | 0,9 min | 59 k |
| 0x41–0x100 | 29 | 29 | 100 % | 2,8 | 0,7 min | 60 k |
| 0x101–0x200 | 11 | 11 | 100 % | 4,5 | 1,5 min | 72 k |
| 0x201–0x400 | 4 | 4 | 100 % | 3,2 | 1,4 min | 80 k |
| > 0x400 | 5 | 3 | 60 % | 27,0 | 12,8 min | 220 k |
| **Total** | **68** | **66** | **97 %** | 5,1 | 1,8 min | 75 k |

\* Le chiffre est celui que l'outil d'agents remonte : la taille du contexte de l'agent à la fin,
dont environ 45 k de consignes système communes. Ce n'est pas la facture. Chaque appel d'outil
renvoie tout le contexte au modèle, en grande partie depuis le cache, donc les jetons traités
réellement valent plusieurs fois ce chiffre. Total mesuré : 5,08 M pour les fonctions et 0,32 M
pour l'intégration.

Autres constats :

- **33 fonctions sur 66 ont abouti au premier essai**, y compris des fonctions de 0x2c4 et
  0x740 octets. La médiane est de 1,5 essai.
- La difficulté ne suit pas la taille, mais les **calculs arithmétiques serrés** où
  l'allocation de registres hésite :
  - `0218934c` (0x34 octets) a demandé 24 essais ;
  - `02188ba0` (0x30 octets) en a demandé 15 ;
  - `02184dcc` (0x70 octets) en a demandé 14.
- **Débit**, au parallélisme de 12 agents :
  - environ 130 fonctions (33 Ko) par heure de temps réel, intégration comprise ;
  - environ 29 fonctions par heure d'agent.
- Le décompte des essais est un minimum : deux agents ont aussi compilé des variantes à la main,
  hors `essai`, pour sonder le compilateur.
- Le temps d'agent de la 2ᵉ passe de `02185c90` exclut une coupure de 53 minutes sans aucune
  activité (attente côté service).

## Extrapolation au reste du jeu

Il reste **12 606 fonctions (2,62 Mo de code)** non décompilées, toutes régions confondues :

| Tranche | Fonctions | Octets | Minutes d'agent / fonction (pilote) | Heures d'agent |
|---|---|---|---|---|
| ≤ 0x40 | 5 387 | 172 k | 0,9 | 81 |
| 0x41–0x100 | 4 700 | 643 k | 0,7 | 55 |
| 0x101–0x200 | 1 411 | 498 k | 1,5 | 35 |
| 0x201–0x400 | 692 | 489 k | 1,4 | 16 |
| > 0x400 | 416 | 821 k | 12,8 | 89 |
| **Total** | **12 606** | **2,62 M** | | **≈ 276** |

À ces 276 h s'ajoutent environ 42 h d'intégration (12 s d'agent par fonction au pilote).

- **Hypothèse haute** (taux du pilote) : environ 320 heures d'agent, soit **environ 27 heures de
  temps réel** à 12 agents en parallèle. Environ 98 % des fonctions aboutiraient, et les échecs
  se concentreraient dans les fonctions de plus de 0x400 octets.
- **Hypothèse prudente** (que je retiens) : le bestiaire est un module facile. C'est du code
  d'interface autonome, sans table virtuelle, et ses appels vont surtout vers des fonctions de
  main déjà nommées ou faciles à typer. Le cœur du jeu (combat, champ, scripts, rendu) aura plus
  de C++ objet et de types partagés inconnus. Je compte donc :
  - un temps **×2 à ×3**, soit **60 à 80 heures de temps réel** ;
  - **85 à 90 % de réussite**.

  Il resterait 1 000 à 2 000 fonctions, surtout longues, à reprendre en deuxième passe, à la main
  ou avec un « permuter ».
- En jetons, l'indicateur mesuré donnerait environ 0,9 G sur le jeu entier. La consommation
  réelle facturée est plusieurs fois plus élevée, en grande partie servie par le cache.

## Ce qui bloque, et les remèdes

1. **Les longues fonctions.** Les deux échecs sont des écarts d'allocation de registres ou
   d'ordonnancement dans des fonctions de 0x4a8 et 0xc28 octets : 4 lignes d'un côté, 1 seule de
   l'autre. Chaque essai change tout le code, et plus de 60 essais n'ont pas suffi. Pistes :
   - un « permuter » automatique, qui essaie des milliers de variantes ;
   - une passe humaine sur decomp.me à partir des propositions de `non_matching/`.
2. **Les types inconnus.** Chaque agent redéfinit ses `struct` locales, et c'est l'intégration
   qui unifie les types. Elle marche bien à l'échelle d'un module : 3 agents, une vingtaine de
   minutes. Pour un type partagé par plusieurs modules (objets du moteur), il faudra intégrer
   d'abord les classes de main, puis travailler module par module en lisant ces en-têtes.
3. **Les données partagées.** Les fichiers ne couvrent que le `.text`. Chaînes, tables et
   pointeurs de fonctions membres restent en `extern`. Le compilateur range les chaînes d'une
   unité de traduction dans un seul « pool » `.data` : on pourrait le reconstituer, mais il
   faudrait décompiler tout le `.data` de l'overlay d'un coup.

   Deux tables `static` locales de pointeurs de fonctions membres ont dû être émulées :
   - la garde est écrite à la main ;
   - une déclaration en `namespace` force le doublon du pool, voir `BestiaryScreen::Update`.

   C'est exact au bit près, mais ce n'est probablement pas la source d'origine. À reprendre
   quand les données de l'overlay seront décompilées.
4. **Les renommages en cascade.** Un fichier qui appelle des fonctions renommées dans un autre
   ne peut être vérifié qu'avec lui (`verifier` accepte plusieurs fichiers), et l'intégration
   doit se faire dans l'ordre avec un `ninja` entre deux. C'est automatisé dans `integrer.py`.
5. **L'orchestration manuelle.** J'ai lancé les agents un par un et relevé leurs mesures à la
   main. Pour passer à l'échelle du jeu, il faudra un script d'orchestration : file d'attente,
   relance des échecs, relevé automatique des jetons.
6. **La version japonaise.** Les fichiers ne sont branchés que pour la version USA. Le build JPN
   n'est pas modifié. Le porter demandera les adresses JPN (`#ifdef jpn`, comme dans
   `LootableContainer.cpp`).

## Prochaines étapes proposées

1. **Prochains modules** :
   - overlays autonomes de la même famille : 6 (marmite d'alchimie, 132 fonctions), 8
     (palmarès, 63), 16 et 11 (beaucoup de petites fonctions) ;
   - puis les feuilles de main, en commençant par les classes déjà en partie connues.
2. **Script d'orchestration** : lancer un module entier sans intervention et produire les
   mesures automatiquement.
3. **Les deux fonctions du bestiaire** : `02185c90` et `02188d48`, avec un permuter ou une
   relecture humaine. Elles fermeraient l'overlay en deux fichiers.
4. **Le `.data` de l'overlay 14** (chaînes, tables d'états), pour des unités de traduction
   complètes et des tables `static` locales réelles.
5. **Le port JPN** des quatre fichiers.
