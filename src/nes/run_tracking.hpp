#pragma once
#include <cstdint>
#include <cstring>
constexpr unsigned SPLIT_COUNT=8;
inline const char *split_names[SPLIT_COUNT]={"SHIP","CANOE","AIRSHIP","EARTH ORB","FIRE ORB","WATER ORB","AIR ORB","CHAOS"};
struct RunProgress {
 bool started=false,finished=false,practice=false;
 unsigned stable=0;
 double split[SPLIT_COUNT]{};
 bool reached[SPLIT_COUNT]{};
 double best[SPLIT_COUNT]{}; // baseline frozen when run loads
 double records[SPLIT_COUNT]{};
 unsigned char orb[4]{};
};
inline bool overworld_ready(unsigned vehicle,unsigned music,unsigned hp) {
 return (vehicle==1||vehicle==2||vehicle==4||vehicle==8) &&
     ((music&0x7f)==0x44||(music&0x7f)==0x45||(music&0x7f)==0x46) && hp>0 && hp<1000;
}
inline bool record_milestone(RunProgress &r,unsigned i,double seconds) {
 if(i>=SPLIT_COUNT||r.reached[i])return false;
 r.reached[i]=true;r.split[i]=seconds;
 if(!r.practice && seconds>0 && (!r.records[i]||seconds<r.records[i])){r.records[i]=seconds;return true;}
 return false;
}
bool load_records(const char *path,std::uint64_t rom,double values[SPLIT_COUNT]);
bool save_records(const char *path,std::uint64_t rom,const double values[SPLIT_COUNT]);
