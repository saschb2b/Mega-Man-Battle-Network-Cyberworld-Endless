---
type: Engine Mechanism
title: "Transplanted Navis"
description: "Issue #82's Navi track: BN5's Team Colonel Navis, the Dark Soul and two Darkloids moved into BN6's battle engine (`src/emu/xnavi_install.c`, `tools/xnavirecipe.py`), BN6's Navi tables grown, each proved frame by frame by `build.py xdiff`."
tags: [multirom, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-25T00:29:32Z }
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
  take in their own order; the battle the same otherwise. CloudMan and
  CosmoMan are derived (`--report --ai 15 16`) with decisions open: their
  object types, CloudMan's RAM table at 0x0203CA30 and a sprite loaded by a
  number made otherwise (0x080C5EEC).
