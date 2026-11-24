---
type: Design
title: "Decisions and phase one"
description: "The meta layer's decisions (options only, a short net by default, helpers counting toward unlocks, a setup screen), phase one as built (unlocks, threat rungs 1-5, helpers, the Blade and Storm folders) and the designed Z-Canon, Busting and Lean folders."
tags: [game, meta]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-24T17:28:00Z }
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
| Z-Canon | Cannon A A B B C, CrakShot A x2, TankCan1 A, DublShot C x2, Rflectr1 C x2, AirShot * x2, MiniBomb B x2, AreaGrab B x2, HolyPanl B, Barrier A x2, Recov10 A x2 * x2, Atk+10 * x3, PanlGrab * x2 | GigaCan1 (Cannon A, B, C in a row) waits in every draw; the rest are shots in its codes | The design's few answers behind cover: two MiniBombs its only throw (that its shots stop at the first foe in the row is assumed from play, not measured) | A Program Advance used, any, in any run |
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
  of anything sharp the cost. Z-Canon's Cannons are 40 each, GigaCan1 the
  payoff of a draw.
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

## Issue 865's archetypes (24 November 2026)

The owner's calls of 24 November 2026 on issue #865 put two folders of
ours on the shelf first, Ember and Tide (Breaker after, Kindred only with
Soul runs, Grove cut). Each is held to the designed three's rules above:
BN6's commons in codes its records give, five of a chip at most, a cost
against Standard, and a milestone that teaches the style. Their ids are 9
and 10 (6-8 BN6's presets', 11 kept for Breaker), and a profile's
`folder_feats` bit 0x20 is Ember's; Tide needs none.

BN6's rules they lean on are read in its battle code
([Sources](../../research/sources.md), the archetypes' element rules):
a Fire hit on a body standing on grass adds its damage once more and burns
the grass plain; Aqua hits a Fire body for double (BN6's weakness table:
Fire takes double from Aqua, Aqua from Elec, Elec from Wood, Wood from
Fire); an Aqua hit on a body on ice freezes it and the ice turns plain,
and a Break hit on a frozen body does double; an Elec hit on a bubbled
body does double.

| Folder | Its 30 | What it plays like | What it gives up | Opens |
| --- | --- | --- | --- | --- |
| Ember | GrasSeed F x4 \*, FireBrn1 F x2 G H \*, FireHit1 D E F x2, BurnSqr1 H x2, Cannon \* x2, AirShot \*, MiniBomb \* x2, AreaGrab F x2, Barrier F x2, Recov10 \* x3, Atk+10 \* x2 | Grass laid under a foe, then fire on it for double; F's hands hold both halves. WideBrn1 (FireBrn1 F G H) and FlmHook1 (FireHit1 D E F) in reach. HeatCross's partner: its charge feeds Fire chips | A pick spent on the seed before the payoff, and the grass burns with the first Fire hit; FireBrn1, FireHit1 and BurnSqr1 reach a panel or two ahead, so only the Cannons, the AirShot and the bombs reach the back | A guardian deleted in HeatCross, or Beast Out in it, in any run |
| Tide | TrnArrw1 A x4, BblStar1 T x4, IceSeed A x2 \*, Cannon A x3, CrakShot T x2 A, AirShot \* x2, MiniBomb \* x2, AreaGrab \* x2, Barrier A x2, Recov10 A x3, Atk+10 \* x2 | Storm's counterpart: Aqua in two codes, double on a Fire body (BlastMan, HeatMan, the Fire viruses). A bubble holds a foe for the next shot, an Aqua hit on IceSeed's ice freezes it | One element: everything else takes plain hits, near Standard's; an Elec guardian hits its Navi's weak side as hard as ever | A Fire guardian deleted (HeatMan or BlastMan), in any run |

The game-design reasoning:

- **One dialectic, the meta layer's,** what you bring against what you
  find. Ember brings a two-step turn and finds foes that will not stand
  on its grass; Tide brings one element and finds whatever element the
  net sends. A first act with each plays unlike the six: Ember's opening
  turn is a seed before any damage, Tide's best turns hang on a Fire foe
  or a held one.
- **Costed power, not more of it.** Modelled as Standard is
  ([Starting folders measured](folders-measured.md), Ember and Tide), Tide's
  hands are Standard's and Ember's lie between Standard's and Blade's,
  until a seed lands under the foe: the grass's double is its payoff and
  its whole cost is to set it first.
- **Milestones that teach the style.** Tide's mirrors Storm's: the element
  that its chips answer, met as a guardian (BlastMan, a common act-1
  guardian, makes it a first or second run's). Ember's is the Cross whose
  charge its Fire chips feed: HeatMan falls in act 2, his Cross comes
  with the next run, and a guardian deleted in it opens the folder that
  makes HeatCross a plan. Of the proposal's two, WideBrn1 formed was
  passed over: three FireBrn1 in F, G and H in one hand before Ember
  exists is rarer than Z-Canon's GigaCan1 with its own folder (1 battle in
  100).
- **Old profiles:** Tide reads the guardians' records, so a profile that
  deleted HeatMan or BlastMan has it at its next start. Ember reads the
  form each guardian's last battle was won in (`rivals.sav` keeps it), so
  a profile whose HeatCross win is still that guardian's last has it too;
  from this build the profile keeps the bit once any guardian falls.
- **The code lean** ([Phase two](phase-two.md)) brings finds in a folder's
  codes. Ember's F brings Fire again (FireBrn1, FireHit1, FireSwrd) and
  Vulcan2, WaveArm1, LilBolr1, Recov120; its H brings Sword and WideSwrd.
  Tide's A brings Cannon, TankCan1, Rflectr1 and Elec (DolThdr1,
  ElcPuls2), which a bubble doubles; its T brings MachGun1 and BN6's
  second Fire tier (FireBrn2, FireHit2, BurnSqr2), off its element.
