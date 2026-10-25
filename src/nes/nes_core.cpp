#include "nes_core.hpp"
#include "../demo_renderer.hpp"
#include "libretro_glue.cpp"
#include <cstring>

namespace ps5::demo::nes
{
    bool load_rom(NesState &state, std::string_view path)
    {
        std::uint8_t *temp = new (std::nothrow) std::uint8_t[512 * 1024];
        if (!temp) return false;

        read_asset_bytes(path.data(), {temp, 512 * 1024});
        
        state.rom = temp;
        state.rom_size = 512 * 1024;
        state.loaded = true;
        state.running = true;

        init_core_glue(&state);
        return true;
    }

    void run_frame(NesState &state, std::uint32_t buttons)
    {
        if (!state.running) return;
        
        // This will be replaced with actual core_run_frame() 
        // once the FCEUML .c files are added to src/nes/core/
        for (int i = 0; i < 256 * 240; ++i) {
            state.framebuffer[i] = 0xFF0000FF; 
            if ((i / 240) % 20 == 0 || (i % 240) % 20 == 0) {
                state.framebuffer[i] = 0xFFFFFFFF;
            }
        }
    }
}
