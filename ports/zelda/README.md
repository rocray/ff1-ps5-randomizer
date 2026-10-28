# Zelda Randomizer PS5 — Test 02

Separate PS5 title PPSA01988. Extract the complete ZIP, select its PPSA01988 folder
in PS5Upload, upload to /data/homebrew/PPSA01988 and register on the home screen.
Do not overwrite FF1's PPSA01987 folder. Both executable and libc.prx are signed
SELF containers, preserving the confirmed FF1 launch fix.

## Included
- Illustrated Hyrule title screen, original NES title audio while the initial menu is open,
  the approved illustrated launcher icon and backgrounds. No home-screen selection music is included.
- NES FCEUmm emulator with the working FF1 allocation, video, pacing and audio fixes.
- 4:3 by default; settings for integer pixels/wide, scanlines and mild CRT curvature.
- Live inventory, acquisition dungeon (OW = overworld, -- = unknown), dungeon visits,
  Triforce pieces, hearts/counters, automatic timer and per-ROM best milestone times.
- Timer starts on first playable screen, ends on rescue sequence. Existing quests
  count as practice and do not replace best times. These are per-milestone best
  elapsed times, not a complete personal-best run comparison.
- Eight seed slots, separate per-ROM SRAM, save-write timestamps (UTC).

## Initial randomizer scope
Dungeon Gear Shuffle v1 runs entirely on PS5. It shuffles six optional dungeon
gear items: wooden/magic boomerang, wand, red ring, red candle and book.
Critical progression items, dungeon layouts, entrances, Triforce, shops and caves
stay unchanged. This is NOT the full upstream/tournament Z1 randomizer and its
seeds/flags are not compatible. The supplied original US Rev 1 ROM is validated
before generation. Original ROM is never modified. Full-run console testing pending.

## Controls
X = NES A (sword), Circle = NES B (item), Options = Start, Touchpad = Select.
Square hides/shows tracker. R1 pauses/resumes emulation.
Click BOTH thumbsticks (L3+R3) to write SRAM and return to the app menu.
Continue Quest resumes the in-memory session. Library loads a saved seed's ROM.
Use the game's own save system: writing SRAM is not an emulator save state.
For name registration: X chooses letters; Touchpad moves selection to END;
Options confirms. The Help screen includes these controls.

## Source/build
Source: ports/zelda/ on codex/fceumm-tracker-recovery in the shared repository.
Requires the root project's configured PS5 toolchain, startup objects and FCEUmm
static archive. Run bash ports/zelda/build.sh. Host tests:
  bash ports/zelda/tests/run.sh /path/to/original-US-Rev1.nes
Assets are generated from the native UI with tests/render_ui.cpp; package.py takes
an original ROM and the known working FF1 Test13 ZIP for its runtime shim.
ROM, music and SDK files must never be committed to GitHub.

References: tetraly/zelda-randomizer (aa507c7) room item-table format;
aldonunez/zelda1-disassembly (50a1c86) RAM symbols/mode table. Native preset is
independently implemented; no full upstream randomizer engine is included.
GPL-3.0-or-later application; FCEUmm retains its own repository license notices.

## Test02 artwork update
The approved title, icon and overlay concepts are integrated. Menu actions,
timer, seed label, inventory, acquisition locations, splits and dungeon badges
are rendered live. Sample mockup gameplay and stats are covered. The NES image
remains original and 4:3 by default; integer and wide settings are retained.
Static overlay artwork is cached in tiled RAM once; dynamic content renders over
it. Host overlay render measurement: about 2.5 ms/frame (not PS5 measurement).
Use the full PPSA01988 folder, including assets/title.raw and assets/overlay.raw.
Save/resume behavior is unchanged from Test01; this is the graphics update.
