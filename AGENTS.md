# AGENTS.md

## Project and scope

Cyberworld Endless is a roguelike that runs Mega Man Battle Network 6:
Cybeast Gregar (USA) from the player's own ROM on an embedded mGBA core and
directs it: generated layers in the game's map formats, the run's structure,
loot and saves. It is written in C11 with SDL2 and ships as a PortMaster port
for ROCKNIX, the primary target, as a Linux desktop build and as a browser
build on GitHub Pages. Reference
devices: Retroid Nova (1280x960) and Retroid Pocket Flip 2 (1920x1080).

Never commit or publish ROMs, save files, extracted assets, emulator save
states or disassembly files. `.gitignore` covers the usual names; check
`git status` before committing anything. Screenshots of the game running
are the one exception: `docs/screenshots/` and the short videos in
`docs/clips/`, made by `build.py screenshots` and `build.py clips`, for the
README and the site.

## Read before changing

| Task | Source of truth |
| --- | --- |
| Player experience, controls, install | [README.md](README.md) |
| How the engine drives the game, and the memory it writes | [docs/EMULATION.md](docs/EMULATION.md) |
| Where ROM data lives and how it was found | [docs/ROM_DATA.md](docs/ROM_DATA.md) |
| What is original, generated or adapted | [docs/FIDELITY.md](docs/FIDELITY.md) |
| Shipped changes | [CHANGELOG.md](CHANGELOG.md) |

## Layout

| Path | Contents |
| --- | --- |
| `src/core/` | Entry point, platform (window, canvas, input, timing), ROM access and `RomLayout`, chip and virus data, loot, run state and saves |
| `src/gfx/` | Sprite decoding and animation, ROM tiles, the font |
| `src/audio/` | MP2K sequencer and mixer (title music and sounds); the core's sound during play |
| `src/net/` | Layer generation (rooms, walkways, objects) |
| `src/scenes/` | Title (with the run summary) and the sprite gallery |
| `src/emu/` | The mGBA core, calls into the game (warps, chat), boot, event flags, debug output, the autopilot and the scene (`docs/EMULATION.md`) |
| `src/map/` | Layers as game maps: tiles learned from the original maps, walls and warp-pad triggers, the map tables taken over |
| `src/layer/` | What stands on a layer: NPC and text scripts, services, shops, choices, guardians |
| `src/director/` | The run on the game: layers, warps, encounters, bosses, checkpoints, powers |
| `tests/test_core.c` | ROM-free unit tests |
| `tools/romlab/` | libmgba research harness (dev only, needs your ROM) |
| `tools/uinput_keys.py` | On-device input injection for testing |
| `port/` | PortMaster launcher and metadata |
| `linux/` | The Linux desktop release's README and menu installer |
| `web/` | The project site on GitHub Pages, laid out like BN6's PET screens: the home page (`index.html`, `assets/`), the player in `play/` (ROM check and storage, scaling). Its frames and icons (`assets/ui/*.png`) are drawn by `tools/site_art.py`, not taken from the ROM |
| `docs/screenshots/` | Screenshots of the game for the README and the site (`build.py screenshots`) |
| `docs/clips/` | Short videos of the game for the site: WebM, MP4 and a poster each (`build.py clips`, ffmpeg in a pinned image) |
| `docker/` | Build images: `Dockerfile` (host and ROCKNIX, Debian trixie), `Dockerfile.linux` (desktop release, bookworm, SDL2 from source), `Dockerfile.web` (Emscripten, mGBA without threads) |
| `.github/` | CI (`ci.yml`: checks, every target, Pages from `main`), releases (`release.yml`, on `v*` tags), the cached image build action, Dependabot |

## Rules

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
  screen shape.
- Draw calls must not change game state. Input is read once per frame in
  `platform_poll`.
- Keep a layer reproducible from `run.layer_seed`: a checkpoint rebuilds the
  layer before it loads `run.state`, so generation and object placement
  must not depend on the game's RAM.
- Run state is saved as a raw struct with a checksum. Changing `Run` breaks
  old saves: bump `RUN_MAGIC` in `save.c`. Emulator states (`boot-3.state`,
  `run.state`) are made on the device and never committed.

## Build and verify

```sh
python3 build.py            # host + aarch64 binaries (Docker)
python3 build.py test       # ROM-free unit tests
python3 build.py shot --scene emu --run-depth 2 --frames 400 --shot "300:/src/.build/a.bmp"
CYBERWORLD_AUTOPILOT=1 python3 build.py shot --scene emu --frames 15000   # walk layers, fight
python3 build.py package    # build/port/
python3 build.py run        # the Linux desktop build, played here in a window
python3 build.py serve      # the site and the browser build on http://localhost:8080
python3 build.py screenshots [NAMES]   # docs/screenshots from scripted headless runs
python3 build.py clips [NAMES]         # docs/clips: the same runs as 30 fps videos
python3 tools/site_art.py              # web/assets/ui: the site's pixel-art frames and icons
python3 build.py release    # build/release/: the PortMaster zip, the Linux tar.gz, the site zip
```

The host and Linux builds are desktop builds (`CW_DESKTOP`): a resizable
window, saves in `~/.local/share/cyberworld-endless`, the ROM looked for
there, beside the binary and in `./rom`. The ROCKNIX build fills the screen
and takes both folders from its launcher. `build.py run` is the quickest way
to play a change; the headless `shot` stays the way to capture one.

The browser build (`__EMSCRIPTEN__`) has no threads: the page drives one
game frame per 1/60 s (`emscripten_set_main_loop`), and writes reach
IndexedDB through `platform_persist()`, which saves and states call. Keep
the frame loop free of blocking waits. `build.py serve` also serves the
developer's ROM at `/.dev/rom.gba` for tests in a local browser; the page
itself only takes a ROM the player chooses.

CI runs without a ROM: it builds every target with `-Werror`
(`WERROR=1`, set when `CI` is) and runs `tests/test_core.c` under the
sanitizers. Everything that needs the game (captures, autopilot, atlas,
pacing) stays local.

`shot` runs headless in the build image with the repository at `/src` and
`~/.cache/mmbn-ref/roms` (override with `CYBERWORLD_ROM_DIR`) mounted
read-only; `--data-dir` defaults to `.build/data`. `--scene emu` starts a new
run on the game, `--run-depth N` at depth N, `--net-biome N` in one area,
`--seed S` with a given seed. `--input "FRAMES:BUTTONS,..."` scripts the
buttons (`UP+RIGHT`, `A`), `--shot FRAME:PATH,...` and `--shot-range A:B:PREFIX`
save frames, and `--sheet CAT:IDX:ANIM[:PAL]:PATH` or `--sheet
@CAT:FIRST:COUNT:PATH` draw sprites. Environment variables reach the image
only when `build.py` lists them (`CYBERWORLD_EMU_DEBUG`, `CYBERWORLD_AUTOPILOT`
and the audio ones).

`tools/romlab` runs the plain ROM in libmgba for research: scripted input,
memory peeks and pokes, states and recordings. `labtrace.py` traces captured
tiles, palettes and OBJs back to ROM offsets.

| Change | Checks |
| --- | --- |
| Engine code | `build.py test`, a headless capture of the affected screen, an autopilot run |
| Layer generation or maps | `build.py test` (connectivity over 300 seeds), captures of every area (`--net-biome 0`-`7`) |
| Layer objects, scripts, shops | A capture of the talk or screen with scripted input |
| Difficulty, encounters, guardians, rewards | `build.py test`, `build.py pacing` (0 past their band), an autopilot run |
| Audio | `--render-song ID:SECONDS:PATH` and a listen on a device |
| Browser page or platform code | `build.py serve` and a run in a browser: ROM choice, a new game, CONTINUE after a reload |
| The site | `build.py serve` at desktop and phone widths; `build.py screenshots` and `build.py clips` again when what they show changed |
| Release | Device run on the Nova and the Flip 2, `build.py release`, the Linux archive started from a fresh unpack; tag `vX.Y.Z` on `main` |

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
