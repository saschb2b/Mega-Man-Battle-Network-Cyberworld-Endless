---
type: Code Map
title: "The platforms and their packaging"
description: "What each platform's folder holds: the PortMaster port, the Linux, Windows, macOS, iOS and Android releases, the New 3DS, Switch, PS Vita, PS4 and PS5 builds, the Docker build images and the CI and release workflows."
tags: [code]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-14T14:23:41Z }
---

# The platforms and their packaging

| Path | Contents |
| --- | --- |
| `port/` | The PortMaster port: the launcher, `port.json`, `gameinfo.xml` and the README of PortMaster-New's `ports/cyberworld/`, and `update.py`, which the launcher runs after the game to put [the game's own update](update.md) in place; `build.py package` puts them in that folder's layout in `build/port/cyberworld/`, with the screenshot (`docs/screenshots/portmaster.png`), the binary and one `licenses/LICENSE.<part>.txt` per part |
| `linux/` | The Linux release: its README, the tar.gz's menu installer, the `.desktop` entry, AppStream metadata and the icons (`tools/app_icon.py`) that the AppImage, the `.deb` and the Flatpak (`linux/flatpak/`: its manifest, how to build it and bring it to Flathub) carry. The app ID `io.github.saschb2b.Mega-Man-Battle-Network-Cyberworld-Endless` is the window's class too (`src/core/platform.h`); `src/core/desktop.c` finds the ROM, opens the file chooser for the ROMs screen (`zenity`, `kdialog`, `osascript`) and adds the AppImage to the menu. `linux/steam/`: `add-to-steam.py` (the game as a non-Steam shortcut, with the library artwork `tools/steam_art.py` draws), which every Linux build carries and `--add-to-steam` runs |
| `windows/` | The Windows release: its README, the NSIS installer script (per user, no administrator; run by [the game's own update](update.md) with `/S /RUN`, it waits for the old .exe, installs over it and starts the game), the .exe's manifest (UTF-8 paths, per-monitor DPI), resource script and icon (`tools/app_icon.py`); `src/core/desktop_win.c` is its desktop (the ROM in Windows' file dialog and Downloads) and `src/core/compat.h` what differs from POSIX |
| `android/` | The Android app (`build.py android`): a Gradle project whose `RomActivity` starts the game (the icon's ROMs shortcut asking for the ROMs screen), whose `RomLook` looks at what the ROMs screen's pickers chose (Android's folder picker, or the files in one picker; each `.gba` told by its game code, BN6's and each other game's the C side offers (`romsOffer`, from `pick_android.c`: BN5's; BN4's once `xrom_offered` says so) checked by SHA-1 and copied into the app's files; the folder, kept read and write, looked in again at each start and given the saves' copy; `res/xml/backup_rules.xml` keeps the ROMs out of Android's backups) whose `GameActivity` is SDL's activity running the game's C in a process of its own (`:game`, ended with the session), and whose `Updater` and `UpdateReceiver` update the app itself ([The game's own updates](../engine/updates.md), Android), built by the NDK from every `src/*/*.c` (`app/jni/CMakeLists.txt`) with SDL2's own source and the GBA core; its README says how releases are signed |
| `macos/` | The macOS app: `deps.sh` (SDL2 and mGBA as universal static libraries, on a Mac), the bundle's `Info.plist`, its icon (`tools/app_icon.py`) and the .dmg's README; `build.py macos` runs only on a Mac, so CI's macOS job builds and checks it |
| `ios/` | The iPhone and iPad app (`build.py ios`, on a Mac with Xcode): its README (AltStore and SideStore, the ROM through Files, building), `deps.sh` (SDL2 and mGBA as static arm64 libraries for the phone or the Simulator), the bundle's `Info.plist` and its icons (`tools/app_icon.py`). `src/core/ios.m` is UIKit's (the Files pickers for the ROM folder or files and the folder's looks at each start, taking BN6's and the extra ROMs offered, their codes, SHA-1s and copies' names read from rom.c's layouts, haptics, the safe area, the `main` that hands SDL's to UIKit), `src/core/ios_saves.m` SAVES' pickers and the saves' copy in a folder; `tools/altstore_source.py` writes the AltStore source each release carries. CI's ios job builds the IPA and starts the Simulator's app |
| `3ds/` | The Nintendo 3DS build (`build.py 3ds`, issue #9): its README (playing, building, testing on a 3DS), the icon, the CIA's title settings (`cia.rsf`, for makerom) and the HOME Menu's banner (`banner.png`, `tools/steam_art.py`; its sound a few seconds of the current trailer's music, `tools/trailer_music.py --banner`). `src/core/present_3ds.c` puts the canvas on the top screen through the GPU (citro2d), and the second screen on the bottom one (`src/second/`, drawn by `platform.c` every tenth frame); `src/emu/emu.c` runs the GBA core on the New 3DS's third core where the system gives one, its picture drawn on the main core (mGBA's threaded video); `src/core/start_3ds.c` splits the app's memory, gives back the ROM buffer mGBA's 3DS setup takes and never uses, and reads one extra ROM offered (BN5's first) where the heap has room for it, a second left unread and said on the PET ([The 3DS's memory](../engine/emulation/3ds-memory.md); `tools/heap3ds.py` models that heap on the host build, BN4's beside BN5's too) |
| `switch/` | The Nintendo Switch build (`build.py switch`): its README (playing, building, testing on a Switch) and the Homebrew Menu's icon (`icon.jpg`, `tools/app_icon.py`); the `.nro` carries it with its title and version (`nacptool`). `src/core/start_switch.c` brings up the sockets, sends the output to nxlink or `log.txt` and looks for the ROMs where Switch players keep them; `src/core/pads.c` maps libnx's pads by their labels; online play and the statistics go over devkitPro's libcurl on the console's own TLS (`src/online/online_stream_curl.c`, shared with the 3DS) |
| `vita/` | The PlayStation Vita build (`build.py vita`): its README (playing, building, testing on a Vita), the bubble (`icon0.png`, `tools/app_icon.py`) and the LiveArea (`bg.png` and `startup.png`, `tools/steam_art.py`; `template.xml`), which the VPK carries with its `param.sfo` (title ID `CWEN00001`). `pad.c` is 8 KB the Makefile links in only where `vita-elf-create` finds no room after the code (vita/README.md). `src/core/start_vita.c` sets its heap, stack, clocks and network and looks for the ROMs where Vita players keep them; `src/emu/emu.c` runs the GBA core on a core of its own there; `src/core/platform.c` names SDL's GXM renderer; online play and the statistics go over VitaSDK's libcurl with OpenSSL (`src/online/online_stream_curl.c`, shared with the 3DS and the Switch) |
| `psp/` | The PSP build (`build.py psp`): its README (playing, building, testing in PPSSPP and on a PSP), the XMB's icon (`icon0.png`, `tools/app_icon.py`) and background (`pic1.png`, `tools/steam_art.py`), which the EBOOT.PBP carries with its `PARAM.SFO` (`MEMSIZE=1`). Its GBA core is gpSP, not mGBA (`CORE=gpsp`, `src/emu/emu_gpsp.c`, `docker/gpsp.sh` with `docker/gpsp-cyberworld.patch`; GPL-2.0, so the PSP's build is distributed under the GPL's terms); `src/core/start_psp.c` sets its heap, stack and clock and reads `args.txt` and `env.txt`; `src/core/platform.c` fills its 480x272 screen smoothly from 1x, its video memory holding no sharp texture ([The PSP and the DS, measured](../engine/emulation/psp-and-ds.md)) |
| `ps4/` | The PS4 build, a PS5's too (`build.py ps4`): its README (playing, building, testing on a PS4), the home screen's icon (`icon0.png`, `tools/app_icon.py`) and the fake PKG's project (`pkg.gp4`), which `PkgTool.Core` packs with the eboot and its `param.sfo`. It builds against the OpenOrbis toolchain's SDL 2.0.9 (`src/core/sdl_old.h` stands in for what it lacks); `src/core/start_ps4.c` sets its folders and ROM places; online play goes over BSD sockets and mbedTLS (`src/online/online_stream_tls.c`), the SDK having no libcurl |
| `ps5/` | The PS5's own build (`build.py ps5`): its README (playing, building, submitting to Prospero Store, testing on a PS5) and the icon; `sce_sys/param.json` at the repository's root (title ID `CWEN00001`, its contentVersion raised with each release and held to the CHANGELOG by `build.py lint`), where Prospero Store's catalog reads it (`icon0.png`, `tools/app_icon.py`). drakmor's ps5-native-app-boilerplate (GPL-3.0, pinned) gives the CRT, linker script, ELF converter, FSELF signer and clean-room `libc.prx`; the Makefile lays out `CWEN00001/` and zips it, links only against modules a game's process gets (`PS5_LOADED`) and sends the allocations to `src/core/ps5_heap.c`'s heap on flexible memory, which `param.json`'s `kernel.flexibleMemorySize` gives. `src/core/ps5_libc.c` gives what the native runtime lacks of the C library; online play and the statistics go over PacBrew's libcurl (`src/online/online_stream_curl.c`) |
| `docker/` | Build images: `Dockerfile` (host, Debian trixie), `Dockerfile.portmaster` (the PortMaster port's aarch64 binary: Debian bullseye's glibc 2.31 and SDL2 2.0.14, older than any firmware PortMaster serves; `debian/eol` from its snapshot), `Dockerfile.linux` (desktop release, bookworm, SDL2 from source), `Dockerfile.windows` (MinGW-w64, SDL2 and mGBA static, NSIS), `Dockerfile.android` (the SDK, NDK and Gradle, SDL2's source, mGBA per ABI), `Dockerfile.web` (Emscripten, mGBA without threads), `Dockerfile.3ds` (devkitPro's devkitARM, SDL2 and mGBA's library for the 3DS, makerom and bannertool from their sources for the CIA: `ctrtools.sh`), `Dockerfile.switch` (devkitPro's devkitA64 with libnx, its SDL2 and libcurl, and mGBA's library for the Switch), `Dockerfile.vita` (VitaSDK with its SDL2 and libcurl, and mGBA's library for the Vita), `Dockerfile.psp` (pspdev's toolchain, pinned, with its SDL2; gpSP with the game's additions, `gpsp.sh` and `gpsp-cyberworld.patch`; mGBA's library for the parts the rest of the code names), `Dockerfile.ps4` (the OpenOrbis toolchain, and mbedTLS and mGBA's library built for it through `ps4.cmake`: `mbedtls.sh`), `Dockerfile.ps5` (the native-app template's pipeline, its converter's RELRO placement corrected, the PS5 payload SDK and PacBrew's SDL2 and libcurl, and mGBA's library through `ps5.cmake`), `Dockerfile.music` (the trailers' music when a piece plays a soundfont: Debian's FluidR3 GM, MIT, never in git; `tools/trailer_soundfont.py` reads it, `tools/trailer_music.py`'s render_any runs a piece there) |
| `.github/` | CI (`ci.yml`: checks, every target on a push and a pull request's share of them, [below](#ci-on-a-pull-request), Pages from `main`), the files tools write, committed after each push to `dev` (`generated.yml`, [Merging in parallel](../project/merging.md)), releases (`release.yml`, on `v*` tags; each carries the update.json `tools/update_manifest.py` writes for [the game's own updates](update.md)), the cached image build action, Dependabot (`dependabot.yml`: the actions and the build images' bases monthly, its PRs to `dev`), the issue chooser (`ISSUE_TEMPLATE/`: the bug report's form `bug.yml` and `config.yml`, [below](#issue-forms)) |

Systems the game is not built for: an old 3DS or 2DS, whose ARM11 takes
longer than a frame for mGBA alone (`3ds/README.md`; gpSP, the PSP's
core, may change that), and the Nintendo DS, which has neither the speed
nor the memory for a GBA core ([The PSP and the DS,
measured](../engine/emulation/psp-and-ds.md), which measured the PSP's
build too).

## CI on a pull request

A pull request's run took about ten minutes, and its slowest job set that:
iOS, 7.8 minutes at the median, five of them starting the app in the
Simulator, on macOS runners that also wait longest for a machine. Measured
on 11 November 2026 over recent runs: a run is 16 jobs and 41
runner-minutes; of 400 pull request runs 20 failed, 8 of them in the code
checks and 8 in one platform's job alone (iOS 4, macOS, Windows, Vita and
PS5 one each); the unit tests never failed there alone.

So a pull request always runs the scripts' lint, the code checks, the
unit tests, the Linux desktop, PortMaster and Windows builds and the 3DS's
(the 32-bit console), and each of the other nine (Switch, Vita, PSP, PS4,
PS5, Android, macOS, iOS, the browser) only where its change can reach
it. `tools/ci_changes.py`, the workflow's first job, decides:

| The pull request changes | Runs |
| --- | --- |
| A platform's own folder (`ios/`, `vita/`, `web/` ...; `macos/` for iOS too, `ps4/` for the PS5 too) | That platform's job |
| The Makefile, `build.py`, `docker/`, `.github/`, `shared/` | All nine |
| A source file that tests a system's macro (`__3DS__`, `__APPLE__`, `_WIN32`, the PSP's core's `CW_GPSP` ...) or is Objective-C, or one removed | All nine |
| Anything else | None of the nine |

Of the last 146 merged pull requests this would have run all eight for
58, some for 5 and none for 83. A push to `dev` or `main`, and a run
started by hand, run everything, and every push to `dev` is checked to
its end on its own (a later push no longer cancels it), so a target a
pull request left out is built on its merge and a break is that merge's.
To build every target on a branch first: `gh workflow run CI --ref BRANCH`.

## Issue forms

`.github/ISSUE_TEMPLATE/bug.yml` is the bug report's form, written after
issue #482: three sentences and two screenshots, with no version, ROMs,
chip or save, cost an hour to read and still could not be played again.
The form asks for what it takes to load the reporter's run. Required:
the version (the title's bottom left corner), the system, and what
happened and what was expected. Asked: the device (a PortMaster
handheld's firmware, a phone's model, the browser), BN5's and BN4's ROMs
read (BN6's always is), where in the run (a dropdown, then the layer and
area), in a battle the last chip, the Navi played and the enemies, whether
it happens again after CONTINUE, the save, and pictures or a video. The
save is the title's SAVES > EXPORT ([Portable saves](../engine/saves.md)):
its `.cwsave` holds the run's checkpoint and the profile and no ROM, and
GitHub takes it zipped where it refuses the extension. The form's first
lines say never to attach a ROM. It sets the label `bug`; triage adds the
rest.

`config.yml` keeps blank issues (the owner's own) and sends ideas and
questions to Discussions' Ideas and Q&A, and to the
[guide](../GUIDE.md)'s Troubleshooting; a second form for ideas would
double what Discussions holds.

GitHub draws a form from the default branch alone, so a change to one is
seen once it reaches `main`. Before that, check it against GitHub's
schema: the ids and labels unique, every dropdown with its options,
`validations` on inputs, textareas and dropdowns only (SchemaStore's
`github-issue-forms.json` and `github-issue-config.json`).
