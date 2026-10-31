// SPDX-License-Identifier: GPL-3.0-or-later
// Native US MMC1 profile; not upstream FFR flags/seeds. Map/reward data reference:
// FiendsOfTheElements/FF1Randomizer e49d72e ItemLocations.cs / Data/ShopData.cs.
#include "options.hpp"
#include "patches.hpp"
#include "../randomizer.hpp"
#include <cstring>
#include <initializer_list>
namespace ff1logic {
namespace {
// Original major checks: seven chests, eight NPCs, caravan. Vehicles/orbs fixed.
constexpr unsigned chest[]={0x3113,0x312c,0x314d,0x3182,0x3193,0x31c4,0x31f9};
constexpr unsigned npc[]={princess_item,astos_item,matoya_item,elf_item,sarda_item,cube_item,fairy_item,chime_item};
constexpr std::uint8_t original[]={6,2,9,11,13,8,7,1,3,4,5,10,14,16,12,15};
struct Rng {std::uint32_t s;unsigned pick(unsigned n){unsigned v,threshold=(0u-n)%n;do{s=s*1664525u+1013904223u;v=s;}while(v<threshold);return v%n;} template<class T> void mix(T*p,unsigned n){for(unsigned i=n-1;i>0;i--){auto j=pick(i+1);auto t=p[i];p[i]=p[j];p[j]=t;}}};
unsigned shop(const std::uint8_t*r,unsigned n){unsigned at=0x38312+2*n;return 0x30010+r[at]+256u*r[at+1];}
unsigned count(const std::uint8_t*r,unsigned at){unsigned n=0;while(n<5&&r[at+n])n++;return n;}
void price(std::uint8_t*r,unsigned id,unsigned value){r[0x37c10+id*2]=value&255;r[0x37c11+id*2]=value>>8;}
unsigned price(const std::uint8_t*r,unsigned id){return r[0x37c10+id*2]+256u*r[0x37c11+id*2];}
void set_major(std::uint8_t*r,const std::uint8_t*p){for(unsigned i=0;i<7;i++)r[chest[i]]=p[i];for(unsigned i=0;i<8;i++)r[0x30011+npc[i]]=p[7+i];r[0x38471]=p[15];}
void shops(std::uint8_t*r,Rng&rng){
 // Same list lengths/pointers: no overflow into the next shop or code.
 for(unsigned n=0;n<16;n++)if(n<6||n>=10){unsigned at=shop(r,n),len=count(r,at),first=n<6?28:68;bool used[40]{};
  for(unsigned j=0;j<len;j++){unsigned v;do{v=rng.pick(40);}while(used[v]);used[v]=true;r[at+j]=first+v;}
 }
 // Affordable starter coverage; unarmed monks remain supported.
 unsigned w=shop(r,0),a=shop(r,10);r[w]=29;r[w+1]=30;
 for(unsigned j=2;j<count(r,w);j++)if(r[w+j]==29||r[w+j]==30)r[w+j]=31+j;
 r[a]=68;for(unsigned j=1;j<count(r,a);j++)if(r[a+j]==68)r[a+j]=69+j;
 // Item shops retain HEAL/PURE and get chest-tier weapons. Other slots rotate
 // camping/stone cures. The caravan remains the single major-item slot.
 for(unsigned n=60;n<=65;n++){unsigned at=shop(r,n),len=count(r,at);r[at]=25;r[at+1]=26;r[at+2]=47+rng.pick(21);
  for(unsigned j=3;j<len;j++)r[at+j]=j==3?22+rng.pick(3):27;
 }
 // Guaranteed non-weapon refill availability, with all six consumables sold.
 r[shop(r,61)+3]=22;r[shop(r,62)+3]=23;r[shop(r,62)+4]=27;r[shop(r,63)+3]=24;
 for(auto&p:shops_patches)std::memcpy(r+p.offset,p.data,p.size);
}
// Keep logical spell IDs tied to destination slots. Move the entire spell
// identity instead, so learned slots and MP usage naturally use the new level.
// Reference: upstream Magic.cs / EnemyScripts.cs and original disassembly.
void magic_shuffle(const std::uint8_t*base,std::uint8_t*r,Rng&rng){
 std::uint8_t old_at_new[64],new_of_old[64];
 for(unsigned i=0;i<64;i++)old_at_new[i]=i;
 rng.mix(old_at_new,64);
 for(unsigned i=0;i<64;i++){
  unsigned old=old_at_new[i];new_of_old[old]=i;
  std::memcpy(r+0x301f0+i*8,base+0x301f0+old*8,8); // effect, targeting, animation
  std::memcpy(r+0x2b870+i*2,base+0x2b870+old*2,2); // item-name pointer
  r[0x304d0+i]=base[0x304d0+old]; // battle message
 }
 // Permissions follow the spell; Open Spell Learning clears them afterwards.
 for(unsigned cls=0;cls<12;cls++)for(unsigned level=0;level<8;level++){
  unsigned value=0;
  for(unsigned bit=0;bit<8;bit++){unsigned old=old_at_new[level*8+bit];
   if(base[0x3ad28+cls*8+old/8]&(0x80u>>(old%8)))value|=0x80u>>bit;
  }
  r[0x3ad28+cls*8+level]=value;
 }
 // Preserve enemy scripts and equipment spell effects, including enemy FIRE
 // under confusion. Script skill/ability slots are a separate table, untouched.
 for(unsigned script=0;script<44;script++)for(unsigned j=2;j<10;j++){
  unsigned at=0x31030+script*16+j,old=base[at];if(old<64)r[at]=new_of_old[old];
 }
 for(unsigned i=0;i<40;i++)for(unsigned at:{0x30013+i*8,0x30153+i*4}){
  unsigned old=base[at];if(old&&old<=64)r[at]=new_of_old[old-1]+1;
 }
 r[0x3322e]=new_of_old[4];
 for(unsigned i=0;i<13;i++){
  unsigned at=0x3af0a+i*7;r[at]=176+new_of_old[base[at]-176];
 }
 for(auto&p:magic_patches)std::memcpy(r+p.offset,p.data,p.size);
}
void prices(std::uint8_t*r,Rng&rng){
 for(unsigned start:{22u,28u,68u}){unsigned n=start==22?6:40;unsigned values[40];for(unsigned j=0;j<n;j++)values[j]=price(r,start+j);rng.mix(values,n);for(unsigned j=0;j<n;j++)price(r,start+j,values[j]);}
 // Preserve an affordable opening and recurring healing supplies.
 price(r,28,10);price(r,29,5);price(r,30,5);price(r,68,10);
 price(r,25,30+rng.pick(121));price(r,26,10+rng.pick(66));price(r,15,1000+rng.pick(59001));
}
}
void read_major(const std::uint8_t*r,std::uint8_t out[16],bool patched){for(unsigned i=0;i<7;i++)out[i]=r[chest[i]];for(unsigned i=0;i<8;i++)out[7+i]=patched?r[0x30011+npc[i]]:original[7+i];out[15]=r[0x38471];}
Report validate(const std::uint8_t*r,bool patched){
 std::uint8_t rewards[16];read_major(r,rewards,patched);Report result;unsigned pool=0;
 for(auto item:rewards){if(item<1||item>16||(pool&(1u<<item)))return result;pool|=1u<<item;}
 unsigned have=0,seen=0;bool canal=false,canoe=false,airship=false;
 auto has=[&](unsigned item){return bool(have&(1u<<item));};
 // Garland -> bridge and Bikke -> ship have no item requirements; always reachable.
 for(unsigned sphere=0;sphere<32;sphere++){
  unsigned before=have;bool old_canal=canal,old_canoe=canoe,old_airship=airship;
  canal|=has(6);bool west=canal||airship;
  bool earth=west&&has(10);canoe|=earth;airship|=canoe&&has(11);west=canal||airship;
  bool reachable[]={has(5),true,west,canoe||airship,
   canoe&&west&&has(2),airship&&canoe&&has(16),airship&&has(12)&&has(14),
   true,has(2),has(3),has(4),west&&(has(9)||airship),
   airship&&canoe,airship&&has(15),airship&&has(8),airship};
  for(unsigned i=0;i<16;i++)if(reachable[i]){have|=1u<<rewards[i];seen|=1u<<i;}
  result.spheres=sphere+1;
  if(have==before&&canal==old_canal&&canoe==old_canoe&&airship==old_airship)break;
 }
 result.items=have;result.locations=seen;result.canal=canal;result.canoe=canoe;result.airship=airship;
 // Earth -> canoe -> Fire, Sea/Oxyale -> Water, Chime+Cube -> Air;
 // all four orbs, Lute and Key permit the final Temple/Chaos route.
 result.beatable=seen==65535&&have==0x1fffe&&canoe&&airship&&has(10)&&has(16)&&has(12)&&has(14)&&has(1)&&has(5);
 return result;
}
bool shuffle(const std::uint8_t*base,std::size_t size,std::uint32_t seed,std::uint8_t*out,Options options,const char*&error){
 if(!out||size!=randomizer::rom_size||!randomizer::validate_nes(base,size)||randomizer::crc32(base+16,size-16)!=randomizer::base_crc){error="NEEDS ORIGINAL US FF1 ROM";return false;}
 if(options.treasure){if(!randomizer::shuffle(base,size,seed,out,error))return false;}else std::memcpy(out,base,size);
 // Independent streams: toggling shops/prices doesn't silently reroll progression.
 if(options.major){
  for(auto&p:major_patches)std::memcpy(out+p.offset,p.data,p.size);
  Rng rng{seed^0xa4b19d37u};bool valid=false;std::uint8_t pool[16];
  for(unsigned attempt=0;attempt<16384;attempt++){std::memcpy(pool,original,16);rng.mix(pool,16);set_major(out,pool);if(validate(out).beatable){valid=true;break;}}
  if(!valid){error="NO BEATABLE ITEM PLACEMENT - TRY ANOTHER SEED";return false;}
 }
 if(options.shops){
  Rng rng{seed^0x73686f70u};shops(out,rng);
  Rng spell_rng{seed^0x6d616769u};magic_shuffle(base,out,spell_rng);
  for(auto&p:slot_prices_patches)std::memcpy(out+p.offset,p.data,p.size);
  // shop_id is ONE based. Price tokens reference the original global price
  // table, independently of the shuffled item name/reward in the same slot.
  std::memset(out+0x37690,0,355);
  for(unsigned n=0;n<70;n++){unsigned at=shop(base,n);for(unsigned j=0;j<5;j++)out[0x37690+(n+1)*5+j]=base[at+j];}
 }
 if(options.shops&&options.prices){Rng rng{seed^0x70726963u};prices(out,rng);}
 // Permission bit set means forbidden. Learning is open; MP growth is unchanged.
 if(options.open_magic)std::memset(out+0x3ad28,0,96);
 if(!validate(out,options.major).beatable){error="PROGRESSION CHECK FAILED";return false;}
 error="";return true;
}
}
