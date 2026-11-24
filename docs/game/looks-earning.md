---
type: Design
title: "MegaMan's palettes, earned"
description: "How a profile earns MegaMan's palettes (epic #854): six by milestones it already records, three at AsterLand's counter for 6000 zenny each, two as a rare find one run in a while by the Spins' rule; each ends in looks_grant, MegaMan's word and BN6's got box, and Dad's mail the first time."
tags: [game, meta, looks]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-24T16:47:10Z }
sources:
  - id: owner
    resource: "The owner's calls on issue 854, 24 November 2026"
    title: "MegaMan's looks: decided"
    author: human:saschb2b
  - id: economy
    resource: "python3 build.py economy, 300 runs, 24 November 2026 (the opening plan in play, 0 outside its bands before and after)"
    title: "The economy report"
---

# MegaMan's palettes, earned

A profile's MegaMan wears colours of its own (epic #854): channels on his
plain body, each coloured from a palette the profile owns. Classic is
free. The other eleven are earned three ways, each ending in
`looks_grant` (`src/core/looks_routes.c`). Looks only: nothing here
changes how a run plays, and the week's net earns none
([the week's net](meta/weekly-net.md)).

## The design

- **The dialectic** is the run's own, *spend now or keep for later*, here
  in its meta costume: this run's power (a chip at the Net Dealer, a Mega
  order at home) against something the profile keeps and shows. A
  palette is a trophy you wear, as [the title's marks](meta/title-marks.md)
  are trophies you see: never a key, never power.
- **The loop layer** is the meta's (what a run leaves for the next), fed
  by the session's (the act's zenny at home) and the layer's (a Mystery
  Data on a far detour).
- **Patterns**: achievement cosmetics (a milestone named in its reward),
  a cosmetic sink in the run's own currency (no meta currency, ever: the
  [core loop's review](core-loop.md)), and discovery over schedule, the
  [Spins'](meta/spins.md) rule for the finds: one a run at most, never one
  the profile owns, not marked or announced.
- Each palette is named for where it was earned, so a look tells where a
  player has been.

## Milestones: six

Each is granted at the moment the profile records it, and checked every
frame on the map (`looks_milestones`, a handful of comparisons): a
profile from before palettes gets the ones it holds at its next start, and
MegaMan says so there.

| Palette | The milestone | Its record |
| --- | --- | --- |
| Undernet | A run reaches the Undernet | No record: granted as an Undernet layer begins (the endless net's fifth act, or the dark warp's side layer); a profile's best dive past layer 12 for one from before |
| Gregar | The Cybeast's fall at the endless Nest | Bass's mark, or a Nest cleared past the short nets' wins (as [BeastOut](meta/beastout-cybeast.md) reads it) |
| ProtoMan | Chaud's respect | His second clearance, three duels won: the netbattle with ProtoMan won, the rivalry's top rung ([the rival](rival.md)) |
| Nebula | BN5's Darkloids | One of the four deleted (rivals.sav) |
| Tournament | BN4's tournament won | No record: granted as its final's Navi falls; a profile that won one before palettes earns it at the next win |
| Hazard | The top threat rung cleared | The title's disc |

A palette earned as a run ends (the disc's Hazard) is named on the
summary ("Unlocked: the Hazard colors") and said at the next start.

## The home shop: three

AsterLand's clerk ([the home shop](../world/home/home-shop.md)) sells
Central Town, Seaside and Sky beside his orders while the profile lacks
one, in BN6's own chat box: his greeting, then BN6's options, "Order a
chip" (his Order Service as before, one order a visit) or "MegaMan's
colors" (the ones not owned), his Yes or No with the price, BN6's own
zenny check (`ts_check_take_zenny`), and "MegaMan got:". A sale sets
`LOOKS_SOLD_FLAG` + its place (`0x14DA`-`0x14DC`), which the director
grants at once; the same palette asked for again that visit gets "Those
are yours now". A profile that owns all three hears his orders alone.

**The price: 6000 zenny each** (`LOOKS_SHOP_PRICE`). Measured against
what a run earns by act (the economy report, 300 runs, the opening plan):

| Act | Zenny earned in it | 6000 is |
| --- | --- | --- |
| 1 | 2,840 | two acts' worth |
| 2 | 3,700 | (with act 1's) nearly all of both |
| 3 | 7,070 | 85% |
| 4 | 8,190 | 73% |
| 5 | 11,980 | half |
| 6 | 10,370 | 58% |

So it is a choice wherever it is met: early, the act's answer chips and
HPMemory against saving for a look; from act 3, against a Mega chip's
order (8000) or a common one (1000) at the same counter. Three palettes
are 18,000 zenny, a run's surplus over its wants by act 4, so a profile
buys them over runs, never all in its first. The report measures what a
run hands out, not what it spends: the plan's numbers are unchanged, 0
outside their bands before and after, and no band's centre moves.

## The rare find: two

One a run at most, never one the profile owns (`looks_find_palette`):

- **Glitch**, where the run's seed gives it a [Glitch layer](glitch-layer.md)
  and the profile lacks it: in that layer's best blue Mystery Data.
- **Mystery** otherwise, where the profile lacks it: in the best blue
  Mystery Data of one normal layer from the seed, an act's first or last
  of 4 to 7 (`looks_find_depth`; never an act's middle, which may be its
  dark layer, nor the Spin's layer).

The data's content is rolled as any other's and then swapped for
ColrData, a key item of the run's own (`0x4D`, named in the PET's names
table), so a profile that owns the palette finds the layer otherwise the
same: no other Mystery Data's roll moves (`LAYER_MAKE` 177). Picked up,
BN6's own "MegaMan got: ColrData" box, then MegaMan names the palette.
The profile keeps the run that found it (`look_find_run`), so a
checkpoint from before the find gives it again, never a second.

## The moment

A palette got is said at the first calm moment on the map (no chat box,
the player's keys free, half a second; one got in a layer's first five
seconds, as the Undernet's is, waits for home, where no board's talk or
guardian's staging is due), by MegaMan in a box of his own
naming where it came from ("We beat the beast,Lan! I kept its colors!"),
then BN6's "MegaMan got:" box naming it (`src/director/looks_words.c`).
Several at once (a profile's from before palettes) are one word and a box
each. The shop's sale is its own got box. `profile.looks_told` keeps
what has been said.

The first palette past Classic brings Dad's mail, **Colors** (mail 24,
NEW once): where MegaMan changes his colours, Colours in the PET's start menu ([MegaMan's look](looks.md)), and that
AsterLand sells some ([the PET](pet.md), E-Mail).

## Checks

`tests/scenarios/looks_routes.py`: a milestone granted once (ProtoMan,
`--dev duels=3`), the Mystery and the Glitch finds picked up, and a run
whose profile owns Mystery holding none; `tests/scenarios/looks_shop.py`:
a sale, too little zenny, and the clerk's orders alone once all three are
owned. `--dev looks=MASK` sets the palettes owned and said.
