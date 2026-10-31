#pragma once
#include <cstdint>
#include <cstring>
constexpr unsigned SPLIT_COUNT=9;
inline const char *split_names[]={"TRIFORCE 1","TRIFORCE 2","TRIFORCE 3","TRIFORCE 4","TRIFORCE 5","TRIFORCE 6","TRIFORCE 7","TRIFORCE 8","ZELDA"};
inline const char *item_names[]={"SWORD","BOW","ARROWS","CANDLE","RECORDER","WAND","RAFT","LADDER","BOOK","RING","MAGIC KEY","BRACELET","LETTER","BOOMERANG","MAGIC BOOM","SHIELD"};
inline constexpr unsigned item_addr[]={0x657,0x65a,0x659,0x65b,0x65c,0x65f,0x660,0x663,0x661,0x662,0x664,0x665,0x666,0x674,0x675,0x676};
struct RunProgress{
 bool started=false,finished=false,practice=false;unsigned stable=0,level=0,mode=0,quest=0,triforce=0,visited=0,hearts=0,rupees=0,keys=0,bombs=0;
 double seconds=0,split[9]{},best[9]{},records[9]{};bool reached[9]{};unsigned char items[16]{},seen[16]{};int found_level[16]{};
};
inline bool record_milestone(RunProgress&r,unsigned i){if(r.reached[i])return false;r.reached[i]=true;r.split[i]=r.seconds;if(!r.practice&&r.seconds>0&&(!r.records[i]||r.seconds<r.records[i])){r.records[i]=r.seconds;return true;}return false;}
// aldonunez/zelda1-disassembly Variables.inc / mode table. Verify each ROM revision.
inline bool sample_run(RunProgress&r,const std::uint8_t*m,double fps){
 r.mode=m[0x12];r.level=m[0x10];r.triforce=m[0x671];r.hearts=(m[0x66f]>>4)+1;r.rupees=m[0x66d];r.keys=m[0x66e];r.bombs=m[0x658];r.quest=m[0x62d+(m[0x16]<3?m[0x16]:0)];
 for(unsigned i=0;i<16;i++)r.items[i]=m[item_addr[i]];
 if(!r.started){r.stable=(r.mode==5&&m[0x11]&&m[0x66f]>=0x20)?r.stable+1:0;if(r.stable<2)return false;r.started=true;r.practice=r.triforce||r.quest;for(unsigned i=0;i<16;i++){r.seen[i]=r.items[i];r.found_level[i]=-1;if(r.items[i])r.practice=true;}}
 if(r.finished)return false;r.seconds+=1.0/(fps>1?fps:60.0988);
 if(r.mode==5&&r.level>=1&&r.level<=9)r.visited|=1u<<(r.level-1);
 for(unsigned i=0;i<16;i++)if(r.items[i]>r.seen[i]){r.seen[i]=r.items[i];r.found_level[i]=r.level>=1&&r.level<=9?r.level:0;}
 bool changed=false;for(unsigned i=0;i<8;i++)if(r.triforce&(1u<<i))changed|=record_milestone(r,i);
 if(r.mode==0x13){changed|=record_milestone(r,8);r.finished=true;}return changed;
}
bool load_records(const char*,std::uint64_t,double*);
bool save_records(const char*,std::uint64_t,const double*);
