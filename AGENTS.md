# AGENTS.md

## Project and scope

Cyberworld Endless is a roguelike engine that reads its graphics, text, stats
and music from the player's own Mega Man Battle Network 6: Cybeast Gregar
(USA) ROM. It is written in C11 with SDL2 and ships as a PortMaster port for
ROCKNIX. Reference devices: Retroid Nova (1280x960) and Retroid Pocket Flip 2
(1920x1080).

Never commit or publish ROMs, save files, extracted assets, emulator save
states or disassembly files. `.gitignore` covers the usual names; check
`git status` before committing anything.

## Read before changing

| Task | Source of truth |
| --- | --- |
| Player experience, controls, install | [README.md](README.md) |
| Where ROM data lives and how it was found | [docs/ROM_DATA.md](docs/ROM_DATA.md) |
| Battle timings measured from the original | [docs/BATTLE_FLOW.md](docs/BATTLE_FLOW.md) |
| What is original, adapted or still engine-made | [docs/FIDELITY.md](docs/FIDELITY.md) |
| Shipped changes | [CHANGELOG.md](CHANGELOG.md) |

## Layout

| Path | Contents |
| --- | --- |
| `src/core/` | Entry point, platform (window, canvas, input, timing), ROM access and `RomLayout`, game data, run state and saves |
| `src/gfx/` | Sprite decoding and animation, panels, chip art, fonts, backgrounds, HUD layouts |
| `src/audio/` | MP2K sequencer and mixer playing the ROM's songs |
| `src/battle/` | Grid battle, chips, virus and navi AI, Custom screen, crosses |
| `src/net/` | Layer generation, floors learned from the original maps, the overworld scene |
| `src/scenes/` | Title, gallery, game over, messages and menus |
| `src/emu/` | The embedded mGBA core running the game's own code (`docs/EMULATION.md`) |
| `tests/test_core.c` | ROM-free unit tests |
| `tools/romlab/` | libmgba research harness (dev only, needs your ROM) |
| `tools/uinput_keys.py` | On-device input injection for testing |
| `port/` | PortMaster launcher and metadata |

## Rules

- Read data from the ROM through `RomLayout` offsets. Do not embed or
  generate files containing Capcom graphics, text, samples or sequences.
  Derived layout tables (like `panel_layout.inc`) are numbers only.
- A new ROM offset needs a note in `docs/ROM_DATA.md` saying how it was
  located and how to verify it.
- The core battle view is 240x160 and must stay whole on every screen shape.
  Extra canvas space may hold UI; never scale the field unevenly.
- Draw calls must not change game state. Input is read once per frame in
  `platform_poll`.
- Keep the run reproducible from `run.seed`: layer contents derive from
  `run.layer_seed`. Battle randomness may use the shared RNG.
- Run state is saved as a raw struct with a checksum. Changing `Run` breaks
  old saves: bump `RUN_MAGIC` in `save.c`.

## Build and verify

```sh
python3 build.py            # host + aarch64 binaries (Docker)
python3 build.py test       # ROM-free unit tests
python3 build.py shot --scene net --seed 5 --frames 600 --shot "300:.build/shots/a.bmp"
python3 build.py shot --battle "n:12:0" --frames 600 --shot "300:.build/shots/b.bmp"
python3 build.py shot --scene net --seed 9 --bot 3 --frames 20000   # soak test
python3 build.py package    # build/port/
```

`shot` mounts `~/.cache/mmbn-ref/roms` (override with `CYBERWORLD_ROM_DIR`)
read-only. `--battle` takes `v:FAMILY:VER`, `n:NAVI:VER`, `x:CROSS`
(99 = beast), `f:CHIP:CODEINDEX`, `r` (empty the folder), `k` (enemies at
1 HP) and `h:HP` (MegaMan's HP). `--shot-range A:B:PREFIX` saves every frame
from A to B; `--sheet CAT:IDX:ANIM[:PAL]:PATH` draws every frame of one
sprite animation.

Battle changes are checked against the original frame by frame: record the
same inputs in `tools/romlab` (`rec`, `watch`), capture ours with
`--shot-range`, and compare. `tools/romlab/labtrace.py` traces captured
tiles, palettes and OBJs back to ROM offsets.

| Change | Checks |
| --- | --- |
| Engine code | `build.py test`, a headless capture of the affected screen, a bot soak |
| Battle or AI | `--battle` captures of the affected enemies; flow timing against a romlab recording (docs/BATTLE_FLOW.md) |
| Generation | `build.py test` (connectivity over 300 seeds) |
| Audio | `--render-song ID:SECONDS:PATH` and a listen on a device |
| Release | Device run on the Nova and the Flip 2 |

## Device testing

Use an existing SSH control socket; do not store device credentials here.
Confirm the device is idle (`curl -s localhost:1234/runningGame`) before
launching. Deploy to `/storage/roms/ports/cyberworld/`, launch through
EmulationStation (`POST localhost:1234/launch` with the launcher path), drive
it with `tools/uinput_keys.py`, and capture with `grim`. Never overwrite a
player's `savedata/`.

When handing off, state what changed, what ran, and what is unverified on
hardware.
