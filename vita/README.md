# The PlayStation Vita build

The same game as every other build, for a PlayStation Vita or PlayStation
TV with custom firmware (HENkaku or h-encore, Enso): a `.vpk` that
VitaShell installs, with its bubble and LiveArea. Homebrew is the only
way there: a game that runs and patches another game's ROM cannot be a
signed title.

## Playing

- Copy `cyberworld-endless.vpk` to the memory card and install it from
  VitaShell (X on the file). The game has its bubble on the home screen.
- The ROM, Mega Man Battle Network 6: Cybeast Gregar (USA), goes in
  `ux0:data/cyberworld-endless/rom/`, or stays where Vita players keep
  GBA games (`ux0:roms/gba`, `ux0:roms`, `ux0:data/gba`). Any file name
  works: the game reads the header and checks the contents.
- Mega Man Battle Network 5: Team Colonel (USA), if you have it, goes
  beside BN6's ROM, or in any of those folders. Its net joins the runs as
  on every other system (docs/GUIDE.md, What you need).
- Saves, `settings.ini`, `keys.ini` and `log.txt` are in
  `ux0:data/cyberworld-endless/`.
- The GBA's buttons are on the Vita's: Cross is A, Circle B, L and R,
  START and SELECT, the D-pad or the left stick; Options' Controls moves
  them. The front touch screen shows the touch controls, as on a phone.
- Online is as on every system: the gate in Lan's HP, the Net Square,
  NetBattles, trades and the DeepNet, over the Vita's Wi-Fi.
- Closing the game from the LiveArea ends it at once, as every Vita game:
  the run goes on from its last checkpoint (a layer's start, a guardian
  arena's door), not from where MegaMan stood.

## Building

```sh
python3 build.py vita    # build/vita/cyberworld-endless.vpk
```

It builds in `docker/Dockerfile.vita`: VitaSDK with its packages (SDL2
2.32, libcurl 8 with OpenSSL), and mGBA's library (`docker/mgba.sh vita`:
the whole core, as on the 3DS and the Switch, not position-independent,
whose relocations `vita-elf-create` refuses). The Makefile turns the ELF
into `eboot.bin` (`vita-elf-create`, `vita-make-fself`), writes its
`param.sfo` (title ID `CWEN00001`, the version's major and minor as its
APP_VER) and packs the VPK with the bubble (`vita/icon0.png`,
`tools/app_icon.py`) and the LiveArea (`vita/bg.png` and
`vita/startup.png`, `tools/steam_art.py`, 8-bit as the LiveArea takes
them; `vita/template.xml`).

What differs from the other builds:

- **The start** (`src/core/start_vita.c`): a 256 MB heap and a 1 MB main
  thread's stack, which VitaSDK's start reads before `main` (its own
  heap is 32 MB); the CPU at 444 MHz and the GPU at 222; the network's
  module and memory; the memory card's folders, `log.txt` and the ROM's
  places.
- **The GBA core** (`src/emu/emu.c`) runs on a core of its own, its
  picture drawn beside it, as on the New 3DS; `log.txt` says so.
- **The renderer** (`src/core/platform.c`): SDL's own GXM renderer, named
  by its hint: SDL lists OpenGL ES first, through vitaGL, whose shaders
  need a compiler (`libshacccg.suprx`) most consoles have not been given.
- **Pads** (`src/core/pads.c`): SDL maps the Vita's buttons by their
  places, Cross the lower one, and the game names them as PlayStation's.
  The Vita has no motor: Options shows no rumble.
- **The network** (`src/online/online_stream_curl.c`,
  `src/analytics/analytics_http.c`): VitaSDK's libcurl with OpenSSL, as
  the 3DS's libcurl with mbedTLS, Let's Encrypt's roots given with each
  connection (`CURLOPT_CAINFO_BLOB`: OpenSSL has no store of the
  console's to read).
- **Files**: a rename onto a taken name is refused there, as on the 3DS
  (`cw_rename`). Remote play's pipes (`tools/play.py`) and quit signals
  are not built.

## Testing on a Vita

Built and checked by CI; not yet run on a console. The checks for a first
run, on a Vita with HENkaku or h-encore and VitaShell:

- **VitaShell's FTP** (SELECT in VitaShell) copies the VPK over and
  fetches `log.txt`: `curl -T build/vita/cyberworld-endless.vpk
  ftp://VITA-ADDRESS:1337/ux0:/`, then install it from VitaShell.
- The first start: `log.txt`'s "vita:" line (444 MHz, the 256 MB heap,
  the network up) and "emu: the GBA core runs on a core of its own", the
  title, NEW GAME to the first layer at full speed (`frame_log = on` in
  `settings.ini`: a line a second of the frames' times), Cross as A, the
  touch controls on a touch, a close from the LiveArea and CONTINUE.
- BN5's ROM beside BN6's: the title's "BN5 found", a layer of its areas.
- Online: the gate in Lan's HP reaches the Net Square, a trade, and a
  NetBattle, whose frames' times `frame_log` gives: whether the pair's
  two GBAs run in a frame's time on its cores
  (docs/online/arena/rollback/new-3ds.md measured the New 3DS's, which
  could not). A VPK takes no command line, so `--dev` switches are not
  for this build.
- The statistics: a yes at the first start, and the game's start counted
  on Umami's dashboard under `vita` (docs/devtools/statistics.md).
