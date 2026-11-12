---
type: Engine Mechanism
title: "Transplanted Navis"
description: "Issue #82's Navi track: BN5's Team Colonel Navis, the Dark Soul, all four Darkloids, Team ProtoMan's NapalmMan, GyroMan, SearchMan, Meddy and MagnetMan and Nebula's LarkMan and GridMan moved into BN6's battle engine (`src/emu/xnavi_install.c`, `tools/xnavirecipe.py`), BN6's Navi tables grown, each proved frame by frame by `build.py xdiff`."
tags: [multirom, bn5]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-11T16:49:36Z }
---

# Transplanted Navis

Issue #82's Navi track: BN5's Team Colonel Navis fight in BN6's own battle
engine, each proved against BN5's own battle frame by frame before it is
installed. A Navi is a family with more (`src/emu/xnavi_install.c`): its
code and records are moved by the families' code mover (`xvirus_move.h`,
the same blocks, pools, veneers and checks), and it is found through
BN6's Navi tables instead of the families' ([Navis moved into BN6's battle engine](../rom/navis.md), Navis moved
into the battle engine).

- **The room** (`src/emu/xnavi_room.c`), made once a core after the
  families' and before any Navi: BN6's thirteen per-AI Navi tables (the
  state tables, the pre-step, hook A, struct1, struct2, the post hook,
  spawn start and end, paralysed, frozen, bubbled, flinching, dragged)
  and the four triples' Navi words (the ice block's size, the held chips,
  the companion's start and end) copied to 256 AI indexes, the companion
  overlay's kinds to 64, each word that names one pointed at the copy;
  the arm points' records of list 2 grown (`xnavi_sprites.c`); and the
  moved Navis' names, an archive of their own handed out by a hook on
  BN6's name lookup for their ids (BN6's own Navis read BN6's archive as
  before). BN5's Navi n of Team Colonel (7-12) is Navi AI 32 + n - 7 with
  four ids from 0x1C4 + 4 (n - 7): V1, V2, V3 and SP (the Dark Soul's
  AI 38, id 0x1DC); its battle sprite list 2's 0x20 + n - 7.
- **The control** (`--dev xnavicontrol`, `tools/xcontrol.py --navi`):
  BN6's own HeatMan derived by the same walk from BN6's ROM and moved as
  Navi AI 25 with ids 0x1C4-0x1C9. Fought by the same keys as the native
  HeatMan, his battle is the same byte for byte in EWRAM and IWRAM frame
  for frame (fight, statuses, deletion), each difference the id, the AI
  index or the copies' places.
- **A Navi's recipe** (`src/emu/xnavi_bn5.c`, written by
  `tools/xnavirecipe.py`; [The Navis' recipe generator](../devtools/navi-recipe-generator.md)):
  its code walked from its states, hooks and companion routines; the
  objects it spawns that BN6 runs otherwise moved through the
  second-level dispatch, one decision a type for every track
  (`tools/bn5_twins.py`, TYPES: moved unless BN6 runs the same code and
  draws the same art); its calls paired with BN6's twins (the families'
  pairs and audits, `tools/bn5_navi_twins.py`'s beside them); its state
  numbers from 6 made BN6's from 8; its pre-step and hooks as BN6's
  tables take them (hook A the empty hook, its frozen and bubbled hooks
  its paralysis hook: BN5 freezes and bubbles no Navi); its companion's
  start and end, a routine of BN5's copied where BN6's twin spawns an
  overlay kind BN6 lacks, the kind then one of its own past BN6's 19
  (`XN_KIND_OF`) with its masks copied; its panel types BN6's (BN5's
  lava 8 and sea 0xA as BN6's 13 and 14); KnightMan's chip by its name;
  Colonel's table of two entries by side moved from BN5's `0x02034000` to
  `BN6_COLONEL_TABLE` (`0x0203FF60`, zeroed with each battle's RAM) and
  his slash's three effects in a row. What BN6's engine lacks goes to the
  Navis' stubs (`XN_STUBS`): BN5's panel claims, which no battle in BN6's
  engine reaches (their gate, BattleState +0x32 bit 0x40, is set only by
  BN5's battle setup at `0x0802D594`, which BN6 has no twin of: r0 0, as
  BN5 answers without it), and the DS version's chip AI, which no recipe
  installs (logged).
- **What it draws**, in issue #82's ranges for the Navis: its sprites
  copied once into the shared art whichever Navi draws them, at list 4's
  0xA0-0xB5 and list 5's 0xA0-0xA4 (a sprite of list 3 copied into list
  4, its load's list offset made list 4's), BN6's own where BN6 lists the
  same bytes; its effects BN6's of the same sprite, animation and palette,
  else its own from 0xA0 (to 0xBF; 0xA0-0xAC taken, one number a BN5
  effect whichever Navi or engine object spawns it); its sounds BN6 numbers otherwise copied in by `xsong.c` at
  the slot issue #82's plan gives each BN5 sound whichever track plays it
  (0xC0 at 0x62, 0xB7 at 0x61, 0x70 at 0x5F, 0x117 at 0x5E, 0xE1 at 0x5B,
  0x115 at 0x5A, 0x105 at 0x59, 0x10C at 0x58, 0x10D at 0x57, 0xCF at
  0x56), copied only where the slot still holds BN6's empty header.
- **The engine's objects moved once** (`src/emu/xnavi_objects.c`,
  `tools/bn5_objects.py`): BN5's image object, the first kind's type 5,
  which ShadowMan and Colonel spawn through its spawner, is BN6's type 5
  in name only (BN6 reads a kinds table of its own, its sprites at other
  numbers). Its module whole, `0x080B9A5C`-`0x080B9D10` (the kinds table,
  58 kinds of five bytes; the type's routine; the spawner every attack
  calls; its helpers), is one block installed before any family or Navi
  at `XN_OBJECTS` (`xinstall_core`), its type moved through the
  dispatch for every track; a Navi's or a family's call to its spawner
  goes to the copy (WuNote's, its image kind 0x30). Its kinds' sprites, one
  table for every track (`tools/bn5_objects.py`): the twelve kinds whose
  sprite BN6 lists with the same bytes name BN6's (two at another
  number), kind 3 (ShadowMan's and Colonel's, BN5's 3:0x00) a copy at list
  4's 0xB5, kinds 0x0D and 0x23 the Navis' at list 5's 0xA5 and 0xA6,
  WuNote's 0x30 (BN5's 4:0x21) at list 4's 0x6C of the families' range,
  the other 41 keep BN5's bytes, reached by no installed code
  (the generator refuses a recipe that spawns one; a track that needs one
  adds its art there from its own range). BN5's sea's marker, the
  fourth kind's 0x3E (BN6's 0x3E is its invisibility's shadow, drawn
  otherwise and spawned by nothing), is a second module of the same unit,
  `0x080E4C4C`-`0x080E4D3C`, for the sea's submerge (below); the sea's
  splash, BN5's effect 0x63, which BN6 lacks, comes with them as BN6's
  0xAC.
- **The proof**: `build.py xdiff`, BN5's own battle against BN6's with the
  Navi moved, every draw of BN6's replayed from BN5's: each Navi's V1
  alone with MegaMan kept from harm (`*_v1`), V1 against MegaMan's 62
  inputs (`*_v1_keys`), SP alone (`*_sp`); its objects, its moved types'
  objects, its effects and its overlay compared in BN6's numbers, and
  what each draws compared by its sprite's bytes, never its number
  (`src/dev/xdiff_sprite.c`). `--mutations` breaks each Navi's copy the
  way a transplant bug would (a state number, an effect, an overlay's
  kind, a sprite's number, a sprite table's entry, the image object's
  kind and its twin's number, a copied sound): each FAILs its spec.
- **Where they fight**: a battle the `foes` step starts names their ids
  beside BN6's (`0:foes 1C8@52` ShadowMan V1; [A battle on demand](../devtools/battle-on-demand.md), A battle
  on demand), and the run's guardians are BN5's Navis in BN6's engine by
  the same ids since the last round ([BN5's Navis](../game/bosses/bn5-navis.md);
  `--guardian 24`-`29`; One net, The last round).
- **Status** (as built): the six pass all three specs, ToadMan's SP with
  BN5's sea's submerge in BN6's engine (One net, The sea's submerge and
  hold, above), which his own specs on the sea prove beside it
  (`toadman_sea`, `toadman_sea_elec`; `tomahawkman_sea` the hold).
  Colonel's write of BattleState +0x5C's bits (0x10 or 0x20, his side,
  as his spawn ends), whose 0x10 BN6 reads as the field's slide at a
  battle's end, goes to a field of the Navis' own instead ([Navis moved into BN6's battle engine](../rom/navis.md),
  Colonel's sides' bits); BN5's one reader, the obstacles' dark hole
  beside his panels, has no twin in BN6 (open, with the obstacles).
- **The Dark Soul** (AI 0x16, id 0x18D; BN6's AI 38, id 0x1DC):
  moved with BN5's chip AI, which BN6 dropped ([Navis moved into BN6's battle engine](../rom/navis.md), The
  Dark Soul in BN6's engine): its machine (`0x0802BA18`), its 31
  routines (`0x0802BD96`-`0x0802C0E0`) and its approach
  (`0x08029B40`-`0x0802AE70`) copied with it, a dead call into
  `0x081BC994` (data, behind a count nothing raises) answered 0, the
  Navis' buster (BN5's attack setup 0x3E, family 0x16) made by a stub as
  BN6's family 0x66 in the grown table, the chip it plays started as BN6
  starts a chip (setAttack with BN6's record's family), BN6's twins of
  BN5's two tests of its id (its collision's region 0x13; arriving in a
  fight under way, as a failed Chaos Unison brings it, no entry in the
  foes' list of four that shows their HP, BN6's `0x0801DC7C`, where at a
  battle's start both games list it as any foe) made by hooks. Its name
  is DarkSoul, the word BN5's script calls it by (`BN5_DARK_SOUL_NAME`):
  BN5's names archive gives its id DrkCloud, which BN5's own first Custom
  screen shows for it, and BN6's second screen names it as it fights. What BN6's engine has no room
  for is kept apart: its AI data's 0x140 bytes (BN6's are 0x100; BN5's
  chip AI keeps its state from +0xF0) in the battle's RAM, its two
  lists there too, and the chip records it reads, BN6's with the four
  bytes BN5 gives a meaning of their own (the approach, the region it
  aims from, two flags), by BN6's id. Its lists are BN5's battle's own
  making, from the save's record of the chips MegaMan uses most, which
  BN6 keeps none of: in BN6 they are empty, so it moves and fires its
  buster; a list given in BN5 and written into BN6's by the chip
  registry is proved too (`darksoul_cannon`, `darksoul_sword`: it takes
  the chip, goes where it reaches MegaMan, uses it, turns its list). Its
  DS versions of the six are left out (the design). Where a failed Chaos
  Unison spawns it, BN5's `0x08006AAE`, BN6's twin is
  `BN6_SPAWN_ENEMY_1` (`0x08007696`, the same body with its two entries
  in the other order), for the Souls track.

- **The Darkloids** ([Stage 4, as built: BN5's Darkloids](../game/liberation/building/stage-4-darkloids.md)): BN5's AI
  13-16, ShadeMan, BlizzardMan, CloudMan and CosmoMan, the Liberation
  Missions' bosses, seats 7-10 past the Dark Soul's (`XN_SEAT`): AI 39-42,
  ids 0x1E0-0x1EF (V1, V2, V3, SP), overlay kinds past the Dark Soul's,
  battle sprites at list 2's 0x27-0x2A (`xnavi_sprites.c`, in the sprites'
  room after the Dark Soul's), homes 8-11 past the control's, and their code
  and art in a room of their own past the shared art (`XN_MORE`,
  `XN_MORE_ART`: four slots of each; the shared art ends there). Moved:
  BlizzardMan and ShadeMan (`blizzardman_*`, `shademan_*`: PASS), with
  four rules the generator learned from them: a moved object's compare of
  its parent's CurAction (`ldr rP,[r5,#0x4C]; ldrb rX,[rP,#9]`) with one
  of the Navi's own states gets BN6's number, two up (BlizzardMan's slide's
  hit, the third kind's 0x2F, at 0x080CA8B8); a Navi's state or hook in
  another Navi's code of BN5's, with no BN6 twin, is walked and copied
  with him (ShadeMan's 9 and 12, in AI 21's, his last form's); a write of 0
  to an object's DirectionFlip is no flip (`no_flip`'s pair); and
  setPanelType's sites whose type the back walk does not reach are read by
  hand (`bn5_navi_twins.py`'s PANEL_SITES: BlizzardMan's ice, 7 in both).
  Two engine pairs audited: the HP lowered (BN5's also tallies a Navi's
  harm in a side record BN6 lacks) and an attack's end showing its user
  (BN5's hidden is 0x80000004, which xsea.c's hook makes BN6's see).
  Their sounds: BlizzardMan's 0xE3 copied at slot 0x40, the Darkloids'
  from the first slot up (the chips' go down from 0x4B). The differential
  watches only the spec's own Navis' code now (`xd_log_navis`): the Dark
  Soul's copy of BN5's chip AI's region (0x08012C30) is the engine's in
  another Navi's battle, as the families' copies are, and a draw in it
  keyed alike in both games (ShadeMan's bats' panel, BN6's twin
  0x08015E46). A word a moved object's spawn takes from a register its
  caller never set, which nothing reads, is a layout's rule
  (`spawn_leftovers`: the 0x2F's Z, each engine's leftover r3). Each SP
  spec has MegaMan at (1,2): from (2,2) BlizzardMan's Snowball lands on
  his ice, which BN6's engine answers with its own freeze, and two of
  ShadeMan's bats strike in one frame, which the two engines' hit passes
  take in their own order; the battle the same otherwise.
- **CosmoMan** (AI 16, AI 42 in BN6, ids 0x1EC-0x1EF; `cosmoman_v1`,
  `cosmoman_v1_keys`, `cosmoman_sp`: PASS): his eight object types moved,
  each recorded in `bn5_twins.py`'s TYPES as the generator derived it (no
  BN6 type runs the same code on the same data and art), the one decision
  his recipe had. Three of his objects take a word from a register their
  spawn's caller never set, part of which the spawner then writes itself,
  so `spawn_leftovers` rules may name the bytes they compare
  (`compares`): the third kind's 0x79's +4 its count's byte over each
  engine's r4, the 0x6C's Z its upper half 0x10 over each engine's r3
  (BN5's 0x41D0 below it, BN6's 3), the 0x6D's Z its 0x6C's, each read in
  the ROM (`src/emu/bn5_darkloids.h`; [BN5's Liberation Missions](../rom/liberation.md),
  BN5's Darkloids); the engine's spark where one of them struck takes its
  Z from it, so its Z is read by the struck attack's rule. Five mutations
  break his copy (a type spawned as another of his, a hit's offset, a
  sprite's number, the 0x6C's height, the 0x79's count) and FAIL. His
  attack below half his HP (`BN5_COSMO_HALF_HP`) is reached by no spec:
  no spec brings him under half.
- **Team ProtoMan's room** (BN5's AI 1-6; [Team ProtoMan: foes and Souls](../game/team-protoman.md),
  The build, unit 1): seats 11-16, Navi AI 26-31, ids 0x1F0-0x1FF for
  ProtoMan to NapalmMan and 0x3E4-0x3EB for MagnetMan and Meddy (the
  formula's 0x1B0-0x1C3 are BN6's player's forms), their homes, code and
  art in `FREE_TEAM_NAVIS` (`XN_TEAM_*`), their battle sprites at list 2's
  0x2B-0x30. Their code rooms are `XN_TEAM_SLOT_CODE` (0x2800: the
  largest walk, ProtoMan's, 8648 bytes), which leaves the shared art a
  second room (`XN_TEAM_SHARED`, 0x7800 bytes to `FREE_TEAM_SPRITES`): a
  sprite of any Navi's goes there once `XN_SHARED_ART`'s is full, as it was
  for NapalmMan's 5:0x05; the Navis' numbers grew with him to list 4's
  0xA0-0xBA (BN4's late families keep 0xBB-0xBF, `LATE_LIST4`, which
  reached down into his before) and list 5's 0xA0-0xAF, the moved
  objects' still taken before Team ProtoMan's (the sea's splash 0xAC).
  The shared art's third room is code-low's free tail
  (`FREE_TEAM_SHARED_MORE`, `XN_TEAM_MORE`, 60 KB), which GyroMan,
  SearchMan, MagnetMan and Meddy fill; a sprite past the Navis' numbers
  of its list goes to list 2's 0x80-0x9F (`NAVI_LIST_SPILL`, skipped by
  the families' `LIST_NUMBERS`), its load's list offset made list 2's.
  DarkProto's art (some 290 KB, lists 0-2 and 5) fits no room left
  (open).
- **NapalmMan** (AI 4, Navi AI 29, ids 0x1FC-0x1FF; `napalmman_v1`,
  `napalmman_v1_keys`, `napalmman_sp`: PASS): his five object types moved
  (the third kind's 0x33, 0x38, 0x39, 0x3A, the fourth's 0x12), his
  companion's start and end copied (`COMPANIONS`: BN5's overlay kind 4, his
  own 33), his chip below half his HP the one he holds next (BN5's "the
  chip an object uses", `CHIP_OBJECT_USES`), his flames over a pattern of
  panels given the Navis' 0xAD in r2 (`EFFECT_CALLS`), each fact in
  `src/emu/bn5_team_protoman.h`. His 0x33 spawns the third kind's 0x00,
  which MegaMan's buster's hit is in both engines too: by the rule of TYPES
  its one copy is now the Navis' (installed before the chips'), its kinds'
  rows fixed in it as `xchiprecipe.py`'s TABLE_SPRITES and TABLE_PANELS fix
  them (`kind_table_art`, `kind_table_panels`; the table copied whole, its
  31 rows), and Guard's unit spawns it. Five mutations break his copy and
  FAIL (two effects' numbers, the flames' among them, a hit's offset, a
  sprite's number, his overlay's kind). His 0x38's spawn of his 0x39 and his
  CurAction 9 are reached by no spec. No spec of his sets a lava
  panel: his flames are the 0x00's kind 0x11, which sets none; the table's
  lava row (26) is BN6's 13 in the copy, as the chips' was.
- **GyroMan** (AI 2, Navi AI 27, ids 0x1F4-0x1F7; `gyroman_v1`,
  `gyroman_v1_keys`, `gyroman_sp`: PASS): his four object types moved (the
  third kind's 0x19, 0x1B, 0x1C, 0x1E), his companion's start and end
  copied (`COMPANIONS`: BN5's overlay kind 0, his own 31; the end the body
  of Colonel's), each fact in `src/emu/bn5_team_protoman.h`. His sprites
  are the first in list 2's spill (0x80-0x84), his overlay's 4:0x0A
  (0x6000 bytes) the first in the third room; his three sounds BN6
  numbers otherwise (BN5's 0x103, 0x104, 0x14C) take BN6's empty slots
  0x43-0x45 (`xsong.c`, Team ProtoMan's from 0x43 up). Five mutations
  break his copy and FAIL (his overlay's kind, a sprite's number, a
  sprite's list, a hit's offset, a state's number).
- **SearchMan** (AI 3, Navi AI 28, ids 0x1F8-0x1FB; `bn5_searchman_v1`,
  `bn5_searchman_v1_keys`, `bn5_searchman_sp`: PASS): his seven object types moved
  (the third kind's 0xAA, 0xAB, 0xAF, the fourth's 0x5A, 0x5B, 0x61,
  0x62), no companion of his own, his status hooks his own code; two of
  his objects compare their parent's CurAction with his states (10 and 11,
  BN6's 12 and 13: `BN5_SEARCH_PARENT_STATE`). His sprites take list 2's
  spill (0x85-0x8A), his effect 0x14 the Navis' 0xA0 (NapalmMan's too); his
  sounds were all copied before him; his battle sprite at list 2's 0x2D,
  `FREE_TEAM_SPRITES` + 0x1F000. His fourth kind's 0x5A takes its Z from
  the r3 his state sets to an object's address (his AI data's +0x2C) and
  writes Z's upper half itself: the low half, each game's address's, is a
  `spawn_leftovers` rule of the fourth kind (its spawner `0x080031CC` the
  third's with kind 4). Five mutations break his copy and FAIL (a
  sprite's number, a sprite's list, a parent's state, a hit's offset, a
  state's number); his effect 0x14's two sites are reached by no spec.
- **Meddy** (AI 6, Navi AI 31, ids 0x3E8-0x3EB; `meddy_v1`,
  `meddy_v1_keys`, `meddy_sp`: PASS): her seven object types moved (the
  third kind's 0x09, 0x0C, 0x0D, the fourth's 0x0B, 0x0E, 0x0F, 0x3C), no
  companion of her own. One rule the generator learned from her: an effect
  whose id is put beside a parameter (`movs r4,#id; orrs r4,rX`, rX shifted
  up past the id's byte: her 0x0C's and 0x0D's spawn of the fourth kind's
  0x00, the parameter a byte of `BN5_MEDDY_EFFECT_KINDS` by its kind) is
  read by the movs's id, effect 0, BN6's own. Her sprites take list 2's
  spill (0x8B-0x8D) and the third room, her effect 0x44 the Navis' 0xB0,
  her sound BN5's 0xE0 BN6's empty slot 0x46. Five mutations break her
  copy and FAIL (a sprite's number, two sprites' lists, a hit's offset, a
  state's number); her effect 0x44's site and one object's compare of
  its parent's state are reached by no spec.
- **MagnetMan** (AI 5, Navi AI 30, ids 0x3E4-0x3E7; `magnetman_v1`,
  `magnetman_v1_keys`, `magnetman_sp`: PASS): his object types (the third
  kind's 0x60, 0x62, 0x63, 0x64, 0x66, the fourth's 0x19) moved, and his
  child at his place, the first kind's 0x55, through `SPAWNS_OTHER` (BN6's
  0x55 runs the same code on its own list 4's 0x3C, another sprite). The
  panels he lays are BN5's magnet panel, BN6's 15 (One net, The magnet
  panel): the generator makes his type 5 15 where he sets it, counts it
  (BN5's panel counter, `PANEL_COUNTERS`) or compares it through a
  register, the rule that also made the Dark Soul's counts of lava and sea
  BN6's 13 and 14. His battle sprite at `FREE_TEAM_SPRITES` + 0x2A000, his
  sound 0x100 at BN6's slot 0x47, his child's sprite in the shared art's
  fourth room. Five mutations break his copy and FAIL (the panel he lays,
  a state's number, a sprite's number, his child's sprite, a hit's
  offset); the pull's own breaks show in MegaMan's state alone, where the
  comparator stops without a verdict, and his count and check of his
  magnet panels decide in no spec.
- **GridMan** (AI 18, Navi AI 44, ids 0x3F0-0x3F3; `gridman_v1`,
  `gridman_v1_keys`, `gridman_sp`: PASS since 12 November), the first of
  Nebula's two of the story. *Their seats:* LarkMan's and GridMan's AI 17
  and 18 met Team ProtoMan's seats 11-12 by `XN_SEAT`'s old formula; they
  are seats 17-18 now, AI 43-44 (`XN_NEBULA_FIRST_AI`, between the
  Darkloids' 42 and BN4's 48), ids past MagnetMan's and Meddy's
  (`XN_MORE_IDS` + 4 (seat - 15): 0x3EC, 0x3F0), overlay kinds past
  BN6's by their seat, battle sprites list 2's 0x31 and 0x32, and a room of
  their own (`FREE_NEBULA_NAVIS`, `XN_NEBULA_*`: homes, code rooms of
  0x3000, art rooms, GridMan's battle sprite; LarkMan's in
  `FREE_NAVI_SPRITES`' tail), placed after the layers' headroom gave
  `code-low` its back ([Free space](../engine/emulation/free-space.md),
  Moving regions). They install after Team ProtoMan's (`NEBULA` in
  `tools/xnavirecipe.py`), so no other Navi's numbers or places moved.
  *His types:* the first kind's 0x18, the third's 0x94, 0xC6 and 0xC8 moved
  as his; the third's 0x95 (Football's ball) and the fourth's 0x58 (his
  Navi chip's tackle's effect), the chips' copies until then, are his copy
  now, the chips' GridMan and Football units calling into it (the chips'
  `navi_modules` reading `XN_NEBULA_CODE`). BN4's chips had taken
  third-kind dispatch numbers from BN5's own that no BN5 track had moved
  (`tools/xchiprecipe_bn4.py`, `TYPES3`): his 0xC8 and 0xC6 were
  Counter's and WhitWeb's, until the BN4 chips' reservation (PR #376)
  kept every BN5 Navi's numbers apart (`tools/bn5_twins.py`,
  `dispatch_reserved`) and moved those units off them. *His
  afterimage:* his effect 0x5F draws his own battle sprite (BN5's list 2
  0x12); the recipe names that sprite at its place, list 2's 0x32
  (`NAVI_SPRITE_PLACES`; `xnavi_recipe.c` `own_sprite`), where the effects'
  table finds it, `xnavi_sprites.c` writing it and the install only
  checking its bytes. His attack's routines are pool words his states store
  in AttackVars +0x28 and call by `bx` (`BN5_GRID_ATTACK_POOL`), moved with
  his blocks; the comparator reads a pointer into the new room as the
  code it came from (`tools/xdiff_snap.py`, `XN_NEBULA_ROOM`), and the
  third kind's 0xC6's Z is each engine's leftover r3 (`spawn_leftovers`,
  `BN5_GRID_SPAWN_C6`), as BlizzardMan's slide's hit's. Five mutations
  break his copy (an effect's number, the 0xC6 spawned as his 0xC8, his
  attack's routine left at BN5's address, a sprite's number, a parent's
  compare) and FAIL. Not yet a guardian: Oran Area's second, as BN5's
  record sets his SP on Oran Area 2, comes next ([BN5's
  Navis](../game/bosses/bn5-navis.md)).
- **LarkMan** (AI 17, Navi AI 43, ids 0x3EC-0x3EF; `larkman_v1`,
  `larkman_v1_keys`, `larkman_sp`: PASS since 12 November), Nebula's
  first seat. *His types:* the third kind's 0xB0 and 0xBB moved as his;
  the third's 0xBA, 0xC7 and 0xCB (his Navi chip's, WildBird's) and the
  fourth's 0x28, the swords' afterimage (Z Saver's, MoonBld's, Katana's,
  ShadowSoul's), the chips' copies until then, are his copy now, the
  chips and the Souls calling into it (the Souls' generator reading the
  Navis' newer rooms). *His afterimage:* its object loads the sprite its
  spawner hands it in r4 (`BN5_AFTERIMAGE_SPAWN`, `BN5_AFTERIMAGE_LOAD`);
  his own spawn ORs a pool word into r4 (`BN5_LARK_AFTERIMAGE_WORD`,
  list 2's 0x11), which the recipe sets to his place, list 2's 0x31
  (`tools/xnavirecipe.py`, `afterimage_sprites`), the chips' and
  ShadowSoul's spawns handing MegaMan's own. The chips' LarkMan and
  GridMan units draw the Navis' one copy of each battle sprite now, as
  CosmoMan's does (their `navis` a 32-bit mask). The comparator reads a
  moved Navi's battle sprite where an object keeps its number (the
  afterimage's +5) by `xnavi_sprites.c`'s places, and the third kind's
  0xB0's and 0xBA's +0x3C low halves as each engine's leftover r3, their
  heights compared (`spawn_leftovers`, `compares`). Five mutations break
  his copy (the afterimage's word, both objects' heights, a wait, a
  sound) and FAIL. Not yet a guardian: where BN5's record sets his SP,
  the Undernet's second, waits on its design ([BN5's
  Navis](../game/bosses/bn5-navis.md)).
- **CloudMan** (AI 15, AI 41 in BN6, ids 0x1E8-0x1EB; `cloudman_v1`,
  `cloudman_v1_keys`, `cloudman_sp`: PASS since 12 November). *His object
  types* (the third kind's 0x0B, 0x36, 0x37, 0x3D, 0x40, 0x41, 0x44, 0x5F)
  are recorded in TYPES as CosmoMan's were, each moved. *His RAM table at
  0x0203CA30* is no table but BN5's third kind's objects less 0x10, read
  by his search for his clouds (`BN5_CLOUD_FIND_37`: type 0x37 on a side
  but a slot), which BN6's engine cannot answer by the same bytes: a moved
  type's object holds the dispatch's type (0xD2) and BN5's number in the
  dispatch's side table, so the search is the Navis' stub `XN_STUB_CLOUDS`
  (`bn5_navi_twins.py`'s STUBS; `src/emu/xnavi_room.c`): BN6's 32 objects,
  the type at +1 the dispatch's and the side table's number by the slot
  0x37, the list and count BN5's, never a wider literal. The stubs' room
  grew to 0xA0 bytes for it, the sea's code and the moved objects one
  0x100 on (`XN_OBJECTS` at `FREE_NAVI_CODE` + 0x200,
  `tools/bn5_objects.py`). *The sprite loaded by a number made otherwise
  (0x080C5EEC)* is the third kind's 0x0B's, whose record by its kind gives
  the sprite (kinds 2-5 his, `BN5_CLOUD_KINDS_0B`): its one copy is the
  Navis' (TYPES: the Navis install first of those that spawn it), its twelve
  rows' sprites given BN6's numbers by the Navis' `kind_table_art` from
  `xchiprecipe.py`'s TABLE_SPRITES. The chips' `Bubbles` helper, which held
  that copy until then, is gone: SideBub's and the Navi chip CloudMan's
  calls to its spawner (0x080C5FCE, 0x080C5FE8) go to his copy
  (`xchiprecipe.py`'s NAVI_SPAWNERS, its `navi_modules` reading the
  Darkloids' room `XN_MORE` and Team ProtoMan's too), and the code mover
  takes a call into any Navi code room (`xvirus.c`'s `navi_room`). Three
  rules the generator learned from him, each found by his specs' FAIL at
  t9 and t51 (tried first on 9 November, which stopped there):
  - a moved object's every compare of its parent's CurAction is BN6's
    number, not the first alone: his 0x5F compares it with 8 and then 9
    (0x080D1EEA, 0x080D1EF0: `BN5_CLOUD_5F_PARENT`), its second left at
    BN5's 9 kept its countdown (+0x64) from running;
  - a block whose code stores a byte of its own routines' addresses keeps
    its low byte in the copy (`LOW_BYTE_KEPT`): two of his clouds'
    sub-states (`BN5_CLOUD_ROW_LEFTOVER`, 0x080CBD9A and 0x080CBDE4) write
    r1, which they never set, as the cloud's panel row (+0x13), r1 the
    sub-state's own address its state machine branched through
    (0x080CBC54), so a cloud parks on an off-field row its routine's low
    byte names (0x7D in BN5, 0x9D in a copy placed otherwise);
  - a type a Navi's code moves is the Navis' copy in TYPES or a decision
    (`types_decided`): the chips' or the families' copy as well would
    leave the dispatch running one of them. It flags Team ProtoMan's
    ProtoMan (AI 1) today: the third kind's 0x2B and 0x69 and the fourth's
    0x28 are the chips' copies.
  His five sprites take Navi numbers before Team ProtoMan's (he installs
  first, a Darkloid's seat): list 4 to 0xBA, then list 2's spill (0x80-
  0x8F with NapalmMan's, GyroMan's and SearchMan's, theirs moved up by
  his), so BN4's late families keep 0xBB-0xBF. His 16 KB in the first
  room moved Team ProtoMan's later sprites on, past the third room's end:
  the shared art has a fourth room, the art zone's tail
  (`FREE_NAVI_SHARED_FOUR`, `XN_SHARED_FOUR`, 24 KB at +0x7FA000), where
  Meddy's last two sprites go. Before
  that split (PR #356) his numbers reached 0xBE and 0xBF and VolGear and
  Kilby refused to install. Five mutations break his copy
  (the 0x5F's second compare, the cloud spawned as his 0x36, its sprite's
  number, the 0x0B's row 2's sprite, the search's type) and FAIL.
