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
of the generator with its measures, and stages 1 and 2 in the engine: the
dark layer and its board, MegaMan alone (Stage 1, as built, below), and
BN5's liberation battle on BN6's own battle loop (Stage 2, as built).

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

### The par's prize (ROM, play)

The Darkloid deleted, the mission's end (the state's routine 0x18) shows
SUCCESS!, then its step 2 picks the rewards' row whose phases hold the
phase taken (`BN5_LIB_REWARD_PICK`) and opens the end's message for its
tier (`BN5_LIB_END_TEXT`, docs/ROM_DATA.md's BN5's Liberation Missions'
rewards). The team's leader speaks (Colonel in Team Colonel): the target
and the actual phases, then the Officials' reward, then the chip or the
zenny given by the message's own command. Played in romlab on mission 1
(par 7) at 5, 7 and 8 phases: BlizMan SP B ("Good job,everyone!"), BlizMan
B ("Good Job."), 1000 Zennys ("Too bad you couldn't make it."; the zenny
`0x0200299C` 0 before, 1000 after).

| Mission | Two under par or better | Within par | Over par |
| --- | --- | --- | --- |
| 1 | BlizMan SP B | BlizMan B | 1000 z |
| 2 | ShadeMn SP S | ShadeMan S | 2000 z |
| 3 | CloudMn SP C | CloudMan C | 3000 z |
| 4 | FullCust * | AntiNavi M | 4000 z |
| 5 | CosmoMn SP C | CosmoMan C | 5000 z |
| 6 | Z Saver Z | Colonel C | 5000 z |
| 7 | BlakWing W | ShadeMn DS S | 5000 z |
| 8 | Muramasa M | CloudMn DS C | 5000 z |
| 9 | Anubis A | CosmoMn DS C | 5000 z |

The two chips are a better one and a lesser one: the Darkloid's SP chip
and his plain one, from mission 7 a Mega chip and his DS chip. The zenny
is 1000 a mission, 5000 at most. No row holds a phase: 100 zenny (the
lookup's default; every mission's last row runs to 99).

### The board (ROM, play)

A board is a set of panels on a square lattice, following the map's floor:
platforms and one-wide walkways, as the net is (docs/LEVEL_DESIGN.md). Its
kinds (`BN5_PANEL_*`):

- **Clean** (0): the team walks it.
- **Dark** (1): liberated by a battle. Guardians walk dark panels.
- **Item** (3): looks dark and holds an item or a trap.
- **BonusPanel** (9, "?"): a roulette (below), at the ends of stubs;
  freed by the Liberation command without a battle or the Navi's action
  (**ROM**, **play**: BonusPanels as BN5's, below).
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
all viruses deleted within three turns (**ROM**, read in its battle loop:
Stage 2, as built, below: BattleState +7 counts the Custom screens opened,
TURN n START! and FINAL TURN! as each turn's fight starts, TIME UP! and
LIBERATE FAILED! where the third turn's Custom screen is asked for).
Deleting them within the first turn liberates every panel around the Navi
too (**text**; the ROM's win shows 1 TURN LIBERATE! where +7 is 1). Which columns the field gives depends on which of the
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
and from the BonusPanel's roulette (5, **ROM**, **play**).

### Items, traps and BonusPanels (ROM)

An item panel's item (`BN5_LIB_PTR_ITEMS`, its kinds' table
`BN5_LIB_ITEM_KINDS`): a heart (HP, its amount times ten: 30-900, rising
with the missions), a BarrierPanel's key, Order Points (2-3), zenny (its
amount times 100), BugFrags, a chip. Traps (`BN5_LIB_PTR_TRAPS`) from
mission 5: damage (50 in missions 5 and 6, 100 later) or paralysis. A
BonusPanel's roulette (`BN5_LIB_BONUS_ROLL`) starts on one of five slots
from the random number and is stopped by A; its five slots are each
mission's records in this order (**ROM**, **play**): the team untouchable
for a phase, the acting Navi's HP full, five Order Points, one of the
mission's four chips, the nearest guardian deleted (none: the Darkloid a
fifth of his max HP down). The guides' list was right; its order and what
each does exactly are traced below (BonusPanels as BN5's).

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

### The Darkloids' phase as BN5 shows it (play, ROM)

Recorded in romlab on mission 1 (both Navis' Pass taken, then MegaMan's
place, his unit's world place at `0x0203E000` +0x1C and +0x20, poked
two panels from a BigBrute, and in a second run two from BlizzardMan),
frame by frame, each OBJ traced to its sprite and each sound to its
caller (docs/ROM_DATA.md, BN5's Liberation board: the Darkloids' phase):

- **The order.** The camera pans to the Darkloid, DARKLOID PHASE, and he
  acts first; then the camera visits each guardian in turn (its pan 22
  frames, its rest 15) and back to the Navi for PLAYER PHASE. A
  guardian with no Navi near does nothing at all.
- **The Darkloid's heal.** Right after the banner where dark panels stand
  round him: rising lines over him (effect 0x78), sound 0x8A, his HP up
  at once (204 to 400 in the recording).
- **A guardian's move is a warp, not a walk.** All its steps at once: a
  puff and then a beam's lines where it stands (sprite list 7's 59, 2
  frames each; the guardian gone with the puff), sound 0xB4, the camera's
  pan to the panel it takes, then the lines and the puff there, sound
  0xB4 again 46 frames after the first, the guardian back 4 frames into
  it, sound 0x94.
- **A strike in five beats.** Where a Navi stands in the unit's reach:
  every board panel in the reach turns red (look 0x0D, BG palette 7; on
  9 frames, off 7, on 9, off 7, then on, sound 0x91 as each light
  comes); 68 frames on, the unit's line in a chat box, its attack's name
  ("Grrrowl! BeastBreath!" with no face for a BigBrute, BlizzardMan's
  with his mugshot); once it closes, the unit's strike animation
  (BigBrute's 12-27 frames after the box closed, BlizzardMan's 19-52),
  the blow on the Navi with sound 0x6B (0x6E where nothing harms him):
  BigBrute's breath of fire over him (sprite list 7's 56: rising,
  burning, falling, 27 frames), BlizzardMan's Snowball falling on him;
  the harm in yellow digits over the Navi (BN5's HP digits, palette
  `0x087E4678`, 29 pixels over his feet) as his HP under him drops (at
  the blow +30 for BigBrute, +21 for BlizzardMan), held 60 frames; the
  red put out (123 and 127 frames after the box closed) and the next
  unit's pan.
- **The units stand animated.** The guardians in their standing
  animations by the way they face (list 6's 83-85: 0-7 by the eighth,
  two or four frames looped), facing down-left on the board; their
  strikes their own animations (BigBrute's 33-40, TinHawk's 25-26,
  Bladia's 25-28).

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
  ring until the DarkHoles fall); then three more: the start's (kind
  0x0C), every entry blank, as a team starts on its map's own walkway
  ends or (mission 1) its lilac pad, never on a panel, and two diamonds
  on palette 7 whose kinds (0x0D, 0x0E) no record lists; palettes 8-12
  the board's own. Each
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
  liberations' battles BN5's virus battle song 0x15; a battle won in its
  first turn ends on 0x24 in place of the win's usual song, and the
  mission won plays 0x18 over the Colonel's tally of its phases, once
  (its routine 6; docs/ROM_DATA.md, Where BN5 plays its songs).
- **The camera** pans to the unit about to act: to the Darkloid before
  DARKLOID PHASE (about 20 pixels a frame, then a quarter of what is left
  each frame, some 32 frames, the band 3 frames after it stops), to each
  guardian as it moves and back to the Navi before PLAYER PHASE; at the
  mission's opening it tours the board to the Darkloid. In the Darkloids'
  phase each pan takes some 22 frames and the camera rests 15 on the unit
  before its turn. Under L the D-pad moves it, 4 pixels a frame, 8 with B
  ("MoveCam(F)"), eight arrows at the picture's edges (their second look 8
  frames of every 32) and a cross at its middle.
- **The guardians on the board** are sprites of BN5's list 6: 83 BigBrute
  (a red, spiked beast), 84 TinHawk, 85 Bladia, each standing in its
  animation by the way it faces (down-left, 5) with its shadow; how they
  move and strike: The Darkloids' phase as BN5 shows it, above.

Still to trace: the window's font, when an item's pop shows and goes,
the sounds of a DarkHole closing and of LIBERATE SUCCESS, the Navis' own
board sprites beside MegaMan. (The other eight missions' maps carry the
same sets, but for their backdrops and colours: Each mission's own look,
as built, below.) The mission's end, the L view's keys and the Darkloid's
first word are traced below (The board's words and banners as BN5's, as
built).

So the engine's board is to be BN5's: its overlay tiles, diamonds and
palettes read from BN5's ROM at run time (never kept), its pulse, its
outline, its dissolve, its banners and its item icons, over a floor of
the mission maps' panel grid where BN5's ROM is read. The dark layer's
first build (stage 1) tints BN6's floor instead and has none of the
animations; its rework follows this section.

### BonusPanels as BN5's (ROM, play)

Traced in BN5's own board (`tools/romlab`'s BN5 lab, mission 1 from its
board's state, the faced dark panel made a BonusPanel in RAM and the
mission's first BonusPanel record moved to it; each slot forced by its
byte and played to its end; `src/emu/bn5_liberation.h`'s
`BN5_LIB_BONUS_*`):

- **Freed by command.** A BonusPanel is not walked (the walk mask of
  MegaMan's unit, 0x1001, is clean and start alone); facing it, the
  command window offers Liberation, which frees it **without a battle**:
  the player's phase's step `0x0804C468` sends kind 9 to the roulette's
  steps, and the Navi's actions left (his unit's +2) are untouched: the
  same Navi's menu opens again after it.
- **The lead-in:** MegaMan's line ("A BonusPanel!"); the panel dissolves
  (19 frames) and shines (31); three white flashes over the whole picture,
  sound 0xD7 each, two frames white then down by 2/16 a frame, the second
  26 frames after the first, the third 10 after that; the band opens 30
  frames after the third.
- **The roulette** (its state `0x0200ABB0`, ten routines from
  `0x08054EF4`): the phase banner's band (BG0 filled with one tile of the
  phase words' palette, WIN1, blended over the board at half) opens in two
  frames to rows 56-87; 16 frames on, sound 0xD6, the slot's icon slides
  in from the right, 32 pixels a frame, to the middle (x 120, y 72); 8
  frames later "PRESS A BUTTON" shows under it (y 98) and A is heard. The
  slot steps every 4 frames (each BonusPanel record's u16) from a random
  first one, sound 0x80 at each step once the icon is in place; the icon
  shown is the slot's, one frame behind it, so the slot stopped on is the
  one on screen as A is pressed (sound 0x92). Its name replaces the prompt
  for 88 frames, then the icon slides out to the left, the band shuts in
  two frames, and the slot is given (`0x0804C754`'s five handlers).
- **The icons and words** are BN5's sprites of list 9: the icons
  0x3F, 0x3C, 0x3D, 0x3E and 0x40 (`BN5_LIB_BONUS_ICONS`: a flexing
  green Navi, a heart, an Order Point, a chip, a skull), the words list
  9's 25 (`BN5_LIB_PHASE_WORDS`' sprite): animation 4 "PRESS A BUTTON",
  6-10 the names "1PHS INVIN", "HP Rec!", "Order Pt!", "BattleChip!",
  "Major Hit!".
- **The slots, in BN5's order** (each mission's BonusPanel records,
  `BN5_LIB_PTR_BONUS`, the same five in all nine):
  0. no harm on the board for the rest of the phase: every Navi's unit
     flagged 0x80 (sound 0x93, the team tinted green), cleared at the
     next player's phase;
  1. the acting Navi's HP full: an item find of a heart of 100 (x 10),
     his alone (30 of 100 to 100; Colonel's 50 of 200 untouched);
  2. five Order Points (3 to 8), an item find;
  3. a chip, an item find: one of the mission's four (mission 1: four
     standard chips, one of them a "*" code), their weights alike (0x20);
  4. "Damages the closest enemy the most!": the nearest guardian (BigBrute,
     TinHawk or Bladia, by squared distance from the Navi) deleted, the
     camera on it ("Gyaaaaahh!!", its explosion); with no guardian left,
     the nearest Darkloid loses a fifth of his max HP (400 to 320, 1000 to
     800, 300 of 400 to 220: his max's, not his HP's).

### Not found, and assumed

- The commands' shapes in the ROM: the board's command code
  (`0x0804B000`-`0x0805D000`) was located, not read through; the shapes
  are the guides'.
- The record's +9, what the Darkloid's battle does past three turns: not
  traced. (The BonusPanel's slots: traced, BonusPanels as BN5's.)
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

## The quality bar

The owner's question (23 October 2026): what makes a liberation worthy of
the run, not just more of it. Reasoned with the game-design skill: the
experience stays the decision's ("I see the whole dark area and plan my way
through it; every phase I spend is the better reward slipping away"), the
dialectic the run's own, and the patterns a cumulative-modifier staircase
(one lesson a rung, introduce, develop, twist, test), an honest telegraph, a
branching map whose branch is variety, and a hand-made unit set laid out by
the generator. Each rule below is held by the generator
(`src/director/liberation_board.c`), its planner
(`src/director/liberation_plan.c`) and the prototype (`tools/liberation.py`):
a board that fails one is rejected and the next seed tried, and the tests
(`test_liberation_board`) and the prototype print the measures over
hundreds of boards.

1. **One dialectic, every phase.** The run's *press on or prepare*, at the
   board's scale: liberate toward a DarkHole, or take Order Points, a heart,
   a rest, a guardian out of the way, a key, a detour for loot or the
   route; the par's prize against HP and Order Points. **Measured:** the
   planner, a fair player, counts each phase's sensible moves, those of a
   different purpose worth four fifths of the best or more. A purpose is
   what the move is for (the DarkHole it heads to, the Darkloid, an item, a
   key, the route, a guardian, a rest), its command (Liberation or
   LongSwrd), and for a panel whether it is safe or bold (four or more dark
   round it: the one-turn win's gamble, BN5's own skill). A phase with one
   is dead. A strike at a DarkHole or the Darkloid beside him is the board's
   beat and counted apart. **Rule:** no more than 55% of the phases on the
   way dead.
2. **The bare board is a puzzle.** The same plays with every reward
   stripped (Order Points, hearts, zenny, chips, BonusPanels) still decide:
   no more than 60% dead. The dark's shape carries it: fields, not
   corridors (a one-wide way's panel dark half as often, the loneliest dark
   thinned first), the guardians' reach, the DarkHoles' order.
3. **One new rule per act** (a staircase, not a slider). Each act asks one
   question more, the earlier ones kept:

   | Act | The new rule | Its question |
   | --- | --- | --- |
   | 2 | The basics (BN5's mission 1): dark panels, DarkHoles and their BigBrutes, the Darkloid, Order Points and hearts, BonusPanels | Which way through the dark, and when is an Order Point worth a LongSwrd? |
   | 3 | Barriers and keys, met safe: the sealed region (rule 6) | Is the route worth the phases? |
   | 4 | Guardians' reach: a roaming TinHawk, and a BarrierPanel across the way with a way round | Round his reach, through him, or fetch the key? |
   | 5 | The board fights back: trap panels on the way, each with a way round; Bladia darkening clean panels | HP or phases? |
   | 6 | The maze: a second barrier across the way, its key past the first (where the floor gives one) | In what order? |

   A run meets dark layers in acts 2 to 4 (`run_dark_area`); acts 5 and 6
   are `--dev dark`'s and the separate mode's to come. The size grows with
   the questions: a lone MegaMan's boards hold 14 dark panels beyond the
   DarkHoles' rings at act 2 and 26 at act 6, two DarkHoles to three, and
   the par from 12 to 21 (BN5's 7 to 16 for teams of two to five).
4. **Honest telegraphs.** Everything that can hurt or cost shows before the
   move. A trap is a panel of its own look (BN5 hides its traps in item
   panels; ours show BN5's paled dark), the par is said in the briefing, and
   the guardians' reach and next step are a function of the board
   (`lib_threat`: the HP the units would take off a Navi standing on a
   panel in the next Darkloids' phase, their steps as the phase takes
   them), which the planner reads as a player reads the board. No unit
   reaches the start. Drawing that reach (BN5 has no such overlay; the L
   view is the place) is lib-feel's to do.
5. **Every teammate has a niche.** Each command's shape has a board moment
   where it is the best move: the command rule (The generator) for the
   teammates in the prototype, NumberCheck's new niche the traps it
   disarms. In the engine, MegaMan alone, LongSwrd is taken half a time a
   play or more and is the best move in at most 40% of the phases: never
   useless, never the only answer.
6. **Routes that matter.** From act 3 every board holds a sealed region:
   five to sixteen panels cut off by a BarrierPanel of their own, nothing
   of the board's in them, in sight from the walk; its key in a dark panel
   two or more off the way; the route at its far end. Never required (the
   tests open every other barrier and reach every target without it).
   Its cost is measured: a player going for it takes 2 to 10 phases more
   than the par's player, else the board is rejected (cheaper is a gift,
   dearer a wall). Its reward is variety, not power: the act's next route
   (Special routes, below).
7. **Rewards feed the run, failure is graduated.** The par's tiers are
   BN5's (the owner's decision 5): a Server's prize two under par, one
   tier lower at par, zenny over it (The par's prize, as built); item
   panels add a chip of the layer's band (one at act 2, two later) and the
   act's zenny (BN5's amounts, 300 at act 2 to 700 at act 6), off the way:
   a detour's worth. Over par is a lesser prize, never a lost run. The dark
   way stays a costed choice at the fork: the second layer's Net Dealer and
   heal given up.
8. **Fun measured.** The planner plays every board 12 times for the par
   and the measures, 6 times stripped, 4 times for the route; the tests and
   the prototype print, act by act, dead phases, sensible moves a phase,
   each command's share of the best moves and LongSwrd's uses, the par's
   middle half, the DarkHoles' orders, the route's cost and what was
   rejected for what (The quality bar's measures, below).

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
| 3 | 2-3 | 165-215 | 3 | 3 | 1 (sealed) | 45-58% | 70-88% | 27% | 3.4-4.5 | 8 | 3 | BigBrute | 600, 2, 50 |
| 4 | 4-5 | 210-245 | 4 | 4 | 2 | 52-66% | 86-100% | 42% | 4.6-6.2 | 12 | 3 | + TinHawk, roaming | 900, 3, 80 |
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
4. **Barriers**, by the staircase (The quality bar, 3): from act 3 the
   sealed region's, a wall of one or two panels (a two-wide way walled
   across) that cuts five to sixteen panels off by itself (the prototype's
   smaller boards allow thirty), nothing of the board's in them, its key in
   a dark panel two or more off the way and the route at the region's far
   end; from act 4 one across a cheapest way, a way round it kept, its key
   off the way; at act 6 a second across the way, its key past the first
   where the floor gives one. **BonusPanels** at stub ends.
5. **Items** in the dark, corners and dead ends first: one Order Points
   item always, then hearts and Order Points; the act's chips and zenny
   off the way (a detour's worth). From act 5 **traps**: dark panels on a
   cheapest way that a way round, a dark panel or two longer, avoids, shown
   as traps (BN5 hides them in item panels).
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

The quality bar's rules add theirs: a board is rejected where more than
55% of its phases on the way are dead, 60% stripped of its rewards, where
its sealed region's route costs the route's player fewer than 2 or more
than 10 phases over the par's (the prototype: 1 to 8, scaled as the par),
where LongSwrd goes unused or wins more than 40% of the phases, where the
act's barrier or traps find no place, or where a unit reaches the start.

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

The engine's planner (`src/director/liberation_plan.c`, MegaMan alone)
plays each board 12 times the same way, with BN5's battle odds (the one-turn
win 75% less 5 a dark panel round it, the failure 4% plus 3.5 a dark panel),
the act's virus harm (14 to 40) and a run's HP for MegaMan (300 at act 2, a
hundred more an act: its HPMemory, docs/ECONOMY.md), resting where his HP
weighs more than a phase, never risking a battle whose worst harm would end
the run. The par is the median of its plays won (BN5's ratio is for teams;
a lone MegaMan is the planner's own yardstick until stage 3 brings the
teams): an act 2 board's par is 8 to 19, act 4's 12 to 31.

### Special routes

A sealed route is the region behind the sealed barrier (The quality bar,
6): its key off the way, its region a detour of 2 to 10 phases, the route
at its far end an item panel the player liberates. Found, it is the hook
the act's next route reads (`lib_route_found`). The design it feeds: the
route turns the act's next fork (docs/META.md, 7) toward a third way, the
deep's (the Undernet, as the short net's dark way goes) or another area's
guardian; after the endless net's act 4, whose next act has no fork, the
Nest's second guardian, a different last boss; in the separate mode,
another last Darkloid. **Not built:** keeping the route found for the run
(a bit in `Run`, saved: the board's save is stage 1's open item, and
`RUN_MAGIC` bumps with it) and `run_ways` offering the third way; the
board places it, prices it and finds it.

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

### The quality bar's measures, the prototype

The same staircase, barriers, traps and measures in the prototype, its
teams BN5's (`python3 tools/liberation.py --seeds 20 --runs 8`, an act a
core; the route's player's plays a quarter of the par's):

| Act | Kept of tried | Rejected most for | Dead decisions on the way | Sensible moves a decision | Par (middle half) | The route | Traps |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 2 | 20 of 160 | dead 81, commands 22, situations 36 | 46% | 1.6 | 8.7 (2.3 wide) | none | none |
| 3 | 2 of 240 | dead 106, barrier 94, route 19 | 51% | 1.6 | 11.5 (3.3) | +2.5 | none |
| 4 | 13 of 240 | barrier 147, route 35, commands 22 | 50% | 1.8 | 12.1 (2.7) | +2.4 | none |
| 5 | 2 of 240 | barrier 162, situations 26, holes 16 | 49% | 2.0 | 14.0 (2.1) | +3.7 | 4 |
| 6 | 5 of 240 | barrier 137, route 31, commands 21 | 50% | 1.8 | 13.0 (2.4) | +2.5 | 4 |

A team decides about as often a decision as MegaMan alone (one in two
dead), and its phases hold more decisions, one a Navi. The prototype's
floors, BN5's trees of platforms, seldom cut off a sealed region by a wall
of its own (three to thirty panels with nothing in them): from act 3 most
of its tries fail there, where the engine's layers, looped, keep 38 to 40
of 40. Its numbers stand for the rules' direction; the engine's for the
boards a run meets.

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
reach (the floor of the layer's map as built since: The board on the map's
floor, below); its kinds placed by `tools/liberation.py`'s rules from the layer's
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

**The par** (stage 1's estimate, `lib_par`, the dark panels on the way at
two a phase) gave act 2 12 (8-21); the planner's replaced it (The quality
bar, as built).

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
however many of its panels are dark. (A panel the map left without floor
at its middle, netmap's legal.c, was off the board; the board is made on
the map's floor since.) A beside the panel he faces (the one
his facing points at, a little ahead of him) opens the command menu on it: **Liberation** (a battle of BN6's: its
virus battles' easier half with two dark panels round it or fewer, the
band with three to six, the next act's band one version up with seven or
eight, a DarkHole or its guardian; won, the panel is liberated, and the
eight round it where no second Custom screen opened, BN5's one-turn win),
**LongSwrd** (an Order Point: two panels in a line, one battle) or
**Pass** (half his max HP back, BN5's guides). Item panels give Order
Points (3 to 8) or a heart (30% of max HP); a BonusPanel stepped on spun
four of BN5's roulette's slots (since BN5's own: BonusPanels as BN5's, as
built, below). Then DARKLOID PHASE: each guardian
steps toward him over the dark (BigBrute two and hits all in one,
TinHawk three and one in three, Bladia two, darkening what it lands on),
the Darkloid hits him within his reach (two or three); the board's harm
never takes his last HP (alone, a deletion off a battle would end the run
unfought). A DarkHole sends its deleted guardian back the Darkloids' phase
after next (BN5's the next: with a team another Navi strikes the hole
meanwhile). Every DarkHole down, the Darkloid's ring can be liberated, and
the Darkloid fought from it: a Navi of BN6's whose HP fits the act's band,
no act's guardian (`run_darkloid`), in his own battle. Deleted, the exit
opens where he stood and the par's prize is given, BN5's tiers (The par's
prize, as built, below); the board's battles pay BN6's own rewards beside
it. No random battles on the board.

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
BN5's ROM). A dark layer is drawn in a BN5 Liberation map (0x86:00 at
first; its own mission's since: Each mission's own look, below), as an
older net's area is (a held area of BN5's, never a run's dressing): its
floor learned from the board's own panels, which the mission's record lists
(`area_src_board`: the map's walls cover its walkways alone, the board
moving its units itself), at the record's origin (`+6`, `+7`, the panels'
edges at X = -o6 and Y = -o7, mod 32), not its pads' look; its tiles and
its board's colours (`BN5_LIB_BOARD_PAL_ACDC`, loaded over the map's as BN5's
board loads them: the frames grey) copied in. Each panel's kind lies over
it on BG2 as BN5 lays it: one of BN5's twelve looks (`BN5_LIB_LOOKS`,
picked by the kind as `BN5_LIB_LOOK_OF` picks it: clean cyan, the dark
swirl, the item's stud, the DarkHole's pit, the BonusPanel's "?", the
Darkloid's ring a grey grid until the DarkHoles fall, the start clean's:
BN5's start look is blank over its map's pad, and ours stands among its
clean panels on a panel's frame, which bare read as a hole, the owner
found; the start's 21 panels of mission 1 are left out of the floor
learned, as their pad's pieces drew lilac round the start), its 8 x 4 entries at the panel's centre less 32 and 16 pixels and 8
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
phase (resting BN5's 15 frames on each before it moves), back to MegaMan
before PLAYER PHASE; under L the D-pad moves it at BN5's 4 and 8 pixels a
frame, kept over the board, BN5's arrows and cross over it; let go, it
follows MegaMan again. Stage 3's R, BN5's switch between Navis, pans with
the same `lib_camera_at` to the Navi taken. MegaMan walks as BN6 walks him throughout: it moves only
while he stands. The board's music is BN5's: its song 0x22 the dark layer's map's own
(`lib_board_song`), 0x1C at the briefing, 0x22 again at PLAYER PHASE
01, copied into BN6 after the areas' songs and themes (`xsong.c`, each
keeping its place) and started by BN6's PlayMusic
(`liberation_sound.c`); BN5's one-turn win 0x24 at a battle won in its
first turn, and 0x18 as the board is won, the board's own again once it
ends (`liberation_cues.c`; why, docs/MULTIROM.md, BN5's music in a run).
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

**Left for later:** a guardian's HP under it (the Darkloid's came with
The board's HP as BN5's); the economy report and pacing for
dark layers; the second screen's board; stages 2 to 5. (Barriers, keys,
the sealed region, traps, chips and zenny in item panels and the
planner's par came with the quality bar, below.)

Moving BN5's own board code (0x12000 bytes of Thumb, its tables, text and
graphics) into BN6's core instead was weighed: its exact rules for free,
but most of it calls BN5's overworld engine at other addresses, and the
generated boards need none of BN5's missions. Its code is read for the
rules where the guides are unsure (the commands' shapes, the Darkloid's
heal).

### Stage 2, as built: the liberation battle (23 October 2026)

**BN5's rules, read in its code** (`src/emu/bn5_liberation.h`; docs/ROM_DATA.md,
BN6's battle loop and banners). BattleState +7 counts the Custom screens
opened (played in romlab: 0 until the first opens, then 1). As each turn's
fight starts (`BN5_LIB_TURN_BANNER`) the battle shows TURN n START!, n the
turn, and from the third FINAL TURN!. Where the Custom screen is asked for
in the third (`BN5_LIB_TIME_ASK`), the battle's phase 0x18 instead
(`BN5_LIB_TIME_UP`): TIME UP!, 30 frames, LIBERATE FAILED!, the battle
over. At a win (`BN5_LIB_WIN_BANNER`), 1 TURN LIBERATE! where it is the
first turn, else LIBERATE SUCCESS!. The banners are BN5's battle messages
(`BN5_BATTLE_MESSAGES`), and BN6 keeps that table whole (`0x0801EF84`,
TURN n START! to n TURN LIBERATE! at the same numbers), but none of its
liberation rules.

**On BN6's loop** (`src/director/liberation_battle.c`, `src/emu/bn6_battle_loop.h`):
the loop random battles and bosses run, answered by hooks while a
liberation's battle is armed (the director arms it as the command is
taken), no code moved:

- each turn's fight starts with BN5's banner in BN6's own message engine
  (the fight's BATTLE START! answered: TURN 1 START!, TURN 2 START!,
  FINAL TURN!);
- the final turn's gauge full, the loop goes to BN6's own draw (a
  NetBattle's judge's end): the battle stops, its banner TIME UP!, the
  draw run again for LIBERATE FAILED!, then result 3, which ends the
  battle with no results screen and no reward, MegaMan's HP as it stands.
  BN5 asks it as the Custom screen is asked for; BN6 opens that on L or R
  alone, so here the gauge's filling ends the turn, or the final turn
  could be held forever;
- a win shows 1 TURN LIBERATE! or LIBERATE SUCCESS! in the win's place
  (ENEMY DELETED!), so the board no longer shows its own banner after the
  battle (stage 1's);
- no running from it (the record's run bit off, as from BN5's).

**What a battle does on the board** (`liberation.c`):

| Battle | Won | Won in its first turn | Time up |
| --- | --- | --- | --- |
| A dark panel (Liberation) | the panel freed, its item given | and the dark around it (its eight: dark, items, the Darkloid's ring once open; no unit's) | the panel stays dark, the phase spent |
| LongSwrd (an Order Point) | the panel and the one behind it | and the dark around the first | as above, the Order Point spent |
| A guardian | deleted (back from its DarkHole while the hole stands), its panel freed | and the dark around it | the guardian stays |
| A DarkHole | closed: its guardians gone for good, the eight around freed | the same | the hole stays |
| The Darkloid | deleted, the board won | | (no turns to count) |

The battle's viruses are the act's own, BN5's on BN5 territory, from the
run's encounter tables, harder with the dark around the panel (stage 1's
bands). **The Darkloid's battle** is the run's guardian Navi of the act's
band in his own battle (stage 1), with no turns to count, and from its
eleventh turn every hit does double (`BN5_LIB_LATE_DOUBLE`: BN5 doubles an
object's hits of the frame, `BN5_LIB_DOUBLE_HIT`; here BN6's
object_subtractHP, every HP taken, while his battle runs).

**The stakes, before the battle.** With the command menu open, a box above
BN5's window (under the PET look's menu without BN5's ROM) says what the
command under the cursor stakes: "3 TURNS: FREES 1" and "1 TURN: 5 MORE"
(the dark around that a first-turn win frees too), "3 TURNS: DELETES IT"
for a guardian, "3 TURNS: CLOSES IT" and "FREES 9" for a DarkHole, "NO
TURN LIMIT" and "TURN 11 ON: HITS X2" for the Darkloid
(`liberation_battle_words.c`).

**The design** (the game-design skill): the battle restates the board's
*press on or prepare*. The dark around a panel makes its battle harder
and its first-turn prize larger, so pressing into the thick dark is a
gamble the folder must back, and a safe panel at the edge frees little:
a costed choice, shown before it is taken (an honest telegraph: the
turns, the prize, the banners in the battle counting them down). Failure
is graduated: a time up costs the phase (and LongSwrd's Order Point) and
the HP the battle took, never the run on its own; the board's harm still
never takes the last HP. A deletion in the battle is the run's, as in
every battle.

**Checked:** `--dev dark,god` with BN5's ROM, a dark panel taken and its
battle left to run: TURN 1 START!, TURN 2 START!, FINAL TURN!, the gauge
full, TIME UP!, LIBERATE FAILED!, the board again with the panel dark
(`liberation: battle 0 at 34,33 time up (turn 3)` in the debug log); with
`onehit`, 1 TURN LIBERATE! and the dark around freed. The autopilot with
`god,onehit` (seed 1) won the boards of acts 2 and 3, guardians, DarkHoles
and Darkloids (act 2's Darkloid in his battle's seventh turn); with `god` alone
its buster battles timed out in 13 of 25 and the board ran past 60000
frames, and without `god` (seed 2) MegaMan was deleted in its third
battle: the autopilot's battles are weak, and a dark layer asks a folder
that deletes in three turns.

**Left:** BN5's Surrounded battle (dark on both sides of the Navi, L and R
to turn round: its text is BN5's battle archive's "Surrounded! Press L or
R Button to turn around!"): BN6's own fields hold no field of enemies on
both sides that we know of; whether BN5's own Custom screen opens by
itself as a liberation turn's gauge fills; the Darkloid's intro and BN5's Darkloids (stage 4);
the team (stage 3).

### The quality bar, as built (23 October 2026)

The staircase, the barriers and their keys, the sealed region and its
route's hook, the traps, the chips' and zenny's finds and the planner's par
in the engine (The quality bar, The generator), MegaMan alone. In play: a
key opens its barrier's panels (their fence's walls switched off as a
liberated panel's are), a trap's harm is taken as it is liberated (never his
last HP, BN6's board harm), a chip or zenny is given by a script of BN6's
own (`ta_lib_find_chip`, `ta_lib_find_zenny`, installed with the layer as
the par's prizes are), and the route found is told and kept for the layer
(`lib_route_found`, the state in words' `route found`). The units' rules
(steps, reach, harm) are the board's (`lib_guard_rule`, `lib_harm`), so the
engine, the planner and the telegraph read the same ones. `LAYER_MAKE` 98.

**The measures** (`tests/test_core.c`'s `test_liberation_board`, 40
layers an act, each board made twice; the planner's 12 plays, 6 stripped
and 4 for the route):

| Act | Kept | Panels | Dark | Par (range) | Middle half | Dead on the way | Sensible moves a phase | Best: Liberation / LongSwrd / Pass / fight | LongSwrd a play | DarkHole orders | The route | Dead, stripped | Rejected most for |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 2 | 40 of 40 | 197 | 20% | 12 (9-19) | 9-14 | 43% | 1.5 | 49 / 27 / 7 / 15% | 3.4 | 1.4 | none | 43% | dead 65, LongSwrd 20 |
| 3 | 38 of 40 | 198 | 25% | 14 (9-21) | 12-18 | 45% | 1.5 | 48 / 27 / 9 / 14% | 3.8 | 1.6 | +4 | 42% | dead 66, barrier 31 |
| 4 | 39 of 40 | 233 | 25% | 17 (12-31) | 14-20 | 46% | 1.6 | 44 / 30 / 14 / 9% | 5.0 | 2.7 | +4 | 51% | dead 116, stripped 49 |
| 5 | 25 of 40 | 195 | 32% | 20 (12-34) | 17-23 | 49% | 1.5 | 44 / 29 / 13 / 12% | 5.8 | 2.9 | +5 | 53% | dead 231, barrier 53 |
| 6 | 21 of 40 | 188 | 36% | 21 (15-33) | 19-25 | 48% | 1.5 | 41 / 29 / 17 / 11% | 6.1 | 1.9 | +5 | 50% | dead 217, unwon 91 |

(Dead on the way: phases whose sensible moves were one, strikes at a
DarkHole or the Darkloid apart. Middle half: the plays' phases from the
quarter to the three quarters. The route: the route's player's phases
over the par's. A layer whose sixteen tries fail is made plain: acts 5 and
6, which a run does not meet yet, keep half their boards.)

What they say:

- **A lone MegaMan decides about one phase in two.** Half his phases offer
  two sensible moves or more (another DarkHole, an item, a rest, LongSwrd
  or not, a bold panel or a safe one); the other half are a corridor's
  next panel or a rest he cannot put off. The rule keeps the boards at 55%
  or under; the dead half is the lever stage 3 pulls, a team's Navis each
  a decision a phase (the prototype's teams: 46% to 54%).
- **The staircase reads in the numbers.** Each act's boards are a little
  larger in what they ask: the par grows 12, 14, 17, 20, 21; the DarkHoles'
  orders open from 1.4 to 2.9 ways as the holes grow from two to three;
  resting matters more (the best move 7% of the phases at act 2, 14-17%
  from act 4) as the reach and the tolls come in.
- **The route is a price, not a gift.** It costs its player four or five
  phases over the par, about a tier of the par's prize; a board where it
  costs fewer than two or more than ten is rejected.
- **LongSwrd has its niche.** It is the best move in 27-30% of the
  phases, taken three to six times a board: neither the only answer nor
  idle.
- **The autopilot** (`--dev dark,god,onehit`, its one-hit battles all won
  in one turn, so better than the planner's player) cleared act 2's board
  in 11 phases against its par 13 and act 3's in 9 against 18 from depth 5,
  and from depth 8 act 3's again and act 4's in 16 against 19, both of its
  keys found and their barriers opened, on to act 5's dark layer.
  Started at depth 11 directly, MegaMan was deleted in a liberation's
  battle on dev's build as on this one: not this change's.

**Left for later:** the route kept for the run and offered at the next fork
(Special routes); the board's
make on the 3DS timed (about 55 ms a board on the desktop, the planner's
plays the most of it: a dark layer's make on the New 3DS may take a second
or two); acts 5 and 6 tuned for a lone MegaMan (their BN5 harm wants a
team); the teammates' commands in the engine (stage 3), the command rule
with them.

### The board on the map's floor, as built (23 October 2026)

The board was made from the layer's cells before its map was built, and
the map's build changes them: netmap's legal.c makes the floor drawable
in the area's tiles, writing the layer's cells in place, and drops the
panels it cannot draw (most of them one panel wide, a walkway's beside
another's). So a way of the board's could be no way of the map's: seed
1's act 5 board at depth 14 lost 22 of its panels after it was made
(`off the map's floor` in the debug log), its start's corridor cut from
the rest, and the autopilot stood on its first panel for the whole run.

Now the map is built first (`build_layer`, `src/director/director_layer.c`)
and the board made on its floor: `lib_board_make` takes the floor as a
predicate per panel (`LibFloor`), and the director's (`map_floor`,
`src/director/liberation.c`) reads the map as built, its floor at the
ground's height (`netmap_floor_cell` at the panel's middle, the cell the
fence reads) and no stair. The map's floor goes by panels (a wall cell is
floor where the panel its middle lies in is, and on a panel's edge where
both are), so two board panels side by side are a way MegaMan walks, and
the planner's graph is the map's. What the map's build took from the
board before needs nothing now: the exit moved to the Darkloid's panel and
the units' objects locked their panels' floor against legal.c, and the
board stands on floor already there. A dark layer whose map gives no
board is made plain and its map built again (the second build is the one
cost, on those layers alone). The tests' boards (`test_liberation_board`)
pass no predicate, the layer's flat cells as generated, as before, and
`test_liberation_floor` makes boards on a made-up map's floor (a panel in
29 gone) and finds none of their panels off it. `LAYER_MAKE` 99.

**The measures** (the debug log of 20 seeds an act, `--dev dark`):

| Act | Kept, before | Kept, now | Panels, before / now | Panels lost to the map after the board was made, before | Now |
| --- | --- | --- | --- | --- | --- |
| 2 | 20 of 20 | 20 of 20 | 181 / 161 | 446 (all 20 boards) | 0 |
| 3 | 15 | 14 | 203 / 179 | 429 (all 15) | 0 |
| 4 | 15 | 16 | 192 / 174 | 310 (all 15) | 0 |
| 5 | 9 | 7 | 219 / 196 | 224 (all 9) | 0 |
| 6 | 10 | 6 | 225 / 195 | 191 (all 10) | 0 |

Every board lost panels to the map before, about twenty a board, and the
planner had counted on them; now none does. The boards are smaller by
those panels, and a run's acts (2 to 4) keep about as many; acts 5 and
6, which a run does not meet yet, keep fewer, the cut floor's corridors
failing the planner's dead phases.

**The autopilot** (`--dev dark,god,onehit`, seeds 1 to 5; acts 4 to 6
again with `hp=2000`, as `god` does not hold through a battle that
deletes MegaMan from full: stage 2's battles deleted him on both builds
in every act 4 to 6 board without it): all 18 boards it met played
through, the act's Darkloid deleted and the exit taken (9 in acts 2 and
3; 9 in acts 4 to 6, seeds 1 to 6, with `hp=2000`), and none left
MegaMan standing at a way the map does not have. Before, over the same
seeds, 1 of 20 did: seed 1's act 5 board left the autopilot on its first
panel for the whole run (the rest found ways round the panels they had
lost). On its map's floor no board of that layer's sixteen tries passes
the planner, and it is a plain one.

### The Darkloids' phase as BN5's, as built (23 October 2026)

The owner saw the 0.15.0 trailer's board: the units stood still and
glided, the Darkloids' phase did not play as BN5's, and the exit's ring
was clipped. The phase is now BN5's as traced above
(`src/director/liberation_turn.c`, a unit's turn a frame at a time):

- **The Darkloid first**, then each guardian, as BN5 orders them; the
  camera's pan and rest to each as before.
- **A guardian's move** is BN5's warp: its steps taken at once by the
  board's rules (`lib_guardian_step`), its object parked off the floor
  for the warp (`LB.hidden`), BN5's puff and lines drawn over the board
  from its ROM (sprite list 7's 59, its medium pair), sounds 0xB4 out
  and in and 0x94 as it shows, the camera's pan between. The object is
  hidden and put back a frame before the effect, as BN6 draws an
  object's new place in its next frame.
- **A strike** where MegaMan stands in a unit's reach: the reach in
  BN5's red look on BG2 (`LB.reach`, look 0x0D; tinted red where the
  board is not in BN5's look), its blink and sound; the unit's line
  (`lib_strike_words`: the guardians' cries and attacks' names,
  BeastBreath, HawkAttack and DarkSlash, BN5's; the Darkloid's in his
  own habit of speech with his mugshot); his strike animation; the blow
  with BN5's sound, BigBrute's flame over MegaMan; the harm in BN5's
  yellow digits over him (`lib_piece_harm`) as his HP drops; BN5's
  frames throughout. MegaMan's end-of-phase "Ow!" is gone: the harm
  shows as it lands.
- **The units' animations** are the board's: each unit's object is a
  standing NPC whose animation follows a byte of its own script
  (`npc_live`), written each frame from `LB.anim`; the guardians stand
  facing down-left in BN5's standing animation (two or four frames) and
  strike toward MegaMan in their own. The Darkloid is a BN6 Navi in his
  overworld sprite: his strike is his title card's pose where he has one
  in list 6 (HeatMan, ElecMan, SlashMan, EraseMan, ChargeMan and BN5's
  six); the others stand, BN6's overworld sprites having no strike.
- **The exit's pad** shows on the Darkloid's panel liberated as he is
  deleted, drawn over BG2 (layer priority 2, `npc_sealed_pad`'s
  `over_bg2`) where BN5's looks lie, still under MegaMan by its depth.
- **The overlays' camera:** while the board holds BN6's camera (the
  Darkloids' phase), the drawing takes the camera's own place for the
  picture's middle; the BG scroll's wrap read it a half turn off away
  from MegaMan.

Not done: TinHawk's and Bladia's strike effects and the BN6 Darkloids'
(not traced: their strikes are their animations and the harm); a deleted
guardian's return through BN5's warp. Teammates stay stage 3's. (The
Darkloid's heal and the units' HP digits: The board's HP as BN5's,
below.)

**Checked:** an autopilot run through seed 1's act 2 board from its
opening to its exit (`--run-depth 5 --seed 1 --dev dark,god,onehit`,
11-12 phases on its par of 12, the exit taken to depth 6); seed 2's
board for a BigBrute's warp and strike (frame 5409 on), seed 1's for
the Darkloid's (frame 13667 on).

### The par's prize, as built (25 October 2026)

BN5's tiers exactly, the owner's decision 5: the board won two phases
under par or better pays a chip, within par a chip, over par zenny, the
same three rows and bounds as every BN5 mission's (The par's prize, above;
`LB.prize` from the phase and par as the Darkloid falls,
`src/director/liberation.c`). What each pays, reasoned with the
game-design skill (`src/layer/lib_prizes.c`, `lib_par_zenny` in
`src/director/liberation_board.c`):

- **The chips:** BN5's two are a better chip and a lesser one of the same
  kind (the Darkloid's SP and plain chips, later a Mega chip and his DS
  one). The run's are a chip a Server's win pays two under par (two
  layers deeper, the rarer tiers weighted) and one a tier lower at par,
  as stage 1 had them: our Darkloid is a BN6 Navi whose own battle pays
  BN6's rows for him beside the prize, so his chip again would be a
  second copy, not a better one.
- **The zenny:** BN5 pays 1000 a mission, 5000 from the fifth. The run
  pays BN5's first mission's 1000 at the second act, where a board first
  stands, and a quarter of it more each act, to 2000 at the sixth; dearer
  on later cycles as a chip's price is, and the zenny curve's share as
  every zenny the engine sets (653, 959, 1,288, 1,636, 2,000 under the
  opening plan). BN5's own climb would pay 3,435 to 5,000 from the fourth
  act, more than the chip at par is worth at a dealer's (2,345 to 2,430,
  `build.py economy`'s dark table), and racing the par would lose to
  dawdling: over par stays the lesser prize, never a lost run, as BN5's.
- **How it is told:** the mission won's tally, in BN5's own words (The
  board's words and banners as BN5's, below: SUCCESS! and MegaMan's
  "Target" and "Actual"), then the chip or the zenny given in BN6's own
  box at once, as BN5's "You got a battle chip for:" and "You got:"
  follow its tally (`ta_lib_prize`, `ta_lib_prize_zenny`,
  `src/layer/service_words.c`). Without BN5's ROM, Lan's words say the
  phases and the tier (`lib_won_words`). (This pull request first had
  MegaMan say what the Darkloid left before the give; merged with the
  board's words, BN5's tally says it, and the give follows bare.) BN5's
  Officials stay out: no one on our board stands for them.
- **Checked:** `build.py test` (the zenny by act and cycle, under either
  plan), `build.py economy` (0 outside the bands; the dark table flags an
  act whose zenny over par reaches the chip at par's price), and seed 3's
  act 2 board played by the autopilot at each tier with `--dev
  libpar=N` (its 8 phases two under a par of 10, at a par of 8, over one
  of 6), beside BN5's mission 1 at 5, 7 and 8 phases in romlab (the
  pictures in the pull request).

### The board's HP as BN5's, as built (25 October 2026)

**BN5's rules, read in its ROM and played in romlab** (mission 1;
`src/emu/bn5_liberation.h`, docs/ROM_DATA.md's row): each board unit
keeps its HP, its max and the HP its digits show. The digits stand under
every unit in play but the Navi in play (his HP is the HP box's): white,
8x16 (BN5's HP digits), centred on the unit's place on the floor, their
top there; the HP shown steps 4 a frame toward the HP. A unit fought
lends its battle's marked enemy its HP and max as the enemy spawns, and
takes back the HP that enemy has as the battle ends, so one not deleted
keeps on the board what its battle left (one left at none by a battle
not won gets a quarter of its max). The Darkloid heals as his turn
begins, as DARKLOID PHASE's band closes, where his HP is below his max
and dark panels stand round him: a tenth of his max for each (dark, an
item's, a DarkHole, his ring: BN5's weights), his max at most, the
digits with it at once; rising lines over him (list 7's 120, in the
palette its variant names) with sound 0x8A, then 20 frames before his
turn goes on. (The guides' "100 a panel" is a tenth of a 1000 HP
Darkloid's.) A Navi's rest heals by BN5's story's progress, a fifth of
his max on mission 1 to a half (its ROM); ours is the guides' half.

**On our board** (`liberation.c`, `liberation_battle.c`,
`liberation_turn.c`, `liberation_pieces.c`, `liberation_draw.c`):

- **The Darkloid's HP is the board's**: his own battle's (the version
  make_boss gives him) as the board begins, kept across a CONTINUE
  (`run.lib`, its note `CLB3`), shown under him in BN5's digits, rolled
  as BN5 rolls them, hidden under a chat box. His battle begins from it:
  the director gives his object the board's HP as it is first in play,
  his max his own (BN6's MaxHP, as BN5 gives the board's max), and
  reads his HP each frame of it; a battle that leaves him standing
  leaves the board that HP.
- **His heal** is BN5's, at the start of his turn in the Darkloids'
  phase, its effect and sound BN5's, read from its ROM, then BN5's wait.
- **The guardians show no HP**: our guardians' battles are the act's
  viruses, not one foe of theirs, so they have none to keep.

With MegaMan alone nothing on the board lowers the Darkloid's HP and
his one battle ends with his deletion or the run's, so his heal and his
HP carried come into play with the team (stage 3: ShadowMan's
SneakAttack, a teammate deleted in his battle) and BN5's BonusPanel's
harm to the nearest enemy. `--dev libhp=N` begins him at N HP to see
them.

**Checked:** BN5 and ours side by side (the pull request's pictures):
the digits under BlizzardMan and a guardian in BN5, under the Darkloid
on our board; the heal with `--dev dark,god,onehit,libhp=100` (seed 1,
depth 5): 100 to 660 at once (his ring's eight dark, 70 each, frame
2266), BN5's lines over him in its purple, 700 the next phase;
`test_liberation_heal` (the weights, the cap, none at full HP or with
none dark round him); the autopilot through seed 1's board to its exit.

**Left:** a guardian's own HP (BN5's guardian battles' marked enemy not
traced: which of its battle's enemies the record marks); the rest's heal
by BN5's table (a game-design call: ours is the guides' half); the HP
box and the team's digits with stage 3.

### BonusPanels as BN5's, as built (25 October 2026)

A BonusPanel is BN5's (traced: BonusPanels as BN5's, above), drawn from
BN5's ROM as the game runs, nothing of it kept:

- **The rule** (`src/director/liberation_bonus.c`): fenced like a dark
  panel, so MegaMan no longer steps on it; facing it, A opens the menu
  with Liberation (LongSwrd not offered), which frees it without a battle,
  and his action is not spent: the board goes back to his move. A CONTINUE
  saved during it resumes at his move with the panel freed.
- **The picture** (`src/director/liberation_roulette.c`): "A BonusPanel!",
  the panel freed as BN5's dissolve, the three flashes and their sound,
  the band, the slot's icon sliding in, "PRESS A BUTTON", the slot every
  4 frames from a random first one with BN5's tick, A to stop it with
  BN5's sound, its name held 88 frames, out and shut: BN5's sprites
  (list 9's icons and words) and frames.
- **The slots, BN5's order:** no harm on the board this phase
  (`LB.shield`, BN5's sound); HP full; five Order Points; a chip; Major
  Hit (the camera on the nearest guardian, deleted, its DarkHole sending
  it back as ever; with none left, the Darkloid: `LB.darkloid_cut` counts
  the fifths of his max HP BN5 takes, for his battle's HP once the board
  keeps it). HP, Order Points and the chip pop over the freed panel in
  BN5's icons with their words, as an item panel's do.
- **The chip:** BN5's are a mission's standard chips, picked among four
  alike, as its item panels' are; a board's BonusPanels give one of four
  chips rolled in its item panels' band (`lib_prizes.c`, the layer's
  seed's rolls of their own), so a chip from a BonusPanel is worth what an
  item panel's is (docs/ECONOMY.md, Where it is decided). The economy
  report does not play dark layers yet (Stage 1's left for later).
- **The placement** (`place_bonus`): a board without a sealed region got
  none (its test read every panel as the sealed region's), and the floor
  of a layer's map has few stubs; BonusPanels now go to stubs' ends, else
  to a clean corner (two neighbours), never by a DarkHole: every board has
  its act's two or three.
- **The planner** frees a BonusPanel for nothing as before and stops the
  roulette as a player does, where it helps most (HP when below half, a
  guardian within three, else Order Points, else no harm), one in five
  missed; boards made in acts 5 and 6 stay above half (43 of 80).
  `LAYER_MAKE` 101. Without BN5's ROM, the roulette is not shown: its
  first slot is given at once.

Reasoned with the game-design skill: the BonusPanel stays a lure at a
stub's end, the detour its only price (*prepare or press on*); BN5's
stop by A makes it a small act of timing in a turn-based board, and its
five a choice by need (no harm before a risky phase, HP when low, Order
Points for a LongSwrd, a chip for the run, a guardian off the way).

### Each mission's own look, as built (25 October 2026)

Every dark layer stood in BN5 mission 1's map. Each now takes the BN5
mission that matches its area, its map, backdrop and colours read from
BN5's ROM as the layer is made (`src/director/liberation_look.c`,
`lib_look_mission`; docs/ROM_DATA.md, Each Liberation Mission's own look).

**What differs between BN5's nine boards** (each opened in romlab on the
plain ROM, as mission 1 was: the mission set, BN5's own warp into its map,
A tapped through its opening to the board): less than their maps suggest.
The nine maps (0x86:00-08) share one tile set and BN5's board one overlay
and its twelve looks, so the framed panels, the swirl and the looks are the
same in all nine. A mission's own is:

- **its map**: the board's shape, and the pieces round its panels where
  the map draws them (End 2's walkway joins, Nebula's stone slabs);
- **its backdrop**, one of five: ACDC 3's purple diamonds (mission 1),
  Oran 3's with green specks (2), SciLab's purple binary digits (3),
  End's magenta diamonds (4 and 5), the Undernet's and Nebula Area's purple
  static (6-9), each the map's own backdrop in a mission's colours
  (`BN5_LIB_MISSION_BACKDROPS`), and the map's own tile animation;
- **its board's colours**, two sets: the mission's GFXAnim list
  (`BN5_LIB_MISSION_ANIMS`, the lists group 0x86 takes while a mission is
  on) loads banks 1-12 over the map's as it is entered, missions 1-5 from
  `BN5_LIB_BOARD_PAL_ACDC`, 6-9 from `BN5_LIB_BOARD_PAL_NEBULA` (the two
  differ in banks 1 and 3, the frames' and pieces' shades; the panels'
  banks 8-12 are the same).

The music is the same in all nine: the board's song 0x22, started by an
immediate in the board's code and read on the music player in missions 2,
3, 5, 7 and 9 (1 before).

**Which mission an area takes.** A BN6 area takes the missions of the BN5
net area that dresses it in a run (rom.c's `like`; docs/MULTIROM.md): each
mission's map is one of its areas' maps by BN5's own labels and by the
maps BN5's random battles name for that area (`NetAreaDef.xbattles`:
0x86:00 ACDC Area's, 0x86:01 Oran's, 0x86:02 SciLab's, 0x86:03 End's,
0x86:05 the Undernet's, 0x86:06 and 07 Nebula's).

| BN6 area | Dressed by | Missions |
| --- | --- | --- |
| Central Area, ACDC HP | ACDC Area | 1 (ACDC 3) |
| Green Area, Green HP | Oran Area | 2 (Oran 3) |
| Sky Area, Sky HP | SciLab Area | 3 (SciLab) |
| Seaside Area | End Area | 4 (End 2) in act 2, 5 (End 5) from act 3 |
| Undernet | Undernet | 6 |
| Graveyard | Nebula Area | 7 (Nebula 1) in act 2, 8 (Nebula 3) in act 3, 9 (Nebula 5) from act 4 |
| Undernet Zero, the Cybeast Nest | none: deeper than BN5's net | 9, BN5's last |
| the comps and homepages | none | 1, BN5's first |

An area with several takes a later one in a later act, as BN5 meets them
(the homepages named for an area take that area's by their name alone).

**How.** Each of the nine maps is a held net area of BN5's (`rom.c`,
`XAREAS_MAX` 16 for them), so what an area brings comes with it: its floor
learned from its own board's panels (`area_src_board`, already per
record), its backdrop and animations copied by `xbackdrop_map` (the
mission's lists and records, as BN5's group 0x86 takes them on a board),
its colours over the map's (`area_src.c`'s `board_colours`: the list's
script that writes bank 1's copy, 0x180 bytes, its frame's source).
`xbackdrop` copies each script and record once however many maps share it
(the nine share five backdrops and their scripts; each its own filled the
64 KB range by the fifth): the nine take 14 KB beside the six areas' 37.
Nothing is kept: every byte is read from the player's ROM.

**The boards.** A mission's map draws more of a layer's generated floor
than mission 1's did (End 2's and Nebula's maps learn joins 0x86:00 never
shows), so the boards on them are bigger (a tenth to a fifth more panels; the planner's par within three phases of before, either way), and in acts 3
and 4 the generator's barriers fail a few more of them in all 16 tries.
Where none fits on its mission's map, the layer is made again on mission
1's map, where every dark layer stood before, and only then plain
(`dark_again`, `src/director/director_layer.c`). The debug log of 8 seeds
each, `--dev dark` (kept boards; before, every one on mission 1's map):

| Mission | Area, act | Kept, before | Now (on mission 1's map) | Panels, before / now | Par, before / now |
| --- | --- | --- | --- | --- | --- |
| 1 | Central, 2 | 8 of 8 | 8 (0) | 160 / 160 | 12.5 / 12.5 |
| 2 | Green, 2 | 8 of 8 | 8 (1) | 166 / 178 | 12.2 / 12.5 |
| 3 | Sky, 2 | 8 of 8 | 8 (0) | 159 / 182 | 13.1 / 11.5 |
| 4 | Seaside, 2 | 8 of 8 | 8 (0) | 185 / 223 | 13.1 / 11.8 |
| 5 | Seaside, 3 | 8 of 8 | 8 (1) | 194 / 221 | 13.5 / 11.1 |
| 6 | Undernet, 2 | 8 of 8 | 8 (0) | 177 / 195 | 14.2 / 12.2 |
| 7 | Graveyard, 2 | 8 of 8 | 8 (0) | 168 / 176 | 11.2 / 13.8 |
| 8 | Graveyard, 3 | 5 of 8 | 5 (2) | 189 / 193 | 12.6 / 14.0 |
| 9 | Graveyard, 4 | 8 of 8 | 8 (3) | 195 / 206 | 21.1 / 19.1 |
| 9 | Undernet Zero, 2 | 8 of 8 | 8 (0) | 171 / 197 | 11.4 / 11.4 |
| all | | 77 of 80 | 77 (7) | | |

`LAYER_MAKE` 101.

**Checked:** BN5's nine boards beside ours, one mission an area (the
pull request's contact sheet); mission 1's boards the same as before on
every seed measured (the control); autopilot runs from depth 5, seed 1
(`--dev dark,god,onehit`, 36000 frames), each through its area's first
dark board to the exit and on: Central (mission 1) to depth 8, Seaside
(4, then 5 in act 3) to 10, the Graveyard (7; its act 3 board plain on
this seed, as before) to 11, SciLab dressing Sky (`x1`, 3) to 9, Oran
dressing Green (`x4`, 2) to 8, the Undernet (6) with `hp=2000` to 8
(without it MegaMan was deleted in a board's battle at 100 HP, as stage
2's runs found).

### The board's words and banners as BN5's, as built (25 October 2026)

Traced in romlab on the plain ROM (docs/ROM_DATA.md, BN5's Liberation
board: the L view's keys, the Darkloid's first word, the mission won and
its tally) and drawn or read from BN5's ROM as the game runs, its text
and tiles never kept:

- **The Darkloid's entrance.** BN5's opening tour passes the Darkloid in
  silence (mission 1 recorded: the tour, then Colonel's tutorial; missions
  2-9 open on the team's words). He speaks at his first turn of the first
  DARKLOID PHASE, after the banner and before anything of his phase, with
  his mugshot, once (his archive's entry 0, through a flag: BlizzardMan
  claims the area for a ski resort). Ours the same: at the first
  Darkloids' phase, before the DarkHoles send anyone back and before his
  turn, the run's Darkloid says his line with his face, in his own habit
  of speech (`lib_entrance_words`, `src/director/liberation_words.c`; the
  run's Darkloids are BN6's Navis and BN5's six, none a Darkloid of
  BN5's, so the lines are ours, docs/VOICE.md's guardians). Not kept
  across a CONTINUE: a board continued in its first phase hears it again.
- **The L view's keys.** BN5's help at the top right, its eight OBJs
  ("+ MoveCam", "B+ + MoveCam(F)", "L Back") from its tiles on the band's
  fill in WIN0, drawn with the view's phases and arrows
  (`lib_piece_phases`).
- **The guardians' reach in the L view.** BN5's L view shows no reach: the
  camera taken onto a BigBrute in it, A, R and SELECT pressed, nothing
  changes (its help names the D-pad, B and L alone). The quality bar's
  honest telegraph (rule 4) puts it there, in the one look BN5 gives a
  reach, the red it lights a strike's in (look 0x0D): while L is held, every
  panel where the units would strike a Navi in their next phase
  (`lib_threat`, the planner's own reading) lies in BN5's red, steady, and
  goes as L is let go (`src/director/liberation_look.c`). Reasoned with the
  game-design skill: the dialectic is the board's *press on or prepare*,
  and the reach is what a phase's choice costs; an honest telegraph shown
  on demand (BN5's own map view, which a player opens to plan) keeps the
  board as BN5 draws it the rest of the time; steady, not BN5's blink and
  sound, which say a strike is coming now.
- **The mission won.** BN5's routine 6 after the Darkloid's battle:
  BN5's song for it (0x18) at once, SUCCESS! in the phases' band 22
  frames on (its words BN5's, PLAYER PHASE's palette, their sound), the
  band's 104 frames, then 20 more, and the team's leader's tally:
  "Target" and "Actual" phases, then the Officials' reward (a chip two
  under par or better, another at par, zenny over it), BN6's give after.
  Ours the same from the Darkloid's deletion (`LIB_STEP_CLEAR`,
  `src/director/liberation.c`): the song, SUCCESS!, then MegaMan's tally
  in BN5's own words, MegaMan's three (the leader of a lone MegaMan's
  team), read from BN5's ROM as it is said with the par and the phases
  ours (`lib_piece_tally`), then the prize given by BN6's own give alone
  (`ta_lib_prize`; the Server's "The signal left a chip!" no longer
  before it). Over par the prize is zenny, as BN5's (the owner's decision
  5, The par's prize, as built), the tally's first page BN5's and the
  zenny given after it. Then "The exit's open,Lan!", the fence open
  through it.

**Checked:** side by side with BN5 in the pull request (the entrance,
the L view, the clear); an autopilot run through seed 1's act 2 board
(`--run-depth 5 --seed 1 --dev dark,god,onehit`).

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
5. **The rewards** (decided 25 October 2026): BN5's tiers exactly, a chip
   two under par, a chip at par, zenny over par (The par's prize, as
   built). (Not chosen: the route a sealed region opens as the only
   special reward; the route stays a reward of its own.)

Still open:

4. **BN6's Crosses as teammates** (commands of BN5's kinds for BN6's
   Navis), for runs without BN5's ROM or beside the Souls: not decided;
   until then a run without BN5's ROM meets no dark layer, or meets it with
   MegaMan alone.
