#pragma once
#include <cstdint>
#include <cstddef>
namespace zlogic {
struct Location {unsigned offset,mask,level,room,cave;};
struct Report {bool beatable=false;unsigned spheres=0,locations=0,collected=0,triforce=0;std::uint32_t equipment=0;bool ganon=false,zelda=false;};
unsigned locations(const std::uint8_t*,Location*,unsigned capacity);
Report validate(const std::uint8_t*);
bool shuffle(const std::uint8_t*,std::size_t,std::uint32_t,std::uint8_t*,const char*&);
}
