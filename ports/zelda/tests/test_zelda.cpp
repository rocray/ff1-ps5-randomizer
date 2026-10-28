#include "../src/nes/nes_core.hpp"
#include "../src/randomizer.hpp"
#include "../src/logic/item_logic.hpp"
#include "../src/qol.hpp"
#include "libretro.h"
#include <cassert>
#include <cstdio>
#include <fstream>
#include <vector>
#include <array>
#include <filesystem>
std::size_t audio_count=0;bool audio_nonzero=false;
namespace ps5::demo::music{void submit(const std::int16_t*p,std::size_t n)noexcept{audio_count+=n;for(unsigned i=0;i<n*2;i++)if(p[i])audio_nonzero=true;}}
using namespace ps5::demo::nes;
void step(NesState&s,unsigned b,unsigned n){for(unsigned i=0;i<n;i++)run_frame(s,b);}
void shot(NesState&s,const char*path){auto*r=(unsigned char*)retro_get_memory_data(RETRO_MEMORY_SYSTEM_RAM);printf("mode %u x %u y %u sword %u timer %.2f\n",r[0x12],r[0x70],r[0x84],r[0x657],s.progress.seconds);std::ofstream f(path,std::ios::binary);f<<"P6\n256 240\n255\n";for(unsigned i=0;i<256*240;i++){auto p=s.framebuffer[i];char c[]={char(p),char(p>>8),char(p>>16)};f.write(c,3);}}
void start_new(NesState&s){step(s,0,180);step(s,8,1);step(s,0,120);step(s,8,1);step(s,0,120);step(s,0x4000,1);step(s,0,10);for(unsigned i=0;i<3;i++){step(s,0x100000,1);step(s,0,15);}step(s,8,1);step(s,0,120);step(s,8,1);step(s,0,240);assert(s.progress.started&&!s.progress.practice);}
int main(int argc,char**argv){assert(argc==2);std::ifstream f(argv[1],std::ios::binary);std::vector<std::uint8_t>base((std::istreambuf_iterator<char>(f)),{}),out(base.size()),again(base.size());const char*error="";assert(base.size()==131088);constexpr unsigned offsets[]={0x18954,0x1895f,0x18985,0x18c10,0x18c5a,0x18c7f};
 for(unsigned seed=0;seed<100;seed++){assert(randomizer::shuffle(base.data(),base.size(),seed,out.data(),error));assert(randomizer::shuffle(base.data(),base.size(),seed,again.data(),error));assert(out==again&&out!=base);std::array<unsigned,32>before{},after{};for(unsigned j=0;j<base.size();j++){bool allowed=false;for(auto k:offsets)if(j==k)allowed=true;if(!allowed)assert(base[j]==out[j]);else{assert((base[j]&0xe0)==(out[j]&0xe0));before[base[j]&31]++;after[out[j]&31]++;}}assert(before==after);}
 assert(!randomizer::shuffle(out.data(),out.size(),0,again.data(),error));const char*dir="/tmp/z1-ps5-test";std::filesystem::create_directory(dir);randomizer::Library lib;char path[256];assert(randomizer::generate(argv[1],1234,0,lib,path,sizeof(path),error,dir));randomizer::Library loaded;assert(randomizer::load_library(loaded,dir));assert(randomizer::select(loaded,0,path,sizeof(path),error,dir));
 NesState s;s.save_path="/tmp/z1-sram-test";std::remove(s.save_path);assert(load_rom(s,path));step(s,0,6000);assert(!s.progress.started);unload_rom(s);std::remove(s.save_path);assert(load_rom(s,path));start_new(s);assert(s.progress.hearts==3);shot(s,"build/zelda/game.ppm");
 // Walk into the starting cave: Link's left edge must align with x=64.
 step(s,0x80,43);step(s,0x10,90);step(s,0,120);shot(s,"build/zelda/cave.ppm");
 step(s,0x10,70);step(s,0,240);step(s,0x20,6);step(s,0x10,65);step(s,0,100);shot(s,"build/zelda/sword.ppm");
 assert(s.progress.items[0]==1&&s.progress.seen[0]==1);
 assert(audio_count>100000&&audio_nonzero);s.running=false;double t=s.progress.seconds;run_frame(s,0);assert(s.progress.seconds==t);s.running=true;save_ram(s);char stamp[64];qol::read_stamp(s.save_path,stamp,sizeof(stamp));assert(std::strstr(stamp,"UTC"));unload_rom(s);
 // Exercise randomized starting-cave pickups in the real emulator.
 for(unsigned seed=0;seed<24;seed++){
  assert(zlogic::shuffle(base.data(),base.size(),seed,out.data(),error));
  {std::ofstream rom("/tmp/z1-all-core.nes",std::ios::binary);rom.write((char*)out.data(),out.size());}
  std::remove(s.save_path);assert(load_rom(s,"/tmp/z1-all-core.nes"));start_new(s);
  step(s,0x80,43);step(s,0x10,90);step(s,0,120);step(s,0x10,70);step(s,0,240);step(s,0x20,6);step(s,0x10,65);step(s,0,100);
  unsigned item=out[0x18611]&63;auto*ram=(unsigned char*)retro_get_memory_data(RETRO_MEMORY_SYSTEM_RAM);
  constexpr unsigned addresses[]={0,0x657,0x657,0x657,0x65d,0x65c,0x65b,0x65b,0x659,0x659,0x65a,0x664,0x660,0x663,0,0,0x65f,0x661,0x662,0x662,0x665,0x666};
  if(item==26)assert(s.progress.hearts==4);
  else if(item<22)assert(addresses[item]&&ram[addresses[item]]!=0);
  else if(item==28)assert(ram[0x676]);else if(item==29)assert(ram[0x674]);else if(item==30)assert(ram[0x675]);else assert(false);
  unload_rom(s);
 }
 std::remove("/tmp/z1-all-core.nes");puts("PASS: 24 all-item ROMs booted and randomized cave pickups collected in FCEUmm");
 for(unsigned i=0;i<12;i++){assert(load_rom(s,path));step(s,0,10);assert(!load_rom(s,"/no/zelda.nes"));assert(s.loaded);unload_rom(s);}
 std::uint8_t ram[2048]{};ram[0x12]=5;ram[0x11]=1;ram[0x66f]=0x22;RunProgress r;sample_run(r,ram,60);sample_run(r,ram,60);assert(r.started&&!r.practice);ram[0x10]=4;ram[0x660]=1;ram[0x671]=8;assert(sample_run(r,ram,60));assert(r.reached[3]&&!r.reached[0]&&r.found_level[6]==4);double split=r.split[3];sample_run(r,ram,60);assert(r.split[3]==split);ram[0x12]=0x13;assert(sample_run(r,ram,60));assert(r.finished&&r.reached[8]);t=r.seconds;sample_run(r,ram,60);assert(r.seconds==t);RunProgress practice;ram[0x12]=5;sample_run(practice,ram,60);sample_run(practice,ram,60);assert(practice.practice);for(double best:practice.records)assert(best==0);
 std::remove(s.save_path);std::remove("/tmp/z1-sram-test.time");std::filesystem::remove_all(dir);puts("PASS: 100 seeds, byte boundaries, disk library, real core boot/title audio/registration/timer/sword pickup, pause, SRAM timestamp, 12 reloads; synthetic split/location/finish/practice tests");}
