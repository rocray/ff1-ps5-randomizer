# FF1 Randomizer — Complete Test14

Extract the ZIP and upload the whole PPSA01987 folder with PS5Upload, overwriting
FF1's existing folder. Register it on the home screen. Keep /download0 saves.
PPSA01988 is Zelda; do not overwrite that folder. Close the app before uploading.
This package includes the existing music, artwork, original ROM and tested libc.

## New Random Run
Square switches between seed editing and checkboxes. In the checkbox panel,
Up/Down selects a row; X or Left/Right toggles it. Square returns to the seed;
X then generates and plays. Triangle rolls the seed; L1/R1 selects a library slot.
Choices are saved for the next app launch. Run-button preference remains in Settings.

- Treasure Shuffle: existing ordinary chest-loot shuffle. Quest checks are separate.
- Major Item Shuffle: all 16 quest items exchange places among their 16 original
  major checks: seven chests, eight NPC rewards and the caravan. This does not
  put quest items in every ordinary chest. NPC gates/trade requirements remain
  intact; ship, bridge, canal event, canoe and orb events keep normal behavior.
- Shop Stock + Magic: weapon/armor stores draw from the full equipment pools;
  ordinary item shops can also stock chest-only weapons. Item-shop weapons go
  into the first free party weapon slot; use the game's equipment menu to move
  them. Full inventory means no purchase and no gold charge. HEAL/PURE supplies
  and affordable starter equipment remain available; camping/stone cures remain
  sold. Caravan stays a major-item check.
- Magic shops shuffle the eight spells WITHIN EACH original spell level across
  that level's white/black shops. All 64 spells remain available exactly once;
  their effects, charge levels and town/shop levels are unchanged.
- Shop Price Shuffle: equipment/consumable prices reshuffle in their categories,
  with affordable starter gear/healing and a bounded caravan Bottle price.
  Magic prices, clinics and inns keep their existing values.
- Open Spell Learning: all classes can learn spells. This does not grant MP or
  change class growth: a class with zero charges still cannot cast that level.

## Progression rules and limits
The original US MMC1 ROM is checksum-checked before generation; never overwritten.
The major-item solver starts with the reachable Garland/bridge/Bikke/ship route,
then simulates key doors, NPC trades, canal, Earth Orb/canoe, Floater/airship,
Crown/Ordeals, Bottle/Fairy/Oxyale, Slab/Unne/Chime, Cube/Sky and the final
four-orb/Lute/Key route. All 16 checks must become accessible. Failed candidates
are rejected; a bounded exhausted search reports an error and saves no ROM.

NPC rewards have persistent one-time claim flags independent of item ownership.
Consuming a reward does not make the NPC give it again. Matoya can still trade
if the prince was awakened using a Herb found elsewhere. Reward dialogue names
the actual item using the game's treasure message; existing story hints may
still refer to vanilla rewards.

The checks assume normal leveling, combat, gold farming and use of the game's
save/retry mechanics. They guard against modeled progression deadlocks, not every
possible player-induced trap, challenge-party limitation or gameplay bug.
Full console playthroughs remain necessary. This is a native preset, not the
upstream 4.9.7 engine; seeds and flags are not compatible with that engine.

## Existing saves and timing
Old treasure/run seeds still load. Custom filenames include seed AND options;
SRAM/best-times remain tied to the resulting ROM hash. The same resulting ROM
uses its existing save, even if listed in a different slot. Choosing no changes
produces the original game. This update does not implement exact savestate resume
or all-seed personal-best history: existing progress is still practice, and best
milestones are per-ROM. L3+R3 saves SRAM and returns to menu, not an emulator state.

## Validation / build
Host: 1,000 deterministic all-option seeds, all 32 checkbox combinations,
11 deliberate dependency deadlocks, permitted ROM-byte boundaries, unchanged
spell levels/all-spell availability, shop capacities, catalog/save separation.
6502 CPU tests exercise NPC gates/one-time rewards/consumed items, all 40 weapon
purchases/full bags, consumable caps, charging only on success and all 12 classes
learning all 64 spells in the proper level slots. Separate FCEUmm tests boot the
option combinations with the run patch, audio and automatic timer.
These tests are not 1,000 full playthroughs or PS5 certification.

Source: src/ff1logic/, src/randomizer.*, src/main.cpp.
Readable ROM patches: tools/asm/ff1-options.asm; regenerate patches.hpp with
python3 tools/assemble-ff1-options.py (requires py65). Native generation uses no
Python or network. Run tools/test-ff1-options.sh with the original ROM after
building the current host core via tools/test-randomizer.sh. Rebuild the PS5 core
via tools/build-fceumm.sh before tools/build-ff1-options.sh; old local core
archives may have the obsolete tracker RAM whitelist. Preserve the tested runtime.
