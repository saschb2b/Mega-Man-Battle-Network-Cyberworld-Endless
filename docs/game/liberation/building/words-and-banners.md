---
type: Feature
title: "The board's words and banners as BN5's, as built"
description: "The board's words and banners as BN5's: the Darkloid's entrance line (`lib_entrance_words`), the L view's keys, the units' reach in red while L is held (`lib_threat`), and the win's SUCCESS! and tally."
tags: [game, liberation, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-25T01:28:59Z }
---

# The board's words and banners as BN5's, as built (25 October 2026)

Traced in romlab on the plain ROM ([BN5's Liberation Missions](../../../rom/liberation.md), BN5's Liberation
board: the L view's keys, the Darkloid's first word, the mission won and
its tally) and drawn or read from BN5's ROM as the game runs, its text
and tiles never kept:

- **The Darkloid's entrance.** BN5's opening tour passes the Darkloid in
  silence (mission 1 recorded: the tour, then Colonel's tutorial; missions
  2-9 open on the team's words). He speaks at his first turn of the first
  DARKLOID PHASE, after the banner and before anything of his phase, with
  his mugshot, once (his archive's entry 0, through a flag: BlizzardMan
  claims the area for a ski resort). Ours the same: at the first
  Darkloids' phase, before the DarkHoles send anyone back and before his
  turn, the run's Darkloid says his line with his face, in his own habit
  of speech (`lib_entrance_words`, `src/director/liberation_words.c`; the
  run's Darkloids are BN6's Navis and BN5's six, none a Darkloid of
  BN5's, so the lines are ours, [Voice](../../../voice/rules.md)'s guardians). Not kept
  across a CONTINUE: a board continued in its first phase hears it again.
- **The L view's keys.** BN5's help at the top right, its eight OBJs
  ("+ MoveCam", "B+ + MoveCam(F)", "L Back") from its tiles on the band's
  fill in WIN0, drawn with the view's phases and arrows
  (`lib_piece_phases`).
- **The guardians' reach in the L view.** BN5's L view shows no reach: the
  camera taken onto a BigBrute in it, A, R and SELECT pressed, nothing
  changes (its help names the D-pad, B and L alone). The quality bar's
  honest telegraph (rule 4) puts it there, in the one look BN5 gives a
  reach, the red it lights a strike's in (look 0x0D): while L is held, every
  panel where the units would strike a Navi in their next phase
  (`lib_threat`, the planner's own reading) lies in BN5's red, steady, and
  goes as L is let go (`src/director/liberation_look.c`). Reasoned with the
  game-design skill: the dialectic is the board's *press on or prepare*,
  and the reach is what a phase's choice costs; an honest telegraph shown
  on demand (BN5's own map view, which a player opens to plan) keeps the
  board as BN5 draws it the rest of the time; steady, not BN5's blink and
  sound, which say a strike is coming now.
- **The mission won.** BN5's routine 6 after the Darkloid's battle:
  BN5's song for it (0x18) at once, SUCCESS! in the phases' band 22
  frames on (its words BN5's, PLAYER PHASE's palette, their sound), the
  band's 104 frames, then 20 more, and the team's leader's tally:
  "Target" and "Actual" phases, then the Officials' reward (a chip two
  under par or better, another at par, zenny over it), BN6's give after.
  Ours the same from the Darkloid's deletion (`LIB_STEP_CLEAR`,
  `src/director/liberation.c`): the song, SUCCESS!, then MegaMan's tally
  in BN5's own words, MegaMan's three (the leader of a lone MegaMan's
  team), read from BN5's ROM as it is said with the par and the phases
  ours (`lib_piece_tally`), then the prize given by BN6's own give alone
  (`ta_lib_prize`; the Server's "The signal left a chip!" no longer
  before it). Over par the prize is zenny, as BN5's (the owner's decision
  6, The par's prize, as built), the tally's first page BN5's and the
  zenny given after it. Then "The exit's open,Lan!", the fence open
  through it.

**Checked:** side by side with BN5 in the pull request (the entrance,
the L view, the clear); an autopilot run through seed 1's act 2 board
(`--run-depth 5 --seed 1 --dev dark,god,onehit`).
