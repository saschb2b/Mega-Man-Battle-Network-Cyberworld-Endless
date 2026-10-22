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
  The game then has its icon on the HOME Menu, and its banner plays a
  few seconds of the current trailer's music. FBI's Titles list deletes it again.
- **The Homebrew Launcher:** copy `cyberworld-endless.3dsx` to
  `sdmc:/3ds/` and start it there. It is the same game, with the same
  saves.
- The ROM, Mega Man Battle Network 6: Cybeast Gregar (USA), goes in
  `sdmc:/3ds/cyberworld-endless/rom/`, or stays where 3DS players keep GBA
  games (`sdmc:/roms/gba`, `sdmc:/roms`, `sdmc:/gba`). Any file name works:
  the game reads the header and checks the contents.
- Mega Man Battle Network 5: Team Colonel (USA), if you have it, goes
  beside BN6's ROM, or in any of those folders. Its net joins the runs as
  on every other system (docs/GUIDE.md, What you need): BN5's areas, their
  viruses, Navis, chips, Souls and DarkChips. The start then takes a few
  seconds more, as the game reads and checks BN5's file, and the title
  says "BN5 found" in its top right corner. A 3DS whose memory had no room
  for it would leave it unread and play BN6's net alone, and say so (the
  PET's home and `log.txt`); a New 3DS has room (Memory, below).
- The first NEW GAME boots BN6 from power-on once, about 15 seconds of black
  screen; later starts load what it kept (`boot-3.state`).
- The picture fills the top screen's height (1.5x, its pixels mixed at their
  edges). `screen = whole` in `settings.ini` shows it sharp at 1x in the
  middle.
- The bottom screen is the PET beside the game (issue #72), framed like
  BN6's own PET screens: the screen's name and the place in its header,
  HP, Zenny and BugFrags under it. On the net it shows the layer's map,
  always open: the floor MegaMan has seen, the way on to the exit or the
  guardian, the services and gates he has come near, and a mark on the
  frame's edge for those he senses. It is SELECT's map, larger; SELECT
  still opens it over the picture. In the folder editor it shows the
  whole folder as BN6's chip icons, the cursor's chip lit, with the
  folder's codes, elements and Mega and Giga chips, and the pack. In a
  battle it shows the chip under the Custom screen's cursor as a large
  card with its Library text, CROSSSELECT's Cross and Beast Out with
  what each gives, and while MegaMan fights, his chips in order, his
  Cross, Beast Out and Full Synchro, and the enemies with their HP. In
  the NaviCustomizer it shows the program under the cursor (its shape,
  whether it fits, whether it turns, what it does) and what RUN would
  bring as the board stands. In the town it is the PET at home:
  MegaMan's face, the run's next step and its setup, which the PET's
  menu, in the town or on the net, dims under ACCESSING; MegaMan's
  status adds the run's record, the Library the chip under the cursor
  as a large card (the codes it comes in, the run's copies) and its
  collection, E-Mail the mail's sender by face and name; in a shop it
  shows the entry under the cursor with what the run holds of it, at a
  trader what it takes and gives; on the title, the PET at rest with
  the profile's record, and whether BN5's net joins the runs.
- Saves, `settings.ini`, `keys.ini` and `log.txt` are in
  `sdmc:/3ds/cyberworld-endless/`.
- The 3DS's buttons are the GBA's: A, B, L, R, START, SELECT, and the D-Pad
  or the Circle Pad. HOME pauses as for any game.
- The first start asks whether the game may send anonymous play
  statistics (docs/GUIDE.md, Anonymous statistics); SELECT on the title asks
  again. A yes sends them over the 3DS's Wi-Fi with devkitPro's libcurl
  and mbedTLS, which carry Let's Encrypt's roots: the 3DS's own SSL
  module stops at TLS 1.1, which the server refuses.

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
Its banner is `3ds/banner.png` (`tools/steam_art.py`) with 2.9 seconds of
the current trailer's music, its BANNER (`tools/trailer_music.py --banner
--rate 32728`), made into a banner by bannertool. A piece that plays a
soundfont's samples (0.14.0's, its SOUNDFONT) cannot render in this image,
which has no soundfont: its banner is rendered once in
`docker/Dockerfile.music`'s image (`tools/trailer_music.py --write-banner
--piece v0_14_0_music`) and kept as our own audio in `3ds/banner-0.14.0.wav`
(its BANNER_AUDIO), which `--banner` copies at the rate asked; the
Makefile rebuilds the banner when that file changes. Write it again after
any change to the piece's banner bars.

What differs from the other builds:

- **Memory** (`src/core/start_3ds.c`): the app splits its memory itself before
  `main`, the heap as large as its 96 MB area takes, the linear heap (the
  screens' and the sound's buffers, and the GBA's own RAM, which mGBA puts
  there) the rest; a heap of all but the linear heap's share passed the
  area. mGBA's 3DS setup, linked in, takes a ROM buffer from the heap
  before `main` (16 MB, `docker/mgba.sh`) for loaders of its own that the
  game never calls: the core maps the game's padded 16 MB copy as it is
  (`src/emu/emu.c`), so the game gives the buffer back at its start. The
  heap then holds BN6's ROM (8 MB), the core's copy (16 MB) and BN5's ROM
  where it is read (8 MB): 32 MB, against the 40 MB BN6 alone had taken
  with the buffer held. BN5's ROM is read where the heap keeps 80 MB free
  for it, the core's copy still to come and a run's play. The tiles
  learned from a layer's area's original maps are kept for that area
  alone (`src/map/netmap.c`): kept for every area a run passed through,
  they filled the heap in long runs. An area a run comes back to is
  learned again, a few seconds more on that layer's making (`log.txt`:
  "area N learned again"). What play takes, and what a long run leaves,
  is measured in docs/EMULATION.md, The 3DS's memory. mGBA draws 32-bit
  pictures as on every other target.
- **The picture** (`src/core/present_3ds.c`): the software renderer draws a
  240 x 160 canvas into memory the GPU reads; the GPU's copy engine moves it
  into a texture and citro2d draws it. SDL's own present copied and turned
  every pixel on the CPU.
- **The bottom screen** (`src/core/platform_second.c`): every tenth frame,
  and at once when its panel changes, the second screen (`src/second/`)
  draws the PET's frame and its panel straight into the memory the GPU copies
  from (`gfx_draw_into`: its rectangles and
  text, blended as the software renderer blends them), and `present_3ds.c`
  draws it on the bottom screen once for each of the screen's two buffers,
  then leaves it until the next. A panel the game's state left as it was
  (a battle's, the folder's: not the map, where MegaMan's mark pulses) is
  not drawn again, nor copied, but once a second. Through the software
  renderer and read back, the map had taken 20 ms, a frame lost at each
  redraw.
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
- **NetBattles** are not offered on the 3DS: a NetBattle runs both
  players' GBAs on each system, and two GBAs on one ARM11 core take
  25.8 ms a frame with the picture (20.7 without) against a 60th of a
  second's 16.7. The Net Square, its cards, the referee's calls of the
  channel's battles, trades and the week's dive work as elsewhere, and
  the NetPass says why the ring stays closed. The Arena's code is built
  (its two cores share one ROM copy, 16 MB of the heap), so `--dev
  rollbench` and `--dev arena` measure it on the device
  (docs/ONLINE_ARENA.md, The New 3DS).
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
  picture), drawing and present, the second's longest update and
  drawing, and the heap ("heap N KB in use, M KB at most of 98304 KB":
  M is its high-water mark, which a layer's area learned raises).
- **The heap on this computer**: `python3 tools/heap3ds.py` plays long
  runs of the host build headless with the 3DS's heap modelled
  (`tools/heap3ds.c`: 96 MB, the core's copy kept, an allocation past it
  refused), as the 3DS before mGBA's buffer was given back, with BN6
  alone, and with BN5 read; per run its peak, the allocations refused
  and how deep it went. Before a device run of a change to what a layer
  allocates.
- **BN5 on the 3DS**, on a New 3DS with both ROMs (BN5's beside BN6's):
  `log.txt` says "mGBA's ROM buffer given back to the heap, 16384 KB",
  "BN5 found: Mega Man Battle Network 5: Team Colonel (USA)", "BN5 read
  and checked in N ms" with the heap, and after NEW GAME "the core
  started, the installs done" with the heap, beside the installs' own
  lines (BN5's Navis, chips' code, families); the title's "BN5 found" note
  and the PET's home ("BN5 found: the older net joins the runs"); a layer
  of each of BN5's areas, over 3dslink with `-- --scene emu --net-biome
  x0` (to `x5`), or new games until an act's card names one, its music,
  bystanders and a battle of its viruses; with `frame_log = on`, the
  heap's high-water mark through a few acts, under the heap's size;
  CONTINUE on one of its layers; and BN5's ROM taken away: the PET says
  "BN5 not found beside BN6", and a run continued goes on in BN6's tiles.
- **3dslink** (`/opt/devkitpro/tools/bin/3dslink -a 3DS-ADDRESS FILE.3dsx`,
  Y in the Homebrew Launcher) sends the build and shows its output here
  (`-s`, a line at a time). Arguments follow `--`: `--data-dir
  sdmc:/3ds/cyberworld-endless/test` keeps a test's files apart (the
  ROMs are still found in the game's own folder), and `--dev
  statistics=off,rollbench,arena,arenaboth,arenabusy,arenadelay=1,arenalag=4,quitend`
  measures the Arena's pair, plays a battle by itself and quits back to
  the launcher, so one Y covers it. The netloader takes one connection:
  a probe of its port ends its wait.
- Luma3DS keeps crash dumps in `sdmc:/luma/dumps/arm11/`: registers at 40
  bytes in, then the code and the stack; `arm-none-eabi-addr2line -f -e
  build/3ds/cyberworld-endless.elf ADDRESS` in the image names the place.
