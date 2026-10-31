#include <initializer_list>
#include "../src/frame_pacer.hpp"
#include <cassert>
#include <cstdio>
int main(){
 for(unsigned hz:{30u,60u,120u}){
  FramePacer p;unsigned frames=0;
  for(unsigned i=0;i<=hz*20;i++)frames+=p.advance(1000000+static_cast<std::uint64_t>(i)*1000000/hz,60.0988,true);
  assert(frames==1202);
 }
 FramePacer p;assert(p.advance(1,60,true)==1);assert(p.advance(33334,60,true)==1);
 assert(p.advance(5000000,60,false)==0);assert(p.advance(9000000,60,true)==1);
 assert(p.advance(20000000,60,true)==4);assert(p.advance(20000000,60,true)==0);
 assert(!menu_chord(2,2));assert(!menu_chord(4,4));assert(menu_chord(6,2));assert(menu_chord(6,4));assert(menu_chord(6,6));assert(!menu_chord(6,0));assert(!menu_chord(0x1000,0x1000));
 puts("PASS: same emulation rate at 30/60/120 render FPS, pause, bounded stalls, L3+R3 edges");
}
