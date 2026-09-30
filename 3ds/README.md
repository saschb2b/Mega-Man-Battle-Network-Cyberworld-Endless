# The Nintendo 3DS build

The same game as every other build (issue #9), a `.3dsx` for the Homebrew
Launcher on a New 3DS or New 2DS XL with custom firmware (Luma3DS). An old
3DS or 2DS is not supported: the GBA core alone takes longer than a frame
there.

## Playing

- Copy `cyberworld-endless.3dsx` to `sdmc:/3ds/` and start it from the
  Homebrew Launcher.
- The ROM, Mega Man Battle Network 6: Cybeast Gregar (USA), goes in
  `sdmc:/3ds/cyberworld-endless/rom/`, or stays where 3DS players keep GBA
  games (`sdmc:/roms/gba`, `sdmc:/roms`, `sdmc:/gba`). Any file name works:
  the game reads the header and checks the contents.
- The first NEW GAME boots BN6 from power-on once, about 15 seconds of black
  screen; later starts load what it kept (`boot-3.state`).
- The picture fills the top screen's height (1.5x, its pixels mixed at their
  edges). `screen = whole` in `settings.ini` shows it sharp at 1x in the
  middle.
- Saves, `settings.ini`, `keys.ini` and `log.txt` are in
  `sdmc:/3ds/cyberworld-endless/`.
- The 3DS's buttons are the GBA's: A, B, L, R, START, SELECT, and the D-Pad
  or the Circle Pad. HOME pauses as for any game.

It runs below full speed for now: the GBA core takes 17 to 21 ms a frame on
the New 3DS's 804 MHz ARM11, where a frame has 16.7.

## Building

```sh
python3 build.py 3ds    # build/3ds/cyberworld-endless.3dsx
```

It builds in `docker/Dockerfile.3ds`: devkitPro's devkitARM with libctru and
citro2d, SDL2 built for the 3DS (`docker/sdl2.sh 3ds`) and mGBA's library
(`docker/mgba.sh 3ds`). `3ds/icon.png` is the Homebrew Launcher's icon.

What differs from the other builds:

- **Memory** (`src/core/main.c`): the app splits its memory itself before
  `main`, the heap as large as its 96 MB area takes, the linear heap (the
  screens' and the sound's buffers) the rest; a heap of all but the linear
  heap's share passed the area. mGBA's 3DS setup, linked in, copies the ROM
  into a buffer of its own, which `docker/mgba.sh` makes 16 MB, and draws
  32-bit pictures as on every other target.
- **The picture** (`src/core/present_3ds.c`): the software renderer draws a
  240 x 160 canvas into memory the GPU reads; the GPU's copy engine moves it
  into a texture and citro2d draws it. SDL's own present copied and turned
  every pixel on the CPU.
- **The GBA core** (`src/emu/emu.c`) runs on the New 3DS's third core where
  the system gives it one, the drawing of the frame done beside the next
  frame's emulation; `log.txt` says which.
- **Files**: mGBA's own files go to the SD card's file system, which takes
  paths without the C library's `sdmc:`. A rename onto a taken name is
  refused there, as on Windows (`cw_rename`).

## Testing on a 3DS

- **ftpd** (FTP) copies the build over and fetches `log.txt`:
  `curl -T build/3ds/cyberworld-endless.3dsx ftp://3DS-ADDRESS:5000/3ds/`.
- `frame_log = on` in `settings.ini` writes a line a second: frames shown and
  played, and a frame's update (the GBA's share, with and without its
  picture), drawing and present.
- **3dslink** (`/opt/devkitpro/tools/bin/3dslink -a 3DS-ADDRESS FILE.3dsx`,
  Y in the Homebrew Launcher) sends the build and shows its output here.
- Luma3DS keeps crash dumps in `sdmc:/luma/dumps/arm11/`: registers at 40
  bytes in, then the code and the stack; `arm-none-eabi-addr2line -f -e
  build/3ds/cyberworld-endless.elf ADDRESS` in the image names the place.
