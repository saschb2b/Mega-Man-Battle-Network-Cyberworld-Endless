---
type: Feature
title: "Stage 2, as built: the liberation battle"
description: "Stage 2, the liberation battle on BN6's loop (`liberation_battle.c`): BN5's three-turn limit and banners, the Custom screen opening on a full gauge, what each battle does on the board, and the stakes box."
tags: [game, liberation, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-25T00:15:50Z }
---

# Stage 2, as built: the liberation battle (23 October 2026)

**BN5's rules, read in its code** (`src/emu/bn5_liberation.h`; [BN6's battles](../../../rom/battle.md),
BN6's battle loop and banners). BattleState +7 counts the Custom screens
opened (played in romlab: 0 until the first opens, then 1). As each turn's
fight starts (`BN5_LIB_TURN_BANNER`) the battle shows TURN n START!, n the
turn, and from the third FINAL TURN!. Where the Custom screen is asked for
in the third (`BN5_LIB_TIME_ASK`), the battle's phase 0x18 instead
(`BN5_LIB_TIME_UP`): TIME UP!, 30 frames, LIBERATE FAILED!, the battle
over. At a win (`BN5_LIB_WIN_BANNER`), 1 TURN LIBERATE! where it is the
first turn, else LIBERATE SUCCESS!. The banners are BN5's battle messages
(`BN5_BATTLE_MESSAGES`), and BN6 keeps that table whole (`0x0801EF84`,
TURN n START! to n TURN LIBERATE! at the same numbers), but none of its
liberation rules.

**On BN6's loop** (`src/director/liberation_battle.c`, `src/emu/bn6_battle_loop.h`):
the loop random battles and bosses run, answered by hooks while a
liberation's battle is armed (the director arms it as the command is
taken), no code moved:

- each turn's fight starts with BN5's banner in BN6's own message engine
  (the fight's BATTLE START! answered: TURN 1 START!, TURN 2 START!,
  FINAL TURN!);
- the gauge full, BN5 asks for the Custom screen by itself (its
  liberation's ask tests the gauge-full flag where every other battle's
  tests L or R: played, the screen opened the frame the gauge filled, no
  key held), so here too: below the final turn BN6's own way to the
  Custom screen from L or R is taken (`BN6_FIGHT_CUSTOM`); in the final
  turn the loop goes to BN6's own draw (a NetBattle's judge's end): the
  battle stops, its banner TIME UP!, the draw run again for LIBERATE
  FAILED!, then result 3, which ends the battle with no results screen
  and no reward, MegaMan's HP as it stands;
- a win shows 1 TURN LIBERATE! or LIBERATE SUCCESS! in the win's place
  (ENEMY DELETED!), so the board no longer shows its own banner after the
  battle (stage 1's);
- no running from it (the record's run bit off, as from BN5's).

**What a battle does on the board** (`liberation.c`):

| Battle | Won | Won in its first turn | Time up |
| --- | --- | --- | --- |
| A dark panel (Liberation) | the panel freed, its item given | and the dark around it (its eight: dark, items, the Darkloid's ring once open; no unit's) | the panel stays dark, the phase spent |
| LongSwrd (an Order Point) | the panel and the one behind it | and the dark around the first | as above, the Order Point spent |
| A guardian | deleted (back from its DarkHole while the hole stands), its panel freed | and the dark around it | the guardian stays |
| A DarkHole | closed: its guardians gone for good, the eight around freed | the same | the hole stays |
| The Darkloid | deleted, the board won | | he stays, the phase spent |

The battle's viruses are the act's own, BN5's on BN5 territory, from the
run's encounter tables, harder with the dark around the panel (stage 1's
bands), set out on BN5's field for the panel (below, The battle's field).
**The Darkloid's battle** is the run's guardian Navi of the act's band in
his own battle (stage 1), three turns like every other: BN5's battle reads
nothing of its board but a unit's HP (below), and his own record played in
romlab timed up in its third turn. (Stage 2 first took his battle as
untimed with every hit doubled from its eleventh turn, `BN5_LIB_LATE_DOUBLE`;
that rule is a type 1 battle's, and every liberation battle is type 6.)

**The stakes, before the battle.** With the command menu open, a box above
BN5's window (under the PET look's menu without BN5's ROM) says what the
command under the cursor stakes: "3 TURNS: FREES 1" and "1 TURN: 5 MORE"
(the dark around that a first-turn win frees too), "3 TURNS: DELETES IT"
for a guardian, "3 TURNS: CLOSES IT" and "FREES 9" for a DarkHole, "3
TURNS: DELETES HIM" for the Darkloid, and "SURROUNDED: L/R TURNS" under
them where the battle will be (`liberation_battle_words.c`).

**The design** (the game-design skill): the battle restates the board's
*press on or prepare*. The dark around a panel makes its battle harder
and its first-turn prize larger, so pressing into the thick dark is a
gamble the folder must back, and a safe panel at the edge frees little:
a costed choice, shown before it is taken (an honest telegraph: the
turns, the prize, the banners in the battle counting them down). Failure
is graduated: a time up costs the phase (and LongSwrd's Order Point) and
the HP the battle took, never the run on its own; the board's harm still
never takes the last HP. A deletion in the battle is the run's, as in
every battle.

**Checked:** `--dev dark,god` with BN5's ROM, a dark panel taken and its
battle left to run: TURN 1 START!, TURN 2 START!, FINAL TURN!, the gauge
full, TIME UP!, LIBERATE FAILED!, the board again with the panel dark
(`liberation: battle 0 at 34,33 time up (turn 3)` in the debug log); with
`onehit`, 1 TURN LIBERATE! and the dark around freed. The autopilot with
`god,onehit` (seed 1) won the boards of acts 2 and 3, guardians, DarkHoles
and Darkloids (act 2's Darkloid in his battle's seventh turn); with `god` alone
its buster battles timed out in 13 of 25 and the board ran past 60000
frames, and without `god` (seed 2) MegaMan was deleted in its third
battle: the autopilot's battles are weak, and a dark layer asks a folder
that deletes in three turns.

**Left:** the Darkloid's intro and BN5's Darkloids (stage 4); the team
(stage 3); a unit's HP carried from the board into its battle and back
(below); BN5's "Surrounded!" line in the battle (below).
