FF1 RANDOMIZER — COMPLETE TEST15

Close FF1, then upload the entire PPSA01987 folder and overwrite the previous
installation. This ZIP includes the ROM, music, graphics and known-good runtime.
Keep your /download0 saves. Zelda (PPSA01988) is unchanged.
The menu footer must say TEST 15 after installation.

NEW RANDOM RUN
Square switches between the seed editor and checkboxes. Up/Down selects a row;
X toggles it. Square returns to the seed; X generates and plays.

SHOP STOCK + MAGIC
  [ ] SHUFFLE PRICES

Shuffle Prices is a child of Shop Stock + Magic. Turning off the parent clears
and disables the child. It defaults off unless a previous enabled preference
was saved with shop stock enabled.

Prices OFF: a replacement inherits the original price of that exact shop slot.
The same replacement can cost different amounts in different shops. Both the
list and purchase confirmation show the amount charged. Ordinary resale values
are unchanged; cheap high-value equipment can therefore be sold for a profit.

Prices ON: the price values are randomized using a separate deterministic stream,
with affordable starter gear and healing safeguards retained. Magic prices stay
at their original levels; inn/clinic prices are unchanged. The original-slot
price tokens use that shuffled price table. Selling uses half the item's current
price-table value, as in the original game.

Applies to NEW generated ROMs. Old Treasure v1 and Custom v1 seeds still load
unchanged. New Custom v2 files use separate names, so creating the same numeric
seed will not overwrite its older ROM. New ROM content has separate SRAM/best
records; use the original library entry to continue an old run.

TESTING
Host checks passed for generated-ROM progression, pricing with the option on
and off, actual bank-switched 6502 inventory/confirmation/payment/selling paths,
legacy catalog compatibility, and FCEUmm new-game/audio/timer regression.
Native SELF and segment mapping are verified. PS5 testing is still required.

These are native presets, not upstream FF1 Randomizer 4.9.7 seed/flag compatibility.
Saves remain SRAM; exact save-state/timer resume and overall completed-run history
are separate pending features.
