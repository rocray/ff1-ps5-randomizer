#include "run_tracking.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <cmath>
#include <cstddef>
namespace {
struct Record {std::uint32_t magic,version;std::uint64_t rom;double times[SPLIT_COUNT];std::uint64_t checksum;};
std::uint64_t checksum(const Record &r){std::uint64_t h=14695981039346656037ull;auto *p=reinterpret_cast<const unsigned char*>(&r);for(unsigned i=0;i<offsetof(Record,checksum);i++){h^=p[i];h*=1099511628211ull;}return h;}
}
bool load_records(const char *path,std::uint64_t rom,double *v){
 int fd=open(path,O_RDONLY);if(fd<0)return false;Record r{};std::size_t offset=0;
 while(offset<sizeof(r)){auto n=read(fd,reinterpret_cast<char*>(&r)+offset,sizeof(r)-offset);if(n<=0)break;offset+=n;}close(fd);
 if(offset!=sizeof(r)||r.magic!=0x5a504231||r.version!=1||r.rom!=rom||r.checksum!=checksum(r))return false;
 for(double t:r.times)if(!std::isfinite(t)||t<0||t>604800)return false;
 std::memcpy(v,r.times,sizeof(r.times));return true;
}
bool save_records(const char *path,std::uint64_t rom,const double *v){
 Record r{};r.magic=0x5a504231;r.version=1;r.rom=rom;std::memcpy(r.times,v,sizeof(r.times));r.checksum=checksum(r);
 int fd=open(path,O_WRONLY|O_CREAT|O_TRUNC,0666);if(fd<0)return false;std::size_t offset=0;
 while(offset<sizeof(r)){auto n=write(fd,reinterpret_cast<const char*>(&r)+offset,sizeof(r)-offset);if(n<=0)break;offset+=n;}
 const bool ok=offset==sizeof(r)&&fsync(fd)==0;close(fd);return ok;
}

