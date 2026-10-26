// FF1 prototype integration. SPDX-License-Identifier: GPL-3.0-or-later
#include "libretro.h"
#include "nes_core.hpp"
#include "../demo_renderer.hpp"
#include <cstring>
#include <cstdarg>
#include <cstdio>
namespace ps5::demo::nes {
static NesState *current=nullptr;
static retro_pixel_format format=RETRO_PIXEL_FORMAT_0RGB1555;
static retro_game_info_ext game_info{};
static void core_log(enum retro_log_level level,const char *fmt,...) {
    if(!current || level<RETRO_LOG_WARN || current->core_message[0]) return;
    va_list args;va_start(args,fmt);
    std::vsnprintf(current->core_message,sizeof(current->core_message),fmt,args);
    va_end(args);
    for(char *p=current->core_message;*p;p++) if(*p=='\n'||*p=='\r') *p=' ';
}
static bool environment(unsigned cmd,void *data) {
    switch(cmd) {
    case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:
        static_cast<retro_log_callback*>(data)->log=core_log;return true;
    case RETRO_ENVIRONMENT_GET_GAME_INFO_EXT:
        if(!current || !current->rom || !current->rom_size) return false;
        // This FCEUmm revision consumes memory content only through info_ext.
        // The frontend owns these bytes until retro_unload_game has completed.
        game_info = {};
        game_info.full_path = "/app0/rom/FF1.NES";
        game_info.dir = "/app0/rom";
        game_info.name = "FF1";
        game_info.ext = "nes";
        game_info.data = current->rom;
        game_info.size = current->rom_size;
        game_info.persistent_data = true;
        *static_cast<const retro_game_info_ext **>(data) = &game_info;
        return true;
    case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
        if (*static_cast<retro_pixel_format*>(data)>RETRO_PIXEL_FORMAT_RGB565) return false;
        format=*static_cast<retro_pixel_format*>(data); return true;
    case RETRO_ENVIRONMENT_GET_CAN_DUPE: *static_cast<bool*>(data)=true; return true;
    case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE: *static_cast<bool*>(data)=false;return true;
    case RETRO_ENVIRONMENT_GET_VARIABLE: static_cast<retro_variable*>(data)->value=nullptr;return false;
    case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
    case RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY: *static_cast<const char**>(data)="/app0/assets";return true;
    case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY: *static_cast<const char**>(data)="/download0";return true;
    default: return false;
    }
}
static void video(const void *data,unsigned width,unsigned height,std::size_t pitch) {
    if (!current || !current->framebuffer || !data || data==RETRO_HW_FRAME_BUFFER_VALID || !width || !height) return;
    const unsigned bytes=format==RETRO_PIXEL_FORMAT_XRGB8888?4:2;
    if (width>1024 || height>1024 || pitch<width*bytes) return;
    for(unsigned y=0;y<240;y++) for(unsigned x=0;x<256;x++) {
        const auto *p=static_cast<const std::uint8_t*>(data)+(y*height/240)*pitch+(x*width/256)*bytes;
        unsigned r,g,b;
        if(bytes==4) { std::uint32_t v;std::memcpy(&v,p,4);r=(v>>16)&255;g=(v>>8)&255;b=v&255; }
        else { std::uint16_t v;std::memcpy(&v,p,2);b=(v&31)*255/31;
            if(format==RETRO_PIXEL_FORMAT_RGB565){r=((v>>11)&31)*255/31;g=((v>>5)&63)*255/63;}
            else {r=((v>>10)&31)*255/31;g=((v>>5)&31)*255/31;}}
        current->framebuffer[y*256+x]=0xff000000u|(b<<16)|(g<<8)|r;
    }
}
static void audio_sample(std::int16_t,std::int16_t) {}
static std::size_t audio_batch(const std::int16_t*,std::size_t frames) {return frames;}
static void poll() {}
static std::int16_t input(unsigned port,unsigned device,unsigned,unsigned id) {
    if(!current || port || device!=RETRO_DEVICE_JOYPAD) return 0;
    using namespace ps5::demo;
    const std::uint32_t masks[]={pad::circle,0,pad::touchpad,pad::options,pad::up,pad::down,pad::left,pad::right,pad::cross};
    return id<9 && (current->buttons&masks[id])?1:0;
}
void init_core_glue(NesState *state) {
    current=state;format=RETRO_PIXEL_FORMAT_0RGB1555;
    retro_set_environment(environment);retro_set_video_refresh(video);
    retro_set_audio_sample(audio_sample);retro_set_audio_sample_batch(audio_batch);
    retro_set_input_poll(poll);retro_set_input_state(input);
}
}
