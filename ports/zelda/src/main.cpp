// SPDX-License-Identifier: GPL-3.0-or-later
#include "demo_renderer.hpp"
#include "nes/nes_core.hpp"
#include "title_music.hpp"
#include "frame_pacer.hpp"
#include "randomizer.hpp"
#include "qol.hpp"
#include <cstdio>
#include <cstring>
#include <new>
#include <time.h>
namespace {
using namespace ps5::demo;
namespace core=ps5::demo::nes;
constexpr Color gold=static_cast<Color>(0xff48cbed),navy=static_cast<Color>(0xff241509),green=static_cast<Color>(0xff72ba39);
core::NesState *game=nullptr;qol::Settings settings;randomizer::Library library;FramePacer pacer;
enum class Screen{menu,game,seeds,library,settings,help,error,generating};Screen screen=Screen::menu;
unsigned selection=0,row=0,slot=0,digit=0;std::uint32_t seed=1;randomizer::Mode mode=randomizer::Mode::Gear;bool generation_drawn=false;bool overlay=true,intro=true,settings_ok=true;
char run_name[64]="ORIGINAL / FIRST QUEST",stamp[64]="NO TIMESTAMP",rom_path[256]{};const char *error="";
std::uint64_t now(){timespec t{};if(clock_gettime(CLOCK_MONOTONIC,&t))return 0;return std::uint64_t(t.tv_sec)*1000000+t.tv_nsec/1000;}
void panel(Canvas&c,unsigned x,unsigned y,unsigned w,unsigned h){c.rectangle(x,y,w,h,gold);c.rectangle(x+3,y+3,w-6,h-6,navy);}
void tri(Canvas&c,unsigned x,unsigned y,unsigned size,Color col){c.triangle(x,y,size/2,size,col);}
void triforce(Canvas&c,unsigned x,unsigned y,unsigned size,Color col){tri(c,x,y,size,col);tri(c,x-size/2,y+size,size,col);tri(c,x+size/2,y+size,size,col);}
void hand(Canvas&c,unsigned x,unsigned y){c.rectangle(x,y+10,24,24,Color::white);c.rectangle(x+16,y+4,30,12,Color::white);c.rectangle(x+20,y+16,12,12,Color::white);}
// Artwork is loaded once; rendering uses the existing cached tiled staging buffer.
static std::uint32_t title_art[1920*1080],overlay_art[1672*941];
static bool art_ready=false;
void load_art(){read_asset_bytes("/app0/assets/title.raw",{reinterpret_cast<std::uint8_t*>(title_art),sizeof(title_art)});read_asset_bytes("/app0/assets/overlay.raw",{reinterpret_cast<std::uint8_t*>(overlay_art),sizeof(overlay_art)});art_ready=(title_art[0]>>24)==255&&(overlay_art[0]>>24)==255;}
void art(Canvas&c,unsigned x,unsigned y,unsigned w,unsigned h,unsigned sx,unsigned sy,unsigned sw,unsigned sh,bool dim=false){c.region(x,y,w,h,overlay_art,1672,sx,sy,sw,sh,dim);}
void icon(Canvas&c,unsigned id,unsigned x,unsigned y,bool on){
 if(art_ready){
 // Source sprite centers in the approved overlay, ordered by live RAM item profile.
 constexpr unsigned xs[]={1265,1265,1265,1265,1265,1265,1265,1265,1468,1468,1468,1468,1468,1468,1468,1468};
 constexpr unsigned ys[]={233,277,320,364,407,451,492,536,233,277,320,364,407,451,492,536};
 art(c,x,y,38,42,xs[id]-19,ys[id]-21,38,42,!on);return;}
 Color a=on?gold:Color::steel,b=on?green:Color::steel;auto box=[&](unsigned xx,unsigned yy,unsigned w,unsigned h,Color col){c.rectangle(x+xx*2,y+yy*2,w*2,h*2,col);};
 switch(id){
 case 0:case 2:case 5:box(6,1,2,12,a);box(3,10,8,2,b);box(5,13,4,3,b);break;
 case 1:box(3,1,2,14,a);box(5,2,3,2,a);box(8,4,2,7,a);box(5,12,3,2,a);break;
 case 3:box(5,5,5,10,on?Color::white:Color::steel);box(6,1,3,3,a);box(4,14,7,2,b);break;
 case 4:box(6,1,3,15,a);for(unsigned j=4;j<14;j+=3)box(7,j,1,1,navy);break;
 case 6:for(unsigned j=2;j<15;j+=3)box(j,3,2,11,a);box(1,6,14,2,b);box(1,11,14,2,b);break;
 case 7:box(2,1,2,15,a);box(11,1,2,15,a);for(unsigned j=2;j<15;j+=4)box(4,j,7,2,b);break;
 case 8:case 12:box(3,2,11,13,a);box(5,3,7,10,navy);box(7,5,3,6,b);break;
 case 9:case 11:box(4,2,7,2,a);box(2,4,2,9,a);box(11,4,2,9,a);box(4,13,7,2,a);box(5,1,5,3,b);break;
 case 10:box(3,1,7,2,a);box(3,3,2,4,a);box(8,3,2,4,a);box(5,7,3,8,a);box(8,11,4,2,b);break;
 case 13:case 14:for(unsigned j=0;j<6;j++)box(3+j,2+j,3,3,a);for(unsigned j=0;j<5;j++)box(8-j,8+j,3,3,b);break;
 case 15:box(2,2,12,9,a);box(4,11,8,3,a);box(6,14,4,2,a);box(7,4,2,8,b);box(4,6,8,2,b);break;
 }}
void scenery(Canvas&c){if(art_ready){c.image(0,0,1920,1080,title_art);return;}c.clear(navy);for(unsigned y=0;y<730;y+=8)c.rectangle(0,y,1920,8,static_cast<Color>(0xff000000|((25+(730-y)*22/730)<<16)|((y/50+12)<<8)|8));
 for(unsigned i=0;i<80;i++)c.rectangle((i*277+101)%1920,(i*97+17)%620,3,3,Color::steel);
 c.circle(1630,140,65,gold);c.circle(1608,120,62,static_cast<Color>(0xff281808));
 for(unsigned i=0;i<8;i++)c.triangle(70+i*280,430+(i%3)*50,240,420,static_cast<Color>(0xff302316));
 c.rectangle(0,840,1920,240,static_cast<Color>(0xff151b0a));for(unsigned i=0;i<20;i++){c.triangle(i*110,690+(i%3)*30,85,245,static_cast<Color>(0xff203515));c.rectangle(i*110,870,14,150,static_cast<Color>(0xff19200c));}
 c.rectangle(186,120,20,260,Color::white);c.rectangle(150,325,94,18,gold);c.rectangle(183,343,26,60,green);triforce(c,1560,280,105,gold);
 c.text_centered(910,100,"THE LEGEND OF",6,gold);c.text_centered(910,170,"ZELDA",20,gold);c.text_centered(910,335,"RANDOMIZER",5,Color::white);
}
void footer(Canvas&c){c.rectangle(0,1010,1920,70,navy);c.text(50,1033,"X SELECT   O BACK",3,Color::white);c.text(1360,1033,"ZELDA / TEST 03",3,gold);}
const char *choices[]={"NEW RANDOM QUEST","SEED LIBRARY","PLAY ORIGINAL","CONTINUE QUEST","SETTINGS","HELP / CONTROLS"};
void menu(Canvas&c){
 if(!art_ready){panel(c,700,470,590,500);panel(c,1340,515,510,455);}
 // Clear baked mockup text and draw only the application's real actions.
 c.rectangle(710,510,563,414,navy);c.rectangle(710,510,4,414,gold);c.rectangle(1350,755,485,195,navy);
 for(unsigned i=0;i<6;i++){unsigned y=535+i*62;if(i==selection){c.rectangle(732,y-12,530,51,static_cast<Color>(0xff284724));hand(c,742,y);}c.text(800,y,choices[i],3,i==selection?Color::white:gold);}
 c.text(1380,773,randomizer::mode_name(unsigned(mode)),3,gold);c.text(1380,816,mode==randomizer::Mode::Gear?"6 DUNGEON GEAR ITEMS":"32 ITEM LOCATIONS",3,Color::white);
 const char*actions[]={"CHOOSE SEED","OPEN LIBRARY","PLAY ORIGINAL","CONTINUE QUEST","OPEN SETTINGS","OPEN GUIDE"};
 panel(c,1375,870,435,61);c.text_centered(1592,891,actions[selection],3,Color::white);
}
void update_stamp(){char path[256];if(randomizer::save_path(library,slot,path,sizeof(path)))qol::read_stamp(path,stamp,sizeof(stamp));else std::strcpy(stamp,"NO TIMESTAMP");}
bool original(){const char *paths[]={"/app0/rom/ZELDA.NES","/data/homebrew/PPSA01988/rom/ZELDA.NES"};for(auto p:paths){if(core::load_rom(*game,p))return true;if(std::strcmp(game->error,"ROM OPEN FAILED"))break;}return false;}
void fail(const char*s){error=s;screen=Screen::error;}
void play_seed(bool generate){const char *err="";bool ok=false;if(generate){const char *paths[]={"/app0/rom/ZELDA.NES","/data/homebrew/PPSA01988/rom/ZELDA.NES"};for(auto p:paths){ok=randomizer::generate(p,seed,slot,library,rom_path,sizeof(rom_path),err,"/download0",mode);if(ok||std::strcmp(err,"BASE ROM MISSING OR WRONG SIZE"))break;}}else ok=randomizer::select(library,slot,rom_path,sizeof(rom_path),err);
 if(!ok){fail(err);return;}if(!core::load_rom(*game,rom_path)){fail(game->error);return;}std::snprintf(run_name,sizeof(run_name),"%s / %08X",library.entries[slot].flags==2?"ALL V1":"GEAR V1",library.entries[slot].seed);intro=false;screen=Screen::game;pacer.reset();}
void seed_page(Canvas&c,bool saved){panel(c,160,435,1560,530);c.text(210,475,saved?"SEED LIBRARY":"NEW RANDOM QUEST",5,gold);char line[96];
 if(saved){for(unsigned i=0;i<8;i++){auto&e=library.entries[i];if(e.crc)std::snprintf(line,sizeof(line),"SLOT %u %08X %s",i+1,e.seed,e.flags==2?"ALL":"GEAR");else std::snprintf(line,sizeof(line),"SLOT %u   EMPTY",i+1);if(i==slot)hand(c,210,550+i*43);c.text(275,550+i*43,line,3,i==slot?Color::white:Color::steel);}c.text(930,580,"SAVE FILE UPDATED",3,Color::steel);c.text(930,630,stamp,3,Color::white);c.text(930,750,"X PLAY   O BACK",3,gold);}
 else{c.text(210,535,mode==randomizer::Mode::Gear?"DUNGEON GEAR: SIX OPTIONAL GEAR ITEMS":"ALL ITEMS: EQUIPMENT + HEARTS / 32 LOCATIONS",3,gold);c.text(210,575,mode==randomizer::Mode::Gear?"PROGRESSION LOCATIONS STAY IN PLACE":"DUNGEONS / CAVES / SHOP GEAR / ARMOS / COAST",3,Color::white);c.text(210,610,"SQUARE CHANGE MODE / REFILLS + TRIFORCE STAY FIXED",3,Color::steel);std::snprintf(line,sizeof(line),"%08X",seed);c.text(240,650,line,9,Color::white);c.rectangle(240+digit*54,720,46,4,gold);std::snprintf(line,sizeof(line),"SLOT %u%s",slot+1,library.entries[slot].crc?" / REPLACES LIST ENTRY":" / EMPTY");c.text(830,670,line,3,gold);c.text(210,780,"D-PAD EDIT SEED   TRIANGLE ROLL   L1 / R1 SLOT",3,Color::white);c.text(210,850,"X GENERATE AND PLAY    O BACK",4,gold);}}
void settings_page(Canvas&c){panel(c,160,435,1560,530);c.text(210,475,"SETTINGS",5,gold);const char *labels[]={"GAME SHAPE","SCANLINES","CRT CURVE","TITLE MUSIC"};const char *shapes[]={"4:3","INTEGER PIXELS","WIDE"};const char *scans[]={"OFF","LIGHT","STRONG"};const char *values[]={shapes[settings.layout%3],scans[settings.scanlines],settings.curve?"ON":"OFF",settings.music?"ON":"OFF"};for(unsigned i=0;i<4;i++){unsigned y=570+i*70;if(i==row)hand(c,210,y);c.text(275,y,labels[i],4,Color::white);c.text(850,y,values[i],4,gold);}c.text(210,890,settings_ok?"LEFT / RIGHT OR X CHANGE   O BACK":"SETTINGS SAVE FAILED",3,Color::steel);}
void help(Canvas&c){panel(c,160,435,1560,530);c.text(210,475,"HOW TO PLAY",5,gold);const char *lines[]={"X NES A / SWORD    CIRCLE NES B / ITEM","OPTIONS START     TOUCHPAD SELECT","REGISTER NAME: X LETTERS; TOUCHPAD TO END; OPTIONS CONFIRM","L3 + R3 SAVE SRAM AND RETURN TO MENU","R1 PAUSE     SQUARE HIDE / SHOW TRACKER","TIMER: FIRST PLAYABLE SCREEN TO ZELDA RESCUE","TRIFORCE AUTO SPLITS; ITEMS SHOW ACQUISITION DUNGEON","USE ZELDA'S OWN SAVE SCREEN; SRAM IS NOT A SAVE STATE","RESUMED SAVES ARE PRACTICE; BEST SPLITS ARE PROTECTED"};for(unsigned i=0;i<9;i++)c.text(210,545+i*42,lines[i],3,Color::white);}
void clock_text(char*out,std::size_t n,double t,bool full=false){unsigned s=t;if(full)std::snprintf(out,n,"%02u:%02u:%02u",s/3600,s/60%60,s%60);else std::snprintf(out,n,"%02u:%02u",s/60,s%60);}
void overlay_background(Canvas&c){
 alignas(64) static std::uint32_t cached[Canvas::tile_words];static bool ready=false;
 if(ready){c.restore_tiles(cached);return;}
 if(art_ready)c.region(0,0,1920,1080,overlay_art,1672,0,0,1672,941);else c.clear(navy);
 // Replace illustrative gameplay and every sample statistic with live content.
 c.rectangle(101,16,1221,823,Color::ink);
 c.rectangle(1480,63,395,105,navy);c.rectangle(1440,145,442,25,navy);
 c.rectangle(1430,239,458,422,navy);c.rectangle(1428,715,460,278,navy);
 c.rectangle(120,869,1268,146,navy);
 c.copy_tiles(cached);ready=true;
}
struct View{unsigned x,y,w,h;};
View game_view(){if(!overlay){if(settings.layout==1)return {448,60,1024,960};if(settings.layout==2)return {0,0,1920,1080};return {240,0,1440,1080};}
 if(settings.layout==1)return {328,67,768,720};if(settings.layout==2)return {101,84,1221,687};return {164,16,1096,822};}
void tracker(Canvas&c){auto&r=game->progress;char t[100];
 if(!art_ready){panel(c,1408,0,510,179);panel(c,1408,185,510,480);panel(c,1408,680,510,325);}
 clock_text(t,sizeof(t),r.seconds,true);c.text_centered(1660,78,t,6,green);
 c.text(1450,137,run_name,2,gold);c.text(1450,157,r.finished?"ZELDA RESCUED":!r.started?"WAITING FOR GAMEPLAY":!game->running?"PAUSED":r.practice?"PRACTICE / RESUMED SAVE":"RUNNING",2,Color::white);
 for(unsigned i=0;i<16;i++){unsigned col=i<8?0:1,j=i%8,x=1435+col*230,y=250+j*50;icon(c,i,x,y,r.seen[i]!=0);c.text(x+44,y+4,item_names[i],2,r.seen[i]?Color::white:Color::steel);if(!r.seen[i])std::strcpy(t,"--");else if(r.found_level[i]>0)std::snprintf(t,sizeof(t),"L%u / %u",unsigned(r.found_level[i]),r.items[i]);else std::snprintf(t,sizeof(t),"%s / %u",r.found_level[i]==0?"OW":"--",r.items[i]);c.text(x+44,y+24,t,2,r.seen[i]?gold:Color::steel);}
 for(unsigned i=0;i<9;i++){unsigned y=727+i*29;c.text(1445,y,split_names[i],2,Color::white);if(r.reached[i])clock_text(t,sizeof(t),r.split[i]);else if(r.best[i])clock_text(t,sizeof(t),r.best[i]);else std::strcpy(t,"--:--");c.text(1640,y,t,2,r.reached[i]?Color::white:Color::steel);Color col=Color::steel;std::strcpy(t,"--:--");if(r.reached[i]&&r.best[i]){double d=r.split[i]-r.best[i];unsigned dt=d<0?-d:d;std::snprintf(t,sizeof(t),"%c%u:%02u",d<=0?'-':'+',dt/60,dt%60);col=d<=0?green:Color::bad;}c.text(1790,y,t,2,col);}
 for(unsigned i=0;i<9;i++){unsigned x=155+i*112;bool won=i<8?(r.triforce&(1u<<i)):r.finished;if(art_ready){art(c,x,911,81,71,130,794,70,64,!won);if(!won&&(r.visited&(1u<<i)))c.rectangle(x+6,985,67,3,green);}else tri(c,x+40,911,63,won?gold:Color::steel);std::snprintf(t,sizeof(t),"%u",i+1);c.text_centered(x+40,990,t,3,won?gold:Color::white);}
 std::snprintf(t,sizeof(t),"HEARTS %u",r.hearts);c.text(1190,893,t,2,Color::white);std::snprintf(t,sizeof(t),"RUPEES %u",r.rupees);c.text(1190,929,t,2,green);std::snprintf(t,sizeof(t),"K %u B %u",r.keys,r.bombs);c.text(1190,965,t,2,gold);
}
void draw_game(Canvas&c){if(overlay)overlay_background(c);else c.clear(navy);auto v=game_view();const auto*pixels=settings.curve?qol::curve_frame(game->framebuffer):game->framebuffer;c.region(v.x,v.y,v.w,v.h,pixels,256,0,0,256,240,false,settings.scanlines);if(overlay)tracker(c);if(game->error[0])c.text(110,20,game->error,3,Color::bad);}
void frame(Canvas&c,const Input&input)noexcept{
 using namespace pad;music::start();music::set_active(false);bool menu_intro=intro&&game->loaded&&screen!=Screen::game&&screen!=Screen::generating;music::set_game((screen==Screen::game&&game->running)||(menu_intro&&settings.music));
 if(screen==Screen::game){if(menu_chord(input.held,input.pressed)){core::save_ram(*game);screen=Screen::menu;pacer.reset();music::set_game(false);}else{if(input.pressed&r1)game->running=!game->running;if(!input.connected)game->running=false;if(input.pressed&square)overlay=!overlay;}}
 if(screen==Screen::game||menu_intro){unsigned due=pacer.advance(now(),game->fps,game->running);for(unsigned i=0;i<due;i++)core::run_frame(*game,screen==Screen::game?input.held:0);}else pacer.reset();
 if(screen==Screen::game){draw_game(c);return;}
 scenery(c);if(screen!=Screen::menu)c.rectangle(0,425,1920,585,navy);switch(screen){
 case Screen::menu:if(input.pressed&up)selection=(selection+5)%6;if(input.pressed&down)selection=(selection+1)%6;if(input.pressed&cross){switch(selection){case 0:seed=now();screen=Screen::seeds;break;case 1:randomizer::load_library(library);update_stamp();screen=Screen::library;break;case 2:if(original()){std::strcpy(run_name,"ORIGINAL / FIRST QUEST");intro=false;screen=Screen::game;}else fail(game->error);pacer.reset();break;case 3:if(game->loaded&&!intro){screen=Screen::game;game->running=true;}else fail("NO ACTIVE QUEST");pacer.reset();break;case 4:screen=Screen::settings;break;default:screen=Screen::help;}}menu(c);break;
 case Screen::seeds:if(input.pressed&left)digit=(digit+7)%8;if(input.pressed&right)digit=(digit+1)%8;{unsigned shift=(7-digit)*4,n=(seed>>shift)&15;if(input.pressed&up)n=(n+1)&15;if(input.pressed&down)n=(n+15)&15;seed=(seed&~(15u<<shift))|(n<<shift);}if(input.pressed&triangle)seed=now();if(input.pressed&l1)slot=(slot+7)%8;if(input.pressed&r1)slot=(slot+1)%8;if(input.pressed&square)mode=mode==randomizer::Mode::Gear?randomizer::Mode::AllItems:randomizer::Mode::Gear;if(input.pressed&cross){generation_drawn=false;screen=Screen::generating;}if(input.pressed&circle)screen=Screen::menu;seed_page(c,false);break;
 case Screen::library:if(input.pressed&up)slot=(slot+7)%8;if(input.pressed&down)slot=(slot+1)%8;if(input.pressed&(up|down))update_stamp();if(input.pressed&cross)play_seed(false);if(input.pressed&circle)screen=Screen::menu;seed_page(c,true);break;
 case Screen::generating:panel(c,160,435,1560,530);c.text(250,560,"GENERATING AND CHECKING SEED",4,gold);c.text(250,650,"ONLY VALID PLACEMENTS ARE SAVED",3,Color::white);if(generation_drawn)play_seed(true);else generation_drawn=true;break;
 case Screen::settings:if(input.pressed&up)row=(row+3)%4;if(input.pressed&down)row=(row+1)%4;if(input.pressed&(left|right|cross)){unsigned*v=row==0?&settings.layout:row==1?&settings.scanlines:row==2?&settings.curve:&settings.music;unsigned count=row<2?3:2;*v=(*v+((input.pressed&left)?count-1:1))%count;settings_ok=qol::save(settings);}if(input.pressed&circle)screen=Screen::menu;settings_page(c);break;
 case Screen::help:if(input.pressed&(circle|cross))screen=Screen::menu;help(c);break;
 case Screen::error:panel(c,300,500,1320,300);c.text(350,550,"PLEASE CHECK",5,gold);c.text(350,640,error,3,Color::white);c.text(350,730,"O BACK",3,Color::steel);if(input.pressed&(circle|cross))screen=Screen::menu;break;
 default:break;}footer(c);
}
}
int main(){load_art();game=new(std::nothrow)core::NesState;if(!game)return 1;qol::load(settings);if(settings.layout>2)settings.layout=0;randomizer::load_library(library);original();ps5::demo::run(frame);}
