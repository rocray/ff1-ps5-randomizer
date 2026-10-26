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
