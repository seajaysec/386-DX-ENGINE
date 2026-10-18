#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Build the example (examples/sing.c) against the engine with the computer's own compiler.
eSpeak NG 1.52.0 must be in vendor/espeak-ng (see README.md).

    tools/build.py        -> build/sing"""
import os, subprocess, sys
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
root = Path(__file__).resolve().parents[1]
os.chdir(root)
sys.path.insert(0, str(root / "tools"))
from prepare_espeak import prepare
speech = prepare(root)                   # eSpeak NG, patched and its English data compiled, in build/espeak
out = root / "build/obj"
out.mkdir(parents=True, exist_ok=True)
cc = os.environ.get("CC", "cc")
base = [cc, "-O2", "-ffp-contract=off", "-DDX_HOST=1", "-Ivendor/espeak-ng/src/include",
        "-Ivendor/espeak-ng/src/ucd-tools/src/include", "-Isrc", "-Ibuild/espeak/src", "-Wno-everything"]
units = list(speech) + [Path("src") / u for u in ("speech.c", "speech_runtime.c", "singing.c")]
def compile(f):
    f = Path(f).relative_to(root) if Path(f).is_absolute() else Path(f)
    obj = out / (str(f).replace("/", "_") + ".o")
    extra = ["-include", "src/espeak_port.h"] if str(f).startswith(("vendor/", "build/espeak/")) else []
    subprocess.run([*base, *extra, "-c", str(f), "-o", str(obj)], check=True)
    return obj
with ThreadPoolExecutor(8) as pool:
    objects = list(pool.map(compile, units))
subprocess.run([*base, "examples/sing.c", *map(str, objects), "-lm", "-o", "build/sing"], check=True)
print("build/sing")
