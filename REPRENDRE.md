# Reprendre la décompilation de ce fork

Ce dépôt est le fork d'Alexandre du projet [DQIX/dqix-decomp](https://github.com/DQIX/dqix-decomp). Il a avancé de son côté avec des agents IA. Cette page explique où on en est, comment construire le jeu et comment continuer.

## Où on en est (version USA, 16/10/2026)

| | Fonctions | Code |
|---|---|---|
| Au départ du fork (15/10) | 1 111 sur 14 788 (7,51 %) | 4,86 % |
| Aujourd'hui (`main`) | 2 298 sur 14 788 (15,54 %) | 12,92 % |

Pour comparaison, le projet d'origine en est à 8,02 % des fonctions. Les badges en haut du README sont les siens.

Ce qui a été fait :

1. [PR #1](https://github.com/Alexandre78500/dqix-decomp/pull/1) et [PR #2](https://github.com/Alexandre78500/dqix-decomp/pull/2) : DQIX utilise les mêmes bibliothèques Nintendo que Pokémon HeartGold (NitroSDK 4.2, NitroSystem, NitroWiFi, libCPS, MSL_C). 216 fichiers sources de la décompilation [pokeheartgold](https://github.com/pret/pokeheartgold) compilent aux octets exacts de DQIX. Ils sont dans `libs/`.
2. [PR #3](https://github.com/Alexandre78500/dqix-decomp/pull/3) : pilote de décompilation du code du jeu par agents IA. L'overlay 14 (bestiaire) est fait à 66 fonctions sur 68.
3. [PR #4](https://github.com/Alexandre78500/dqix-decomp/pull/4) : l'overlay 6 (marmite d'alchimie) est entièrement fait, 132 fonctions sur 132.

## Ce qu'il faut

- **Une machine Linux x86_64 ou Windows.** Le compilateur DS (Metrowerks `mwccarm`) est un programme Windows. Sous Linux, il tourne avec `wibo`, que le build télécharge seul. Un Mac Apple Silicon ne convient pas.
- Python 3.11 ou plus récent, `ninja` (`pip install ninja` suffit) et `gcc`.
- **Ta propre ROM USA de Dragon Quest IX.** Son SHA-1 doit être `c7c3014c237900c8281289b8bc76a781969b6278`. Place-la dans `extract/baserom_dqix_usa.nds`. Elle est ignorée par git : ne la commite jamais, ne la partage pas.

## Construire et vérifier

```bash
python -m venv .venv && . .venv/bin/activate
pip install -r tools/requirements.txt ninja
python tools/configure.py usa
ninja rom check report
```

- `check modules` vérifie que chaque module est identique octet par octet à la cartouche.
- `check symbols` vérifie que chaque fonction est à la bonne adresse.
- Les deux doivent passer avant toute PR.
- `build/usa/report.json` donne la progression (`matched_functions_percent`, `matched_code_percent`).
- La cible `min` citée dans le README n'existe plus.
- Le hash complet de la ROM (`ninja`) demande le BIOS ARM7 de la DS. Sans lui, la ROM reconstruite diffère de 4 octets d'en-tête, et c'est normal.

## La méthode IA

Tout est dans [`tools/decomp_ia/`](tools/decomp_ia/README.md). Commence par son `README.md`, puis lis `CONSIGNES_AGENT.md` (les consignes à donner à chaque agent, avec les astuces de correspondance trouvées), `CONSIGNES_INTEGRATION.md` et `RAPPORT_PILOTE_OV014.md` (les mesures du pilote).

Le principe : un sous-agent par fonction ou par petit lot de fonctions voisines, une douzaine en parallèle.

1. `decomp.py info <fonction>` : l'assembleur et le contexte (fonctions appelées, données, types connus).
2. `decomp.py essai` : compile une proposition C++ dans un bac à sable, avec les options exactes du projet, et donne le score objdiff. L'agent corrige jusqu'à 100 %.
3. `lots.py` regroupe les petites fonctions voisines en lots.
4. `rescorer.py` recompile la proposition finale de chaque fonction. Ne crois pas un agent qui annonce 100 % : certains se trompaient.
5. `integrer.py` écrit les fichiers `.cpp` complets et met à jour `symbols.txt` et `delinks.txt`.

Rythme mesuré sur le bestiaire : environ 29 fonctions par heure d'agent, 100 % de réussite jusqu'à 0x400 octets, 3 sur 5 au-delà.

## Quoi attaquer

Fonctions restantes par module (USA), des plus petits modules aux plus gros. Finir des modules entiers donne des fichiers propres et une progression visible.

| Module | Fonctions | Reste | Remarque |
|---|---|---|---|
| ov030 | 11 | 11 | travail commencé, pas encore publié (voir plus bas) |
| ov034 | 5 | 5 | idem |
| ov020 | 18 | 18 | |
| ov026 | 24 | 24 | |
| ov028 | 34 | 34 | |
| ov013 | 40 | 40 | |
| ov009 | 43 | 43 | |
| ov008 | 63 | 63 | registre des combats |
| ov012 | 71 | 71 | |
| ov016 | 87 | 87 | |
| ov005 | 89 | 89 | |
| ov015 | 103 | 103 | |
| ov011 | 186 | 186 | |
| ov002 | 213 | 213 | |
| ov025 | 304 | 304 | |
| ov003 | 510 | 510 | |
| ov001 | 515 | 515 | |
| ov004 | 532 | 532 | |
| ov024 | 573 | 573 | |
| ov000 | 826 | 818 | |
| ov023 | 827 | 827 | |
| ov017 | 1 194 | 1 192 | |
| ov031 | 1 961 | 1 499 | Wi-Fi Nintendo, déjà 462 fonctions de bibliothèque |
| main | 6 206 | 4 645 | le programme principal |

Petits restes : ov010, ov019, ov021 (3 à 5 fonctions), deux fonctions du bestiaire bloquées à 99,74 % et 98,32 % (`ov014` `02185c90` et `02188d48`), une fonction dans ov027 et ov033.

## Points ouverts

- **Resynchroniser avec le projet d'origine.** Il a 28 commits de plus depuis le fork. Il a renommé `src/System` en `src/NitroSDK`, compile désormais ses fichiers SDK en `-O4,p` et décompile lui-même les fonctions SDK de l'écran tactile. La fusion donne 10 conflits, surtout dans `symbols.txt`, où ce fork a donné les noms Nintendo. À faire dans une branche dédiée, build vert à la clé.
- **Le travail coupé de la campagne 2 (overlays 30 et 34).** Il est resté sur le PC d'Alexandre, dans un commit non vérifié. Il sera poussé en branche `wip/` dès que le PC sera allumé.
- **Les bibliothèques restantes**, voir [`tools/heartgold_import/README.md`](tools/heartgold_import/README.md). Une cinquantaine de fichiers diffèrent de HeartGold, car DQIX a un SDK un peu plus récent. 75 chevauchent du code déjà décompilé.
- **La version japonaise** n'a pas pu être vérifiée faute de ROM JPN. `configure.py jpn` tourne, et les fichiers de `libs/` sont réservés à la version USA.

## Règles de ce fork

- Une branche par lot, et une PR par module ou petit groupe de modules, ouverte sur **Alexandre78500/dqix-decomp** avec la progression avant et après.
- Build propre vert (`check modules` et `check symbols`) avant la PR.
- Des noms de classes, de fonctions et de champs qui disent ce que fait le code. Des fichiers `.cpp` cohérents, marqués `complete` dans `delinks.txt` seulement quand tout leur `.text` correspond.
- Jamais de ROM, de BIOS ni de fichier tiré de la cartouche dans le dépôt.
- Pas de ligne de co-auteur IA dans les commits ni les PR.
- Rien vers le projet d'origine (PR, Discord) sans en parler à Alexandre.
