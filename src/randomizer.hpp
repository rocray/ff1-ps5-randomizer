#pragma once
#include <cstdint>
#include <cstddef>
namespace randomizer {
inline constexpr std::size_t rom_size=262160;
inline constexpr std::uint32_t base_crc=0xcebd2a31;
inline constexpr unsigned slots=8;
struct Entry {std::uint32_t seed=0,crc=0,flags=0;};
struct Library {std::uint32_t magic=0x31525446,version=2;Entry entries[slots]{};std::uint32_t checksum=0;};
std::uint32_t crc32(const std::uint8_t*,std::size_t);
bool validate_nes(const std::uint8_t*,std::size_t);
bool shuffle(const std::uint8_t*,std::size_t,std::uint32_t,std::uint8_t*,const char*&);
bool load_library(Library&,const char *directory="/download0");
bool generate(const char *base,std::uint32_t seed,unsigned slot,Library&,char *out,std::size_t,const char *&error,const char *directory="/download0",bool run=false);
bool select(const Library&,unsigned slot,char *out,std::size_t,const char *&error,const char *directory="/download0");
bool enable_run(std::uint8_t*,std::size_t);
bool save_path(const Library&,unsigned,char*,std::size_t,const char *directory="/download0");
}
