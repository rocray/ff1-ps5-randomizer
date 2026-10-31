FF1 RANDOMIZER — COMPLETE TEST17

INSTALL
Close FF1, upload the entire PPSA01987 folder, and overwrite the previous app.
Keep /download0: it contains your seeds, SRAM, checkpoints and best times.
The footer should say TEST 17. Zelda is unchanged.

FULL MAGIC SHUFFLE
Enable SHOPS + ALL MAGIC before generating a NEW run. All 64 white and black
spells enter one pool and are distributed across all shop levels and colors.
The destination slot determines the new learning level and MP charge level.
Every spell appears exactly once. Names, effects and menu-use spells follow it.
Enemy and equipment spells retain their original effects.

OPEN SPELL LEARNING remains separate: enable it to remove class restrictions.
It does not grant characters extra MP or change their natural MP growth.
With it off, original spell-specific class permissions follow each spell.
SHUFFLE PRICES remains a child of the shop option. With it off, each destination
slot retains its original purchase price. Magic prices use the destination level;
the price-shuffle option continues to affect ordinary stock, not magic prices.

Existing seeds and checkpoints stay unchanged. Generate a fresh seed to see
this shuffle, even if you use a seed number you have played before. New runs use
CUSTOM V3 filenames and do not overwrite prior CUSTOM V1/V2 ROMs.

WHAT TO TEST IN TEST17
Generate a new run with SHOPS + ALL MAGIC and OPEN SPELL LEARNING enabled.
Check the opening magic shops for spells from different original levels.
Buy and cast a spell: its new shop level should determine its MP cost.
Try a healing spell outside battle, then L3 + R3, close/reopen and CONTINUE RUN.

EXACT SAVE / RESUME
- Play normally. The app checkpoints about every 30 seconds of emulation.
- Click BOTH thumbsticks (L3 + R3) to save the current run and return to the menu.
  Wait for the menu before closing the app. A failed save displays an error.
- After restarting the app, choose CONTINUE RUN. It restores your latest saved
  run, including game position, timer, splits, item acquisition history, orbs,
  practice/completion status and the same previous-best comparison times.
- Paused runs stay paused. Press R1 to resume.
- Opening a previously played seed, or PLAY ORIGINAL, restores that ROM's own
  checkpoint when available. Each ROM keeps a separate checkpoint.
- A checkpoint includes its ROM, so CONTINUE RUN does not depend on the seed
  still occupying its old library slot. The original app ROM is not changed.
- Completion also triggers a checkpoint. In-game SRAM saves are still written.

WHAT TO TEST
Start a new run, obtain something, note the timer, then press L3 + R3.
Close the PS5 app completely. Reopen it and choose CONTINUE RUN.
The position, timer, item icons and splits should match where you left off.
Repeat once while paused; R1 should resume without counting the offline time.

LIMITS
Older builds saved only SRAM. They cannot supply the exact position or elapsed
clock from an earlier session. Continue playing that SRAM save in Test17 to
create its first checkpoint; existing progress remains a practice run.
An abrupt close/power loss can lose play since the last checkpoint (about 30s).
A damaged, mismatched or incompatible checkpoint is rejected, not partly loaded;
your SRAM remains separate. Back up the affected .srm.resume file before removing
it if you want to fall back to the older in-game SRAM save.

Host tests passed: 1000 deterministic generation seeds, all 32 checkbox
combinations booting with audio and auto timer, actual 6502 spell permissions,
all 64 spells / four characters using destination-level MP and correct effects,
out-of-battle routing and a real shuffled CURE cast. Exact checkpoint/resume
regression passed on a newly shuffled ROM. Native mapping and signed SELF checks
passed. PS5 hardware testing is pending.

Overall completed-run history across all seeds remains pending; existing
per-ROM milestone records and exact resume are preserved. Zelda is unchanged.
