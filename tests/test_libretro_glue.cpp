// SPDX-License-Identifier: GPL-3.0-or-later
#include "libretro.h"
#include "../src/nes/nes_core.hpp"
#include "../src/demo_renderer.hpp"
#include <cassert>
#include <cstring>
#include <cstdio>
static retro_environment_t env;
static retro_video_refresh_t video;
static retro_input_state_t input;
extern "C" {
void retro_set_environment(retro_environment_t cb){env=cb;}
void retro_set_video_refresh(retro_video_refresh_t cb){video=cb;}
void retro_set_audio_sample(retro_audio_sample_t){}
void retro_set_audio_sample_batch(retro_audio_sample_batch_t){}
void retro_set_input_poll(retro_input_poll_t){}
void retro_set_input_state(retro_input_state_t cb){input=cb;}
}
int main(){
 using namespace ps5::demo;nes::NesState s;std::uint32_t pixels[256*240]{};s.framebuffer=pixels;nes::init_core_glue(&s);
 const retro_game_info_ext *info=nullptr;
 assert(!env(RETRO_ENVIRONMENT_GET_GAME_INFO_EXT,&info));
 std::uint8_t rom[16]={'N','E','S',0x1a};s.rom=rom;s.rom_size=sizeof(rom);
 assert(env(RETRO_ENVIRONMENT_GET_GAME_INFO_EXT,&info));
 assert(info && info->data==rom && info->size==sizeof(rom));
 assert(info->persistent_data && !info->file_in_archive && !std::strcmp(info->ext,"nes"));
 for(auto format:{RETRO_PIXEL_FORMAT_0RGB1555,RETRO_PIXEL_FORMAT_RGB565,RETRO_PIXEL_FORMAT_XRGB8888}){
  assert(env(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT,&format));
  unsigned char data[24]{};unsigned red=format==RETRO_PIXEL_FORMAT_XRGB8888?0xff0000:format==RETRO_PIXEL_FORMAT_RGB565?0xf800:0x7c00;
  unsigned bytes=format==RETRO_PIXEL_FORMAT_XRGB8888?4:2;std::memcpy(data,&red,bytes);
  unsigned blue=31;if(bytes==4)blue=255;std::memcpy(data+12+bytes,&blue,bytes);
  video(data,2,2,12);assert(pixels[0]==0xff0000ff);assert(pixels[239*256+255]==0xffff0000);
  video(nullptr,2,2,12);assert(pixels[0]==0xff0000ff);
 }
 s.buttons=pad::cross|pad::options;assert(input(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_A)==1);
 assert(input(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_START)==1);
 assert(input(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_B)==0);
 assert(input(1,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_A)==0);
 puts("PASS: RGB1555/RGB565/XRGB8888, padded pitch, RGBA conversion, duplicate frame, input mapping");
}
