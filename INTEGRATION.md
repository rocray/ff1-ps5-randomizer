# FCEUmm + tracker prototype

Based on 528c19d. This branch preserves the title ID and existing native renderer.
Run `make app` on the configured PS5 build machine. Its build script fetches the
pinned FCEUmm revision and statically links it; no SDL dependency is introduced.
A first build needs Git/network access. The core is outside src/ so the frontend
build cannot accidentally compile all emulator files with inappropriate flags.

Keep the existing local assets (FF1.NES, bg.raw, shell artwork/audio). No ROM or
music is added to Git by this change. Both ROM paths from the original project
are retained; the packaged /app0/assets/FF1.NES path is preferred.

Controls: Cross NES A, Circle NES B, Options Start, touchpad Select, D-pad moves.
L1 arms/stops the clock; R1 pauses/unpauses; Triangle saves SRAM and returns to
menu. CONTINUE RUN resumes the existing session. PLAY FF1 reloads the ROM and
resets the tracker. Clock advances by emulated frames while armed/unpaused.
It is not a race-certified real-time timer. Disconnect pauses emulation; reconnect
and press R1 to resume. Display still follows the existing renderer's vblank loop.

The game is scaled from a real 256x240 buffer to 1280x960 (4:3). Video conversion
handles RGB1555, RGB565 and XRGB8888, row pitch and duplicate frames. SRAM is
written via a temporary file and rename under /download0. This prototype uses
one fixed ROM/save slot; do not swap seeded ROMs while reusing that SRAM file.

Tracker uses an experimental original-FF1 inventory profile at CPU addresses
0x6000..0x6034. Acquisitions are recorded only while the clock is armed, and
FOUND stays lit after consumption. Item rows show first acquisition timestamps;
no personal-best comparison, orb/Chaos autosplits or persistent tracker state yet.

Known missing: game audio (samples are consumed silently), seed generation,
ROM browser, native SaveData API and suspend-state persistence. These are not
represented as working options in this prototype.

## Validation
`bash tools/test-nes.sh /path/to/FF1.NES` builds host tests. The smoke test expects
the supplied original USA ROM (262160 bytes). Bridge tests run under ASan/UBSan;
leak checking is disabled in this sandbox. Native console validation is separate.
The console test owner should update AGENTS.md with the commit and result.


Update: native cross-build now passes. See AGENTS.md for the console-confirmed SELF packaging fix and recovery ZIP. Gameplay remains pending PS5 validation. Test01 retains the known-good signed runtime. ROM preferred path is /app0/rom/FF1.NES.
