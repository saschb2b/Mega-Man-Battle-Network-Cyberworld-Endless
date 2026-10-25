---
type: Feature
title: "His messages checked"
description: "How each GBA checks a partner's messages by his screen's rules (`team_check.c`): his picks, Program Advance, Cross and Soul refused alike on every game, the code rule assumed and the bound BN6's screen's ten."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-22T16:53:46Z }
---

# His messages checked

A partner's turn is his own client's: BN6's own Custom screen on a copy
of the pair, so what it says is what BN6 lets him pick. Each GBA still
checks every message as it comes (`team_check.c`, issue #181's review),
so that a client that says what no screen of his could (an edited one)
is refused on every GBA alike, no game told apart. Each check reads only
what every game and spectator holds the same: his deck in the core's
RAM, his MegaMan as every game was given it (`arena_team_custom_allow`:
over the network his loadout, his Crosses, Beast Out and Souls; under
the dev flag what the stood-in screens offer), and the ROM's own lists.

- **His picks**, as they are set for his hand: five at
  most, each a slot of the hand dealt (the first chips of his deck, ten
  at most, BN6's Custom screen's slots) picked once, all of one code
  (`*` any) or all one chip. The Beast Out emblem and UNITE under OK,
  which BN6's screen keeps among its picks as they are chosen
  (`BN6_CUSTOM_EMBLEM`, 11), are passed over, as his hand takes no chip
  of them.
- **His Program Advance**: from the picks it joins (its first pick and
  three or four from there), a recipe of that chip in one of BN6's lists
  his screen's check walks (`BN6_PA_LIST_REF`'s, the Custom screen's,
  and `BN6_PA_LIST_LINK_REF`'s, the link battles': twenty recipes of its
  own, then the first list's): kind 4 its chips in order, kind 0 its one
  chip three times in codes in a row, `*` standing in for one.
- **His Cross**: Gregar's Cross N where his MegaMan holds it, Beast Out
  (in a Cross he holds) where he has it, the Souls' proxy form where he
  holds a Soul; no other form. The proxy form with no Soul said by his
  last pick is no form.
- **His Soul**: one he holds (its bit by BN5's own flag for it,
  `soulunite_held_bit`), not united this battle (`BN6_SOUL_USED`, the
  rule a lead's UNITE keeps); ColonelSoul's chip only with ColonelSoul
  his, this turn's or still on, and a chip of the hand dealt.

A message refused is no pick (his turn's picks none, his Program
Advance with them) or no form, and says so on each core ("side 0's
partner's picks refused: ..."); the end of a battle counts them ("0 of
his messages refused by his screen's rules" in every run below). The
rules are the ones BN6's own screen keeps, as far as read: the code rule
is the series' (assumed: BN6's routine for it not traced), and the
hand's bound is the screen's ten, not his Custom level: BN6's count
(`0x0802A408`) is his Custom level with the turn's extra chips, eight at
most, ten in one case and fewer by another count, which a second copy of
ours could get wrong where his own screen does not (issue #181's
review's ask, not met: the bound is BN6's screen's).
