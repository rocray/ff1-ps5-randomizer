// SPDX-License-Identifier: GPL-3.0-or-later
// Native first-quest, glitchless item-logic profile for original US Rev1.
// Data/room references: tetraly/zelda-randomizer aa507c7, GPL-3.0;
// aldonunez/zelda1-disassembly 50a1c86. No external engine is run on PS5.
#include "item_logic.hpp"
#include "overworld.hpp"
#include "../randomizer.hpp"
#include <cstring>
#include <initializer_list>
namespace zlogic {
namespace {
constexpr unsigned head=16;
constexpr unsigned sword=1,white=2,magic=3,bait=4,recorder=5,blue=6,red=7,arrow=8,silver=9,bow=10,mkey=11,raft=12,ladder=13,wand=16,heart=26,triforce=27;
constexpr int delta[]={-16,1,16,-1}; // N,E,S,W
unsigned grid(unsigned level){return (level<7?0x18700:0x18a00)+head;}
unsigned info(unsigned level){return 0x19300+head+level*0xfc;}
struct Room {
 const std::uint8_t*r;unsigned level,n;
 unsigned byte(unsigned table)const{return r[grid(level)+table*128+n];}
 unsigned type()const{return byte(3)&63;}
 unsigned enemy()const{return (byte(2)&63)|((byte(3)&128)?64:0);}
 unsigned item()const{return byte(4)&31;}
 bool drop()const{return (byte(5)&5)==5;}
 unsigned wall(unsigned d)const{return (byte(d==1||d==3?1:0)>>(d==0||d==3?5:2))&7;}
 bool water()const{unsigned t=type();return t==0x13||t==0x14||t==0x16||t==0x18||t==0x19;}
 bool push()const{switch(type()){case 1:case 6:case 7:case 8:case 9:case 10:case 12:case 13:case 17:case 31:case 34:return (byte(3)&64)!=0;default:return false;}}
 bool stairs()const{if(type()>=26&&type()<=28)return true;for(unsigned d=0;d<4;d++)if(wall(d)==7)return false;return push();}
};
bool major(unsigned i){return (i>=1&&i<=3)||(i>=5&&i<=13)||(i>=16&&i<=21)||i==26||i==28||i==29||i==30;}
struct Inv {
 std::uint32_t bits=0;unsigned hearts=3,tri=0;bool bombs=false,keys=false,food=false,ganon=false,zelda=false;
 bool has(unsigned i)const{return bits&(1u<<i);}
 bool sword()const{return has(1)||has(2)||has(3);}
 bool weapon()const{return sword()||has(wand)||has(red);}
 bool arrows()const{return has(bow)&&(has(arrow)||has(silver));}
 bool candle()const{return has(blue)||has(red);}
 void add(unsigned i,unsigned l){if(i==heart){if(hearts<16)hearts++;}else if(i==triforce&&l>=1&&l<=8)tri|=1u<<(l-1);else if(i<32)bits|=1u<<i;}
};
bool kill(const Room&r,const Inv&i){unsigned e=r.enemy();
 if(e==0||e==0x2b||e==0x49||e==0x4a||(e>=0x4b&&e<=0x52)||e==0x37)return true;
 if(e==0x3e)return i.sword()&&i.has(bow)&&i.has(silver);
 if(e==0x36)return i.food;
 if(e==0x31||e==0x32)return i.bombs;
 if(e==0x33||e==0x34)return i.arrows();
 if(e==0x38||e==0x39)return i.has(recorder)&&i.weapon();
 if(e==0x23||e==0x24||e==0x71||e==0x77||e==0x7b||e==0x7c)return i.sword();
 if((e>=0x42&&e<=0x45)||e==0x47||e==0x48)return i.sword()||i.has(wand);
 if(e==0x16||e==0x70||e==0x78)return i.sword()||i.has(wand)||i.arrows();
 if(e==0x14||e==0x15||e==0x1b||e==0x1c||e==0x1d||e==0x6e)return i.weapon()||i.has(29)||i.has(30);
 return i.weapon();
}
bool travel(const Room&r,unsigned entry,unsigned exit,const Inv&i){
 unsigned t=r.type();
 if(t==14&&(entry!=0&&entry!=2||exit!=0&&exit!=2))return false;
 if(t==15&&(entry!=1&&entry!=3||exit!=1&&exit!=3))return false;
 if(t==18&&(entry==2||exit==2))return false;
 if(!i.has(ladder)){
  unsigned allowed=t==19?13:t==24?14:t==25?10:t==22?0:15;
  if(allowed!=15&&(entry>3||!(allowed&(1u<<entry))||!(allowed&(1u<<exit))))return false;
 }
 if(r.enemy()==0x36&&exit==0&&!i.food)return false;
 unsigned w=r.wall(exit);if(w==1)return false;
 if(w==4&&!i.bombs)return false;
 // Refillable keys count only once an actual key shop is reachable and farming
 // is possible. Otherwise locked doors require the Magical Key. No free keys.
 if((w==5||w==6)&&!i.keys&&!i.has(mkey))return false;
 if(w==7&&!kill(r,i))return false;
 return true;
}
struct Reach {bool rooms[10][128]{};bool items[10][128]{};bool caves[20]{};bool hearts[128]{};bool armos=false,coast=false,ganon=false,zelda=false;};
bool access(unsigned type,const Inv&i){switch(type){case 0:return true;case 1:return i.bombs;case 2:return i.candle();case 3:return i.has(recorder);case 4:return i.has(raft);case 5:return i.has(20);case 6:return i.bombs&&i.has(ladder);default:return false;}}
bool neighbor(unsigned room,unsigned d,unsigned&next){int v=int(room)+delta[d];if(v<0||v>=128)return false;if((d==1&&room%16==15)||(d==3&&room%16==0))return false;next=v;return true;}
void dungeon(const std::uint8_t*r,unsigned l,const Inv&i,Reach&out){
 struct Node{unsigned room,entry;};Node todo[640];bool seen[128][5]{};unsigned begin=0,end=0;
 auto add=[&](unsigned n,unsigned e){if(n<128&&e<5&&!seen[n][e]){seen[n][e]=true;todo[end++]={n,e};}};
 add(r[info(l)+0x2f],2);
 while(begin<end){auto node=todo[begin++];unsigned n=node.room,e=node.entry;Room rm{r,l,n};if(rm.type()>=62)continue;out.rooms[l][n]=true;
 bool can_item=(!rm.water()||i.has(ladder))&&(!rm.drop()||kill(rm,i))&&rm.type()!=18;
 if(rm.type()==14&&(e!=0&&e!=2))can_item=false;if(rm.type()==15&&(e!=1&&e!=3))can_item=false;
 if(rm.enemy()==0x3e&&!kill(rm,i))can_item=false;
 if(can_item)out.items[l][n]=true;
 if(rm.enemy()==0x3e&&kill(rm,i))out.ganon=true;
 if(rm.enemy()==0x37&&i.ganon)out.zelda=true;
 for(unsigned d=0;d<4;d++){unsigned next;if(neighbor(n,d,next)&&travel(rm,e,d,i)){
  Room target{r,l,next};if(target.enemy()==0x37&&!i.ganon)continue;add(next,(d+2)%4);
 }}
 if(!rm.stairs()||(!kill(rm,i)&&rm.push()))continue;
 for(unsigned j=0;j<10;j++){unsigned sn=r[info(l)+0x34+j];if(l==3&&j==0&&sn==255)sn=15;if(sn>=128)continue;Room stairs{r,l,sn};unsigned left=stairs.byte(0)&127,right=stairs.byte(1)&127;
  if(stairs.type()==63&&left==n&&right==n)out.items[l][sn]=true;
  if(stairs.type()==62){if(left==n)add(right,4);if(right==n)add(left,4);}
 }
 }
}
Reach reach(const std::uint8_t*r,const Inv&i){Reach out;bool levels[10]{};
 for(unsigned s=0;s<128;s++){
  if(r[0x18410+5*128+s]&128)continue;
  unsigned dest=r[0x18410+128+s]>>2;if(!dest||!access(overworld_access[s],i))continue;
  if(dest>=1&&dest<=9)levels[dest]=true;
  else if(dest>=16&&dest<36){unsigned c=dest-16;
   if((c==2&&i.hearts<5)||(c==3&&i.hearts<12)||(c==10&&!i.has(21)))continue;
   out.caves[c]=true;if(c==1)out.hearts[s]=true;
  }
 }
 out.armos=true;out.coast=i.has(ladder);
 for(unsigned l=1;l<=9;l++)if(levels[l]&&(l!=9||i.tri==255))dungeon(r,l,i,out);
 return out;
}
void full(Inv&i){i.bits=0xffffffff;i.hearts=16;i.tri=255;i.bombs=i.keys=i.food=i.ganon=true;}
}
unsigned locations(const std::uint8_t*r,Location*out,unsigned cap){Inv all;full(all);auto reachable=reach(r,all);unsigned count=0;
 auto add=[&](unsigned off,unsigned mask,unsigned level,unsigned room,unsigned cave){for(unsigned j=0;j<count;j++)if(out[j].offset==off)return;if(count<cap)out[count++]={off,mask,level,room,cave};};
 for(unsigned l=1;l<=9;l++)for(unsigned room=0;room<128;room++){Room rm{r,l,room};if(reachable.items[l][room]&&major(rm.item())&&(rm.item()!=3||r[0x1785f]==14))add(grid(l)+4*128+room,31,l,room,99);}
 for(unsigned c=0;c<20;c++)if(c==0||c==2||c==3||c==8||(c>=13&&c<=16))for(unsigned p=0;p<3;p++){unsigned off=0x18610+3*c+p;if(major(r[off]&63))add(off,63,0,0,c);}
 add(0x10d05,255,0,0,20);add(0x1789a,255,0,0,21);return count;
}
Report validate(const std::uint8_t*r){Inv inv;bool got[10][128]{},caves[20][3]{},hearts[128]{},armos=false,coast=false;Report report;Location loc[96];report.locations=locations(r,loc,96);
 for(unsigned pass=0;pass<96;pass++){
  Reach a=reach(r,inv);bool changed=false;auto item=[&](unsigned v,unsigned l){inv.add(v,l);changed=true;};
  for(unsigned c=0;c<20;c++)if(a.caves[c])for(unsigned p=0;p<3;p++){
   unsigned v=r[0x18610+3*c+p]&63;if(c>=13&&c<=16){if(!inv.weapon())continue;if(v==0&&!inv.bombs){inv.bombs=true;changed=true;}if(v==25&&!inv.keys){inv.keys=true;changed=true;}if(v==4&&!inv.food){inv.food=true;changed=true;}}
   if(c==1||c==6||c==11||c==12||c>=17||v>=63)continue;
   if(!caves[c][p]){caves[c][p]=true;item(v,0);}
  }
  for(unsigned s=0;s<128;s++)if(a.hearts[s]&&!hearts[s]){hearts[s]=true;item(heart,0);}
  if(a.armos&&!armos){armos=true;item(r[0x10d05],0);}if(a.coast&&!coast){coast=true;item(r[0x1789a],0);}
  for(unsigned l=1;l<=9;l++)for(unsigned n=0;n<128;n++)if(a.items[l][n]&&!got[l][n]){got[l][n]=true;unsigned v=Room{r,l,n}.item();if(v!=14&&v!=3)item(v,l);else if(v==3&&r[0x1785f]==14)item(v,l);}
  if(a.ganon&&!inv.ganon){inv.ganon=true;changed=true;}if(a.zelda)inv.zelda=true;
  report.spheres=pass+1;if(!changed)break;
 }
 auto a=reach(r,inv);for(unsigned j=0;j<report.locations;j++){auto l=loc[j];bool ok=l.level?a.items[l.level][l.room]:l.cave==20?a.armos:l.cave==21?a.coast:a.caves[l.cave];if(ok)report.collected++;}
 report.triforce=inv.tri;report.equipment=inv.bits;report.ganon=inv.ganon;report.zelda=inv.zelda;
 report.beatable=inv.zelda&&inv.ganon&&inv.tri==255&&report.collected==report.locations;
 return report;
}
bool shuffle(const std::uint8_t*base,std::size_t size,std::uint32_t seed,std::uint8_t*out,const char*&error){
 if(size!=randomizer::rom_size||!randomizer::validate_nes(base,size)||randomizer::crc32(base+16,size-16)!=randomizer::base_crc){error="NEEDS ORIGINAL US REV 1 ZELDA ROM";return false;}
 Location loc[96];unsigned count=locations(base,loc,96);if(count<20||count>=96||base[0x1785f]!=3){error="ITEM LOGIC ROM PROFILE MISMATCH";return false;}
 std::uint8_t items[96],pool[96];for(unsigned j=0;j<count;j++)items[j]=base[loc[j].offset]&loc[j].mask;
 auto next=[&](){seed=seed*1664525u+1013904223u;return seed;};
 for(unsigned attempt=0;attempt<4096;attempt++){
  std::memcpy(pool,items,count);for(unsigned j=count-1;j>0;j--){unsigned k=next()%(j+1);auto t=pool[j];pool[j]=pool[k];pool[k]=t;}
  std::memcpy(out,base,size);
  // Vanilla uses item 3 as 'no room item'. Move that sentinel to 14 so the
  // Magical Sword can be a genuine dungeon pickup (upstream Mags fix).
  for(unsigned bank:{0x18710u,0x18a10u})for(unsigned n=0;n<128;n++){auto&v=out[bank+512+n];if((v&31)==3)v=(v&224)|14;}
  out[0x1785f]=14;
  for(unsigned j=0;j<count;j++)out[loc[j].offset]=(out[loc[j].offset]&~loc[j].mask)|pool[j];
  if(validate(out).beatable){error="";return true;}
 }
 error="NO VALID PLACEMENT FOUND - TRY ANOTHER SEED";return false;
}
}
