# AI-assisted matching decompilation

Scripts to decompile game functions with AI agents, one function per agent, then merge the
results into real source files. Comments, messages and agent instructions are in French.
`decomp.py` and `integrer.py` work for any human too: they are a local, scriptable
decomp.me.

Everything is sandboxed in `build/ia/` (ignored by git): the project build is never touched
until `integrer.py` writes `symbols.txt` and `delinks.txt`. Run `ninja` once first, so that
`build/usa/delinks` holds the target objects.

## `decomp.py`

```
decomp.py info FONCTION              target assembly + context
decomp.py squelette FONCTION -o f    starting .cpp with extern declarations
decomp.py essai FONCTION f.cpp       compile f.cpp, score it against the game
decomp.py verifier f.cpp ADDR [g.cpp ADDR ...]
decomp.py journal                    tries logged so far
```

`FONCTION` is a `symbols.txt` name or an address (`ov014:0x021842a0`).

- `info` prints the target assembly (from objdiff, with relocations named), the functions it
  calls (decompiled or not, C++ name, header that declares the class), the data it reads
  (contents, strings, relocated pointers), its callers and its neighbours.
- `essai` compiles with the exact `mwcc` command of `build.ninja`, diffs with `objdiff-cli`
  and prints the score and the differing lines only. A relocation to a symbol that the target
  object defines (overlay data, a neighbour function) but that the proposal declares `extern`
  is not counted as a difference: the linked bytes are the same. Every try is appended to
  `build/ia/journal.jsonl`.
- `verifier` pairs the functions of each file, in order, with the game functions from `ADDR`
  on and diffs each pair. Calls between functions renamed in the given files are accepted, so
  several files can be checked together before `symbols.txt` is renamed. Multidef functions
  (implicit operators, out-of-line inlines) are skipped: the linker keeps one copy.

## `integrer.py`

`integrer.py src/X.cpp ov014:0x021842a0 [src/Y.cpp ADDR ...]` runs `verifier`, renames the functions of the range
in `symbols.txt` with the names of the compiled file, and adds the file to `delinks.txt`
(marked `complete`, `.text` range only). Then `python tools/configure.py usa && ninja`.
Files that call each other are integrated together, in one command: they are verified
together, then each one is renamed and added to `delinks.txt`.

After `ninja rom check`, a missing game function in `check symbols` usually means the linker
dropped it as dead code: its only callers are in other overlays and go through a relocation that
dsd could not resolve (`module:overlays(1,6)`). List it in `config/usa/arm9/force_active.txt`.

## `lots.py`

`lots.py ov006` groups the undecompiled functions of a module into lots for agents: neighbouring
small functions together (up to `--octets` bytes and `--max` functions), large ones alone.

## `rescorer.py`

`rescorer.py ov006 ov008` recompiles the final proposal of every function of the modules and
prints its real score. Agent reports are not reliable on their own (see the campaign 2 report):
only the files in `build/ia/` count.

## `mesures.py`

Aggregates `journal.jsonl`, `agents.jsonl` (one line per agent or lot of functions: tokens,
duration, written by the orchestrator from the agent usage report, split evenly over the lot) and each `resultat.json` into a table per size
range: success rate, tries, duration, tokens.

## Agent instructions

- `CONSIGNES_AGENT.md`: one agent, one function, up to 40 tries, sandbox only, writes its
  findings to the shared `build/ia/notes_<module>.md`.
- `CONSIGNES_INTEGRATION.md`: one agent turns the matched proposals of an address range into
  classes and source files, checked with `verifier`.
