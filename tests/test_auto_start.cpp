#include "../src/nes/nes_core.hpp"
#include "../src/demo_renderer.hpp"
#include <cstdio>
extern "C" unsigned char ff1_read_cpu(unsigned);
int main(int argc,char **argv){if(argc!=2)return 4;using namespace ps5::demo::nes;NesState s;s.save_path="/tmp/ff1-new-empty.srm";std::remove(s.save_path);if(!load_rom(s,argv[1]))return 1;
 for(unsigned i=0;i<24000;i++){
 unsigned b=0;
 // Start skips story; confirm default party and names with A.
 if(i%30==0)b=ps5::demo::pad::cross;
 if(i%600==0)b=ps5::demo::pad::options;
 run_frame(s,b);
 if(s.progress.started){std::printf("AUTO START frame=%u time=%f practice=%d vehicle=%u music=%u\n",i,s.tracker.seconds,s.progress.practice,ff1_read_cpu(0x42),ff1_read_cpu(0x4b));return s.progress.practice?3:0;}
 }
 std::printf("NOT STARTED vehicle=%u music=%u hp=%u\n",ff1_read_cpu(0x42),ff1_read_cpu(0x4b),ff1_read_cpu(0x610a));return 2;
}

