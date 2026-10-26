#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
core="$root/vendor/fceumm"
revision=ac0d00a7fc01166f84c7bac01d1084ebfd2b865e
if [[ ! -f "$core/.ff1-revision" ]]; then
    [[ ! -e "$core" ]] || { echo 'Unmanaged vendor/fceumm exists; move it aside first.' >&2; exit 1; }
    mkdir -p "$core"
    git -C "$core" init -q
    git -C "$core" remote add origin https://github.com/libretro/libretro-fceumm.git
    git -C "$core" fetch --depth 1 origin "$revision"
    git -C "$core" checkout --detach FETCH_HEAD
    python3 - "$core" <<'PY'
from pathlib import Path
import sys
root=Path(sys.argv[1])
p=root/'Makefile.common';s=p.read_text();s=s.replace('ifneq ($(STATIC_LINKING), 1)','ifeq (1, 1)').replace('ifneq ($(STATIC_LINKING),1)','ifeq (1, 1)');p.write_text(s)
p=root/'src/drivers/libretro/libretro.c'
p.write_text(p.read_text()+'\n/* FF1 tracker: read only the mapped inventory region. */\nunsigned char ff1_read_cpu(unsigned address) { if(address < 0x6000 || address > 0x6034) return 0; return ARead[address](address); }\n')
PY
    printf '%s\n' "$revision" > "$core/.ff1-revision"
fi
[[ $(cat "$core/.ff1-revision") == "$revision" ]] || { echo 'Unexpected core revision' >&2; exit 1; }
