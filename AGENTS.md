# AGENTS.md

## Project and scope

Cyberworld Endless is a roguelike that runs Mega Man Battle Network 6:
Cybeast Gregar (USA) from the player's own ROM on an embedded mGBA core and
directs it: generated layers in the game's map formats, the run's structure,
loot and saves. It is written in C11 with SDL2 and ships as a PortMaster port
for ROCKNIX, the primary target, as a Linux desktop build, as a Windows
build, as a macOS app, as a browser build on GitHub Pages and as a
homebrew build for the New 3DS (a CIA for the HOME Menu and a .3dsx) and an
iPhone and iPad app through AltStore and SideStore. Reference
devices: Retroid Nova (1280x960) and Retroid Pocket Flip 2 (1920x1080).

Never commit or publish ROMs, save files, extracted assets, emulator save
states or disassembly files. `.gitignore` covers the usual names; check
`git status` before committing anything. Screenshots of the game running
are the one exception: `docs/screenshots/` and the short videos in
`docs/clips/`, made by `build.py screenshots`, `build.py clips` and
`tools/trailer.py` (the trailer, its music original), for the README and
the site.

## Read before changing

| Task | Source of truth |
| --- | --- |
| Player experience, controls, install | [README.md](README.md) |
| How the engine drives the game, and the memory it writes | [docs/EMULATION.md](docs/EMULATION.md) |
| The real world and the town | [docs/OVERWORLD.md](docs/OVERWORLD.md) |
| Where ROM data lives and how it was found | [docs/ROM_DATA.md](docs/ROM_DATA.md) |
| What is original, generated or adapted | [docs/FIDELITY.md](docs/FIDELITY.md) |
| What carries over between runs: unlocks, the run's setup, threat | [docs/META.md](docs/META.md) |
| The PET's entries in a run: Save, E-Mail (the lab's mails), KeyItem | [docs/PET.md](docs/PET.md) |
| The rival, Chaud and ProtoMan: busting duels, the record, what his respect opens | [docs/RIVAL.md](docs/RIVAL.md) |
| Other games' ROMs (Battle Network 5 first): what they can lend a run, and how | [docs/MULTIROM.md](docs/MULTIROM.md) |
| What the original games have, do and lack: the sources to look in first, how a claim is verified, what players taught us | [docs/SOURCES.md](docs/SOURCES.md) |
| What the project has mapped of BN6 and BN5, written out for others: symbol files for debuggers, tables, how they stay in step; bn6f's functions located in Gregar and BN5, its names kept out of git | [docs/SYMBOLS.md](docs/SYMBOLS.md) |
| Shipped changes | [CHANGELOG.md](CHANGELOG.md) |

## Layout

| Path | Contents |
| --- | --- |
| `src/core/` | Entry point, platform (window, canvas, input, timing), ROM access and `RomLayout`, chip and virus data, loot, run state and saves |
| `src/gfx/` | Sprite decoding and animation, ROM tiles, the font, the QR code (`qr.c`, as the site's `qr.js`) |
| `src/audio/` | MP2K sequencer and mixer (title music and sounds, and the start's chime of our own); the core's sound during play |
| `src/net/` | Layer generation (rooms, walkways, objects) |
| `src/scenes/` | The start (`scene_intro.c`: the developer's boot screen and MegaMan's word on GitHub, before the title at a plain start), the title (with the run summary) and the sprite gallery |
| `src/emu/` | The mGBA core, calls into the game through hooks (`gamecall.c`: warps, chat), hooks on its code (`hook.c`), its wait for VBlank halted (`idle.c`), boot, event flags, debug output, the autopilot and the scene (`docs/EMULATION.md`); `bn6.h` names the game's addresses and the fields of its structures |
| `src/map/` | Layers as game maps: tiles learned from the original maps, walls and warp-pad triggers, the map tables taken over |
| `src/layer/` | What stands on a layer: NPC and text scripts, services, shops, choices, guardians |
| `src/director/` | The run on the game: the town, layers, warps, encounters, bosses, checkpoints, powers; the game's events from its hooks (`events.h`: battles in `encounter.c`, maps, choices and key items in `events.c`), taken up after each frame |
| `src/world/` | The real world: the town where a run begins, learned from Central Town's tiles and planned per run (`docs/OVERWORLD.md`) |
| `tests/test_core.c` | ROM-free unit tests; `tests/test_emu.c` the hooks on mGBA, and two cores side by side as BN6's and the guest's run (each one's hooks, RAM, sound and state its own), with ROMs of its own bytes; `tests/test_add_to_steam.py` runs `linux/steam/add-to-steam.py` against a made-up Steam folder; `tests/lint/` the baselines `build.py lint` checks against |
| `tools/romlab/` | libmgba research harness (dev only, needs your ROM) |
| `tools/uinput_keys.py` | On-device input injection for testing |
| `tools/play.py` | Playtests: the Linux build headless (`--remote`), played a batch of input at a time, a picture and the state in words after each |
| `.claude/skills/playtest-loop/` | The playtest loop as a skill: a persona plays through `tools/play.py`, its reports are triaged and fixed, and the loop's log (`lessons.md`) grows with every session |
| `.claude/skills/game-design/` | The game-design skill (from saschb2b/skills): the lens, pattern catalog and frameworks to reason from before a design decision |
| `.claude/skills/agent-browser/` | The agent-browser skill: a real Chrome driven from the shell (`npm i -g agent-browser`), to look over the site at desktop and phone widths |
| `port/` | The PortMaster port: the launcher, `port.json`, `gameinfo.xml` and the README of PortMaster-New's `ports/cyberworld/`; `build.py package` puts them in that folder's layout in `build/port/cyberworld/`, with the screenshot (`docs/screenshots/portmaster.png`), the binary and one `licenses/LICENSE.<part>.txt` per part |
| `linux/` | The Linux release: its README, the tar.gz's menu installer, the `.desktop` entry, AppStream metadata and the icons (`tools/app_icon.py`) that the AppImage, the `.deb` and the Flatpak (`linux/flatpak/`: its manifest, how to build it and bring it to Flathub) carry. The app ID `io.github.saschb2b.Mega-Man-Battle-Network-Cyberworld-Endless` is the window's class too (`src/core/platform.h`); `src/core/desktop.c` asks for the ROM and adds the AppImage to the menu. `linux/steam/`: `add-to-steam.py` (the game as a non-Steam shortcut, with the library artwork `tools/steam_art.py` draws), which every Linux build carries and `--add-to-steam` runs |
| `windows/` | The Windows release: its README, the NSIS installer script (per user, no administrator), the .exe's manifest (UTF-8 paths, per-monitor DPI), resource script and icon (`tools/app_icon.py`); `src/core/desktop_win.c` is its desktop (the ROM in Windows' file dialog and Downloads) and `src/core/compat.h` what differs from POSIX |
| `android/` | The Android app (`build.py android`): a Gradle project whose `RomActivity` asks once for the folder the ROMs are in (Android's folder picker, or the files in one picker; each `.gba` told by its game code, BN6's and BN5's checked by SHA-1 and copied into the app's files; the folder looked in again at each start, the icon's ROMs shortcut opens the page again) and whose `GameActivity` is SDL's activity running the game's C in a process of its own (`:game`, ended with the session), built by the NDK from every `src/*/*.c` (`app/jni/CMakeLists.txt`) with SDL2's own source and the GBA core; its README says how releases are signed |
| `macos/` | The macOS app: `deps.sh` (SDL2 and mGBA as universal static libraries, on a Mac), the bundle's `Info.plist`, its icon (`tools/app_icon.py`) and the .dmg's README; `build.py macos` runs only on a Mac, so CI's macOS job builds and checks it |
| `ios/` | The iPhone and iPad app (`build.py ios`, on a Mac with Xcode): its README (AltStore and SideStore, the ROM through Files, building), `deps.sh` (SDL2 and mGBA as static arm64 libraries for the phone or the Simulator), the bundle's `Info.plist` and its icons (`tools/app_icon.py`). `src/core/ios.m` is UIKit's (the Files pickers for the ROM folder or files and the folder's looks at each start, haptics, the safe area, the `main` that hands SDL's to UIKit); `tools/altstore_source.py` writes the AltStore source each release carries. CI's ios job builds the IPA and starts the Simulator's app |
| `3ds/` | The Nintendo 3DS build (`build.py 3ds`, issue #9): its README (playing, building, testing on a 3DS), the icon, the CIA's title settings (`cia.rsf`, for makerom) and the HOME Menu's banner (`banner.png`, `tools/steam_art.py`; its sound the trailer's first bar, `tools/trailer_music.py --seconds`). `src/core/present_3ds.c` puts the canvas on the top screen through the GPU (citro2d), and the layer's map on the bottom one (`director_draw_second_screen`, drawn by `platform.c` every tenth frame); `src/emu/emu.c` runs the GBA core on the New 3DS's third core where the system gives one, its picture drawn on the main core (mGBA's threaded video); `src/core/main.c` splits the app's memory |
| `web/` | The project site on GitHub Pages, laid out like BN6's PET screens: the home page (`index.html`, `assets/`), the downloads in `download/` (the newest release's files, one system at a time), the player in `play/` (ROM check and storage: BN6's, and BN5's beside it, each told by its game code and checked by its SHA-1; scaling), the FAQ in `faq/` as the PET's E-Mail, its answers taken from the docs, and `404.html`. `assets/qr.js` draws the 3DS title's link as a QR code for FBI; `assets/site.js` sends the analytics' events (Umami: what was done, never by whom; links name theirs with `data-umami-event`). Each page has its canonical address, Open Graph tags and schema.org data; `build.py site` adds the sitemap, the FAQ's FAQPage data and the content hash on each page's own files. Its frames and icons (`assets/ui/*.png`) are drawn by `tools/site_art.py`, not taken from the ROM |
| `docs/screenshots/` | Screenshots of the game for the README and the site (`build.py screenshots`) |
| `docs/clips/` | Short videos of the game for the site: WebM, MP4 and a poster each, and a GIF of those the README shows (`build.py clips`, ffmpeg in a pinned image); the trailer, the site's hero, and `trailer-play.png`, its poster with a play button, which the README links to the MP4 (`tools/trailer.py`) |
| `docker/` | Build images: `Dockerfile` (host, Debian trixie), `Dockerfile.portmaster` (the PortMaster port's aarch64 binary: Debian bullseye's glibc 2.31 and SDL2 2.0.14, older than any firmware PortMaster serves; `debian/eol` from its snapshot), `Dockerfile.linux` (desktop release, bookworm, SDL2 from source), `Dockerfile.windows` (MinGW-w64, SDL2 and mGBA static, NSIS), `Dockerfile.android` (the SDK, NDK and Gradle, SDL2's source, mGBA per ABI), `Dockerfile.web` (Emscripten, mGBA without threads), `Dockerfile.3ds` (devkitPro's devkitARM, SDL2 and mGBA's library for the 3DS, makerom and bannertool from their sources for the CIA: `ctrtools.sh`) |
| `.github/` | CI (`ci.yml`: checks, every target, Pages from `main`), releases (`release.yml`, on `v*` tags), the cached image build action, Dependabot |

## Rules

- A claim about the original games (what BN6 or BN5 has, does or lacks)
  is looked up in docs/SOURCES.md's sources before it shapes a design,
  a doc or code, and written as verified (how) or assumed; a negative
  one names the search that came up empty. Hardcore players know the
  games better than we do: a player's claim is a lead to verify and
  credit there.
- Read data from the ROM through `RomLayout` offsets or the addresses in
  `src/emu/bn6.h`. Do not embed or generate files containing Capcom
  graphics, text, samples or sequences (sprites, tiles, palettes, fonts,
  text dumps, music); tables the engine derives are numbers only.
  Screenshots of whole frames of the game running, for the README and the
  site, are allowed; crops that isolate a sprite, a logo or a mugshot as an
  asset are not.
- Patch only the core's in-memory ROM copy, and put new data in the free
  space map in `docs/EMULATION.md`. Never write the player's ROM file.
- A new ROM offset needs a note in `docs/ROM_DATA.md` saying how it was
  located and how to verify it.
- The game's 240x160 picture stays whole at a whole-number scale on every
  screen shape. The exceptions: the 3DS's top screen (400x240), where 1x
  is the only whole scale: there it fills the screen's height at 1.5x by
  default, and `screen = whole` in `settings.ini` gives 1x; and screens
  where a whole scale leaves the picture a quarter smaller or more than
  filling would (640x480: 2x against 2.67x, issue #36), which fill it with
  sharp scaling (whole-scaled, then smoothly to size) by default, and
  `screen = whole` or `fill` in `settings.ini` sets it either way; and a
  phone or tablet held upright, its touch controls under the picture,
  which fills the screen's width the same way (1080 wide: 4.5x against
  4x) as far as the controls keep their size, `screen = whole` keeping
  whole scales there too.
- MegaMan walks as BN6 walks him, always: no lining-up help, no sliding
  onto a walkway, no movement assist of any kind (one was removed by the
  owner's choice, docs/FIDELITY.md). Where walking drags (walkway mouths,
  lanes), the generated maps change instead, connected as Capcom's are
  (docs/LEVEL_DESIGN.md, Navigation).
- Draw calls must not change game state. Input is read once per frame in
  `platform_poll`.
- The C builds warning-free with the Makefile's `WARN` (`-Wall -Wextra`,
  `-Wshadow`, `-Wcast-qual`, `-Wwrite-strings`, `-Wformat=2` and more; issue
  #19) on every target, `-Werror` in CI. `build.py lint` holds the rest to
  its baselines in `tests/lint/`: no new finding of GCC's analyzer, no new
  function no build reaches, no function joins or grows past CCN 25 or 120
  lines, no ROM offset without its name in `docs/ROM_DATA.md`, no field of
  the game's structures as a bare offset outside `src/emu/bn6.h` (issue
  #34: `BN6_PLAYER_X`, not `BN6_PLAYER + 0x1C`), and no ROM,
  save, state or large file in git outside `docs/`. A baseline only
  shrinks: after fixing one, `build.py lint --update`.
- Keep a layer reproducible from `run.layer_seed`: a checkpoint rebuilds the
  layer before it loads `run.state`, so generation and object placement
  must not depend on the game's RAM.
- Run state is saved as a raw struct with a checksum. Changing `Run` breaks
  old saves: bump `RUN_MAGIC` in `save.c` (a `_Static_assert` on its size
  there fails the build until someone does, and sets the new size). Changing
  what a layer seed makes (layouts, object placement, loot and stock rolls):
  bump `LAYER_MAKE` in `src/net/layer_make.h`, so a run saved by an older
  build continues its layer afresh; where generation changed, the unit tests
  fail on `LAYER_MAKE_HASH` beside it and name the new hash to set with
  it. Emulator states (`boot-3.state`, `run.state`) are made on the device
  and never committed.

## Game design

A change that decides how the game plays (balance values, scaling, drop
rates, prices, rewards, progression, the NaviCust and other loadouts, menus
and screens, "is this fun") goes through the game-design skill first: name
the dialectic it serves, the loop layer it feeds and the patterns it uses,
before the numbers. This holds mid-implementation too.

## Build and verify

```sh
python3 build.py            # host + aarch64 binaries (Docker)
python3 build.py test       # ROM-free unit tests
python3 build.py lint       # the code's checks against tests/lint (--update after fixing one)
python3 build.py shot --scene emu --run-depth 2 --frames 400 --shot "300:/src/.build/a.bmp"
CYBERWORLD_AUTOPILOT=1 python3 build.py shot --scene emu --frames 15000   # walk layers, fight
python3 build.py package    # build/port/cyberworld (PortMaster-New's layout) and the port's zip
python3 build.py run        # the Linux desktop build, played here in a window
python3 build.py windows    # build/release: the Windows installer and zip (MinGW-w64, NSIS)
python3 build.py android    # build/release/cyberworld-endless.apk (the NDK and Gradle in Docker; android/README.md)
python3 build.py macos      # build/release/cyberworld-endless-macos.dmg, on a Mac only (Apple's SDK)
python3 build.py ios        # build/release/cyberworld-endless.ipa, on a Mac with Xcode (--simulator: the Simulator's app)
python3 build.py flatpak    # build/release/cyberworld-endless.flatpak (flatpak-builder on this machine)
python3 build.py 3ds        # build/3ds: cyberworld-endless.cia and .3dsx (devkitARM in Docker; 3ds/README.md)
python3 tools/play.py start NAME [--fresh]   # a headless game for a playtest (build.py linux first)
python3 tools/play.py do NAME "press A; hold UP 30" [--every 10] [--keep DIR]   # input, then a picture and the state (--keep: each frame into DIR)
python3 build.py serve      # the site and the browser build on http://localhost:8080
python3 build.py tiles [SEEDS]  # the tile test: every area's layers drawn, wrong-floor tiles and seams per area
python3 build.py town [SEEDS]  # the town drawn per seed, and the game around it
python3 build.py world        # the real world's original maps, drawn and toured
python3 build.py screenshots [NAMES]   # docs/screenshots from scripted headless runs
python3 build.py clips [NAMES]         # docs/clips: the same runs as 30 fps videos
python3 tools/site_art.py              # web/assets/ui: the site's pixel-art frames and icons
python3 tools/app_icon.py              # linux/icons and src/core/app_icon.h: the application icon
python3 tools/steam_art.py             # linux/steam: Steam's library artwork (capsules, hero, logo); 3ds/banner.png
python3 tools/social_preview.py        # build/social-preview.png: the repository's social preview, uploaded by hand
python3 tools/trailer.py [--keep]      # docs/clips/trailer.*: the 20-second trailer, its music by tools/trailer_music.py
python3 tools/before_after.py v0.5.3 --new 0.6.0   # docs/screenshots/compare-*.png: one spot as the last release and this build draw it, for the notes
python3 build.py release    # build/release/: the PortMaster zip, the Linux AppImage, .deb and tar.gz, the site zip
```

The host, Linux, Windows and macOS builds are desktop builds
(`CW_DESKTOP`): a resizable window, saves in
`~/.local/share/cyberworld-endless` (Windows:
`%LOCALAPPDATA%\cyberworld-endless`, macOS: `~/Library/Application
Support/cyberworld-endless`), the ROM looked for there, beside the binary
and in `./rom`. The ROCKNIX build fills the screen
and takes both folders from its launcher. `build.py run` is the quickest way
to play a change; the headless `shot` stays the way to capture one.

The browser build (`__EMSCRIPTEN__`) has no threads: the page drives one
game frame per 1/60 s (`emscripten_set_main_loop`), and writes reach
IndexedDB through `platform_persist()`, which saves and states call. Keep
the frame loop free of blocking waits: BN5's guest core runs its frames in
BN6's frames' place, and its first boot a slice a frame (`guest_warm` at
the title, `guest_boot_slice` behind a note when a battle waits for it;
docs/MULTIROM.md, Guest battles). `build.py serve` also serves the
developer's ROMs, told by their SHA-1, for tests in a local browser:
BN6's at `/.dev/rom.gba`, BN5's at `/.dev/bn5.gba`; the page itself only
takes the ROMs the player chooses.

CI runs without a ROM: it builds every target with `-Werror`
(`WERROR=1`, set when `CI` is), runs `build.py lint` and
`tests/test_core.c` under the sanitizers, then `tests/test_add_to_steam.py`. Everything that needs the game (captures, autopilot, atlas,
pacing) stays local.

`shot` runs headless in the build image with the repository at `/src` and
`~/.cache/mmbn-ref/roms` (override with `CYBERWORLD_ROM_DIR`) mounted
read-only; `--data-dir` defaults to `.build/data`. `--scene emu` starts a new
run on the game, `--run-depth N` at depth N, `--net-biome N` in one area (`xN` another game's area N, its ROM beside BN6's: docs/MULTIROM.md),
`--guardian N` with navi N guarding every area,
`--seed S` with a given seed, `--scene town` from the town as NEW GAME does,
`--scene summary` the title's run summary (a win's with `--setup short` at
`--run-depth 10`), `--setup NET,FOLDER,THREAT,HELPERS[,CROSS]` the run's setup (CROSS the navi whose Cross it brings, 1-5),
`--marks HEX` the title's marks, `--touch` the touch controls from the
start (as on a phone; with `--size` a phone's screen and `--dpi N` its density), `--talk NAME:FRAME,...` opens a
layer's chats at its frames (npc, shop, heal, programs, gift, challenge,
undernet, gate, navigate for a Navi gate, vault, duel for ProtoMan's terms, official for an official gate, trader and bugtrader for a Chip or BugFrag Trader; intro, defeat, reward for the guardian; status for L; rumor for the layer's whisper (rumors.c); dark for a flame of darkness (docs/META.md);
fragment for MegaMan's words at a ScrtData; bugfrags gives 50 BugFrags, keys an Unlocker, rushfood three RushFood, wwwid a WWW-ID, zenny 10000 zenny, regup a RegUP3; guest a battle in BN5's engine at once on its territory, docs/MULTIROM.md). `--input "FRAMES:BUTTONS,..."` scripts the
buttons (`UP+RIGHT`, `A`; play.py's dev steps too, `0:place X Y FACE`, `0:flags FROM TO 1` and `0:battle`, the layer's next random battle as soon as MegaMan is free on its map, BN5's on its territory; `300:battle` waits its 300 frames first), `--taps "FRAME:X,Y[>X2,Y2];..."` fingers at screen pixels (a tap, or a drag over 20 frames: the touch controls, their menu and editor), `--shot FRAME:PATH,...` and `--shot-range A:B:PREFIX`
save frames (the canvas; `--screen-shot FRAME:PATH,...` the whole screen, the touch controls on it; `--second-shot FRAME:PATH,...` the second screen, the 3DS's bottom one with the layer's map), and `--sheet CAT:IDX:ANIM[:PAL]:PATH` or `--sheet
@CAT:FIRST:COUNT:PATH` draw sprites. Environment variables reach the image
only when `build.py` lists them (`CYBERWORLD_EMU_DEBUG`, `CYBERWORLD_AUTOPILOT`
and the audio ones).

`tools/play.py` keeps one game running per NAME (`.build/play/NAME`: its
saves in `data/`, every command in `history.txt`, the seed in its first
line), so a playtester, person or agent, plays it like the handheld,
reading each picture; a session replays from the seed, `data0/` and the
history. Each start copies the current build into the session's `bin/`;
with `NAME/bin.pin` present its restarts keep that copy, so a rebuild
during a playtest leaves its CONTINUE alone. Dev steps beside the
player's: `place X Y FACING` puts MegaMan somewhere, `flags FROM TO 1`
sets a block of event flags and `flags FROM TO 0` puts them back, and
`battle` starts the layer's next random battle once he is free on its
map (a guest battle on BN5 territory; the state then says `battle (the
older net's)`).

`tools/romlab` runs the plain ROM in libmgba for research: scripted input,
memory peeks and pokes, states and recordings. `labtrace.py` traces captured
tiles, palettes and OBJs back to ROM offsets.

| Change | Checks |
| --- | --- |
| Engine code | `build.py test`, `build.py lint`, a headless capture of the affected screen, an autopilot run |
| Layer generation or maps | `build.py test` (connectivity over 300 seeds), `build.py tiles` (no layer worse than `tests/atlas_baseline.txt`, the close-ups looked over), captures of every area (`--net-biome 0`-`7`) |
| The town | `build.py town` (its seeds drawn, misses marked, the game around it), an autopilot run from `--scene town` through the jack-in |
| Layer objects, scripts, shops | A capture of the talk or screen with scripted input |
| Difficulty, encounters, guardians, rewards | `build.py test`, `build.py pacing` (0 past their band), an autopilot run |
| Audio | `--render-song ID:SECONDS:PATH` and a listen on a device |
| Browser page or platform code | `build.py serve` and a run in a browser: ROM choice (BN6 and BN5, and a wrong file refused), a new game, a guest battle in a BN5 area, CONTINUE after a reload |
| The site | `build.py serve` at desktop and phone widths (agent-browser); `build.py screenshots` and `build.py clips` again when what they show changed |
| Windows packaging | `build.py windows`; the .exe under Wine (a Debian image with `wine`, `wine32:i386` and `xvfb`): a headless `--scene emu` capture with the ROM, a start in a window, the installer's silent install (`/S`), a start and `uninstall.exe /S` |
| Android packaging | `build.py android`; the APK in an emulator (KVM, the emulator and an image in `.build/android-sdk`; ROM copies pushed with `adb push`): a fresh install asks for the ROM folder; a folder with BN6 and BN5 takes both (logcat `-s Cyberworld SDL/APP`: "BN5 found"), one with only a wrong `.gba` refuses it with its reason, BN5 put in the folder after the first start comes in at the next, and ROMs right in Download go through **Choose the files instead** (both picked at once) and the icon's **ROMs** shortcut; a new game by touch, the D-pad walking, a turn to landscape, Back twice and the game opened again |
| macOS packaging | CI's macOS job (`lipo`, `otool -L`, `codesign --verify`, a ROM-less headless start); on a Mac, `build.py macos` and a run with the ROM: the open panel, a new game, CONTINUE |
| iOS packaging | CI's ios job (arm64, iOS 14, only iOS's frameworks, `codesign --verify`, the Simulator's app reaching its no-ROM screen: look at the artifact's screenshot); on a Mac, the Simulator with the ROMs: CHOOSE FOLDER on a folder with BN6 and BN5 (both taken, the title's "BN5 found"), on one with only a wrong `.gba` (refused with its reason), BN5 put in the folder after the first start (taken at the next), CHOOSE FILES with both, a new game, a turn to landscape, CONTINUE after the app was sent to the background; on an iPhone, AltStore's install from the release's source |
| 3DS build | `build.py 3ds`; on a New 3DS (3ds/README.md): the CIA installed with FBI (its QR code, or from the SD card) and started from the HOME Menu, the .3dsx over ftpd and a start from the Homebrew Launcher, NEW GAME to the first layer and its map on the bottom screen, `log.txt`'s frame lines (`frame_log = on`); the other targets still build, as the 3DS's changes are mostly shared code |
| PortMaster port | `build.py package`; `build/port/cyberworld` copied into a PortMaster-New checkout's `ports/` passes its `tools/build_release.py --do-check` and `tools/build_gameinfo.py`; the binary asks for glibc 2.30 at most (CI checks); a device run; a new PortMaster-New pull request needs the testing its template lists (ArkOS, AmberELEC, ROCKNIX, muOS, Knulli; 640x480 and larger) |
| Linux packaging | `build.py linux`; the AppImage and the `.deb` in a clean distribution container (first start without a ROM, the menu entry, a start with one); `--add-to-steam` with `HOME` at a made-up Steam folder |
| Release | Device run on the Nova and the Flip 2, `build.py release`, the Linux AppImage, `.deb` and archive each started fresh; tag `vX.Y.Z` on `main` |

## Device testing

Use an existing SSH control socket; do not store device credentials here.
Confirm the device is idle (`curl -s localhost:1234/runningGame`) before
launching. `tools/device_run.py HOST --control-path SOCK --shots 10,30 --
--scene emu` does all of the following. Test builds run from a temporary launcher beside the real one
(`ports/zz-cwtest.sh`, reloaded with `GET localhost:1234/reloadgames` and
started with `POST localhost:1234/launch`) that passes `--data-dir` to a
throwaway directory; remove it and reload the games afterwards. Drive the
game with `tools/uinput_keys.py` and capture with `grim`. Update the
installed port in `/storage/roms/ports/cyberworld/` only when asked, never
overwrite a player's `savedata/`, and compare save hashes before and after.

When handing off, state what changed, what ran, and what is unverified on
hardware.
