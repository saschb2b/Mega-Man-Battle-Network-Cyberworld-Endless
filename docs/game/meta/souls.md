---
type: Feature
title: "Souls"
description: "BN5's Souls (issue #69): a beaten BN5 Navi's Soul serves every battle of the run, UNITE on BN6's Custom screen giving up a chip of its kind, Chaos Unison with a DarkChip, kept in `run.souls`."
tags: [game, meta]
generated: { by: claude-code/unrecorded, at: 2026-10-25T13:09:14Z }
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

  | Soul | Its kind | The chips of it a run can hold: BN6's; BN5's own | Chaos Unison with |
  | --- | --- | --- | --- |
  | Colonel's | Obstacle | RockCube, AirRaid1-3, LilBolr1-3, TimeBom1-3, Fanfare, Discord, Timpani, Silence, WhiCapsl, VDoll, Guardian, Anubis; BoyBomb1-3 | DrkSonic |
  | ShadowMan's | Invisible | Invisibl, Mine, AntiDmg; Blinder | DarkInvs |
  | NumberMan's | Plus | Atk+10, Atk+30, Navi+20, BusterUp, ColorPt, DblPoint | DarkPlus |
  | TomahawkMan's | Wood | CornSht1-3, RlngLog1-3, RskyHny1-3, Boomer, HiBoomer, M-Boomer, BambSwrd, Lance, Snake, WoodDrgn; CactBal1-3, WoodNos1-3, AntiWood, MudWave, his three Navi chips | DrkLance |
  | KnightMan's | Break | AuraHed1-3, GolmHit1-3, IronShl1-3, AirHocky, DrilArm, JustcOne, BigHook, MetrKnuk; DrilArm1-3, Quake1-3, AirHoc, CannBall, his three Navi chips | DarkDril |
  | ToadMan's | Aqua | AquaNdl1-3, BblStar1-3, TrnArrw1-3, AquaSwrd, BblWrap, WideSht, AquaDrgn, BlzrdBal, DiveMan; AqWhirl1-3, SideBub1-3, WideSht1-3, WavePit, AntiWatr, Geyser, MetaGel, RainyDay | DarkWide |

  (Counted by `build.py economy`, its last table: the chips of each kind in
  `chip_pool.h`'s pools, BN6's by BN5's kind of their names as soulkinds.h
  reads them, and BN5's own of the registry; DarkChips and the chips a run
  never holds left out, Otenko among them.)

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
- **The key with the lock** (corrected 25 October 2026): four of the six
  kinds have chips enough, by BN5's kind of their names (the table above):
  Obstacle 18 of BN6's and 3 of BN5's own, Wood 16 and 11, Break 14 and
  11, Aqua 17 and 14. The thin kinds are ShadowMan's Invisible (Invisibl,
  Mine, AntiDmg, BN5's Blinder) and NumberMan's Plus (six of BN6's). This
  said Aqua and Break were the thin ones, reading BN6's own kinds: BN5
  counts the needles, the bubbles and WideSht Aqua, and AuraHed, GolmHit
  and IronShl Break. Until the last round his Guardian Data gave a chip of
  his Soul's kind; now it gives his own Navi chip, as BN6's guardians give
  theirs ([Guardians and super bosses](../bosses/overview.md)). What a run lacked was the chips in its folder:
  it cannot farm, and its starting folders hold almost none of the six
  kinds, so a won Soul often went unused. A player asked for Souls without
  the chip given up; the owner kept BN5's rule and chose to bring the
  chips instead (25 October 2026, as recommended; [A held Soul's kind](../economy/held-soul-kind.md), A held
  Soul's kind): a quarter of the Mystery Data's chips are of a held Soul's
  kind, each of the tier it rolled, and a Net Dealer's fourth chip is one,
  the kind the folder holds fewest of first (`src/core/loot_souls.c`). The
  counts stay; only which chips come changes, and DarkChips never come so.
  The Souls held as each layer is made are what its finds and stock lean
  to (`souls_layer`, beside the checkpoint), so a Soul won on a layer
  counts from the next and a CONTINUE makes the same stock.
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
