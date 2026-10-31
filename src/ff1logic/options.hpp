#pragma once
#include <cstdint>
#include <cstddef>
namespace ff1logic {
struct Options {bool treasure=true,major=false,shops=false,prices=false,open_magic=false;};
constexpr unsigned custom=2,treasure_flag=4,major_flag=8,shops_flag=16,prices_flag=32,magic_flag=64,slot_prices_flag=128,full_magic_flag=256,known_flags=511;
constexpr unsigned flags(Options o,bool run=false){return custom|slot_prices_flag|full_magic_flag|(run?1:0)|(o.treasure?treasure_flag:0)|(o.major?major_flag:0)|(o.shops?shops_flag:0)|(o.shops&&o.prices?prices_flag:0)|(o.open_magic?magic_flag:0);}
struct Report {bool beatable=false;unsigned items=0,locations=0,spheres=0;bool canal=false,canoe=false,airship=false;};
Report validate(const std::uint8_t*,bool patched=true);
void read_major(const std::uint8_t*,std::uint8_t out[16],bool patched=true);
bool shuffle(const std::uint8_t*,std::size_t,std::uint32_t,std::uint8_t*,Options,const char*&);
}
