// FF1 prototype integration. SPDX-License-Identifier: GPL-3.0-or-later
#include "nes_core.hpp"
#include "libretro.h"
#include <cstdio>
#include <cstring>
#include <new>

extern "C" unsigned char ff1_read_cpu(unsigned);
namespace ps5::demo::nes {
static NesState *owner=nullptr; // libretro is a single-instance API
void save_ram(NesState &s) {
    if(!s.loaded)return;
    auto *p=retro_get_memory_data(RETRO_MEMORY_SAVE_RAM);
    auto n=retro_get_memory_size(RETRO_MEMORY_SAVE_RAM);
    if(!p || !n)return;
    char tmp[1024];
    if(std::snprintf(tmp,sizeof(tmp),"%s.tmp",s.save_path)>=static_cast<int>(sizeof(tmp))){s.error="SAVE PATH TOO LONG";return;}
    FILE *f=std::fopen(tmp,"wb");
    if(!f){s.error="SRAM SAVE FAILED";return;}
    bool ok=std::fwrite(p,1,n,f)==n;
    if(std::fclose(f)!=0)ok=false;
    if(!ok || std::rename(tmp,s.save_path)!=0)s.error="SRAM SAVE FAILED";
}
void unload_rom(NesState &s) {
    if(owner==&s){save_ram(s);retro_unload_game();retro_deinit();owner=nullptr;}
    delete[] s.rom;delete[] s.framebuffer;s.rom=nullptr;s.framebuffer=nullptr;
    s.rom_size=0;s.loaded=s.running=false;
}
bool load_rom(NesState &s,std::string_view path) {
    if(owner && owner!=&s){s.error="CORE ALREADY IN USE";return false;}
    unload_rom(s);
    char filename[1024];
    if(path.size()>=sizeof(filename)){s.error="ROM PATH TOO LONG";return false;}
    std::memcpy(filename,path.data(),path.size());filename[path.size()]=0;
    FILE *f=std::fopen(filename,"rb");
    if(!f){s.error="ROM NOT FOUND";return false;}
    if(std::fseek(f,0,SEEK_END)!=0){std::fclose(f);s.error="ROM READ FAILED";return false;}
    long size=std::ftell(f);std::rewind(f);
    if(size<16 || size>8*1024*1024){std::fclose(f);s.error="INVALID ROM SIZE";return false;}
    s.rom=new(std::nothrow) std::uint8_t[size];
    s.framebuffer=new(std::nothrow) std::uint32_t[256*240]{};
    bool ok=s.rom && s.framebuffer;
    if(ok)ok=std::fread(s.rom,1,size,f)==static_cast<std::size_t>(size);
    std::fclose(f);
    if(!ok || std::memcmp(s.rom,"NES\x1a",4)!=0){unload_rom(s);s.error="INVALID OR UNREADABLE NES ROM";return false;}
    s.rom_size=static_cast<std::size_t>(size);init_core_glue(&s);retro_init();
    retro_game_info info{filename,s.rom,s.rom_size,nullptr};
    if(!retro_load_game(&info)){retro_deinit();unload_rom(s);s.error="CORE REJECTED ROM";return false;}
    owner=&s;s.loaded=s.running=true;s.timer_armed=false;s.frames=0;s.buttons=0;s.error="";
    tracker_reset(&s.tracker);
    retro_system_av_info av{};retro_get_system_av_info(&av);s.fps=av.timing.fps>1?av.timing.fps:60.0988;
    auto *ram=retro_get_memory_data(RETRO_MEMORY_SAVE_RAM);auto n=retro_get_memory_size(RETRO_MEMORY_SAVE_RAM);
    if(ram&&n){f=std::fopen(s.save_path,"rb");if(f){std::fseek(f,0,SEEK_END);long actual=std::ftell(f);std::rewind(f);if(actual==static_cast<long>(n) && std::fread(ram,1,n,f)!=n)s.error="SRAM READ FAILED";std::fclose(f);}}
    return true;
}
void run_frame(NesState &s,std::uint32_t buttons) {
    if(owner!=&s || !s.loaded || !s.running)return;
    s.buttons=buttons;retro_run();++s.frames;
    if(s.timer_armed){s.tracker.seconds+=1.0/s.fps;std::uint8_t values[ITEM_COUNT];for(int i=0;i<ITEM_COUNT;i++)values[i]=ff1_read_cpu(item_addr[i]);tracker_sample(&s.tracker,values);}
    if(s.frames%1800==0)save_ram(s);
}
}
