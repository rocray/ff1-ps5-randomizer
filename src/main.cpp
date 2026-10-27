/*
 * FF1 Randomizer - PS5 home menu.
 * Copyright (c) 2026
 *
 * Implements the main menu mockup: title, left navigation list, right
 * "New Adventure" detail panel, and button-hint footer.
 */

#include "demo_renderer.hpp"
#include "title_music.hpp"
#include "frame_pacer.hpp"
#include <time.h>
#include "nes/nes_core.hpp"

#include <cstdint>
#include <new>
#include <cstdio>
#include <cstring>

namespace
{
using ps5::demo::Canvas;
using ps5::demo::Color;
using ps5::demo::Input;
namespace music = ps5::demo::music;
using ps5::demo::nes::NesState;

constexpr unsigned screen_width = 1920;
constexpr unsigned screen_height = 1080;
constexpr std::size_t background_bytes =
    static_cast<std::size_t>(screen_width) * screen_height * 4;

std::uint32_t *background = nullptr;
NesState *nes = nullptr;
std::uint32_t *scaled_game = nullptr;
static std::uint32_t icon_art[1672*941];
bool show_overlay=true;
FramePacer pacer;
std::uint64_t monotonic_us() noexcept {
    timespec t{};
    if(clock_gettime(CLOCK_MONOTONIC,&t)!=0)return 0;
    return static_cast<std::uint64_t>(t.tv_sec)*1000000+t.tv_nsec/1000;
}

enum class Screen
{
    menu,
    message,
    help,
    gameplay,
};

Screen screen = Screen::menu;
unsigned selection = 0;
bool music_enabled = true;
const char *message_title = "";
const char *message_line = "";

constexpr const char *menu_items[4] = {
    "PLAY FF1",
    "CONTINUE RUN",
    "TITLE MUSIC",
    "HELP / OVERLAY",
};

void open_message(const char *title, const char *line) noexcept
{
    message_title = title;
    message_line = line;
    screen = Screen::message;
}

bool load_available_rom() {
    if(!nes || !scaled_game) return false;
    const char *paths[]={"/app0/rom/FF1.NES","/app0/assets/FF1.NES",
        "/data/homebrew/PPSA01987/rom/FF1.NES",
        "/data/homebrew/PPSA01987/assets/FF1.NES"};
    for(const char *path:paths){
        if(ps5::demo::nes::load_rom(*nes,path)) return true;
        // A readable but rejected ROM must not be masked by a missing fallback.
        if(std::strcmp(nes->error,"ROM OPEN FAILED")!=0) return false;
    }
    return false;
}

void draw_panel(Canvas &canvas, unsigned x, unsigned y, unsigned width, unsigned height) noexcept
{
    // Paint each border band only once rather than six full overlapping panels.
    const unsigned edges[]={0,2,4,7,10,12};
    const Color colors[]={Color::ink,Color::steel,Color::white,Color::steel,Color::ink};
    for(unsigned i=0;i<5;i++){
        unsigned d=edges[i],n=edges[i+1]-d;
        canvas.rectangle(x+d,y+d,width-2*d,n,colors[i]);
        canvas.rectangle(x+d,y+height-d-n,width-2*d,n,colors[i]);
        canvas.rectangle(x+d,y+d+n,n,height-2*(d+n),colors[i]);
        canvas.rectangle(x+width-d-n,y+d+n,n,height-2*(d+n),colors[i]);
    }
    for(unsigned row=12;row<height-12;row++){
        unsigned blue=85+(height-row)*35/height;
        canvas.rectangle(x+12,y+row,width-24,1,static_cast<Color>(0xff000000|(blue<<16)|(10<<8)|4));
    }
}

void draw_button_footer(Canvas &canvas) noexcept
{
    canvas.rectangle(0,990,1920,90,Color::navy_deep);
    canvas.rectangle(0,990,1920,2,Color::steel);
    canvas.circle(72,1030,22,Color::white);canvas.text(62,1018,"X",3,Color::navy_deep);
    canvas.text(115,1018,"SELECT",4,Color::white);
    canvas.circle(322,1030,20,Color::steel);canvas.circle(322,1030,14,Color::bad);
    canvas.text(360,1018,"BACK",4,Color::white);
    canvas.text(1370,1020,"ORIGINAL FF1 / TEST 11",3,Color::steel);
}

void draw_title(Canvas &canvas) noexcept
{
    if(!background){canvas.text(120,65,"FINAL FANTASY",9,Color::white);canvas.text(280,190,"RANDOMIZER",5,Color::gold);}
}

void draw_hand(Canvas &canvas,unsigned x,unsigned y) noexcept
{
    constexpr const char *rows[]={
        "     ####       ","    #WWWW###### ","    #WWWWWWWWWW#",
        " ####WWWW###### ","#WWWWWWWW#      ","#WWWWWWWWW#     ",
        " #WWWWWWWW#     ","  #WWWWWWW#     ","  ########      "};
    for(unsigned yy=0;yy<9;yy++)for(unsigned xx=0;xx<16;xx++)
        if(rows[yy][xx]!=' ')canvas.rectangle(x+xx*4,y+yy*4,4,4,rows[yy][xx]=='W'?Color::white:Color::ink);
}
void draw_help(Canvas &canvas) noexcept
{
    canvas.rectangle(60,345,1740,615,Color::navy_deep);
    draw_panel(canvas,85,350,660,600);draw_panel(canvas,770,350,1010,600);
    canvas.text(120,385,"OVERLAY GUIDE",5,Color::white);
    canvas.text(120,450,"RUN TIME",3,Color::steel);
    canvas.text(120,495,"00:24:18",8,Color::white);
    canvas.rectangle(110,585,610,2,Color::white);
    canvas.text(120,620,"MILESTONE   TIME   DELTA",3,Color::white);
    canvas.text(120,670,"SHIP        08:42  -0:18",3,Color::good);
    canvas.text(120,715,"CANOE       21:16  +0:32",3,Color::bad);
    canvas.text(120,800,"SAMPLE VALUES - NOT A LIVE RUN",3,Color::gold);
    canvas.text(120,850,"GREEN: AHEAD / RED: BEHIND",3,Color::white);
    const char *lines[]={"HOW TO READ YOUR RUN","AUTO START: FIRST PLAYABLE OVERWORLD",
      "SHIP / CANOE / AIRSHIP / ORBS / CHAOS","SPLITS ARE ELAPSED TIMES FROM START",
      "DELTA COMPARES WITH YOUR BEST SPLIT","DIM TIMES SHOW YOUR PREVIOUS BEST",
      "ITEM ICONS LIGHT UP WHEN ACQUIRED","QUEST PROGRESS AT START = PRACTICE",
      "PRACTICE DOES NOT REPLACE YOUR BEST","R1 PAUSES GAME AND TIMER",
      "SQUARE SHOWS / HIDES THE OVERLAY","L3 + R3 SAVES SRAM AND OPENS MENU",
      "OPTIONS = NES START / TOUCH = SELECT"};
    for(unsigned i=0;i<13;i++)canvas.text(805,385+i*38,lines[i],3,i?Color::white:Color::gold);
    canvas.text(805,910,"O BACK TO MENU",3,Color::steel);
}

void draw_menu(Canvas &canvas) noexcept
{
    draw_panel(canvas,85,350,660,390);
    for(unsigned i=0;i<4;i++){
        unsigned yy=430+i*73;
        if(i==selection){
            canvas.rectangle(165,yy-20,545,65,Color::white);
            canvas.rectangle(170,yy-15,535,55,static_cast<Color>(0xffff3018));
            draw_hand(canvas,120,yy-5);
        }
        canvas.text(225,yy,menu_items[i],4,i==selection?Color::white:Color::steel);
    }
    draw_panel(canvas,770,350,620,560);
    canvas.text_centered(1080,390,selection==3?"HELP / OVERLAY":"NEW ADVENTURE",4,Color::white);
    canvas.rectangle(810,433,540,2,Color::white);
    // Reuse the approved artwork as an atlas; dynamic text is drawn separately.
    if(background)canvas.region(855,463,440,105,background,1920,855,463,440,105);
    const char *details[4][4]={
        {"MODE: ORIGINAL FF1","TIMER: AUTO AT OVERWORLD","SPLITS: BEST MILESTONE TIMES","ROM: FF1.NES"},
        {"RESUME THIS APP SESSION","YOUR CLOCK AND ITEMS REMAIN","L3 + R3 RETURNS TO MENU",""},
        {"LOOPING PRELUDE", "PRESS X TO TOGGLE MUSIC","GAME AUDIO IS ALWAYS ENABLED",music::status()},
        {"TIMER / SPLITS / ITEMS","VIEW THE OVERLAY GUIDE","SQUARE: TOGGLE GAME OVERLAY","R1: PAUSE   L3 + R3: MENU"}
    };
    for(unsigned i=0;i<4;i++)canvas.text(820,590+i*43,details[selection][i],3,Color::white);
    canvas.rectangle(810,785,540,2,Color::white);
    canvas.rectangle(810,810,540,70,Color::white);canvas.rectangle(816,816,528,58,static_cast<Color>(0xffff3018));
    const char *actions[]={"PLAY FF1",nes&&nes->loaded?"CONTINUE RUN":"NO ACTIVE RUN",music_enabled?"MUSIC: ON":"MUSIC: OFF","OPEN GUIDE"};
    canvas.text_centered(1080,835,actions[selection],4,Color::white);
}

void time_text(char *out,std::size_t n,double seconds,bool hours=false) {
    unsigned t=static_cast<unsigned>(seconds);
    if(hours)std::snprintf(out,n,"%02u:%02u:%02u",t/3600,(t/60)%60,t%60);
    else std::snprintf(out,n,"%02u:%02u",t/60,t%60);
}
void draw_overlay(Canvas &canvas) noexcept {
    auto &t=nes->tracker;auto &r=nes->progress;char text[80];
    draw_panel(canvas,1380,0,540,250);canvas.text(1410,27,"RUN TRACKER",5,Color::white);
    canvas.rectangle(1400,70,500,2,Color::white);
    time_text(text,sizeof(text),t.seconds,true);canvas.text(1410,100,text,8,Color::white);
    std::snprintf(text,sizeof(text),".%02u",static_cast<unsigned>(t.seconds*100)%100);canvas.text(1795,125,text,4,Color::white);
    const char *status=r.finished?"FINISHED":!r.started?"AUTO: WAITING FOR OVERWORLD":!nes->running?"PAUSED":r.practice?"RESUMED SAVE / PRACTICE":"ORIGINAL FF1 / RUNNING";
    canvas.text(1410,205,status,3,Color::steel);
    draw_panel(canvas,1380,255,540,475);canvas.text(1410,282,"MILESTONE    TIME    DELTA",3,Color::white);
    canvas.rectangle(1400,322,500,2,Color::white);
    for(unsigned i=0;i<SPLIT_COUNT;i++){
        unsigned y=350+i*43;canvas.text(1410,y,split_names[i],3,Color::white);
        if(r.reached[i])time_text(text,sizeof(text),r.split[i]);else if(r.best[i])time_text(text,sizeof(text),r.best[i]);else std::strcpy(text,"--:--");
        canvas.text(1620,y,text,3,r.reached[i]?Color::white:Color::steel);
        Color c=Color::steel;std::strcpy(text,"--:--");
        if(r.reached[i]&&r.best[i]){double delta=r.split[i]-r.best[i];unsigned d=static_cast<unsigned>(delta<0?-delta:delta);std::snprintf(text,sizeof(text),"%c%u:%02u",delta<=0?'-':'+',d/60,d%60);c=delta<=0?Color::good:Color::bad;}
        canvas.text(1780,y,text,3,c);
    }
    canvas.text(1410,704,"DIM TIMES = SAVED BEST",2,Color::steel);
    draw_panel(canvas,1380,738,540,270);canvas.text(1410,765,"RECENT EVENTS",4,Color::white);canvas.rectangle(1400,807,500,2,Color::white);
    struct Event {double time;unsigned index;bool split;};Event events[ITEM_COUNT+SPLIT_COUNT];unsigned count=0;
    for(unsigned i=0;i<ITEM_COUNT;i++)if(t.seen[i])events[count++]={t.acquired[i],i,false};
    for(unsigned i=3;i<SPLIT_COUNT;i++)if(r.reached[i])events[count++]={r.split[i],i,true};
    for(unsigned i=0;i<count;i++)for(unsigned j=i+1;j<count;j++)if(events[j].time>events[i].time){auto e=events[i];events[i]=events[j];events[j]=e;}
    for(unsigned i=0;i<count&&i<3;i++){
        auto &e=events[i];const char *name=e.split?split_names[e.index]:item_names[e.index];
        std::snprintf(text,sizeof(text),"%s%s",e.split?"":"GOT ",name);canvas.text(1410,832+i*45,text,3,e.split?Color::good:Color::white);
        time_text(text,sizeof(text),e.time);canvas.text(1780,832+i*45,text,3,Color::steel);
    }
    draw_panel(canvas,0,756,1376,324);canvas.text(28,780,"ITEM TRACKER",4,Color::white);canvas.rectangle(16,825,1340,2,Color::white);
    const unsigned centers[]={65,140,215,285,357,430,498,571,645,714,786,858,931,1002,1074,1147};
    for(unsigned i=0;i<16;i++){
        unsigned x=30+i*83;
        canvas.region(x,840,56,58,icon_art,1672,centers[i]-25,720,50,56,!t.seen[i]);
        canvas.text_centered(x+28,910,item_names[i],2,t.seen[i]?Color::white:Color::steel);
    }
    canvas.rectangle(16,945,1340,2,Color::white);canvas.text(30,995,"ORBS",3,Color::white);
    for(unsigned i=0;i<4;i++){
        unsigned x=180+i*110;canvas.region(x,960,62,64,icon_art,1672,168,825,56,60,!r.orb[i]);
        const char *names[]={"EARTH","FIRE","WATER","AIR"};canvas.text_centered(x+31,1036,names[i],2,Color::white);
    }
    canvas.text(650,995,"VEHICLES",3,Color::white);
    const unsigned vx[]={803,920,1045};
    for(unsigned i=0;i<3;i++){
        unsigned x=845+i*165;canvas.region(x,957,90,67,icon_art,1672,vx[i]-43,827,86,64,!t.seen[16+i]);canvas.text_centered(x+45,1036,item_names[16+i],2,Color::white);
    }
    canvas.text(1395,1028,"SQUARE OVERLAY / R1 PAUSE",2,Color::steel);
    canvas.text(1395,1055,nes->records_saved?"L3 + R3 MENU / TEST 11":"BEST SAVE FAILED",2,nes->records_saved?Color::steel:Color::bad);
}

void draw_message(Canvas &canvas) noexcept
{
    draw_panel(canvas, 460, 420, 1000, 240);
    canvas.text_centered(960, 456, message_title, 5, Color::gold);
    std::string_view detail(message_line);
    for(unsigned row=0;row<3 && row*48<detail.size();row++)
        canvas.text(490,520+row*24,detail.substr(row*48,48),3,Color::white);
    canvas.text_centered(960, 604, "PRESS (O) BACK", 4, Color::steel);
}

void draw_frame(Canvas &canvas, const Input &input) noexcept
{
    music::start();
    if(screen!=Screen::gameplay)pacer.reset();
    music::set_game(screen==Screen::gameplay && nes && nes->running);
    music::set_active(music_enabled && screen != Screen::gameplay);
    if (screen != Screen::gameplay && background != nullptr)
        canvas.image(0, 0, screen_width, screen_height, background);
    else
        canvas.clear(Color::background);

    if (screen != Screen::gameplay) draw_title(canvas);

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
                if (load_available_rom())
                {
                    screen = Screen::gameplay;
                }
                else
                {
                    open_message(nes && nes->core_message[0] ? "CORE LOAD FAILED" : "ROM LOAD FAILED", nes ? nes->error : "OUT OF MEMORY");
                }
                break;
            case 1:
                if(nes && nes->loaded) screen=Screen::gameplay;
                else open_message("NO ACTIVE RUN", "SELECT PLAY FF1 FIRST");
                break;
            case 2:
                music_enabled = !music_enabled;
                break;
            default:
                screen=Screen::help;
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
    else if (screen == Screen::help)
    {
        draw_help(canvas);
        if(input.pressed & ps5::demo::pad::circle) screen=Screen::menu;
    }
    else if (screen == Screen::gameplay)
    {
        if (nes && nes->loaded && scaled_game) {
            if(menu_chord(input.held,input.pressed)) {
                ps5::demo::nes::save_ram(*nes);pacer.reset();music::set_game(false);screen=Screen::menu;return;
            }
            if(input.pressed & ps5::demo::pad::square)show_overlay=!show_overlay;
            if(input.pressed & ps5::demo::pad::r1){nes->running=!nes->running;pacer.reset();}
            if(!input.connected)nes->running=false;
            music::set_game(nes->running);
            unsigned due=pacer.advance(monotonic_us(),nes->fps,nes->running);
            for(unsigned i=0;i<due;i++)ps5::demo::nes::run_frame(*nes,input.held);
            if(show_overlay){canvas.region(0,0,1376,756,nes->framebuffer,256,0,0,256,240);draw_overlay(canvas);}
            else canvas.region(240,0,1440,1080,nes->framebuffer,256,0,0,256,240);
            if(nes->error[0])canvas.text(20,10,nes->error,3,Color::bad);
        }
    }

    music::set_active(music_enabled && screen != Screen::gameplay);
    music::set_game(screen==Screen::gameplay && nes && nes->running);
    if(screen != Screen::gameplay) draw_button_footer(canvas);
}
} // namespace

int main()
{
    background = new (std::nothrow) std::uint32_t[background_bytes / 4];
    nes = new (std::nothrow) NesState();
    scaled_game = new (std::nothrow) std::uint32_t[1280*960];
    if (background != nullptr)
        ps5::demo::read_asset_bytes("/app0/assets/bg.raw",
                                    {reinterpret_cast<std::uint8_t *>(background),
                                     background_bytes});
    ps5::demo::read_asset_bytes("/app0/assets/overlay.raw",{reinterpret_cast<std::uint8_t*>(icon_art),sizeof(icon_art)});
    ps5::demo::run(draw_frame);
}

