#!/usr/bin/env python3
"""Private complete Test15 package, reusing verified Test14 assets/runtime.
Usage: python3 tools/package-ff1-options.py /path/to/FF1-Complete-Test14.zip
Never commit the output, ROM, music or runtime module to Git.
"""
from pathlib import Path
import zipfile,sys,hashlib,zlib
root=Path(__file__).resolve().parents[1]
eboot=(root/'dist/PPSA01987/eboot.bin').read_bytes()
assert eboot[:4]==bytes.fromhex('4f153d1d'), 'Expected signed SELF, never raw ELF'
readme=(root/'docs/FF1-Test15.md').read_bytes()
out=root.parent/'FF1-Complete-Test15.zip'
with zipfile.ZipFile(sys.argv[1]) as old:
 assert old.testzip() is None
 libc=old.read('PPSA01987/sce_module/libc.prx')
 assert hashlib.sha256(libc).hexdigest()=='8a29784545983fffd7428446912711ec4e269672673e2d281612d3dbf4c39e88'
 rom=old.read('PPSA01987/rom/FF1.NES')
 assert len(rom)==262160 and zlib.crc32(rom[16:])==0xcebd2a31
 with zipfile.ZipFile(out,'w',zipfile.ZIP_DEFLATED,6) as z:
  for n in old.namelist():
   if n.startswith('PPSA01987/') and not n.endswith('/') and n not in ['PPSA01987/eboot.bin','PPSA01987/README.txt']:
    z.writestr(n,old.read(n))
  z.writestr('PPSA01987/eboot.bin',eboot)
  z.writestr('PPSA01987/README.txt',readme)
  z.writestr('START-HERE.txt',readme)
with zipfile.ZipFile(out) as z:
 assert z.testzip() is None
 assert z.read('PPSA01987/eboot.bin')==eboot
 assert z.read('PPSA01987/sce_module/libc.prx')==libc
print(out, out.stat().st_size, 'bytes')
print('eboot SHA256',hashlib.sha256(eboot).hexdigest())
print('ZIP SHA256',hashlib.sha256(out.read_bytes()).hexdigest())
