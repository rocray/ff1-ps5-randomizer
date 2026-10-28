# Zelda Randomizer PS5 — Test 03

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

## Randomizer modes
On NEW RANDOM QUEST, press Square to change mode before generating:
- Dungeon Gear: original six optional dungeon gear items, unchanged.
- All Items / Logic v1: 32 permanent equipment and heart-container locations
  across dungeons, sword/letter caves, shop equipment slots, Armos and the coast.
  Includes swords, bow/arrows, ladder, raft, recorder and other progression gear.

Triforce pieces, layouts, entrances, refill supplies, keys, maps/compasses,
shop prices and take-any-heart caves stay fixed. "All Items" means equipment
and heart-container pool, not enemy drops or every consumable in the game.
White/Magical Sword caves keep their original heart requirements even when
holding another item. Buying equipment can require farming rupees; existing
shop prices and renewable supplies are preserved. Keep a reusable weapon.

Each candidate must pass native glitchless progression checks: entrances,
room traversal, locked doors, bomb walls, combat requirements, heart gates,
all eight Triforce pieces, Ganon, Zelda and access to all shuffled locations.
Keys/bombs/bait are treated as renewable only after a matching shop and a
reusable weapon are accessible. Logic assumes normal rupee farming and use
of Zelda's retry/save mechanics; it cannot prevent every player-induced trap
or prove every in-game interaction. Full console playthrough remains pending.
Generation is bounded; failed searches do not save a ROM. Original ROM stays
unchanged. Existing gear seeds remain compatible; mode-specific ROM filenames
and ROM hashes keep the new mode's saves and records separate.

This is not the upstream/tournament Z1 engine; seeds/flags are not compatible.
Only the original US Rev1 ROM is accepted. Do not enter ZELDA as your name:
that activates the unrandomized second quest, outside this logic profile.

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
an original ROM and the known working FF1 Test13 or Zelda Test02 ZIP for its runtime shim.
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

## Test03 validation
1,000 deterministic all-item seeds passed the progression model and byte-mask /
item-pool checks. Deliberate raft, recorder, ladder, weapon and key deadlocks
were rejected. This is automated model coverage, not 1,000 completed playthroughs.
