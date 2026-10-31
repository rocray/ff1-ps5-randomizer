#include "../src/nes/nes_core.hpp"
#include "../src/demo_renderer.hpp"
#include "libretro.h"
#include <cassert>
#include <cstring>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <vector>
#include <cmath>
using namespace ps5::demo::nes;
static std::vector<unsigned char> bytes(const char*p){std::ifstream f(p,std::ios::binary);return {(std::istreambuf_iterator<char>(f)),{}};}
int main(int argc,char**argv){
 assert(argc==2);const char*dir="/tmp/ff1-snapshot-test";std::filesystem::remove_all(dir);std::filesystem::create_directories(dir);
 const char*save="/tmp/ff1-snapshot-test/run.srm";const char*snap="/tmp/ff1-snapshot-test/run.srm.resume";
 NesState s;s.save_path=save;assert(load_rom(s,argv[1]));
 // An idle title screen must not replace any real checkpoint.
 assert(save_run(s));assert(!std::filesystem::exists(snap));
 for(unsigned i=0;i<2400;i++)run_frame(s,(i%600<2?ps5::demo::pad::options:0)|(i%30<2?ps5::demo::pad::cross:0));
 assert(s.progress.started&&!s.progress.practice);
 auto*ram=static_cast<unsigned char*>(retro_get_memory_data(RETRO_MEMORY_SAVE_RAM));auto size=retro_get_memory_size(RETRO_MEMORY_SAVE_RAM);
 ram[0]=1;ram[0x2a]=1;run_frame(s,0);ram[0x2a]=0;run_frame(s,0);
 assert(s.tracker.seen[9]&&!s.tracker.current[9]);assert(s.progress.reached[0]);
 s.progress.best[0]=120;s.progress.best[1]=240;s.running=false;
 std::strcpy(s.run_label,"CUSTOM V2 / SNAPSHOT TEST");
 assert(save_run(s));assert(s.snapshot_saved);
 auto original=bytes(snap);assert(original.size()>s.rom_size);
 Tracker tracker=s.tracker;RunProgress progress=s.progress;unsigned frames=s.frames;
 std::vector<unsigned char>saved_ram(ram,ram+size);std::vector<unsigned>video(s.framebuffer,s.framebuffer+256*240);
 // Build a reference future from exactly this checkpoint.
 s.running=true;for(int i=0;i<90;i++)run_frame(s,i<40?ps5::demo::pad::right:0);
 std::vector<unsigned char>future_ram(ram,ram+size);std::vector<unsigned>future_video(s.framebuffer,s.framebuffer+256*240);double future_time=s.tracker.seconds;
 // Damaged checkpoints must be rejected before mutating the active core/tracker.
 auto damaged=original;damaged[damaged.size()/2]^=0x40;
 {std::ofstream f(snap,std::ios::binary);f.write((char*)damaged.data(),damaged.size());}
 assert(!resume_if_available(s));assert(s.tracker.seconds==future_time);assert(!std::memcmp(ram,future_ram.data(),size));
 {std::ofstream f(snap,std::ios::binary);f.write((char*)original.data(),original.size());}
 assert(resume_if_available(s));assert(!s.running);assert(s.frames==frames);assert(!std::memcmp(&s.tracker,&tracker,sizeof(tracker)));
 assert(!std::memcmp(s.progress.best,progress.best,sizeof(progress.best)));assert(s.progress.split[0]==progress.split[0]);
 assert(!std::memcmp(s.framebuffer,video.data(),256*240*4));assert(!std::memcmp(ram,saved_ram.data(),size));
 run_frame(s,0);assert(s.frames==frames&&s.tracker.seconds==tracker.seconds);
 // Unload/reinitialize is a true cold core restart, not an in-memory Continue.
 unload_rom(s);NesState cold;assert(resume_latest(cold,dir));
 assert(!cold.running&&cold.frames==frames&&cold.timer_armed);
 assert(!std::memcmp(&cold.tracker,&tracker,sizeof(tracker)));assert(!std::memcmp(cold.progress.best,progress.best,sizeof(progress.best)));
 assert(cold.progress.reached[0]&&cold.progress.split[0]==progress.split[0]);assert(!cold.progress.practice);
 assert(!std::strcmp(cold.run_label,"CUSTOM V2 / SNAPSHOT TEST"));assert(!std::memcmp(cold.framebuffer,video.data(),256*240*4));
 ram=static_cast<unsigned char*>(retro_get_memory_data(RETRO_MEMORY_SAVE_RAM));assert(!std::memcmp(ram,saved_ram.data(),size));
 cold.running=true;for(int i=0;i<90;i++)run_frame(cold,i<40?ps5::demo::pad::right:0);
 assert(!std::memcmp(ram,future_ram.data(),size));assert(!std::memcmp(cold.framebuffer,future_video.data(),256*240*4));assert(cold.tracker.seconds==future_time);
 // Practice and completion flags persist, without restarting a stopped timer.
 cold.progress.practice=true;cold.progress.finished=true;cold.timer_armed=false;assert(save_run(cold));unload_rom(cold);
 NesState done;assert(resume_latest(done,dir));assert(done.progress.practice&&done.progress.finished&&!done.timer_armed);
 auto time=done.tracker.seconds;run_frame(done,0);assert(done.tracker.seconds==time);
 // A valid checkpoint for a different ROM cannot be applied to the active ROM.
 assert(save_run(done));done.save_allowed=false;unload_rom(done);
 auto other=bytes(argv[1]);other[0x3110]^=1;NesState wrong;wrong.save_path=save;
 assert(load_rom_data(wrong,other.data(),other.size()));auto wrong_id=wrong.rom_id;
 assert(!resume_if_available(wrong)&&wrong.loaded&&wrong.rom_id==wrong_id);wrong.save_allowed=false;unload_rom(wrong);
 assert(resume_latest(done,dir));
 // Save failure is visible and leaves the prior checkpoint intact.
 done.save_path="/no/such/directory/state.srm";assert(!save_run(done)&&!done.snapshot_saved);done.save_allowed=false;unload_rom(done);
 auto complete=bytes(snap);complete.resize(40);{std::ofstream f(snap,std::ios::binary);f.write((char*)complete.data(),complete.size());}
 NesState broken;assert(!resume_latest(broken,dir)&&!broken.loaded);
 std::filesystem::remove_all(dir);
 puts("PASS: cold exact resume, matching frame/RAM/clock/splits/items/baseline, consumed items, paused/practice/finished flags, deterministic replay, corruption/truncation and save failures");
}
