# The PS4 build (and the PS5's)

The same game as every other build, for a PS4 with GoldHEN (a jailbroken
PS4): a fake PKG, installed from its Package Installer, with its tile on
the home screen. A jailbroken PS5 that installs PS4 packages (etaHEN)
should run the same PKG; the PS5's own build, a native app for Prospero
Store, is ps5/README.md. Homebrew is
the only way there: a game that runs and patches another game's ROM
cannot be a signed title.

## Playing

- Copy `cyberworld-endless.pkg` to a USB drive (or send it over the
  network) and install it from GoldHEN's Package Installer, or on a PS5
  from etaHEN's.
- The ROM, Mega Man Battle Network 6: Cybeast Gregar (USA), goes in
  `/data/cyberworld-endless/rom/` on the internal drive (GoldHEN's FTP
  reaches it), or stays on a USB drive: `/mnt/usb0/roms/gba`,
  `/mnt/usb0/gba` or the drive's top. Any file name works: the game reads
  the header and checks the contents.
- Mega Man Battle Network 5: Team Colonel (USA), if you have it, goes
  beside BN6's ROM, or in any of those folders.
- Saves, `settings.ini`, `keys.ini` and `log.txt` are in
  `/data/cyberworld-endless/`.
- The GBA's buttons are on the controller's: Cross is A, Circle B, L1 and
  R1, OPTIONS for START, the D-pad or the left stick; Options' Controls
  moves them and sets SELECT.
- Online is as on every system: the gate in Lan's HP, the Net Square,
  NetBattles, trades and the DeepNet. The anonymous statistics are not
  sent from this build, and it never asks.
- Closing the game from the home screen ends it at once: a run goes on
  from its last checkpoint.

## Building

```sh
python3 build.py ps4    # build/ps4/cyberworld-endless.pkg
```

It builds in `docker/Dockerfile.ps4`: the OpenOrbis toolchain (its
clang for the PS4's FreeBSD-like system, its libc, SDL2 and the system's
libraries' stubs), with mbedTLS (`docker/mbedtls.sh`) and mGBA's library
(`docker/mgba.sh ps4`) built for it through `docker/ps4.cmake`. The
Makefile links the ELF with lld and the toolchain's script, turns it into
`eboot.bin` (`create-eboot`), writes its `param.sfo` (title ID
`CWEN00001`, content ID `IV0000-CWEN00001_00-CYBERWORLDENDLES`) and packs
the fake PKG with `PkgTool.Core` from `ps4/pkg.gp4`, its icon
`ps4/icon0.png` (`tools/app_icon.py`). The package holds the game alone:
OpenOrbis's samples ship Sony's `libc.prx` and `libSceFios2.prx` taken
from a game, which this one neither needs (its libc is linked in) nor may
carry.

**License.** The PKG links OpenOrbis's runtime (GPL-3.0): it is
distributed under GPL-3.0's terms, the game's public MIT-licensed source
its corresponding source.

What differs from the other builds:

- **SDL 2.0.9**, the toolchain's: `src/core/sdl_old.h`, which only this
  build reads first, stands in for what the game calls that SDL lacks
  (textures' scale modes, which keep nearest filtering; pads' kinds; the
  MISC1 button).
- **The start** (`src/core/start_ps4.c`): `/data/cyberworld-endless` for
  the saves and `log.txt`, the ROM's places on the internal drive and USB
  drives, and a stand-in for musl's `__pthread_setcancelstate`, which
  OpenOrbis's libc calls from its timed waits and leaves undefined.
- **The network** (`src/online/online_stream_tls.c`): the SDK has no
  libcurl, so the Net Square's stream is BSD sockets and mbedTLS, with
  Let's Encrypt's roots the game carries and the kernel's random bytes
  (`getentropy`). Checked on this computer's build against Let's
  Encrypt's ISRG Root X1 test site.
- **Pads** (`src/core/pads.c`): named as PlayStation's, Cross the GBA's A.

## Testing on a PS4

Built and checked by CI; not yet run on a console. The checks for a first
run, on a PS4 with GoldHEN:

- The PKG installed from the Package Installer, its tile and title.
- `log.txt` over GoldHEN's FTP: the "ps4:" line, the ROM found, "emu:".
- The title, NEW GAME to the first layer at full speed (`frame_log = on`
  in `settings.ini`), a layer's making on the main thread's stack, Cross
  as A, the picture at 6x on a 1080p screen (SDL 2.0.9 may draw it
  through its software renderer).
- A close from the home screen and CONTINUE.
- Online: the gate in Lan's HP reaches the Net Square, a trade, a
  NetBattle.
- The same on a PS5 with etaHEN.
