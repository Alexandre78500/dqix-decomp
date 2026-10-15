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
4. Keep the files found whole, with every section placed and verified byte for byte:
   `planifier.py <obj> resultats.json config/usa/arm9 plan.json`
5. Write `libs/`, `delinks.txt` entries and Nintendo symbol names:
   `importer.py <pokeheartgold> plan.json <dqix-decomp> [file to skip ...]`
6. `python tools/configure.py usa && ninja rom check report`: `check modules` and
   `check symbols` must pass.

`importer.py` drops a file when it would break the build: name clashes, C++ callers
under a mangled name, calls into code already decompiled under another name, or
addresses referenced inside the file that have no HeartGold symbol.
