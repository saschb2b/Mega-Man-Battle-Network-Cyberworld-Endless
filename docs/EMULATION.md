# Running the game's own logic

Battles, the net, menus, shops and messages run on BN6's own code, executed
from the player's ROM by an embedded [mGBA](https://mgba.io) core (0.10.5,
MPL-2.0, built by `docker/mgba.sh`). Cyberworld Endless is the director
around it: it generates each layer in the game's own formats, patches the
map tables of its in-memory ROM copy to point at them, and watches the game's
memory to move the run along. Nothing from the ROM is shipped.

| Layer | Owner |
| --- | --- |
| CPU, video, sound, input | mGBA core (`src/emu/emu.c`), presented through SDL |
| Battles, net movement, menus, messages, shops, traders | The game's code |
| Run structure, layer generation, loot, saves, the title | Cyberworld Endless |

## Memory the engine writes

The core runs a copy of the ROM padded to 16 MB. Everything the engine adds
lives past the original data, from `EMU_FREE` (`0x08800000`):

| Offset | Contents | Code |
| --- | --- | --- |
| `+0x0000` | Warp record and warp list for direct warps | `gamecall.c` |
| `+0x0100` | Call stub: runs one game routine with r0-r2, sets `BN6_ENGINE_MARK` and leaves the routine's r0, r1 at `BN6_ENGINE_RET` (`0x0203FFF4`, past the game's EWRAM as the mark is) | `gamecall.c` |
| `+0x0180` | Encounter roll wrapper and trampoline | `encounter.c` |
| `+0x0200`, `+0x0280` | Two battle records in turn: BattleSettings, `+0x20` its entity list (MegaMan's panel first, the foes, the field's objects) | `encounter.c` |
| `+0x0300` | The PET's input step: A on Save taken for the engine, then the game's own handler (docs/PET.md) | `pet.c` |
| `+0x2F00` | The layer map's warp list (entry 1: the exit pad) | `mapslot.c` |
| `+0x3000`-`+0x10000` | Layer data in two halves, one per layer in turn: NPC lists and scripts, text archive, Mystery Data, sprite list | `mapslot.c` |
| `+0x10000` | Generated tile map (LZ77, literal blocks) | `netmap.c` |
| `+0x60000` | Generated coordinate data (walls, the exit pad's trigger) | `coords.c` |
| `+0x100000` | The town's tile map (LZ77, literal blocks) | `town.c` |
| `+0x170000`-`+0x1A0000` | Another game's net area (docs/MULTIROM.md): its tile set, a header of its own before its two blocks encoded again, and its palette as it is; the map its layers take over points at them | `netmap.c` |
| `+0x130000` | The town's coordinate data (walls, section 2, the jack-in cells, the checks) | `coords.c` |
| `+0x140000`-`+0x148000` | The town's NPC scripts, text, lists, warps, objects, check table and map text archive, apart from the layers' | `mapslot.c` |
| `+0x150000`-`+0x152000` | The director's conversations: one text archive, rewritten for each | `talk.c` |
| `+0x152000`-`+0x153000` | The map-name label's archive: a copy of the game's with its 244 names pointed at where the run is ("Layer 12", "ACDC Town"); its other scripts (the PET's HP, zenny and BugFrags, 0xF0-0xF2) kept | `director.c` |
| `+0x153000`-`+0x154000` | The PET's PLACE: the same copy, its names pointed at the area and the layer ("JudgeTree 14") | `director.c` |
| `+0x154000`-`+0x166000` | The PET's words (docs/PET.md): the key items' names (+0x154000) and descriptions (+0x155000), the mails' senders and subjects (+0x158000) and bodies (+0x15A000), each BN6's archive rebuilt with the run's scripts in place of some | `pet_text.c` |

A battle with a green Mystery Data on the field also rewrites the reward
row its record names (rows 0 and 1, one per record, of the table at
`0x080211A0`, docs/ROM_DATA.md) with the run's own rewards before the
battle.

In the rival's netbattle (docs/RIVAL.md), once ProtoMan stands on the
field, his battle object's HP and MaxHP (`+0x24`, `+0x26` of his T1 object,
`0x0203A9B0` + 0xD8 per object) are lowered once to the act's guardian
band, where his own 1800 is above it.

The engine also takes over Central Town (`0x01:0`) or ACDC Town (`0x00:0`)
for the town (its tile map, coordinate data, NPC list, map scripts,
objects, sprite list, warp list, jack-in table, check table
`0x0803461C`, text archive `0x08040794` and music), clears the check
flags `0x16C0`-`0x16CF`, rewrites jack-in destination 42 (`0x08099A00` +
42 x 20) to the first layer, and
widens MegaMan's facing probes for talking (`0x0809F164`), redraws the
chat font's version marks as two letters side by side (`0x086AACAC`,
widths `0x08043C74`), points the
map-name label and the PET's at the run's own names, rewrites the stock of
the game's shops 0 and 3 (in RAM and
in the initial table in ROM, `shop.c`), clears the Mystery Data flags
(`0x1400`+) and choice flags (`0x1440`+) it uses, sets the flags that stop
jacking out and the PET's Save (`0x1727`, `0x1706`) and the one that puts
the NaviCust under MegaMan in the PET (`0x00F2`; without it MegaMan's entry
goes straight to his status and a program cannot be installed. Found by
setting blocks of flags with the PET open, `tools/play.py`'s dev `flags`
step, and halving: `0x00AC` and `0x00F7` add Records there instead), and
borrows cbGameState
(`0x080050EC`) for one frame to warp.


At a new run's start the engine writes the chosen starting folder (docs/META.md)
over the game's first folder: 30 u16 at the chip data eToolkit `+0x48` points
at (docs/ROM_DATA.md, chip folders), as BN6's own GiveFolder copies one in.
The Standard folder writes nothing.
## A layer

1. `net_gen.c` generates rooms, walkways and objects from the run seed.
2. `netmap.c` builds the area's tile map from tiles learned per class from
   one original map (`area_src.c`), and its wall list, and points the map's
   descriptor and coordinate data at them.
3. `layer_objs.c` turns the objects into NPC scripts (`npc.c`), text
   (`text.c`, `scripts.c`), Mystery Data and shop stock, and `mapslot.c`
   points the map's NPC list, scripts, objects, Mystery Data and sprite list
   at them.
4. `gamecall.c` warps MegaMan in with the game's own warp routine.

The exit pad is the game's own warp pad: trigger cells that take warp 1 of
the map's warp list. When MegaMan steps on it, the game starts its jack-out;
the director builds the next layer meanwhile and points warp 1 at its start,
so the game jacks him in there. A guardian keeps the pad hidden and shut
(flag `0x16F1`) until its Guardian Data is taken (docs/BOSSES.md). A Yes to the Undernet or the Secret Area
starts the same departure (`0x080059B5`) to a side layer built at once.

`director.c` then watches the game: the exit pad's warp, choices made in text (event flags set by
Yes), battles won (viruses counted, bosses beaten), the game's GAME OVER
(the run ends), MegaMan on any other map for 90 frames (warped back to
the layer's start), and a checkpoint shortly after each arrival (`run.sav` plus
the core's `run.state`), once MegaMan is free to move. Quitting (Escape
twice, SIGTERM or SIGINT from a launcher) saves the run once more where he
stands if he is free on the layer's map with nothing of the guardian under
way; loading it restores the guardian's state from its flags. The shops'
stock is written again after the load (the ROM copy the shop screen checks
the list against is not in a state), each entry the saved list holds
keeping its stock there, and `run.make` records the build's `LAYER_MAKE`: a run saved by a
build that makes layers otherwise continues its layer from the start, with
the layer's flags and Mystery Data picks cleared (its RAM would not match
this build's objects). `battle.sav` keeps what the random battles
remember of the last two (their virus families, loot.h), so the first after
a CONTINUE brings none of them either. `run.seen` beside it keeps the map's panels seen so
far, and event flag `0x144E` in the state that L has told where they are
(`0x144F`: that the Net Dealer has said his words, so a later talk is a
line and the list; `0x1450` the same for the NaviCust vendor, `0x1451` for a
Recovery Mr. Prog, whose heal is then one box; `0x1452` that the Guardian
Data's second way on was taken, which the exit pad's warp reads; `0x1453`
that the layer's collector's vault gave its chip; `0x1454` that the
Guardian Data's dark way into the Undernet was taken; `0x1455` that the
layer's official gate gave its chip, `0x1456` that Chaud's clearance
opens it, `0x1457` that Chaud's call on a duel layer was made,
docs/RIVAL.md).
CONTINUE loads the state and enters the map again, so the game reloads it
from the current build's tables.

It also starts conversations of its own (`talk.c`): Lan and MegaMan on
arriving somewhere new (the first layer, the Undernet and the Graveyard,
the Nest, a side layer, a net rebuilt after the Nest), Chaud's call after
the Secret Area's guardian, and MegaMan's word on where they are when L is
pressed on the map (in battle L still opens the Custom screen). Each is
built into the talk slot and run by the game's chat box
(`BN6_CHAT_RUN_SCRIPT`), once the area card has gone and no guardian scene
is playing; the player's keys only page it until the box closes. In the
town the first conversation is the town's own (its text archive, Dad's
call on the very first run).

## A new run: the town

NEW GAME builds the run's first layer and the town (docs/OVERWORLD.md),
warps Lan to where the town starts him (his front door in Central Town,
the Metroline's stairs in ACDC Town) and clears the flag that stops
jacking in (`0x1727`); the PET's Save stays off. While Lan walks the town
the director waits: the town's jack-in cells and jack-in table send the
game's own jack-in to the first layer, and when MegaMan has arrived there
the run goes on as after any layer's warp (the flags set again, a
checkpoint). A NEW GAME deletes the last run's save at once, so CONTINUE
always resumes a run in the net.

## Boot

`emu_boot` resets the core, presses through the title and NEW GAME, warps to
Seaside Area 1 and ends the intro cutscene, then saves `boot-3.state` in the
data directory. Later runs start from that state. It is made on the device
and never shipped.

The boot's frames are never shown, so the core draws no picture for them;
nor for a frame the main loop plays unshown to catch up with the GBA's pace
(`emu_frame` sets the core's frame-skip counter for that one frame, where
its struct reads back as expected).

## The core on its own thread

On the New 3DS the GBA core takes most of a frame, so it runs on the third
core where the system gives it one (`emu_threaded`). `emu_frame` then starts
the frame and returns, and every other access to the core (`emu_read*`,
`emu_write*`, states, the sound's rate) waits for that frame first: the
director's logic takes in each frame whole, as on the main thread. The emu
scene's update takes the frame done in and then starts the next, which runs
while the frame done is drawn from a copy of its picture; keys, frames and
the logic keep their order, and the autopilot's run log comes out the same.
`CYBERWORLD_EMU_THREAD=1` runs it so on a computer, to test it. With the
core on its own thread, mGBA's threaded video draws its picture on one
more (`threadedVideo`), beside the emulation; the worker waits for it at
each frame's end, so the picture copied is whole. On the 3DS that thread
is on the main core, which waits most of a frame for the emulation.

## Testing

`CYBERWORLD_AUTOPILOT=1` walks MegaMan to each exit (into a guardian's arena
and to its Guardian Data first) and presses through battles; `CYBERWORLD_AUTOPILOT=weak` also keeps
enemies at 1 HP, so what follows a won guardian battle can be tested.
`tools/device_run.py` runs a build on the device from `/tmp`. `CYBERWORLD_EMU_DEBUG=1` prints the depth, game mode, position and
map every 30 frames, prints the generated walls, and writes the tile map to
`.build/gen_tilemap.bin`.
`--net-biome N` puts every layer in one area, `--net-layout N` builds every
layer in one layout (`LAYOUT_*` in `src/net/net_layouts.h`).
