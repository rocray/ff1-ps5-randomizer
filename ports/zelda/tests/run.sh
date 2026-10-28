#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../../.."
c++ -std=c++20 -O1 -Ivendor/fceumm/src/drivers/libretro/libretro-common/include ports/zelda/tests/test_zelda.cpp ports/zelda/src/nes/nes_core.cpp ports/zelda/src/nes/libretro_glue.cpp ports/zelda/src/nes/run_records.cpp ports/zelda/src/qol.cpp ports/zelda/src/randomizer.cpp ports/zelda/src/heap_compat.cpp ports/zelda/src/reserved_heap.cpp -Wl,--wrap=free,--wrap=realloc vendor/fceumm/fceumm-host.a -lm -o build/zelda/test_zelda
build/zelda/test_zelda "$1"
