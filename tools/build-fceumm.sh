#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
bash "$root/tools/setup-fceumm.sh"
sdk="$root/.deps/native/ps5-payload-sdk"
# Force a target rebuild: never reuse Linux objects in the console archive.
python3 - "$root/vendor/fceumm" <<'PY'
from pathlib import Path
import sys
p=Path(sys.argv[1])
for f in p.rglob('*.o'): f.unlink()
(p/'fceumm-ps5.a').unlink(missing_ok=True)
PY
make -C "$root/vendor/fceumm" -f Makefile.libretro -j4 platform=unix \
    STATIC_LINKING=1 TARGET=fceumm-ps5.a HAVE_HDPACK=0 HAVE_NTSC=0 \
    CC="$sdk/bin/prospero-clang" AR="$sdk/bin/prospero-ar"
