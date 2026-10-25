---
type: Engine Mechanism
title: "Transplanted families"
description: "Issue #82's virus families moved from BN5 into BN6's battle engine (`src/emu/xvirus.c`): generated recipes (`tools/xrecipe.py`), the all-or-nothing install, the engine's grown tables and second-level dispatch, and all 25 families proved by `build.py xdiff`."
tags: [multirom, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-13T00:57:11Z }
---

# Transplanted families

Issue #82: BN5's virus families moved into BN6's own battle engine, so a
battle of BN6's holds them beside BN6's viruses. BN5's and BN6's battle
code is one engine a version apart: a family is its own few routines
(its states, their phases, its hooks) over a closed set of the engine's,
with the same objects and fields in both games, so its code can be copied
and its calls aimed again (`src/emu/xvirus.c`):

- **A recipe of numbers** (`src/emu/xvirus_families.c`): the family's
  blocks of code in its game (each with the CRC32 of its bytes) and where
  they go, the ranges in them that are data (literal pools, jump tables,
  tables of numbers), the routines its calls go to and BN6's in their
  place, the literals that name RAM or ROM outside its code and BN6's
  values for them, the immediates to change (each checked against the
  instruction it sits in), its state table in BN6's layout (BN6's generic
  states 0-7, its own from 8), its hooks, its records, its versions'
  names, its AI index (32 on) and its six enemy ids; and what it draws
  (its sprites by their lists and numbers in its game and in BN6, its
  effects and its attacks' types by their ids in both) and, where its
  game lays struct1 out otherwise, the numbers BN6's layout takes. Nothing
  of the game's is in it: the code, records, sprites and names are copied
  from the player's ROMs as the core starts.
- **The install**, once a core, all or nothing (`xvirus_install_family`):
  every write is staged and made only when every check has passed. Each
  block is copied whole to a fixed place past `EMU_FREE` (docs/
  [Running the game's own logic](../engine/emulation/overview.md), `+0x368000`), the same distance from a word boundary, so
  its loads from its own pools and its calls within itself hold; its
  code is walked instruction by instruction (a load from the pool must
  find one of the recipe's pools, a branch stay in its block, no ARM
  code); each word of its pools and tables that points into a block is
  moved with it, one that names RAM or ROM elsewhere must be one of the
  recipe's; each BL out goes through a 12-byte veneer (`push {r0,r1};
  ldr r0,[pc,#4]; str r0,[sp,#4]; pop {r0,pc}` and the target: every
  register, lr and the flags reach the routine as the BL left them, and
  it returns straight to the moved code), BN6's routines being beyond a
  BL's 4 MiB reach from there (`src/emu/thumb_reloc.c`). Its entries go
  into BN6's tables as they were grown for every family (the engine's
  room, below), each slot checked to hold the fill first: the eleven
  per-family tables, its ice block's size and its held-chip lists' word
  at its AI index; its ids' triples; its names in the families' own
  archive; each word of BN6's changed is read first and must be the one
  expected. What it draws is copied too (a sprite decompressed where its
  game's is LZ77: BN6 draws a compressed battle sprite only once its
  battle's start put it in a cache), listed in the grown copy of its list
  at a number that holds the placeholder sprite there (one of BN6's own
  free numbers, or past the list's end), its effects past BN6's 108 in
  the grown effects; its attacks take a type the grown table leaves
  empty. data.c's readers are told of its ids (`EnemyMoved`), so the run's own
  code (the foes step, the second screen's names, a foe's HP and element)
  reads them as the engine does, not as BN6 has them (Mettaur, a blank
  name). Hooks
  keep AI 32 and up out of BN6's 32-byte delete count and out of its
  rare versions' chat of new virus data ([Hooks](../engine/emulation/hooks.md)). Every read of the engine's by AI index that a virus reaches was
  looked for ([Virus families moved into BN6's battle engine](../rom/virus-families.md), the ice block's size): the eleven tables,
  the held chips and two more 35-word tables, the delete count and the ice
  block's size.
- **The engine's room** (`src/emu/xvirus_grow.c`; [Virus families moved into BN6's battle engine](../rom/virus-families.md), the
  engine's room for them): every family at fixed numbers, all at once
  (the static model). Once a core, before any family's install and all or
  nothing, each table a family is found through is copied past
  `EMU_FREE` ([Running the game's own logic](../engine/emulation/overview.md), `+0x330000`) with BN6's entries and a
  fill after them, the one word of the ROM that names it pointed at the
  copy: the eleven per-family tables, the ice blocks' sizes and the three
  35-word tables by AI index to 256 AI indexes; sprite lists 1, 2, 4 and
  5, the effects and the first, third and fourth kinds' types to 256
  (three types taken by the second-level dispatch, below: the first
  kind's 0x5C, the third's 0xD2, the fourth's 0x91); the enemy ids'
  triples and their reward rows to 0x400 ids, the ids past BN6's read as
  its filler 0xBB. A family takes an AI index from 32 and six ids from
  0x200, past every id BN6 has (its Navis end at 0x190, the player's
  forms at 0x1C3); the battle reads 12 bits of an id as it spawns one,
  and its sprite preload, which read 9, is made to read as many; the
  names of the ids from 0x200 are an archive of their own, which a hook on
  BN6's name lookup hands out (it took an id past 0xFF for a Navi's).
  BN6's own battles read the copies as BN6's: with the room made they
  are the same at every byte (`tools/xcontrol.py`, `tools/xfamily.py
  --spec native`). Past it the plan goes on ([Memory the engine writes](../engine/emulation/memory.md), Memory
  the engine writes): the chip registry at `+0x400000` (The chip
  registry, below), the Souls at `+0x480000` (Transplanted Souls,
  below), the lava and sea at `+0x4C0000`, room kept for the Navis at
  `+0x500000` (their battle sprites first) and the chips' moved code at
  `+0x600000` ([The chips' code](chips-code.md)). The mode byte of a
  family's struct1 indexes BN6's 32 bytes for its families (read in one
  battle mode): kept, a family whose game's mode is past them given 0.
- **The second-level dispatch** (`src/emu/xvirus_dispatch.c`;
  [Virus families moved into BN6's battle engine](../rom/virus-families.md), the second-level dispatch): BN6 runs an object by
  its type, a byte, from its kind's table, and BN5's families spawn
  objects of their own types (CanGard's reticle is BN5's third kind's
  0x25, which BN6 runs as another attack). One type of each kind, dead in
  BN6 (the first kind's 0x5C, the third's 0xD2, the fourth's 0x91), runs
  whatever moved routine its object was spawned as: a moved family's
  calls to its game's spawners go to the dispatch's (one for each of
  BN5's spawners, the same kind and place in the run order), which look
  the type up in the kind's TWIN table and spawn BN6's own number (where
  the object is BN6's too) or else the kind's dispatch type, and write
  the game's number into the kind's side table in RAM by the object's
  slot (`0x0203FF00`, `0x0203FF20`, `0x0203FF40`, 32 bytes each, cleared
  by every battle's setup; [Memory the engine writes](../engine/emulation/memory.md#bn6s-ram), BN6's RAM);
  each frame the dispatch type's routine reads the number back and goes
  on to the kind's MOVED table's routine for it, as BN5's runner calls
  one. A recipe names its types (`XvType`: the kind, its game's number,
  BN6's twin or moved) and its moved objects' routines are its own
  blocks; MOVED's entries no family set let a stray object go as it first
  runs. The code is the project's own Thumb, made field by field, past
  `EMU_FREE` with the grown tables ([Running the game's own logic](../engine/emulation/overview.md), `+0x345000`); a
  moved block that reads its object's type is refused, so the moved code
  never sees the dispatch's. BN6's own battles are the same at every byte
  with it made (`tools/xcontrol.py`, `tools/xfamily.py --spec native`).
- **The control** (`--dev xcontrol`): BN6's own Mettaur, AI 1, moved
  through the same steps as AI 32 with ids 0x200-0x205. It must play frame
  for frame as BN6's: `tools/xcontrol.py` fights it and BN6's own under
  the same keys and compares every byte of EWRAM and IWRAM after each
  frame ([Testing](../engine/emulation/testing.md)), in two specs: a fight of shots and
  steps to its deletion (its own states 8-12), and the statuses a hit
  gives, one after another (BN6's generic states 4-7: paralysed, dragged,
  frozen in its ice block, bubbled; pushed along), then the fight. In
  both it does, to the battle's results, where the reward comes from each
  id's own drop row (the control's ids take Mettaur's). That is what is proven: seed 1, one Mettaur at one
  panel, the paths those keys reach. Not reached: state 3 (no way in for
  a virus in BN6) and its table-9 hook, a real chip's hit (the status step
  sets what a hit leaves; the hit's resolution before it is BN6's own
  code, which reads a family only through the tables the installer grows),
  the statuses that are no state (confusion,
  blindness, holds), and a moved and a native Mettaur compared in one
  battle (seen together by eye only).
- **The status step** (`--input "0:status freeze 90"`, play.py's
  `status freeze 90`): the battle's enemies (or one enemy id's) given
  paralysis, freeze, bubble, drag, slide, confusion or blindness as a hit
  leaves it, for the frames named; BN6's engine acts on it on their next
  frame.
- **The foes step** (`--input "0:foes 200@52/01@43"`, play.py's `foes`): a
  battle at once in BN6's engine with the enemies named by id at their
  panels, the moved families among them.
- **BN5's CanGard** (`xvirus_cangard`, AI 14 in BN5; [Virus families moved into BN6's battle engine](../rom/virus-families.md),
  BN5's CanGard): installed wherever BN5's ROM is read (`--dev
  nofamilies` leaves it out), as AI 32 with ids 0x200-0x205 (CanGard,
  CanGrdEX, CanRaid, CanRadEX, Canlada, CanladEX). Two blocks of BN5's:
  CanGard's own code (its decide, guard and fire states, its per-frame
  hook, its dying wrapper and its release hook) and its attack's, BN5's
  type 0x25, a reticle that walks to MegaMan's panel and strikes there.
  Their 39 calls out go through 39 veneers to 38 of BN6's routines, each
  paired by comparing the two bodies, and to the dispatch's spawner of
  the third kind. Five immediates change, each read first: its two
  states' CurActions (7 and 8 in BN5, 9 and 10 in BN6, whose two status
  states come before a family's own), its muzzle's effect (BN5's 0x57
  copied as BN6's 0x6C), the reticle's sprite (list 4's 0x24 as 0x5C),
  and the collision data's flags it reads (BN5's +0x68, BN6's +0x70). The
  reticle keeps BN5's type: spawned through the dispatch as its third
  kind's type with 0x25 in the side table, and run as BN5's 0x25 (before
  the dispatch its immediate was made BN6's empty 0xD1). Its state
  table is BN6's generic 0-7 but its own dying (2), its own from 8; its
  hooks BN5's per frame, and its release in BN6's tables 6 (paralysed), 7
  (frozen) and 8 (bubbled), which BN5 lacks, and 10 (dragged), as BN6's
  AI 17 gives one hook all four. Its struct1 laid out BN6's way (BN5
  keeps the shadow at +5, where BN6 ORs in an element: copied as it is
  it would be Heat with no shadow); its sprite at list 1's placeholder
  0x1E; no held chips; the smallest ice block (BN5 freezes none).
  `tools/xfamily.py` checks it in BN6's battles ([Testing](../engine/emulation/testing.md),
  Testing): it fires at MegaMan beside a Mettaur, its reticle striking
  for its version's damage; freeze, bubble, paralysis and drag each let
  its reticle go, and it fires again; each version is deleted with BN6's
  delete counts and its rare versions' byte untouched (BN6's own
  Mettaur, deleted the same way, changes them); and BN6's own Mettaur
  plays the same frame for frame with CanGard installed. What is not
  checked there: a real chip's hit on it (the status step sets what a hit
  leaves), and its guard against each kind of chip.
- **The differential** (`--xdiff`, `tools/xdiff.py`, `build.py xdiff`;
  [The differential: a moved family against its own game](../devtools/differential.md)): CanGard in BN6's engine against
  CanGard in BN5's own, one battle fought in each, frame by frame by each
  battle's timer, every byte of its objects (and MegaMan's, the panels,
  the order they run in, the sounds begun, the RNG words) read in BN6's
  numbers and each structure through its own layout, every difference
  explained or a failure. Its specs (`tests/xdiff`): `cangard_v1`, V1
  alone at (6,2) against MegaMan idle at (2,2) with 100 HP: the same over
  1200 fight frames, and in both what BN5's battle is known to do (its
  states 8, 9 at t3, 8, 10 at t5, a cycle of 129 frames then 97; its
  reticle on (6,2) at t17, a panel on at t23 and every 12 frames to (2,2)
  by t59, sound 0xAE at t76 as it fires; MegaMan 10 HP down at t85, 279,
  473, 667, 861 and 1055, every other strike, the others while he still
  flashes). `cangard_v1_keys`, the same with 62 inputs by the timer (41 taps
  of B, three held to charge, 18 steps between rows and columns) and
  MegaMan at 2000 HP: the same over 1200 frames, CanGard struck 15 times
  and only in its firing state (10), holding its guard (9) each time
  MegaMan stands out of its row. Its five other versions alone, as
  `cangard_v1` (2000 HP; CanladEX 3000): the same over 1200 frames each,
  its reticle striking for 40, 70, 100, 150 and 200 (its own table, BN5's
  `0x0810912E`, not struct2's) and walking a panel in 10 frames, then in
  5 from V3 (V1's 12). `cangard_mettaur`,
  the issue's formation (CanGard at (5,2), a Mettaur at (4,3)), a smoke
  spec: the Mettaur's own AI data differs from t35 (each game runs its own
  Mettaur, its shockwave and its step's effect: the spec's natives), and
  CanGard is compared on while its world stays the same: the same to
  t134, inconclusive at t135, where BN6's shockwave (22 frames a panel
  against BN5's 25) reaches MegaMan's panel three frames before BN5's;
  CanGard's objects stay the same after it to t362. No difference found
  was CanGard's code, its state table, an id, an offset, a call's twin or
  a table's entry: each was the engine's, each explained in
  `tests/xdiff/legit.json` by both games' routines. No mask touches
  CanGard's own records, its reticle's or its effects' in any spec: what
  the engine writes there in its own numbers is read as BN6's, as an
  enemy id is: the collision data's alliance words (BN6's entries of the
  table by alliance and region mostly carry bit 0x80 where BN5's do not,
  so each BN5 word is read as the BN6 entry at its place, and a hit's word
  as the OR of those it is made of), the renderer's tile place and palette
  slot (their rank among the family's own that frame: the counts run over
  everything drawn, MegaMan's blinking sprite among it), an object's slot
  (from its own). On what is not CanGard's:
  each game's panel restore timers (their bytes and values pinned);
  the blink of an object flashing after a hit, on the other frames (since
  the tooling's milestone, below, only where an object's bit follows its
  owner's: MegaMan's buster flashes);
  MegaMan's own sprites and the place of his charge glow; the player's AI
  data +0x0F BN6 sets as the Custom screen closes; BN6's panel
  reservation flag (since the milestone read from BN5's own reservations,
  no mask); BN5's buster's one more random draw; BN6's buster
  sound on a player a hit's sound holds; the hit spark's frame more of
  life. BN5's split of a hit's result (by the hitter's side, +0x6C and
  +0x70) and of its modifier (+0x18, +0x19) is no mask but held to its
  rule, the OR BN5 makes of them, compared with BN6's one word; BN6's own
  bytes of a hit (its multiplier and elements, +0x74-0x7B) must be 0,
  as they are where no element is in play. The comparator can fail: eleven
  breaks of the install as a transplant bug would make them (an
  immediate, a state number and an offset left as BN5's, a call aimed at
  nothing or at its wrong twin, a grown table's entry, a number in its
  data, the muzzle's effect id left as BN5's, alone and beside the
  Mettaur, the strike's effect id, the reticle's collision region) each
  fail their spec, nine of them where every timeline check still holds (`build.py xdiff
  --mutations`); since the dispatch, three more fail `cangard_v1`: the
  reticle's MOVED entry left the stray's, its TWIN entry made BN6's own
  0x25, and its spawn aimed at BN6's spawner as before the dispatch. What it did not reach then (12 October 2026), each
  reached since by the tooling's milestone (below): an extra object the
  subject spawns whose owner names MegaMan's or a native's, a family that
  draws random numbers, and a family whose objects flash or step onto
  panels. What it still does not reach: CanGard deleted, struck by a
  chip, or beside BN5's other families; an element in play; BN6's freeze
  and bubble (BN5 has neither; `tools/xfamily.py` covers them in BN6
  alone).
- **The differential's tooling milestone** (issue #82): each core
  carries hooks of its own (src/emu/hook.c keeps a table and a queue per
  core, found by the CPU that meets the BKPT; the guest's are the
  differential's alone: the run's guest battles, while they lasted, had
  none, their pictures and logs the same as before), on each
  game's own routines: its draws, every spawn of the first, third and
  fourth kinds, its sounds, effects, setAttack, a panel reserved, the
  blink and a sprite set up, each logged with its frame, its caller, the
  family's code that led to it and the outermost caller
  (`src/dev/xdiff_events.c`; [The differential: a moved family against its own game](../devtools/differential.md)). The
  comparator compares the family's calls by their call sites and gives an
  object to the family whose code spawned it. With `rng=replay` each of
  BN6's draws is given the word BN5's draw of the same key (the family's
  call site, or the caller's BN5 twin) drew from, so the family draws
  BN5's numbers whatever the engine drew around it; BN6's camera shake,
  which draws from its second word where BN5's draws from its first, is
  given BN5's numbers the same way, and BN5's buster's extra draw is the
  one draw left unmatched, the engine's own (both routines cited in
  `tests/xdiff/legit.json`, rng). BN6's reservation bit and the blink's
  phase are read from BN5's own events into its records as BN6 would have
  written them: translations, not masks. The control runs with the
  replay on and stays the same frame for frame (`tools/xcontrol.py`: each
  of the native run's 8800 and 10783 draws replayed, none unmatched).
- **BN5's Handi** (`xvirus_handi`, AI 4 in BN5; [Virus families moved into BN6's battle engine](../rom/virus-families.md), BN5's
  Handi), the second family, the first that draws random numbers and
  moves: installed after CanGard as AI 33 with ids 0x206-0x20B (Handi,
  HandiEX, Hando, HandoEX, Handum, HandumEX), the static layout's second
  family (its home, code and sprite the second of each). One block of BN5's, its
  decision (a coin for its wait), the engine's wait and its attack: a
  teleport to a panel its callback picks at random (BN5's
  `0x08012DCC`, BN6's `0x08015FDC`, which draws), reserved as it goes,
  then a bomb dropped in front of it (the third kind's type 0x4B, BN6's
  own, an obstacle with HP that counts down and bursts for its version's
  damage, 50 to 200). Three entries of its attack's jump table name the
  engine's own steps (the teleport's start, the teleport, the wait): the
  installer takes them from the recipe's words, BN6's twins (each body
  compared; BN5's teleport also sets the facing again at its end, to the
  0 an enemy spawns with on a plain field, where BN6's leaves it: not
  reached on a field of another column pattern). 14 calls to 11 routines, two
  immediates (its wait's and its attack's CurAction). Its bomb is spawned
  by the bomb's own spawner, BN5's `0x080CF37E` paired with BN6's
  `0x080CF4EC` (BN6's type 0x4B in both), so none of Handi's code calls a
  spawner of the engine's: nothing of Handi's goes through the dispatch. Its specs, all
  with `rng=replay`: `handi_v1`, Handi alone against MegaMan idle: the
  same over 1200 frames, its three teleports (to 4,1, 4,2, 4,3) and its
  four bombs as BN5's battle has them, MegaMan 50 down at t306, t636 and
  t966; `handi_v1_keys`, cangard_v1_keys' 62 inputs: the same, Handi hit
  9 times, its bombs shot down but one, its teleports to other panels;
  its five other versions alone (`handi_ex` to `handi_v3ex`): the same,
  their bombs for 70, 90, 120, 160 and 200. Its breaks fail (`--mutations`):
  its wait's CurAction left as BN5's (t2), its panel's pick rebased to
  the routine after it (t21: it teleports onto MegaMan's side), its draws
  shifted onto the other word (t2: no draw of BN5's to give them).
- **The recipe generator** (`tools/xrecipe.py`; [The recipe generator](../devtools/recipe-generator.md), The
  recipe generator; [Virus families moved into BN6's battle engine](../rom/virus-families.md), The recipe generator's reads of
  BN5): every BN5 family's recipe derived from the two ROMs, numbers
  only; the installed families' written into `src/emu/xvirus_bn5.c`,
  which it writes and nothing else does, the rest reported
  (`.build/xrecipe/report.md`). Per family, in install order (AI 32 on, six ids each from
  0x200, code, homes and art at the static layout's next places): its
  blocks (a walk from its states and hooks to the next family's struct1),
  their data by how the code reads it, its calls with their twins (the
  project's pairs in `tools/bn5_twins.py`, then the symbol map's, then a
  BN6 routine of the same body, then where its neighbours put it), each
  pair's two bodies compared (the same; the same but for the offsets of
  loads and stores; or differing, served only once audited for what the
  family's code asks of it: [Virus families moved into BN6's battle engine](../rom/virus-families.md), The engine's differing
  twins), the immediates and pool words that are numbers in each game's
  own terms, its states, hooks, records, its held chips as BN6's chip ids
  (a chip BN6 has none of held out), what it draws, and its types for the
  dispatch. What the recipe format has gained for them, each checked by
  the validator and the staged installer: pool words that are numbers
  (`XvNumber`), bytes of its data that are ids (`XvByte`), a routine
  copied once more with a call aimed otherwise (`XvClone`, a status
  wrapper's), its types for the dispatch (`XvType`) and its held chips. A
  family is whole where no decision is open, and only a whole one is
  written; `build.py test` holds the generated CanGard and Handi to the
  installed recipes field for field (they are the same). Since issue
  #82's A2b every family BN5 has and BN6 lacks is whole and installed
  (BN5's families, all of them, below), what each derives in
  `.build/xrecipe/report.md`. Tuby and the unnamed AI 31-33 are left
  out: no battle record of BN5's names them (all 386 of its enemy lists
  read, none with an id from 0xB5 on; AI 31 and 32 have no ids at all).
- **BN5's families, all of them** (issue #82's A2b): the 23 after CanGard
  and Handi, installed after them in BN5's AI order wherever BN5's ROM is
  read, each by its generated recipe and nothing written by hand: AI
  34-56, six ids each from 0x20C to 0x295, their homes, code and art one
  after another ([Memory the engine writes](../engine/emulation/memory.md), the families' room). Per family
  (`.build/xrecipe/report.md` has the rest): its blocks and their bytes,
  its calls to the engine (each through a veneer to BN6's twin), its
  immediates and pool words given BN6's numbers, the object types it
  moved through the second-level dispatch (`shared`: run by the copy of a
  family before it, or of the engine's objects moved once), what it draws (its sprites copied and the effects
  given ids of their own, where BN6 has none of the same bytes), its
  sounds BN6 has no song of (each at its fixed slot), its held chips as
  BN6's ids, and what was made for it of the project's own Thumb:

| Family | BN5 AI | BN6 AI, ids | Code: blocks, bytes | Calls | Immediates | Moved types | Art: sprites, effects | Sounds | Held chips (BN6 ids) | Shims and pre-step |
|---|---|---|---|---|---|---|---|---|---|---|
| CanGard | 14 | 32, 0x200-0x205 | 2, 1432 | 38 | 5 | 3:0x25 | 3, 1 | - | - | - |
| Handi | 4 | 33, 0x206-0x20b | 1, 524 | 11 | 2 | - | 1, 0 | - | - | 1 NameID range |
| Powie | 2 | 34, 0x20c-0x211 | 3, 1464 | 36 | 9 | - | 2, 2 | 0xC0, slot 0x62 | 211 | 1 NameID range |
| Bugtank | 3 | 35, 0x212-0x217 | 3, 898 | 26 | 9 | 3:0x24 | 2, 0 | - | 157, 158, 163 | - |
| Dominerd | 5 | 36, 0x218-0x21d | 3, 1416 | 35 | 8 | - | 1, 0 | - | - | 4 NameID ranges, the pre-step, 2 clones |
| MetFire | 6 | 37, 0x21e-0x223 | 5, 1122 | 35 | 5 | 3:0x56 | 2, 0 | - | 143, 163 | 1 NameID range, 1 id in its data |
| Shakey | 7 | 38, 0x224-0x229 | 4, 1242 | 25 | 5 | - | 1, 0 | - | - | - |
| NinJoy | 9 | 39, 0x22a-0x22f | 3, 1200 | 28 | 6 | - | 2, 1 | - | - | - |
| Lark | 11 | 40, 0x230-0x235 | 2, 824 | 18 | 2 | - | 1, 0 | - | 211, 450, 456, 460 | 1 NameID range, 2 chip compares |
| BomBoy | 12 | 41, 0x236-0x23b | 6, 1562 | 40 | 7 | 3:0x3E | 2, 0 | - | - | - |
| Zomon | 13 | 42, 0x23c-0x241 | 2, 1388 | 23 | 7 | - | 2, 2 | - | 163 | 1 NameID range, 2 ids in its data |
| Drixol | 16 | 43, 0x242-0x247 | 5, 1572 | 37 | 7 | 4:0x78 | 2, 0 | - | - | 1 NameID range |
| Batty | 17 | 44, 0x248-0x24d | 2, 1372 | 43 | 5 | 3:0x6A | 3, 1 | - | - | - |
| Skarab | 18 | 45, 0x24e-0x253 | 2, 1612 | 47 | 11 | 3:0xC1 | 2, 0 | 0xB7, slot 0x61 | 163, 456 | - |
| Draggin | 20 | 46, 0x254-0x259 | 3, 1226 | 40 | 7 | 3:0x87 | 2, 0 | - | 156, 459 | 1 NameID range |
| Whirly | 21 | 47, 0x25a-0x25f | 3, 1822 | 43 | 7 | - | 2, 0 | - | 450 | 1 NameID range, 2 ids in its data |
| Marina | 22 | 48, 0x260-0x265 | 6, 2122 | 52 | 9 | 3:0x0B, 3:0x8F | 3, 1 | - | - | 1 NameID range, 4 ids in its data |
| Flashy | 23 | 49, 0x266-0x26b | 4, 1628 | 45 | 13 | 3:0x6F | 2, 0 | 0xBA, slot 0x60 | - | 1 NameID range |
| Eleogre | 24 | 50, 0x26c-0x271 | 3, 1300 | 25 | 7 | 3:0x6F shared | 1, 0 | - | - | 1 NameID range |
| Cactikil | 25 | 51, 0x272-0x277 | 5, 1698 | 57 | 5 | 1:0x11 | 2, 0 | - | - | - |
| WuNote | 26 | 52, 0x278-0x27d | 6, 2344 | 65 | 11 | 1:0x05 shared (the engine's objects'), 1:0x16, 3:0x84 | 1, 0 | - | - | - |
| Appley | 27 | 53, 0x27e-0x283 | 6, 2218 | 59 | 10 | 1:0x2F, 3:0xAC, 4:0x90 | 3, 1 | - | - | the pre-step |
| BigBrute | 28 | 54, 0x284-0x289 | 3, 1338 | 35 | 7 | 3:0x61 | 1, 2 | - | - | - |
| TinHawk | 29 | 55, 0x28a-0x28f | 4, 2384 | 37 | 11 | - | 2, 1 | - | - | - |
| Bladia | 30 | 56, 0x290-0x295 | 4, 1186 | 29 | 6 | - | 2, 1 | - | - | - |

- **What they asked of BN6's engine, and how it is given** ([Virus families moved into BN6's battle engine](../rom/virus-families.md),
  The recipe generator's reads of BN5, their shims and pre-steps, their
  sounds): a held chip is the chip registry's, BN6's own chip of the
  name or, for a chip BN6 lacks, the registry's id of its own (MetaGel
  450, Lark's and Whirly's; Blinder 456, Lark's and Skarab's; AntiWatr
  459, Draggin's; AntiElec 460, Lark's), written into the six lists of its home, which BN6's
  table of held chips by AI index names; a chip id the code compares past
  an immediate's 255 (Lark's) goes through a shim (`XvCmp`), and one the
  code gives its object by hand (Zomon's AreaGrab, `movs r0,#117` before
  `strh r0,[r5,#0x2A]`) is BN6's in the immediate. A held chip whose code
  is not moved yet still works as a held chip: BN6's engine picks it from
  the list as any virus's, the family's code compares it by its id and
  hands it to the chip writer (BN6's twin, audited: `held_bn6`), which
  writes its numbers from the registry's record, whose family is the
  inert 0x1C and subfamily 3 until the chips track moves its code; the
  virus's setAttack0 then runs BN6's family 0x1C (`0x080ED6DC`), whose
  subfamily 3 (`0x08010488`) spawns an effect in front of a virus
  (AttackVars +0x1B 0), plays sound 0x90 and ends the attack: the virus
  uses it with a puff and goes on with its states (no spec's battle gives
  one: in every spec only Zomon's object holds a chip, its own AreaGrab).
  A family's lookups by a range of NameIDs make their bounds in
  instructions, past an immediate's 255 for BN6's ids: a shim loads them
  (`XvRange`, 12 families). BN5's virus dispatcher steps Dominerd and
  Appley before their state each frame, BN6's for no family's id: a hook
  on BN6's dispatcher runs BN5's step, ported (`XV_PRESTEP`). Dominerd wraps
  a generic state in a routine of its own, which is copied once more for
  each of BN6's two status states (frozen and bubbled), its call aimed at
  BN6's (`XvClone`). The ids in a table of a family's
  data are byte fix-ups (`ID_TABLES`: Zomon's effects, Whirly's sprites,
  Marina's water's records, the fire column's kinds). A sound BN6 has no
  song of is copied once, at the slot issue #82's tracks share for it
  (Powie's 0xC0 at 0x62, Skarab's 0xB7 at 0x61, Flashy's and Eleogre's
  0xBA at 0x60), on the players BN5's own entry names, as the Navis'
  copies are, and noted with `xsong.c` so that the other tracks' installs
  find it there and the differential reads it by BN5's number. A sprite is BN6's own where BN6
  has the same bytes, else copied, decompressed, at the families' numbers
  (lists 1, 4 and 5 upward from BN6's end, list 1's placeholders 0x1E and
  0x00 first; list 2 from 0x30; a sprite of list 3, where it is not
  grown, in list 4), and an effect of other art takes an id from 0x6C. Every object type a family's
  code spawns is decided once for every track (`bn5_twins.TYPES`): moved
  through the dispatch unless BN6's type of the number runs the same code
  and draws the same sprites by their bytes (19 moved, 6 twins, the
  fourth kind's 0x83 unreachable: BN5's panel claims, which BN6's engine
  never asks for, go to the dispatch's stray). Flashy's and Eleogre's
  flash is one type, Flashy's copy run for both. BN5's image object (the
  first kind's 0x05, which WuNote spawns with kind 0x30) is the engine's
  objects' module, moved once for every track before any family (The
  engine's objects moved once, below): WuNote's call to its spawner goes
  to that copy and its type is run as that install's (`XV_TYPE_SHARED`);
  its kind 0x30 is fixed in the one table (`tools/bn5_objects.py`, its art
  BN5's 4:0x21 at the families' list 4 number 0x6C), and a family spawning
  a kind not fixed there is refused. A family's numbers its code keeps as they
  are (an alliance's bits, a collision region's spawn words) are read by
  hand in BN6 first (`KEPT`). BN6 finds a virus's arm points (where a
  status's mark stands over it) by its battle sprite's number in list 1,
  past BN6's records for every family: list 1's records are grown and
  each family's install writes its own, BN5's 30 points and BN6's four
  after them ([Virus families moved into BN6's battle engine](../rom/virus-families.md), their arm points; the chips track found
  CanladEX's marks 10 pixels to one side). The rare versions (4 and 5) reach BN6's
  rare path only behind its flag, as BN6's own do, and their deletes
  leave its byte alone (`tools/xfamily.py`). Their reward rows are BN5's,
  each chip as BN6's of its name (else 200 zenny, until BN5's chips are
  BN6's too).
- **Each fought in both engines**: seven specs a family (`tests/xdiff`:
  V1 alone where BN5's own records set it against MegaMan idle, V1
  against cangard_v1_keys' 62 inputs, and each other version alone), all
  with `rng=replay` and `"sprites": "bytes"`: all 175 the same over
  every fight frame, among them every one of CanGard's and Handi's as
  before, and Zomon's seven since its chip's family keeps BN5's timing
  (below). What the families showed the
  comparator of the engine's own, each in `tests/xdiff/legit.json` with
  both games' routines: the hit spark a family's attack strikes with and
  the effect a twin routine of the engine's spawns are each game's art
  (their kind, place, owner and id compared, their frames not), MegaMan's
  status effect is BN5's 2 pixels higher and drawn from each game's own
  confusion picture, BN6's Elec damage of a hit, a status's saved
  CurAction (+0x5D), and one routine of BN6's that fixes a bug of BN5's a
  family can see, BN5's own copied in its place (the collision data's
  update, Marina's). Their breaks (`--mutations`): for each of the 23 an
  immediate (the CurAction it gives setAttack0 or compares, left as
  BN5's) and a rebased call (its veneer to setAttack0 aimed at the
  families' empty hook), each failing its V1 spec. `tools/xfamily.py
  --spec families` holds each one's V1 to its record and states, its
  four statuses taken and let go with what it held, its confusion's and
  blindness's marks over it at BN5's arm point, and each of its six
  versions deleted with BN6's delete counts and the rare path's byte
  untouched.
- **Zomon** failed all seven from the frame after its first AreaGrab
  (V1 t391, EX t363, V1's keys t410, V2 t344, V2EX t325, V3 t307, V3EX
  t280), every byte the same before it, until issue #82's final engine
  round. Its set-up gives its object AreaGrab by hand, BN6's own chip of
  the name (the registry's 117 as 163), which BN6's own chip code runs:
  both games spawn the time freeze at the same frame, take MegaMan's
  front column and play its sounds at the next, but BN6's family of
  time-freeze chips (0x15, `0x080ED0DC`) ends its user's attack a frame
  later than BN5's (`0x080EC400`, which spawns and ends in one frame:
  `bl 0x0800F29E` at `0x080EC448`; BN6's marks its AttackVars +1 at
  `0x080ED12A`-`0x080ED12C` and ends at the next frame's `0x080ED130`),
  so Zomon's own states ran a frame behind. Now a moved family's call
  to the chip writer (BN5's `0x0800FE6A`) goes to the dispatch's
  `XV_CHIP_WRITER`, which calls BN6's (`0x080126E4`) and answers its
  support family as 0x65 (`XV_SUPPORT_FAMILY`), whose routine runs
  BN6's support routine twice in its first frame, the chip's and the
  attack's end (`XV_SUPPORT`): BN5's timing for a virus's chip, MegaMan's
  support chips BN6's family as before ([The chip registry and BN5's chips' code](../rom/chips-code.md), A virus's
  support chip in BN6's engine). BN6 plays the hurt sound of a lethal
  hit before its HP test (`0x0801BA3C`), BN5 none (`0x080185C4`): the
  engine's, a mask. All seven PASS; two mutations (the writer's call
  aimed at BN6's own, the family's entry BN6's routine) fail V1's.
  `tools/xfamily.py` gives its statuses outside the time freeze its
  AreaGrab leaves standing once its attack has ended (BN6's status step
  waits while time stands still, `BN6_TIME_STOP`): at battle frame 120,
  Zomon holding no object of its own there.

- **Its rewards**: its reward rows are its game's own for its versions,
  each entry as BN6's (a chip as BN6's of its name, else 200 zenny until
  BN5's chips are BN6's too), written into the rows of its ids in the
  grown copy, which the battle's results then lean as BN6's
  ([BN5's battles, records and rewards](../rom/bn5-battles.md), BN5's records in BN6's engine).
- **Not yet**: the families' place in formations and a run's battles (a
  game-design pass). The held chips whose code is BN5's alone (MetaGel,
  Blinder, AntiWatr, AntiElec) wait for the chips track to move it; until
  then a virus uses one with a puff. Zomon's AreaGrab is BN6's own chip,
  run with BN5's support family's timing. Every family is installed
  wherever BN5's ROM is read, the New 3DS too (Platforms, above), and
  BN5's records that name one are fought in BN6's engine (`build.py
  xrecords`). A state saved while a moved family is in battle needs
  the same families installed to load (they are, at every boot with the
  same ROMs and switches); one saved on a map is the same byte for byte
  with the room made and the families installed or not (a checkpoint's
  after two of BN6's battles, [Testing](../engine/emulation/testing.md)): the install
  stands in the way of no run continued with BN5's ROM taken away or put
  in.
