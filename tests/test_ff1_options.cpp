#include "../src/randomizer.hpp"
#include "../src/ff1logic/patches.hpp"
#include <cassert>
#include <fstream>
#include <vector>
#include <array>
#include <filesystem>
#include <cstdio>
#include <cstring>
#include <chrono>
static unsigned shop(const std::vector<unsigned char>&r,unsigned n){unsigned at=0x38312+n*2;return 0x30010+r[at]+r[at+1]*256;}
static void check_magic(const std::vector<unsigned char>&base,const std::vector<unsigned char>&r,bool open){
 unsigned old_at_new[64],inverse[64],moved=0;bool seen[64]{};
 for(unsigned i=0;i<64;i++){
  unsigned old=0;while(old<64&&std::memcmp(&base[0x2b870+old*2],&r[0x2b870+i*2],2))old++;
  assert(old<64&&!seen[old]);seen[old]=true;old_at_new[i]=old;inverse[old]=i;moved+=old/8!=i/8;
  assert(!std::memcmp(&r[0x301f0+i*8],&base[0x301f0+old*8],8));assert(r[0x304d0+i]==base[0x304d0+old]);
  for(unsigned cls=0;cls<12;cls++)assert(bool(r[0x3ad28+cls*8+i/8]&(0x80>>(i%8)))==(!open&&bool(base[0x3ad28+cls*8+old/8]&(0x80>>(old%8)))));
 }
 assert(moved>20); // Full-pool mode actually crosses levels, rather than a cosmetic shop swap.
 for(unsigned n=20;n<40;n++){unsigned at=shop(base,n);for(unsigned j=0;j<5&&base[at+j];j++)assert(r[at+j]==base[at+j]);}
 for(unsigned i=0;i<44;i++)for(unsigned j=2;j<10;j++){unsigned at=0x31030+i*16+j;assert(base[at]==255||base[at]<64);assert(r[at]==(base[at]<64?inverse[base[at]]:255));}
 for(unsigned i=0;i<40;i++)for(unsigned at:{0x30013+i*8,0x30153+i*4}){assert(base[at]<=64);assert(r[at]==(base[at]?inverse[base[at]-1]+1:0));}
 assert(r[0x3322e]==inverse[4]);for(unsigned i=0;i<13;i++){unsigned at=0x3af0a+i*7;assert(r[at]==176+inverse[base[at]-176]);}
}
int main(int argc,char**argv){assert(argc==2);std::ifstream f(argv[1],std::ios::binary);std::vector<unsigned char>base((std::istreambuf_iterator<char>(f)),{}),a(base.size()),b(base.size());assert(base.size()==randomizer::rom_size);const char*error="";assert(ff1logic::validate(base.data(),false).beatable);
 std::vector<bool>allowed(base.size());
 for(unsigned i=0;i<64;i++){
  for(unsigned j=0;j<8;j++)allowed[0x301f0+i*8+j]=true;
  allowed[0x2b870+i*2]=allowed[0x2b871+i*2]=allowed[0x304d0+i]=true;
 }
 for(unsigned i=0;i<44;i++)for(unsigned j=2;j<10;j++)allowed[0x31030+i*16+j]=true;
 for(unsigned i=0;i<40;i++)allowed[0x30013+i*8]=allowed[0x30153+i*4]=true;
 allowed[0x3322e]=true;for(unsigned i=0;i<13;i++)allowed[0x3af0a+i*7]=true;
 for(auto&p:ff1logic::magic_patches)for(unsigned j=0;j<p.size;j++)allowed[p.offset+j]=true;
 for(unsigned i=0x3110;i<0x3210;i++)allowed[i]=true;
 for(unsigned i=0x3839f;i<0x38477;i++)allowed[i]=true;
 for(auto&p:ff1logic::major_patches)for(unsigned j=0;j<p.size;j++)allowed[p.offset+j]=true;
 for(auto&p:ff1logic::slot_prices_patches)for(unsigned j=0;j<p.size;j++)allowed[p.offset+j]=true;
 for(unsigned i=0x37690;i<0x377f3;i++)allowed[i]=true;
 for(auto&p:ff1logic::shops_patches)for(unsigned j=0;j<p.size;j++)allowed[p.offset+j]=true;
 for(unsigned i=22;i<108;i++)allowed[0x37c10+i*2]=allowed[0x37c11+i*2]=true;
 allowed[0x37c10+30]=allowed[0x37c11+30]=true;
 for(unsigned i=0;i<96;i++)allowed[0x3ad28+i]=true;
 auto start=std::chrono::steady_clock::now();unsigned worst=0;
 for(unsigned seed=0;seed<1000;seed++){
  ff1logic::Options o{true,true,true,true,true};assert(ff1logic::shuffle(base.data(),base.size(),seed,a.data(),o,error));assert(ff1logic::shuffle(base.data(),base.size(),seed,b.data(),o,error));assert(a==b);for(unsigned i=0;i<a.size();i++)if(!allowed[i])assert(a[i]==base[i]);
  check_magic(base,a,true);
  auto report=ff1logic::validate(a.data());assert(report.beatable&&report.locations==65535);if(report.spheres>worst)worst=report.spheres;
  std::array<unsigned,256>before{},after{};for(unsigned i=0;i<256;i++){if(base[0x3110+i]>=22)before[base[0x3110+i]]++;if(a[0x3110+i]>=22)after[a[0x3110+i]]++;}assert(before==after);
  std::array<unsigned,64>spells{};for(unsigned n=20;n<40;n++){auto at=shop(base,n);for(unsigned j=0;base[at+j];j++){assert(a[at+j]>=176&&a[at+j]<240);assert((a[at+j]-176)/8==(base[at+j]-176)/8);spells[a[at+j]-176]++;}}
  for(auto count:spells)assert(count==1);
  for(unsigned n=0;n<70;n++){auto at=shop(base,n);assert(at==shop(a,n));if(at==0x3839e)continue;if(n>=40&&n<60){for(unsigned j=0;j<3;j++)assert(a[at+j]==base[at+j]);continue;}unsigned j=0;while(base[at+j]&&j<5)j++;assert(a[at+j]==0);}
  for(unsigned n=60;n<=65;n++){auto at=shop(a,n);assert(a[at]==25&&a[at+1]==26&&a[at+2]>=47&&a[at+2]<=67);}
  for(unsigned i=0;i<96;i++)assert(a[0x3ad28+i]==0);
 }
 // Every checkbox combination; inactive features and PRNG streams are independent.
 for(unsigned bits=0;bits<32;bits++){
  ff1logic::Options o{bool(bits&1),bool(bits&2),bool(bits&4),bool(bits&8),bool(bits&16)};assert(ff1logic::shuffle(base.data(),base.size(),77,a.data(),o,error));assert(ff1logic::validate(a.data(),o.major).beatable);
  if(o.shops)check_magic(base,a,o.open_magic);
  if(!o.major){unsigned char p[16],q[16];ff1logic::read_major(base.data(),p,false);ff1logic::read_major(a.data(),q,false);assert(!std::memcmp(p,q,16));}
  if(!o.shops)for(unsigned n=0;n<69;n++){auto at=shop(base,n);for(unsigned j=0;j<5&&base[at+j];j++)assert(base[at+j]==a[at+j]);}
  if(!o.shops||!o.prices)assert(!std::memcmp(a.data()+0x37c10,base.data()+0x37c10,512));
  if(o.shops)for(unsigned n=0;n<70;n++)for(unsigned j=0;j<5;j++)assert(a[0x37690+(n+1)*5+j]==base[shop(base,n)+j]);
  if(!bits)assert(a==base);
 }
 ff1logic::Options all{true,true,true,true,true};assert(ff1logic::shuffle(base.data(),base.size(),123,a.data(),all,error));
 // Force each progression gate to contain its own prerequisite.
 for(unsigned target:{0u,3u,5u,6u,8u,9u,10u,12u,13u,14u,15u}){
  unsigned needed[]={5,0,0,10,0,16,14,0,2,3,4,9,11,15,8,11};unsigned char p[16];ff1logic::read_major(a.data(),p);unsigned from=0;while(p[from]!=needed[target])from++;
  constexpr unsigned offsets[]={0x3113,0x312c,0x314d,0x3182,0x3193,0x31c4,0x31f9,0x30011+ff1logic::princess_item,0x30011+ff1logic::astos_item,0x30011+ff1logic::matoya_item,0x30011+ff1logic::elf_item,0x30011+ff1logic::sarda_item,0x30011+ff1logic::cube_item,0x30011+ff1logic::fairy_item,0x30011+ff1logic::chime_item,0x38471};
  b=a;std::swap(b[offsets[from]],b[offsets[target]]);assert(!ff1logic::validate(b.data()).beatable);
 }
 const char*dir="/tmp/ff1-options-test";std::filesystem::create_directories(dir);assert(randomizer::save_options(all,dir));ff1logic::Options persisted;assert(randomizer::load_options(persisted,dir)&&ff1logic::flags(persisted)==510);randomizer::Library l;char path[256],first[256];assert(randomizer::generate(argv[1],123,0,l,path,sizeof(path),error,dir));std::snprintf(first,sizeof(first),"%s",path);assert(randomizer::generate(argv[1],123,1,l,path,sizeof(path),error,dir,true,&all));assert(std::strcmp(first,path));randomizer::Library read;assert(randomizer::load_library(read,dir)&&read.entries[1].flags==511);assert(randomizer::select(read,0,path,sizeof(path),error,dir));assert(randomizer::select(read,1,path,sizeof(path),error,dir));
 {std::ofstream rom("build/ff1-options-test.nes",std::ios::binary);rom.write((char*)a.data(),a.size());}
 // Preserve c1 catalog paths and files rather than silently rerolling old seeds.
 read.entries[2]={456,randomizer::crc32(base.data(),base.size()),126};
 {std::ofstream old(std::string(dir)+"/ff1-c1-000001c8-7e.nes",std::ios::binary);old.write((char*)base.data(),base.size());}
 assert(randomizer::select(read,2,path,sizeof(path),error,dir));assert(std::strstr(path,"ff1-c1-"));
 ff1logic::Options no_shop{true,false,false,true,false},prices_off=no_shop;prices_off.prices=false;
 assert(ff1logic::flags(no_shop)==ff1logic::flags(prices_off));
 assert(randomizer::save_options(no_shop,dir));assert(randomizer::load_options(persisted,dir)&&!persisted.prices);
 assert(ff1logic::shuffle(base.data(),base.size(),123,a.data(),no_shop,error));assert(ff1logic::shuffle(base.data(),base.size(),123,b.data(),prices_off,error));assert(a==b);
 all.prices=false;assert(ff1logic::shuffle(base.data(),base.size(),123,a.data(),all,error));
 {std::ofstream rom("build/ff1-slot-prices-off.nes",std::ios::binary);rom.write((char*)a.data(),a.size());}
 all.open_magic=false;assert(ff1logic::shuffle(base.data(),base.size(),123,a.data(),all,error));check_magic(base,a,false);
 {std::ofstream rom("build/ff1-magic-restricted.nes",std::ios::binary);rom.write((char*)a.data(),a.size());}
 read.entries[3]={789,randomizer::crc32(base.data(),base.size()),254};
 {std::ofstream old(std::string(dir)+"/ff1-c2-00000315-fe.nes",std::ios::binary);old.write((char*)base.data(),base.size());}
 assert(randomizer::select(read,3,path,sizeof(path),error,dir));assert(std::strstr(path,"ff1-c2-"));
 std::filesystem::remove_all(dir);printf("PASS: 1000 deterministic all-option seeds, 32 checkbox combinations, 11 forced deadlocks, spell levels/pool, refill safeguards, legacy/custom catalog. Max spheres %u, %.2fs\n",worst,std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count());
}
