#include "../src/randomizer.hpp"
#include "../src/nes/nes_core.hpp"
#include <cassert>
#include <fstream>
#include <vector>
#include <array>
#include <filesystem>
#include <cstdio>
extern std::size_t test_audio_frames;extern bool test_audio_nonzero;
int main(int argc,char **argv){
 assert(argc==2);std::ifstream f(argv[1],std::ios::binary);std::vector<std::uint8_t> base((std::istreambuf_iterator<char>(f)),{}),a(base.size()),b(base.size());const char *error="";
 assert(base.size()==randomizer::rom_size);assert(randomizer::validate_nes(base.data(),base.size()));
 for(unsigned seed=0;seed<100;seed++){
  assert(randomizer::shuffle(base.data(),base.size(),seed,a.data(),error));
  assert(randomizer::shuffle(base.data(),base.size(),seed,b.data(),error));assert(a==b);assert(a!=base);
  std::array<unsigned,256> before{},after{};
  for(unsigned i=0;i<base.size();i++){
   if(i<0x3110||i>=0x3210||base[i]<22)assert(a[i]==base[i]);
   if(i>=0x3110&&i<0x3210){before[base[i]]++;after[a[i]]++;}
  }
  assert(before==after);
 }
 assert(!randomizer::shuffle(a.data(),a.size(),1,b.data(),error));
 assert(!randomizer::validate_nes(base.data(),base.size()-1));
 auto bad=base;bad[6]|=4;assert(!randomizer::validate_nes(bad.data(),bad.size()));
 bad=base;bad[4]=32;assert(!randomizer::validate_nes(bad.data(),bad.size()));
 const char *dir="/tmp/ff1-seeds-test";std::filesystem::create_directory(dir);randomizer::Library lib;char path[256];
 for(unsigned i=0;i<8;i++)assert(randomizer::generate(argv[1],i,i,lib,path,sizeof(path),error,dir));
 randomizer::Library read;assert(randomizer::load_library(read,dir));
 using namespace ps5::demo::nes;NesState s;
 char previous_save[128]{};
 for(unsigned i=0;i<24;i++){
  assert(randomizer::select(read,i%8,path,sizeof(path),error,dir));assert(load_rom(s,path));
  assert(std::string(s.save_path)!="/download0/ff1.srm");
  if(i)assert(std::string(previous_save)!=s.save_path);std::snprintf(previous_save,sizeof(previous_save),"%s",s.save_path);
  unsigned count=i==0?2400:180;
  for(unsigned j=0;j<count;j++)run_frame(s,i==0?((j%600==0)?0x0008:(j%30==0)?0x4000:0):0);
  if(i==0)assert(s.progress.started&&!s.progress.practice);
  assert(!load_rom(s,"/tmp/does-not-exist-ff1.nes"));assert(s.loaded);run_frame(s,0);assert(s.frames==count+1);
 }
 assert(test_audio_frames>0&&test_audio_nonzero);unload_rom(s);
 {std::ofstream f(path,std::ios::binary|std::ios::trunc);f<<"bad";}
 assert(!randomizer::select(read,7,path,sizeof(path),error,dir));
 assert(!randomizer::generate(argv[1],0,9,lib,path,sizeof(path),error,dir));
 assert(!randomizer::generate(argv[1],0,0,lib,path,sizeof(path),error,"/missing/ff1-dir"));
 std::filesystem::remove_all(dir);
 puts("PASS: 100 deterministic seeds, changed loot, unchanged progression, preserved loot counts, disk roundtrip, bad/truncated files, 24 core switches with audio, distinct SRAM paths, failed load preserves session");
}
