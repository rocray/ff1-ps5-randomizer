// SPDX-License-Identifier: GPL-3.0-or-later
// Native Treasure Shuffle v1. This is NOT the upstream 4.9.7 algorithm/seed format.
// Data reference: FiendsOfTheElements/FF1Randomizer ItemLocations.cs and Items.cs,
// e49d72e20f2dcb2c034affddce29f72d54b60d5e. PRG offsets exclude iNES header.
#include "randomizer.hpp"
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
static bool eligible(unsigned i,std::uint8_t item){
 // Preserve unused slots and all quest/orb values. Only existing ordinary loot moves.
 return i!=0&&!(i>=0x91&&i<=0x94)&&!(i>=0xbb&&i<=0xc3)&&i!=0xff&&item>=22;
}
bool shuffle(const std::uint8_t *base,std::size_t size,std::uint32_t seed,std::uint8_t *out,const char *&error){
 if(!out||size!=rom_size||!validate_nes(base,size)||((base[6]>>4)|(base[7]&0xf0))!=1||crc32(base+16,size-16)!=base_crc){error="NEEDS ORIGINAL US FF1 ROM";return false;}
 std::memcpy(out,base,size);unsigned indices[256],count=0;
 for(unsigned i=0;i<256;i++)if(eligible(i,base[0x3110+i]))indices[count++]=i;
 // Fixed 32-bit LCG plus rejection sampling: reproducible on PC and PS5, seed 0 valid.
 auto next=[&](){seed=seed*1664525u+1013904223u;return seed;};
 for(unsigned i=count-1;i>0;i--){unsigned bound=i+1,threshold=(0u-bound)%bound,v;do{v=next();}while(v<threshold);unsigned j=v%bound;auto a=0x3110+indices[i],b=0x3110+indices[j];auto t=out[a];out[a]=out[b];out[b]=t;}
 error="";return true;
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
bool save_options(const ff1logic::Options &options,const char *dir){
 std::uint32_t data[]={0x314f5246,ff1logic::flags(options),0};data[2]=crc32(reinterpret_cast<const std::uint8_t*>(data),8);
 char path[256];std::snprintf(path,sizeof(path),"%s/ff1-randomizer-v1.bin",dir);return write_atomic(path,data,sizeof(data));
}
bool load_options(ff1logic::Options &options,const char *dir){
 options={};std::uint32_t data[3]{};char path[256];std::snprintf(path,sizeof(path),"%s/ff1-randomizer-v1.bin",dir);
 if(!read_exact(path,data,sizeof(data))||data[0]!=0x314f5246||data[2]!=crc32(reinterpret_cast<const std::uint8_t*>(data),8)||!(data[1]&2)||(data[1]&~510u))return false;
 options={bool(data[1]&4),bool(data[1]&8),bool(data[1]&16),bool((data[1]&16)&&(data[1]&32)),bool(data[1]&64)};return true;
}
static std::uint32_t library_crc(const Library &l){return crc32(reinterpret_cast<const std::uint8_t*>(&l),offsetof(Library,checksum));}
static bool rom_path(char *out,std::size_t n,const char *dir,std::uint32_t seed,unsigned flags=0){if(flags&ff1logic::custom)return std::snprintf(out,n,"%s/ff1-c%u-%08x-%02x.nes",dir,(flags&ff1logic::full_magic_flag)?3u:(flags&ff1logic::slot_prices_flag)?2u:1u,seed,flags)<int(n);return std::snprintf(out,n,"%s/ff1-t1-%08x%s.nes",dir,seed,(flags&1)?"-run":"")<int(n);}
bool load_library(Library &l,const char *dir){
 char path[256];std::snprintf(path,sizeof(path),"%s/ff1-seeds-v1.bin",dir);Library candidate{};
 if(read_exact(path,&candidate,sizeof(candidate))&&candidate.magic==0x31525446&&candidate.version==2&&candidate.checksum==library_crc(candidate)){
  for(const auto &e:candidate.entries)if((e.flags&~ff1logic::known_flags)||(e.flags>1&&!(e.flags&ff1logic::custom))){l={};return false;}
  l=candidate;return true;
 }
 struct OldEntry {std::uint32_t seed,crc;};
 struct Legacy {std::uint32_t magic,version;OldEntry entries[slots];std::uint32_t checksum;} old{};
 l={};
 if(!read_exact(path,&old,sizeof(old))||old.magic!=0x31525446||old.version!=1||old.checksum!=crc32(reinterpret_cast<const std::uint8_t*>(&old),offsetof(Legacy,checksum)))return false;
 for(unsigned i=0;i<slots;i++)l.entries[i]={old.entries[i].seed,old.entries[i].crc,0};
 return true; // v1 files remain intact until the next successful generation.
}
// Original US MMC1 bank 0F:$D077. Equivalent to upstream EnableDash's
// slowMapMove/BVS variant: normal speed unless NES B is held. No frame skipping.
bool enable_run(std::uint8_t *rom,std::size_t size){
 static const std::uint8_t original[]={0xa9,1,0x85,0x34,0xa5,0x2d,0x4a,0xb0,9,0xa5,0x42,0xc9,2,0x90,3,0x4a,0x85,0x34,0x60};
 static const std::uint8_t patch[]={0xa5,0x42,0x4a,0x69,0,0x4a,0x69,0,0x0a,0x24,0x20,0x70,1,0x4a,0x85,0x34,0x60};
 constexpr unsigned at=0x3d077+16;
 if(!rom||size!=rom_size||std::memcmp(rom+at,original,sizeof(original)))return false;
 std::memcpy(rom+at,patch,sizeof(patch));return true;
}

bool generate(const char *base,std::uint32_t seed,unsigned slot,Library &l,char *out,std::size_t capacity,const char *&error,const char *dir,bool run,const ff1logic::Options *options){
 if(slot>=slots){error="INVALID SEED SLOT";return false;}
 if(!read_exact(base,base_buffer,rom_size)){error="BASE ROM MISSING OR WRONG SIZE";return false;}
 if(options){if(!ff1logic::shuffle(base_buffer,sizeof(base_buffer),seed,generated,*options,error))return false;}
 else if(!shuffle(base_buffer,sizeof(base_buffer),seed,generated,error))return false;
 const unsigned flags=options?ff1logic::flags(*options,run):(run?1u:0u);
 if(run&&!enable_run(generated,sizeof(generated))){error="RUN PATCH VERSION MISMATCH";return false;}
 if(!rom_path(out,capacity,dir,seed,flags)||!write_atomic(out,generated,sizeof(generated))){error="CANNOT SAVE GENERATED ROM";return false;}
 if(!read_exact(out,base_buffer,sizeof(base_buffer))||std::memcmp(base_buffer,generated,sizeof(generated))){error="GENERATED ROM VERIFY FAILED";return false;}
 Library updated=l;updated.entries[slot]={seed,crc32(generated,sizeof(generated)),flags};updated.checksum=library_crc(updated);
 char path[256];std::snprintf(path,sizeof(path),"%s/ff1-seeds-v1.bin",dir);
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
 return std::snprintf(out,capacity,"%s/ff1-%016llx.srm",dir,static_cast<unsigned long long>(h))<int(capacity);
}
}
