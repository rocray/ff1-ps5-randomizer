#include "../src/nes/nes_core.hpp"
#include "../src/randomizer.hpp"
#include "libretro.h"
#include <fstream>
#include <vector>
#include <cassert>
#include <cstdio>
int main(){std::ifstream f("build/test-FF1.nes",std::ios::binary);std::vector<std::uint8_t> rom((std::istreambuf_iterator<char>(f)),{});assert(randomizer::enable_run(rom.data(),rom.size()));{std::ofstream out("/tmp/ff1-run-test.nes",std::ios::binary);out.write((char*)rom.data(),rom.size());}
 using namespace ps5::demo::nes;NesState s;s.save_path="/tmp/ff1-run-empty.srm";std::remove(s.save_path);assert(load_rom(s,"/tmp/ff1-run-test.nes"));for(unsigned i=0;i<3000&&!s.progress.started;i++)run_frame(s,i%600==0?8:i%30==0?0x4000:0);assert(s.progress.started);
 auto *ram=(std::uint8_t*)retro_get_memory_data(RETRO_MEMORY_SYSTEM_RAM);assert(ram);
 for(unsigned k=0;k<600;k++)run_frame(s,0);
 bool walk=false,run=false;
 for(unsigned k=0;k<60;k++){run_frame(s,0x20);walk|=ram[0x34]==1;}
 for(unsigned k=0;k<60;k++){run_frame(s,0x80|0x2000);run|=ram[0x34]==2;}
 assert(walk&&run);puts("PASS: actual core boots run-enabled ROM, normal walking speed 1 and Circle-held speed 2");
 unload_rom(s);std::remove(s.save_path);std::remove("/tmp/ff1-run-test.nes");}
