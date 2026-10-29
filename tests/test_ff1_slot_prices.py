#!/usr/bin/env python3
"""Execute the bank-switched 6502 pricing path, list tokens and confirmations."""
import sys
from pathlib import Path
from py65.devices.mpu6502 import MPU
rom=Path(sys.argv[1]).read_bytes()
base=Path(sys.argv[2]).read_bytes()
class Machine:
 def __init__(self):
  self.m=MPU();self.bank=14;self.swap(14)
  self.m.memory[0xc000:0x10000]=rom[0x3c010:0x40010]
  self.m.memory[0xfe03]=0x60
 def swap(self,bank):
  self.bank=bank;self.m.memory[0x8000:0xc000]=rom[16+bank*16384:16+(bank+1)*16384]
  if bank==14:
   # UI drawing itself is outside this CPU test; don't execute PPU waits.
   self.m.memory[0xaa5b]=self.m.memory[0xaa32]=0x60
 def call(self,pc,stop=0x7000):
  m=self.m;m.sp=0xfd;m.memory[0x1fe]=0xff;m.memory[0x1ff]=0x6f;m.pc=pc
  for _ in range(10000):
   if m.pc==stop:return
   if m.pc==0xfe03:self.swap(m.a)
   m.step()
  raise AssertionError(hex(m.pc))
 def word(self,at):return self.m.memory[at]+256*self.m.memory[at+1]
# Global prices (including chest gold and spells) and register/bank contract.
x=Machine()
for item in range(256):
 x.m.a=item;x.m.x=77;x.call(0xecb9)
 assert x.word(0x10)==int.from_bytes(rom[0x37c10+2*item:0x37c12+2*item],'little')
 assert x.m.x==77 and x.m.memory[0x12]==0 and x.bank==14
slots=0;different=[]
for shop in list(range(1,7))+list(range(11,17))+list(range(61,67))+[70]+list(range(21,41)):
 x=Machine();m=x.m;m.memory[0x51]=shop;x.call(0xa7d1)
 ptr=int.from_bytes(base[0x38310+2*shop:0x38312+2*shop],'little')+0x30010
 items=[]
 for j in range(5):
  item=rom[ptr+j]
  if not item:break
  items.append(item)
  assert m.memory[0x340+j]==base[ptr+j]
 assert m.memory[0x305]==0 and m.memory[0x345]==1 and x.bank==14
 # Execute actual inventory string builder, stop immediately before rendering.
 x.call(0xa989 if 21<=shop<=40 else 0xa857,0xa9cf if 21<=shop<=40 else 0xa891)
 saved=m.memory[0x310:0x339]
 for j,item in enumerate(items):
  m.memory[0x310:0x339]=saved
  assert m.memory[0x311+j*8]==item
  token=m.memory[0x316+j*8]
  assert (token==item if 21<=shop<=40 else token==base[ptr+j])
  if 21<=shop<=40:assert (token-176)//8==(base[ptr+j]-176)//8
  # PrintPrice calls the same lookup used to display list/confirmation prices.
  m.a=token;x.call(0x8e44,0x8e47)
  expected=int.from_bytes(rom[0x37c10+2*token:0x37c12+2*token],'little')
  assert x.word(0x10)==expected
  m.memory[0x62]=j;x.call(0xaa65)
  assert m.memory[0x30c]==item,(shop,j,item,m.memory[0x30c])
  assert x.word(0x30e)==expected
  # Affordability and subtraction use exactly the confirmed price.
  if expected:
   m.memory[0x601c:0x601f]=[(expected-1)&255,(expected-1)>>8,0]
   x.call(0xa4eb);assert m.p&1
  m.memory[0x601c:0x601f]=[0xff,0xff,0]
  x.call(0xa4eb);assert not(m.p&1)
  x.call(0xa4cd)
  assert x.word(0x601c)==65535-expected
  slots+=1
  different.append((item,expected,shop,j))
 # Entering sell mode clears custom tokens; actual name and price ID coincide.
 x.call(0x84f8);assert m.y==0 and m.a==0 and m.memory[0x345]==0
 m.memory[0x300:0x306]=[67,29,30,0,0,0]
 x.call(0xa857,0xa891)
 for j,item in enumerate([67,29,30]):assert m.memory[0x316+j*8]==item
# At least one identical replacement is priced differently in different slots.
assert any(a[0]==b[0] and a[1]!=b[1] for a in different for b in different)
print(f'PASS: all 256 global prices; {slots} shop slots: bank switching, item/price tokens, confirmation, charge, and sell-mode reset')
