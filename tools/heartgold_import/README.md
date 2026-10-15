# HeartGold library import

Dragon Quest IX and Pokémon HeartGold/SoulSilver ship the same Nintendo libraries
(NitroSDK 4.2, NitroSystem, NitroWiFi, libCPS, MSL_C). These scripts find the library
functions that are byte-identical in both games and import the matching C sources from
the [pokeheartgold](https://github.com/pret/pokeheartgold) decompilation into `libs/`.

Comments and messages in the scripts are in French.

## Steps

1. Generate `lib/include/nitro/fx/fx_const.h` in your pokeheartgold clone
   (`make -C tools/gen_fx_consts && tools/gen_fx_consts/gen_fx_consts lib/include/nitro/fx/fx_const.h`).
2. Compile every library file of pokeheartgold with the settings of its Makefile:
   `compiler_hg.py <pokeheartgold> tools/mwccarm ./wibo <obj>`
3. Compare each function with DQIX (relocated bytes are ignored on both sides):
   `comparer.py <obj> config/usa/arm9 resultats.json`, then `verifier.py resultats.json`.
4. Place every file whose sections are found in DQIX, verified byte for byte:
   `planifier.py <obj> resultats.json config/usa/arm9 plan.json`.
   mwccarm puts each function and each data object in its own section, and the original
   link (`-dead`) dropped the ones nothing referenced. A file is kept when the sections
   present in DQIX follow each other in file order with no gap but alignment; it is then
   compiled whole and our own `-dead` drops the same sections. Refusals and their reason
   are written next to the plan (`plan.refus.json`); `DIAG=<file> planifier.py ...` prints
   where each section of a file was placed.
5. Check the plan against the game without writing anything:
   `importer.py <pokeheartgold> plan.json <dqix-decomp> --essai`. It lists the files it
   drops and writes `../renommages_<worktree>.txt`: game functions that the library code
   calls under their Nintendo name. Rename them with
   `renommer_cpp.py <dqix-decomp> <list>` (C++ free functions become `extern "C"`;
   `noms_cpp.txt` holds the renames already applied), build, and re-run the plan.
6. Write `libs/`, `delinks.txt` entries and Nintendo symbol names:
   `importer.py <pokeheartgold> plan.json <dqix-decomp> [file to skip ...]`.
   It adds to a previous import (files already in `delinks.txt` are not planned again).
7. `python tools/configure.py usa && ninja rom check report`: `check modules` and
   `check symbols` must pass.

`importer.py` drops a file when it would break the build: name clashes, C++ callers
under a mangled name, calls into code already decompiled under another name, addresses
referenced inside the file that have no HeartGold symbol, or names dsd relies on
(`AutoloadCallback`).

Functions of a partly found file that are missing from DQIX are cut from the copied
source (`decoupe.py`) when they would break the link: they call a symbol DQIX does not
have, their name is already defined elsewhere, or the source keeps them with
`#pragma force_active on` (the MSL_C files of pokeheartgold).
