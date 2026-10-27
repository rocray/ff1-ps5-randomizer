#include "../src/qol.hpp"
#include "../src/randomizer.hpp"
#include <cassert>
#include <cstring>
#include <cstdio>
#include <fstream>
#include <vector>
#include <filesystem>
int main(int argc,char **argv){
 assert(argc==2);auto v=qol::viewport(0,true);assert(v.w*3==v.h*4&&v.x==184&&v.bottom);v=qol::viewport(1,true);assert(v.w*3==v.h*4&&!v.bottom);
 for(unsigned m=0;m<4;m++)for(bool b:{false,true}){v=qol::viewport(m,b);assert(v.x+v.w<=1920&&v.y+v.h<=1080);}
 qol::Settings s;s.layout=3;s.scanlines=2;s.curve=1;s.run=1;assert(qol::save(s,"/tmp/ff1-settings-test"));qol::Settings restored;assert(qol::load(restored,"/tmp/ff1-settings-test"));assert(restored.layout==3&&restored.run==1);std::remove("/tmp/ff1-settings-test");assert(!qol::load(restored,"/tmp/ff1-settings-test")&&restored.layout==0);
 char text[64];qol::format_utc(951782400,text,sizeof(text));assert(!std::strcmp(text,"2000-02-29 00:00 UTC"));qol::format_utc(0,text,sizeof(text));assert(!std::strcmp(text,"NO TIMESTAMP"));assert(qol::stamp_save("/tmp/ff1-stamp-test"));qol::read_stamp("/tmp/ff1-stamp-test",text,sizeof(text));assert(std::strstr(text," UTC"));std::remove("/tmp/ff1-stamp-test.time");
 std::vector<std::uint32_t> pixels(256*240,0xffabcdef);const auto *curved=qol::curve_frame(pixels.data());assert(curved[0]==0xff000000&&curved[120*256+128]==pixels[0]);
 std::ifstream f(argv[1],std::ios::binary);std::vector<std::uint8_t> rom((std::istreambuf_iterator<char>(f)),{}),patch=rom;
 assert(randomizer::enable_run(patch.data(),patch.size()));assert(!randomizer::enable_run(patch.data(),patch.size()));
 for(unsigned i=0;i<rom.size();i++)if(i<0x3d087||i>=0x3d087+17)assert(patch[i]==rom[i]);
 // Execute the tiny 6502 movement routine for all four vehicle modes and B states.
 for(unsigned vehicle:{1u,2u,4u,8u})for(bool held:{false,true}){
  unsigned pc=0x3d087,a=0,carry=0,speed=0;bool overflow=false;
  for(unsigned steps=0;steps<20;steps++){unsigned op=patch[pc++];if(op==0xa5){assert(patch[pc++]==0x42);a=vehicle;}else if(op==0x4a){carry=a&1;a>>=1;}else if(op==0x69){unsigned val=patch[pc++],sum=a+val+carry;overflow=((~(a^val)&(a^sum))&128)!=0;carry=sum>255;a=sum&255;}else if(op==0x0a){carry=a>>7;a=(a<<1)&255;}else if(op==0x24){assert(patch[pc++]==0x20);overflow=held;}else if(op==0x70){unsigned n=patch[pc++];if(overflow)pc+=n;}else if(op==0x85){assert(patch[pc++]==0x34);speed=a;}else if(op==0x60)break;else assert(false);}
  assert(speed==(vehicle==8?2u:1u)*(held?2:1));
 }
 const char *dir="/tmp/ff1-qol-seeds";std::filesystem::create_directory(dir);
 struct Old {std::uint32_t magic=0x31525446,version=1;std::uint32_t entries[16]{};std::uint32_t checksum=0;} old;
 old.entries[0]=123;old.entries[1]=456;old.checksum=randomizer::crc32(reinterpret_cast<std::uint8_t*>(&old),offsetof(Old,checksum));
 {std::ofstream o(std::string(dir)+"/ff1-seeds-v1.bin",std::ios::binary);o.write(reinterpret_cast<char*>(&old),sizeof(old));}
 randomizer::Library lib;assert(randomizer::load_library(lib,dir));assert(lib.entries[0].seed==123&&!lib.entries[0].flags);
 const char *err="";char path[256];assert(randomizer::generate(argv[1],42,1,lib,path,sizeof(path),err,dir,true));assert(std::strstr(path,"-run.nes"));assert(lib.entries[1].flags==1);assert(randomizer::load_library(lib,dir));assert(lib.entries[0].seed==123&&lib.entries[1].flags==1);assert(randomizer::select(lib,1,path,sizeof(path),err,dir));
 std::filesystem::remove_all(dir);
 puts("PASS: layouts, settings persistence/defaults, UTC leap date, save timestamp, CRT bounds, run patch boundaries/speeds, v1 library migration and run-ROM generation");
}
