# FF1 PS5 shared work log

## Coordination
- Work from actual checked-in source, not reconstructed handoff snippets.
- Use separate branches and record the exact commit tested on PS5.
- Preserve PPSA01987 and the working renderer/startup unless a change is justified.
- Never commit ROMs, music, tokens, or downloaded SDKs. User explicitly authorized the console-tested recovery ZIP in recovery/; keep other generated binaries out of source control.
- Build client owns local deployment. ChatGPT's environment cannot reach the PS5.
- Avoid rapid repeated launches: fully close the app and wait before another test.
- Do not mark a host-tested change as PS5-tested or merged.

## Baseline
- 528c19d: user reports working PS5 menu on firmware 13.60.
- Native renderer handles tiled RGBA framebuffer output and controller polling.
- ROM supplied locally at /app0/assets/FF1.NES.

## 2026-10-25 — ChatGPT integration branch (pending console validation)
- Replaced diagnostic grid/nonstandard Libretro glue with real FCEUmm API callbacks.
- Pinned core ac0d00a7fc01166f84c7bac01d1084ebfd2b865e; setup script fetches it.
- Added exact-size ROM reads, 256x240 framebuffer allocation, pixel-format/pitch conversion.
- Corrected game scaling to a separate 1280x960 image before Canvas::image.
- Added NES controller mapping, pause, return-to-menu, and active-session continue.
- Added manual active-emulation clock and experimental item acquisition history/splits.
- Added SRAM load/save at /download0/ff1.srm; saves every 1800 frames and on menu exit.
- Host tests PASS: FF1 boot/video, input mapping, pause/timer, SRAM reload,
  three pixel formats, padded pitch, duplicate frames, consumed-item history.
- AddressSanitizer/UBSan enabled for bridge tests; leak scanning disabled because
  this sandbox cannot run LeakSanitizer. The emulator archive itself is not sanitized.
- PS5 cross-build: PASS. Gameplay hardware test: pending. Menu recovery hardware test: PASS (user confirmed).
- Audio sink intentionally silent; seed generator, verified orbs/Chaos, disk-based
  tracker/suspend persistence, and a selectable ROM/seed library remain pending.

## Next owner: local build/deployment AI
1. Pull the integration branch and record the exact commit.
2. Run make app on the existing configured Linux build machine.
3. Preserve local assets and do not replace the known-good runtime shim gratuitously.
4. Close the running title; upload the completed build using the established route.
5. Launch ONCE. Check menu -> PLAY FF1 -> NES title/story -> party/menu controls.
6. L1 arms clock; R1 pauses; Triangle saves SRAM and returns to menu;
   CONTINUE RUN resumes the in-memory game. Circle is NES B, never menu exit.
7. Verify inventory acquisition against an actual FF1 save before calling the tracker verified.
8. Record build output, on-screen result and logs here; don't merge based only on host tests.

## Confirmed launch recovery — 2026-10-25 (America/New_York)
- Symptom: CE-107750-0, including after restoring the original 73,172-byte eboot.
- Kernel evidence: `readHeader invalid state`, `sceSblAuthMgrAuthHeader` error 46,
  then `sceSblACMgrGetFsSandboxType(.../PPSA01987/eboot.bin)` -> `0x80020008`.
- Uploaded original and newer packages contained raw ELF files, not SELF containers.
- Successful action: wrap BOTH original eboot and runtime libc.prx with the repo's
  native SELF tool. Keep original param.json. User confirmed: "ok it loaded".
- Exact tested files are in `recovery/FF1-Working-Menu-Recovery.zip` (three-file
  overwrite patch; retain local assets). No ROM, music, or SDK is included.
- eboot.bin SHA256: 6c9dc8189832b1e2838d394bc4d0990a38d0951684582ea8b8902582395c2fcb
- sce_module/libc.prx SHA256: 8a29784545983fffd7428446912711ec4e269672673e2d281612d3dbf4c39e88
- This fixes the observed FF1 header failure, NOT every possible CE-107750-0.
  No conclusion was established for Vampire Survivors.
- Console: FW13.60, kstuff-lite1.11, only ShadowMountPlus1.7beta7; etaHEN off.
  FF1 fakelib Disabled was requested; exact final UI state was not reconfirmed.
- Do not deploy build/eboot.elf renamed to eboot.bin. Deploy the signed output
  from the SELF stage. Inspect both modules before uploading.
- Signing commands (tool built from tooling/native):
  `ps5-native-tool self --sign --in ORIGINAL_EBOOT_ELF --out eboot.bin --magic 0x1D3D154F`
  `ps5-native-tool self --sign --in ORIGINAL_LIBC_ELF --out libc.prx`
- Repaired a separate converter issue for the larger NES build: RELRO file offset
  must account for .data.rel.ro before .got. Merely aligning the old offset is
  insufficient because it maps the wrong pointer-table bytes.
- Regression: `python tests/test_module_mapping.py build/llvm-pie.elf build/eboot.elf`
  passes alignment and byte comparisons for text, rodata, pointer tables, GOT, data.
- Gameplay Test01 uses FCEUmm, manual clock, item history, and rom/FF1.NES
  (assets/FF1.NES fallback). It retains the EXACT tested runtime above.
- Host tests pass: ROM boot/video, controls, pause/timer, SRAM, pixel formats/pitch,
  item history, and 5 tooling tests. Full lint/GoogleTest suite not run this session.
- Pending: user hardware test of PLAY FF1. Do not label gameplay working yet.

## Gameplay Test02 — ROM memory handoff
- Test01 opens on PS5, but PLAY FF1 reports CORE REJECTED ROM (user result).
- Pinned FCEUmm only consumes in-memory content via GET_GAME_INFO_EXT; its ordinary
  retro_game_info fallback copies the filename and reopens through libretro VFS.
- Added persistent extended content info exposing the already-read ROM buffer.
  This avoids that second filesystem path. Buffer lives until unload completes.
- Host FF1 boot/video test passes with core-facing /app0/rom/FF1.NES absent on host,
  confirming memory loading. Added callback pointer/size/lifetime contract checks.
- Native relink, segment mapping checks, and SELF integrity pass; console test pending.

## Gameplay Test03 — low-level ROM I/O and useful errors
- User confirmed ROM exists at /data/homebrew/PPSA01987/rom/FF1.NES, 256KiB.
- Test02 still failed; clean install displayed ROM NOT FOUND. Prior fallback logic
  could overwrite a genuine core-load failure with a missing-file error.
- Only try another path when open fails; include the absolute rom/ fallback.
- Use open/lseek/read/close, matching working renderer asset I/O, instead of stdio.
- Provide core log callback and display first warning/error on load failure.
- Visible TEST03 menu marker distinguishes uploaded builds. Hardware test pending.

## Menu/music Test04 — 2026-10-26
- User requests cleaner menu and supplied Prelude music; retains Test03 ROM diagnostics.
- Menu now has Play, Continue, Title Music toggle and Controls; removed inactive
  seed/settings entries and inaccurate fixed party description. Seed generation remains pending.
- Added dedicated native AudioOut thread, 48kHz signed-16 stereo PCM streaming,
  exact end-of-file looping, menu-only playback, and visible audio failure status.
- API reference: ps5-payload-dev/SDL release-2.30.x-ps5 audio backend.
- Convert local music: ffmpeg -i prelude.mp3 -ar 48000 -ac 2 -f s16le assets/prelude.pcm
- Music/ROM remain private assets; preserve tested SELF runtime and existing snd0.at9.
- Native build and mapping validation required; menu music hardware test pending.
- Test04 validation: native compile/link, mapped-section regression and SELF integrity PASS.
  Host audio test checks repeated PCM wrapping across output grains and output-error handling.
  Hardware playback still pending. Existing NES host tests passed in Test03.

## Test05 — error text visibility
- User confirms Test04 menu music plays; PLAY fails with only an exclamation mark.
- Renderer had uppercase glyphs only and silently replaced lowercase with blanks.
  Fold ASCII lowercase for all text so emulator diagnostics and ROM paths render.
- Preserve preformatted FCEUD_PrintError strings directly through its %s callback.
- Distinguish CORE LOAD FAILED from ROM LOAD FAILED; marker TEST05.
- This is a diagnostic correction, not a confirmed emulator loading fix.

## Test06 — calloc compatibility
- Test05 console result: CORE LOAD FAILED / ERROR ALLOCATING MEMORY.
- Native fceu-memory.o disassembly proves malloc+memset optimized into calloc.
- Runtime shim heap table populates malloc/free/posix_memalign only; calloc slot absent.
- Added app-local hidden calloc with overflow checking and volatile zero-fill to
  prevent compiler transformation into recursive calloc. Existing malloc/free retained.
- Host regression: ROM-sized zero fill, multiplication overflow, zero size PASS.
- Preserve known-good runtime; PS5 gameplay validation pending.

## Test07 — quantify remaining allocation failure
- User reports same allocation error after Test06; calloc hypothesis is not confirmed fixed.
- Verified native FCEU_malloc calls the app-local calloc directly, which calls malloc.
- Track failed calloc byte count and show it without printf formatting when ROM load fails.
- Reset diagnostics before core initialization so early failures are included.
- New private ff1-prelude.mp3 replaces menu PCM in this package; snd0.at9 unchanged.
- Test07 is diagnostic. Gameplay remains unverified on PS5.

## Test08 — aligned allocation fallback
- Test07 console result: CALLOC FAILED: 262144 BYTES. Replacement music confirmed working.
- This confirms local calloc is reached and malloc returns null for a normal 256KiB
  request; it does not establish the underlying heap limit or system memory exhaustion.
- Try free-compatible posix_memalign (64-byte alignment) if malloc fails, then zero
  the buffer. Both APIs are populated in the runtime heap interface.
- Host failure-injection test verifies the exact 262144-byte fallback, alignment,
  zero fill, free, both-allocator failure and multiplication overflow.
- PS5 test pending. If fallback fails, message says BOTH ALLOCATORS FAILED.

## Test09 — reserved allocation pool and menu help
- Test08 console result: both runtime allocators fail at 262144 bytes; cause unknown.
- calloc now uses a 16MiB BSS pool with 16KiB pages, locking and reusable spans.
- --wrap=free and --wrap=realloc release/resize pool allocations; foreign pointers
  delegate to runtime. Keep both flags when linking.
- Host tests pass: 256KiB allocations, exhaustion, full reuse, resize preservation;
  actual FF1 boot/video/input/pause/timer/SRAM/item-history using reserved calloc.
- Added pointing glove and HELP / OVERLAY with sample tracker, controls and limits.
  Original generated mockup unavailable; follows visible reference and existing theme.
- Menu/help layout visually checked using host render. New Prelude retained.
- PS5 gameplay and new allocator validation still pending. Full Test09 ZIP includes all files.

## Test10 — confirmed boot, game audio, automatic tracker and approved UI
- User confirmed Test09 ROM loads on PS5. Preserve reserved calloc pool and linker
  free/realloc wrappers; known-good eboot SHA256:
  900c82cc610eaa19b0a673be7293abbd7133c9d734c2238a56ee5664b74a2cd3.
- Randomizer explicitly deferred. UI says Original FF1, not generated seed/preset.
- Audio callbacks now feed bounded stereo queue to existing AudioOut worker.
  Core option fceumm_sndrate_hint=48KHz. Menu music and game audio share output port.
- Original FF1 profile: automatic start after two frames with valid party HP,
  overworld vehicle and overworld/ship/airship music. Timer uses emulated frame time;
  pause and app menu stop it. Timing starts at overworld, not title/new-game selection.
- Ship/canoe/airship flags 6000/6012/6004; earth/fire/water/air orbs 6035/6032/6033/6034.
  Chaos stop: formation 006A==7B and battle result 6B86==FF.
  Read-only bridge expanded to explicit whitelist, including 0042/004B/610A/610B.
- Primary reference: Entroper/FF1Disassembly variables.inc, bank_0F.asm (GameStart,
  PrepOverworld and orb handlers), bank_0B.asm (BattleOver_ProcessResult).
- Per-milestone fastest elapsed times stored in /download0/ff1-best-<ROM FNV64>.bin,
  checksum/version guarded, with frozen prior-best delta baseline for each run.
  These are individual best milestone times, not splits from a single full PB run.
- Existing quest progress at auto-start marks practice; no best records overwritten.
  Untested modified/randomized ROM profiles are not validated. No competitive timing claim.
- Approved mockups reused as private raw artwork: bg.raw 1920x1080 RGBA and
  overlay.raw 1672x941 RGBA atlas. Dynamic text/timer/splits/items drawn in native code.
  Game screen follows mockup proportions; Square toggles full-screen 4:3 view.
- Inputs: X=A, O=B, Options=NES Start, touchpad=NES Select, R1 pause, Triangle menu.
- Host PASS: real ROM boot with nonzero stereo audio; auto-start frame1301 during
  scripted new-game entry; records persistence/ROM isolation/corruption; stereo queue;
  existing framebuffer/input/tracker tests. Native mapping and SELF checks PASS.
- Native AudioOut playback, autosplits, save permissions and visual output still require
  PS5 test. No emulator save-state resume; CONTINUE RUN is current process only.


## Test11 — overlay performance and L3 + R3 menu (2026-10-27)
- Hardware report for Test10: ROM and audio work, but overlay visible runs about
  half speed (20 seconds real time for 10 game seconds). Hiding overlay restores
  normal speed. This isolates the expensive overlay path; audio starvation follows.
- Preserve working SELF/runtime, reserved allocator and Test10 assets.
- Canvas now renders to cached BSS with the SAME proven tiled address layout.
  Present copies only the 8,847,360-byte tiled surface sequentially to the selected
  WC back buffer and sfence completes writes. Removed per-frame clflush across
  both 16MiB buffers. Registration, two-buffer flip order and native format retained.
- Panel border bands paint once instead of six overlapping full-panel fills;
  nearest-neighbor region scaling hoists divisions outside the per-pixel loop.
- Monotonic scheduler runs NES at core-reported FPS independently of display FPS,
  bounded to four catch-up frames. Pause/menu reset debt. Timer still advances only
  through actual emulation; no wall-clock-only cosmetic timer correction.
- Clicking BOTH sticks (L3 + R3), either press order, saves SRAM and opens menu.
  Hold does not retrigger; Triangle no longer exits gameplay. Help/footer updated.
- Host PASS: identical panel pixels; scale/clipping/dim pixels; cached overlay
  benchmark 9.14ms/frame on this host (not PS5); scheduler 1202 frames over 20s
  at 30/60/120 render FPS; pause/stall cap/chord edges. Native link, segment mapping
  and signed SELF integrity PASS. Test11 hardware performance pending.
- eboot SHA256: 2c5595a7909c3eb800cf321e2041b112db8169006c365fe410bdd2e97256bdb4


## Test12 — first fully offline seed preset (2026-10-27)
- User confirms Test11 works on PS5. Preserve its renderer, pacing, audio, allocator,
  SELF startup and runtime shim. User requires generation entirely on PS5.
- Inspected upstream FiendsOfTheElements/FF1Randomizer: C#/.NET with Blazor UI;
  cannot directly link into this native app. Full 4.9.7 engine/progression port is
  NOT implemented. Do not claim compatibility with its seeds, flags or presets.
- First native preset: Treasure Shuffle v1. Fixed 32-bit PRNG and Fisher-Yates;
  shuffles existing ordinary chest loot, keeps all quest/orb values and unused
  slots fixed. No map/code/mapper/enemy/NPC changes. Original US PRG CRC32
  cebd2a31 and exact 262160-byte supported cartridge required before any patch.
- Data references: upstream Items.cs / ItemLocations.cs at
  e49d72e20f2dcb2c034affddce29f72d54b60d5e. Treasure PRG offset 3100,
  file offset 3110; 256 slots. No upstream algorithm copied or equivalence claimed.
- Menu: New Random Run, Seed Library, Play Original, Continue Run, Title Music,
  Help. Eight hex digits edited with D-pad; Triangle picks seed; L1/R1 chooses
  library slot; X generates/validates/saves/loads. L3+R3 retains save/menu action.
- Files: /download0/ff1-t1-<8hex>.nes and checksummed ff1-seeds-v1.bin (8 slots).
  Writes use temp+fsync+rename, then ROM readback/byte verification. Replacing a
  library slot does not delete old ROM/save files. Same seed resumes its existing SRAM.
- Generated ROM SRAM isolated by full-ROM hash. Original /download0/ff1.srm kept;
  best times already isolated by ROM hash. Original source ROM is never modified.
- Loader stages/validates exact iNES length, supported mapper/size/trainer format
  BEFORE unloading an existing session. Invalid/missing requests preserve it.
  Core/allocation failures return errors; no absolute claim of crash immunity.
- Host PASS: 100 reproducible different-from-base seeds; no changes outside loot;
  quest bytes and total loot multiset retained; saved-library reload; damaged/truncated
  file rejection; 24 successive core loads with sound and separate SRAM paths;
  bad path preserves active session. Existing ROM/bridge tests PASS.
- Menu/seed editor rendered and visually inspected. Native segment mapping and
  signed SELF integrity PASS. PS5 generation/write permissions/reload test pending.
- Full upstream progression randomizer remains future work; current preset is
  explicitly named and scoped in UI. No PC or network required by this preset.
- Additional host check: generated seed 00000000 enters a new game and starts the
  automatic tracker without practice flag. Native Test12 eboot SHA256:
  2d3af89bfa84a629ea1794f9b404b83140120420489417d33107e4afd5eb2e98.

## Test13 — FF1 display, timestamp and run QOL
- User confirmed Test12 works through obtaining the ship on PS5.
- Settings persist to /download0/ff1-settings-v1.bin with checksum and safe defaults.
  Default Classic 4:3 (1008x756, full overlay); Large 4:3 (1376x1032, side-only
  timer/splits/events); Integer Pixels (768x720 overlay, 1024x960 hidden); Wide
  Stretch retains Test12. Square hides overlay; hidden default is 1440x1080 4:3.
- Gameplay-only scanlines Off/Light/Strong; optional mild curvature sampled with
  cached 256x240 map. UI unaffected. This is a lightweight effect, not a CRT shader.
- SRAM successful writes get .time sidecars, UTC minute timestamp from console clock.
  UI displays save-file update time, not a savestate or guaranteed latest position.
  Existing saves display NO TIMESTAMP until next successful write. Seed library
  reads timestamps only on selection, not every rendered frame.
- New ROM run-button toggle bakes hold-NES-B/Circle movement into generated ROMs.
  Equivalent routine to upstream EnableDash slowMapMove/BVS variant, adapted from
  MMC3 bank1F to original MMC1 bank0F:$D077; checks original 19 bytes before patch.
  Only 17-byte routine replaced; no emulator fast-forward or battle speed changes.
- Run ROM filenames have -run suffix and distinct ROM-hash SRAM/best times.
  v2 library entries add flags; v1 catalog migration preserves all Test12 entries.
  Existing seeds/original ROM remain unchanged. Toggle affects NEW generation only.
- Host PASS: layouts/bounds, settings roundtrip/defaults, leap-day UTC conversion,
  timestamp write/read, curve bounds, run-patch bytes and all vehicle/B speeds,
  legacy library migration, generated run-ROM disk verification, 100 seed tests,
  24 core switches, bridge/audio tests. Actual core boot into overworld confirms
  movement speed 1 normally and 2 with Circle held (test_run_button.cpp).
- Settings preview visually checked; native compile/link, mapping and SELF PASS.
  Hardware filter performance/timestamps/run option pending user validation.

## Zelda Test01 — separate native app, host tested; PS5 test pending
- FF1 Test13 source is 8e830174a0276b38f4fc04faf4fb4d69cc7eb78f. Saved complete
  Test13 ZIP eboot SHA256 373db317683a385738cd811113f9806b89ba716a39a0bdfdb774a6f4a706db61.
- New ports/zelda/ target PPSA01988, distinct from FF1 PPSA01987. Shared runtime
  implementation copied at Test13; keep allocator, signed SELF libc and cached
  tiled framebuffer fixes intact. Build with bash ports/zelda/build.sh after root
  toolchain/startup objects/core archive are available. Native compile/link,
  module mapping and signed SELF integrity PASS.
- Zelda Test01 eboot SHA256 db4293c5561c3490c877efce74fd1c8906c4e559c4ad0760d617b70736646030.
  Runtime libc SHA256 8a29784545983fffd7428446912711ec4e269672673e2d281612d3dbf4c39e88.
- Original Zelda US Rev1 ROM is private, at /app0/rom/ZELDA.NES; PRG CRC eaf7ed72.
  Never commit ROM. ZIP contains user's supplied ROM and known working runtime.
- Native Dungeon Gear Shuffle v1 only: six low-five-bit optional gear fields
  (file offsets 18954,1895f,18985,18c10,18c5a,18c7f). Progression locations remain
  fixed. Not the full Z1 tournament randomizer or compatible seed/flag format.
- Custom procedural title/menu/hand, original NES title audio at initial app menu,
  gold/navy 4:3 overlay: 16 inventory fields, found dungeon, 9 milestones, visited
  dungeon/Triforce strip and counters. Menu soundtrack does not resume after
  returning from gameplay; it comes from the initial background NES title.
- Timer starts after 2 stable playable mode5 frames and stops at mode13hex rescue.
  RAM profile documented in run_tracking.hpp. Existing inventory/Triforce/quest2
  marks practice, prevents best-time changes. Best times are independent elapsed
  milestone minima per ROM, not a full PB-run comparison. Location history is
  in-memory; resumed items show unknown rather than inventing locations.
- /download0/z1.srm original, z1-<ROM hash>.srm generated; .time UTC timestamps,
  z1-best-<hash>.bin records, z1-seeds-v1.bin library, z1-settings-v1.bin settings.
  Saves are SRAM, not savestates. L3+R3 writes SRAM and returns to app menu.
- Host PASS: 100 deterministic seeds, only six permitted fields change, gear
  multiset/room flags preserved, disk catalog/readback, rejected modified base,
  6000 title frames without timer start, actual new-game registration and cave
  sword pickup updating inventory, audio, pause, SRAM timestamps, 12 core reloads.
  Synthetic RAM tests cover nonlinear Triforce split, acquisition dungeon,
  rescue finish and practice best-time protection. No full-playthrough claim.
- Native menu/overlay visually rendered. Launcher BC7/PNG assets validated;
  complete ZIP CRC verified. PS5 launch, audio/performance and dungeon behavior
  remain user validation. Use separate PPSA01988 folder; never overwrite FF1.
- Source references: tetraly/zelda-randomizer aa507c7 item table layout;
  aldonunez/zelda1-disassembly 50a1c86 RAM symbols. Independently implemented preset.

## Zelda Test02 — approved artwork integrated (PS5 validation pending)
- User requested applying all three generated concepts to the actual app.
  Reusable source PNG artwork is under ports/zelda/artwork; raw runtime assets
  and launcher DDS/PNG are generated by tools/package.py. These PNGs are app
  source assets, not compiled binaries or ROMs.
- Title/menu, launcher icon/backgrounds, item sprite atlas and decorative overlay
  are live. Mockup text/gameplay/statistics are covered and real values rendered.
  No fake sample timer, loot, heart count or split is shown. Menu has only the
  implemented actions; no nonfunctional RECORDS/RESUME button from concept art.
- Gameplay keeps default 4:3 within the illustrated frame; integer/wide and
  overlay-hidden layouts remain. Inventory/dungeon badges use illustrated sprites.
- Added Canvas copy_tiles/restore_tiles for a static tiled overlay cache.
  Host render benchmark decreased ~10.55 to ~2.46 ms/frame. Cached CPU staging
  and existing VideoOut copy preserved; PS5 timing still requires user test.
- Host previews checked: title menu, selection on settings, empty/active tracker.
  Native compile/link, segment mapping, signed SELF integrity and launcher asset
  validation PASS. Complete ZIP CRC PASS. NES core/randomizer/save code unchanged.
- eboot SHA256 934925a1d9a35362b5d6fae1e648d7afee339173669b4516c1606961d0c8f4eb.
- Full replacement Zelda-Complete-Test02.zip includes original ROM, runtime and
  new assets; upload PPSA01988. Preserve /download0 saves; FF1 PPSA01987 unchanged.

## Pending requested next functional work — not included in Zelda artwork Test02
- Exact save-state resume with matched timer/splits/history, including app restart.
- FF1 shop inventory/prices shuffle; spells shuffled while retaining magic levels,
  remove class learning requirements (clarify/implement charge growth semantics).
- Overall best completed run across all seeds, with seed/rules context, not only
  per-ROM best individual milestone. User intends repeated long-term play.
- Reference FF1Randomizer e49d72e: ShopData shop pointer 0x38302, pointer base
  0x30000, PriceOffset 0x37c00, permission table 0x3ad18; these are PRG offsets.
  Research only so far; none of these functional changes is implemented yet.

## Zelda Test03 — two randomizer modes (2026-10-28)
- Square on NEW RANDOM QUEST switches original Gear (flags0) / All Items
  (flags2). Existing catalog v1 migration and v2 gear entries preserved.
  z1-g1-SEED.nes and z1-a1-SEED.nes separate mode paths; saves/bests hash ROM.
- All Items means 32 permanent equipment/heart-container locations: dungeon
  floor/stairs, sword/letter caves, shop gear, Armos and coast. Refill supplies,
  keys/maps/compasses, Triforce, prices, take-any hearts/layouts remain fixed.
- Native logic in ports/zelda/src/logic checks first-quest overworld gates,
  room traversal/stairs, combat, locked doors, renewable shop resources,
  heart-gated caves, eight Triforce pieces, Ganon/Zelda and all pool locations.
  Requires original US Rev1 CRC eaf7ed72. Max4096 candidate attempts; reject
  without writing on exhaustion. Original base ROM never mutated.
- Magical Sword uses ID3, vanilla dungeon no-item sentinel also3. Full mode
  migrates first-quest grid sentinel to14 and CMP immediate at file1785f to14,
  following upstream Mags fix. Preserve all other flags/bytes. Rev1 pickup
  routine already avoids downgrading stronger equipment; no speculative patch.
- Logic assumes normal renewable rupee farming and retry/save use; not proof
  against all player-induced traps. Never register name ZELDA: second quest
  is outside profile. Do not advertise full upstream/tournament compatibility.
- PASS: 1000 deterministic seeds, multiset/masks, all32 reachable, 8 Triforce,
  Ganon/Zelda; five intentional progression deadlocks rejected; catalog/mode
  and save isolation. ASan+UBSan passed (leak detection unsupported by host).
  24 actual all-item ROMs booted and cave pickups acquired in FCEUmm; original
  100-seed/core/audio/timer/SRAM/reload regression passed. Not full playthroughs.
- Native compile/link/module mapping/signed SELF passed. UI mode preview
  checked. Complete Test03 includes ROM/runtime/artwork. PS5 testing pending.
- Retain PPSA01988, existing runtime shim, cached overlay, working heap/audio
  fixes. Preserve /download0 saves. Unrelated FF1 requests remain pending.
- Packaging Test03 reuses verified Test02 artwork bytes via --reuse-artwork.
  Restored source icon PNG failed Pillow decoding in this workspace; do not
  regenerate art from it until repaired. Runtime assets from Test02 are intact.

## FF1 Test14 — native checkbox rules (2026-10-28)
- Adds persistent independent Treasure, Major Items, Shop Stock+Magic, Shop Prices,
  Open Spell Learning checkboxes. Square switches seed/options focus; Up/Down
  chooses row, X toggles, Square returns, X generates. Defaults preserve treasure
  preset. Run remains separate in Settings. New ff1-randomizer-v1.bin checked
  options file; generated library entries retain actual rules even after changes.
- Major pool: all16 quest items, seven existing major chests/eight NPCs/caravan.
  No ordinary-chest mixing or vehicle/orb shuffle. Native fixed-point checker
  covers trade/gate/canal/canoe/airship/orb/final-Temple dependencies and requires
  every major check reachable. Deterministic bounded rejection, no save on failure.
- NPC patches in tools/asm/ff1-options.asm; checked-in generated patches.hpp means
  no Python needed on PS5. Highbit of NPC game_flags is persistent claim flag;
  preserves visibility/event/chest bits. Matoya no longer becomes unavailable
  after awakening the prince with a shuffled Herb. NPC actual reward uses item
  name in generic treasure message; vanilla story hints may be inaccurate.
- Shop pointers/lengths retained. Weapon/armor shops can carry full equipment
  pools; normal item shops stock chest-tier weapons plus refill safeguards.
  New 6502 item-shop grant routes weapons into first free party weapon slot;
  full inventory rejects without charging. Starter weapons/armor affordable.
- All64 spells shuffled within original eight levels across white/black shops;
  learning-permission toggle zeroes96 permission bytes but DOES NOT grant MP.
  Prices optional, bounded; magic/inn/clinic prices unchanged. Caravan major
  slot is excluded from ordinary stock shuffle. Only original PRG CRC cebd2a31.
- Custom flags: bit0 run, bit1 custom-v1 marker, bit2 treasure, bit3 major,
  bit4 shops, bit5 prices, bit6 open magic. Filenames ff1-c1-SEED-FLAGS.nes;
  old flags0/1 treasure/-run paths still supported; ROM-hash save/best isolation.
  Same resulting ROM still reuses its prior SRAM (including unchanged original).
- PASS: 1000 deterministic all-option seeds, 32 checkbox combinations, 11 forced
  deadlocks, byte-write whitelist, pool/level/shop limits, persisted choices and
  legacy/custom catalog. ASan/UBSan passed (host leak detection unsupported).
- PASS: py65 executes actual ROM patches: NPC prerequisites/one-time claims,
  consumed rewards/Matoya after Herb, all40 weapon IDs/full bags/counter caps,
  charging only on success, all12 classes/all64 spells in correct level slots.
- PASS: all32 option combinations plus run patch boot/new-game/automatic timer/
  audio in FCEUmm. Existing100-seed/24reload/bridge/audio/SRAM/QOL regressions pass.
  Native mapping/SELF checks and menu preview pass. No full-playthrough claim;
  progression model assumes combat/leveling/gold farming/retry mechanics.
- Rebuilt BOTH core archives using current setup-fceumm RAM whitelist. Restored
  workspace had obsolete Test08 archives, which made automatic-start test fail;
  do not reuse those archives. tools/build-ff1-options.sh preserves private assets.
- Complete Test14 repackages verified Test13 ROM/music/art/runtime, updates eboot
  and guide. Preserve PPSA01987, known-good signed libc, /download0 and Zelda.
- Shop requests above are now implemented. Exact save-state/timer resume and
  overall best completed-run history across seeds remain pending, unchanged.
- Test14 eboot SHA256 3ea1ed26bf2223b7f0aca4c7dff2bf65236cfd56399c200c72dd787b609dd291.

## FF1 Test15 — shop-slot prices (2026-10-29)
- User correction: Shuffle Prices must be a child of Shop Stock + Magic.
  Turning the parent off clears/disables the child; settings load and generation
  normalize inconsistent flags. The child is indented; footer identifies TEST 15.
- OFF means ORIGINAL SLOT purchase price, not the replacement item's global
  price. Identical replacements may cost different amounts in different shops.
  No global price-table writes when off. ON keeps the independent price stream;
  cap original Coneria slot price IDs28/29/30/68 for affordable starter coverage.
  Magic level prices and inn/clinic costs are unchanged. Resale remains the
  original global-table/2 behavior; cheap equipment can yield resale profit.
- ROM implementation: parallel original slot item IDs, one-based shop_id, at
  bank0D B680..B7E2. Code B600..B67F is in documented unused B5C9..B7FF space.
  Fixed-bank ECB9 preserves LoadPrice semantics via the bank0D helper; ECD0
  loads parallel price tokens at RAM0340..0344, with mode flag0345. This is above
  shop str_buf0300..0338 and below palette03C0, rebuilt on inventory load.
- Buy-list name/reward tokens remain shuffled IDs; price tokens use original IDs.
  Confirmation reads name and price separately. Sell-list entry clears the mode
  and continues ordinary item-table pricing. Shop padding8467..847F and
  84F8..84FF hold small hooks. No ROM expansion or core memory writes needed.
- Assembler now understands bank-specific offsets, rejects out-of-range branches
  and overlapping patch blocks. Header regenerated from readable assembly.
- New custom flag bit7 denotes Custom v2 / ff1-c2-SEED-FLAGS.nes. Retain legacy
  flags0/1 and c1 paths/catalog entries unchanged. Same numeric seed never
  overwrites an old c1 ROM. Saves/bests continue to follow full ROM content hash.
- Host validation: 1000 deterministic seeds, 32 checkbox combinations, progression
  deadlock regressions, slot table/global price preservation, disabled-parent
  normalization, old/new catalog selection, and ASan/UBSan. py65 executes real
  bank-switched lookup/list/confirmation/affordability/payment/sell-reset paths;
  all256 global prices, ordinary shops/caravan and all64 magic slots covered.
  Existing NPC/reward/learning patches and FCEUmm boot/audio/timer regressions pass.
- Preserve working native allocator, audio, pacing, signed runtime and all private
  assets from complete Test14. Native mapping/SELF checks pass. Hardware test
  remains pending; do not label PS5-verified or merged. Zelda unchanged.
- Complete Test15 eboot SHA256:
  148990c9a7ecd050276c20c2ff3b839954849701f12acd29dbae72b78899d28a.
  ZIP SHA256 e18c11236939f756e4c4151731baa46d574c36d86d7883059217dd6ac75bdb4b.

## FF1 Test16 — exact persistent run checkpoints (2026-10-29)
- User asked to proceed assuming Test15 works; this is an assumption, not a new
  confirmed PS5 hardware result. Preserve Test15 source/features and runtime.
- src/nes/run_snapshot.cpp writes one versioned CRC-protected envelope with ROM,
  FCEUmm serialized state, framebuffer, tracker clock/history, splits, frozen PB
  baseline, current records, orb/practice/finished/paused flags and run label.
  Separate static read/write/rollback arenas avoid PS5 large allocator problems.
- Each SRAM path gets .resume; ff1-last-run.bin in the same save directory points
  to the latest checkpoint. Both writes are temp+fsync+rename. Never checkpoint
  an idle title screen over a playable run. Autosave every1800 emulated frames,
  on completion, L3+R3, and before unloading/switching a played ROM. Failed save
  prevents a requested ROM switch. Corrupt snapshots fail before core mutation;
  failed unserialize rolls the current core back or disables saving if rollback
  fails. SRAM remains separate. Latest checkpoint carries its own ROM bytes.
- CONTINUE RUN survives process restart. Selecting a saved seed/original resumes
  that ROM's checkpoint. Paused saves remain paused. No offline/menu time added;
  framebuffer restored without running an extra frame. Input/pacer resets.
- Frozen delta baseline is restored exactly. More recent best records on disk
  merge by minima, never roll back; record writes now use atomic replacement.
  Overall completed-run/PB history across all seeds remains pending.
- Pin checkpoint core format0xAC0D0001 to ac0d00a. setup-fceumm replaces the
  core's 1MB serialization scratch malloc with calloc (reserved heap path), and
  checks FCEUSS_Save_Mem's actual size rather than unconditional success.
  Rebuild BOTH host and PS5 archives after this setup change.
- Host PASS: cold restart restores exact framebuffer/RAM/timer/history/splits/
  frozen baseline; deterministic90-frame replay, consumed items, paused/practice/
  completion flags, invalid checksum/truncation/wrong-ROM rejection, failed write.
  Existing100-seed/24reload ROM/audio/SRAM regression and32 checkbox core boot
  tests pass, plus actual6502 price/NPC/learning and1000 progression seeds.
- Native signed SELF/module mapping pass. Complete Test16 retains all Test15
  ROM/music/art/runtime bytes except eboot and guide. PS5 testing pending.
- Legacy SRAM cannot reconstruct an old timer or exact position; first resumed
  legacy progress remains practice. Keep /download0 and PPSA01987; Zelda untouched.
- Test16 eboot SHA256 3fb9579f15b03d89e69be123ad44cdfb08f565dafb7fe839f53b9806675fe535.
  Complete ZIP SHA256 fedfd65e278940e5a216af171ef5db31f0110d535b7daeb5ad0b44bea2a64238.

## FF1 Test17 — full magic pool (2026-10-30)
- User correction: pool all64 white/black spells across all eight levels, rather
  than swapping only within each level. SHOPS + ALL MAGIC now does this; separate
  Open Spell Learning checkbox preserves choice of class restrictions.
- Logical spell IDs stay tied to destination slots. Permute effect/animation data,
  name pointers and battle-message indices. Destination supplies learning/storage
  level and MP charge. Spell class permissions follow original identity unless
  Open Spell Learning clears them; natural MP growth remains unchanged.
- Remap enemy script spells, weapon/armor casts, confused-enemy FIRE and all13
  out-of-battle dispatch comparisons to retain their intended effects. Fix vanilla
  ORA-based out-of-battle slot arithmetic to ADC: slot8 otherwise aliases an
  adjacent level when shuffled utility spells occupy it. Bounded assembler patch.
- Independent magic RNG stream; ordinary stock/price behavior retained. Prices
  OFF retains original destination-slot purchase price; magic always uses its
  destination level price. No new magic-price randomization in this build.
- Custom v3 flag256, known_flags511 and ff1-c3 filenames preserve existing c1/c2
  ROMs/checkpoints. Existing seeds unchanged; generate NEW to use full shuffle.
- PASS: 1000 deterministic seeds, 32 option combinations, byte whitelist,
  progression rejection tests, each64 spell once, remapped data/references.
  Actual6502 tests: all12 classes/64 spell permissions, all4 characters/64 slots
  battle effect pointer and MP debit, menu routing/MP checks and real CURE heal.
  Existing NPC/slot-price tests pass. All32 combinations boot with audio/automatic
  timer in FCEUmm. Cold exact resume/deterministic replay passes on Test17 ROM.
- Native compile/link/module mapping/signed SELF PASS; menu preview checked.
  Hardware testing pending. Preserve known-good libc, heap, audio, pacing,
  Test16 checkpoint format, /download0 and PPSA01987. Zelda unchanged.
- Full Test17 ZIP reuses Test16 assets/runtime; only executable and guides change.
  eboot SHA256 939736b2ea9bde15cf051384ca5c05b6e69b2936eaf2cdcfbb785f81da1bb1a5
  ZIP SHA256 8275867b4fb3da64bc30ed932e4a248e937c6e5a133e01c5f5ab84725d53fad0
- Overall completed-run history across all seeds remains pending.
