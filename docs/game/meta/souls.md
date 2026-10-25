---
type: Feature
title: "Souls"
description: "BN5's Souls (issue #69): a beaten BN5 Navi's Soul serves every battle of the run, UNITE on BN6's Custom screen giving up a chip of its kind, Chaos Unison with a DarkChip, kept in `run.souls`."
tags: [game, meta]
generated: { by: claude-code/unrecorded, at: 2026-10-13T12:46:43Z }
---

# Souls (4 October 2026, issue #69; in every battle since issue #82)

A BN5 Navi beaten as a guardian ([BN5's Navis](../bosses/bn5-navis.md)) gives his
Soul for every battle of the run since issue #82's last round (until it,
for the run's BN5 battles alone, fought in BN5's engine on the guest
core): BN5's Soul Unison, which its US
version calls Double Soul and its Custom screen UNITE, and with a
DarkChip, its Chaos Unison (the owner's call). Reasoned with the
game-design skill; the owner approved all of it (4 October 2026), the
points first marked as proposals among them.

- **The dialectic** is the meta layer's, what you bring against what you
  find: the Soul is found, and the folder brought decides how often it
  unites, since each Soul takes chips of one kind. Inside a battle it
  restates the run's own tension, prepare or press on: a chip of the hand
  given up now for a few turns of the Soul.
- **The loop layers:** the moment (UNITE on the Custom screen beside
  CROSSSELECT, the turns it lasts), the session (a layer's battles), the
  run (which Souls it carries into every battle after). None reaches the
  meta loop.
- **The patterns:** costed power (each unison costs a chip and comes once
  a battle), nested progression (BN5's Souls beside BN6's Crosses, a Soul
  or a Cross at a time, never both), bonus with drawback (Chaos Unison),
  meta as variety, not power.
- **What each needs**, BN5's own rule as its ROM has it ([BN5's battles, records and rewards](../../rom/bn5-battles.md),
  BN5's Souls): the last chip picked on the Custom screen of the Soul's
  kind, given up for it. Team Colonel's MegaMan unites with the Souls of
  Colonel's team alone (the other team's six are never offered in this
  version). BN6's chips light it by BN5's kind of their names (soulkinds.h;
  [Transplanted Souls](../../multirom/souls.md)), BN5's own of the registry by
  their own:

  | Soul | Its kind | BN6's chips of it | Chaos Unison with |
  | --- | --- | --- | --- |
  | Colonel's | Obstacle | RockCube, TimeBom1-3, Fanfare, Discord, Timpani, Silence, VDoll, Guardian, Anubis, Otenko, WhiCapsl | DrkSonic |
  | ShadowMan's | Invisible | Invisibl, Mine, AntiDmg | DarkInvs |
  | NumberMan's | Plus | BusterUp, ColorPt, DblPoint | DarkPlus |
  | TomahawkMan's | Wood | Boomer, Lance, Snake | DrkLance |
  | KnightMan's | Break | JustcOne, BigHook, MetrKnuk | DarkDril |
  | ToadMan's | Aqua | BblWrap | DarkWide |

- **Where**: BN6's Custom screen, in the place under OK where BN6 puts
  its Beast Out emblem (BN6's emblem is BN5's UNITE button): UNITE while
  the last chip picked lights a held Soul, the emblem otherwise where the
  run has Beast Out; a Soul and a Cross are not worn together (another
  chosen replaces the one he wears), and no Beast Out while a Soul is
  united. The second screen's card under the cursor names the Soul, its
  turns, the chip it takes; MegaMan's form line there names the Soul he
  wears and its turns left.
- **When**, BN5's own: the UNITE button stands under OK only while
  MegaMan is neither worried nor dark as the Custom screen opens, and
  each Soul unites once a battle, for the turns BN5 gives it (three for
  every Soul, one for a Chaos Unison: BN5's OK, `0x08024FFA`, writes 3
  plus its battle NaviStats' +0x32, 0 in BN5's boot state, as the
  differential's KnightSoul and the game's NumberMan's show), its Chaos
  Unison once besides (BN5 keeps a bit for each).
- **Chaos Unison**, BN5's own: a DarkChip of the Soul's kind given up in
  the chip's place, where BN5's chaos flag is set, which the run sets
  while it holds a Soul. A DarkChip comes into the hand only while
  MegaMan worries (above), and UNITE stands only while he does not, so
  Chaos Unison asks for a DarkChip offered when hurt and kept to a calmer
  turn (a counter hit's Full Synchro, a virus deleted: each lifts his
  mood); a DarkChip once in the hand stays there, worried or not. Its
  charged shot is the DarkChip's attack (seen in BN5's engine: KnightMan's
  Soul with DarkDril fired a drill). Seen in BN5's engine, before the last
  round: DarkPlus offered worried, kept, and picked on a calm turn united
  MegaMan with NumberMan's Soul in Chaos Unison. In BN6's engine the
  same play is held to BN5's own battle frame by frame (`soul_mood_chaos`:
  DarkDril offered worried and kept, a Cannon landed, KnightSoul's Chaos
  Unison on the next screen); what lifts him is a chip's landed hit (a
  Cannon 0x1E, as BN5 counts it), never the buster's. Chaos
  Unison's charge is the Souls track's S6 ([Other games' ROMs](../../multirom/overview.md)).
- **No price for it** (as BN5 has it): BN5 counts a DarkChip
  given up for Chaos Unison as no DarkChip used (its use flag stays
  clear), and the run follows it: no 20 max HP (seen in the game: none
  taken after that Chaos Unison). The DarkChip still leaves the hand for
  the battle. The owner kept it unpriced.
- **A Soul's span**: the rest of the run, in every battle, BN6's areas'
  and BN5's. A run holds every Soul it wins, as BN5's MegaMan holds his:
  one per BN5 guardian; a guardian whose Soul the run holds already adds
  nothing. Nothing carries over to the next run, and no Soul is brought
  at setup: it would be power across runs. A run continued without BN5's
  ROM keeps its Souls, asleep until the ROM is back, and MegaMan says so
  once ("I can't feel the old Souls,Lan...").
- **The key with the lock**: two kinds have few chips in BN6
  (ToadMan's Aqua only BblWrap, KnightMan's Break JustcOne and two
  Gigas). Until the last round his Guardian Data gave a chip of his
  Soul's kind; now it gives his own Navi chip, as BN6's guardians give
  theirs ([Guardians and super bosses](../bosses/overview.md)), BN5's chips of every kind being in the run's
  finds and stock on its areas ([One net: the design](../../multirom/one-net.md)).
- **Said where it happens:** at his Guardian Data MegaMan names the Soul,
  that it is with them every battle, the chip kind it takes (with one of
  the run's chips of it), UNITE, once a battle; all of it the first time
  a profile wins a Soul (`profile.soul_taught`), "His Soul,Lan! It's with
  us every battle." after that, once a profile that a Soul and a Cross
  are not worn together (`profile.soul_form_taught`), and "We already
  have his Soul!" where the run holds it.
- **How it is kept:** which Souls the run holds, beside its checkpoint as
  its DarkChips are (`run.souls`), for the run whose seed it names; a new
  run holds none. As each battle begins they are handed to BN6's engine
  (`soulunite_held`), where the Souls stand wherever BN5's Navis are.
- **What could go wrong:** the strongest Soul for a folder taken every
  time it is offered (each costs a chip and comes once a battle);
  Chaos Unison as the one right play for a DarkChip (its window is narrow:
  offered worried, united calm); five Souls stacking power within a run
  (each still once a battle, each a chip given up); a Soul and a Cross
  read as one thing (the second screen's card and MegaMan's word say
  they are not worn together).
