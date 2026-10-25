#include "libretro.h"
#include "nes_core.hpp"
#include <cstring>

namespace ps5::demo::nes {

    static NesState* g_current_state = nullptr;

    void video_refresh_cb(const void *data, uint32_t width, uint32_t height) {
        if (!g_current_state || !g_current_state->framebuffer) return;

        // Simple copy from emulator buffer to PS5 framebuffer
        // Note: We assume emulator output matches our layout or we convert here.
        // For FCEUML, typically 256x240.
        const uint32_t* pixels = static_cast<const uint32_t*>(data);
        for (int i = 0; i < 256 * 240; ++i) {
            g_current_state->framebuffer[i] = pixels[i];
        }
    }

    void audio_sample_batch_cb(const int16_t *data, size_t frames) {
        // Audio not implemented yet
        (void)data;
        (void)frames;
    }

    void input_poll_cb(void) {
        // Handled via core_run_frame in this simplified glue
    }

    static retro_system_av g_av = {
        video_refresh_cb,
        audio_sample_batch_cb,
        input_poll_cb
    };

    extern "C" {
        retro_system_av* retro_get_system_av() {
            return &g_av;
        }
    }

    void init_core_glue(NesState* state) {
        g_current_state = state;
    }
}
