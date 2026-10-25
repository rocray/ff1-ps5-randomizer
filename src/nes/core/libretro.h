#ifndef LIBRETRO_H
#define LIBRETRO_H

#include <stdint.h>
#include <stddef.h>

typedef struct retro_system_av {
    void (*video_refresh_cb)(const void *data, uint32_t width, uint32_t height);
    void (*audio_sample_batch_cb)(const int16_t *data, size_t frames);
    void (*input_poll_cb)(void);
} retro_system_av;

typedef struct retro_game_info {
    const char *path;
} retro_game_info;

typedef struct retro_core_env {
    void (*core_load_game)(const retro_game_info *game);
    void (*core_run_frame)(void);
    void (*core_load_state)(const void *data, size_t size);
    void (*core_save_state)(void **data, size_t *size);
    void (*core_reset)(void);
    void (*core_deinit)(void);
    void (*core_get_system_av)(retro_system_av *av);
} retro_core_env;

#endif
