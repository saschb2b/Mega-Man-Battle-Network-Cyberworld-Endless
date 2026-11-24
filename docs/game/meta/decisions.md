---
type: Design
title: "Decisions and phase one"
description: "The meta layer's decisions (options only, a short net by default, helpers counting toward unlocks, a setup screen), phase one as built (unlocks, threat rungs 1-5, helpers, the Blade and Storm folders) and the designed Z-Canon, Busting and Lean folders."
tags: [game, meta]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-24T22:30:00Z }
sources:
  - id: owner
    resource: "The owner's decisions, given in the project's sessions and written down here"
    title: "The owner's decisions"
    author: human:saschb2b
---

# Decisions and phase one

## Decisions (28 September 2026)

1. **Stance: options only.** Unlocks add folders, Crosses, routes and
   threat rungs; MegaMan's strength resets every run.
2. **Run length: a short net by default.** Three acts and the Nest, about
   an hour; the endless cycle unlocks after a first clear.
3. **Helpers count toward unlocks.** They are an accessibility dial; the
   summary names them.
4. **Phase one starts with a setup screen** after NEW GAME, in the PET's
   style; the town hub comes later.

## Phase one, in order (built)

1. Returning players hear less: the arrival's words, the town's opening
   and the gift's offers shorten after the first runs (a playtester called
   a restart after a long run a chore).
2. The profile's unlocks and the run's setup (folder, threat, helpers,
   short or endless), saved with the run (`RUN_MAGIC` changes once).
3. The short net: three acts and the Nest, paced in [Progression](../progression/overview.md)'s
   bands; the endless cycle once the Nest has fallen.
4. Threat rungs 1 to 5 and three helpers, on the engine's pacing knobs.
5. Starting folders: Standard and two to unlock, written into the game's
   folder at the run's start (its RAM layout from bn6f, noted in
   [ROM data](../../rom/overview.md)).
6. The setup screen after NEW GAME, and the summary's line for what a run
   unlocked and what is closest.

Built as planned, with these particulars:

- The setup screen is skipped on a profile's very first run, and remembers
  the last run's choices. A run given up for a NEW GAME counts no run and
  has no summary, but what it opened opens the setup, marked NEW: a first
  run that deleted ElecMan, given up, had jacked in with no setup, so
  neither his Cross nor the Blade folder could be chosen.
- The two folders to unlock are Blade, once any guardian falls, and
  Storm, once an Aqua guardian (DiveMan, SpoutMan) does, in any run. They
  first opened with SlashMan and ElecMan, whom the net brings in act 3 or
  the Nest: a playtester's setup offered nothing to choose until a first
  win. The milestones now come in the first or second run and still
  teach the folder's style (Storm's Elec answers Aqua). BN6's five other preset folders ([ROM data](../../rom/overview.md)) wait for later
  phases: one is a Program Advance folder, one a Navi-chip folder.
- The design's other three, Z-Canon, Busting and Lean, came on 26
  October 2026 ([the core loop](../core-loop.md), Phase 2: folders are
  the meta axis players name most, and of 251 runs the statistics saw
  135 brought Standard, 76 Storm, 40 Blade). Below.
- The helpers are two more HPMemory from the gift, a heal on every layer,
  and gentle battles (the lower half of the band all run); since issue
  #18 a fourth, All *, every chip in * (below).
- `--setup NET,FOLDER,THREAT,HELPERS` sets a headless run's (HELPERS a bit
  each: 1 HP+, 2 Heals, 4 Gentle, 8 All *, 16 BeastOut); the weak
  autopilot wins a short net (layers 1-10).
- Since 5 November 2026 the setup is three pages under tabs, RUN, HELP
  and EXTRAS, the last holding what the run adds to BN6, each on or off
  ([the extras](extras.md)).

## The designed three (26 October 2026)

Each a different way to play, none a stronger one (the design's stance):
its chips are BN6's commonest, in codes BN6's records give them (read at
`0x021DA8` + id x 0x2C, [ROM data](../../rom/overview.md); `test_folders`
holds each folder to them, five of a chip at most), and each gives up
something Standard keeps.

| Folder | Its 30 | What it plays like | What it gives up | Opens |
| --- | --- | --- | --- | --- |
| Z-Canon (rebuilt 24 November 2026, below) | Cannon A A B B C, FireBrn1 F G G H H, FireHit1 D D E E F, Sword S, WideSwrd S, LongSwrd S, AreaGrab \* x2, Barrier \* x2, Recov10 \* x4, Atk+10 \* x4 | Three Program Advances of codes in a row (GigaCan1, WideBrn1, FlmHook1) and LifeSrd's three once each; a hand holds a recipe's part or two and waits for the rest | Reach and codes: FireBrn1 and FireHit1 hit close, nothing throws over cover, and nine codes split its hands where Standard's five join | A Program Advance used, any, in any run |
| Busting | TankCan1 R x4, HiCannon L x3, WideSwrd L x3, Sword L x3, MiniBomb L x3 R x2, Thunder R x3, Barrier R x2, AreaGrab * x2, PanlGrab *, Atk+10 * x4 | The heaviest chips act 1 allows, in two codes; Thunder holds a foe for TankCan1's wind-up | No Recovery chip: every heal is the net's (a Mr.Prog's, a find's). No LongSwrd, so LifeSword stays Blade's | An act cleared with no Recovery chip used in it (its guardian deleted; the Secret Area's master ends no act) |
| Lean | Cannon B x5, MiniBomb B x5, Vulcan1 B x5, Thunder B x5, AreaGrab B x4, Atk+10 * x3, Recov10 * x3 | Seven kinds, all in B or *: any five chips make a hand, every turn | Every chip weak and near the same; and the code lean (phase two) brings its finds in B too, so a strong find off B breaks the hands it is built on | Layer 10 reached in any run (the best depth, so a veteran's profile has it) |

On the setup all six stand on one shelf, the locked ones with a hint at
their milestone rather than the milestone itself, which a rumor in the net
or the BBS's late post tells ([The shelf of starting folders](shelf.md), 24
November 2026).

The game-design reasoning, the part the design left open:

- **The dialectic** is the meta layer's, what you bring against what you
  find: Z-Canon brings a combo and finds cover; Busting brings damage and
  finds its heals in the net, which sharpens the run's prepare or press on
  (a Mr.Prog's heal is worth a detour); Lean brings whole hands and finds
  chips that would break them, a loadout as budget where the budget is
  the code.
- **Costed power, not more of it.** Standard's Recov10s heal 40 HP a
  battle at most, so Busting's cost is in its chips too: TankCan1's
  wind-up, Sword and WideSwrd's reach, and no LongSwrd. Lean's chips are
  Standard's weakest kinds; its five-chip turns are the power, its lack
  of anything sharp the cost. Z-Canon's parts are 40 to 70 each, a
  Program Advance (300) the payoff of a hand held for it.
- **Milestones that teach the style** (the design's rule, never a count):
  a Program Advance is what Z-Canon is for, a dry act is Busting's whole
  run, and layer 10 is the middle of the net, where Lean's weak chips
  begin to need the finds it leans.
- **Measured:** the statistics' `folder` (the run's start event) names
  each; a folder nobody brings needs a reason to exist (the design's
  Measuring it).

Two milestones are seen in battle: BN6's chip use's last step
(`BN6_DARK_AFTER`, `src/director/darkbn6.c`, its one hook there) reports
MegaMan's Program Advances and Recovery chips (`meta_chip_watched`) to
`meta_chip_used`, which keeps them in the profile (`folder_feats`, and the
act of the last Recovery chip with its run's seed); a guardian deleted
asks `meta_guardian_deleted` whether its act had one. Checked in the game
with a folder of GigaCan1 and one of Recov10 (`--dev folder=320`,
`folder=154`): the profile's bit and act written as each ran. `--dev
folders` opens every folder for a capture of the setup
([the dev menu](../../devtools/dev-menu.md)).

## Each game's start (24 November 2026)

The owner's call on issue 865: BN5's and BN4's NEW GAME folders
([Starting folders measured](folders-measured.md), 3) are not shelf
entries. With FORM on SOUL BN5 or SOUL BN4 the Standard folder is that
game's start, read from its ROM; the other folders are the same in every
form. Built in [MegaMan's forms](forms.md), Each game's start.

## Z-Canon rebuilt (24 November 2026)

The first Z-Canon (Cannon A A B B C and shots in their codes) almost
never showed what it was for: GigaCan1 formed in 1 battle in 100, and its
hand was the weakest of the six ([Starting folders measured](folders-measured.md)).
The owner's call on issue #865 rebuilt it from BN6's own Program Advance
preset (its fifth, given by a Soul Injection's script) one tier down.

- **What BN6's preset holds:** M-Cannon R S T, FireBrn2 S T U, FireHit2
  R S T (GigaCan3, WideBrn2, FlmHook2), Sword, WideSwrd and LongSwrd S
  three times each (LifeSrd), GrasSeed \* x3, AreaGrab S x2, BusterUp \*
  x4, Roll R, Roll2 R, Roll3 R: twice Standard's hand.
- **Taken a tier down:** Cannon A B C (GigaCan1), FireBrn1 F G H
  (WideBrn1), FireHit1 D E F (FlmHook1), each recipe checked in BN6's
  table (`0x02BA60`: kind 0, one chip three times in codes in a row) and
  each code in the chips' records (`0x021DA8`). Two of every code but the
  last of each line, so five of a kind stay the most, and a hand holding
  two parts finds the third more often.
- **Left out of the preset:** the GrasSeeds (grass then fire is the
  Ember sketch's, an archetype of ours, so the two stay apart), Roll's
  three (a Navi chip a folder may hold once; Roll alone is no recipe),
  BusterUp, and LifeSrd's nine swords cut to three: with nine in S, LifeSrd
  formed in 5% of battles and the folder played as Blade with fire. In
  their place the support Standard keeps, in \* so any recipe's part makes
  a hand with them: Recov10 x4, Atk+10 x4, Barrier x2, AreaGrab x2.
- **Measured** (`tools/folder_hands.py`, 4000 draws): turn 1 113 (Standard
  128), three turns 324 (303), a Program Advance in 9 battles in 100 (the
  first Z-Canon 1): GigaCan1 3.6, FlmHook1 2.5, WideBrn1 2.4, LifeSrd
  0.3. Under the greedy pick that holds nothing back; a player who keeps
  his parts forms more.
- **Through the lens.** The dialectic stays the meta layer's, what you
  bring against what you find: Z-Canon brings a combo and finds reach.
  Its cost is now honest in the hand rather than in rarity: nine codes,
  so a turn spent waiting for a recipe's last part is a turn of small
  hits, and its close chips leave a foe behind cover to the Cannons. A
  promise of a combo, not a habit: one battle in eleven, which the shelf's
  words say ("Three Program Advances", never "every draw").
