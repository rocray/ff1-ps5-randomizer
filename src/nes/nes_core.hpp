#pragma once
// FF1 prototype integration. SPDX-License-Identifier: GPL-3.0-or-later
#include <cstdint>
#include <cstddef>
#include <string_view>
#include "tracker.hpp"
namespace ps5::demo::nes {
struct NesState {
    bool loaded=false, running=false, timer_armed=false;
    std::uint8_t *rom=nullptr;
    std::size_t rom_size=0;
    std::uint32_t *framebuffer=nullptr; // exactly 256x240, RGBA bytes
    double fps=60.0988;
    std::uint32_t buttons=0;
    Tracker tracker{};
    const char *error="";
    char core_message[160]{};
    unsigned frames=0;
    const char *save_path="/download0/ff1.srm";
};
bool load_rom(NesState &,std::string_view);
void run_frame(NesState &,std::uint32_t);
void save_ram(NesState &);
void unload_rom(NesState &);
void init_core_glue(NesState *);
}
