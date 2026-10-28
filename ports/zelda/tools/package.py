#!/usr/bin/env python3
"""Package a private build. ROM/runtime inputs are never added to Git.
Run native build and tests/render_ui first; requires Pillow.
"""
from pathlib import Path
from PIL import Image
import struct,json,zipfile,hashlib,sys
root=Path(__file__).resolve().parents[3]
app=root/'dist/PPSA01988'
for sub in ['sce_sys','sce_module','rom']: (app/sub).mkdir(parents=True,exist_ok=True)
rom=Path(sys.argv[1]).read_bytes()
assert len(rom)==131088 and hashlib.sha256(rom).hexdigest()=='89232edf4f9b52e3cb872094bc78973de080befca2ddea893b6e936066514d4e'
(app/'rom/ZELDA.NES').write_bytes(rom)
with zipfile.ZipFile(sys.argv[2]) as z:
 libc=z.read('PPSA01987/sce_module/libc.prx')
 assert hashlib.sha256(libc).hexdigest()=='8a29784545983fffd7428446912711ec4e269672673e2d281612d3dbf4c39e88'
 (app/'sce_module/libc.prx').write_bytes(libc)
p=json.loads((root/'sce_sys/param.json').read_text())
p['titleId']='PPSA01988';p['conceptId']='01988';p['contentId']='UP9000-PPSA01988_00-Z1RANDOMIZER0000'
p['localizedParameters']['en-US']['titleName']='The Legend of Zelda Randomizer'
(app/'sce_sys/param.json').write_text(json.dumps(p,indent=2)+'\n')
(root/'ports/zelda/sce_sys/param.json').write_text(json.dumps(p,indent=2)+'\n')
Image.open(root/'build/zelda/icon.ppm').convert('RGBA').save(app/'sce_sys/icon0.png')
# Valid BC7 mode 6 constant-color blocks. Pixel-art background needs no encoder.
# Each 4x4 output block samples one 2x2 source block; endpoints are equal.
def block(rgb):
 v=1<<6;bit=7
 for c in (*rgb,255):
  for _ in range(2):v|=(c>>1)<<bit;bit+=7
 v|=3<<bit # Both endpoint p-bits = 1; remaining indices are zero.
 return v.to_bytes(16,'little')
im=Image.open(root/'build/zelda/background.ppm').convert('RGB').resize((960,540),Image.Resampling.BOX)
data=b''.join(block(rgb) for rgb in im.getdata())
header=bytearray(148);header[:4]=b'DDS '
struct.pack_into('<7I',header,4,124,0x81007,2160,3840,len(data),0,1)
struct.pack_into('<8I',header,76,32,4,0x30315844,0,0,0,0,0)
struct.pack_into('<I',header,108,0x1000)
struct.pack_into('<5I',header,128,98,3,0,1,0)
for name in ['pic0.dds','pic1.dds']:(app/'sce_sys'/name).write_bytes(header+data)
# Decode through Pillow to independently check encoding/alpha.
decoded=Image.open(app/'sce_sys/pic0.dds').convert('RGBA')
assert decoded.size==(3840,2160) and decoded.getpixel((100,100))[3]==255
a=decoded.getpixel((100,100));b=im.getpixel((25,25));assert all(abs(a[i]-b[i])<=1 for i in range(3))
(app/'README.txt').write_text((root/'ports/zelda/README.md').read_text())
out=root.parent/'Zelda-Complete-Test01.zip'
with zipfile.ZipFile(out,'w',zipfile.ZIP_DEFLATED,6) as z:
 for f in sorted(app.rglob('*')):
  if f.is_file():z.write(f,f.relative_to(app.parent))
with zipfile.ZipFile(out) as z:assert z.testzip() is None
print(out, out.stat().st_size,'bytes')
print('eboot SHA256',hashlib.sha256((app/'eboot.bin').read_bytes()).hexdigest())
