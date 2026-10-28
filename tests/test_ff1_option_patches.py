#!/usr/bin/env python3
"""Execute the patched ROM routines with py65's NMOS6502 CPU, not a mock award.
Battle/video calls are stubs; inventory, gates and reward persistence run as ROM.
"""
from pathlib import Path
from py65.devices.mpu6502 import MPU
import sys
rom=Path(sys.argv[1]).read_bytes()
def cpu():
 m=MPU();m.memory[0x8000:0xc000]=rom[0x38010:0x3c010];m.memory[0xc000:]=rom[0x3c010:0x40010]
 m.memory[0x6200:0x6300]=[5]*256 # visible and opened-chest bits must survive
 m.memory[0x11:0x14]=[0x20,0x21,0x22]
 m.memory[0x90c5]=0x60 # battle returns a win; rendering calls do not change RAM
 return m
def call(m,pc,stop=0x7000):
 m.pc=pc;m.sp=0xfd;m.memory[0x1fe]=(stop-1)&255;m.memory[0x1ff]=(stop-1)>>8
 for _ in range(3000):
  if m.pc==stop:return
  m.step()
 raise AssertionError(('routine did not return',hex(m.pc)))
def entry(m,obj):
 at=0x90d3+obj*2;return m.memory[at]+m.memory[at+1]*256
# reward NPCs: original map requirements handled by actual ROM gates.
for obj in [0x06,0x07,0x0a,0x0d,0x11,0x12,0x13,0xbb]:
 m=cpu();m.memory[0x6206]|=2;m.memory[0x620b]|=2;m.memory[0x620c]&=~1;m.memory[0x6022]=1;m.memory[0x6023]=1
 before=m.memory[0x6021:0x6031].copy();call(m,entry(m,obj));after=m.memory[0x6021:0x6031].copy()
 assert m.memory[0x6200+obj]&0x80 and m.memory[0x6200+obj]&4
 assert m.a==0xf0 and 1<=m.memory[0x61]<=16
 reward=m.memory[0x61];assert after[reward-1]==before[reward-1]+1
 # A consumed quest reward must not be given again; this bit survives SRAM saves.
 m.memory[0x6020+reward]=0;prior=m.memory[0x6020:0x6031].copy();call(m,entry(m,obj));assert m.memory[0x6020:0x6031]==prior
 # Gate absence must not claim an item.
 if obj in [6,7,10,13,187]:
  m=cpu();call(m,entry(m,obj));assert not m.memory[0x6200+obj]&0x80
# Matoya must still trade after the prince is awake if her own reward is unclaimed.
m=cpu();m.memory[0x6206]|=2;m.memory[0x6023]=1;call(m,entry(m,10));assert m.memory[0x620a]&0x80 and m.memory[0x6023]==0
# Full equipment bags cannot corrupt item counters or charge the player.
for item in range(28,68):
 for full in [False,True]:
  m=cpu();m.memory[0x030c]=item
  for char in range(4):
   for slot in range(4):m.memory[0x6118+char*64+slot]=1
  if not full:m.memory[0x619a]=0
  counters=m.memory[0x6020:0x6040].copy();call(m,0x84e2)
  assert bool(m.p&1)==full and m.memory[0x6020:0x6040]==counters
  if not full:assert m.memory[0x619a]==item-27
for item in list(range(1,17))+list(range(22,28)):
 for qty in [0,98,99]:
  m=cpu();m.memory[0x030c]=item;m.memory[0x6020+item]=qty;call(m,0x84e2)
  assert m.memory[0x6020+item]==min(qty+1,99) and bool(m.p&1)==(qty==99)
# Test the purchase entry itself: pay only on successful award.
for full in [False,True]:
 m=cpu();m.memory[0x030c]=67;m.memory[0xa4cd:0xa4d1]=[0xe6,0xfe,0x60,0xea];m.memory[0xaa5b]=0x60
 if full:
  for c in range(4):m.memory[0x6118+64*c:0x611c+64*c]=[1]*4
 call(m,0xa4ad,0xa481);assert m.memory[0xfe]==int(not full)
print('PASS: 6502 NPC gate/one-shot/consumed-reward/Matoya-after-Herb checks; all 40 weapons, full bags, consumable cap; payment only after award')
# Open spell learning uses the game's real permission and level-slot routine.
for cls in range(12):
 for spell in range(64):
  m=cpu();m.memory[0x6100]=cls;m.memory[0x030a]=0;m.memory[0x030c]=176+spell
  call(m,0xaadf);assert m.memory[0x030b]==spell%8+1 and m.memory[0x030a]==(spell//8)*4
print('PASS: all 12 classes can learn all 64 spells in original level slots')
