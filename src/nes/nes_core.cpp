// FF1 prototype integration. SPDX-License-Identifier: GPL-3.0-or-later
#include "nes_core.hpp"
#include "libretro.h"
#include <cstdio>
#include <cstring>
#include <new>

extern "C" unsigned char ff1_read_cpu(unsigned);
extern "C" void ff1_heap_reset();
extern "C" const char *ff1_heap_error();
extern "C" { int open(const char *, int, ...); long read(int, void *, std::size_t); long lseek(int, long, int); int close(int); }
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
    // Use the same low-level file API as the working background asset loader.
    const int fd=open(filename,0);
    if(fd<0){s.error="ROM OPEN FAILED";return false;}
    const long size=lseek(fd,0,SEEK_END);
    if(size<16 || size>8*1024*1024){close(fd);s.error="INVALID ROM SIZE / SEEK";return false;}
    if(lseek(fd,0,SEEK_SET)<0){close(fd);s.error="ROM SEEK FAILED";return false;}
    s.rom=new(std::nothrow) std::uint8_t[size];
    s.framebuffer=new(std::nothrow) std::uint32_t[256*240]{};
    bool ok=s.rom && s.framebuffer;
    std::size_t offset=0;
    while(ok && offset<static_cast<std::size_t>(size)){
        const long n=read(fd,s.rom+offset,static_cast<std::size_t>(size)-offset);
        if(n<=0){ok=false;break;} offset+=static_cast<std::size_t>(n);
    }
    close(fd);
    FILE *f=nullptr;
    if(!ok || std::memcmp(s.rom,"NES\x1a",4)!=0){unload_rom(s);s.error="INVALID OR UNREADABLE NES ROM";return false;}
    ff1_heap_reset();s.core_message[0]=0;s.rom_size=static_cast<std::size_t>(size);init_core_glue(&s);retro_init();
    retro_game_info info{filename,s.rom,s.rom_size,nullptr};
    if(!retro_load_game(&info)){retro_deinit();unload_rom(s);s.error=ff1_heap_error()?ff1_heap_error():(s.core_message[0]?s.core_message:"CORE REJECTED ROM (NO DETAIL)");return false;}
    owner=&s;s.loaded=s.running=true;s.timer_armed=false;s.frames=0;s.buttons=0;s.error="";
    tracker_reset(&s.tracker);s.progress={};
    s.rom_id=14695981039346656037ull;
    for(std::size_t i=0;i<s.rom_size;i++){s.rom_id^=s.rom[i];s.rom_id*=1099511628211ull;}
    std::snprintf(s.records_path,sizeof(s.records_path),"/download0/ff1-best-%016llx.bin",static_cast<unsigned long long>(s.rom_id));
    load_records(s.records_path,s.rom_id,s.progress.best);
    std::memcpy(s.progress.records,s.progress.best,sizeof(s.progress.best));
    s.records_saved=true;
    retro_system_av_info av{};retro_get_system_av_info(&av);s.fps=av.timing.fps>1?av.timing.fps:60.0988;
    auto *ram=retro_get_memory_data(RETRO_MEMORY_SAVE_RAM);auto n=retro_get_memory_size(RETRO_MEMORY_SAVE_RAM);
    if(ram&&n){f=std::fopen(s.save_path,"rb");if(f){std::fseek(f,0,SEEK_END);long actual=std::ftell(f);std::rewind(f);if(actual==static_cast<long>(n) && std::fread(ram,1,n,f)!=n)s.error="SRAM READ FAILED";std::fclose(f);}}
    return true;
}
void run_frame(NesState &s,std::uint32_t buttons) {
    if(owner!=&s || !s.loaded || !s.running)return;
    s.buttons=buttons;retro_run();++s.frames;
    auto &r=s.progress;
    if(!r.started) {
        unsigned hp=ff1_read_cpu(0x610a)|(ff1_read_cpu(0x610b)<<8);
        if(overworld_ready(ff1_read_cpu(0x42),ff1_read_cpu(0x4b),hp))++r.stable;else r.stable=0;
        if(r.stable>=2){
            r.started=true;s.timer_armed=true;s.tracker.seconds=0;
            for(int i=0;i<ITEM_COUNT;i++)if(ff1_read_cpu(item_addr[i]))r.practice=true;
            for(unsigned i=0;i<4;i++)if(ff1_read_cpu(0x6032+i))r.practice=true;
        }
    }
    if(s.timer_armed && !r.finished){
        s.tracker.seconds+=1.0/s.fps;
        std::uint8_t values[ITEM_COUNT];for(int i=0;i<ITEM_COUNT;i++)values[i]=ff1_read_cpu(item_addr[i]);
        tracker_sample(&s.tracker,values);
        if(r.started){
            bool changed=false;
            for(unsigned i=0;i<3;i++)if(values[16+i])changed|=record_milestone(r,i,s.tracker.seconds);
            const unsigned addresses[]={0x6035,0x6032,0x6033,0x6034};
            for(unsigned i=0;i<4;i++){r.orb[i]=ff1_read_cpu(addresses[i])!=0;if(r.orb[i])changed|=record_milestone(r,3+i,s.tracker.seconds);}
            if(ff1_read_cpu(0x6a)==0x7b && ff1_read_cpu(0x6b86)==0xff){
                changed|=record_milestone(r,7,s.tracker.seconds);r.finished=true;s.timer_armed=false;
            }
            if(changed)s.records_saved=save_records(s.records_path,s.rom_id,r.records);
        }
    }
    if(s.frames%1800==0)save_ram(s);
}
}

