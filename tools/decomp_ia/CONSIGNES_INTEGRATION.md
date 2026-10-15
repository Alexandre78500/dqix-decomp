# Consignes pour un agent d'intégration (un fichier source)

Les agents « une fonction » ont produit, dans `build/ia/<fonction>/<fonction>.cpp`, une version
qui correspond octet pour octet, écrite comme fonction libre `extern "C"` avec des `struct`
locales. Ton travail : en faire **un vrai fichier source** du projet, lisible, avec des classes et
des noms qui ont du sens, sans perdre la correspondance.

## Règles

- Tu écris seulement les fichiers qu'on t'indique dans `src/` et `include/`. Tu ne touches ni à
  `config/` (symbols.txt, delinks.txt), ni au build, ni à git : l'orchestrateur s'en charge.
- Commentaires et noms en anglais, comme le reste du dépôt ; style des fichiers existants
  (`src/World/LootableContainer.cpp`, `include/World/LootableContainer.h`) : accolades sur leur
  propre ligne pour les fonctions, 4 espaces, `#pragma once` dans les en-têtes.

## Étapes

1. Lis les propositions de ta plage d'adresses (toutes à 100 %, sauf indication contraire),
   `build/ia/notes_<module>.md` et les en-têtes déjà écrits qu'on t'indique.
2. Établis les types : une classe par objet (mêmes décalages vus par plusieurs fonctions = même
   classe). Champs nommés d'après leur usage ; `unk_XX` quand l'usage reste obscur, rembourrage
   `u8 pad_XX[n]` entre les champs connus. Vérifie la taille avec
   `static_assert`-like commentaires (`// size 0x194`) seulement si elle est établie.
3. Les fonctions deviennent des **méthodes** (`this` = r0) ou des fonctions libres C++ selon le
   cas ; garde l'**ordre des adresses** dans le fichier : l'éditeur de liens place les fonctions
   dans l'ordre du fichier.
4. Fonctions non décompilées et données : `extern "C"` sous leur nom `func_.../data_...`
   (jamais de chaîne littérale ni de table initialisée, sauf si on te dit d'inclure la
   section de données correspondante).
5. Vérifie après chaque étape :
   ```
   .venv/bin/python tools/decomp_ia/decomp.py verifier src/X.cpp ov014:0xDEBUT [autre.cpp ov014:0x... ]
   ```
   Chaque fonction du fichier est comparée, dans l'ordre, à celle du jeu à la même position.
   Les appels vers des fonctions renommées dans les fichiers passés en argument sont acceptés.
   Si une fonction tombe sous 100 % après passage en méthode, compare avec sa proposition
   d'origine (`essai`) et reviens à la forme qui correspond (ordre des déclarations, types).
6. Écris en tête de chaque fonction dont le rôle est sûr un commentaire d'une ligne si le nom ne
   suffit pas. Pas de commentaire sur l'historique du matching.

## Rendu

- Les fichiers demandés, `verifier` à 100 % pour toutes les fonctions.
- Réponds en quelques lignes : fichiers écrits, classes et noms choisis, fonctions restées
  sous 100 % s'il y en a, points incertains.
