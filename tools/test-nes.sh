#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
[[ $# == 1 ]] || { echo 'Usage: bash tools/test-nes.sh /path/to/FF1.NES'; exit 2; }
rom=$(realpath "$1")
bash "$root/tools/setup-fceumm.sh"
python3 - "$root/vendor/fceumm" <<'PY'
from pathlib import Path
import sys
p=Path(sys.argv[1])
for f in p.rglob('*.o'):f.unlink()
(p/'fceumm-host.a').unlink(missing_ok=True)
PY
make -C "$root/vendor/fceumm" -f Makefile.libretro -j4 platform=unix STATIC_LINKING=1 \
    TARGET=fceumm-host.a HAVE_HDPACK=0 HAVE_NTSC=0 CC=cc AR=ar >/dev/null
mkdir -p "$root/build/tests"
g++ -std=c++20 -O1 -g \
    -I"$root/vendor/fceumm/src/drivers/libretro/libretro-common/include" \
    "$root/tests/test_nes.cpp" "$root/src/nes/nes_core.cpp" "$root/src/nes/libretro_glue.cpp" \
    "$root/src/randomizer.cpp" "$root/src/nes/run_records.cpp" "$root/tests/audio_stub.cpp" "$root/src/heap_compat.cpp" "$root/src/reserved_heap.cpp" \
    -Wl,--wrap=free,--wrap=realloc "$root/vendor/fceumm/fceumm-host.a" -lm -o "$root/build/tests/test_nes"
ASAN_OPTIONS=detect_leaks=0 "$root/build/tests/test_nes" "$rom"

g++ -std=c++20 -O1 -g -fsanitize=address,undefined \
    -I"$root/vendor/fceumm/src/drivers/libretro/libretro-common/include" \
    "$root/tests/test_libretro_glue.cpp" "$root/src/nes/libretro_glue.cpp" "$root/tests/audio_stub.cpp" \
    -o "$root/build/tests/test_glue"
ASAN_OPTIONS=detect_leaks=0 "$root/build/tests/test_glue"


