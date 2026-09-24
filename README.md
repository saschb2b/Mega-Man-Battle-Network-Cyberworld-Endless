# Mega Man Battle Network: Cyberworld Endless

A roguelike for Mega Man Battle Network 6. Every run jacks MegaMan into a
freshly generated net and sends him down, layer by layer, for as long as he
lasts. The net changes as you go deeper: from the surface areas and the
story's comps down through the Undernet and the Graveyard to the
Underground, and then around again, harder each time. Every third layer
ends at a Navi guarding the exit.

Everything you see and hear is BN6 itself, running from your own ROM: its
battles, chips, PET, shops and music. Cyberworld Endless builds the layers,
places the Mystery Data and shopkeepers, and keeps the run going.

It runs on ROCKNIX handhelds through PortMaster and was made for the Retroid
Nova (4:3) and the Retroid Pocket Flip 2 (16:9). A Linux desktop build plays
the same game in a window on a PC.

## What you need

- A handheld running ROCKNIX with PortMaster installed, or an x86-64 Linux
  PC (glibc 2.34 or newer: Ubuntu 22.04, Debian 12, Fedora 35 and later).
- **Mega Man Battle Network 6: Cybeast Gregar (USA)** as an unmodified `.gba`
  file, dumped from your own cartridge. Its SHA-1 is
  `89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6`. Cybeast Falzar, other regions
  and the Legacy Collection version do not work yet.

The port contains no Capcom data. Without the ROM there is no game.

## Install

1. Get the port folder. There is no release download yet; build it with
   `python3 build.py package` (see [Building](#building)), which puts it in
   `build/port/`.
2. Copy `cyberworld/` and `Cyberworld Endless.sh` into the handheld's
   `ports` folder (on ROCKNIX: `/storage/roms/ports/`).
3. Copy your ROM into `ports/cyberworld/rom/`. The file name does not matter;
   the game checks the contents.
4. Refresh the game list and start **Cyberworld Endless**.

The first start records the game's boot once, which takes a few seconds.
After that the title screen appears straight away.

### On a Linux PC

1. Get `cyberworld-endless-linux-x86_64.tar.gz` (built with
   `python3 build.py linux` into `build/release/`) and unpack it anywhere.
2. Copy your ROM into `~/.local/share/cyberworld-endless/rom/` or the
   `rom/` folder next to `cyberworld-endless`.
3. Run `./cyberworld-endless`. `./install.sh` adds it to the application
   menu.

It opens in a window at the largest whole scale that fits; F11 or Alt+Enter
switches to fullscreen. Saves live in `~/.local/share/cyberworld-endless/`.
The archive's own `README.md` has the keyboard keys.

## Playing

On the title screen, press Start and choose **NEW GAME** or **CONTINUE**.
CONTINUE shows how deep your saved run is; the corner shows your best depth.

In the net, the buttons are the Game Boy Advance's and BN6 plays as it always
has. On a PC keyboard the arrows move, X is A, Z is B, A and S are L and R,
Enter is Start.

| Button | In the net | In battle |
| --- | --- | --- |
| A | Talk, open Mystery Data | Use a chip |
| B | Run | Fire the buster |
| L / R | | Open the Custom screen |
| Start | Open the PET | Pause |

### A run

Each layer is a new layout of platforms and walkways in the style of one of
the game's areas. Find the exit pad to go one layer deeper. On the way, the
game's own random battles come up, with the viruses of that area. How hard
a battle is depends on how deep you are, not on the area: the viruses grow
stronger (V2, V3, SP) act by act, and a battle never holds more than MegaMan
can be expected to handle at that point. The first battles of a run, and
the first after each guardian, are gentler. Rewards follow the Busting
Level as in BN6.

Three layers make an act. The second layer of every act always has the Net
Dealer and a Recovery Mr. Prog. The third ends in a guardian's arena, and
the room before it again has a heal and the Net Dealer. The card at the
start of each act names the guardian waiting at its end, so you can set
your folder for it. Step into the arena and the Navi logs in for the game's
own boss battle. Guardians remember how your earlier battles went.

The first four acts visit four of Central, Seaside, Sky and Green Area, the
Robot Control, Aquarium, Judge Tree, Mr. Weather and CopyBot comps, two home
computers and the Aquarium, ACDC, Green and Sky homepages. The order is
random, but the gentler areas come first (Central, Robot Control, the
Aquarium, Sky HP, a home computer) and the hardest last (Sky, Mr. Weather,
ACDC HP, CopyBot's comp). Then come the Undernet and the Graveyard, and
layer 19 is the Underground. After that the cycle starts again, harder.

### Getting stronger

- **A gift to start.** On the first layer a Mr. Prog lets you pick one: two
  HPMemory, a ★3 chip or a NaviCust program. If your last run ended before
  its first guardian, he adds an HPMemory.
- **Guardian Data.** Every guardian leaves five HPMemory (+100 max HP), its
  own Navi chip at the version you beat, and its Cross where it has one.
  Taking it also restores MegaMan's HP.
- **Crosses.** Deleting HeatMan, ElecMan, SlashMan, EraseMan or ChargeMan
  gives MegaMan their Cross for the rest of the run, chosen in the Custom
  screen as in BN6.
- **Beast Out.** The Graveyard's guardian wakes the Cybeast, and Beast Out
  joins the Custom screen.
- **Chips.** Mystery Data, shops and traders draw from the whole chip
  library by rarity: Megas deeper down and, rarely, a Giga. Green Mystery
  Data holds chips, zenny and BugFrags; in deep layers a blue one may hold
  ScrtData.

### Places to find

| Place | What it does |
| --- | --- |
| Net Dealer (Mr. Prog) | The game's shop: chips, an HP Memory and SubChips |
| Program vendor (Mr. Prog) | NaviCust programs from the game's own shops |
| Chip Trader | Three chips in, one out |
| BugFrag Trader | The game's BugFrag trades |
| Recovery Mr. Prog | Restores HP |
| Server | A strong virus signal: an optional harder battle that pays a better chip. From the fourth act it may hold an SP Navi. Never on the first layer |
| Dark flame | Enters the Undernet: tougher viruses, and an exit one layer deeper |
| Golden gate | Three ScrtData open the Secret Area in Undernet Zero |

### Saving and losing

The run is saved each time you arrive on a layer, and CONTINUE brings you
back there. The PET's Save is switched off during a run. When MegaMan is
deleted the run is over: the title screen shows how deep you got, how many
viruses and Navis you deleted, and your best depth.

Saves live in `ports/cyberworld/savedata/`. To give up a run without playing
it out, delete `savedata/run.sav`; your best depth is kept in `profile.sav`.

## Screen

The game's 240x160 picture is scaled by a whole number so it stays sharp: 5x
on the Nova's 1280x960 screen and 6x on the Flip 2's 1920x1080 screen, with
black borders around it. On a PC the window keeps the same rule as it is
resized.

## Troubleshooting

- **"Put your Mega Man Battle Network 6: Cybeast Gregar (USA) ROM in ..."**:
  the game found no ROM. Check that the `.gba` file is in
  `ports/cyberworld/rom/`.
- **"... is not a supported ROM"** or **"... is not an 8 MB GBA ROM"**: the
  file is a different game, version or region, or it is patched or
  compressed. Only the unmodified US Cybeast Gregar works.
- **Anything else**: the last start's output is in `ports/cyberworld/log.txt`.
  Please attach it when you report a problem.

## Building

For developers. Builds run in Docker (Debian trixie, matching ROCKNIX's glibc
and SDL2):

```bash
python3 build.py
```

```bash
python3 build.py test
```

```bash
python3 build.py package
```

```bash
python3 build.py run
```

`run` builds the Linux desktop binary and plays it on this machine in a
window, with the ROM from `~/.cache/mmbn-ref/roms` (or `CYBERWORLD_ROM_DIR`)
and saves in `.build/desktop`. `python3 build.py release` writes both
release archives to `build/release/`: `cyberworld.zip` for PortMaster and
`cyberworld-endless-linux-x86_64.tar.gz`. The Linux build compiles in its
own image on Debian bookworm, with SDL2 built so that it loads X11, Wayland
and the sound servers at run time.

`build.py shot` runs the game headlessly for scripted screenshots and soak
tests, `build.py atlas` draws every area's layers and `build.py tour` has the
game show every room of them. In the game, Select+R opens a dev menu (no
random battles, can't die, one-hit enemies, speed, skip to the next layer or
guardian). See [docs/DEVTOOLS.md](docs/DEVTOOLS.md) for the tools,
[AGENTS.md](AGENTS.md) for the development workflow,
[docs/LEVEL_DESIGN.md](docs/LEVEL_DESIGN.md) for how layers are laid out and
[docs/ROM_DATA.md](docs/ROM_DATA.md) for where the ROM data comes from.

## Credits

Mega Man Battle Network is © Capcom. This project is not affiliated with or
endorsed by Capcom. The code is MIT-licensed. The
[bn6f disassembly](https://github.com/dism-exe/bn6f) was an invaluable map of
the game's data; none of its files are included here. The game's own code
runs on an embedded [mGBA](https://github.com/mgba-emu/mgba) core (0.10.5,
MPL-2.0; its license ships in `licenses/`).
