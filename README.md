# Mega Man Battle Network: Cyberworld Endless

A roguelike built on Mega Man Battle Network 6. Each run jacks MegaMan into a
freshly generated net, played on the game's own engine. Every third layer
ends at a Navi guarding the exit, and the net changes character as you go
deeper: from the surface areas down through the Graveyard and the Undernet
to the Underground, and then on without end.

It runs as a port on ROCKNIX handhelds through PortMaster, and was made for
the Retroid Nova (4:3) and the Retroid Pocket Flip 2 (16:9).

## Bring your own ROM

The download contains no Capcom data. The game itself runs from your own copy
of the ROM on an embedded Game Boy Advance core: its battles, its net, its
menus, shops and music. Cyberworld Endless builds each layer in the game's
own map formats, places its Mystery Data, shopkeepers and exits, and keeps
the run going.

You need **Mega Man Battle Network 6: Cybeast Gregar (USA)** as an
unmodified `.gba` file (SHA-1 `89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6`).
Copy it into `ports/cyberworld/rom/`. Any file name works; the game checks
the contents. Other versions, including the Legacy Collection, are not
supported yet.

## Install

1. Install PortMaster on the handheld.
2. Copy `cyberworld/` and `Cyberworld Endless.sh` into your `ports` folder
   (on ROCKNIX: `/storage/roms/ports/`).
3. Put your ROM in `ports/cyberworld/rom/`.
4. Refresh the game list and start **Cyberworld Endless**.

Saves live in `ports/cyberworld/savedata/`. The first start also records the
game's boot as `boot-3.state` beside them, which takes a few seconds.

## Controls

The handheld's buttons are the Game Boy Advance's: the game plays exactly as
BN6 does. A talks, opens Mystery Data and uses chips; B runs and fires the
buster; L and R open the Custom screen; Start opens the PET.

## A run

- **Layers.** Each layer is a new layout of rooms and walkways built from one
  of the game's areas. The exit pad leads one layer deeper. Every third
  layer, a Navi guards the exit in the game's own navi battle.
- **Areas.** The first four acts visit Central, Seaside, Sky and Green Area in
  a random order, then the Graveyard and the Undernet. Layer 19 is the
  Underground. After that the cycle starts again, harder each time.
- **Battles.** The game's own battles: random encounters on the net pick
  viruses and versions (V2, V3, SP) that grow with depth, and rewards follow
  the Busting Level as in BN6.
- **Mystery Data.** Green data holds chips, zenny and BugFrags. In deep
  layers a blue one may hold ScrtData.
- **Checkpoints.** The run is saved when you arrive on a layer; CONTINUE on
  the title screen returns there. When MegaMan is deleted the run ends, and
  the title shows how deep you went.

### Places to find

| Place | What it does |
| --- | --- |
| Net Dealer (Mr. Prog) | The game's shop: chips, an HP Memory and SubChips |
| Program vendor (Mr. Prog) | NaviCust programs from the game's own shops |
| Chip Trader | Three chips in, one out |
| BugFrag Trader | The game's BugFrag trades |
| Recovery Mr. Prog | Restores HP |
| Server | A strong virus signal: an optional hard battle |
| Dark flame | Enters the Undernet: tougher viruses, and an exit one layer deeper |
| Golden gate | Three ScrtData open the Secret Area in Undernet Zero |

## Screens

The game's 240x160 picture is shown at a whole-number scale: 5x on the
Nova's 1280x960 screen and 6x on the Flip 2's 1920x1080 screen, with black
borders around it.

## Building

Builds run in Docker (Debian trixie, matching ROCKNIX's glibc and SDL2):

```bash
python3 build.py
```

```bash
python3 build.py test
```

```bash
python3 build.py package
```

`build.py shot` runs the game headlessly for scripted screenshots and soak
tests. See [AGENTS.md](AGENTS.md) for the development workflow and
[docs/ROM_DATA.md](docs/ROM_DATA.md) for how the ROM data is located.

## Credits

Mega Man Battle Network is © Capcom. This project is not affiliated with or
endorsed by Capcom. The code is MIT-licensed. The
[bn6f disassembly](https://github.com/dism-exe/bn6f) was an invaluable map of
the game's data; none of its files are included here. The game's own code
runs on an embedded [mGBA](https://github.com/mgba-emu/mgba) core (0.10.5,
MPL-2.0; its license ships in `licenses/`).
