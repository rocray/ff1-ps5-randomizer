// SPDX-License-Identifier: GPL-3.0-or-later
#include "../src/nes/nes_core.hpp"
#include "../src/demo_renderer.hpp"
#include <cassert>
#include <cstdio>
#include <cstdint>
#include <set>
#include "libretro.h"
extern std::size_t test_audio_frames;extern bool test_audio_nonzero;
int main(int argc,char **argv) {
    assert(argc==2);using namespace ps5::demo::nes;
    NesState s;s.save_path="/tmp/ff1-core-test.srm";std::remove(s.save_path);
    assert(!load_rom(s,"/no/such/rom.nes"));assert(!s.loaded&&!s.rom&&!s.framebuffer);
    assert(load_rom(s,argv[1]));assert(s.rom_size==262160);assert(s.framebuffer);assert(s.fps>59&&s.fps<61);
    retro_system_av_info av{};retro_get_system_av_info(&av);assert(av.timing.sample_rate==48000);
    s.timer_armed=true;
    std::size_t max_colors=0;
    for(int i=0;i<240;i++){run_frame(s,0);std::set<std::uint32_t> c(s.framebuffer,s.framebuffer+256*240);if(c.size()>max_colors)max_colors=c.size();}
    std::printf("max colors %zu final %08x\n",max_colors,s.framebuffer[0]);
    assert(test_audio_frames>100000 && test_audio_nonzero);
    assert(s.frames==240 && s.tracker.seconds>3.9 && s.tracker.seconds<4.1);
    std::set<std::uint32_t> colors(s.framebuffer,s.framebuffer+256*240);assert(max_colors>2);
    for(auto c:colors)assert((c>>24)==255);
    s.running=false;run_frame(s,ps5::demo::pad::cross);assert(s.frames==240);
    s.running=true;run_frame(s,ps5::demo::pad::options);assert(s.frames==241);
    save_ram(s);assert(!s.error[0]);unload_rom(s);assert(!s.loaded&&!s.rom&&!s.framebuffer);
    assert(load_rom(s,argv[1]));run_frame(s,0);unload_rom(s);
    Tracker t;tracker_reset(&t);std::uint8_t items[ITEM_COUNT]{};
    t.seconds=12;items[0]=1;tracker_sample(&t,items);assert(t.count==1&&t.acquired[0]==12);
    items[0]=0;tracker_sample(&t,items);assert(t.seen[0]&&!t.current[0]);
    items[0]=1;tracker_sample(&t,items);assert(t.count==1);
    std::remove(s.save_path);puts("PASS: real FF1 boot/video, input submission, pause, timer, SRAM reload, item history");
}
