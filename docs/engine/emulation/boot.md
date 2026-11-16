---
type: Engine Mechanism
title: "A new run and boot"
description: "NEW GAME's start in the town (first layer built, Lan warped to his start, jack-in flag 0x1727 cleared) and `emu_boot`, which presses through BN6's title to Seaside Area 1 and saves `boot-4.state`, its number bumped when what NEW GAME gives changes, and a kept state that holds a played game made again (issue #688)."
tags: [engine, emulation]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-16T23:10:00Z }
---

# A new run and boot

## A new run: the town

NEW GAME builds the run's first layer and the town ([The overworld](../../world/overworld.md)),
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
Seaside Area 1 and ends the intro cutscene, then saves `boot-4.state` in the
data directory. Later runs start from that state. It is made on the device
and never shipped.

Every later build loads the state the first one kept, so its number
(`BOOT_STATE` in `src/emu/boot.c`) is bumped whenever the boot's sequence
changes or what NEW GAME gives in the core's copy does. 4 came with
ten-bit chip ids ([the census](../../rom/chips-code.md), Ten-bit chip ids),
which widen BN6's preset folders in the core's copy: a `boot-3.state`, kept
by a build before them, held the Standard folder as BN6's ROM holds it,
nine-bit. On an install that had kept one before 0.21.0, every new run
on the Standard folder from 0.21.0 to 0.21.2 read those entries as other
chips: Cannon B as BN4's CloudMn (513) in A, AirShot * as AirShot in N,
MiniBomb B as BN4's 566 in A. A code the chip's record lacks is dealt
blank, and so is a chip of BN4's where its ROM is not read: no icon, and
used, no effect. A New 3DS, which reads BN5's ROM beside BN6's and not
BN4's, showed it first (November 2026: 26 of its folder's 30 entries).
The new state is made at the next start, and the old one is deleted once
it is written. A run begun from the old state is mended as it continues
(`own_folder_chips`, `src/director/director_folder.c`): an entry of the
Standard folder's nine-bit form that the pack counts in its nine bits
and not in its ten is widened, as GiveFolder had counted it, and the log
says how many (`folder: N of the Standard folder's entries widened`).

### A kept state that is no new game's

A new run's game is the kept state's, whole: BN6's RAM and its cart's
save RAM as the boot left them. Issue #688 (0.22.1, Android) had every
new run begin with the last run's MegaMan: his HP, zenny, SubChips,
RegUPs, NaviCust and Crosses, and his pack, which made blank entries of
the Standard folder and "damaged chip data" at its edit, until the app's
data was deleted. The player's `.cwsave` on a fresh data folder began
clean runs, and a played run's checkpoint put where the kept state lies
gives exactly the report (verified: the run's zenny and chip won in the
new run's PET), so the kept state on that device held a played game. How
it came to is not found: no writer of the file but the boot's own, after
its own power-on.

So the boot no longer takes a kept state on trust (`new_game_held`): it
reads what a run gains there (zenny and BugFrags none, MegaMan's maximum
HP and its base 100, no RegUP, SubChip or NaviCust program among the key
items, the pack holding the Standard folder's 30 chips and no other) and,
where any differs, says so in the log (`boot: the kept state is no new
game's (zenny 10000, ...): made again`), boots from power-on and keeps
that instead. A clean NEW GAME's own key items (0x00 once, 0x75 four
times) are left out of the reading. A device holding such a state is
mended at its next NEW GAME, with no data deleted.
`tests/scenarios/run_new_from_a_played_boot.py` gives the boot the run's
own checkpoint as its kept state (`--dev bootplayed`): without the check
the new run kept the run's 10000 zenny, with it none.

The boot's frames are never shown, so the core draws no picture for them;
nor for a frame the main loop plays unshown to catch up with the GBA's pace
(`emu_frame` sets the core's frame-skip counter for that one frame, where
its struct reads back as expected).
