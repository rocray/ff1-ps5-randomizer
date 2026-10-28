#include <cstdint>
#include <span>
#include <string_view>
#define private public
#include "../src/demo_renderer.hpp"
#undef private
#define main zelda_main
#include "../src/main.cpp"
#undef main
#include <vector>
#include <fstream>
#include <chrono>
#include <cassert>
unsigned off(unsigned x,unsigned y){return ((((y>>7)*15+(x>>7))<<16)+(((y<<4)&0x70)^((y<<5)&0xf00)^((y<<9)&0x1000)^((y<<8)&0x4000)^((x<<2)&0xc)^((x<<5)&0x380)^((x<<4)&0x400)^((x<<6)&0x800)^((x<<9)&0xa000)))/4;}
void dump(const std::vector<std::uint32_t>&fb,const char*name,unsigned w=1920,unsigned h=1080){std::ofstream out(name,std::ios::binary);out<<"P6\n"<<w<<" "<<h<<"\n255\n";for(unsigned y=0;y<h;y++)for(unsigned x=0;x<w;x++){auto p=fb[off(x,y)];char rgb[]={char(p),char(p>>8),char(p>>16)};out.write(rgb,3);}}
int main(){std::vector<std::uint32_t>fb(15*9*65536/4);Canvas c(fb.data());{std::ifstream a("dist/PPSA01988/assets/title.raw",std::ios::binary),b("dist/PPSA01988/assets/overlay.raw",std::ios::binary);a.read((char*)title_art,sizeof(title_art));b.read((char*)overlay_art,sizeof(overlay_art));assert(a&&b);art_ready=true;}scenery(c);dump(fb,"build/zelda/background.ppm");menu(c);footer(c);dump(fb,"build/zelda/menu.ppm");core::NesState state;game=&state;
 std::ifstream f("build/zelda/game.ppm",std::ios::binary);std::string line;for(int i=0;i<3;i++)std::getline(f,line);std::vector<std::uint32_t>pixels(256*240);for(auto&p:pixels){unsigned char rgb[3]{};f.read((char*)rgb,3);p=0xff000000|(rgb[2]<<16)|(rgb[1]<<8)|rgb[0];}
 state.framebuffer=pixels.data();state.progress.started=true;state.running=true;state.progress.hearts=3;state.progress.seconds=14;draw_game(c);dump(fb,"build/zelda/overlay.ppm");
 state.progress.items[0]=state.progress.seen[0]=1;state.progress.found_level[0]=0;state.progress.triforce=5;state.progress.visited=7;state.progress.reached[0]=true;state.progress.split[0]=132.4;state.progress.best[0]=140;state.progress.seconds=530;draw_game(c);dump(fb,"build/zelda/overlay-active.ppm");
 auto start=std::chrono::steady_clock::now();for(unsigned i=0;i<120;i++){state.progress.seconds+=1.0/60;draw_game(c);}auto end=std::chrono::steady_clock::now();printf("Artwork overlay rendering: %.2f ms/frame on host\n",std::chrono::duration<double,std::milli>(end-start).count()/120);
 selection=4;scenery(c);menu(c);footer(c);dump(fb,"build/zelda/menu-settings-selected.ppm");

 c.clear(navy);c.rectangle(10,10,492,492,gold);c.rectangle(15,15,482,482,navy);c.rectangle(245,50,22,330,Color::white);c.rectangle(190,310,130,20,gold);c.rectangle(242,330,28,80,green);triforce(c,256,95,100,gold);c.text_centered(256,425,"ZELDA",10,gold);dump(fb,"build/zelda/icon.ppm",512,512);
}
