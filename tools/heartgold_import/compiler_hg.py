#!/usr/bin/env python3
"""Compile les bibliothèques C de pokeheartgold (lib/) avec les réglages de son Makefile.

Usage : compiler_hg.py <racine pokeheartgold> <dossier mwccarm> <wibo> <sortie>
"""
import fnmatch, os, re, subprocess, sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

hg, mwroot, wibo, out = (Path(a).resolve() for a in sys.argv[1:5])

SDK_SUB = "os mi fx gx card cp ctrdg fs math pxi rtc snd mb".split()
LIB_SUB = "cw NitroSDK NitroSystem NitroDWC NitroWiFi libCPS libVCT MSL_C".split()
dirs = ["lib/src"] + [f"lib/{d}/src" for d in LIB_SUB] + [f"lib/NitroSDK/src/{d}" for d in SDK_SUB]
sources = sorted(p.relative_to(hg) for d in dirs for p in (hg / d).glob("*.c"))

BASE = {
    "MWCCVER": "2.0/sp2p2",
    "OPTFLAGS": "-O4,p",
    "EXCCFLAGS": "-Cpp_exceptions off",
    "IPA": "-ipa file",
}
# Règles du Makefile : d'abord les motifs (%), puis les cibles exactes, comme make.
rules = []
for mk in ("common.mk", "Makefile"):
    for line in (hg / mk).read_text().splitlines():
        m = re.match(r"^\$\(BUILD_DIR\)/(\S+?)\.o:\s*private\s+(\w+)\s*:=\s*(.*)$", line.strip())
        if m:
            rules.append((m.group(1).replace("%", "*"), m.group(2), m.group(3).strip()))

def settings(src: Path):
    s = dict(BASE)
    stem = str(src.with_suffix(""))
    for exact in (False, True):
        for pat, var, val in rules:
            is_exact = "*" not in pat
            if is_exact != exact or not fnmatch.fnmatch(stem, pat):
                continue
            if var == "MWCFLAGS" and "filter-out -ipa file" in val:
                s["IPA"] = ""
            elif var in s:
                s[var] = val
    return s

INC = "-i ./src -i ./include -i ./include/library -i ./files -I./lib/include -i ./lib/NitroSDK/include -i ./lib/NitroSDK/include-mw -i ./lib/NitroSDK/include-pokediamond"

def build(src: Path):
    s = settings(src)
    obj = out / src.with_suffix(".o")
    obj.parent.mkdir(parents=True, exist_ok=True)
    cmd = (f'{wibo} {mwroot}/{s["MWCCVER"]}/mwccarm.exe -DSDK_ARM9 -DSDK_CODE_ARM -DSDK_FINALROM {s["OPTFLAGS"]} '
           f'-sym on -enum int -lang c99 {s["EXCCFLAGS"]} -gccext,on -proc arm946e -msgstyle gcc -gccinc {INC} '
           f'{s["IPA"]} -interworking -inline on,noauto -char signed -c -o {obj} {src}')
    r = subprocess.run(cmd, shell=True, cwd=hg, capture_output=True, text=True, env={**os.environ, "MWCIncludes": "lib/include"})
    return src, s["MWCCVER"], r.returncode == 0 and obj.exists(), (r.stdout + r.stderr)[-300:]

with ThreadPoolExecutor(os.cpu_count()) as ex:
    res = list(ex.map(build, sources))
ok = [r for r in res if r[2]]
print(f"{len(ok)}/{len(res)} fichiers compilés")
for src, ver, good, log in res:
    if not good:
        print("ÉCHEC", src, ver, log.strip().splitlines()[-1:] if log.strip() else "")
