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
fetched, built or shipped.

The Makefile compiles the game with the template's clang (target
`x86_64-sie-ps5`) and our own warnings, links it with the SDK's lld, the
template's CRT and linker script against the system's stubs, turns it
into `eboot.elf` and signs it into `eboot.bin` with the template's tool,
then lays out `CWEN00001/` (`sce_sys/param.json` at the repository's
root, as the catalog wants it committed; `ps5/icon0.png`,
`tools/app_icon.py`; the template's `libc.prx`) and zips it with the
folder at its top.

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
- **The C library** (`src/core/ps5_libc.c`): what the native app's
  runtime lacks of what PacBrew's OpenSSL, libcurl, libzstd and libiconv
  call, built on what the system has (`gmtime_r`, `pipe2`, `_setjmp`)
  or answered empty for paths the game never takes there (`getpwuid_r`,
  `sendmmsg`, `dladdr`).
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

Built and checked by CI; not yet run on a console. The checks for a first
run, on a jailbroken PS5 with ShadowMountPlus:

- The folder in `/data/homebrew/`, the title on the home screen.
- `log.txt` over FTP: the "ps5:" line, the ROM found, "emu:".
- The title, NEW GAME to the first layer at full speed, Cross as A, the
  picture on a 1080p and a 4K screen.
- Online: the gate in Lan's HP reaches the Net Square, a trade, a
  NetBattle; the statistics' question at the first start.
- An update: a later zip's folder over the earlier one, the saves kept.
