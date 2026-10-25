---
type: Engine Mechanism
title: "Testing"
description: "Testing the engine against the game: the autopilot (`CYBERWORLD_AUTOPILOT`, weak), debug output, battle traces for `tools/xcontrol.py`'s byte-by-byte control family, `tools/xfamily.py`'s CanGard checks and `tools/xdiff.py`'s differential."
tags: [engine, emulation]
generated: { by: claude-code/unrecorded, at: 2026-10-12T21:51:29Z }
---

# Testing

`CYBERWORLD_AUTOPILOT=1` walks MegaMan to each exit (into a guardian's arena
and to its Guardian Data first; by the layer's heal when he is hurt, once a
layer, then below half his HP or with the guardian next) and fights its
battles from their state ([BN6's battles](../../rom/battle.md), the autopilot's fight): on the
Custom screen the chips that go together worth the most, each used from a
panel where it reaches an enemy, off poison and away from the attacks it
can read (a lit panel, a shot coming down the row, a mine's landing panel),
the buster charged in between. Over seeds 1-12 and 30000 frames it won 43
of 44 virus battles and reached act 1's guardian in 11 runs, but beat him
once: 400-600 HP against MegaMan's 100. `CYBERWORLD_AUTOPILOT=weak` keeps
the first autopilot's blind button rhythm and walk, which the docs'
pictures are timed by, and keeps enemies at 1 HP, so what follows a won
guardian battle can be tested. The autopilot never picks a DarkChip of
either net and never presses A on UNITE or the Beast Out emblem
(`custom_guard`). A step it could take that leaves MegaMan where he stood
for half a second marks that panel blocked for the battle (a rock of
BN5's fields: in BN6's engine since the last round, an autopilot that
held UP into one stood 13000 frames, `--net-biome x4 --seed 12`), and a
step into the row it is after, blocked in his column, goes aside first.
Before issue #82's last round it fought the guest
core's BN5 battles too, from BN5's memory (99 of 100 won over seeds 1-12
in three of BN5's areas); BN5's areas and guardians are BN6's battles
now.
`tools/device_run.py` runs a build on the device from `/tmp`. `CYBERWORLD_EMU_DEBUG=1` prints the depth, game mode, position,
map and the hooks' hits so far every 30 frames, prints the generated walls, and writes the tile map to
`.build/gen_tilemap.bin`.
`--net-biome N` puts every layer in one area, `--net-layout N` builds every
layer in one layout (`LAYOUT_*` in `src/net/net_layouts.h`).
`CYBERWORLD_BTRACE=1` writes each frame of a battle's memory, EWRAM and
IWRAM, into the data folder's `btrace.bin`, as what changed since the
frame before (`src/emu/battle_trace.c`, its format in its header), for
`tools/xcontrol.py`: the control family's differential (issue #82; docs/
[Transplanted families](../../multirom/families.md)). Each of its two specs runs three
headless battles, each from the same boot, layer and keys and each against
one Mettaur at the same panel, started by the foes step (`0:foes 01@52`):
BN6's own, BN6's own with the control installed (`--dev xcontrol`), and
the control's moved Mettaur (`0:foes 200@52`); it compares them frame by
frame, every byte, and each byte that differs must have its reason (a
pointer into what was moved, read through the installer's map; the enemy
id and AI index where the battle keeps them; the text archive the name
plate reads its name from, BN6's viruses' for 0x01 and the families' for
0x200; the delete count the
installer's hook keeps AI 32 out of; the interrupt stack below its
`sp_irq`, `0x03007F60`, dead between frames, where an interrupt's entry
lands a few cycles apart as the moved code runs; the main stack below the
main loop's pointer as it waits for the next frame, `0x03007DFC` (read
by a hook at its DISPSTAT loop), also dead between frames, where the
VBlank handler's nested frames save what they interrupted (the BIOS's
decompression of the battle's sprites) a few cycles apart where the
lava's and the sea's install has the panels' flags read from ROM; and,
with BN5's ROM read, the code in IWRAM that install keeps patched,
`0x0300791C`-`0x0300791F`, `0x03007742`-`0x03007771` and
`0x03007678`-`0x0300768D`, and the sea's submerge's, `0x0300724A`-`0x03007257`
and `0x0300728C`-`0x0300729B`). The values of the enemy
id and AI index are allowed wherever they stand, not at fixed addresses;
the tool prints each address they explained (the battle's three copies of
the id, each a u16: `0x020348D4`, `0x0203AAB0`, `0x0203EB56`; the name
plate's archive at `0x0200A260`, the AI index at `0x02034181`, and the index times 4 that the
dispatcher leaves in r4: in the deferred-call queue at `0x0200A4A4`, and
in the parameters of an ice block it spawns, `0x02036874`). The `fight`
spec: seed 1, MegaMan at 1000 HP (`--dev hp=1000`): 3899 frames of battle
each, the native and installed runs the same at every byte (BN6's tables
grown for the families and the control installed, BN6's battle reads them
as its own: not one byte differs), and the moved
Mettaur the same as BN6's to the results (2903 frames: it changes rows 6
times and attacks 21 times, its shockwaves taking 110 of MegaMan's HP, and
takes 12 shots and 3 charged ones to its deletion), where the reward comes
from each id's own drop row (`0x4064` both: the control's rows are
Mettaur's, copied to its ids). One immediate changed
in the copy (the counter time at `0x0810B142`, `0x1E` to `0x1F`) shows at
frame 365. The fight never leaves the Mettaur's own states (8-12), so the
`status` spec gives it, by the status step (`0:status freeze 90`,
[Dev tools](../../devtools/overview.md)), each status a hit leaves for the engine to act on, at
the same frames in all three runs: frozen 90 frames (shot at twice),
paralysed and bubbled the same, dragged, pushed along, frozen 200 frames
left to thaw, then the fight's keys: 6299 frames each, native and
installed the same at every byte, and the moved Mettaur the
same as BN6's to the results (3565 frames), through BN6's generic states 4
(paralysis), 5 (drag, its 25 phases), 6 (frozen, twice) and 7 (bubble),
their hooks of tables 6, 7, 8 and 10 called through the grown tables, and
both ice blocks of size 0. Before the ice blocks' sizes were grown the
moved Mettaur's block was size 1 (it read past BN6's 30 viruses into the
Navis') and the run differed at the first freeze, frame 342; with the
control's recipe set to `.ice = 1` it differs there the same way. Not
reached: CurAction 3 (no way in for a virus, [ROM data](../../rom/overview.md)) and its
table-9 hook, a hit's own way to the statuses (the step sets what a hit
leaves; the resolution before it is BN6's own code, which reads a family
only through the tables the installer grows), and the rest of a battle's
statuses that are no state (confusion, blindness, holds).
`tools/xfamily.py` checks BN5's CanGard, the first family of another
game's ([Transplanted families](../../multirom/families.md)), in BN6's own battles:
headless runs with both ROMs (copied into `.build/xfamily/rom`), seed 15
(whose first area is BN6's own, not dressed as BN5's), each battle started
by the foes step and traced. `fight`: CanGard V1 at (5,2) beside a Mettaur
at (4,3), MegaMan idle at (2,2): its states 8, 9 and 10 alone after its
spawn, its reticle (BN5's type `0x25` through the second-level dispatch: the
third kind's `0xD2`, `0x25` in its side table) spawned on its panel and walking a panel
at a time to MegaMan's, then spawned on his, striking him for 10 each time
he was not still flashing from a hit (4 times in 1500 frames; Mettaur's
shockwaves the rest), its muzzle's blast (effect `0x6C`) drawn as it
fires, its MaxHP 60, element 0 and a shadow. `status`: freeze, bubble,
paralysis and drag, each by the status step while its reticle is out
(battle frames 43, 140, 237 and 334): each the frame after in BN6's
state for it (6, 7, 4, 5), its AttackVars +0x30 cleared and the reticle
gone two frames on (the release hook in tables 7, 8, 6 and 10), back in
its own states when the status ends and firing again; its ice block size
0; the battle on to the end. `delete`: each version alone, `--dev
onehit`, MegaMan shooting, event flag 0x1D8 set before the battle (the
gate of BN6's rare path, which versions 4 and 5 take): each spawned with
its struct2 row's MaxHP, deleted, its reticle gone, the results reached,
BN6's delete counts (`0x02000070`-`0x0200009F`, versions 0-3's path) and
the rare path's byte (`0x0203EAEA`, the chat of new virus data's index)
unchanged. Then the control, BN6's own Mettaur deleted the same way:
version 0 (id `0x01`) raises `0x02000071`, version 4 (id `0x05`) puts 1
at `0x0203EAEA`. With the installer's hooks on the delete count left out,
versions 0-3 raise `0x02000090` and versions 4 and 5 put 32 at
`0x0203EAEA`, and the check fails for each. `native`: BN6's Mettaur fought
by `xcontrol.py`'s fight keys with CanGard installed and with `--dev
nofamilies`: 3899 frames the same at every byte. Beside it, the autopilot (`CYBERWORLD_AUTOPILOT=1`) with both
ROMs, CanGard installed and not: seed 1's 15000 frames the same run log
and last frame, seed 15's 40000 (`--dev god`, two of BN6's battles won)
the same run log and its checkpoint's `run.state` the same byte for byte,
so a state saved on a map carries nothing of the install (again with
BN6's tables grown for the families, against `--dev nofamilies`: run
log, `run.state`, `run.sav` and `battle.sav` the same). Not checked there: a real
chip's hit on it, its guard against each kind of chip.
`tools/xdiff.py` (`build.py xdiff`; [The differential: a moved family against its own game](../../devtools/differential.md))
checks CanGard against BN5's own CanGard, frame by frame: one battle fought
in BN5's engine on the guest core (`--xdiff`'s reference, BN5's battle as
BN5 has it) and in BN6's with the family installed, aligned by each
battle's timer, compared by role through each game's numbers and layouts,
each difference either an engine difference explained with both games'
routines in `tests/xdiff/legit.json` (never on the family's own
records, where the engine's numbers are read as BN6's) or a failure,
MegaMan's own state stopping a spec as
inconclusive. Its specs and what they showed are in [Transplanted families](../../multirom/families.md),
Transplanted families; `--mutations` breaks the install eleven ways a
transplant bug would and each break fails its spec.
