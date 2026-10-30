#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
[[ $# == 1 ]] || { echo 'Usage: bash tools/test-randomizer.sh /path/to/FF1.NES'; exit 2; }
bash "$root/tools/test-nes.sh" "$1"
g++ -std=c++20 -O1 -I"$root/vendor/fceumm/src/drivers/libretro/libretro-common/include" \
 "$root/tests/test_randomizer.cpp" "$root/src/qol.cpp" "$root/src/randomizer.cpp" "$root/src/ff1logic/options.cpp" "$root/src/nes/nes_core.cpp" "$root/src/nes/run_snapshot.cpp" \
 "$root/src/nes/libretro_glue.cpp" "$root/src/nes/run_records.cpp" "$root/tests/audio_stub.cpp" \
 "$root/src/heap_compat.cpp" "$root/src/reserved_heap.cpp" -Wl,--wrap=free,--wrap=realloc \
 "$root/vendor/fceumm/fceumm-host.a" -lm -o "$root/build/tests/test_randomizer"
"$root/build/tests/test_randomizer" "$(realpath "$1")"
