// SPDX-License-Identifier: GPL-3.0-or-later
// One atomic envelope holds emulation, ROM, video, clock and tracker history.
#include "nes_core.hpp"
#include "libretro.h"
#include "../randomizer.hpp"
#include "../qol.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <ctime>
#include <cerrno>
namespace ps5::demo::nes {
namespace {
constexpr unsigned magic=0x31535246,version=1,core_version=0xac0d0001;
constexpr std::size_t capacity=2*1024*1024,core_capacity=512*1024,video_size=256*240*4;
struct Header {
 std::uint32_t magic,version,core_version,total,rom_size,core_size,checksum,flags;
 std::uint64_t rom_id;
 std::int64_t timestamp;
 double fps;
 std::uint32_t frames,stable;
 Tracker tracker;
 double split[8],best[8],records[8];
 std::uint8_t reached[8],orb[4];
 char label[96];
};
struct Last {std::uint32_t magic,version,checksum;char path[256];};
// Separate read/write arenas: saving the previous run cannot overwrite an incoming save.
alignas(16) std::uint8_t input[capacity],output[capacity],rollback[core_capacity];
std::uint64_t hash(const std::uint8_t*p,std::size_t n){std::uint64_t h=14695981039346656037ull;while(n--){h^=*p++;h*=1099511628211ull;}return h;}
bool path_for(const NesState&s,char*p,std::size_t n){return std::snprintf(p,n,"%s.resume",s.save_path)<int(n);}
bool read_file(const char*path,void*data,std::size_t cap,std::size_t&size){
 int f=open(path,O_RDONLY);if(f<0)return false;long n=lseek(f,0,SEEK_END);bool ok=n>0&&std::size_t(n)<=cap&&lseek(f,0,SEEK_SET)>=0;size=ok?n:0;
 std::size_t at=0;while(ok&&at<size){auto k=read(f,static_cast<char*>(data)+at,size-at);if(k<=0){ok=false;break;}at+=k;}close(f);return ok;
}
bool write_file(const char*path,const void*data,std::size_t size){
 char tmp[300];if(std::snprintf(tmp,sizeof(tmp),"%s.tmp",path)>=int(sizeof(tmp)))return false;
 int f=open(tmp,O_WRONLY|O_CREAT|O_TRUNC,0666);if(f<0)return false;std::size_t at=0;
 while(at<size){auto n=write(f,static_cast<const char*>(data)+at,size-at);if(n<=0)break;at+=n;}
 bool ok=at==size&&fsync(f)==0;if(close(f)!=0)ok=false;
 if(ok)ok=std::rename(tmp,path)==0;if(!ok)unlink(tmp);return ok;
}
bool time_ok(double t){return std::isfinite(t)&&t>=0&&t<=31536000;}
bool decode(const char*path,Header&h){
 std::size_t size=0;if(!read_file(path,input,sizeof(input),size)||size<sizeof(h))return false;
 std::memcpy(&h,input,sizeof(h));
 if(h.magic!=magic||h.version!=version||h.core_version!=core_version||h.total!=size||h.rom_size>1024*1024||h.core_size>core_capacity||!h.core_size||
    sizeof(h)+h.rom_size+h.core_size+video_size!=size||h.flags&~31u||!std::memchr(h.label,0,sizeof(h.label)))return false;
 auto crc=h.checksum;reinterpret_cast<Header*>(input)->checksum=0;
 if(randomizer::crc32(input,size)!=crc)return false;
 const auto*rom=input+sizeof(h);
 if(!randomizer::validate_nes(rom,h.rom_size)||hash(rom,h.rom_size)!=h.rom_id||!std::isfinite(h.fps)||h.fps<50||h.fps>61)return false;
 const auto&t=h.tracker;if(t.magic!=0x46463154||t.version!=1||!time_ok(t.seconds)||t.count<0||t.count>ITEM_COUNT)return false;
 unsigned seen=0;
 for(int i=0;i<ITEM_COUNT;i++)if(t.seen[i]>1||t.current[i]>1||!time_ok(t.acquired[i])||t.acquired[i]>t.seconds)return false;
 for(int i=0;i<t.count;i++){int k=t.order[i];if(k<0||k>=ITEM_COUNT||(seen&(1u<<k))||!t.seen[k])return false;seen|=1u<<k;}
 for(unsigned i=0;i<8;i++)if(h.reached[i]>1||!time_ok(h.split[i])||h.split[i]>t.seconds||!time_ok(h.best[i])||!time_ok(h.records[i]))return false;
 for(auto v:h.orb)if(v>1)return false;
 return true;
}
bool restore(NesState&s,const Header&h){
 if(s.rom_id!=h.rom_id||s.rom_size!=h.rom_size||retro_serialize_size()!=h.core_size){s.error="RESUME ROM OR CORE VERSION MISMATCH";return false;}
 if(!retro_serialize(rollback,h.core_size)){s.error="CANNOT PROTECT CURRENT STATE";return false;}
 if(!retro_unserialize(input+sizeof(h)+h.rom_size,h.core_size)){
  if(!retro_unserialize(rollback,h.core_size)){s.running=false;s.save_allowed=false;}
  s.error="RESUME CORE STATE REJECTED";return false;
 }
 s.tracker=h.tracker;s.frames=h.frames;s.fps=h.fps;s.buttons=0;
 auto&r=s.progress;r.started=h.flags&1;r.finished=h.flags&2;r.practice=h.flags&4;r.stable=h.stable;
 s.timer_armed=h.flags&8;s.running=h.flags&16;
 std::memcpy(r.split,h.split,sizeof(r.split));std::memcpy(r.best,h.best,sizeof(r.best));std::memcpy(r.records,h.records,sizeof(r.records));
 for(unsigned i=0;i<8;i++)r.reached[i]=h.reached[i];std::memcpy(r.orb,h.orb,sizeof(r.orb));
 // Keep the frozen comparison baseline. Never roll newer persistent records back.
 double disk[8]{};if(load_records(s.records_path,s.rom_id,disk))for(unsigned i=0;i<8;i++)if(disk[i]>0&&(!r.records[i]||disk[i]<r.records[i]))r.records[i]=disk[i];
 s.records_saved=save_records(s.records_path,s.rom_id,r.records);
 std::memcpy(s.run_label,h.label,sizeof(s.run_label));
 std::memcpy(s.framebuffer,input+sizeof(h)+h.rom_size+h.core_size,video_size);
 if(h.timestamp>0)qol::format_utc(h.timestamp,s.saved_at,sizeof(s.saved_at));
 s.snapshot_saved=true;s.save_allowed=true;s.error="";return true;
}
}
bool save_run(NesState&s){
 if(!s.loaded||!s.save_allowed)return false;
 save_ram(s);
 // Never replace an actual run with an idle title screen.
 if(!s.progress.started)return true;
 auto fail=[&](const char*msg){s.snapshot_saved=false;s.error=msg;return false;};
 const auto n=retro_serialize_size();if(!n||n>core_capacity||sizeof(Header)+s.rom_size+n+video_size>capacity)return fail("RESUME STATE SIZE UNSUPPORTED");
 std::memset(output,0,sizeof(Header));auto&h=*reinterpret_cast<Header*>(output);
 h.magic=magic;h.version=version;h.core_version=core_version;h.rom_size=s.rom_size;h.core_size=n;h.total=sizeof(h)+s.rom_size+n+video_size;h.rom_id=s.rom_id;
 h.timestamp=std::time(nullptr);h.fps=s.fps;h.frames=s.frames;h.stable=s.progress.stable;
 h.flags=(s.progress.started?1:0)|(s.progress.finished?2:0)|(s.progress.practice?4:0)|(s.timer_armed?8:0)|(s.running?16:0);
 h.tracker=s.tracker;std::memcpy(h.split,s.progress.split,sizeof(h.split));std::memcpy(h.best,s.progress.best,sizeof(h.best));std::memcpy(h.records,s.progress.records,sizeof(h.records));
 for(unsigned i=0;i<8;i++)h.reached[i]=s.progress.reached[i];std::memcpy(h.orb,s.progress.orb,sizeof(h.orb));std::snprintf(h.label,sizeof(h.label),"%s",s.run_label);
 std::memcpy(output+sizeof(h),s.rom,s.rom_size);
 if(!retro_serialize(output+sizeof(h)+s.rom_size,n))return fail("RESUME SERIALIZATION FAILED");
 std::memcpy(output+sizeof(h)+s.rom_size+n,s.framebuffer,video_size);h.checksum=randomizer::crc32(output,h.total);
 char path[256];if(!path_for(s,path,sizeof(path))||!write_file(path,output,h.total))return fail("EXACT RESUME SAVE FAILED");
 Last last{};last.magic=magic;last.version=version;std::snprintf(last.path,sizeof(last.path),"%s",path);last.checksum=randomizer::crc32(reinterpret_cast<const std::uint8_t*>(&last),sizeof(last));
 char index[300];std::snprintf(index,sizeof(index),"%s",s.save_path);auto*slash=std::strrchr(index,'/');if(slash)slash[1]=0;else index[0]=0;std::strcat(index,"ff1-last-run.bin");
 if(!write_file(index,&last,sizeof(last)))return fail("RUN SAVED / LAST RUN INDEX FAILED");
 s.snapshot_saved=true;s.error="";return true;
}
bool resume_if_available(NesState&s){
 char path[256];if(!path_for(s,path,sizeof(path))){s.error="RESUME PATH TOO LONG";return false;}
 int fd=open(path,O_RDONLY);if(fd<0){if(errno==ENOENT)return true;s.error="CANNOT READ RESUME FILE";return false;}close(fd);
 Header h{};if(!decode(path,h)){s.error="RESUME FILE INVALID / SRAM KEPT";return false;}
 return restore(s,h);
}
bool resume_latest(NesState&s,const char*directory){
 if(s.loaded){s.error="RUN ALREADY ACTIVE";return false;}
 char path[300];std::snprintf(path,sizeof(path),"%s/ff1-last-run.bin",directory);Last last{};std::size_t n=0;
 if(!read_file(path,&last,sizeof(last),n)||n!=sizeof(last)){s.error="NO SAVED RUN / PLAY FIRST";return false;}
 auto crc=last.checksum;last.checksum=0;
 if(last.magic!=magic||last.version!=version||!std::memchr(last.path,0,sizeof(last.path))||randomizer::crc32(reinterpret_cast<const std::uint8_t*>(&last),sizeof(last))!=crc){s.error="LAST RUN INDEX INVALID";return false;}
 Header h{};if(!decode(last.path,h)){s.error="RESUME FILE INVALID / SRAM KEPT";return false;}
 // Embedded ROM removes dependence on library slot replacement or app reinstalls.
 if(!load_rom_data(s,input+sizeof(h),h.rom_size))return false;
 // Preserve the original SRAM destination for this checkpoint (also host-test paths).
 auto len=std::strlen(last.path);if(len<7||std::strcmp(last.path+len-7,".resume")||len-7>=sizeof(s.save_storage)){s.save_allowed=false;s.error="RESUME PATH INVALID";return false;}
 std::memcpy(s.save_storage,last.path,len-7);s.save_storage[len-7]=0;s.save_path=s.save_storage;
 if(!restore(s,h)){s.save_allowed=false;unload_rom(s);return false;}
 return true;
}
}
