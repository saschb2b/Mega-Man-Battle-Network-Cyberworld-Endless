---
type: Feature
title: "BN6's own DarkChips"
description: "BN6's own DarkChips (issue #70): what DrkSword, DarkThnd, DrkRecov, DarkInvs and DarkPlus do in BN6's battles, the owner's calls (20 max HP plus BN6's BugFrag, a BN6 flame per act) and `src/director/darkbn6.c`."
tags: [game, meta]
generated: { by: claude-code/unrecorded, at: 2026-10-13T09:50:43Z }
---

# BN6's own DarkChips (4 October 2026, issue #70)

BN6 keeps five of BN5's DarkChips with their battle code: DrkSword,
DarkThnd, DrkRecov, DarkInvs and DarkPlus (a player's lead, verified by
the code and in play: [Sources](../../research/sources.md); [The chip registry and BN5's chips' code](../../rom/chips-code.md), BN6's own
DarkChips). They pair with BN5's of the same names (`src/core/xchips.c`),
so these five could serve a run in both nets and on every build, the
3DS's with its one ROM too. First what BN6 does with them, then the
design, reasoned with the game-design skill and approved by the owner as
recommended (4 October 2026), then what was built (As built, below).

**What BN6's battles do with them** (verified):

| DarkChip | With a BugFrag in hand | With none: its base chip | The NaviCust's HP bug for the battle |
| --- | --- | --- | --- |
| DrkSword | 400 in LifeSword's area, 2 columns by 3 rows ahead | Sword, 80 | +2 (1 HP each 35 frames) |
| DarkThnd | 200, a purple Thunder that paralyses | Thunder, 40 | +1 (each 40) |
| DrkRecov | 1000 HP back | Recov10 | 7, the most (each 10) |
| DarkInvs | 8 seconds untouchable while the game moves and attacks for MegaMan | Invisibl | none |
| DarkPlus | nothing: the US version never lets it join the chip before it, and alone it makes a puff | Atk+10 alone: nothing | +4 (each 25) |

- The BugFrag goes at the use, from the run's own count; the chip stays in
  the folder for the next battle. The HP bug never takes MegaMan below 1
  HP and ends with the battle, but what it drains is lost as a hit's is,
  and the run carries HP from battle to battle ([Progression](../progression/overview.md)).
- The Custom screen and the HUD show the dark power even with no BugFrags
  ("DrkSword 400", then a Sword's 80): a telegraph that lies.
- The US version cut them: the pack never lists them, so BN6's folder
  editor cannot take them in; their icons are placeholders reading their
  id in hex ("11E"), their picture a blank purple square. BN6's folder
  rule for them is still there: with its DarkChip flag set in the core's
  copy, the editor takes three at most, MegaMan says so in his own
  NAVIGATOR line ("You can use only 3 of the same DarkChips."), and their
  cards turn purple.
- BN6 already burns BugFrags for dark power: its Giga chip BugRSwrd, which
  a run can find, turns the charged shot into a DrkSword of 200 for a
  BugFrag, a Sword without (read in the code; a bystander's rumor in BN6's
  text says the same).

## The design (approved)

- **The experience:** the dark power is there when a fight turns, and the
  run remembers what it took. A player on a 3DS meets the same temptation
  as one with BN5 beside BN6.
- **The dialectic** stays the one BN5's DarkChips restate, survival now
  against strength later: the run's prepare or press on, at a battle's
  scale. BN6's net adds a second currency to it: a BugFrag burnt on
  darkness is one the BugFrag Trader, and later the NaviCust vendor
  ([The NaviCust](../navicust.md), 3), does not get. Every currency a verb.
- **The loop layers:** the moment (a DarkChip drawn and used, its bug
  draining the rest of the fight), the act (a flame's offer on its middle
  layer; the guardian, where a DarkChip earns its price), the run (the max
  HP lost to darkness, the folder's dark slots, the BugFrags kept for
  them). Nothing reaches the meta loop: variety, not power.
- **The patterns:** a bonus with a drawback (costed power forces
  commitment); loadout as budget (a dark slot is one of the folder's 30
  that a chip of its codes loses, and a base chip once the BugFrags are
  gone); every currency a verb; an honest telegraph; nothing carried over.
- **What could go wrong, before any number.** A dominant strategy if the
  price is light: a battle's viruses hold 90 to 650 HP together and an
  act's guardian 400 to 2000 ([Progression](../progression/overview.md)), so DrkSword deletes
  every virus in its area in one cut, takes a third or more of most
  guardians, and DrkRecov is a full heal at any act, every battle one is
  drawn. The hoarder's trap if it is heavy: a player who
  never spends max HP leaves them out of the folder. The lying telegraph.
  Three prices for one decision. DarkInvs taking the controls for eight
  seconds. DarkPlus as a trap: a slot, a BugFrag and a bug for nothing.

### 1. The price in BN6's battles

- **A. BN6's own alone:** a BugFrag a use, the base chip at none, the HP
  bug for the battle; BN5's battles keep the owner's 20 max HP. Faithful,
  the player's account as it stands, but light: BugFrags come from
  skipped drafts, Mystery Data and battles, and each buys a 400 cut or a
  full heal. The dominant strategy above.
- **B. One price in every net, each net's own on top:** every battle in
  which a DarkChip's dark power ran costs 20 max HP, in BN6's battles as
  in BN5's; BN6 also takes its BugFrag and gives its bug, BN5 its worry
  and darkness. A DarkChip that fell back to its base chip costs the run
  nothing. The rule is learned once: darkness always takes a piece of
  MegaMan, and in our net it eats bugs too. DrkRecov becomes a full heal
  bought with max HP when the fight is lost otherwise, best late, as its
  bug drains the rest of the fight at the most: BN6's own reason. Counted
  where BN6 runs a DarkChip's after-effects (`0x0800B79A`), which only a
  paid use reaches (BugRSwrd's charged shot spends BugFrags too and is no
  DarkChip).
- **C. BugFrags in both nets:** BN5's DarkChips paid in BugFrags after its
  battles, no max HP. One currency, but it undoes the owner's hefty price.
- **D. Max HP alone, BN6's BugFrag patched out:** one price, but BN6's own
  rule, its fall-back to the base chip and darkness fed by bugs all go.
- **E. B with BN5's gate:** BN6's deck keeps a DarkChip back until MegaMan
  is down to a quarter of his HP, as the guest's shelf did (the guest
  battles' until issue #82's last round, [Guest battles](../../multirom/guest-battles.md), Guest
  battles; with one engine BN5's own worried rule deals BN5's DarkChips
  in BN6's battles, Transplanted Souls, S7). Closest to BN5's feel, but BN6 has no
  worry of its own to read, so the gate would be ours, and the player
  loses the when.

*Recommended: B.* E if B still dominates; A only if the owner holds
BN6's own rule above the run's balance.

### 2. Where they come from where BN5 never dresses

- **A. A flame of darkness in BN6's own art:** BN5's flame's rules in
  BN6's acts. On the middle layer of each act whose battles are BN6's, in
  a bystander's place, a flame holds a DarkChip the run lacks of the four
  that work in BN6 (DrkSword, DarkThnd, DrkRecov, DarkInvs), drawn from
  the run's seed and the act; MegaMan names the price and asks, starting
  on No. Its object is one of BN6's, recoloured in the core's copy as
  BN5's flame is turned purple (the Undernet's braziers are a candidate;
  not tried). Every build meets DarkChips at one rhythm, the 3DS too, and
  where BN5 dresses an act its own flame stands as now: telegraphed,
  chosen, priced.
- **B. The BugFrag Trader:** a DarkChip among its prizes. The chip beside
  the currency it burns, but a roll, never an offer, its price unsaid.
- **C. A Graveyard guardian's Guardian Data:** a DarkChip in his Navi
  chip's place. Late, and in the endless net only.
- **D. Purple Mystery Data:** dark data behind an Unlocker. It doubles a
  lock that already means rare chips.

*Recommended: A.*

### 3. The blank picture and the icons

- **A. Blank, as the US version left them:** honest about cut data, but
  the hand's "11E" icons read as a bug.
- **B. The base chip's picture and icon:** the records' picture, palette
  and icon pointers set to Sword's, Thunder's, Recov10's and Invisibl's in
  the core's copy (pointers only, nothing embedded), with BN6's own marks
  by its DarkChip flag: the purple card in the folder (seen) and, by the
  code, a dark frame on the icon (not seen). The picture then shows what
  the chip becomes without BugFrags.
- **C. BN5's own, where its ROM sits beside BN6's:** not on the 3DS, so
  two looks for one chip.
- **D. The base chip's picture darkened:** its palette copied and darkened
  in the core's free space. Clearer than B, more work.

*Recommended: B, and D later if a hand cannot tell a DrkSword from a
Sword.*

### 4. How many, and the folder's rules

*Recommended:* BN6's own rule, brought back by its DarkChip flag in the
core's copy: three in a folder at most, MegaMan's own line at a fourth.
The run's: one of each kind, as a flame offers only a kind the run lacks.
The pack lists them (their library flag 0x20 cleared there), and the
player puts them in with EDIT as any chip. BN5's battles keep their own
rule (three, one of each, put in by the engine), a DarkChip in BN6's
folder counting as the run's one of its kind. DarkPlus sits out of BN6's
battles: BN6's flame never offers it, and one from BN5's stays out of
BN6's pack. BN6 names the chips that join the one before them by id
(`0x08029224`); a DarkPlus that joins would be engine work, its price
counted by the engine.

### 5. Teaching and fiction

- **The fiction**, the issue's own: dark power that feeds on bugs in BN6's
  net (a BugFrag a use, and it bugs MegaMan for the fight) and on MegaMan
  himself in either (20 max HP, with B). BN6's own BugRSwrd already pays a
  BugFrag for a DrkSword.
- **At the flame:** MegaMan names the chip, what it does and its price,
  all of it the first time a profile meets a BN6 flame, the chip and the
  price after. DarkInvs said as it is: the darkness fights for us for
  eight seconds, and nothing touches us.
- **BN6's own lines:** a bystander on the flame's layer runs BN6's own
  script about DarkChips ("A dar...DarkChip!? Those are against the
  law!"), a rumor before the find; at a fourth DarkChip the folder editor's
  NAVIGATOR line is MegaMan's own.
- **Just in time:** after a profile's first battle with dark power used,
  MegaMan names what it took (the BugFrag, the bug gone with the battle,
  with B the 20 max HP that stays gone); the first time one falls back
  for want of BugFrags, he says it was only a Sword.
- **An honest telegraph:** while the battle's BugFrags are 0, the records
  read their base chip's power in the core's copy, so the Custom screen
  says 80 where 80 will land (whether BN6 reads the record anew each time
  is to be tried); on the map MegaMan says once that the BugFrags have run
  out with a DarkChip in the folder.

### Not chosen, and what to measure

- Not chosen: DarkChips in the run's setup (power across runs); a
  DarkChip used up by its use (BN5 and BN6 both keep it); BN5's other
  seven in BN6's battles (BN6 has no code for them).
- To measure: offers taken and declined; DarkChips in the folder at each
  guardian; uses a run in each net and the max HP they took; DrkRecov's
  share of them; BugFrag income against what darkness, the BugFrag Trader
  and the vendor take.

### The owner's calls (4 October 2026: each as recommended)

1. The price: B, 20 max HP for every battle a DarkChip's dark power ran
   in, in either net; BN6's BugFrag and HP bug on top; the base chip's
   fallback free.
2. Where they come from: a BN6 flame on the middle layer of every act
   whose battles are BN6's (the 3DS's runs too).
3. The look: the base chips' pictures and icons with BN6's purple card.
4. The folder: BN6's three and the run's one of each; DarkPlus out of
   BN6's battles.
5. The words: MegaMan at the flame and after the first dark battle, BN6's
   two lines, the honest power.

## As built (4 October 2026)

Every build meets them, the 3DS's and a run without BN5's ROM too: BN6's
own code runs them, our part is the run around it (`src/director/darkbn6.c`,
`darkchips.c`; [The chip registry and BN5's chips' code](../../rom/chips-code.md), BN6's own DarkChips in a run).

- **The price, one in every net.** A hook where BN6 runs a chip's
  after-effects (`0x0800B79A`), which a DarkChip's id reaches only where
  its dark power ran, a BugFrag paid, marks the battle; back on the map,
  20 max HP go as BN5's battles take them (`dark_price`), once a battle
  however many uses. BN6's own price comes on top, by its own code: the
  BugFrag at each use, from the save and the battle's count, and the
  NaviCust's HP bug for the rest of that battle. A use that ran as its
  base chip for want of a BugFrag costs nothing. BN5's battles keep their
  rule, and Chaos Unison stays free.
- **The source.** On the middle layer of every act whose battles are
  BN6's, in a bystander's place as BN5's flame stands, a flame of
  BN6's own: its blue flame of sprite list 7, copied into the core's free
  space and turned purple as BN5's is (never a committed asset), holding
  one of DrkSword, DarkThnd, DrkRecov and DarkInvs the run lacks, from
  the run's seed and the act (`dark_flame_pick`, its seed mixed first so
  that seeds apart in their low bits alone pick apart too). The layer
  keeps its flame: a CONTINUE on it finds the same kind, held by then
  where it was taken before the save, and gives nothing twice (before,
  by the code, a CONTINUE on a layer the PET's Save kept after its BN5
  flame was taken picked the next kind and gave it at once; read, not
  seen in play). An install-time actor: no layer's generation changed.
- **The chips.** Taken, the chip goes to the Pack in *, and EDIT puts it
  in. The run holds one of each kind, shared with BN5's DarkChips of the
  same names: from either net's flame BN6's chip of a kind its battles
  play goes to the Pack, and each layer gives the Pack one the run holds
  that neither the Pack nor the folder has (a run saved by an earlier
  build, holding BN5's DrkSword, gets BN6's as it continues: the same
  path a run begun with `--dev darkchips` takes, seen; an earlier build's
  save itself not tried). DarkPlus stays out of BN6's Pack and battles.
- **The folder.** BN6's own rule, by its DarkChip flag set in the core's
  copy: three at most, refused in MegaMan's own NAVIGATOR line ("You can
  use only 3 of the same DarkChips."). It held only once their sort keys
  were set too: the US version left them 0, and the editor took any two
  DarkChips for one chip and let one in for another past the three.
- **The look.** Their icons, pictures and palettes are their base chips'
  (Sword, Thunder, Recov10, Invisibl), on BN6's purple DarkChip card in
  the folder and the pack (seen). BN6 puts the Custom screen's cursor on a
  DarkChip in the hand, as BN5 does (seen); the screen's fade and music
  for one under the cursor, which its code holds, did not show in our
  captures.
- **Honest power.** The four records' power is the one that lands: their
  own while the BugFrags held are one or more, their base chip's with
  none ("DrkSword 80" over Sword's picture), and their base chip's for the
  rest of a battle whose last BugFrag a DarkChip spent (BN6 reads the
  record anew at each Custom screen: seen; a chip already picked keeps
  the power it was picked with, read in the code, not seen).
- **The words.** MegaMan at a BN6 flame: what its dark power does, the
  BugFrag a use burns, the bug for the battle, the base chip without one,
  the 20 max HP, all of it a profile's first time and the rule after, then
  the question on Yes, and where it goes. After a profile's first battle
  its dark power ran in, what it took: the BugFrags, the bug gone with
  the battle, the max HP that stays gone; then as BN5's battles say it.
  After a battle where one ran as its base chip, once a session: "With
  none to burn, that DrkSword was only a Sword." BN6's own words, run
  from the player's ROM: on a flame's layer the first bystander free of
  a part to play says its BBS's post on DarkChips ("A dar...DarkChip!?
  Those are against the law!"), and the folder's NAVIGATOR line is
  MegaMan's.
- **Where BN5's ROM is read** (the owner's call of 13 October 2026,
  [DarkChips](darkchips.md)): DarkPlus and DarkInvs are BN5's chips in BN6's
  records (DarkInvs from either flame, by BN5's rule and words; DarkPlus
  from BN5's alone). BN6's rule here stays where BN5's ROM is not read.
- **In BN5's areas** (until issue #82's last round BN5's own battles,
  where BN5's rule brought a kind of these four from the run's DarkChips,
  shelved until MegaMan worried) the four play as everywhere: from the
  folder, by BN6's rule, a BugFrag a use; BN5's flame offers them there
  with BN6's flame's words ([DarkChips](darkchips.md)).
- **The autopilot** never picks a DarkChip, BN6's or BN5's.
