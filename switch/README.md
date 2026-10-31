# The Nintendo Switch build

The same game as every other build, for a Switch with custom firmware
(Atmosphère): a `.nro` for the Homebrew Menu. Homebrew is the only way
there: a game that runs and patches another game's ROM cannot be a
signed title.

## Playing

- Copy `cyberworld-endless.nro` into `sdmc:/switch/cyberworld-endless/`
  and start it from the Homebrew Menu. Open the Menu by holding R while
  you start a game: the game then has a title's whole memory. Opened from
  the Album, the Menu runs it as an applet, with less; `log.txt`'s first
  line says which, and how much it had.
- The ROM, Mega Man Battle Network 6: Cybeast Gregar (USA), goes in
  `sdmc:/switch/cyberworld-endless/rom/`, or stays where Switch players
  keep GBA games (`sdmc:/roms/gba`, `sdmc:/roms`, `sdmc:/gba`). Any file
  name works: the game reads the header and checks the contents.
- Mega Man Battle Network 5: Team Colonel (USA), if you have it, goes
  beside BN6's ROM, or in any of those folders. Its net joins the runs as
  on every other system (docs/GUIDE.md, What you need).
- Saves, `settings.ini`, `keys.ini` and `log.txt` are in
  `sdmc:/switch/cyberworld-endless/`.
- The GBA's buttons are on the Switch's buttons of the same names: A, B,
  L and R, Plus for START and Minus for SELECT, the D-pad or the left
  stick; Options' Controls moves them.
- Online is as on every system: the gate in Lan's HP, the Net Square,
  NetBattles, trades and the DeepNet, over the console's own Wi-Fi.
- HOME closes it as any homebrew; a run is kept where MegaMan stands.

## Building

```sh
python3 build.py switch    # build/switch/cyberworld-endless.nro
```

It builds in `docker/Dockerfile.switch`: devkitPro's devkitA64 with libnx
and its portlibs (SDL2 2.28, libcurl), and mGBA's library
(`docker/mgba.sh switch`: the whole core, as on the 3DS, without the
OpenGL ES 3 renderer mGBA turns on there). `switch/icon.jpg`
(`tools/app_icon.py`) is the Homebrew Menu's icon; `nacptool` gives the
title, the author and the version.

What differs from the other builds:

- **The start** (`src/core/start_switch.c`): the sockets, the output to
  nxlink where the game came over it, else to `log.txt` beside the saves,
  the SD card's folders, the ROM's places and the memory the Homebrew
  Menu gave.
- **Pads** (`src/core/pads.c`): libnx's pads are one "Switch Controller"
  a player, which SDL's own mapping names by their places, an Xbox pad's;
  the game maps them by their labels, as SDL names a Switch pad's buttons
  elsewhere, so A is the button marked A.
- **The network** (`src/online/online_stream_curl.c`,
  `src/analytics/analytics_http.c`): devkitPro's libcurl 7.69 over the
  console's own TLS service, as the 3DS's libcurl with mbedTLS. The
  console trusts Let's Encrypt's ISRG Root X1 from its system 10.1.0 on;
  the game adds the roots it carries (`analytics_roots.c`) to the
  service's context before each connection (`CURLOPT_SSL_CTX_FUNCTION`:
  7.69 has no `CURLOPT_CAINFO_BLOB`), so an older system connects too.
- **Files**: a rename onto a taken name is refused there, as on the 3DS
  (`cw_rename`). Remote play's pipes (`tools/play.py`) and quit signals
  are not built.

## Testing on a Switch

Built and checked by CI; not yet run on a console. The checks for a first
run, on a Switch with Atmosphère:

- **nxlink** sends the game over Wi-Fi, its output in the terminal: in
  the Homebrew Menu press Y (the netloader), then
  `docker run --rm --network host -v "$PWD":/src -w /src cyberworld-switch
  /opt/devkitpro/tools/bin/nxlink -a SWITCH-ADDRESS -s
  build/switch/cyberworld-endless.nro`, with `-- --scene emu --net-biome 0`
  and the game's other options after it.
- **ftpd** (or sys-ftpd) copies the `.nro` over and fetches `log.txt`.
- The first start: `log.txt`'s "switch:" line (a title's or an applet's
  memory, docked or handheld, the sockets up), the title, NEW GAME to the
  first layer, A on the button marked A, the picture docked and in the
  console's hands (and a dock or an undock while playing), HOME and
  closing it, then CONTINUE on the same layer.
- BN5's ROM beside BN6's: the title's "BN5 found", a layer of its areas.
- Online: the gate in Lan's HP reaches the Net Square, a trade, and a
  NetBattle; `--dev rollbench` says whether the pair's two GBAs run in a
  frame's time (docs/online/arena/rollback/new-3ds.md measured the New
  3DS's).
- The statistics: `--dev statscheck` sends one `game-dev-check` event
  (docs/devtools/statistics.md).
- `frame_log = on` in `settings.ini`: a line a second of the frames' times.
