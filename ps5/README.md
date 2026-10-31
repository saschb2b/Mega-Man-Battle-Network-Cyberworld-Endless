# The PS5 build

The same game as every other build, as a native PS5 app for a jailbroken
PS5: an app folder, `CWEN00001/` (`eboot.bin`, `sce_sys/param.json` and
its icon, `sce_module/libc.prx`), zipped as Prospero Store, the PS5's
homebrew store, takes it. ShadowMountPlus registers the folder as a
title. Homebrew is the only way there: a game that runs and patches
another game's ROM cannot be a signed title. (A PS5 whose etaHEN
installs PS4 packages can take the PS4 build's PKG instead, ps4/README.md.)

## Playing

- From Prospero Store once it lists the game, or by hand: unzip
  `cyberworld-endless-ps5.zip` and copy its `CWEN00001/` folder into
  `/data/homebrew/` (over FTP); ShadowMountPlus registers it.
- The ROM, Mega Man Battle Network 6: Cybeast Gregar (USA), goes in
  `/data/cyberworld-endless/rom/`, or stays on a USB drive:
  `/mnt/usb0/roms/gba`, `/mnt/usb0/gba` or the drive's top. Any file name
  works. BN5 Team Colonel's goes beside it, or in any of those folders.
- Saves, `settings.ini`, `keys.ini` and `log.txt` are in
  `/data/cyberworld-endless/`, outside the app's folder, so an update
  keeps them; the PS4 build keeps its own there too.
- Cross is the GBA's A, Circle its B; Options' Controls moves the rest.
- Online is as on every system, and the anonymous statistics are asked
  for as elsewhere.

## Building

```sh
python3 build.py ps5    # build/ps5/cyberworld-endless-ps5.zip, its CWEN00001/ inside
```

It builds in `docker/Dockerfile.ps5`, from drakmor's
ps5-native-app-boilerplate (GPL-3.0), pinned to a commit and run once on
its own hello world: that fetches the public PS5 payload SDK and zlib,
each checked against its SHA-256, and builds the template's ELF
converter and FSELF writer, its clean-room `libc.prx` and its CRT;
PacBrew's ports for the SDK (SDL2 2.30, libcurl 8 on OpenSSL) come with
it, checksum-pinned too. mGBA's library is built with the template's
compiler (`docker/mgba.sh ps5`, `docker/ps5.cmake`). No Sony file is
fetched, built or shipped. One line of the template's converter is
corrected there before it is built: it put the RELRO segment's file at
`.got`'s, right only where nothing comes before `.got`. The game's large
`.data.rel.ro` does, and the system's loader refused the misplaced
segment ("not page aligned"), so 0.18.0 never started (the launch's
0x80aa001a).

The Makefile compiles the game with the template's clang (target
`x86_64-sie-ps5`) and our own warnings, links it with the SDK's lld, the
template's CRT and linker script against the system's stubs, turns it
into `eboot.elf` and signs it into `eboot.bin` with the template's tool,
then lays out `CWEN00001/` (`sce_sys/param.json` at the repository's
root, as the catalog wants it committed; `ps5/icon0.png`,
`tools/app_icon.py`; the template's `libc.prx`) and zips it with the
folder at its top.

`sce_sys/param.json` gives the app 1 GiB of flexible memory
(`kernel.flexibleMemorySize`, as RetroArch's does), from which the game's
heap is made (below); without it the system gives a native app little.

Each release raises `sce_sys/param.json`'s contentVersion, which is how a
console sees an update: the CHANGELOG's version's major plus one, its
minor and its patch (0.18.0 as `01.018.000`, 0.19.0 as `01.019.000`).
`build.py lint` holds the two together, so the release commit sets it.

The title ID is the game's own, `CWEN00001`, a custom prefix the catalog
takes: no PS5 game or other homebrew uses it, as the template's
`PPSA99999` many do. A store older than ProsperoStore 1.000.110 does not
show custom prefixes.

**License.** The PS5 binary links the payload SDK's runtime and PacBrew's
libraries, and ships the template's `libc.prx`, all GPL-3.0: the zip is
distributed under GPL-3.0's terms. The game's source, MIT-licensed and
public, is its corresponding source. The PS4's PKG is in the same place
with OpenOrbis's runtime.

What differs from the other builds:

- **The start** (`src/core/start_ps4.c`, the PS4's and the PS5's): the
  folders, `log.txt` and the ROM's places.
- **The system's modules.** A game's process gets only some of the
  system's modules, and a call into another is a call to nothing (SDL's
  keyboard was the first). The Makefile's `PS5_LOADED` lists those the
  system's loader put into the game's process; the link leaves out the
  stubs of the rest that the libraries would ask for (WebKit's POSIX, the
  kernel's system and web parts, the keyboard, the text dialog,
  libSceRandom) and stops at any module outside the list.
- **The C library** (`src/core/ps5_libc.c`, built hidden: the template's
  converter publishes nothing of an app's): what the native app's runtime
  lacks of what PacBrew's OpenSSL, libcurl, libzstd and libiconv call,
  built on what the system has (`gmtime_r`, `pipe2`, `_setjmp`) or
  answered empty for paths the game never takes there (`getpwuid_r`,
  `sendmmsg`, `dladdr`); the name lookups on libSceNet's resolver
  (`getaddrinfo`, `getnameinfo`); no keyboard and no text dialog, as SDL
  takes their errors (the game's own key grid takes text), and random
  bytes from `/dev/urandom`; and the folders read with the kernel's
  `getdents`, in the internal drive's 64 KB blocks, where libc's `opendir`
  is refused (EPERM).
- **The heap** (`src/core/ps5_heap.c`): libc's `malloc` there draws on the
  kernel's internal memory, some 14 MB, too little for SDL's 1080p window
  alone. The linker sends the game's and its libraries' `malloc`,
  `calloc`, `realloc`, `posix_memalign` and `free` (`--wrap`) to a 512 MB
  mspace of libSceLibcInternal's on flexible memory; a pointer from
  outside it (libc's own `strdup`, `getcwd`, `realpath`) goes back to the
  system's `free`.
- **The launch and the end** (`src/core/main.c`, `start_ps4.c`): the
  system passes the app its title ID as an argument, which the game
  passes over; and an app may not return from `main` or call `exit` (its
  process crashes on the way out, SIGSYS), so the game's end, the saves
  written, waits for the system to close it (`console_park`).
- **The network** (`src/online/online_stream_curl.c`,
  `src/analytics/analytics_http.c`): PacBrew's libcurl on OpenSSL, as the
  3DS's and the Vita's, Let's Encrypt's roots given with each connection;
  the random bytes `sceRandomGetRandomNumber`'s.
- **Pads** (`src/core/pads.c`): named as PlayStation's, Cross the GBA's A.
- Remote play's pipes (`tools/play.py`) and the quit signals are not
  built.

## Submitting to Prospero Store

Its [submit page](https://prosperostore.net/submit/) reads the
repository's latest release: the release's `cyberworld-endless-ps5.zip`
is the artifact, the release's tag its version, and `sce_sys/param.json`
at the tag the title ID, name and contentVersion. Type: Game; category:
homebrew-games; the icon `ps5/icon0.png` at the release's tag; the
author Saschb2b. Its button opens the record as a pull request on
darkxex/ps5-homebrew-catalog from the repository owner's account (0.18.0:
darkxex/ps5-homebrew-catalog#960, made with `gh` when GitHub's fork button
would not answer the browser extension, and from the zip, as 0.18.0's tag
had no `sce_sys/param.json` yet). A later release's update is the same
record with its new `version`, `artifact_url` and `sha256`.

## Testing on a PS5

First run on a console on 2026-10-31 (a jailbroken PS5, ShadowMountPlus,
the app as Prospero Store installed it): started from the home screen,
`log.txt` over FTP with the "ps5:" line, the 1080p display, BN6's and
BN5's ROMs found in `/data/cyberworld-endless/rom/`, the audio and the
title's music; the system's quit ends it without a crash. Still to check
by hand: NEW GAME to the first layer at full speed, Cross as A, a 4K
screen, online (the Net Square, a trade, a NetBattle), the statistics'
question, and an update over an earlier folder with the saves kept.

When it does not start, the console says why in its kernel log, not on
the screen: over the PS5's shell (shsrv, port 2323) `cat /dev/klog` while
the title is launched (`launch CWEN00001`). The loader's refusals are its
`[rtld]` lines; a crash prints the thread's registers, a backtrace and the
modules loaded. A backtrace's addresses in the game are its ELF's plus
0x400000: `llvm-symbolizer-18 --obj=build/ps5/cyberworld-endless.elf`
on each less 0x400000 names the function. A copy under another title ID
in `/data/homebrew/` (its param.json's titleId, contentId and conceptId
changed) is a test slot beside the installed app; a file the game is
running cannot be replaced over FTP, so stop it first (`kill -9`).
