// SPDX-License-Identifier: GPL-3.0-or-later
#include "qol.hpp"
#include "randomizer.hpp"
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
namespace qol {
namespace {
bool read_file(const char *path,void *out,std::size_t size){int f=open(path,O_RDONLY);if(f<0)return false;bool ok=lseek(f,0,SEEK_END)==static_cast<long>(size)&&lseek(f,0,SEEK_SET)==0;std::size_t n=0;while(ok&&n<size){auto r=read(f,static_cast<char*>(out)+n,size-n);if(r<=0)ok=false;else n+=r;}close(f);return ok;}
bool write_file(const char *path,const void *data,std::size_t size){char temp[256];if(std::snprintf(temp,sizeof(temp),"%s.tmp",path)>=int(sizeof(temp)))return false;int f=open(temp,O_WRONLY|O_CREAT|O_TRUNC,0666);if(f<0)return false;std::size_t n=0;while(n<size){auto r=write(f,static_cast<const char*>(data)+n,size-n);if(r<=0)break;n+=r;}bool ok=n==size&&fsync(f)==0;if(close(f)!=0)ok=false;if(ok)ok=std::rename(temp,path)==0;if(!ok)unlink(temp);return ok;}
struct Stamp {std::uint32_t magic=0x31534d54,version=1;std::uint64_t seconds=0;std::uint32_t checksum=0,reserved=0;};
}
bool load(Settings &s,const char *path){Settings v;if(!read_file(path,&v,sizeof(v))||v.magic!=0x314c4f51||v.layout>3||v.scanlines>2||v.curve>1||v.run>1||v.music>1||v.checksum!=randomizer::crc32(reinterpret_cast<const std::uint8_t*>(&v),offsetof(Settings,checksum))){s={};return false;}s=v;return true;}
bool save(Settings s,const char *path){s.checksum=randomizer::crc32(reinterpret_cast<const std::uint8_t*>(&s),offsetof(Settings,checksum));return write_file(path,&s,sizeof(s));}
void format_utc(std::uint64_t seconds,char *out,std::size_t n){
 if(seconds<946684800ull||seconds>4102444799ull){std::snprintf(out,n,"NO TIMESTAMP");return;}
 // Gregorian calendar, UTC; avoids depending on the runtime shim's timezone APIs.
 unsigned days=seconds/86400,year=1970;
 auto leap=[](unsigned y){return y%4==0&&(y%100!=0||y%400==0);};
 while(days>=365u+leap(year)){days-=365u+leap(year);++year;}
 const unsigned months[]={31,28,31,30,31,30,31,31,30,31,30,31};unsigned month=0;
 while(days>=months[month]+(month==1&&leap(year))){days-=months[month]+(month==1&&leap(year));++month;}
 std::snprintf(out,n,"%04u-%02u-%02u %02u:%02u UTC",year,month+1,days+1,unsigned(seconds/3600%24),unsigned(seconds/60%60));
}
bool stamp_save(const char *path){timespec ts{};if(clock_gettime(CLOCK_REALTIME,&ts)||ts.tv_sec<946684800)return false;Stamp s;s.seconds=ts.tv_sec;s.checksum=randomizer::crc32(reinterpret_cast<const std::uint8_t*>(&s),offsetof(Stamp,checksum));char meta[256];if(std::snprintf(meta,sizeof(meta),"%s.time",path)>=int(sizeof(meta)))return false;return write_file(meta,&s,sizeof(s));}
void read_stamp(const char *path,char *out,std::size_t n){Stamp s;char meta[256];std::snprintf(meta,sizeof(meta),"%s.time",path);if(!read_file(meta,&s,sizeof(s))||s.magic!=0x31534d54||s.version!=1||s.checksum!=randomizer::crc32(reinterpret_cast<const std::uint8_t*>(&s),offsetof(Stamp,checksum)))s.seconds=0;format_utc(s.seconds,out,n);}
const std::uint32_t *curve_frame(const std::uint32_t *src){
 static std::uint16_t map[256*240];static std::uint32_t dst[256*240];static bool ready=false;
 if(!ready){for(unsigned y=0;y<240;y++)for(unsigned x=0;x<256;x++){
  const double u=(x+0.5)/128.0-1,v=(y+0.5)/120.0-1;
  const double sx=(u*(1+0.09*v*v)+1)*128,sy=(v*(1+0.09*u*u)+1)*120;
  map[y*256+x]=(sx<0||sx>=256||sy<0||sy>=240)?65535:unsigned(sy)*256+unsigned(sx);
 }ready=true;}
 for(unsigned i=0;i<256*240;i++)dst[i]=map[i]==65535?0xff000000:src[map[i]];
 return dst;
}
}
