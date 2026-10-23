# Liberation Missions

Battle Network 5's Liberation Missions, taken apart to their rules, and how
Cyberworld Endless makes them afresh from a seed: a turn-based board where a
team of Navis liberates dark panels toward a Darkloid. The owner's brief
(23 October 2026): every mission generated, dissected "into its very
details to create a fantastic generator that spits out fun missions every
time", "feeling it could come straight out of mmbn5". A player's request
started it: a liberation-mission roguelike, co-op, maps that grow toward
the last boss, special routes to other bosses, each Navi's ability a reason
to try him.

Where it stands: the dissection (from BN5's ROM, its own text, and the
players' guides), the design decision, which the owner approved on 23
October 2026 (The owner's decisions, at the end), a ROM-free prototype
of the generator with its measures, and stage 1 in the engine: the dark
layer and its board, MegaMan alone (Stage 1, as built, below).

## Summary

- **BN5's missions are data**, nine records in Team Colonel's ROM
  (`BN5_LIB_MISSIONS`, `src/emu/bn5_liberation.h`): each a list of panels
  laid over one of its net maps, their kinds, the team, the Darkloid and
  guardians, item panels, traps, BonusPanels, the par and the rewards. The
  boards were opened in romlab by setting the mission and entering its map.
- **The decision (the owner's, 23 October 2026):** a liberation is a kind
  of layer the endless net may hold (a "dark layer"), from act 2, chosen at
  a route fork and announced on the act card; the team is MegaMan and the
  Navis whose Souls the run has won, which in Team Colonel are exactly
  BN5's liberation team, each fought as MegaMan in his Soul. Co-op comes
  later over the server; a separate mode may follow on the same generator.
- **The generator** lays a board out in the net's shape language, places
  BN5's kinds by BN5's rules, rejects what breaks a guarantee, and sets the
  par from a planner that plays it by BN5's rules, scaled by the planner's
  own result on BN5's nine.
- **The prototype** (`tools/liberation.py`; `tools/liberation_bn5.py` for
  BN5's own boards) makes 144 missions over acts 2-6 and measures them
  against BN5's nine: the boards keep to BN5's bands of shape and dark,
  route choice opens up from act 3, and every teammate's command earns
  its Order Point on the boards kept.

## BN5's missions, dissected

How each fact is known: **ROM** (a record or routine read, named in
`src/emu/bn5_liberation.h` and docs/ROM_DATA.md's BN5's Liberation
Missions), **play** (seen in romlab on the plain ROM), **text** (BN5's own
tutorial, read in its ROM, not kept), **guides** (the players' guides,
assumed until checked: Casebier's GameFAQs guides to both versions, played
on the Japanese release in 2005; MMKB's Liberation Mission page; Wikibooks'
Liberation Guide and its walkthroughs).

### Where they live

A mission runs on the map game state as a sub-mode of its own: EnterMap
(`BN5_ENTER_MAP_BOARD`) sets sub-mode `0x18` instead of the map's 4 where a
mission is under way and its map is the map entered (`BN5_LIB_ON_MAP`). The
liberation state (`BN5_LIB_STATE`) holds the mission number, the Order
Points, the phase and par, and pointers to the mission's record and to its
panels in RAM. **Play:** with the mission number set and BN5's own warp
into the mission's map from a state after its intro, the board starts: its
camera tour, its tutorial, MegaMan and Colonel on the start, the Order
Points gauge at 3, PHASE 1 / TRGT PHASE 7 (pictures in the pull request).
Its 100 panels in RAM are the ROM's list for mission 1, panel for panel,
with each one's look worked out.

### The nine missions (ROM)

Every mission stands on a map of group 0x86, the same net maps BN5's areas
use (docs/MULTIROM.md), its panels 32 world units each. The Darkloids and
areas by the guides, the rest by the records:

| # | Map | Darkloid | Par | Team (Team Colonel) | Panels | Dark | Holes | Guardians | Items | Traps | Bonus | Barriers | Rewards: best / par / over |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | 86:00 ACDC 3 | BlizzardMan | 7 | MegaMan, Colonel | 100 | 33% | 2 | 2 | 7 | 0 | 2 | 0 | chip at 5 or fewer / chip at 6-7 / 1000 z |
| 2 | 86:01 Oran 3 | ShadeMan | 9 | + KnightMan | 175 | 43% | 3 | 4 | 8 | 0 | 3 | 0 | 7 / 8-9 / 2000 z |
| 3 | 86:02 SciLab | CloudMan | 9 | + ShadowMan | 220 | 48% | 4 | 6 | 8 | 0 | 2 | 7 | 7 / 8-9 / 3000 z |
| 4 | 86:03 End 2 | Dark MegaMan | 10 | Colonel, KnightMan, ShadowMan, TomahawkMan | 220 | 61% | 5 | 7 | 9 | 0 | 3 | 4 | 8 / 9-10 / 4000 z |
| 5 | 86:04 End 5 | CosmoMan | 9 | MegaMan, Colonel, KnightMan, TomahawkMan, NumberMan | 237 | 51% | 5 | 6 | 21 | 14 | 3 | 13 | 7 / 8-9 / 5000 z |
| 6 | 86:05 Undernet | Dark Colonel | 10 | MegaMan, ShadowMan, TomahawkMan, NumberMan, ToadMan | 248 | 60% | 5 | 7 | 18 | 8 | 3 | 6 | 8 / 9-10 / 5000 z |
| 7 | 86:06 Nebula 1 | ShadeMan | 12 | as 5 | 217 | 59% | 5 | 6 | 18 | 8 | 4 | 14 | 10 / 11-12 / 5000 z |
| 8 | 86:07 Nebula 3 | CloudMan | 14 | as 6 | 223 | 64% | 5 | 7 | 29 | 12 | 3 | 12 | 12 / 13-14 / 5000 z |
| 9 | 86:08 Nebula 5 | CosmoMan (and BlizzardMan) | 16 | MegaMan, Colonel, KnightMan, ShadowMan, ToadMan | 222 | 50% | 5 | 7 | 12 | 0 | 2 | 59 | 14 / 15-16 / 5000 z |

What rises with the missions: the board (100, then 175-250 panels), the
DarkHoles (2 to 5), the team (2 to 5), the guardians and their kinds, the
items and, from mission 5, traps; barriers from mission 3, a maze of them in
9. The par rises from 7 to 16 but not with the board: missions 5 and 6 are
larger than 2 and 3 at the same par, with more Navis. The rewards are one
table a mission (`BN5_LIB_PTR_REWARDS`): a chip for two phases under par or
better, another for within par, zenny after (**ROM**; the guides agree).
Every mission starts with 3 Order Points (the record's +0xB). The record's
+9 (13-22) is read into the state's +0xA; what it does is not traced.

### The board (ROM, play)

A board is a set of panels on a square lattice, following the map's floor:
platforms and one-wide walkways, as the net is (docs/LEVEL_DESIGN.md). Its
kinds (`BN5_PANEL_*`):

- **Clean** (0): the team walks it.
- **Dark** (1): liberated by a battle. Guardians walk dark panels.
- **Item** (3): looks dark and holds an item or a trap.
- **BonusPanel** (9, "?"): a roulette (below), at the ends of stubs.
- **DarkHole** (0x0A): in a ring of dark panels, its guardian beside it;
  every one must fall before the Darkloid can be fought (**text**).
- **The Darkloid's ring** (0x0B): the eight panels around him.
- **BarrierPanels** (5, 6, 7): walls numbered 1-3, opened by the item panel
  that holds their key (**ROM**: the key's handler; **text**: "You unlocked
  the BarrierPanel"). Mission 7 walls a region off; mission 9 is a maze.
- **Start** (0x0C): where the team stands (missions 3 and 5-9 place the
  team by its records instead).

Placement as BN5 does it, read off the nine boards: the start at an edge
(a cross or a column of panels on mission 1, a 3x3 on 7); the Darkloid's
3x3 on the far side; the DarkHoles spread out, each in a full ring of dark
panels; clean walkways near the start, dark thickening toward the holes and
the Darkloid; item panels inside dark clusters and at corners; BonusPanels
on the ends of stubs; barriers across walkways.

### A phase (text, guides)

The player's phase, then the Darkloids' phase. In the player's phase each
Navi acts once: he moves over clean panels, then liberates a dark panel
beside him, uses his command for one Order Point, or rests ("Pass": he
skips his turn and regains some HP, **text**; half his max HP, **guides**).
The more dark panels around a Navi, the harder his battle (**text**). R
switches Navis, L moves the camera over the board, START offers to retreat.

### Liberating a panel (ROM, text)

A liberation battle is a battle record of type 1 (`BN5_BATTLE_LIBERATION`):
all viruses deleted within three turns (`BN5_LIB_TURNS_UP`: the battle
loop ends it when BattleState +7, the Custom Gauge's fills, reaches 3).
Deleting them within the first turn liberates every panel around the Navi
too (**text**). Which columns the field gives depends on which of the
panels around him are dark, so viruses can stand on both sides of him
(**text**: L and R turn him around; the guides give the rule). His HP
carries from battle to battle. Every Navi but MegaMan has one special chip
for the Custom screen, once a battle (**text**: Colonel's C-Cannon).
From a liberation battle's eleventh turn every hit does double
(`BN5_LIB_LATE_DOUBLE`, **ROM**), which only a battle that outlasts three
turns meets: assumed to be the Darkloid's.

### Order Points (ROM, text)

One pool for the team: 3 at the start, 8 at most (`BN5_LIB_OP_ADD` caps
it), 1 for every command (**text**), found in item panels (2 or 3, **ROM**)
and from the BonusPanel's roulette (5, **guides**).

### Items, traps and BonusPanels (ROM)

An item panel's item (`BN5_LIB_PTR_ITEMS`, its kinds' table
`BN5_LIB_ITEM_KINDS`): a heart (HP, its amount times ten: 30-900, rising
with the missions), a BarrierPanel's key, Order Points (2-3), zenny (its
amount times 100), BugFrags, a chip. Traps (`BN5_LIB_PTR_TRAPS`) from
mission 5: damage (50 in missions 5 and 6, 100 later) or paralysis. A
BonusPanel's roulette (`BN5_LIB_BONUS_ROLL`) starts on one of five slots
from the random number; its five outcomes are each mission's records (full
HP, Order Points, one of the mission's chips, damage to the nearest enemy,
the team untouchable for a phase, **guides**; the slots' order in the
records is not traced).

### The Navis (ROM, text, guides)

Each Navi's HP rises mission by mission (`BN5_LIB_NAVI_HP`, **ROM**):
Colonel 200 to 800, KnightMan 400 to 900, ShadowMan 250 to 700, TomahawkMan
300 to 700, NumberMan 300 to 650, ToadMan 300 to 600; MegaMan has his own.
The commands are BN5's own list (**text**): Liberation, LongSwrd, WideSwrd,
ScrenDiv, TwinLiberation (twice), LifeMelody, BombLiberation, SneakAttack,
PanelSearch, NumberCheck, Napalm, TomahawkSwing, MagnetBarrier, Save, Pass.
Their shapes are the guides' (the routines that apply them are not traced):

| Navi | Command | What it does |
| --- | --- | --- |
| MegaMan | LongSwrd | Two panels in a line, one battle |
| Colonel (ProtoMan) | ScrenDiv (WideSwrd) | Three across, one battle; can save |
| TomahawkMan | TomahawkSwing | Two deep and three wide, one battle |
| NapalmMan | Napalm | Four ahead and both sides of the third |
| NumberMan | NumberCheck | Two by three ahead, no battle: takes the items, disarms traps, liberates nothing |
| SearchMan | PanelSearch | A line of dark panels: items shown, then liberated after one battle |
| ShadowMan | SneakAttack | Passes over dark panels; strikes a guardian or the Darkloid one or two ahead (never below 1 HP) |
| GyroMan | BombLiberation | Flies over dark panels; liberates the one beneath him without a battle |
| KnightMan | (none) | Takes no damage on the board; shields a Navi beside him |
| MagnetMan | MagnetBarrier | The team takes no damage on the board until the next phase |
| ToadMan | LifeMelody | A teammate beside him liberates five ahead this phase |
| Meddy | TwinLiberation | Battles at both ends of a line; a second Navi finishes it from the far end |

Team ProtoMan's six (ProtoMan, GyroMan, NapalmMan, SearchMan, MagnetMan,
Meddy) are the other version's; this ROM keeps their commands' names.

### The Darkloids' phase (text, guides)

Guardians "attack closer enemies first" (**text**), and one is best
deleted before the Darkloids' phase or its DarkHole liberated in one shot.
A DarkHole's liberation is a fight with its guardian; won, it frees the
eight panels around it; while the hole stands, a deleted guardian comes
back (**guides**). The kinds (`0x11`-`0x13` in the records): BigBrute
moves two and hits every Navi beside it; TinHawk hits one within three;
Bladia teleports, darkening the clean panel it lands on (**guides**, with
HP and damage by mission). The Darkloid attacks Navis in his reach and
heals for each dark panel around him (100 a panel, **guides**). All down
means a retreat (**text**).

### How the board looks and moves (play, ROM)

The owner's direction (23 October 2026): "I expect a near mmbn5 like
result. Take the original as hard reference." What BN5 shows, recorded in
romlab on the plain ROM (mission 1, from the board's start through a won
liberation and an item's find; the recordings are in the pull request):

- **Two layers.** BG1 is the mission's map itself (group 0x86): a floor
  built as a grid of framed panels, one a 32-unit board cell, with BN5's
  walkway trim at the start. BG2 lays each panel's kind over it as an
  isometric diamond of tiles (two, four, four and two across): cyan for a
  clean panel (palette 8), a magenta swirl for a dark one (palette 9, which
  changes every frame: the dark panels' pulse), a pit for a DarkHole.
  An item panel is a dark panel with a teal stud. The start is the map's
  own floor, no overlay.
- **The faced panel** gets a yellow outline, a ripple, while the command
  window is open over a panel to liberate (6 frames after A; never while
  walking, never on a clean panel); the command window (Liberation, the
  Navi's command, Pass) greys what cannot be used there.
- **A won liberation:** "LIBERATE SUCCESS!" over the battle (or "1 TURN
  LIBERATE!" for a one-turn win), a fade back to the board, the freed
  panel turning from the swirl to grey to cyan, then the Navi's line in
  his chat box ("We liberated it!").
- **An item's find:** the item pops out of its freed panel as an icon (a
  zenny coin, a heart, Order Points and so on), its shadow on the panel:
  up 8 pixels after 3 frames, down 2 after 5 more, held while the Navi
  says what he found ("I found some Zennys!"). The board queues it as an
  event (`BN5_LIB_EVENT`, 12 bytes each from `0x02001AB0`). Each kind's
  icon is a sprite of list 7, its index the item kinds' table's
  (`BN5_LIB_ITEM_KINDS` +5: 0x18 a heart, 0x19 a key, 0x25 zenny, 0x26
  BugFrags, 0x29 Order Points, 0x53 a chip), its animation 0 with its
  shadow.
- **The tiles** are the mission maps' own: the overlay's tiles stand from
  tile 865 of map 0x86:00's tile set, uncompressed at `0x08623130` (its
  tile t at `0x0861C510` + 32t, copied whole into the BG's characters),
  and the diamonds in twelve looks after them (`BN5_LIB_LOOKS`, 8 x 4
  tile-map entries each, named by the mission record's first block, the
  same in all nine, a panel's kind its look's index): clean, dark, a
  paled dark (kind 2, the dissolve's middle),
  an item's stud, a "+5", BarrierPanels 1-4 (numbered, a keyhole), a
  BonusPanel's "?", a DarkHole, a grey grid (kind 0x0B, the Darkloid's
  ring until the DarkHoles fall); palettes 8-12 the board's own. Each
  panel's four look bytes are its corners, each the three neighbours
  around it the Navi in play can walk (`BN5_LIB_CORNERS`); they choose
  height modifiers on the panel's border cells in the map's collision
  (`BN5_LIB_EDGES`), so a Navi stops where he cannot walk: no drawing.
- **The palettes move.** The dark panels' (9) rest 120 frames, dim
  over three frames and come back, 216 frames round; 10 steps there and
  back over four, 20 frames a step; the barriers' (11) glint every 132
  frames; the BonusPanel's (12) breathes over six, 12 frames a step.
  They are the mission map's own GFXAnim scripts (`BN5_LIB_ANIMS`), the
  engine BN6 runs too, so in BN6 they run as BN5's own scripts copied,
  in place of the host map's: a host map's script left running writes
  its colours into the same banks every few frames and flickers (PR
  #202's first walk: Central Area 1's bank 9 cycle, every 16 frames).
- **The freed panel** blinks once the board has faded back in: its
  look toggles between its old kind and clean every 4 frames, 14 times,
  then stays clean.
- **The sprites:** the faced panel's outline and the items' icons are
  BN5 sprites (list 7: the outline 15, the icons 24, 25, 37, 38, 41 and
  83). The outline shows only while the command window is open over a
  dark panel faced, never while walking.
- **The banners** ("LIBERATE SUCCESS!", "1 TURN LIBERATE!", "TURN n
  START!") are five 32x16 affine OBJs at y 64, built from 8x16 letters
  (`BN5_LIB_LETTERS`, digits `BN5_LIB_BIG_DIGITS`, palette 11):
  they open from squashed to full size in 8 frames, hold 43 and close in
  8.

- **The HUD and the window:** the Order Points gauge is six OBJs at the
  top right (`BN5_LIB_HUD_OAM`), its frame, bar and digits BN5's
  (`0x087E4xxx`, OBJ palette 12); the L view's PHASE and TRGT PHASE are
  OBJs on palette 13; the command window is a frame on BG0 (nine tiles,
  `BN5_LIB_WINDOW_TILES`) with its text in 21 OBJs of 16x16 drawn by
  BN5's code (palette 15), 14 pixels a line, BN5's menu cursor beside
  it; the gauge's bar is 6 pixels an Order Point (`BN5_LIB_HUD_MASKS`);
  the battle's
  banners come from BN5's battle-message table (`BN5_LIB_MESSAGES`),
  the engine BN6 also has.
- **The backdrop** is the mission map's own animated backdrop, recoloured
  purple (BG palette 0 from `0x0861BF20`): outside a mission the same map
  is BN5's teal ACDC Area 3.
- **The phases' banners** ("PLAYER PHASE 02", "DARKLOID PHASE"): a band
  across the board, BG0 filled with one dark colour and shown in WIN1
  alone (rows 56-87), blended so the board shows through at half; it
  opens over three frames from its middle row, and 18 frames on the
  words slide in from the right, 32 pixels a frame, with sound 0xD6, hold
  65 frames and slide out to the left; the band closes 104 frames after
  it opened. The words are OBJs of letters' pieces (`BN5_LIB_PHASE_WORDS`,
  each phase its own palette), the player's with the phase in two
  digits. Before each, BN5's camera pans to the unit about to act (the
  Darkloid, a guardian, the Navi).
- **The L view's panels** are the same band's fill, in WIN1 behind PHASE
  and TRGT PHASE (20-107 by 120-147) and in WIN0 behind the keys' help
  (123-219 by 12-42).
- **The music:** song 0x22 is the board's own in both phases (its clock
  runs on from one to the next: no change of song at a phase), 0x1C plays
  at the mission's opening (the board's routine 1, its tour), the
  liberations' battles BN5's virus battle song 0x15.
- **The camera** pans to the unit about to act: to the Darkloid before
  DARKLOID PHASE (about 20 pixels a frame, then a quarter of what is left
  each frame, some 32 frames, the band 3 frames after it stops), to each
  guardian as it moves and back to the Navi before PLAYER PHASE; at the
  mission's opening it tours the board to the Darkloid. Under L the D-pad
  moves it, 4 pixels a frame, 8 with B ("MoveCam(F)").
- **The guardians on the board** are sprites of BN5's list 6: 83 BigBrute
  (a red, spiked beast), 84 TinHawk, 85 Bladia, each standing in its
  animation 0 with its shadow.

Still to trace: the window's font, when an item's pop shows and goes,
the sounds of a DarkHole closing and of LIBERATE SUCCESS, the mission's
end (BN5's Navi reads the target and the actual phases and the Officials'
chip reward follows, its text in BN5's ROM from `0x087E8854`), the
Navis' own board sprites beside MegaMan, and whether the other eight
missions' maps carry other sets.

So the engine's board is to be BN5's: its overlay tiles, diamonds and
palettes read from BN5's ROM at run time (never kept), its pulse, its
outline, its dissolve, its banners and its item icons, over a floor of
the mission maps' panel grid where BN5's ROM is read. The dark layer's
first build (stage 1) tints BN6's floor instead and has none of the
animations; its rework follows this section.

### Not found, and assumed

- The commands' shapes in the ROM: the board's command code
  (`0x0804B000`-`0x0805D000`) was located, not read through; the shapes
  are the guides'.
- The BonusPanel's five slots' order, the record's +9, what the Darkloid's
  battle does past three turns: not traced.
- A hard phase limit: none found in the state's readers or the guides; a
  mission runs on past par for the lesser reward.

## The decision

Reasoned with the game-design skill.

- **The experience:** "I see the whole dark area and plan my team's way
  through it; every phase I spend is the better reward slipping away."
- **The dialectic** is the run's own, *prepare or press on*
  (docs/META.md), at the board's scale: rest, bank Order Points and clear
  the guardians, or race the par for the better reward. Each phase restates
  it for each Navi.
- **The loop layers:** the session (one board, about BN5's 10-20 minutes),
  feeding the run (its reward chip, and the route it can open) and the meta
  only through variety (which Navis a run brings).
- **The patterns:** the hand-made units of a procedural run (BN5's panel
  kinds, guardians and commands as the units, the layout generated); an
  honest telegraph (the whole board, the guardians' reach, the par shown
  before the first move); loadout as budget (a shared pool of 3-8 Order
  Points); give the defender a verb (KnightMan, MagnetBarrier, rest); a
  branching map (the order of the DarkHoles, a sealed route).

### Where a liberation stands

| Option | For | Against |
| --- | --- | --- |
| **A layer kind** (recommended first): a dark layer an act may hold, from act 2, one an act at most, behind one of the route fork's two pads, the act card saying so | The endless net's own shape (a net area taken by darkness, as BN5's are); a risk the player chooses (docs/PROGRESSION.md, Risk the player chooses); one board's length | A run's hour grows by a board; act 2's team is small |
| An act's climax: the act's guardian on the Darkloid's panel | High stakes, the guardian as BN5's Darkloid | Every act alike; the guardian's own arena lost |
| A separate mode: a run of missions growing to a last Darkloid | The player's request whole; co-op fits it | Half a second game to tune; splits the meta |
| Co-op | The player's request; turn-based, so the board is easy to share | Needs the board first; each battle stays one player's |

Decided as recommended (the owner, 23 October 2026): the layer kind first,
co-op later over the server; the separate mode may follow as a setup
option (a run whose acts are all dark, the Nest a last mission: the same
generator).

### The team

BN5's Team Colonel liberates with Colonel, KnightMan, ShadowMan, NumberMan,
TomahawkMan and ToadMan: the six BN5 Navis whose Souls a run already wins
(docs/BOSSES.md, BN5's Navis). So a held Soul lends that Navi to the board,
with his BN5 command; the team grows as BN5's does, mission by mission,
run by run (act 2: MegaMan and one at most). Without BN5's ROM, or for
BN6's Navis, a held Cross could lend its Navi with a command of BN5's kinds
(a line, a wide, an area, mobility, defence, support): the owner's call.

In battle, a teammate's liberation is fought as MegaMan in that Navi's Soul
(or Cross) from the first turn, with the teammate's own HP pool: BN5's own
fiction for a Soul, and the forms already in BN6's engine. MegaMan alone
brings the NaviCust and DarkChips, as in BN5. A teammate as a playable Navi
of his own (ProtoMan's or Colonel's player code moved into BN6's engine) is
the larger alternative.

The owner's standing rules hold: MegaMan walks as BN6 walks him on the
overworld (the board is BN5's own mode, where units move as BN5 moves
them); BN5's content runs in BN6's engine (docs/MULTIROM.md, One net); no
power carried across runs (a run's team is its own Souls); online on the
common ground (co-op needs both players to have BN5's ROM for BN5's Navis,
or the Cross lineup).

## The generator

### What a mission is made of

The board's shape and its dark follow bands measured on BN5's own boards
(`tools/liberation_bn5.py`): **fields**, the share of panels in a 2x2
block of floor; **fill**, the floor's share of its box; **dense**, the dark
panels around a dark panel on average, which sets how often a liberation
is won in one turn. BN5's missions 1-3 stand at 72-82% fields, 22-28% fill
and 3.5-3.9 dense; missions 4-9 at 88-99%, 43-48% and 4.1-6.0: its later
boards are wide fields thick with dark.

| Act | BN5 like | Panels | DarkHoles | Team | Barriers | Dark | Fields | Fill | Dense | Items | Bonus | Guardians | Darkloid |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 2 | 1 | 90-112 | 2 | 2 | none | 28-45% | 68-86% | 26% | 3.3-4.3 | 7 | 2 | BigBrute | 300 HP, reach 1, 30 |
| 3 | 2-3 | 165-215 | 3 | 3 | 1 | 45-58% | 70-88% | 27% | 3.4-4.5 | 8 | 3 | + TinHawk | 600, 2, 50 |
| 4 | 4-5 | 210-245 | 4 | 4 | 2 | 52-66% | 86-100% | 42% | 4.6-6.2 | 12 | 3 | + Bladia | 900, 3, 80 |
| 5 | 6-7 | 215-255 | 5 | 5 | 2 | 58-68% | 86-100% | 44% | 4.6-6.2 | 16 | 3 | all | 1200, 2, 80 |
| 6+ | 8-9 | 220-260 | 5 | 5 | 3 | 55-68% | 86-100% | 45% | 4.2-6.2 | 14 | 3 | all | 1500, 3, 100 |

The steps (`tools/liberation.py` `generate`):

1. **The floor:** platforms (3-5 by 3-4 at act 2, up to 8 by 6 from act 4)
   in a box the act's fill sizes, joined by walkways bending at right
   angles (a spanning tree, then a loop or two), one wide early and two
   wide in six of ten from act 4, as BN5's fields run; a start zone at the
   left edge; the Darkloid's 3x3 at the right; spurs ending in 3x3 pads or
   one-wide stubs. In the engine the floor is the generated layer's own,
   taken at 32 world units a panel.
2. **DarkHoles** on platforms with a whole ring of floor, past a third of
   the walk from the start, six apart and four from the Darkloid; their
   rings dark, from act 4 a second ring where the floor runs on; a guardian
   on each ring (BigBrute first, the act's kinds after); roaming guardians
   from act 3.
3. **Dark panels** thickening with the walk from the start; from act 4 the
   Darkloid's approach dark three deep. Then **the density rule:**
   majority passes until a dark panel has as many dark around it as BN5's
   boards of the act have (the dense band), so a one-turn win is no sure
   thing and a command that frees several panels in one battle is worth
   its Order Point. The first two panels of the walk stay clean.
4. **BonusPanels** at stub ends; **barriers**, walls of one or two panels
   (a two-wide way walled across) that cut a region off the start's side
   (never the Darkloid's), the key in a dark panel on the near side; from
   act 4, half the time, the last barrier's region holds a **sealed
   route** (below).
5. **Items** in the dark, corners and dead ends first: one Order Points
   item always, then hearts, Order Points, zenny, chips; BugFrags and traps
   from act 4.
6. **The par** from the planner, then **the command rule** (below).

### Guarantees, and what is rejected

A board is rejected and the next seed tried when: its size is off; the
Darkloid is nearer than 60% of the farthest walk; the DarkHoles cannot be
spread; its dark misses the act's dense band; a barrier finds no wall or
its key no near spot; a key lies behind its own lock; a DarkHole is cut
off; nothing can be liberated in the first phase; a guardian stands within
three of the start; a teammate's command finds no situation; the planner
wins it less than 60% of the time; or a teammate's command does not earn
its Order Point.

**The command rule.** The run knows which Souls it holds when a layer is
made, so the generator makes the board for its team. First it gives each
teammate's command its situation (`situations`): a roaming guardian on the
way to a DarkHole for SneakAttack; a cache of four item panels in a 3x2 of
dark, a clean panel beside it to stand on (from act 4 one of them a trap)
for NumberCheck; a dark line of five on the way for LifeMelody; at least
three spots of three dark across, or two by three, for ScrenDiv and
TomahawkSwing. Then the planner plays the board again with each teammate
and no command, the same random numbers as before, and the board is kept
only where the planner uses every command (half a time a play at least)
and where:

- a liberating command (LongSwrd, ScrenDiv, TomahawkSwing, LifeMelody)
  saves phases (`EARN`: no worse than a tenth of a phase, scaled as the
  par, within the plays' noise);
- a utility command (SneakAttack, NumberCheck) costs the team no more than
  a quarter of a phase (`UTILITY_EARN`): it pays in safety and loot, a
  guardian weakened or items taken with no battle, which phases do not
  show, so its yield is reported beside it.

KnightMan's StoneBody is passive and always on.

### The par

A planner plays each board 12-24 times by BN5's rules, abstracted: each
Navi moves over clean panels and takes the best of liberating, his command,
fighting a guardian in his way (the guardian's HP carried from fight to
fight; one standing on the way on is worth the detour he forces), or
resting; support acts first (LifeMelody, SneakAttack), then
MegaMan; battles won in one turn, won later or lost by the dark around;
the guardians' and the Darkloid's phase; the Darkloid healing by his dark
ring. On BN5's nine boards, with their own teams, it needs more phases than
BN5's par, by a ratio that changes with the missions (BN5's par over the
planner's median: 0.50 at mission 1, 0.58 at 2-3, 0.59 at 4-5, 0.79 at
6-7, 0.75 at 8-9). A generated board's par is the planner's median times
its act's ratio (`PAR_SCALE`): BN5's own yardstick at that stage.

### Special routes

A sealed route is a region behind the board's highest barrier holding a
gate panel: its key off the main way, its region a detour of phases. The
design: the gate leads to a second Darkloid, and beating him turns the
act's next route (docs/META.md, 7) toward another area or the deep (or, in
the separate mode, toward another last boss). The prototype places it (40%
to 53% of boards from act 4) and does not yet play it.

### How it reads as BN5's own

BN5's units and numbers throughout: its panel kinds and their placement,
its 3x3 Darkloid block and ringed DarkHoles, its guardians, Order Points 3
to 8, its commands' shapes, its par tiers. The board stands on the net
area the act is dressed as (a BN5 area's maps where BN5's ROM dresses it),
its dark panels and units drawn from BN5's ROM at run time, never kept.

## The prototype's measures

`python3 tools/liberation.py --seeds 30 --runs 12` (acts side by side on
all cores, about an hour for the late acts with the command rule's
plays); `python3 tools/liberation_bn5.py` with BN5's ROM for its nine and
the par's yardstick. BN5's own boards, measured the same way, stand
beside them:

| Act | BN5 | Kept of tried | Rejected most for | Panels | Fields | Fill | Dark | Dense | Par (BN5's) | Two under / at / over par | Close calls | Ways to the first holes |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 2 | 1 | 30 of 70 | commands 24 | 111 (100) | 70% (72%) | 28% (22%) | 42% (33%) | 3.6 (3.6) | 9.9 (7) | 8 / 43 / 49% | 41% | 1.3 |
| 3 | 2-3 | 30 of 127 | commands 57, barrier 26 | 194 (175-220) | 78% (74-82%) | 27% (23-28%) | 57% (43-48%) | 3.9 (3.5-3.9) | 14.1 (9) | 14 / 34 / 52% | 43% | 2.2 |
| 4 | 4-5 | 30 of 150 | commands 74, barrier 30 | 222 (220-237) | 94% (97%) | 40% (43-46%) | 66% (51-61%) | 4.9 (5.5-6.0) | 13.3 (9-10) | 14 / 38 / 48% | 46% | 2.8 |
| 5 | 6-7 | 24 of 360 | commands 105, barrier 103, situation 94 | 232 (217-248) | 94% (94-97%) | 40% (43-48%) | 68% (59-60%) | 5.0 (4.8-5.4) | 13.0 (10-12) | 19 / 35 / 45% | 43% | 3.4 |
| 6 | 8-9 | 30 of 297 | commands 106, barrier 88 | 224 (222-223) | 94% (88-99%) | 40% (46-48%) | 66% (50-64%) | 4.9 (4.1-5.1) | 12.6 (14-16) | 11 / 44 / 45% | 45% | 2.8 |

(Fields: panels in a 2x2 block of floor. Fill: the floor's share of its
box. Dense: dark panels around a dark panel. Close calls: the planner's
decisions whose second-best choice was within 10% of its best, a measure
of real choices. Ways: distinct orders of the first two DarkHoles over
the plays. A situation: a command's situation could not be placed.)

Each command's worth on the boards kept, the phases its Navi saves over
himself without it, the same plays paired: TomahawkSwing +1.1 to +1.7,
ScrenDiv +0.5 to +1.7, LifeMelody +0.2 to +0.9, NumberCheck +0.3 to
+1.0 (and 1.1 to 2.6 items taken a play with no battle), SneakAttack +0.3
to +0.5 (100 to 240 HP struck off a play), KnightMan's StoneBody +0.2 to
+0.8, MegaMan's LongSwrd +0.15 to +0.9. What it says:

- **The boards read as BN5's.** Their shape and their dark keep to BN5's
  bands (the pictures in the pull request), and route choice opens up
  from act 3: two to three and a half ways to take the first DarkHoles.
- **Every command earns its Order Point where it is kept**, and the rule
  costs boards: it keeps between one in two and one in fifteen of those
  tried, most often for a utility command the planner had no use for. On
  every board tried (before the rule) TomahawkSwing alone pays everywhere
  (+0.6 to +1.2); ScrenDiv pays early; SneakAttack, NumberCheck and
  LifeMelody pay where their situation is, little elsewhere. The
  situations are what make them, as BN5's hand-made missions do.
- **The par sits above BN5's in acts 2-5 and below in act 6.** It is the
  planner's median times BN5's own ratio at that stage, so it follows
  the generated boards, which are a little larger early and lack BN5's
  late maze of barriers (59 on mission 9 against about 5 here).
- **Left to tune:** the late acts' rejections (a barrier wall that cuts
  a field of two-wide ways, a cache's clean panel), the late maze of
  barriers, and the cost of the command rule in the engine, where its
  plays run as the layer is made (a precomputed check, or the rule's
  plays on a worker, if it is too slow on the 3DS).

## Building it

A staged plan, each stage its own pull request:

1. **The board in the engine** (the director's): the generated layer's
   floor as the board, its state in C from the layer's seed (reproducible
   as a layer is), units as BN6's map objects with BN5's and BN6's
   overworld sprites (both already used), dark panels drawn from BN5's
   board tiles read at run time (their place in BN5's ROM to be found), the
   command menu and the phase banners in BN6's own chat boxes and PET look
   (BN6 has no board screen, so the drawing is ours). Risk: the camera and
   cursor over a map BN6 walks; cost: large.
2. **The liberation battle in BN6's engine:** a three-turn limit on BN6's
   own battle loop (BN6's BattleState keeps the turn byte at +7 too,
   assumed from bn6f), the field's columns by the dark around the Navi,
   the one-turn win's report to the board. Risk: small; BN6's battles
   already end by the director's hooks.
3. **The team:** a Soul's Navi on the board, his battles as MegaMan in his
   Soul from the first turn with his own HP (the Souls' code is in BN6's
   engine: docs/MULTIROM.md, Transplanted Souls).
4. **The Darkloids:** the run's guardians first (already in BN6's engine);
   BN5's Darkloids (BlizzardMan, ShadeMan, CloudMan, CosmoMan) moved as its
   Navis were, later.
5. **The separate mode and co-op:** a setup option; co-op over the
   server's channels, the board shared turn by turn, each battle its
   player's own (no linked cores needed). Risk: the online rules of the
   common ground (docs/ONLINE.md).

### Stage 1, as built (23 October 2026)

**Where a dark layer stands.** An act's second layer, from the second act,
behind one of the two ways its fork offers in Lan's HP (the act's own area
at the pink pad, or the other way): two acts in three that have a fork hold
one, either way by a coin of the run's seed (`run_dark_area`,
`src/core/run.c`). Nothing is saved for it: the fork is worked out again
from the seed (the plan's area, and the other way as `run_route_alt`
offered it), so a CONTINUE knows. MegaMan says so at the way's portal
("Part of it's gone dark. Dark panels,like Nebula's!"), and the act's card
says "A layer here has gone dark" in its motto's place, in Nebula's purple.
The short net meets one in acts 2 and 3, the endless net in acts 2 to 4.
`--dev dark` makes every act's second layer from the second a dark one.
With BN5's ROM (without it there is no dark layer, nothing to mark) the
way's portal reads dark at a glance before MegaMan steps on it: its own
colours where it is drawn (the pink pad's magenta, a link square's glass)
take the shades of BN5's dark panels' swirl, palette 9 read from BN5's
ROM and stepped on BN5's timing (`BN5_LIB_DARK_FRAMES`: at rest 120
frames, then 16, 16, 32, 16, 16), its lightest the ramp's middle, its
darkest its end (`src/director/liberation_way.c`). The frame alone is
recoloured, within the portal's zone (its cells and its art, `lanhp.c`),
placed by BN6's own camera (eToolkit +0x0C's render point), so BN6's
palettes and their scripts, and the sprites over it, are as they were;
with MegaMan at the portal the mark goes and his sprite stays whole.

**The layer.** `layer_generate` with `layer_make_dark` (`src/net/net_gen.c`)
builds the layer's floor, flat, its arrival and exit, and nothing else of a
normal layer (services, data, bystanders, set pieces), so it gives up the
second layer's Net Dealer and Recovery Mr. Prog: the price of the way.
`LAYER_MAKE` 97.

**The board** (`src/director/liberation_board.c`, ROM-free, unit-tested):
the layer's floor, one cell a panel (32 world units), from the arrival's
reach; its kinds placed by `tools/liberation.py`'s rules from the layer's
seed: the start zone clean (two panels' walk), the Darkloid's 3x3 at 60% of
the farthest walk or more, the DarkHoles in whole rings of floor, a third of
the walk on, six apart and four off the Darkloid, each ring dark and its
guardian on it (BigBrute the first); dark panels thickening with the walk
and gathered by one majority pass; BonusPanels at dead ends; items in the
dark's corners first (one of Order Points always, then hearts and Order
Points); roaming guardians from act 3. A smaller team than BN5's meets a
DarkHole less for every two Navis short and dark five points thinner each
(MegaMan alone: one DarkHole in act 2, two in acts 3 and 4, three in 5
and 6). Rejected and the next seed tried as the prototype's guarantees say
(nothing to liberate in the first phase, a guardian within three of the
start, no room for the Darkloid or the holes); a layer whose eight seeds
fail is made plain. Over 30 layers an act: 190-240 panels, 40-47% dark.

**The par** (stage 1's estimate, `lib_par`): the dark panels on the way to
each DarkHole in turn, nearest first, then to the Darkloid, at two a phase
(one, and the one-turn win's eight now and then), with a phase each for a
hole's guardian, the hole and the Darkloid, and a rest: act 2 12 (8-21), act
3 20, act 4 15, acts 5 and 6 24. The prototype's planner, for teams, comes
with stage 3.

**The course** (`src/director/liberation.c`): after the arrival, Lan and
MegaMan name the board ("Like Nebula's dark areas,Lan!", its DarkHoles and
par), then PLAYER PHASE. MegaMan walks the clean panels as BN6 walks him,
as BN5's Navis walk its board (AGENTS.md: no movement of the engine's):
every panel not walked as the board is made (dark, an item's, a
DarkHole, the Darkloid's ring and his panel) is fenced with walls of BN6's
own on its cells that meet another panel's floor (`liberation_fence.c`).
A wall whose value has bit 7 set is skipped while event 0x1640 + its low
bits is set (Rush's gaps' walls, docs/ROM_DATA.md), and the fence's walls
name one of two: 0x16A0, never set, where the wall stands, or 0x16A1, set
each frame (entering a map clears it), where it is skipped (the Link Navi
obstacles' walls take 0x1640-0x1647 and Rush's gaps' 0x16BE-0x16BF). As a panel is
liberated, or darkened again, its walls are switched between the two in
BN6's coordinate data in RAM (`BN6_MAP_COORDS`, which BN6 reads in place
and decompresses again at each map entry, after a battle too), as BN5
raises and lowers its border cells' heights per panel
(`BN5_LIB_EDGES`): no board has more fences than BN6's events allow,
however many of its panels are dark. A panel the map left without floor
at its middle (netmap's legal.c) is off the board. A beside the panel he faces (the one
his facing points at, a little ahead of him) opens the command menu on it: **Liberation** (a battle of BN6's: its
virus battles' easier half with two dark panels round it or fewer, the
band with three to six, the next act's band one version up with seven or
eight, a DarkHole or its guardian; won, the panel is liberated, and the
eight round it where no second Custom screen opened, BN5's one-turn win),
**LongSwrd** (an Order Point: two panels in a line, one battle) or
**Pass** (half his max HP back, BN5's guides). Item panels give Order
Points (3 to 8) or a heart (30% of max HP); a BonusPanel stepped on spins
BN5's roulette's four we can give (HP full, five Order Points, the nearest
guardian deleted, no harm for a phase). Then DARKLOID PHASE: each guardian
steps toward him over the dark (BigBrute two and hits all in one,
TinHawk three and one in three, Bladia two, darkening what it lands on),
the Darkloid hits him within his reach (two or three); the board's harm
never takes his last HP (alone, a deletion off a battle would end the run
unfought). A DarkHole sends its deleted guardian back the Darkloids' phase
after next (BN5's the next: with a team another Navi strikes the hole
meanwhile). Every DarkHole down, the Darkloid's ring can be liberated, and
the Darkloid fought from it: a Navi of BN6's whose HP fits the act's band,
no act's guardian (`run_darkloid`), in his own battle. Deleted, the exit
opens where he stood and the par's prize is given: two under par a chip a
Server's win pays, at par one a tier lower, over par one lower still
(`install_lib_prizes`, `src/layer/layer_objs.c`); the board's battles pay
BN6's own rewards beside it. No random battles on the board.

**On the map, first look.** The units are the layer's own map objects (`OBJ_UNIT`:
HeelNavis for the guardians, the Darkloid in his own overworld sprite),
moved by their objects' places. The dark panels are the layer's own floor
recoloured as it shows (`src/director/liberation_draw.c`), in Nebula's
purple, the Darkloid's ring redder, a DarkHole near black, a BonusPanel
gold, the panel MegaMan faces lit: pixel by pixel in the frame, from the
map layer's scroll (BG1's: `x + y + 256` and `(y - x) / 2 - 256` of the
world at the picture's middle, mod 512, read on captures as MegaMan was
moved), a pixel in a sprite's colour or under a chat box left alone; BN5's
own board tiles are not read yet. The phase, par and Order Points in the
corner, the banners and the command menu in BN6's PET look
(`src/gfx/pet_look.h`); every word in BN6's chat boxes
(`src/director/liberation_words.c`).

**On the map: BN5's own look** (`src/director/liberation_look.c`, with
BN5's ROM). A dark layer is drawn in BN5's Liberation map 0x86:00, as an
older net's area is (a held area of BN5's, never a run's dressing): its
floor learned from the board's own panels, which the mission's record lists
(`area_src_board`: the map's walls cover its walkways alone, the board
moving its units itself), at the record's origin (`+6`, `+7`, the panels'
edges at X = -o6 and Y = -o7, mod 32), not its pads' look; its tiles and
its board's colours (`BN5_LIB_PALETTE`, loaded over the map's as BN5's
board loads them: the frames grey) copied in. Each panel's kind lies over
it on BG2 as BN5 lays it: one of BN5's twelve looks (`BN5_LIB_LOOKS`,
picked by the kind as `BN5_LIB_LOOK_OF` picks it: clean cyan, the dark
swirl, the item's stud, the DarkHole's pit, the BonusPanel's "?", the
Darkloid's ring a grey grid until the DarkHoles fall, the start its map's
floor alone), its 8 x 4 entries at the panel's centre less 32 and 16 pixels and 8
down (fit on the dumped map: the looks over the frames' dark middles, 1.6%
over the void), their tiles copied from BN5's ROM to where BN5's board
copies them (tile 864 on, VRAM taking words), and the board's palettes
animated as BN5 times them (traced): the dark panels' swirl (9) at rest 120
frames, then three frames down and up, 216 frames round; the DarkHoles'
(10) four frames there and back, 20 frames a step; the barriers' glint
(11) every 132 frames; the BonusPanel's breath (12) six frames there and
back. BN6 reads the tile map in place and builds each BG's
screen map in IWRAM, copied to VRAM every frame (`src/emu/bn6_map.h`): a
look goes into the map's layer 1, and BG2's screen map is made from it for
the picture a while after the looks change (the map's layer 1 and BG2
cleared first, as BN5's BG2 holds the looks alone; again after a battle,
whose return loads the tile map afresh). A panel freed blinks as BN5's
do (traced): its old look while the board fades back in (27 frames), then
clean and old by turns every 4 frames, 14 times (`BN5_LIB_BLINK`,
`BN5_LIB_BLINKS`), then clean. The panel MegaMan faces, where A
takes a command there and while the menu is open, is outlined by BN5's
own sprite (list 7's 15, `BN5_LIB_OUTLINE`, read from its ROM: its
yellow blink, 24 frames lit and 24 not, from the moment he turns to it).
The backdrop is the mission's: map 0x86:00's own animated tiles in its
purple (`BN5_LIB_BACKDROP`), as BN5's map group 0x86 loads them while a
mission is on (its LoadBGAnim and LoadGFXAnims each choose a mission's
table by `BN5_LIB_ON_MAP`; `src/gfx/xbackdrop.c` reads both shapes and
takes the mission's). The mission's list moves the board's palettes too
(banks 9-12, `BN5_LIB_ANIM_*`, their writes moved to BN6's palette
buffer): BN5's own scripts are their one writer, so the host map's own
(the Central Area map the layer takes over, whose bank 9 flashed teal
and red every 16 frames) no longer runs there. BN5's own pieces are drawn over it from BN5's ROM
(`src/director/liberation_pieces.c`, its tiles through
`src/gfx/xtiles.c`), each where BN5 lays it (traced): the Order Points
gauge at the top right (its six OBJs, the frame's tiles from its pieces,
the bar masked by the count as BN5 masks it, filling from the right),
the L view while L is held ("PHASE" and "TRGT PHASE" with their numbers,
without BN5's corner arrows: BN6's camera stays on MegaMan), the command
window in BN5's frame on BG0's place with its cursor (its words in BN6's
menu font, white, a command not to be taken in the paler colour of
BN5's letters' edges), "LIBERATE SUCCESS!" or "1 TURN LIBERATE!" squashed in and out
over the board once it has faded back in (27 frames, then 58; MegaMan's
words wait for it), and each item freed popped over its panel in its own
icon until the words of it end, BN5's jingle (0x16D) as they begin.
The phases' banners are BN5's (traced, How the board looks and moves):
the band blended over the board (`xtiles_blend_rect`, the colour whole
and the board at half, as BLDALPHA's 16 and 8), the words' pieces at
BN5's places, sliding in with BN5's sound and out on its frames, 104 in
all, begun once nothing else shows (a CONTINUE's act card first); the L
view's panel behind its phases the same. The guardians stand in BN5's
own sprites (list 6's 83-85, copied into BN6's list 7 at numbers Gregar
leaves on a placeholder, `src/layer/xnavi.c`), the Darkloid in his own.
BN6's own camera moves as BN5's (`src/director/liberation_camera.c`):
held, its mode is 0 (it no longer copies MegaMan's place) and its place
is written each frame, panning at BN5's pace to the Darkloid at the
opening (his entrance: held there a second, then back before the
briefing) and before DARKLOID PHASE, to each guardian as it moves in his
phase, back to MegaMan before PLAYER PHASE; under L the D-pad moves it at
BN5's 4 and 8 pixels a frame, kept over the board; let go, it follows
MegaMan again. MegaMan walks as BN6 walks him throughout: it moves only
while he stands. The board's music is BN5's: its song 0x22 the dark layer's map's own
(`lib_board_song`), 0x1C at the briefing, 0x22 again at PLAYER PHASE
01, copied into BN6 after the areas' songs and themes (`xsong.c`, each
keeping its place) and started by BN6's PlayMusic
(`liberation_sound.c`).
Without BN5's ROM the board keeps stage 1's first look: the act's own floor
recoloured in Nebula's purple, pixel by pixel. The units, the corner, the
banners and the menu are as above (BN5's own still to come).

**Checked:** `tests/test_core.c` (`test_liberation_board`: 150 dark layers
over acts 2-6, the same board twice from a seed, the Darkloid's ring whole,
each hole's ring dark, the start clean, no guardian by it, nine boards in
ten made); the autopilot plays the board (`liberation_auto.c`: the dark
panel that brings it nearest the next target, Pass below two fifths of its
HP): `CYBERWORLD_AUTOPILOT=1 build.py shot --scene emu --run-depth 5 --dev
dark,god,onehit --frames 48000` (the autopilot walking MegaMan with the
pad as a player does) liberated seed 1's act 2 board in 23
phases against its par 14 (the autopilot never wins in one turn), deleted
CircusMan, took the prize (Recov80 *) and the exit, and went on through act
2's guardian, home and act 3's dark layer.

**Across a CONTINUE** (`src/director/liberation_save.c`): the board's
panels, units (where each stands, which are deleted and when a DarkHole
sends one back), the phase, the step and the Order Points are saved
beside the run's checkpoint as `run.lib` (docs/SAVES.md) and taken back
once the checkpoint's state has loaded on the layer its seed made again
(the same layer only: another build's or area's starts afresh, as the
layer does). What was being shown begins again: a banner from its start,
a talk's step its next (no chat box is open as a checkpoint is saved),
the menu closed. Checked: the autopilot's board quit at PLAYER PHASE
04's banner (SIGTERM, the quit's checkpoint) and continued: the same
board, phase 4, the panels it had freed clean and its Order Points as
they were, the banner shown again once the act's card was gone.

**Left for later:** the L view's corner arrows and its keys' help, the
Darkloid's words at his entrance, each unit's HP under it (our guardians
have none on the board yet), the mission's end in BN5's words (the target
and the phases taken), the
other eight missions' maps (stage 1 draws every dark layer in mission
1's); barriers, keys and sealed
routes; chips and zenny in item panels; the BonusPanel's chip; the
planner's par; the economy report and pacing for dark layers; the second
screen's board; stages 2 to 5.

Moving BN5's own board code (0x12000 bytes of Thumb, its tables, text and
graphics) into BN6's core instead was weighed: its exact rules for free,
but most of it calls BN5's overworld engine at other addresses, and the
generated boards need none of BN5's missions. Its code is read for the
rules where the guides are unsure (the commands' shapes, the Darkloid's
heal).

## The owner's decisions

Decided on 23 October 2026, as recommended: "a liberation as a dark layer
in the endless net first, teammates lent by the run's BN5 Souls and fought
as MegaMan in their Soul, co-op later over the server."

1. **Where it stands:** a dark layer in the endless net, first. (Not
   chosen for now: an act's climax; a separate mode first. A separate mode
   stays possible later, on the same generator.)
2. **Co-op:** later, over the server, the board shared turn by turn and
   each battle its own player's.
3. **Which Navis:** the run's BN5 Souls lend their Navis, BN5's own
   liberation team; a teammate's battles are fought as MegaMan in his Soul
   from the first turn, with the teammate's own HP. (Not chosen: a
   teammate as a playable Navi of his own.)

Still open:

4. **BN6's Crosses as teammates** (commands of BN5's kinds for BN6's
   Navis), for runs without BN5's ROM or beside the Souls: not decided;
   until then a run without BN5's ROM meets no dark layer, or meets it with
   MegaMan alone.
5. **The rewards:** BN5's tiers (a chip two under par, a chip at par,
   zenny after) in the run's economy (docs/ECONOMY.md), or the route a
   sealed region opens as the only special reward. Stage 1 gives BN5's
   tiers as chips, a Server's prize two under par down to a lesser one
   over it (Stage 1, as built), reasoned with the game-design skill: the
   dark way is a costed choice (the second layer's dealer, heal and
   Mystery Data given up for the board's battles and its prize), and the
   par's tiers make pressing on pay. The owner's call stands open.
