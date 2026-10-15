# Nintendo libraries

C sources of the Nintendo libraries linked into the game, imported from the
[pokeheartgold](https://github.com/pret/pokeheartgold) decompilation, which uses the same
library versions (NitroSDK 4.2, NitroWiFi 2.1, libCPS, NitroSystem, MSL_C).
Only files that compile to the exact bytes of Dragon Quest IX (USA) are kept.

- `build_settings.json`: compiler version and flags of each file (copied from the
  pokeheartgold Makefile), include paths, the game versions the files are built for, and
  the symbols the linker must keep (see `tools/lcf_force_active.py`).
- `_headers/`: the headers these files need, in the include roots of pokeheartgold.

See `tools/heartgold_import/README.md` to re-run or extend the import.
