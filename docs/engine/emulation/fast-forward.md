---
type: Engine Mechanism
title: "Fast forward"
description: "Fast forward's frame batches in `scene_emu.c`: where BN6's state allows it (`fast_forward_game.c`, never the Custom screen or a fight), the 10 ms budget, sound kept at pitch by crossfaded grains, run time in game frames."
tags: [engine, emulation]
generated: { by: claude-code/unrecorded, at: 2026-10-24T23:07:49Z }
---

# Fast forward

Fast forward ([The player's guide](../../GUIDE.md#fast-forward), Fast forward) plays several of the game's
frames to one shown: `scene_emu.c`'s update runs a batch, each frame with
the update's keys and the run's logic after it, as the dev menu's speed
has always done, and draws the last. Where it may run is read from BN6
before each frame of the batch (`src/emu/fast_forward_game.c`, its rules
in `src/core/fast_forward.c`): the run's own game on the screen, not
online, the Jukebox or the dev menu; `BN6_TOOLKIT`'s mode the game's (not
the PET's screens, a shop, a trader, mail); `BN6_GAMESTATE` not the PET's
menu; and in a battle, `BN6_BATTLE_PHASE` neither the Custom screen's
(`BN6_PHASE_CUSTOM`) nor the fight's (`BN6_PHASE_FIGHT`, the pause and
the time a chip stops with it). A frame that opens the first Custom
screen, the PET or a shop ends the batch, so the next frame is played at
BN6's pace. A trace of seed 1's first battle, frame by frame, shows what
that leaves: `BN6_SUB_BATTLE_INIT` for 16 frames, phases 0x00 and 0x04
for about 125 as the enemies appear, 0x08 from the first Custom screen,
0x0C for the fight, then `BN6_PHASE_RESULTS` from the last deletion (the
result byte 1) for about 155 frames of ENEMY DELETED! and the RESULT
screen, 0x14, and the map. MegaMan's deletion goes from 0x0C to 0x14.
Run on the autopilot, those openings and closes are about five and a
half seconds a battle at the GBA's pace.

A batch keeps to 10 ms of the update (`BUDGET_MS`): another frame is
played only where the batch's time and the last frame's still fit, so a
machine that cannot run 3x runs what it can, and the loop's own pace is
kept (headless, it runs at the speed set). Every frame but the last is
played unshown, mGBA's `frameskipCounter` set so the core draws no
picture for it, as for a frame the loop catches up with. On the core's
own thread (the New 3DS) each frame of the batch waits for the one before
it, so the budget lets fewer through: not measured on a 3DS yet.

**The sound** keeps its pitch. Only the first frame of a batch is heard
(`EMU_FRAME_UNHEARD` on the rest, `emu.c`): the ring takes one frame's
sound an update, as at the GBA's pace, so it neither starves nor floods,
and the music plays faster at its own pitch, as a granular time stretch
does, each frame's sound a grain. The grains are joined by a crossfade:
the first 5 ms of the frame after a heard one, which carries on where it
stopped, faded out under the next heard frame's first 5 ms. On the core's
sound dumped over the same 760 frames of seed 1 with fast forward on, the
largest jump between samples (the second difference) was 16404 with the
grains butted and 9288 crossfaded, and those over 6000 went from 440 to
272; the 272 lie in one burst of a battle's noise, spread evenly over the
frame's 800 samples, not at the joins.

**The run's time** counts the game's frames (`profile_played_frame`, from
`director_update`), not the updates: the statistics' minutes and the
week's DeepNet's tie-break ([Online: the Net Square](../../online/overview.md)) are the game's own time, the
same with fast forward or without, and at the GBA's pace the same as
before.
