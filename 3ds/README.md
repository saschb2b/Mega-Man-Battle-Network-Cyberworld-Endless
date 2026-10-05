# The Nintendo 3DS build

The same game as every other build (issue #9), for a New 3DS, New 3DS XL or
New 2DS XL with custom firmware (Luma3DS): a CIA the HOME Menu installs, and
a `.3dsx` for the Homebrew Launcher. An old 3DS or 2DS is not supported: the
GBA core alone takes longer than a frame there.

## Playing

- **The HOME Menu:** install `cyberworld-endless.cia` with FBI. The
  project's page shows the newest release's CIA as a QR code: in FBI,
  Remote Install, then Scan QR Code, and FBI downloads and installs it.
  Or copy the CIA to the SD card and install it from FBI's SD browser.
  The game then has its icon on the HOME Menu, and its banner plays the
  trailer's opening hits. FBI's Titles list deletes it again.
- **The Homebrew Launcher:** copy `cyberworld-endless.3dsx` to
  `sdmc:/3ds/` and start it there. It is the same game, with the same
  saves.
- The ROM, Mega Man Battle Network 6: Cybeast Gregar (USA), goes in
  `sdmc:/3ds/cyberworld-endless/rom/`, or stays where 3DS players keep GBA
  games (`sdmc:/roms/gba`, `sdmc:/roms`, `sdmc:/gba`). Any file name works:
  the game reads the header and checks the contents.
- The first NEW GAME boots BN6 from power-on once, about 15 seconds of black
  screen; later starts load what it kept (`boot-3.state`).
- The picture fills the top screen's height (1.5x, its pixels mixed at their
  edges). `screen = whole` in `settings.ini` shows it sharp at 1x in the
  middle.
- The bottom screen shows the layer's map, always open: the floor MegaMan
  has seen, the way on to the exit or the guardian, the services and gates
  he has come near, and a mark on the frame's edge for those he senses. It
  is SELECT's map, larger; SELECT still opens it over the picture. The
  town and the title leave the bottom screen dark.
- Saves, `settings.ini`, `keys.ini` and `log.txt` are in
  `sdmc:/3ds/cyberworld-endless/`.
- The 3DS's buttons are the GBA's: A, B, L, R, START, SELECT, and the D-Pad
  or the Circle Pad. HOME pauses as for any game.

It runs at full speed, 60 frames a second, every one shown. A new run's
first layer takes about 20 seconds to make ("Building the net..."), as
does CONTINUE's.

## Building

```sh
python3 build.py 3ds    # build/3ds/cyberworld-endless.cia and .3dsx
```

It builds in `docker/Dockerfile.3ds`: devkitPro's devkitARM with libctru and
citro2d, SDL2 built for the 3DS (`docker/sdl2.sh 3ds`), mGBA's library
(`docker/mgba.sh 3ds`), and makerom and bannertool built from their sources
(`docker/ctrtools.sh`). `3ds/icon.png` (`tools/app_icon.py`) is the icon of
both. The CIA is the same program as the `.3dsx`, packed by makerom with
`3ds/cia.rsf`, its title settings: a New 3DS title at 804 MHz with the L2
cache, 124 MB for the application, the third core allowed, a 1 MB stack.
Its banner is `3ds/banner.png` (`tools/steam_art.py`) with the first 2.9
seconds of the trailer's music (`tools/trailer_music.py --seconds 2.9 --rate
32728`), made into a banner by bannertool.

What differs from the other builds:

- **Memory** (`src/core/start_3ds.c`): the app splits its memory itself before
  `main`, the heap as large as its 96 MB area takes, the linear heap (the
  screens' and the sound's buffers) the rest; a heap of all but the linear
  heap's share passed the area. mGBA's 3DS setup, linked in, copies the ROM
  into a buffer of its own, which `docker/mgba.sh` makes 16 MB, and draws
  32-bit pictures as on every other target.
- **The picture** (`src/core/present_3ds.c`): the software renderer draws a
  240 x 160 canvas into memory the GPU reads; the GPU's copy engine moves it
  into a texture and citro2d draws it. SDL's own present copied and turned
  every pixel on the CPU.
- **The bottom screen** (`src/core/platform.c`): every tenth frame the
  director draws the layer's map (`director_draw_second_screen`) straight
  into the memory the GPU copies from (`gfx_draw_into`: its rectangles and
  text, blended as the software renderer blends them), and `present_3ds.c`
  draws it on the bottom screen once for each of the screen's two buffers,
  then leaves it until the next. Through the software renderer and read
  back, the map had taken 20 ms, a frame lost at each redraw.
  `--second-shot FRAME:PATH` saves the same picture on any build, and
  `frame_log`'s lines give the map's own time.
- **The GBA core** (`src/emu/emu.c`) runs on the New 3DS's third core where
  the system gives it one, the drawing of the frame done beside the next
  frame's emulation; `log.txt` says which. Its picture is drawn on the main
  core beside it, mGBA's threaded video: `docker/mgba.sh` builds mGBA's
  whole core for the 3DS (not the minimal one it builds for a "Generic"
  system) and puts mGBA's own threads on the main core. The emulation
  alone took about 16 ms of a 16.7 ms frame on the 804 MHz ARM11; with
  its picture, on one core, 20. Half of that was BN6 waiting for VBlank
  in a loop, which a hook now halts the CPU through (docs/EMULATION.md,
  The core's time): 11 ms a frame now. With the core on its own thread
  the present waits for the screen's refresh, which paces the frames
  evenly (`C3D_FRAME_SYNCDRAW`); on the main core, as where the third
  core is refused, it does not, as a frame takes longer than a refresh
  there.
- **Closing** from the HOME Menu: no frame is drawn once the system asks
  the game to close (the GPU is the HOME Menu's then; waiting on it hung
  the console), and the core's threads end before the game does.
- **Files**: mGBA's own files go to the SD card's file system, which takes
  paths without the C library's `sdmc:`. A rename onto a taken name is
  refused there, as on Windows (`cw_rename`).

## Testing on a 3DS

- **ftpd** (FTP) copies the build over and fetches `log.txt`:
  `curl -T build/3ds/cyberworld-endless.3dsx ftp://3DS-ADDRESS:5000/3ds/`,
  or the CIA into `/cias/` to install it with FBI.
- **FBI's QR code**, as a player installs it: serve `build/3ds` on the
  network (`python3 -m http.server 8766 --bind 0.0.0.0 --directory
  build/3ds`) and scan a QR code of `http://THIS-PC:8766/cyberworld-endless.cia`
  (the page's `web/assets/qr.js` draws one in node too).
- `frame_log = on` in `settings.ini` writes a line a second: frames shown and
  played, a frame's update (the GBA's share, with and without its
  picture), drawing and present, and the second's longest update and
  drawing.
- **3dslink** (`/opt/devkitpro/tools/bin/3dslink -a 3DS-ADDRESS FILE.3dsx`,
  Y in the Homebrew Launcher) sends the build and shows its output here.
- Luma3DS keeps crash dumps in `sdmc:/luma/dumps/arm11/`: registers at 40
  bytes in, then the code and the stack; `arm-none-eabi-addr2line -f -e
  build/3ds/cyberworld-endless.elf ADDRESS` in the image names the place.
