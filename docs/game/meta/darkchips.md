---
type: Feature
title: "DarkChips"
description: "DarkChips (issue #82): twelve kinds, each played by its own net's rule, offered by a flame of darkness on each act's middle layer, priced at 20 max HP per battle a dark power ran in."
tags: [game, meta]
generated: { by: claude-code/unrecorded, at: 2026-10-13T09:50:43Z }
---

# DarkChips (3 October 2026; in every battle since issue #82)

BN5's DarkChips came into runs with BN5's battles (issue #64, the
owner's decisions): power for a run, at a price the run carries. Until
issue #82's last round they played only in BN5's own engine, on the
guest core ([Guest battles](../../multirom/guest-battles.md)); now every battle is BN6's
engine's, and all twelve kinds play there, each by its own net's rule
([One net: the design](../../multirom/one-net.md), call 6 and the last round).

- **The dialectic** is survival now against strength later: a DarkChip
  wins the battle MegaMan might have lost, and every battle one is used
  in leaves him weaker for the rest of the dive.
- **The pattern** is a bonus with a drawback (costed power forces
  commitment, docs/design: principles): the price is the owner's, "a
  hefty price": each battle a dark power ran in lowers MegaMan's max HP
  by 20, one HPMemory, for the rest of the run, whichever net's chip it
  was. BN5's own -1 is nothing at a run's HP.
- **Twelve kinds, one chip each**: BN6 keeps five of BN5's twelve with
  their code ([BN6's own DarkChips](bn6-darkchips.md)); BN5's other seven are the chip
  registry's (ids 214-220: DarkCirc, DrkLance, DarkWide, DarkMetr,
  DarkDril, DarkTorn, DrkSonic). A kind held is one chip in the Pack, and
  goes into the folder by the PET's EDIT, three DarkChips at most (BN6's
  folder rule; their cards purple).
- **Each by its net's rule**: BN6's four by BN6's (a BugFrag a use for
  its dark power, its base chip without one; below); BN5's own seven by
  BN5's: they come into the hand only while MegaMan worries or is dark,
  BN5's mood brought into BN6's battles ([Transplanted Souls](../../multirom/souls.md), Transplanted
  Souls, S7), and one used makes him dark for the battle (BN6's dark too:
  no Cross, no Beast Out, a weak buster). DarkPlus plays by BN5's rule
  where its ROM is read (issue #82's final engine round: 50 more for the
  next attack chip), and BN5's flame offers it there (the owner's call of
  13 October 2026, below).
- **When he worries** (4 October 2026): BN5 worries MegaMan by hits, 10
  off his mood each, and opened every battle calm, so it took seven hits
  in one battle, whatever his HP: a playtester at 10 of 120 HP never saw
  his DarkChip. A battle opens as worried as his HP says: worried from
  the start at a quarter of his HP or less, else as much calmer as he is
  whole, BN5's hits doing the rest. BN5 deals it into the hand's end the
  next time chips are drawn while he worries; a counter hit's Full
  Synchro calms him first, as in BN5.
- **Where a run gets them:** a flame of darkness on the middle layer of
  each act, holding a DarkChip the run lacks (from the run's seed and the
  act): BN5's flame (Nebula Area's, copied from the player's BN5 ROM as
  its HeelNavi is, and turned purple: the owner's "dark purple ghost
  flame") on the acts BN5's areas dress, offering every kind, DarkPlus
  among them where BN5's code plays it; BN6's own flame elsewhere,
  offering its four, never DarkPlus. MegaMan names the chip, what DarkPlus
  does ("My next attack chip hits 50 harder!", its power the record's),
  when it comes and its price before it is taken, all of it the first
  time a profile meets one (a returning playtester heard only "when I'm
  hurt badly", session 68), the words following the rule the chip plays
  by; then asks, starting on Yes: holding one costs nothing till it is
  used, and a paging A that said No twice (sessions 67 and 68) sent a
  playtester through the whole chat again. Yes gives the chip to the
  Pack, and the flame goes.
- **Counted as each net counts a use:** BN6's where its dark power ran (a
  BugFrag paid); BN5's where it ran by BN5's rule, both where BN6 runs a
  chip's after-effects, once a battle however many ran. A DarkChip given
  up for a Chaos Unison is no DarkChip used (BN5's rule): no max HP for
  it (the owner's call of 4 October 2026).
- **Variety, not power, across runs:** nothing of them carries over, not
  the chips, not their price; a new run starts with none. A run
  continued without BN5's ROM keeps BN5's seven; they sit out of its
  battles until the ROM is back.
- **The owner's calls of 13 October 2026** ("yes to all"; they replace
  the "is the owner's call" lines of issue #82's last rounds):
  1. *DarkInvs:* BN5's own DarkInvs replaces BN6's DarkInvs rule wherever
     BN5's ROM is read: BN5's code in BN6's record 289 (as DarkPlus's
     290), BN5's rule, the game driving MegaMan. BN6's rule (a BugFrag a
     use, Invisibl without one, BN6's flame's words for it) stays only
     where BN5's ROM is absent: a 3DS without it, a run with BN6 alone.
     The price stays as decided: 20 max HP for a battle its dark power ran
     in, once a battle; a DarkChip given up for Chaos Unison costs
     nothing. Built the same day ([The chips' code](../../multirom/chips-code.md),
     DarkInvs, moved): BN5's code moved into BN6's engine, held to BN5's
     battle frame by frame; where BN5's ROM is read DarkInvs comes into
     the hand while MegaMan worries, makes him dark, and for ten seconds
     the game drives him, unseen; its flame's words say so ("For ten
     seconds,the darkness takes over!", then that nothing can touch him)
     with BN5's rule's. Seen in play (`--net-biome x0 --dev worried,god`,
     `chip 289 * 1`): the game fired his buster and threw his MiniBomb, the
     Mettaur fell, max HP 140 to 120 once, the next battle his own.
  2. *DarkPlus:* offered by BN5's flame of darkness, by BN5's rule (50
     more for the next attack chip), never by BN6's flame: BN6's US
     DarkPlus does nothing, and offering it there would be a trap. Built:
     BN5's flame offers it where BN5's code plays BN6's record 290
     (`dark_kinds_offered`), its pick from the run's seed and the act as
     before, DarkPlus where it falls; MegaMan says what it does; the Pack
     lists it with BN5's own art (`src/core/chips.c`, `darkbn6_records`)
     and EDIT puts it in. Seen in play (`tools/play.py`, seed 1024,
     `--net-biome x0 --run-depth 2 --dev worried,god`): the flame's words,
     DarkPlus taken to the Pack and swapped into the folder, dealt by
     BN5's worried rule, the next Cannon "40+50", 100 max HP to 80 after
     the battle, once.
- **Gone with the guest core**: BN5 got MegaMan up at 1 HP after a fall in
  a battle a DarkChip was used in, the darkness fighting with his body
  a while (session 68), and MegaMan said so; BN6's engine has its own
  dark, and that rise does not come with it.
