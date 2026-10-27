#pragma once
#include <cstdint>
#include <cstddef>
namespace ps5::demo::music {
void start() noexcept;
void set_game(bool active) noexcept;
void submit(const std::int16_t *,std::size_t) noexcept;
void set_active(bool active) noexcept;
const char *status() noexcept;
}

