/*
 * ps5-native-app-boilerplate - PS5 video, input, and bitmap-font renderer.
 * Copyright (c) 2026 BlackBearReloaded
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Keeps PS5 VideoOut setup, pad input, and the bitmap font out of main files.
 */

#pragma once

#include <cstdint>
#include <span>
#include <string_view>

namespace ps5::demo
{
enum class Color : std::uint32_t
{
    // 0xAABBGGRR packing (RGBA byte order in memory)
    background = UINT32_C(0xff190d0a),
    panel = UINT32_C(0xff301f17),
    white = UINT32_C(0xffffffff),
    cyan = UINT32_C(0xffffff00),
    magenta = UINT32_C(0xffff00ff),
    yellow = UINT32_C(0xff00ffff),

    // FF1 menu palette
    navy = UINT32_C(0xff52260d),        // panel fill  #0d2652
    navy_deep = UINT32_C(0xff341808),   // deep panel  #081834
    steel = UINT32_C(0xffc0a89a),       // dim text    #9aa8c0
    gold = UINT32_C(0xff27a2c9),        // gold accent #c9a227
    highlight = UINT32_C(0xffd06f2f),   // selection   #2f6fd0
    border = UINT32_C(0xffe8e8e8),      // light frame #e8e8e8
    good = UINT32_C(0xff4ce04c),        // positive    #4ce04c
    bad = UINT32_C(0xff4c4ce0),         // negative    #e04c4c
    ink = UINT32_C(0xff101010),         // near black
};

namespace pad
{
inline constexpr std::uint32_t l3 = 0x0002;
inline constexpr std::uint32_t r3 = 0x0004;
inline constexpr std::uint32_t up = 0x0010;
inline constexpr std::uint32_t right = 0x0020;
inline constexpr std::uint32_t down = 0x0040;
inline constexpr std::uint32_t left = 0x0080;
inline constexpr std::uint32_t l1 = 0x0400;
inline constexpr std::uint32_t r1 = 0x0800;
inline constexpr std::uint32_t triangle = 0x1000;
inline constexpr std::uint32_t circle = 0x2000;
inline constexpr std::uint32_t cross = 0x4000;
inline constexpr std::uint32_t square = 0x8000;
inline constexpr std::uint32_t options = 0x0008;
inline constexpr std::uint32_t touchpad = 0x100000;
} // namespace pad

struct Input
{
    std::uint32_t held;
    std::uint32_t pressed; // edges since previous frame
    bool connected;
};

class Canvas;
using Frame = void (*)(Canvas &, const Input &) noexcept;

[[noreturn]] void run(Frame frame) noexcept;

// Character cell is 5x7 pixels with 1 pixel of spacing; advance is 6 * scale.
[[nodiscard]] inline constexpr unsigned text_advance(std::string_view value,
                                                     unsigned scale) noexcept
{
    return static_cast<unsigned>(value.size()) * 6U * scale;
}

class Canvas final
{
  public:
    void clear(Color color) noexcept;
    void rectangle(unsigned x, unsigned y, unsigned width, unsigned height, Color color) noexcept;
    void circle(unsigned center_x, unsigned center_y, unsigned radius, Color color) noexcept;
    void triangle(unsigned center_x, unsigned top, unsigned half_width, unsigned height,
                  Color color) noexcept;
    void text(unsigned x, unsigned y, std::string_view value, unsigned scale, Color color) noexcept;
    void text_centered(unsigned center_x, unsigned y, std::string_view value, unsigned scale,
                       Color color) noexcept;
    // Copies a linear RGBA (0xAABBGGRR per pixel) image into the tiled buffer.
    void image(unsigned x, unsigned y, unsigned width, unsigned height,
               const std::uint32_t *pixels) noexcept;

    void region(unsigned x,unsigned y,unsigned w,unsigned h,const std::uint32_t *source,unsigned stride,unsigned sx,unsigned sy,unsigned sw,unsigned sh,bool dim=false) noexcept;

  private:
    explicit Canvas(std::uint32_t *pixels) noexcept : pixels_{pixels}
    {
    }

    std::uint32_t *pixels_;

    friend void run(Frame frame) noexcept;
};

void read_asset_text(const char *path, std::span<char> destination,
                     std::string_view fallback) noexcept;
void read_asset_bytes(const char *path, std::span<std::uint8_t> destination) noexcept;
} // namespace ps5::demo

