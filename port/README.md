## Notes

Cyberworld Endless is a roguelike built on Mega Man Battle Network 6: the
game runs on an embedded [mGBA](https://github.com/mgba-emu/mgba) core,
and each run jacks into a newly generated net of areas, battles, shops and
guardians. Made by Sascha Becker:
[source, manual and issues](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless).
Thanks to Capcom for Mega Man Battle Network, to endrift and the mGBA
contributors for the core, and to the [bn6f disassembly](https://github.com/dism-exe/bn6f)
for its map of the game's data.

Copy your own **Mega Man Battle Network 6: Cybeast Gregar (USA)** `.gba`
into `ports/cyberworld/rom/`. The file name does not matter; the game
checks the contents. Nothing from the game is included in this port.

Saves go to `ports/cyberworld/savedata/`.

Where the firmware has libcurl, the first start asks once whether the game
may send anonymous play statistics, and sends nothing before a yes. Select
on the title opens the controls screen, whose Statistics row changes the
answer, as `statistics = off` in `ports/cyberworld/settings.ini` does
([what is sent](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless#anonymous-statistics)).
Without libcurl it never asks.

## Controls

| Button | Action |
|--|--|
| D-Pad | Move |
| A | Talk, open Mystery Data / use a chip |
| B | Run, hold to fast-forward text / fire the buster |
| L | Ask MegaMan what's ahead / open the Custom screen |
| R | Jack in / open the Custom screen |
| Start | Open the PET / pause |
| Select (hold) | Map of the layer |
| Select + Start | Quit |

## Compile

```bash
git clone https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless.git
cd Mega-Man-Battle-Network-Cyberworld-Endless
python3 build.py package
```

The build runs in Docker (`docker/Dockerfile.portmaster`, Debian bullseye)
and leaves this port's folder in `build/port/cyberworld/`.
