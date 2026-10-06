# Running the game's own logic

Battles, the net, menus, shops and messages run on BN6's own code, executed
from the player's ROM by an embedded [mGBA](https://mgba.io) core (0.10.5,
MPL-2.0, built by `docker/mgba.sh`). Cyberworld Endless is the director
around it: it generates each layer in the game's own formats, patches the
map tables of its in-memory ROM copy to point at them, and follows the game
to move the run along: hooks on its code tell it what happens as it happens
(Hooks, below), and it reads the game's memory between frames for the rest.
Nothing from the ROM is shipped.

| Layer | Owner |
| --- | --- |
| CPU, video, sound, input | mGBA core (`src/emu/emu.c`), presented through SDL |
| Battles, net movement, menus, messages, shops, traders | The game's code |
| Run structure, layer generation, loot, saves, the title | Cyberworld Endless |

## Why an emulator

Ship of Harkinian runs Ocarina of Time with no emulator, and Zelda64Recomp
runs Majora's Mask so too. Both stand on work BN6 does not have:

- [Ship of Harkinian](https://github.com/HarbourMasters/Shipwright) compiles
  [zeldaret/oot](https://github.com/zeldaret/oot), a matching decompilation:
  C written by hand that builds back into the original ROM byte for byte.
  [libultraship](https://github.com/Kenix3/libultraship) stands in for the
  N64's hardware, its Fast3D renderer turning the game's display lists into
  OpenGL, Direct3D or Metal; the assets come from the player's ROM at first
  start. [2Ship2Harkinian](https://github.com/2ship2harkinian/2ship2harkinian)
  does the same for Majora's Mask.
- [Zelda64Recomp](https://github.com/Zelda64Recomp/Zelda64Recomp) translates
  the game's MIPS code to C, function by function, with
  [N64Recomp](https://github.com/N64Recomp/N64Recomp), which finds the
  functions by the decompilation's symbols; [RT64](https://github.com/rt64/rt64)
  renders. The translated code ships in the executable.

Neither runs a CPU emulator, and both replace the GPU at the display-list
level, which the N64 allows: its games hand the GPU lists of commands. BN6
keeps mGBA because:

1. **No C decompilation of BN6 exists.** [bn6f](https://github.com/dism-exe/bn6f)
   is a matching disassembly of Falzar, almost all of it assembly.
2. **The GBA has no display list.** BN6 writes tiles, maps, OAM and video
   registers itself, some of it timed to the scanline, so a native build
   would still emulate the GBA's video and sound. Only the ARM interpreter
   would go, the core's cheapest part everywhere but the 3DS.
3. **A decompiled or recompiled BN6 ships Capcom's code.** The project ships
   none of it, which is what keeps it on PortMaster, Flathub, the 3DS and
   GitHub Pages.
4. **Other games' ROMs lend their content as data** (docs/MULTIROM.md); a
   native build would need each game's code ported too.
5. **Save states are free checkpoints** (`run.state`) on every target.

What the native ports do have is worth taking: hooks, where the game's code
calls the port's at named points. The engine's hooks (below) are that, on the
emulator.

The question is worth asking again when a C decompilation of BN6 covers most
of its battle and overworld code **and** a run needs changes deep in battle
logic (new chip behaviour, enemy AI) that hooks make awkward; or when the New
3DS's measurements (issue #35) show that the ARM interpreter, not the
renderer, holds its frame rate back. Even then a decompilation helps first as
a map for hooks, as Majora's Mask's does for Zelda64Recomp.

## Memory the engine writes

The core runs a copy of the ROM padded to 16 MB. Everything the engine adds
lives past the original data, from `EMU_FREE` (`0x08800000`):

| Offset | Contents | Code |
| --- | --- | --- |
| `+0x0000` | Warp record and warp list for direct warps | `gamecall.c` |
| `+0x0100` | Where a game call's routine returns: a hook, which takes its r0 and r1 and goes back to the main loop (until issue #33 a stub of hand-written Thumb, with `0x0203FFF0`-`0x0203FFFB` past the game's EWRAM for its mark and results: free) | `gamecall.c` |
| `+0x0180` | An empty Mystery Data list (12 zero bytes): every map a layer took and left points at it, as BN6 rolls every map's Mystery Data at each jack-in (docs/ROM_DATA.md, Map takeover); the rest to `+0x0200` free (the encounter roll's wrapper and trampoline until hooks took their place, issue #29) | `mapslot.c` |
| `+0x0200`, `+0x0280` | Two battle records in turn: BattleSettings, `+0x20` its entity list (MegaMan's panel first, the foes, the field's objects) | `encounter.c` |
| `+0x0300` | Free (the PET's input step until a hook on the game's own handler took its place, issue #33) | |
| `+0x0700`-`+0x12E0` | Map-object handler 3's records (issue #42): BN6's 86, then the engine's Link Navi obstacles and cubes, 13 variants of 8 slots each (the last four another game's looks: BN5's Security Cube, its wall of dark flames two ways and its dark hole, a prop that never opens, docs/MULTIROM.md); its literal points here, written once a core | `blockers.c` |
| `+0x0400`-`+0x0640` | Rush's gaps (issue #14): the gap table from group 0x80 that the check's literals point at, a group's empty map list, the layer's group's maps, the layer's gap records, and handler 0x25's records (BN6's 13, then the layer's two) | `rush.c` |
| `+0x2F00` | The layer map's warp list (entry 1: the exit pad) | `mapslot.c` |
| `+0x3000`-`+0x10000` | Layer data in two halves, one per layer in turn: NPC lists and scripts, text archive, Mystery Data, sprite list | `mapslot.c` |
| `+0x10000` | Generated tile map (LZ77, literal blocks) | `netmap.c` |
| `+0x60000` | Generated coordinate data (walls, the exit pad's trigger) | `coords.c` |
| `+0x100000` | The town's tile map (LZ77, literal blocks) | `town.c` |
| `+0x170000`-`+0x1A0000` | Another game's net area (docs/MULTIROM.md): its tile set, a header of its own before its two blocks encoded again, and its palette as it is; the map its layers take over points at them | `netmap.c` |
| `+0x1A0000`-`+0x260000` | Other games' songs (docs/MULTIROM.md): each song's sequence with its pointers moved, the voices it selects, their key splits, drum kits, samples and waves (a piece two songs share once); BN6's empty song slots from 0x26 point at them. Every song the other games' areas play is copied the first time one is needed, in the areas' order, then their games' battle themes, so each has the same place in every session (a checkpoint's state holds the music player's pointers into it) | `xsong.c` |
| `+0x260000`-`+0x2A0000` | Other games' Navis (docs/MULTIROM.md): an overworld sprite and its mugshot each, copied whole (their offsets are their own); Gregar's list 6 and mugshots point at them from a number Falzar's Navis have there (72, 74, 76-78). Copied, as the songs are, all at once in the areas' order. After them the other games' map objects (sprites of their list 7), all at once the first time one is needed, each at a list-7 number of its own that points at the placeholder `0x084DC040`: BN5's flame of darkness (a DarkChip's, docs/META.md) at 160, decompressed and its palette turned purple, its Security Cube at 146, its wall of dark flames at 138, decompressed, a dark hole at 135. Its last `0x12000` bytes (from `+0x28E000`) hold a guardian of another game's (docs/BOSSES.md, BN5's Navis): his sprite and face, copied anew on each layer he guards and listed at Gregar's 78, which no area's Navi takes | `xnavi.c` |
| `+0x2A0000`-`+0x2E0000` | Other games' battle backgrounds (docs/MULTIROM.md): each one's BGAnimData record, its tiles and map (LZ77 encoded again), palette, animation scripts and the tiles and lists their frames name; then copies of BN6's three background tables (records, animations, scroll entries) with them after BN6's 22, which the loader's literals point at. Copied all at once in the areas' order | `xbackdrop.c` |
| `+0x2E0000`-`+0x2E1000` | BN6's own flame of darkness (docs/META.md, BN6's own DarkChips): its blue flame of sprite list 7 (`0x3C`, 1612 bytes) copied whole, its palette turned purple as BN5's flame is, and listed at Gregar's list-7 `0x54`, which points at the placeholder in the player's ROM; a place of its own, the same with or without another game's ROM, copied once a core | `xnavi.c` |
| `+0x310000`-`+0x320000` | Other games' net maps' backdrops and animations (docs/MULTIROM.md): for each of their areas, its learned map's BGAnimData record with the backdrop's tiles and map (LZ77 encoded again) and palette, and the map's GFXAnim scripts with the colours, tile lists and tiles their frames name (a palette script's RAM moved to BN6's). The map its layers take over has three entries of its group's tables pointed at them while one of its layers stands: its GFXAnim list, its BGAnimData record, and its 16-byte scroll entry rewritten with BN6's callbacks that do the same; every other layer writes BN6's own back. Copied all at once in the areas' order, as the first layer is built | `xbackdrop.c` |
| `+0x320000`-`+0x330000` | Home's other places (docs/HOME.md, piece 10), a part each: AsterLand's (`+0x320000`) and the Cyber Academy's four maps' (`+0x328000`): their NPC lists and their people's scripts and words, empty map scripts, their own warp lists (BN6's doors copied in, the doors out pointed where the planned town has them) and the music lists | `mapslot.c`, `indoors.c`, `aster_land.c`, `academy.c` |
| `+0x130000` | The town's coordinate data (walls, section 2, the jack-in cells, the checks) | `coords.c` |
| `+0x140000`-`+0x148000` | The town's NPC scripts, text, lists, warps, objects, check table and map text archive, apart from the layers' | `mapslot.c` |
| `+0x148000`-`+0x14C000` | Lan's HP's (docs/HOME.md): its empty NPC list and map scripts, its own warp list (BN6's jack-out on its blue pad, the run's portals pointed by the director) and the map music list it adds to | `mapslot.c`, `lanhp.c` |
| `+0x14C000`-`+0x150000` | Lan's house's and room's: their empty NPC lists and map scripts, their own warp lists (BN6's doors copied in), the room's jack-in table and the music list | `mapslot.c`, `lan_house.c` |
| `+0x150000`-`+0x152000` | The director's conversations: one text archive, rewritten for each | `talk.c` |
| `+0x152000`-`+0x153000` | The map-name label's archive: a copy of the game's with its 244 names pointed at where the run is ("Layer 12", "ACDC Town"); its other scripts (the PET's HP, zenny and BugFrags, 0xF0-0xF2) kept | `director_layer.c` |
| `+0x153000`-`+0x154000` | The PET's PLACE: the same copy, its names pointed at the area and the layer ("JudgeTree 14") | `director_layer.c` |
| `+0x154000`-`+0x166000` | The PET's words (docs/PET.md): the key items' names (+0x154000) and descriptions (+0x155000), the mails' senders and subjects (+0x158000) and bodies (+0x15A000), each BN6's archive rebuilt with the run's scripts in place of some | `pet_text.c` |
| `+0x166000`-`+0x167900` | Portraits of Falzar's Navis (docs/BOSSES.md): one mugshot sprite each, 0x500 apart (SpoutMan first), made from the Navi's battle sprite on the layer he guards; the mugshot table points at them from numbers empty in Gregar | `portrait.c` |

The guest core's ROM copy (BN5's, docs/MULTIROM.md, Guest battles) is
padded the same way; past BN5's 8 MB, from `0x08800000` (`BN5_FREE`): the
battle's record copy (`+0x00`), a copied record's entity list (`+0x20`: its
viruses scaled, or `--dev gem`'s Mystery Data set in) and the deck's
shelf (`+0x100`, the DarkChips'). In BN5's own data it writes the
encounter roll (its answer), the deck's compaction (a jump to the shelf),
its chips' records, reward rows and battlefield Mystery Data's finds
(`0x0801D62C`: All *, the rewards' codes and zenny), and for a guardian
of BN5's (docs/BOSSES.md, BN5's Navis) his
stats row's HP word at his version, capped to the act's band before his
battle and its own written back as it ends. Of its event flags (eToolkit
+0x44, `0x020029F8`) it sets the run's Souls' before each battle, and
clears those the run lacks: Double Soul's (0x0000), Chaos Unison's
(0x0236) and each Soul's own (0x0008-0x000D; docs/META.md, Souls in BN5
territory).

A battle with a green Mystery Data on the field also rewrites the reward
row its record names (rows 0 and 1, one per record, of the table at
`0x080211A0`, docs/ROM_DATA.md) with the run's own rewards before the
battle.

As a battle ends, the reward rows of its enemies (`0x080AC718` + id x
0x28) are rewritten in the folder's codes, half the time, by a hook on the
reward pick just before it reads them (docs/META.md, docs/ROM_DATA.md);
with the All * helper every chip entry in *.

While a run has the All * helper (docs/META.md, issue #18), every chip
record of ids 1-313 holds * alone (its four codes at `0x021DA8` + id x
0x2C written `1A FF FF FF`), and the Program Advance check's `cmp r2, #1`
at `0x08029606` is `cmp r2, #3`; each new layer writes them as the run's
helpers have them, and the ROM's own back before the game's own NEW GAME
boots (the folder it gives is counted in the pack by the records). As
such a run begins, the folder's 30 entries go to * and each chip's four
pack counts to its first. A guest battle in BN5's core does the same to
BN5's copy (its records at `0x01E210`, its check at `0x080252B2`), and
puts the folder's chips, DarkChips and reward rows in *.

BN6's own DarkChips (docs/META.md, docs/ROM_DATA.md; issue #70): as each
layer is made, the records of DrkSword, DarkThnd, DrkRecov and DarkInvs
(ids 286-289) are made whole in the core's copy: effect flag 0x20 set
(BN6's DarkChip class: three in a folder, MegaMan's NAVIGATOR line, the
purple card), library flag 0x20 cleared (the pack lists them), their
icon, picture and palette pointers their base chips' (Sword, Thunder,
Recov10, Invisibl), and their sort keys (IDSortPos their id,
AlphabetSortPos among the names before theirs), which the US version left
0 and by which the folder editor took any two DarkChips for one chip.
Their attack power is written as the BugFrags held say, on the map and as
a battle begins: their own with one or more, their base chip's with none,
and the base chip's for the rest of a battle whose last BugFrag a
DarkChip spent. DarkPlus's record is left as it is, out of the pack.

After a battle in BN5's core, BN6's encounter walk (eToolkit `+0x40`,
bn6f S2001c04: `+0x12` the distance walked since the last battle, `+0x14`
where the roll last checked) is cleared, as BN6 clears it entering the map
its own battles return to: the roll's chance rises with that walk, and
BN6's map is never left around a guest battle.

In the rival's netbattle (docs/RIVAL.md), ProtoMan's HP and MaxHP are
given the act's guardian band at most as he spawns, where his own 1800 is
above it: a hook lowers the value BN6's spawn sets both from.

The engine also takes over Central Town (`0x01:0`) or ACDC Town (`0x00:0`)
for the town (its tile map, coordinate data, NPC list, map scripts,
objects, sprite list, warp list, jack-in table, check table
`0x0803461C`, text archive `0x08040794` and music), clears the check
flags `0x16C0`-`0x16CF`, rewrites jack-in destination 42 (`0x08099A00` +
42 x 20) to the first layer, and
widens MegaMan's facing probes for talking (`0x0809F164`; in the Net
only, BN6's own in the real world, where A's checks look along them), redraws the
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
calls the game's routines (a warp among them) through a hook at
cbGameState (`0x080050EC`), a frame of the game's state update each.


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
On an act's last layer the pad leads home instead (docs/HOME.md,
`director_home.c`): the next layer is built as ever, Lan's HP is
installed (`lanhp.c`) with that layer behind its first portal, and warp 1
points at its arrival, BN6's blue pad, where Lan's PC's jack-in lands. In
Lan's HP the director holds the open links' story flags every frame (BN6's
link markers lock the others), and a portal's warp, seen pending as it
starts, builds its way's layer where another's was built and points the
portal's own warp entry at it before the game reads it. The run's town
stays installed, its jack-in to Lan's HP's arrival, and GameState's saved
real-world place at its port, so BN6's own jack-out from Lan's HP (R, or
the blue pad's warp) sets Lan down there.

`director.c` then watches the game: the exit pad's warp, choices made in text (event flags set by
Yes), battles won (viruses counted, bosses beaten), the game's GAME OVER
(the run ends), MegaMan on any other map for 90 frames (warped back to
the layer's start), and a checkpoint shortly after each arrival (`run.sav` plus
the core's `run.state`), once MegaMan is free to move: free as BN6 tests it
before the pad and START (`src/director/director_hold.c`: no fade, cutscene,
chat, lock, scripted walk, conveyor, NPC chat, warp, PET or pause), as every
save waits (the arena door's, home's, the PET's Save's, the quit's; issue
#23: a state written while BN6 held him came back held). A CONTINUE is
watched until MegaMan first walks: held from its load on with nothing of the
director's under way for two seconds, or the pad pushed three seconds with
no step taken (the player object's `BN6_PLAYER_PAD`), BN6's holds are let go
as its own routines let go of them and the map is entered again (then the
layer's start); a map's entry waiting on a fade's mark no fade runs is let
go too. The dev step `stuck NAME` (play.py, `--input "0:stuck conveyor"`) saves
the run held so, to test it. Quitting (Escape
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
far, and event flag `0x144E` in the state that L has told where they are,
which CONTINUE clears so that L starts over (`0x144F`: that the Net
Dealer has said his words, so a later talk is a line and the list;
`0x1450` the same for the NaviCust vendor, `0x1451` for a
Recovery Mr. Prog, whose heal is then one box; `0x1452` that a super
boss's staging has Bass's stone shake and crack (it was the Guardian
Data's second way on, until the act's ways became ports at home, issue
#86; docs/BOSSES.md, Super bosses); `0x1453`
that the layer's collector's vault gave its chip; `0x1454` that the super
boss strikes his pose out of the white (it was the Guardian Data's dark
way into the Undernet); `0x1455` that the
layer's official gate gave its chip, `0x1456` that Chaud's clearance
opens it, `0x1457` that Chaud's call on a duel layer was made,
docs/RIVAL.md; `0x146E`, never cleared by a layer, that the run's chips
that sit out of the older net's battles were named on its first such
layer; at home `0x1460`-`0x1462` that request 0-2 was taken, `0x1463`
that one is held and `0x146A` that the held one was settled, which the
askers' scripts set and the director keeps as each visit's places are
installed, docs/HOME.md piece 5; `0x146B` that this visit's order was
made at AsterLand's counter, piece 6).
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
A battle on the guest core (docs/MULTIROM.md, Guest battles) begun from a
frame's logic holds BN6's next frame until it ends, as on the main thread:
started at once, BN6 had walked one frame further into the battle's
moment, and the run went another way from there.
`CYBERWORLD_EMU_THREAD=1` runs it so on a computer, to test it. With the
core on its own thread, mGBA's threaded video draws its picture on one
more (`threadedVideo`), beside the emulation; the worker waits for it at
each frame's end, so the picture copied is whole. On the 3DS that thread
is on the main core, which waits most of a frame for the emulation.

BN5's guest core boots on a thread of its own on native builds, from the
game's start on (docs/MULTIROM.md, Guest battles: its boot): a core the main
thread leaves alone until the thread is joined at a frame's end, beside
BN6's on whichever thread BN6's runs. In the browser its boot fills each
frame's spare time instead: the frame's work and its slice within 10 ms
of the 1/60 s (12 ms while a battle waits for it), none in a frame
played unshown (`boot_spare_ms`, `guest_tick`).

## The core's time

Measured on a computer (issue #35), with a build that sampled the GBA's PC
every 1000 cycles and timed each frame with its picture drawn and, every
other frame, not, over autopilot runs: a walked layer, battles, the town.

| Scene | Emulation, ms a frame | With the picture | Busy-waiting for VBlank | With the idle hook |
| --- | --- | --- | --- | --- |
| A layer | 0.52 | 0.64 | 48% of the GBA's cycles | 0.36 (0.48) |
| A battle | 0.53 | 0.67 | 57% | 0.35 (0.49) |
| The town | 0.51 | 0.59 | 44% | 0.37 (0.45) |

- **The picture** is about a fifth of the core's time on a layer and in
  battle. On the New 3DS mGBA's threaded video draws it on the main core,
  beside the emulation, so the core's own thread pays the interpreter.
- **The wait for VBlank** was half of the interpreter's work. BN6's main
  loop waits for each frame by reading DISPSTAT until its VBlank flag is
  set (bn6f `main_awaitFrame`), and mGBA's own idle-loop removal leaves a
  loop that reads DISPSTAT alone, since the flag changes with no
  interrupt to wake a halted CPU. A hook (`src/emu/idle.c`) halts the CPU
  there while the VBlank interrupt is on, which ends the halt where the
  flag would have ended the loop: the autopilot's logs and run logs are
  the same line for line, at about 30% less of the core's time.
- **What is left** is spread out: on a layer the hottest routine is
  bn6f `checkOWObjectInteractions` (`0x080037F4`, a sixth of the busy
  cycles, each object's interaction area against the others'); BN6's
  memory copies through the BIOS's CpuSet and CpuFastSet, which mGBA's
  own BIOS runs as interpreted ARM (a sixth); MP2K's sound mixer,
  copied to IWRAM at `0x03005700` (a tenth); the sprite and OAM code in
  IWRAM; in battle no one routine passes 3.3%.
- **No routine as C.** An answer hook that does a routine's work in C
  takes the routine's GBA cycles away, and with them BN6's own lag
  frames, where a frame's work runs past VBlank and the game takes two
  VBlanks for it. Tried on the object interactions (7-11% less of the
  core's time on a map, on a computer): the game ran one frame more in
  a layer's first second, its RNG a call ahead from there on, and the
  random battles came at other steps. The wait for VBlank is the one
  place the GBA's time goes without the game noticing: no work is
  skipped there, only the wait.

The sound is not the same sample for sample as 0.6.0's: the hooks (and
before them the stubs they replaced, whose own instructions took the
GBA's cycles) move the game's writes to the sound registers by a few
cycles, which shifts the phase of the GBA's tone and noise channels. Its
loudness, 20 ms at a time over 50 s of an autopilot run, follows 0.6.0's
at a correlation of 1.0000, the mean the same.

On a Retroid Nova (ROCKNIX), the same 90 s of an autopilot run (`--seed 7
--dev god,onehit`, `tools/device_run.py` with `frame_log = on`) took the
GBA 3.98 ms a frame with its picture on 0.6.0 and 3.09 ms with the hook
(the median 4.00 and 2.80).

On a New 3DS (`frame_log = on`, a minute or so of the town, a layer and a
battle each), the core's own thread took 15.9 ms a frame with 0.6.0 and
11.0 with the hook (the median), and the main thread's wait for it fell
from 10.4 ms to 1. The frames shown are paced by the refresh now: the
present waits for it where the core has its own thread
(`present_3ds.c`). Before, a timer paced them, and the long core frame
had steadied that timer by accident: with the core faster, 6 frames a
second came early and 8 late (gaps under 12.5 ms or over 20); with the
refresh's pace, 0.1 and 1.3, against 0.6.0's 0.1 and 2. An old 3DS or
2DS has no third core: the core and its picture share the main one,
and there the title stuttered and the net went black after a few
frames, as before (3ds/README.md).

## Hooks

A hook lets one instruction of the game's code call the engine's C right
there, mid-frame, with the CPU's registers (`src/emu/hook.c`, issue #27):
the reach a native port's hooks give (Ship of Harkinian's, N64Recomp's
function replacement), while BN6's code stays the ROM's.

- **The BKPT.** A Thumb `BKPT #0xCE` (`0xBECE`), or an ARM one
  (`0xE1200C7E`), is written over the instruction in the core's ROM copy,
  as every patch is, or in RAM. mGBA runs a BKPT's handler at no cost in
  cycles, with the PC already past it (`src/arm/isa-thumb.c`). The board's
  handler, which mGBA installs once in `GBAInit` (a reset keeps ours),
  takes immediate 0 (the debugger) and 1 (the cheat device): ours comes
  first and passes it every other immediate.
- **After the hook.** `ARMRunFake` puts an instruction into the prefetch in
  the BKPT's place, as mGBA's own cheat hooks do: the original, which runs
  as if never replaced (`HOOK_CONTINUE`, with the registers as the hook
  left them); `bx lr` (`HOOK_RETURN`, r0 and r1 the result; at a routine's
  first instruction, before it pushes anything); or `bx r12` (`HOOK_JUMP`,
  on elsewhere).
- **Two kinds.** An *event hook* only queues the registers it met, and the
  director takes the queue up after the frame (`emu_hook_events`,
  `src/director/events.h` its kinds). An *answer hook* returns, or changes,
  what the director prepared before the frame, and may queue an event of
  its own where only some calls are worth one (`hook_post`). Neither
  touches the director's state.
- **Threads.** Where the core has a thread of its own (the New 3DS,
  `CYBERWORLD_EMU_THREAD=1`), a hook runs on it while the main thread
  draws. `emu_read*` and `emu_write*` wait for the frame in progress, so a
  hook reads and writes the game's memory with `hook_read*` and
  `hook_write*`, straight through mGBA's raw access; the queue is written
  only during a frame and read only after one, so it needs no lock. The
  browser build has no threads, and the same rules hold there.
- **States and resets.** A state holds RAM, not the ROM copy, so a hook in
  the ROM stays; one in RAM is written again after a state is loaded or the
  core reset (`hook_reapply`). A hook taken off restores the original; its
  slot stays only while the CPU may hold its BKPT in the prefetch.

The hooks in use (`src/director/encounter.c`; docs/ROM_DATA.md):

| Address | Routine | Kind | Runs |
| --- | --- | --- | --- |
| `0x08005A98`, `0x08005AE2` | checkThenStartBattle: its first test, the branch after the roll | answer: a forced battle, the engine's record (issue #29) | once a frame on the map |
| `0x08005BC8` | StartBattle | event: the record the battle starts from | once a battle |
| `0x080AC180` | the reward pick (bn6f `sub_80AA910`) | answer: its enemies' reward rows rewritten first | once a battle, as it ends |
| `0x08010D58` | every chip use's DarkChip check (bn6f `sub_8010D58`, `src/director/darkbn6.c`) | answer, posting an event (`hook_post`) where a DarkChip of MegaMan's runs as its base chip, no BugFrag in the battle's count | once a chip used |
| `0x0800B79A` | a chip's after-effects as it runs (bn6f `sub_800B79A`) | answer, posting an event where a DarkChip's dark power ran for MegaMan (its id 0x11E-0x122 reaches it only then): its price after the battle | once a chip used |
| `0x0800E2D8` | object_subtractHP | answer, posting an event (`hook_post`) where MegaMan's HP falls | in the rival's duel only: once per battle object a frame |
| `0x08007740` | the enemy spawn (bn6f `sub_800768C`), HP and MaxHP set | answer: the netbattle's ProtoMan held to the act's band | in the rival's netbattle only |
| `0x08005152` | EnterMap past its wait for the fade (`src/director/events.c`) | event: a map entered, after a warp or a battle | once a map |
| `0x0802F114` | SetEventFlag | answer, posting an event for a layer's choice flag (`0x1440`-`0x1447`) and a request's at home (`0x1460`-`0x1462`, `0x146A`) | about once a frame on the map |
| `0x0803CD6C` | GiveItem | event: a key item given (a ScrtData, the run's Spin) | once an item |
| `0x080050EC` | cbGameState, the game mode's state update (`src/emu/gamecall.c`) | answer: a queued game call jumps to its routine in the update's place, its return to `EMU_FREE` + `0x100` | once a frame of the game mode |
| `EMU_FREE` + `0x100` | where a game call's routine returns | answer: its r0 and r1 kept, r4-r11 put back, on to the main loop | once a call |
| `0x08120B90` | the PET's input handler (`src/director/pet.c`) | answer: A on Save taken for the engine (docs/PET.md) | each frame the PET takes input |
| `0x080003A6` | `main_awaitFrame`'s DISPSTAT loop (`src/emu/idle.c`) | answer: the CPU halted till the VBlank interrupt (The core's time) | about four times a frame (each interrupt wakes it) |

The duel's two are set as the duel begins and taken off as it ends, so
the other battles carry no hook a frame per object. The hooks replaced
reading the game every frame: the battle's objects for MegaMan's HP and
ProtoMan's, the battle state for its record, every 16 frames the
enemies' reward rows, the flags of the layer's choices, the key items
for a ScrtData or the Spin picked up, and the exit pad's warp-off flag,
written every frame because EnterMap clears the map's flags (`0x1640`-
`0x16FF`): now written as a map is entered and as the guardian's exit
opens.

What stays read a frame at a time, where a hook would only move the read:
whether MegaMan walks the map, battles or reads a menu (`on_map`, the
game mode, GAME OVER among them); the exit pad's warp under way and its
arrival; the frames on a map not the layer's before he is warped back;
the duel's DeleteTime, which its HUD shows as it runs.

Where a table written into the core's copy is simpler than a hook, the
table stays (issue #32): a layer's shop stock goes to the ROM's initial
shop table beside RAM, as the shop screen lists only entries found there
(one write per entry, made again with the layer after a CONTINUE); a
Chip Trader's map takes the first entry of the prize pools and of the
trader kinds (two writes), and its pool's records are rewritten in the
folder's codes either way; the BugFrag Trader's trade waits for its chat
to hold, a few reads while a chat is open, and runs as game calls.

`tests/test_emu.c` runs a ROM of the test's own bytes on mGBA (its
`GBAIsROM` wants `0xEA` at offset 3 and `0x96` at `0xB2`, nothing more):
routines called and their results stored, with a hook of every kind on
them, an ARM-state one, an event posted from an answer hook, the board's
`BKPT #1` passed on, the hooks taken off again, one in RAM written again
after a reset, and a halt from a hook. Then two cores side by side, run in
turn as BN6's and the guest's are (issue #61): BN6's made by `emu_init`
with a hook on it, the other as `src/emu/guest.c` makes one, from a copy
of the ROM that also plays a tone. BN6's hook runs in BN6's frames alone;
the guest's copy of the code stays unpatched, and a `BKPT #0xCE` written
into it goes to its own board, which skips it, never to BN6's hooks (the
hook table is BN6's core's: the guest has none); each one's RAM is its
own; the guest's tone reaches the ring BN6's silence fills, and a frame of
it let go (`emu_audio_drop_from`) does not; its state saves and loads with
BN6's RAM untouched. `build.py test` runs
it with the address and undefined-behaviour sanitizers, as CI does.

## Testing

`CYBERWORLD_AUTOPILOT=1` walks MegaMan to each exit (into a guardian's arena
and to its Guardian Data first; by the layer's heal when he is hurt, once a
layer, then below half his HP or with the guardian next) and fights its
battles from their state (docs/ROM_DATA.md, the autopilot's fight): on the
Custom screen the chips that go together worth the most, each used from a
panel where it reaches an enemy, off poison and away from the attacks it
can read (a lit panel, a shot coming down the row, a mine's landing panel),
the buster charged in between. Over seeds 1-12 and 30000 frames it won 43
of 44 virus battles and reached act 1's guardian in 11 runs, but beat him
once: 400-600 HP against MegaMan's 100. `CYBERWORLD_AUTOPILOT=weak` keeps
the first autopilot's blind button rhythm and walk, which the docs'
pictures are timed by, and keeps enemies at 1 HP, so what follows a won
guardian battle can be tested. In a battle on the guest core (docs/
MULTIROM.md, Guest battles) both fight as in BN6's, from BN5's memory,
which lays the battle out as BN6's does at its own addresses
(`src/emu/bn5.h`, docs/ROM_DATA.md); the autopilot never picks a
DarkChip. Over seeds 1-12 in ACDC Area, End Area and Nebula Area
(`--net-biome x0`, `x2`, `x3`, 15000 frames) it won 99 of 100 guest
battles (a BugTank in the back column outlasted it on a grass field),
where the button pattern it pressed there before was deleted in seed 1's
first; `weak`, BN5's viruses at 1 HP, won all 144 of its own.
`tools/device_run.py` runs a build on the device from `/tmp`. `CYBERWORLD_EMU_DEBUG=1` prints the depth, game mode, position,
map and the hooks' hits so far every 30 frames, prints the generated walls, and writes the tile map to
`.build/gen_tilemap.bin`.
`--net-biome N` puts every layer in one area, `--net-layout N` builds every
layer in one layout (`LAYOUT_*` in `src/net/net_layouts.h`).
