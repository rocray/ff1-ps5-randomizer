FF1 RANDOMIZER — COMPLETE TEST16

INSTALL
Close FF1, upload the entire PPSA01987 folder, and overwrite the previous app.
Keep /download0: it contains your seeds, SRAM, checkpoints and best times.
The footer should say TEST 16. Zelda is unchanged.

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
clock from an earlier session. Continue playing that SRAM save in Test16 to
create its first checkpoint; existing progress remains a practice run.
An abrupt close/power loss can lose play since the last checkpoint (about 30s).
A damaged, mismatched or incompatible checkpoint is rejected, not partly loaded;
your SRAM remains separate. Back up the affected .srm.resume file before removing
it if you want to fall back to the older in-game SRAM save.

Host tests passed: cold core restart with matching framebuffer/RAM/timer/splits,
consumed-item history, paused/practice/completed runs, deterministic replay,
wrong-ROM and damaged/truncated file rejection, save failure reporting, and the
existing ROM/randomizer/audio regressions. Native mapping and signed SELF checks
passed. PS5 hardware testing is pending.

This build implements exact resume. Overall completed-run history across all
seeds remains the next feature; existing per-ROM milestone records are preserved.
