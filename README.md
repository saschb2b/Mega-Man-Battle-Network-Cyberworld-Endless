# Mega Man Battle Network: Cyberworld Endless

A roguelike built on Mega Man Battle Network 6. Each run jacks MegaMan into a
freshly generated net. Every third area ends at a Navi guarding the exit, and
the net changes character as you go deeper: from the surface areas down
through the Graveyard and the Undernet to the Cybeast's nest, and then on
without end.

It runs as a port on ROCKNIX handhelds through PortMaster, and was made for
the Retroid Nova (4:3) and the Retroid Pocket Flip 2 (16:9).

## Bring your own ROM

The download contains no Capcom data. Sprites, chip art, the font, battle
panels, enemy stats and music are all read from your own copy of the game
when it starts.

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

Saves live in `ports/cyberworld/savedata/`.

## Controls

| Button | On the net | In battle |
| --- | --- | --- |
| D-pad | Walk. Up moves up-right, as in the original games | Move one panel |
| A | Talk, open data, confirm | Use the next chip |
| B | Hold to run, back | Buster; hold to charge |
| L / R | | Open the Custom screen when the gauge is full |
| Start | Menu | Custom screen: jump to OK |
| Select | Map | Custom screen: change style |

In the Custom screen, pick chips that share a code (`*` matches any code) or
the same chip several times. Hold R to read a chip's description.

## A run

- **Layers.** Each area is a new layout of rooms and walkways. Warp pads lead
  one layer deeper. Every third layer, a Navi blocks the exit.
- **Areas.** The first four acts visit Central, Seaside, Sky and Green Area in
  a random order, then the Graveyard and the Undernet. Layer 19 is the
  Cybeast Nest. After that the cycle starts again, harder each time.
- **Battles.** Real-time 6x3 grid battles with the game's viruses and Navis.
  Viruses gain versions (V2, V3, SP and rare variants) as you go deeper.
  Faster and cleaner wins earn a higher Busting Level and better rewards.
- **Rewards.** After each battle, pick one of three chips or take the zenny.
  The folder holds 30 chips; a full folder asks which chip to replace.
- **Mystery Data.** Green data holds chips and zenny, blue data HP Memory,
  Unlockers and programs. Purple data stays locked without an Unlocker.
- **Red panels.** Virus-infested rooms: more battles, better data.

### Places to find

| Place | What it does |
| --- | --- |
| Net Dealer (Mr. Prog) | Chips, HP Memory and buster upgrades |
| NaviCust vendor | Programs that last the whole run, such as SuperArmor, UnderShirt, AirShoes and Collect |
| Chip Trader | Three chips in, one better chip out |
| BugFrag Trader | Spend BugFrags on rare chips, Unlockers or HP Memory |
| Recovery program | Restores HP once |
| Strong virus signal | An optional hard battle with a better reward |
| Dark warp | Enters the Undernet: tougher viruses, better loot, and an exit one layer deeper |
| Sealed gate | In the Undernet. Three Secret Data fragments open the Secret Area |

### Powers

- **Program Advances.** GigaCan (three Cannons with consecutive codes),
  LifeSrd (Sword, WideSwrd and LongSwrd with one code), WideBrn, H-Burst and
  PwrWave.
- **Crosses.** Deleting HeatMan, ElecMan, SlashMan, EraseMan or ChargeMan
  gives MegaMan that Cross for the rest of the run. Each changes the charged
  shot. A hit from the element it is weak to breaks it.
- **Beast Out.** The Graveyard's guardian awakens Gregar's power: three turns
  of stronger chips and a lock-on charged attack.

## Screens

The battle keeps the Game Boy Advance's 240x160 view at a whole-number scale:
5x on the Nova's 1280x960 screen and 6x on the Flip 2's 1920x1080 screen. The
net and the battle background fill the rest of the screen; on 16:9 the sides
show your hand and depth during battle.

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
the game's data; none of its files are included here.
