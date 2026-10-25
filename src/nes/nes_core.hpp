#pragma once
#include <cstdint>
#include <vector>
#include <string_view>

namespace ps5::demo::nes
{
struct NesState
{
    bool loaded = false;
    bool running = false;
    std::uint8_t *rom = nullptr;
    std::size_t rom_size = 0;
    std::uint32_t *framebuffer = nullptr;
};

    bool load_rom(NesState &state, std::string_view path);
    void run_frame(NesState &state, std::uint32_t buttons);
}
