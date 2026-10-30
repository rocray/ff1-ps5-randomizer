#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
c++ -std=c++20 -O1 -Ivendor/fceumm/src/drivers/libretro/libretro-common/include tests/test_run_snapshot.cpp src/randomizer.cpp src/ff1logic/options.cpp src/nes/nes_core.cpp src/nes/run_snapshot.cpp src/nes/libretro_glue.cpp src/nes/run_records.cpp src/qol.cpp tests/audio_stub.cpp src/heap_compat.cpp src/reserved_heap.cpp -Wl,--wrap=free,--wrap=realloc vendor/fceumm/fceumm-host.a -lm -o build/test_run_snapshot
build/test_run_snapshot "$1"
