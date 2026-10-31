#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.." && pwd)
cd "$root"
export PATH="$root/.deps/host/root/usr/lib/llvm-18/bin:$PATH"
export LD_LIBRARY_PATH="$root/.deps/host/root/usr/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH:-}"
export PS5_PAYLOAD_SDK="$root/.deps/native/ps5-payload-sdk"
mkdir -p build/zelda/obj dist/PPSA01988
objects=()
while IFS= read -r source; do
 obj="build/zelda/obj/${source//\//_}.o"
 sh tooling/prospero-clang18 -std=c++20 -O2 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -Ivendor/fceumm/src/drivers/libretro/libretro-common/include -c "$source" -o "$obj"
 objects+=("$obj")
done < <(find ports/zelda/src -name '*.cpp' -type f | sort)
.deps/native/ps5-payload-sdk/bin/prospero-lld -T tooling/native/ps5-pie.ld --eh-frame-hdr --wrap=free --wrap=realloc --version-script tooling/native/app-symbols.map -e _start -o build/zelda/llvm-pie.elf build/obj/app_crt.o build/obj/app_cpp_runtime.o "${objects[@]}" vendor/fceumm/fceumm-ps5.a --as-needed .deps/native/ps5-payload-sdk/target/lib/*.so
build/host/ps5-native-tool link --in build/zelda/llvm-pie.elf --out build/zelda/eboot.elf --stub-dir .deps/native/ps5-payload-sdk/target/lib --module-sdk 0x02000009 --companion-sdk 0x08050001 --file-name eboot.elf
python3 tests/test_module_mapping.py build/zelda/llvm-pie.elf build/zelda/eboot.elf
build/host/ps5-native-tool self --sign --in build/zelda/eboot.elf --out dist/PPSA01988/eboot.bin --magic 0x1D3D154F
build/host/ps5-native-tool self --inspect --file dist/PPSA01988/eboot.bin
