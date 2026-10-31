#!/usr/bin/env bash
# Requires py65 for execution of the actual 6502 patch routines.
set -euo pipefail
cd "$(dirname "$0")/.."
[[ $# == 1 ]] || { echo 'Usage: bash tools/test-ff1-options.sh /path/to/original-FF1.NES'; exit 2; }
mkdir -p build
c++ -std=c++20 -O2 tests/test_ff1_options.cpp src/randomizer.cpp src/ff1logic/options.cpp -o build/test_ff1_options
build/test_ff1_options "$1"
python3 tests/test_magic_shuffle.py "$1" build/ff1-options-test.nes build/ff1-magic-restricted.nes
python3 tests/test_ff1_option_patches.py build/ff1-options-test.nes
python3 tests/test_ff1_slot_prices.py build/ff1-options-test.nes "$1"
python3 tests/test_ff1_slot_prices.py build/ff1-slot-prices-off.nes "$1"
c++ -std=c++20 -O1 -Ivendor/fceumm/src/drivers/libretro/libretro-common/include tests/test_ff1_options_core.cpp src/randomizer.cpp src/ff1logic/options.cpp src/nes/nes_core.cpp src/nes/run_snapshot.cpp src/nes/libretro_glue.cpp src/nes/run_records.cpp src/qol.cpp tests/audio_stub.cpp src/heap_compat.cpp src/reserved_heap.cpp -Wl,--wrap=free,--wrap=realloc vendor/fceumm/fceumm-host.a -lm -o build/test_ff1_options_core
build/test_ff1_options_core "$1"
