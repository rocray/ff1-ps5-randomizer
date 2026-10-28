#include "../src/logic/item_logic.hpp"
#include "../src/randomizer.hpp"
#include <cassert>
#include <array>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <vector>
#include <filesystem>
#include <chrono>
int main(int argc,char**argv){assert(argc==2);std::ifstream f(argv[1],std::ios::binary);std::vector<unsigned char>base((std::istreambuf_iterator<char>(f)),{}),a(base.size()),b(base.size());assert(base.size()==randomizer::rom_size);const char*error="";zlogic::Location loc[96];unsigned count=zlogic::locations(base.data(),loc,96);assert(count==32);assert(zlogic::validate(base.data()).beatable);
 std::vector<unsigned char>masks(base.size());for(unsigned j=0;j<count;j++)masks[loc[j].offset]=loc[j].mask;for(unsigned bank:{0x18710u,0x18a10u})for(unsigned n=0;n<128;n++)if((base[bank+512+n]&31)==3)masks[bank+512+n]=31;masks[0x1785f]=255;
 std::array<unsigned,64>original{};for(unsigned j=0;j<count;j++)original[base[loc[j].offset]&loc[j].mask]++;
 auto start=std::chrono::steady_clock::now();unsigned max_spheres=0;double worst_ms=0;
 for(unsigned seed=0;seed<1000;seed++){
  auto t=std::chrono::steady_clock::now();assert(zlogic::shuffle(base.data(),base.size(),seed,a.data(),error));double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t).count();if(ms>worst_ms)worst_ms=ms;
  auto r=zlogic::validate(a.data());assert(r.beatable&&r.collected==32&&r.locations==32&&r.triforce==255&&r.ganon&&r.zelda);if(r.spheres>max_spheres)max_spheres=r.spheres;
  assert(zlogic::shuffle(base.data(),base.size(),seed,b.data(),error)&&a==b);assert(a!=base);
  std::array<unsigned,64>items{};for(unsigned j=0;j<count;j++)items[a[loc[j].offset]&loc[j].mask]++;assert(items==original);
  for(unsigned j=0;j<a.size();j++)assert(((a[j]^base[j])&~masks[j])==0);
 }
 // Deliberately impossible placements must fail independently of generation.
 auto move=[&](unsigned item,unsigned target){b=base;unsigned from=0;for(unsigned j=0;j<count;j++)if((b[loc[j].offset]&loc[j].mask)==item)from=loc[j].offset;assert(from);auto old=b[target]&31;b[target]=(b[target]&224)|item;b[from]=(b[from]&224)|old;};
 move(12,0x18970);assert(!zlogic::validate(b.data()).beatable); // raft behind L4 raft entrance
 move(5,0x18c5a);assert(!zlogic::validate(b.data()).beatable); // recorder behind L7 recorder entrance
 move(13,0x1789a);assert(!zlogic::validate(b.data()).beatable); // ladder on coast requiring ladder
 b=base;for(unsigned j=0;j<count;j++){unsigned item=b[loc[j].offset]&loc[j].mask;if(item==1||item==2||item==3||item==7||item==16)b[loc[j].offset]=(b[loc[j].offset]&~loc[j].mask)|17;}assert(!zlogic::validate(b.data()).beatable); // no usable weapon/farming
 // Removing shop keys and the Magical Key cannot magically open locked doors.
 b=base;for(unsigned c=13;c<=16;c++)for(unsigned p=0;p<3;p++){auto&v=b[0x18610+c*3+p];if((v&63)==25)v=(v&192)|24;}for(unsigned j=0;j<count;j++)if((b[loc[j].offset]&loc[j].mask)==11)b[loc[j].offset]=(b[loc[j].offset]&~loc[j].mask)|17;assert(!zlogic::validate(b.data()).beatable);
 const char*dir="/tmp/z1-mode-test";std::filesystem::create_directory(dir);randomizer::Library lib;char path[256],save[256];assert(randomizer::generate(argv[1],42,0,lib,path,sizeof(path),error,dir));std::string gearpath=path;assert(std::strstr(path,"z1-g1-"));assert(randomizer::generate(argv[1],42,1,lib,path,sizeof(path),error,dir,randomizer::Mode::AllItems));assert(std::strstr(path,"z1-a1-")&&lib.entries[1].flags==2);assert(randomizer::load_library(lib,dir));assert(lib.entries[0].flags==0&&lib.entries[1].flags==2);assert(randomizer::select(lib,0,path,sizeof(path),error,dir)&&gearpath==path);assert(randomizer::save_path(lib,0,save,sizeof(save),dir));std::string gs=save;assert(randomizer::save_path(lib,1,save,sizeof(save),dir)&&gs!=save);assert(randomizer::select(lib,1,path,sizeof(path),error,dir));{std::ofstream corrupt(path,std::ios::binary|std::ios::trunc);corrupt<<"bad";}assert(!randomizer::select(lib,1,path,sizeof(path),error,dir));std::filesystem::remove_all(dir);
 printf("PASS: 1000 deterministic beatable seeds; all 32 locations reachable; item pool and byte masks preserved; five deliberate deadlocks rejected; mode/library/save isolation. Max spheres %u, worst generation %.2f ms; suite %.2fs\n",max_spheres,worst_ms,std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count());
}
