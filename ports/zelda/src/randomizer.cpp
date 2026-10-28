// SPDX-License-Identifier: GPL-3.0-or-later
// Native Zelda Dungeon Gear Shuffle v1; not the full tournament randomizer.
#include "randomizer.hpp"
#include "logic/item_logic.hpp"
#include <cstring>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
namespace randomizer {
std::uint32_t crc32(const std::uint8_t *data,std::size_t n){
 std::uint32_t c=~0u;for(std::size_t i=0;i<n;i++){c^=data[i];for(unsigned j=0;j<8;j++)c=(c>>1)^(0xedb88320u&(0u-(c&1)));}return ~c;
}
bool validate_nes(const std::uint8_t *p,std::size_t n){
 if(!p||n<16||std::memcmp(p,"NES\x1a",4)||p[6]&4)return false;
 unsigned mapper=(p[6]>>4)|(p[7]&0xf0);
 if((p[7]&12)==8){if(p[8]||p[9])return false;} // no extended mapper/size in this prototype
 else if((p[7]&12)!=0)return false;
 const std::size_t expected=16+std::size_t(p[4])*16384+std::size_t(p[5])*8192;
 return (mapper==1||mapper==4)&&p[4]&&expected==n&&n<=1024*1024;
}
// Six optional combat gear fields, US Rev 1. High room flags are preserved.
// Layout reference: tetraly/zelda-randomizer room.py/data_table.py (aa507c7).
bool shuffle(const std::uint8_t *base,std::size_t size,std::uint32_t seed,std::uint8_t *out,const char *&error){
 if(!out||size!=rom_size||!validate_nes(base,size)||crc32(base+16,size-16)!=base_crc){error="NEEDS ORIGINAL US REV 1 ZELDA ROM";return false;}
 constexpr unsigned at[]={0x18954,0x1895f,0x18985,0x18c10,0x18c5a,0x18c7f};
 constexpr std::uint8_t expected[]={0x1d,0x1e,0x10,0x13,0x07,0x11};
 std::uint8_t items[6];for(unsigned i=0;i<6;i++){if((base[at[i]]&31)!=expected[i]){error="GEAR TABLE MISMATCH";return false;}items[i]=expected[i];}
 auto next=[&](){seed=seed*1664525u+1013904223u;return seed;};
 for(unsigned i=5;i>0;i--){unsigned j=next()%(i+1);auto t=items[i];items[i]=items[j];items[j]=t;}
 if(!std::memcmp(items,expected,6)){auto t=items[0];items[0]=items[1];items[1]=t;}
 std::memcpy(out,base,size);for(unsigned i=0;i<6;i++)out[at[i]]=(out[at[i]]&0xe0)|items[i];error="";return true;
}
static std::uint8_t base_buffer[rom_size],generated[rom_size];
static bool read_exact(const char *path,void *out,std::size_t size){
 int f=open(path,O_RDONLY);if(f<0)return false;
 bool ok=lseek(f,0,SEEK_END)==static_cast<long>(size)&&lseek(f,0,SEEK_SET)==0;std::size_t done=0;
 while(ok&&done<size){auto n=read(f,static_cast<char*>(out)+done,size-done);if(n<=0)ok=false;else done+=n;}close(f);return ok;
}
static bool write_atomic(const char *path,const void *bytes,std::size_t size){
 char tmp[256];if(std::snprintf(tmp,sizeof(tmp),"%s.tmp",path)>=int(sizeof(tmp)))return false;
 int f=open(tmp,O_WRONLY|O_CREAT|O_TRUNC,0666);if(f<0)return false;
 std::size_t done=0;while(done<size){auto n=write(f,static_cast<const char*>(bytes)+done,size-done);if(n<=0)break;done+=n;}
 bool ok=done==size&&fsync(f)==0;if(close(f)!=0)ok=false;
 if(ok)ok=std::rename(tmp,path)==0;if(!ok)unlink(tmp);return ok;
}
static std::uint32_t library_crc(const Library &l){return crc32(reinterpret_cast<const std::uint8_t*>(&l),offsetof(Library,checksum));}
static bool rom_path(char *out,std::size_t n,const char *dir,std::uint32_t seed,unsigned flags=0){return std::snprintf(out,n,"%s/z1-%s-%08x%s.nes",dir,flags==2?"a1":"g1",seed,flags==1?"-run":"")<int(n);}
bool load_library(Library &l,const char *dir){
 char path[256];std::snprintf(path,sizeof(path),"%s/z1-seeds-v1.bin",dir);Library candidate{};
 if(read_exact(path,&candidate,sizeof(candidate))&&candidate.magic==0x31525a4c&&candidate.version==2&&candidate.checksum==library_crc(candidate)){
  for(const auto &e:candidate.entries)if(e.flags>2){l={};return false;}
  l=candidate;return true;
 }
 struct OldEntry {std::uint32_t seed,crc;};
 struct Legacy {std::uint32_t magic,version;OldEntry entries[slots];std::uint32_t checksum;} old{};
 l={};
 if(!read_exact(path,&old,sizeof(old))||old.magic!=0x31525a4c||old.version!=1||old.checksum!=crc32(reinterpret_cast<const std::uint8_t*>(&old),offsetof(Legacy,checksum)))return false;
 for(unsigned i=0;i<slots;i++)l.entries[i]={old.entries[i].seed,old.entries[i].crc,0};
 return true; // v1 files remain intact until the next successful generation.
}
bool enable_run(std::uint8_t*,std::size_t){return false;}
bool generate(const char *base,std::uint32_t seed,unsigned slot,Library &l,char *out,std::size_t capacity,const char *&error,const char *dir,Mode mode){
 unsigned flags=static_cast<unsigned>(mode);if(flags!=0&&flags!=2){error="UNKNOWN RANDOMIZER MODE";return false;}
 if(slot>=slots){error="INVALID SEED SLOT";return false;}
 if(!read_exact(base,base_buffer,rom_size)){error="BASE ROM MISSING OR WRONG SIZE";return false;}
 if(mode==Mode::AllItems){if(!zlogic::shuffle(base_buffer,sizeof(base_buffer),seed,generated,error))return false;}
 else if(!shuffle(base_buffer,sizeof(base_buffer),seed,generated,error))return false;
 if(!rom_path(out,capacity,dir,seed,flags)||!write_atomic(out,generated,sizeof(generated))){error="CANNOT SAVE GENERATED ROM";return false;}
 if(!read_exact(out,base_buffer,sizeof(base_buffer))||std::memcmp(base_buffer,generated,sizeof(generated))){error="GENERATED ROM VERIFY FAILED";return false;}
 Library updated=l;updated.entries[slot]={seed,crc32(generated,sizeof(generated)),flags};updated.checksum=library_crc(updated);
 char path[256];std::snprintf(path,sizeof(path),"%s/z1-seeds-v1.bin",dir);
 if(!write_atomic(path,&updated,sizeof(updated))){error="CANNOT SAVE SEED LIBRARY";return false;}
 l=updated;error="";return true;
}
bool select(const Library &l,unsigned slot,char *out,std::size_t capacity,const char *&error,const char *dir){
 if(slot>=slots||!l.entries[slot].crc){error="EMPTY SEED SLOT";return false;}
 const auto &e=l.entries[slot];
 if(!rom_path(out,capacity,dir,e.seed,e.flags)||!read_exact(out,generated,sizeof(generated))){error="SAVED ROM MISSING OR TRUNCATED";return false;}
 if(!validate_nes(generated,sizeof(generated))||crc32(generated,sizeof(generated))!=e.crc){error="SAVED ROM CHECKSUM FAILED";return false;}
 error="";return true;
}
bool save_path(const Library &l,unsigned slot,char *out,std::size_t capacity,const char *dir){
 char path[256];const char *error="";if(!select(l,slot,path,sizeof(path),error,dir))return false;
 std::uint64_t h=14695981039346656037ull;for(auto b:generated){h^=b;h*=1099511628211ull;}
 return std::snprintf(out,capacity,"%s/z1-%016llx.srm",dir,static_cast<unsigned long long>(h))<int(capacity);
}
}
