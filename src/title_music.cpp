#include "title_music.hpp"
#include <atomic>
#include <cstddef>
#include <cstdint>
extern "C" {
int open(const char *, int, ...);
long read(int, void *, std::size_t);
long lseek(int, long, int);
int close(int);
int sceAudioOutInit();
int sceAudioOutOpen(int,int,int,unsigned,unsigned,unsigned);
int sceAudioOutOutput(int,const void *);
int sceAudioOutClose(int);
int scePthreadCreate(void **,const void *,void *(*)(void *),void *,const char *);
int sceKernelUsleep(unsigned);
}
namespace ps5::demo::music {
namespace {
std::atomic<bool> active{true};
std::atomic<int> state{0};
void *worker(void *) {
    int fd=open("/app0/assets/prelude.pcm",0);
    if(fd<0) fd=open("/data/homebrew/PPSA01987/assets/prelude.pcm",0);
    if(fd<0){state=2;return nullptr;}
    long size=lseek(fd,0,2);
    if(size<=0 || size%4 || lseek(fd,0,0)<0){close(fd);state=2;return nullptr;}
    sceAudioOutInit();
    // Public SDL PS5 backend: system user, MAIN, signed 16-bit stereo, 48kHz.
    int port=sceAudioOutOpen(0xff,0,0,1024,48000,1);
    if(port<1){close(fd);state=3;return nullptr;}
    state=1;
    alignas(64) std::int16_t samples[2048];
    long position=0;
    for(;;){
        if(!active.load()){sceKernelUsleep(20000);continue;}
        std::size_t filled=0;
        while(filled<sizeof(samples)){
            if(position==size){if(lseek(fd,0,0)<0)goto failed;position=0;}
            std::size_t count=sizeof(samples)-filled;
            if(count>static_cast<std::size_t>(size-position))count=size-position;
            long n=read(fd,reinterpret_cast<char *>(samples)+filled,count);
            if(n<=0)goto failed;
            filled+=n;position+=n;
        }
        if(sceAudioOutOutput(port,samples)<0)goto failed;
    }
failed:
    sceAudioOutClose(port);close(fd);state=3;return nullptr;
}
}
void start() noexcept {
    static bool started=false;
    if(started)return;
    started=true;
    static void *thread=nullptr;
    if(scePthreadCreate(&thread,nullptr,worker,nullptr,"ff1-prelude")!=0)state=3;
}
void set_active(bool value) noexcept {active=value;}
const char *status() noexcept {
    switch(state.load()){
    case 1:return "PRELUDE READY";
    case 2:return "MUSIC FILE UNAVAILABLE";
    case 3:return "AUDIO OUTPUT UNAVAILABLE";
    default:return "STARTING AUDIO";
    }
}
}
