---
type: Design
title: "MegaMan's forms"
description: "MegaMan's form for a run (issue #774): CROSS, SOUL BN5 or SOUL BN4, one system a run, chosen on the setup's FORM row; every guardian pays in the run's form by his kind, else a power the run lacks; the Soul forms opened by their games' Navis; one power brought; SoulT+1 in place of BeastOut; the week's form."
tags: [game, meta]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-22T12:35:44Z }
---

# MegaMan's forms (22 November 2026, issue #774)

A run plays one of MegaMan's forms: **CROSS** (BN6's ten Crosses and
BeastOut), **SOUL BN5** (BN5's twelve Souls and Chaos Unison) or **SOUL
BN4** (BN4's twelve Souls). The owner's calls of 22 November 2026 are in
the issue; this page is what was built from them and how the open points
were settled with the game-design skill. Until BN4's and Team ProtoMan's
Souls land, the forms stand behind `--dev forms` (`--dev form=N` for one
of them): without it every run is the mix below, as before.

## The design

- **The dialectic** is the meta layer's ([The meta layer's design](design.md)),
  what you bring against what you find: the form is the largest thing a
  run brings, and it decides what its guardians can give.
- **The loop layers:** the run (which powers it finds, what the Custom
  screen offers) and the meta (the Soul forms opened across runs, a best
  rung kept per form).
- **The patterns:** meta as variety, not power (one chassis, a kit each);
  a subsystem per character (a Cross's weakness, a Soul's chip given up);
  an opportunity-cost loadout (one system a run, no kitchen sink).

## What a guardian pays

Every guardian pays in the run's form (`src/director/forms.c`,
`forms_pay`):

1. **His own** where his game's form is the run's: a BN6 Navi's Cross in
   CROSS, a BN5 Navi's Soul in SOUL BN5, a BN4 Navi's in SOUL BN4.
2. **Else his kind's**, by #774's table: a BN6 Navi's kind is his Cross's
   element (ProtoMan's and Colonel's, who have no Cross, their Souls' in
   the older games), a BN5 or BN4 Navi's his Soul's. The first of the
   kind's powers the run does not hold: HeatCross before ChargeCross,
   GroundCross before DustCross.
3. **Else one the run lacks**, where his kind has none in the form, or the
   run holds it: one of the form's powers it does not hold, picked by the
   layer's seed and the Navi, so a CONTINUE pays the same.

A form's powers are only those that stand in BN6's engine this session:
Gregar's five Crosses always, Falzar's where CROSSSELECT lists them, a
BN5 Soul where BN5's Souls unite, a BN4 Soul once it is moved
(`forms_ready`). A Soul not yet moved is simply absent; a run holding
every power of its form gets his Navi chip alone.

What the data says is made with the layer, from the powers held as the
layer was made (`run.cross_layer`, `souls_layer_bn5`, `souls_layer_bn4`),
as the Souls' words were. A Cross keeps BN6's "MegaMan got:" box. Where
the power comes in another shape, MegaMan says so ("Whoa... ToadMan's
data is changing!", "It's taking a Cross's shape!"): the fiction is that
he takes a Navi's data in the shape of a form he knows. A BN5 or BN4 Navi
beaten in a CROSS run still gives his Cross by kind, and MegaMan feels
the Soul in his data ("Lan... There's a Soul in his data."), as he does
of a BN4 Navi in SOUL BN5 or a BN5 Navi in SOUL BN4 (`forms_soul_felt`). The lines
are `src/director/forms_words.c`'s.

The super bosses pay as before: Bass his GigaChip, the Cybeast BeastOut
in CROSS. The mix keeps today's rule
([Souls](souls.md), [BeastOut from the Cybeast](beastout-cybeast.md)).

## What a run brings

- **A new profile plays CROSS**, pure BN6; the Soul forms open in later
  runs. The setup's FORM row offers each opened form whose game's ROM is
  read (`meta_form_open`, `forms_offered`): SOUL BN5 once a BN5 Navi fell
  as a guardian in any run (or a Soul was won before forms,
  `profile.soul_taught`), SOUL BN4 once a BN4 Navi did
  (`profile.forms_open`), with BN5's ROM beside BN4's, since the Souls'
  engine BN4's Souls unite in is built from BN5's. A BN4 Soul counts as
  standing once its power attack is placed (`soulpower_bn4`), what UNITE
  asks. Its note names the best
  threat rung won with that form (`profile.form_best`): a record, never a
  key, and no title mark.
- **One opened power brought** (the Cross start widened): in CROSS a
  Cross whose Navi fell, as before; in a Soul form one Soul the profile
  has won in an earlier run (`profile.souls_won`). Settled here: the
  brought power is a head start, not the run's only one. Guardians still
  pay, the brought one counting as held, so a guardian of its kind pays
  the next. Today's Cross start kept the Cross alone, a run holding no
  other; under "every guardian pays" that would leave a guardian of a
  brought run with no prize, which #774 names as the thing to avoid. The
  run's ceiling stays the form's set: a start moves when a power comes,
  not how many.
- **BeastOut only in CROSS.** In a Soul form the Cybeast's fall gives
  BN5's SoulT+1 for the rest of the run, a unison lasting four turns
  (`run.soul_turns`), and the fifth helper brings it from the first
  battle. Settled here: it does not stack with the SoulT+1 program on the
  NaviCust board, one turn more at most (MegaMan's NaviStats' extra turns
  raised to 1 as each battle starts, never added to), and BN4's Souls take
  the same turn. In a Soul form the setup names the helper SoulT+1, and
  Dad's call at the run's start says the beast's power is in MegaMan's
  Souls, not that he unlocked the CybeastButton.
- **Every Soul asks for its chips.** The finds and a Net Dealer's fourth
  chip lean to the kinds of every Soul the run holds, Team ProtoMan's and
  BN4's as Team Colonel's did ([A held Soul's kind](../economy/held-soul-kind.md));
  a CROSS run holds none and leans nothing.

## The week's net

The week's net names a form each week from its seed, the same for every
diver (`run_week_form`): no change on the wire. Settled here: it plays its
form whether or not the profile has opened it, and opens and counts
nothing ([The week's net and the meta layer](weekly-net.md)); every diver
has every game's ROM, which the Deep Net's dive asks.

## What is not done yet

- **The Souls to come.** Team ProtoMan's six (BN5's Souls 1-6) are not in
  SOUL BN5's list until their branch moves them ([Team ProtoMan](../team-protoman.md)), so HeatMan
  pays one of Team Colonel's there today; BN4's twelve come one by one
  (ThunSoul alone now, [BN4's Souls on BN6's proxy form](../../multirom/bn4-souls.md)),
  and a SOUL BN4 run's Souls reach UNITE beside BN5's
  (`soulunite_bn4_bits`, `encounter.c`): ThunSoul, won from a guardian, unites
  in the next battle (`tests/scenarios/forms.py`).
- **The flag.** `--dev forms` stays until every Soul has landed; the
  orchestrating session switches it on for good.

## How it is kept

`run.form` (`RUN_FORM_*`), `run.soul_start`, `run.cross_active`,
`run.cross_held`, `run.cross_layer` and `run.soul_turns` in the `Run`
("CWEE", [Portable saves](../../engine/saves.md)); BN5's and BN4's Souls
held in `run.souls`. A run saved before forms finishes as the mix, every
system at once, and the next run picks a form. The statistics' run event
carries the form ([The anonymous statistics](../../devtools/statistics.md)).
