#pragma once
#include <cstddef>
#include <cstdint>
namespace qol {
struct Settings {std::uint32_t magic=0x314c4f51,layout=0,scanlines=0,curve=0,run=0,music=1,checksum=0;};
bool load(Settings&,const char *path="/download0/z1-settings-v1.bin");
bool save(Settings,const char *path="/download0/z1-settings-v1.bin");
struct Viewport {unsigned x,y,w,h;bool bottom;};
constexpr Viewport viewport(unsigned mode,bool overlay){
 if(!overlay){if(mode==2)return {448,60,1024,960,false};if(mode==3)return {0,0,1920,1080,false};return {240,0,1440,1080,false};}
 switch(mode){case 1:return {0,24,1376,1032,false};case 2:return {304,18,768,720,true};case 3:return {0,0,1376,756,true};default:return {184,0,1008,756,true};}
}
void format_utc(std::uint64_t,char*,std::size_t);
bool stamp_save(const char *save_path);
void read_stamp(const char *save_path,char*,std::size_t);
const std::uint32_t *curve_frame(const std::uint32_t *source);
}
