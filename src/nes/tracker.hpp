#ifndef TRACKER_H
#define TRACKER_H
#include <stdint.h>
#include <string.h>
#define ITEM_COUNT 19
static const char *item_names[ITEM_COUNT]={"LUTE","CROWN","CRYSTAL","HERB","KEY","TNT","ADAMANT","SLAB","RUBY","ROD","FLOATER","CHIME","TAIL","CUBE","BOTTLE","OXYALE","SHIP","CANOE","AIRSHIP"};
static const unsigned item_addr[ITEM_COUNT]={0x6021,0x6022,0x6023,0x6024,0x6025,0x6026,0x6027,0x6028,0x6029,0x602a,0x602b,0x602c,0x602d,0x602e,0x602f,0x6030,0x6000,0x6012,0x6004};
typedef struct {uint32_t magic,version; double seconds; uint8_t seen[ITEM_COUNT],current[ITEM_COUNT]; double acquired[ITEM_COUNT]; int order[ITEM_COUNT],count;} Tracker;
static void tracker_reset(Tracker *t){memset(t,0,sizeof(*t));t->magic=0x46463154;t->version=1;}
static void tracker_sample(Tracker *t,const uint8_t values[ITEM_COUNT]){for(int i=0;i<ITEM_COUNT;i++){t->current[i]=values[i]!=0;if(values[i]&&!t->seen[i]){t->seen[i]=1;t->acquired[i]=t->seconds;if(t->count<ITEM_COUNT)t->order[t->count++]=i;}}}
#endif
