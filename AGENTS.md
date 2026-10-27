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
