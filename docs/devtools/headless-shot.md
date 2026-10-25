---
type: Tool
title: "build.py shot: a headless capture's options"
description: "`build.py shot`'s options for a headless capture in the build image: the scenes, run depth, area, guardian, seed, setup and marks, `--talk`, scripted `--input` and `--taps`, and the frames saved."
tags: [devtools]
generated: { by: claude-code/unrecorded, at: 2026-10-25T17:18:46Z }
---

# build.py shot: a headless capture's options

`shot` runs headless in the build image with the repository at `/src` and
`~/.cache/mmbn-ref/roms` (override with `CYBERWORLD_ROM_DIR`) mounted
read-only; `--data-dir` defaults to `.build/data`. `--scene emu` starts a new
run on the game, `--run-depth N` at depth N, `--net-biome N` in one area (`xN` another game's area N, its ROM beside BN6's: [Other games' ROMs](../multirom/overview.md)),
`--guardian N` with navi N guarding every area,
`--seed S` with a given seed, `--scene town` from the town as NEW GAME does,
`--scene launcher` the ROMs screen (the ROMs in `--rom-dir`; `--launcher auto`, `open` or `off` sets a start's own),
`--scene saves` the SAVES screen,
`--scene summary` the title's run summary (a win's with `--setup short` at
`--run-depth 10`), `--setup NET,FOLDER,THREAT,HELPERS[,CROSS]` the run's setup (CROSS the navi whose Cross it brings, 1-5),
`--marks HEX` the title's marks, `--screen auto|whole|fill|stretch` the
scale over `settings.ini`'s (Options' Display), `--touch` the touch controls from the
start (as on a phone; with `--size` a phone's screen and `--dpi N` its density), `--talk NAME:FRAME,...` opens a
layer's chats at its frames (npc, shop, heal, programs, gift, challenge,
undernet, gate, navigate for a Navi gate, vault, duel for ProtoMan's terms, official for an official gate, trader and bugtrader for a Chip or BugFrag Trader, battler for a Virus Battler (`--dev battler=K` puts one on the layer); intro, defeat, reward for the guardian; status for L; rumor for the layer's whisper (rumor_lines.c); dark for a flame of darkness ([The meta layer](../game/meta/overview.md));
fragment for MegaMan's words at a ScrtData; bugfrags gives 50 BugFrags, keys an Unlocker, rushfood three RushFood, wwwid a WWW-ID, zenny 10000 zenny, regup a RegUP3). `--input "FRAMES:BUTTONS,..."` scripts the
buttons (`UP+RIGHT`, `A`; play.py's dev steps too, `0:place X Y FACE`, `0:flags FROM TO 1`, `0:sig` (MegaMan in the layer's signature, [Identity](../world/level-design/identity.md)) and `0:battle`, the layer's next random battle as soon as MegaMan is free on its map, BN5's on its territory; `300:battle` waits its 300 frames first; `0:foes 200@52/01@43` a battle in BN6's engine of the enemy ids named at their panels, column then row, `0:xrecord 811DE10` one of BN5's battle records in BN6's engine (`build.py xrecords` lists them), and `0:status freeze 90` its enemies given a status as a hit leaves it, [Dev tools](overview.md)), `--taps "FRAME:X,Y[>X2,Y2];..."` fingers at screen pixels (a tap, or a drag over 20 frames: the touch controls, their menu and editor), `--shot FRAME:PATH,...` and `--shot-range A:B:PREFIX`
save frames (the canvas; `--screen-shot FRAME:PATH,...` the whole screen, the touch controls on it; `--second-shot FRAME:PATH,...` the second screen, the 3DS's bottom one with its panel, at `--second-size WxH` an Android display's, and `--second-shot-range A:B:PREFIX` its frames as `--shot-range` the picture's), and `--sheet CAT:IDX:ANIM[:PAL]:PATH` or `--sheet
@CAT:FIRST:COUNT:PATH` draw sprites. Environment variables reach the image
only when `build.py` lists them (`CYBERWORLD_EMU_DEBUG`, `CYBERWORLD_AUTOPILOT`
and the audio ones).
