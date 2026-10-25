/*
 * FF1 Randomizer - PS5 home menu.
 * Copyright (c) 2026
 *
 * Implements the main menu mockup: title, left navigation list, right
 * "New Adventure" detail panel, and button-hint footer.
 */

#include "demo_renderer.hpp"
#include "nes/nes_core.hpp"

#include <cstdint>
#include <new>

namespace
{
using ps5::demo::Canvas;
using ps5::demo::Color;
using ps5::demo::Input;
using ps5::demo::nes::NesState;

constexpr unsigned screen_width = 1920;
constexpr unsigned screen_height = 1080;
constexpr std::size_t background_bytes =
    static_cast<std::size_t>(screen_width) * screen_height * 4;

std::uint32_t *background = nullptr;
NesState *nes = nullptr;

enum class Screen
{
    menu,
    message,
    gameplay,
};

Screen screen = Screen::menu;
unsigned selection = 0;
const char *message_title = "";
const char *message_line = "";

constexpr const char *menu_items[4] = {
    "NEW RANDOMIZED RUN",
    "CONTINUE RUN",
    "SEED LIBRARY",
    "SETTINGS",
};

void open_message(const char *title, const char *line) noexcept
{
    message_title = title;
    message_line = line;
    screen = Screen::message;
}

void draw_panel(Canvas &canvas, unsigned x, unsigned y, unsigned width, unsigned height) noexcept
{
    canvas.rectangle(x, y, width, height, Color::border);
    canvas.rectangle(x + 4, y + 4, width - 8, height - 8, Color::navy);
}

void draw_button_footer(Canvas &canvas) noexcept
{
    canvas.rectangle(130, 1006, 26, 26, Color::white);
    canvas.text(137, 1012, "X", 3, Color::navy);
    canvas.text(166, 1008, "SELECT", 4, Color::white);

    canvas.circle(448, 1019, 14, Color::white);
    canvas.text(474, 1008, "BACK", 4, Color::white);

    canvas.triangle(770, 1004, 15, 28, Color::white);
    canvas.text(796, 1008, "SEED", 4, Color::white);
}

void draw_title(Canvas &canvas) noexcept
{
    canvas.text_centered(960, 56, "FINAL FANTASY", 8, Color::white);
    canvas.text_centered(960, 142, "RANDOMIZER", 4, Color::gold);
    canvas.rectangle(760, 186, 400, 4, Color::gold);
}

void draw_menu(Canvas &canvas) noexcept
{
    draw_panel(canvas, 140, 300, 660, 400);
    for (unsigned index = 0; index < 4; ++index)
    {
        const unsigned row_y = 350 + index * 90;
        if (index == selection)
        {
            canvas.rectangle(160, row_y - 14, 620, 64, Color::highlight);
            canvas.text(176, row_y - 2, ">", 5, Color::white);
            canvas.text(220, row_y, menu_items[index], 5, Color::white);
        }
        else
        {
            canvas.text(220, row_y, menu_items[index], 5, Color::steel);
        }
    }

    draw_panel(canvas, 880, 300, 900, 400);
    canvas.text_centered(1330, 330, "NEW ADVENTURE", 5, Color::gold);
    canvas.rectangle(1135, 376, 390, 3, Color::border);

    canvas.text(930, 410, "PRESET: CLASSIC SHUFFLE", 4, Color::white);
    canvas.text(930, 452, "SEED: A7C319F2", 4, Color::white);
    canvas.text(930, 494, "PARTY: FIGHTER / THIEF /", 4, Color::white);
    canvas.text(930, 536, "      WHITE MAGE / BLACK MAGE", 4, Color::white);
    canvas.text(930, 578, "ROM: FF1 NOT DETECTED", 4, Color::steel);

    const Color button_border = selection == 0 ? Color::highlight : Color::border;
    canvas.rectangle(980, 606, 700, 64, button_border);
    canvas.rectangle(984, 610, 692, 56, Color::navy_deep);
    canvas.text_centered(1330, 624, "GENERATE & PLAY", 5, Color::white);
}

void draw_message(Canvas &canvas) noexcept
{
    draw_panel(canvas, 460, 420, 1000, 240);
    canvas.text_centered(960, 456, message_title, 5, Color::gold);
    canvas.text_centered(960, 530, message_line, 4, Color::white);
    canvas.text_centered(960, 604, "PRESS (O) BACK", 4, Color::steel);
}

void draw_frame(Canvas &canvas, const Input &input) noexcept
{
    if (background != nullptr)
        canvas.image(0, 0, screen_width, screen_height, background);
    else
        canvas.clear(Color::background);

    draw_title(canvas);

    if (screen == Screen::menu)
    {
        if ((input.pressed & ps5::demo::pad::up) != 0)
            selection = (selection + 3) % 4;
        if ((input.pressed & ps5::demo::pad::down) != 0)
            selection = (selection + 1) % 4;
        if ((input.pressed & ps5::demo::pad::cross) != 0)
        {
            switch (selection)
            {
            case 0:
                if (ps5::demo::nes::load_rom(*nes, "/app0/assets/FF1.NES") || 
                    ps5::demo::nes::load_rom(*nes, "/data/homebrew/PPSA01987/assets/FF1.NES"))
                {
                    screen = Screen::gameplay;
                }
                else
                {
                    open_message("ROM NOT FOUND", "PLEASE PLACE FF1.NES AT /app0/assets/");
                }
                break;
            case 1:
                open_message("NO SAVE DATA", "START A NEW RANDOMIZED RUN");
                break;
            case 2:
                open_message("SEED LIBRARY", "COMING SOON");
                break;
            default:
                open_message("SETTINGS", "COMING SOON");
                break;
            }
        }
        draw_menu(canvas);
    }
    else if (screen == Screen::message)
    {
        if ((input.pressed & ps5::demo::pad::circle) != 0 ||
            (input.pressed & ps5::demo::pad::cross) != 0)
            screen = Screen::menu;
        draw_menu(canvas);
        draw_message(canvas);
    }
    else if (screen == Screen::gameplay)
    {
        if (nes) {
            ps5::demo::nes::run_frame(*nes, input.held);
            if (nes->framebuffer) {
                canvas.image(704, 300, 512, 480, nes->framebuffer); 
            }
        }
        
        if ((input.pressed & ps5::demo::pad::circle) != 0)
            screen = Screen::menu;
    }

    draw_button_footer(canvas);
}
} // namespace

int main()
{
    background = new (std::nothrow) std::uint32_t[background_bytes / 4];
    nes = new (std::nothrow) NesState();
    if (background != nullptr)
        ps5::demo::read_asset_bytes("/app0/assets/bg.raw",
                                    {reinterpret_cast<std::uint8_t *>(background),
                                     background_bytes});
    ps5::demo::run(draw_frame);
}
