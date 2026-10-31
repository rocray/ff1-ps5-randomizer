#include <cstdint>
#include <span>
#include <string_view>
#define private public
#include "../src/demo_renderer.hpp"
#undef private
#define main ff1_entry
#include "../src/main.cpp"
#undef main
#include <vector>
#include <fstream>
unsigned at(unsigned x,unsigned y){return ((((y>>7)*15+(x>>7))<<16)+(((y<<4)&0x70)^((y<<5)&0xf00)^((y<<9)&0x1000)^((y<<8)&0x4000)^((x<<2)&0xc)^((x<<5)&0x380)^((x<<4)&0x400)^((x<<6)&0x800)^((x<<9)&0xa000)))/4;}
int main(){std::vector<std::uint32_t>fb(15*9*65536/4),bg(1920*1080);std::ifstream f("dist/PPSA01987/assets/bg.raw",std::ios::binary);f.read((char*)bg.data(),bg.size()*4);background=bg.data();Canvas c(fb.data());c.image(0,0,1920,1080,bg.data());seed_options={true,true,true,true,true};edit_options=true;option_row=1;draw_seed_screen(c,false);draw_button_footer(c);std::ofstream o("build/ff1-options-ui.ppm",std::ios::binary);o<<"P6\n1920 1080\n255\n";for(unsigned y=0;y<1080;y++)for(unsigned x=0;x<1920;x++){auto p=fb[at(x,y)];char rgb[]={char(p),char(p>>8),char(p>>16)};o.write(rgb,3);}}
