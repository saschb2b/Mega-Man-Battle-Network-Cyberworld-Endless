# AGENTS.md

## Project and scope

Cyberworld Endless is a roguelike that runs Mega Man Battle Network 6:
Cybeast Gregar (USA) from the player's own ROM on an embedded mGBA core and
directs it: generated layers in the game's map formats, the run's structure,
loot and saves. It is written in C11 with SDL2 and ships as a PortMaster port
for ROCKNIX. Reference devices: Retroid Nova (1280x960) and Retroid Pocket Flip 2
(1920x1080).

Never commit or publish ROMs, save files, extracted assets, emulator save
states or disassembly files. `.gitignore` covers the usual names; check
`git status` before committing anything.

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
| `src/emu/` | The mGBA core, boot and warps, generated maps, layer objects, scripts, shops and the director (`docs/EMULATION.md`) |
| `tests/test_core.c` | ROM-free unit tests |
| `tools/romlab/` | libmgba research harness (dev only, needs your ROM) |
| `tools/uinput_keys.py` | On-device input injection for testing |
| `port/` | PortMaster launcher and metadata |

## Rules

- Read data from the ROM through `RomLayout` offsets or the addresses in
  `src/emu/bn6.h`. Do not embed or generate files containing Capcom
  graphics, text, samples or sequences; tables the engine derives are
  numbers only.
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
```

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
| Audio | `--render-song ID:SECONDS:PATH` and a listen on a device |
| Release | Device run on the Nova and the Flip 2 |

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
