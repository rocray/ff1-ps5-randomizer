#!/usr/bin/env python3
"""Real 6502 learning, level/MP selection, dispatch and a shuffled CURE cast.
UI drawing/input is stubbed; spell logic, permissions and MP arithmetic are ROM.
"""
from pathlib import Path
from py65.devices.mpu6502 import MPU
import sys
base=Path(sys.argv[1]).read_bytes();rom=Path(sys.argv[2]).read_bytes();restricted=Path(sys.argv[3]).read_bytes()
def cpu(data=rom,bank=14):
 m=MPU();m.memory[0x8000:0xc000]=data[16+bank*16384:16+(bank+1)*16384];m.memory[0xc000:]=data[0x3c010:0x40010];return m
def run(m,pc,stops):
 m.pc=pc
 for _ in range(10000):
  if m.pc in stops:return m.pc
  m.step()
 raise AssertionError(hex(m.pc))
def call(m,pc):
 m.sp=0xfd;m.memory[0x1fe]=0xff;m.memory[0x1ff]=0x6f;return run(m,pc,{0x7000})
names=[base[0x2b870+i*2:0x2b872+i*2] for i in range(64)]
old_at_new=[names.index(rom[0x2b870+i*2:0x2b872+i*2]) for i in range(64)]
assert len(set(old_at_new))==64 and sum(i//8!=v//8 for i,v in enumerate(old_at_new))>20
old_dispatch={base[0x3af0a+i*7]-176:int.from_bytes(base[0x3af0e+i*7:0x3af10+i*7],'little') for i in range(13)}
# Learning permissions follow spell identity, while the destination dictates level.
for cls in range(12):
 for slot,old in enumerate(old_at_new):
  m=cpu(restricted);m.memory[0x6100]=cls;m.memory[0x30c]=176+slot;m.memory[0xaa5b]=0x60
  forbidden=bool(base[0x3ad28+cls*8+old//8]&(0x80>>(old%8)))
  m.sp=0xfd;m.memory[0x1fe]=0xff;m.memory[0x1ff]=0x6f
  end=run(m,0xaadf,{0x7000,0xa365})
  assert (end==0xa365)==forbidden,(cls,slot,old,hex(end))
  if not forbidden:assert m.memory[0x30a]==slot//8*4 and m.memory[0x30b]==slot%8+1
# Out-of-battle IDs and required MP must be correct in all slots, including #8.
for char in range(4):
 for slot,old in enumerate(old_at_new):
  m=cpu();m.memory[0x66]=char;m.memory[0x62]=(slot//8)*4
  m.memory[0x6300+char*64+(slot//8)*4]=slot%8+1
  m.memory[0x6320+char*64+slot//8]=5
  call(m,0xb102);assert m.p&1 and m.memory[0x65]==char*64+0x20+slot//8
  end=run(m,0xaee8,set(old_dispatch.values())|{0xaf54})
  assert end==old_dispatch.get(old,0xaf54),(char,slot,old,hex(end))
  m.memory[0x6320+char*64+slot//8]=0;call(m,0xb102);assert not(m.p&1)
# Battle menu picks the new level's MP and the relocated effect data.
bank=rom[0x30010:0x34010];pattern=bytes.fromhex('bea768de2063');at=bank.index(pattern)+0x8000
for char in range(4):
 for slot,old in enumerate(old_at_new):
  m=cpu(bank=12);m.memory[0x6b7a]=char;m.memory[0x6af8]=slot//32;m.memory[0x6aab]=(slot//8)%4;m.memory[0x6aaa]=0
  m.memory[0x6300+char*64+slot//8*4]=slot+1 # actual battle-converted format
  m.memory[0x6320+char*64:0x6328+char*64]=[5]*8
  run(m,0x9509,{0x9575})
  assert m.memory[0x68a7+char]==char*64+slot//8 and m.memory[0x6b7d]==slot
  ptr=m.memory[0x80]+256*m.memory[0x81]
  assert bytes(m.memory[ptr:ptr+8])==base[0x301f0+old*8:0x301f8+old*8]
  m.y=char;run(m,at,{at+6})
  assert m.memory[0x6320+char*64:0x6328+char*64]==[4 if i==slot//8 else 5 for i in range(8)]
# Execute CURE's actual heal and MP decrement at its shuffled destination.
slot=old_at_new.index(0);m=cpu();m.memory[0x66]=0;m.memory[0x62]=slot//8*4;m.memory[0x6300+slot//8*4]=slot%8+1
m.memory[0x6320:0x6328]=[5]*8;m.memory[0x610a]=1;m.memory[0x610c]=100
call(m,0xb102);run(m,0xaee8,{0xaf5c})
for addr in [0xb400,0xb92b,0xb613]:m.memory[addr]=0x60
# Stub target selection: choose party member 0 and clear cancel carry.
m.memory[0xb3a0:0xb3a7]=[0xa9,0,0x85,0x62,0x18,0x60,0xea]
run(m,0xaf5c,{0xae97});assert 17<=m.memory[0x610a]<=32
assert m.memory[0x6320:0x6328]==[4 if i==slot//8 else 5 for i in range(8)]
print('PASS: all64 spells/all12 class permissions; all4 characters: out-of-battle routing, #8 slot fix, new-level MP, battle effect/charge; real shuffled CURE heals and charges correct level')
