#include "../src/nes/run_tracking.hpp"
#include "../src/audio_queue.hpp"
#include <cassert>
#include <cstdio>
int main(){
 assert(!overworld_ready(0,0x44,30));assert(!overworld_ready(1,0x41,30));assert(!overworld_ready(1,0x44,0));assert(overworld_ready(1,0xc4,30));
 RunProgress r;assert(record_milestone(r,0,120));assert(!record_milestone(r,0,130));assert(r.records[0]==120);
 r.reached[0]=false;assert(!record_milestone(r,0,140));assert(r.records[0]==120);
 r.reached[0]=false;assert(record_milestone(r,0,100));assert(r.records[0]==100);
 r.practice=true;assert(!record_milestone(r,1,5));assert(!r.records[1]);
 const char *path="/tmp/ff1-records-test.bin";assert(save_records(path,42,r.records));double times[8]{};assert(load_records(path,42,times));assert(times[0]==100);assert(!load_records(path,43,times));
 auto *f=std::fopen(path,"wb");assert(f);std::fputs("bad",f);std::fclose(f);assert(!load_records(path,42,times));std::remove(path);
 AudioQueue q;std::int16_t in[]={100,-100,200,-200,300,-300},out[8];q.push(in,3);assert(q.pop(out,4)==3);for(int i=0;i<6;i++)assert(in[i]==out[i]);assert(!out[6]&&!out[7]);
 for(unsigned i=0;i<AudioQueue::capacity+3;i++)q.push(in,1);assert(q.count==AudioQueue::capacity);q.clear();assert(q.pop(out,1)==0);
 puts("PASS: timing predicate, best splits, practice, records reload/corruption/ROM isolation, stereo queue");
}

