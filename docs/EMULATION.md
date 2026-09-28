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
| `+0x0100` | Call stub: runs one game routine with r0, r1 and sets `BN6_ENGINE_MARK` | `gamecall.c` |
| `+0x0180` | Encounter roll wrapper and trampoline | `encounter.c` |
| `+0x0200`, `+0x0240` | Two battle records in turn: BattleSettings, `+0x20` its entity list (MegaMan's panel first) | `encounter.c` |
| `+0x2F00` | The layer map's warp list (entry 1: the exit pad) | `mapslot.c` |
| `+0x3000`-`+0x10000` | Layer data in two halves, one per layer in turn: NPC lists and scripts, text archive, Mystery Data, sprite list | `mapslot.c` |
| `+0x10000` | Generated tile map (LZ77, literal blocks) | `netmap.c` |
| `+0x60000` | Generated coordinate data (walls, the exit pad's trigger) | `coords.c` |
| `+0x100000` | The town's tile map (LZ77, literal blocks) | `town.c` |
| `+0x130000` | The town's coordinate data (walls, section 2, the jack-in cells, the checks) | `coords.c` |
| `+0x140000`-`+0x148000` | The town's NPC scripts, text, lists, warps, objects, check table and map text archive, apart from the layers' | `mapslot.c` |
| `+0x150000`-`+0x152000` | The director's conversations: one text archive, rewritten for each | `talk.c` |
| `+0x152000`-`+0x153000` | The map-name label's archive: a copy of the game's with its 244 names pointed at where the run is ("Layer 12", "ACDC Town"); its other scripts (the PET's HP, zenny and BugFrags, 0xF0-0xF2) kept | `director.c` |

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
stock is written again after the load (the state's RAM holds the saved
one), and `run.make` records the build's `LAYER_MAKE`: a run saved by a
build that makes layers otherwise continues its layer from the start, with
the layer's flags and Mystery Data picks cleared (its RAM would not match
this build's objects). `battle.sav` keeps what the random battles
remember of the last two (their virus families, loot.h), so the first after
a CONTINUE brings none of them either. `run.seen` beside it keeps the map's panels seen so
far, and event flag `0x144E` in the state that L has told where they are
(`0x144F`: that the Net Dealer has said his words, so a later talk is a
line and the list; `0x1450` the same for the NaviCust vendor, `0x1451` for a
Recovery Mr. Prog, whose heal is then one box).
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

## Testing

`CYBERWORLD_AUTOPILOT=1` walks MegaMan to each exit (into a guardian's arena
and to its Guardian Data first) and presses through battles; `CYBERWORLD_AUTOPILOT=weak` also keeps
enemies at 1 HP, so what follows a won guardian battle can be tested.
`tools/device_run.py` runs a build on the device from `/tmp`. `CYBERWORLD_EMU_DEBUG=1` prints the depth, game mode, position and
map every 30 frames, prints the generated walls, and writes the tile map to
`.build/gen_tilemap.bin`.
`--net-biome N` puts every layer in one area, `--net-layout N` builds every
layer in one layout (`LAYOUT_*` in `src/net/net_layouts.h`).
