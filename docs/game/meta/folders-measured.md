---
type: Analysis
title: "Starting folders measured"
description: "Issue 865's groundwork: the six starting folders measured (hands drawn, the autopilot's runs), BN6's five other preset folders and BN5's and BN4's starting folders read from their ROMs and judged as a run's start, and five archetypes of ours sketched; numbers for the owner's calls, no folder changed."
tags: [game, meta, folders, measurement]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-24T19:30:00Z }
sources:
  - id: roms
    resource: "BN6 Gregar's, BN5 Team Colonel's and BN4 Red Sun's ROMs, read by tools/folder_hands.py"
    title: "The three ROMs"
  - id: bn6f
    resource: "bn6f's disassembly: GiveFolder, FolderTable and the text scripts' ts_start_give_folder"
    title: "bn6f"
  - id: mmkb
    resource: "https://megaman.miraheze.org/wiki/Folder"
    title: "MMKB, Folder: each game's starting folder"
  - id: autopilot
    resource: "The host build's autopilot over seeds 1-12 per folder, 24 November 2026"
    title: "Autopilot runs"
---

# Starting folders measured

Groundwork for issue 865 (the starting folders evolved): what the six
folders of [Decisions and phase one](decisions.md) do today, what the
other two sources the owner named hold (BN6's own other presets, BN5's
and BN4's starting folders), and five archetypes of ours as sketches.
Nothing here changes a folder; the proposal built on it is on the issue.
The folders' design rules are in [the meta layer's design](design.md),
1. Starting folders.

## How it was measured

- **Hands.** `python3 tools/folder_hands.py` draws each folder 4000 times
  through a model of the Custom screen: five chips a turn, a chip not
  picked kept for the next turn, each turn's pick the legal set (one
  chip's copies, or one code with `*` among it) of the most damage plus
  half its healing, a Program Advance of BN6's table formed where its
  parts are in the pick. Power is the record's (+0x1A), one hit: a
  Vulcan's hits, a TimeBom's delay, range and foes are not modelled, so
  Vulcan and trap folders read low. It measures what a folder hands the
  player, not how well he plays it.
- **The autopilot.** The host build's autopilot (`CYBERWORLD_AUTOPILOT=1`)
  played each folder from `--scene town` with `--setup short,F,T,0`, a
  fresh data folder a seed, 90000 frames, seeds 1-12 at threat 1 (72 runs, about 40 minutes on six cores); threat
  3 and 5 only for the runs that passed act 1 at threat 1, because the
  rungs change nothing before act 2 (`pacing.c`'s extra version and
  `loot.c`'s from `pacing_act(depth) >= 1`; the shops' from threat 3 are
  not in the autopilot's path), so an act-1 death replays frame for frame
  at every rung (seen: seed 1's six folders identical at threats 1, 3
  and 5). A local logging change, not kept, wrote each chip MegaMan used
  and each battle's frames into the run log. The autopilot dies at the
  act-1 guardian most runs (its baseline, about one guardian in ten), so
  its numbers rank the folders for act 1 and say little past it.
- **The players.** Not read: the anonymous statistics are on the owner's
  Umami, which needs his signed-in browser, and it was signed out. What to
  pull is listed under The players, below.

## 1. The six as they are

### Hands

| Folder | Kinds | Codes (not `*`) | In `*` | Attack power in the 30 | Heal in the 30 | Turn 1: damage, chips | Three turns: damage, heal used, chips, Program Advances |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Standard | 12 | 5 | 8 | 1030 | 40 | 128, 2.5 | 303, 4, 6.5, 0.00 |
| Blade | 8 | 2 | 11 | 1100 | 50 | 183, 3.0 | 388, 14, 6.9, 0.11 |
| Storm | 12 | 2 | 11 | 830 | 40 | 122, 3.4 | 310, 13, 8.9, 0.00 |
| Z-Canon | 13 | 3 | 9 | 700 | 40 | 93, 2.7 | 225, 12, 6.4, 0.01 |
| Busting | 10 | 2 | 7 | 1630 | 0 | 204, 3.1 | 555, 0, 8.3, 0.00 |
| Lean | 7 | 1 | 6 | 700 | 30 | 122, 4.3 | 316, 13, 11.2, 0.00 |

For scale: act 1's virus battles hold 40 to 160 HP of foes (the run
log's `foehp`), its guardian 500 (seed 1's, g13).

- **Busting is the strongest hand by far:** 1.6 times Standard's first
  turn and 1.8 times its three, its cost (no Recovery chip) paid in HP,
  not in damage. A dominant-strategy watch item, measured below.
- **Z-Canon's fantasy almost never shows:** GigaCan1 forms in 1 battle in
  100 under the greedy pick. Its parts are Cannon A, A, B, B, C: all three
  codes are among the first five chips 0.9% of the time, among the first
  ten 9%, among fifteen 27% (an upper bound for a player who holds his
  Cannons). Six Cannons (A, A, B, B, C, C) would make it 1.7%, 16% and
  42%; any codes-in-a-row Program Advance stays rare in a 5-chip hand.
  Its hand is also the weakest of the six (93).
- **Lean** draws the most chips (4.3 a turn) and lands Standard's damage
  with them; **Storm** draws more than Standard for the same damage;
  **Blade** forms LifeSrd in 11% of battles (Sword, WideSwrd, LongSwrd
  all in S).

### The autopilot

Threat 1, seeds 1-12 each (72 runs); the act-1 guardian counted where
the run reached him:

| Folder | Layer reached, mean (each seed) | Act-1 guardian won | Deaths: guardian, virus battle | Act-1 virus battles: frames, chips used, HP lost |
| --- | --- | --- | --- | --- |
| Standard | 3.4 (3 3 3 6 3 5 3 3 3 3 3 3) | 2 of 11 | 10, 2 | 549, 3.4, 4 |
| Blade | 3.8 (3 2 5 6 3 3 3 5 6 3 3 3) | 4 of 10 | 8, 4 | 1103, 8.0, 10 |
| Storm | 4.8 (3 3 5 6 6 6 5 5 6 5 5 3) | 9 of 12 | 7, 5 | 502, 4.8, 3 |
| Z-Canon | 3.7 (3 3 3 6 3 3 6 5 3 3 3 3) | 4 of 12 | 10, 2 | 589, 4.3, 6 |
| Busting | 4.5 (3 3 5 2 8 6 6 6 6 3 3 3) | 6 of 11 | 9, 3 | 354, 3.2, 7 |
| Lean | 5.1 (6 6 5 6 6 3 5 5 3 5 5 6) | 10 of 12 | 7, 5 | 371, 4.7, 4 |

At threats 3 and 5 the 34 runs that passed act 1 at threat 1 were played
again: every one reached the same layer and died the same way at both
rungs (Standard 2 runs, Blade 4, Storm 9, Z-Canon 3, Busting 6, Lean 10;
all ended at layers 5-6, act 2's viruses or its guardian). In these runs
the rungs from 1 to 5 made no difference the autopilot met; where they
bite (act 3 on) it never got.

- **The folder decides the autopilot's act 1.** Seeds give each folder
  the same areas and guardians, and the act-1 guardian falls to Lean 10
  times in 12, Storm 9, Busting 6, but Standard 2 in 11 and Z-Canon 4.
  The autopilot picks the largest legal hand: Lean (one code) and Storm
  (two) hand it four to five chips a turn, Standard's five codes three.
  A player builds hands better than it does, so this overstates the gap;
  but it says which folders give whole hands without thought, and that
  Standard is not the easy folder it is meant to be.
- **Blade's battles take twice as long** (1103 frames, 8 chips, 10 HP
  lost against about 500 frames elsewhere): its swords need MegaMan in
  reach, which the autopilot walks into, taking hits.
- **Busting kills fastest** (354 frames) and pays in HP (7 a battle, no
  Recovery chip), as designed; it reached layer 8 once, the deepest run.

Chips the autopilot used most in act 1, by share of its chip uses:

| Folder | Most used |
| --- | --- |
| Standard | Cannon 29%, Sword 14%, Recov10 10%, CrakShot 9%, MiniBomb 8%, AirShot 8% |
| Blade | Sword 23%, WideSwrd 19%, AreaGrab 15%, LongSwrd 11%, Barrier 9%, Recov10 9% |
| Storm | Thunder 20%, Cannon 13%, Vulcan1 13%, AirShot 9%, Recov10 9%, MiniBomb 7% |
| Z-Canon | Cannon 21%, Recov10 10%, CrakShot 9%, PanlGrab 9%, DublShot 9%, Rflectr1 6% |
| Busting | TankCan1 26%, AreaGrab 15%, HiCannon 14%, WideSwrd 9%, MiniBomb 9%, Thunder 8% |
| Lean | Cannon 26%, Vulcan1 20%, AreaGrab 17%, Thunder 15%, MiniBomb 13%, Recov10 6% |

Each folder's act 1 is its own by what it fires (Storm's Thunder,
Busting's TankCan1, Blade's swords), Standard and Z-Canon the nearest
pair (Cannon first, then the rest); Z-Canon's GigaCan1 did not appear in
the top six. The Program Advances it formed agree with the hand model: Z-Canon's GigaCan1 once in 66 battles (the model's 1%), Blade's LifeSrd
7 times in 73 (the model's 11%); no other folder formed one.

## The players (to read)

`run-start` and `run-end` both carry the setup (`folder`, `threat`,
`helpers`, `cross`, `form`; `src/analytics/analytics.c`, setup_json);
`run-end` adds `won`, `layer` and `by`; `guardian` events carry the
guardian, `result` and `layer`. Since 26 October 2026 (when Z-Canon,
Busting and Lean came), per folder:

1. runs started (`run-start` count), and by threat rung;
2. runs ended, won (`run-end` with `won` true), and the share;
3. the layer reached at `run-end`: the mean, and how many ended in act 1
   (layers 1-3), act 2 (4-6), act 3 (7-9), the Nest (10);
4. what ended the lost ones (`by`, the top five);
5. the act-1 guardian's `guardian` events for runs of that folder, won
   and lost (joined by the run's session, if the events allow).

The question they answer: is any folder brought and never kept (the
design's "subtract"), and does Busting's hand show as a higher win rate.

## 2. BN6's five other presets

BN6 holds six preset folders in Gregar's `BN6_FOLDER_TABLE`
([Chips, folders and the Library](../../rom/chips.md), Chip folders): the
first is the NEW GAME folder, Standard; the other five are given by its
story (who gives each, from bn6f's scripts, read, not played). Their 30:

| Preset | BN6 gives it | Its 30 |
| --- | --- | --- |
| 1, "Extra" | A qualifier's prize | Cannon A B C \*, HiCannon L x3, Vulcan2 D x3, EnergBom K x4, Sword S x3, WideSwrd S x2, LongSwrd S, DublShot \* x2, Recov50 C x4, AreaGrab S, BusterUp \*, Atk+10 \* x2 |
| 2, Folder2 | With the Navi Customizer | Cannon A x2 B x2, AirShot \* x4, Vulcan1 D x4, MiniBomb L x4, Sword S x4, WideSwrd S x2, CrakShot \* x3, Recov10 A x3, Atk+10 \* x2 |
| 3, Navi | Transmitted in a script that names Mr.Famous | WideSht P x3, BblStar2 C x3, DolThdr2 C x3, IceSeed \* x2, Sword \*, WideBlde B x2, LongBlde B x2, VarSwrd V x3, GolmHit2 P x3, AreaGrab \*, ComingRd \*, WhiCapsl \* x3, BlastMan EX B, DiveMan EX D, CircusMan EX C |
| 4, Support | Transmitted in another such script | GunDelS1 C x4, GunDelS2 R x3, WaveArm1 F x3, AirRaid1 R x3, Boomer J x3, TimeBom1 F x3, Fanfare \* x3, Recov80 \* x4, Invisibl \* x3, Barr100 O |
| 5, Program Advance | A Soul Injection's script | M-Cannon R S T, FireBrn2 S T U, FireHit2 R S T, Sword S x3, WideSwrd S x3, LongSwrd S x3, GrasSeed \* x3, AreaGrab S x2, BusterUp \* x4, Roll R, Roll2 R, Roll3 R |

| Preset | Kinds | Codes | In `*` | Attack power | Heal | Turn 1: damage, chips | Three turns: damage, heal, chips, PAs | Against Standard |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | 12 | 7 | 6 | 1285 | 200 | 148, 2.3 | 344, 28, 5.9, 0.03 | 1.15x damage, 5x the heal |
| 2 | 9 | 5 | 9 | 1140 | 30 | 145, 3.0 | 356, 2, 7.6, 0.00 | Standard with fewer kinds |
| 3 | 15 | 5 | 8 | 3095 | 0 | 279, 2.5 | 773, 0, 6.5, 0.01 | 2.2x turn 1, 2.6x three turns |
| 4 | 10 | 5 | 10 | 1020 | 320 | 138, 2.0 | 277, 87, 4.5, 0.00 | Traps and heals (its GunDelS and TimeBom unmodelled) |
| 5 | 12 | 4 | 7 | 2160 | 0 | 253, 2.6 | 635, 0, 6.7, 0.11 | 2.0x turn 1, 2.1x three turns |

Each as a run's start:

- **1, Extra.** Standard's shape with every chip a step up: HiCannon (100)
  for Cannon, Vulcan2, EnergBom, a LongSwrd, and four Recov50 (200 HP of
  heals against Standard's 40). Plays like Standard with a cushion; gives
  up nothing but codes (seven, so its hands are the thinnest, 2.3 chips).
  For act 1 it is a sustain folder, not a damage one. Fair after a trim:
  the Recov50s to Recov30 or two of them out; a later milestone than
  Blade's.
- **2, Folder2.** Standard with BusterUp, WhiCapsl and the second
  Recov10's code gone and a fourth Sword, AirShot and Vulcan: within 15%
  of Standard on every count. It would not play unlike anything on the
  shelf; not worth a slot.
- **3, Navi.** Three Navi chips (BlastMan EX 140, DiveMan EX 150,
  CircusMan EX 25 with its stun), rarity-3 blades (WideBlde, LongBlde 150
  each, LifeSrd's other recipe with the Sword \*), VarSwrd 160, DolThdr2
  150, GolmHit2 190. Its turn 1 alone (279) is half the act-1 guardian;
  every chip but six is above an act-1 find. Far above a start's level:
  as is, an act-3 milestone at best, or rebuilt around its idea (a few
  Navi chips in a support shell, their V1 chips in place of EX) where its
  hand comes near Standard's.
- **4, Support.** The one that plays unlike every folder of ours: summons
  and traps (GunDelS, AirRaid, Boomer, TimeBom), Invisibl and four
  Recov80. Its damage is delayed or placed, so its row reads low; the
  autopilot cannot play it (it picks for damage now). Nearest to a new
  archetype taken from Capcom whole; its cost is slow kills. Needs a
  playtest by hand before a call.
- **5, Program Advance.** Built for Program Advances: GigaCan3 (500, from
  M-Cannon R S T), WideBrn2 (350), FlmHook2 (350), LifeSrd (400, its
  three parts three times in S), and Roll's three in R. Its parts alone
  outhit Standard twice over, and LifeSrd forms in 11% of battles. As is,
  an act-3 folder. At act 1's level its idea is the same recipes one tier
  down (Cannon A B C, FireBrn1 F G H, FireHit1 D E F, Sword, WideSwrd,
  LongSwrd S): modelled, 125 on turn 1 (Standard's 128) and a Program
  Advance in 3% of battles, which is Z-Canon's place on the shelf.

## 3. BN5's and BN4's starting folders

Read where each game's GiveFolder finds its folder 0 (BN5
`BN5_FOLDER_TABLE`, BN4 `BN4_FOLDERS`; [Chips, folders and the
Library](../../rom/chips.md), Other games' folders), and the same chip for
chip as MMKB's Folder page lists them.

| Chip | BN6 (Standard) | BN5 Team Colonel | BN4 Red Sun | In a run |
| --- | --- | --- | --- | --- |
| Cannon | A x2, B x2 | A x2, B x2 | A x2, B x2 | BN6's |
| AirShot | \* x2 | \* x3 | A x3 | BN6's; its record has \* alone |
| Vulcan1 | D x3 | D x3 | V x3 | BN6's; its record has B D S \*, no V |
| MiniBomb | B x2, L x2 | B x2, L x2 | B x2, L x2 | BN6's |
| Sword | S x4 | S x4 | S x4 | BN6's |
| WideSwrd | S x2 | S x2 | S x2 | BN6's |
| CrakShot / CrakOut | CrakShot \* x2 | CrakOut \* x2 | CrakOut \* x3 | CrakOut moved (id 298, BN5's registry row; BN4's paired to it by name) |
| Recov10 | A x2, L x2 | A x2, L x2 | A x2, L x2 | BN6's |
| AreaGrab | S | S | S | BN6's |
| BusterUp | \* | \* | none | BN6's |
| Atk+10 | \* x2 | \* x2 | \* x2 | BN6's (BN5's Attck+10 an alias) |
| WhiCapsl | \* | none | none | BN6's |
| Hand: turn 1, three turns | 128, 303 | 113, 266 | 104, 245 | |

- **BN5's start is playable today, whole:** every chip BN6's own or moved
  (CrakOut), every code one its record holds.
- **BN4's is not, by six codes:** AirShot A x3 and Vulcan1 V x3, codes
  BN6's records lack (a chip given in a code its record lacks is counted
  in its first code, [Chips, folders and the Library](../../rom/chips.md),
  The All * helper). Taken as is it would silently become AirShot \* and
  Vulcan1 B. Moving it whole means a BN4 AirShot and Vulcan1 of their own
  (registry rows with BN4's codes, a unit of the registry's work, each
  one more chip the loot can roll), or the swap named in the shelf's hint
  (AirShot \*, Vulcan1 S).
- **Both are Standard within three slots.** CrakOut breaks a panel where
  CrakShot throws it; BN4's has no BusterUp. Neither plays unlike
  Standard in act 1 (their hands a little weaker, 0.88 and 0.81 of it).
  As shelf entries they fail the issue's own test (a first act unlike
  every other's); as the start a SOUL BN5 or SOUL BN4 run brings in place
  of Standard they are a fidelity touch at no balance cost.
- **What else they hold.** BN5's table has five more (its story's extra
  folders: a stronger start like BN6's preset 1, a tutorial one, two
  Mega-chip folders and a dark one of DrkSword, DarkInvs, DarkThnd and
  three DS Navi chips), many of their chips in codes BN6's records lack
  (`tools/folder_hands.py --list` names each); BN4's eleven more are its
  own folders for its battles, several holding 6 or 12 of a chip. None
  was asked for; named for the owner.

## 4. Archetypes of ours (sketches)

The lens: the meta layer's dialectic, what you bring against what you
find; each folder an opening argument with a cost that bites in act 1
(costed power), opened by a milestone that teaches it (never a count),
built from commons (rarity 0-1, the codes BN6's records give). The test
each must pass is the issue's: a first act that plays unlike every
other's, which the six already crowd (Storm, Lean and Standard draw
alike). Claims about BN6's element and panel rules below are from the
series as players know it and are to be verified in
[the sources](../../research/sources.md) before a folder is built on them.

- **Tide (Aqua, Storm's counterpart).** Bubbles and arrows in A and T:
  TrnArrw1 A x4, BblStar1 T x4, IceSeed A x2 and \*, AquaNdl1 C x2,
  Cannon A x3, CrakShot T x2, Recov10 A x3, Barrier A x2, Wind \* x2,
  AirShot \* x2, Atk+10 \* x2, AreaGrab F. Plays as holds: a BblStar1
  traps a foe for the next shot. Cost: Elec hits Aqua hardest (the
  element wheel), so an Elec guardian is its wall. Milestone: a Fire guardian deleted (Aqua
  answers Fire, as Storm's Aqua one teaches Elec).
- **Ember (Fire on grass, HeatCross's partner).** FireBrn1 F G H, FireHit1
  D E F, BurnSqr1 H, GrasSeed F and \* x3, FireSwrd F, Cannon, MiniBomb
  R, Recov10 x3, Atk+10 \*. The panel's turn: grass laid under a foe, then
  fire on it; two Fire recipes in reach (WideBrn1, FlmHook1). Cost: a
  setup chip before the payoff, and Aqua guardians. Milestone: a guardian
  deleted with HeatCross brought, or WideBrn1 formed.
- **Breaker (the panel game).** Ground over damage: AreaGrab F x3, PanlGrab
  \* x4, CrakShot G x4, GolmHit1 K x2, RockCube \* x2, PnlRetrn \*,
  HolyPanl A x2, SandWrm1 G, Cannon, Recov10 x3, Barrier, Atk+10. Takes
  the foe's columns and cracks his panels; the damage is low (most chips
  10-60) and the win is a foe pinned on two panels. Cost: no burst for a
  guardian's 500 HP. Milestone: a battle won with the foe's side down to
  three panels or fewer (needs a battle-state read, the cost of its
  milestone).
- **Kindred (the Souls' kinds, SOUL BN5's partner).** Two chips of each
  of the kinds BN5's Souls unite by ([Souls](souls.md)), in one code where the records allow, so a hand
  always holds a UNITE chip. Cost: no core (two of everything), and a chip
  given up for each UNITE. Milestone: two Souls united in one run. Only
  where a Soul run is chosen; otherwise a folder of odds and ends.
- **Grove (Wood).** CornSht1 K x3, RlngLog1 K x3, Boomer K, RskyHny1 S x3,
  GrasSeed S and \*, Sword S x2, Recov10 x3, AreaGrab S x2, Atk+10 \*.
  Weakest of the five: BN6's Wood commons are slow or weak, and with
  Ember on the shelf grass is no longer its own. Kept as the candidate to
  cut first.

Costs to build any of them: a table in `meta.c` (the owner's step 2 after
issue 864), its milestone (a profile bit, `folder_feats`, and where
needed a battle read), its hint and words on issue 864's shelf, a test
row in `test_folders`, and a loot table: the code lean reads the
folder's codes as each layer is made, so a folder in A and T makes runs
find A and T ([Phase two](phase-two.md)).

## 5. After: the folders built from BN6's presets

What the owner's calls built ([Decisions and phase one](decisions.md),
Z-Canon rebuilt), measured as above (`tools/folder_hands.py`, 4000
draws; the autopilot on this build over seeds 1-12 at threat 1, Standard
played again beside it so a change of build shows).

| Folder | Kinds | Codes | In `*` | Attack power | Heal | Turn 1: damage, chips | Three turns: damage, heal, chips, PAs |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Standard | 12 | 5 | 8 | 1030 | 40 | 128, 2.5 | 303, 4, 6.5, 0.00 |
| Z-Canon, rebuilt | 10 | 9 | 12 | 1110 | 40 | 113, 2.6 | 324, 12, 7.1, 0.09 |

- **Z-Canon** now forms a Program Advance in 9 battles in 100 under the
  greedy pick (GigaCan1 3.6, FlmHook1 2.5, WideBrn1 2.4, LifeSrd 0.3),
  where the first Z-Canon formed 1, at Standard's damage within a tenth.
