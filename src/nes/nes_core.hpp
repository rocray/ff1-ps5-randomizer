#pragma once
// FF1 prototype integration. SPDX-License-Identifier: GPL-3.0-or-later
#include <cstdint>
#include <cstddef>
#include <string_view>
#include "tracker.hpp"
#include "run_tracking.hpp"
namespace ps5::demo::nes {
struct NesState {
    bool loaded=false, running=false, timer_armed=false;
    std::uint8_t *rom=nullptr;
    std::size_t rom_size=0;
    std::uint32_t *framebuffer=nullptr; // exactly 256x240, RGBA bytes
    double fps=60.0988;
    std::uint32_t buttons=0;
    Tracker tracker{};
    RunProgress progress{};
    std::uint64_t rom_id=0;
    char records_path[128]{};
    bool records_saved=true;
    char saved_at[64]="NO TIMESTAMP";
    char run_label[96]="ORIGINAL FF1";
    bool snapshot_saved=true,save_allowed=true;
    const char *error="";
    char core_message[160]{};
    unsigned frames=0;
    char save_storage[128]{};
    const char *save_path="/download0/ff1.srm";
};
bool load_rom(NesState &,std::string_view);
bool load_rom_data(NesState &,const std::uint8_t *,std::size_t);
bool save_run(NesState &);
bool resume_if_available(NesState &);
bool resume_latest(NesState &,const char *directory="/download0");
void run_frame(NesState &,std::uint32_t);
void save_ram(NesState &);
void unload_rom(NesState &);
void init_core_glue(NesState *);
}


