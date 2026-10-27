#include <array>
#include <cstdint>
#include <span>
#include <string_view>
#define private public
#include "../src/demo_renderer.hpp"
#undef private
#define main ff1_app_entry
#include "../src/main.cpp"
#undef main
#include <vector>
#include <cassert>
#include <chrono>
#include <cstdio>
static unsigned offset(unsigned x,unsigned y){
 unsigned v=((y<<4)&0x70)^((y<<5)&0xf00)^((y<<9)&0x1000)^((y<<8)&0x4000)^((x<<2)&0xc)^((x<<5)&0x380)^((x<<4)&0x400)^((x<<6)&0x800)^((x<<9)&0xa000);
 return (((y>>7)*15+(x>>7))*65536+v)/4;
}
int main(){
 constexpr unsigned size=15*9*65536/4;
 std::vector<std::uint32_t>a(size,0),b(size,0);Canvas fast(a.data()),old(b.data());
 for(auto rect:{std::array<unsigned,4>{1380,0,540,250},{1380,255,540,475},{0,756,1376,324}}){
  auto [x,y,w,h]=rect;
  draw_panel(fast,x,y,w,h);
  old.rectangle(x,y,w,h,Color::ink);old.rectangle(x+2,y+2,w-4,h-4,Color::steel);
  old.rectangle(x+4,y+4,w-8,h-8,Color::white);old.rectangle(x+7,y+7,w-14,h-14,Color::steel);
  old.rectangle(x+10,y+10,w-20,h-20,Color::ink);
  for(unsigned row=12;row<h-12;row++)old.rectangle(x+12,y+row,w-24,1,static_cast<Color>(0xff000000|((85+(h-row)*35/h)<<16)|(10<<8)|4));
 }
 assert(a==b);
 std::uint32_t src[256*240];for(unsigned i=0;i<256*240;i++)src[i]=0xff000000|i;
 fast.region(0,0,1376,756,src,256,0,0,256,240);
 for(unsigned y=0;y<756;y++)for(unsigned x=0;x<1376;x++)assert(a[offset(x,y)]==src[(y*240/756)*256+x*256/1376]);
 fast.region(1900,1060,100,100,src,256,0,0,256,240,true);
 for(unsigned y=0;y<20;y++)for(unsigned x=0;x<20;x++){
  auto v=src[(y*240/100)*256+x*256/100];unsigned r=v&255,g=(v>>8)&255,b=(v>>16)&255;
  if(r+g+b>160){unsigned k=(r+g+b)/6;v=0xff000000|(k<<16)|(k<<8)|k;}assert(a[offset(1900+x,1060+y)]==v);
 }
 NesState state;nes=&state;state.framebuffer=src;
 auto start=std::chrono::steady_clock::now();
 for(unsigned i=0;i<120;i++){fast.clear(Color::background);fast.region(0,0,1376,756,src,256,0,0,256,240);draw_overlay(fast);}
 auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()/120;
 std::printf("PASS: panel pixel equivalence, scaled/clipped/dim pixels; host cached overlay %.2f ms/frame (not PS5 timing)\n",ms);
}
