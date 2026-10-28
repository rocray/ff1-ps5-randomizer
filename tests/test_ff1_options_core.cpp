#include "../src/randomizer.hpp"
#include "../src/nes/nes_core.hpp"
#include "libretro.h"
#include <vector>
#include <fstream>
#include <cassert>
#include <cstdio>
extern std::size_t test_audio_frames;extern bool test_audio_nonzero;
int main(int argc,char**argv){assert(argc==2);std::ifstream f(argv[1],std::ios::binary);std::vector<unsigned char>base((std::istreambuf_iterator<char>(f)),{}),out(base.size());const char*error="";
 using namespace ps5::demo::nes;NesState s;s.save_path="/tmp/ff1-options-core.srm";
 for(unsigned bits=0;bits<32;bits++){
  ff1logic::Options o{bool(bits&1),bool(bits&2),bool(bits&4),bool(bits&8),bool(bits&16)};
  assert(ff1logic::shuffle(base.data(),base.size(),1234+bits,out.data(),o,error));assert(randomizer::enable_run(out.data(),out.size()));
  {std::ofstream f("/tmp/ff1-options-core.nes",std::ios::binary);f.write((char*)out.data(),out.size());}
  std::remove(s.save_path);assert(load_rom(s,"/tmp/ff1-options-core.nes"));
  for(unsigned n=0;n<2400;n++)run_frame(s,n%600==0?8:n%30==0?0x4000:0);
  assert(s.progress.started&&!s.progress.practice);
  auto*save=(unsigned char*)retro_get_memory_data(RETRO_MEMORY_SAVE_RAM);assert(save);
  for(unsigned id:{6u,7u,10u,13u,17u,18u,19u,187u})assert(!(save[0x200+id]&128));
  assert(!load_rom(s,"/tmp/no-ff1-options.nes")&&s.loaded);unload_rom(s);
 }
 assert(test_audio_frames>100000&&test_audio_nonzero);std::remove(s.save_path);std::remove("/tmp/ff1-options-core.nes");
 puts("PASS: 32 checkbox combinations + run patch boot/new game/automatic timer/audio in FCEUmm; NPC claim bits initially clear; missing ROM preserves session");
}
