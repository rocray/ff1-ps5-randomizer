#include <cassert>
#include <cstring>
#include <cstddef>
#include <cstdint>
#include "../src/title_music.hpp"
static unsigned offset=0, outputs=0;
static const unsigned char pcm[]={1,2,3,4,5,6,7,8,9,10,11,12};
extern "C" {
int open(const char *,int,...){return 77;}
long lseek(int,long,int whence){offset=0;return whence==2?sizeof(pcm):0;}
long read(int,void *out,std::size_t n){assert(offset+n<=sizeof(pcm));std::memcpy(out,pcm+offset,n);offset+=n;return n;}
int close(int){return 0;}
int sceAudioOutInit(){return 0;}
int sceAudioOutOpen(int user,int type,int,unsigned grain,unsigned rate,unsigned fmt){assert(user==255&&type==0&&grain==1024&&rate==48000&&fmt==1);return 1;}
int sceAudioOutOutput(int,const void *data){auto p=static_cast<const unsigned char *>(data);for(unsigned i=0;i<4096;i++)assert(p[i]==pcm[(outputs*4096+i)%12]);return ++outputs==3?-1:0;}
int sceAudioOutClose(int){return 0;}
int sceKernelUsleep(unsigned){return 0;}
int scePthreadCreate(void **,const void *,void *(*fn)(void *),void *arg,const char *){fn(arg);return 0;}
}
int main(){ps5::demo::music::start();assert(outputs==3);assert(!std::strcmp(ps5::demo::music::status(),"AUDIO OUTPUT UNAVAILABLE"));ps5::demo::music::start();assert(outputs==3);}
