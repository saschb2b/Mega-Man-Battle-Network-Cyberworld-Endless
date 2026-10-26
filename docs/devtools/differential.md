---
type: Tool
title: "The differential: a moved family against its own game"
description: "`python3 build.py xdiff` (`tools/xdiff.py`) fights each `tests/xdiff` spec in BN5's engine and BN6's with the moved family, Navi, chip or Soul, comparing traces frame by frame to PASS or FAIL; `--mutations` tests the comparator."
tags: [devtools]
generated: { by: claude-code/unrecorded, at: 2026-10-25T00:29:32Z }
---

# The differential: a moved family against its own game

```bash
python3 build.py xdiff                       # every spec in tests/xdiff: PASS, FAIL or INCONCLUSIVE each
python3 build.py xdiff cangard_v1 --story    # one spec, the battle as each game fought it
python3 build.py xdiff cangard_v1_keys --survey   # every difference of every frame, counted, masked or not
python3 build.py xdiff --mutations           # the comparator's own test: each break must FAIL its spec
python3 build.py xdiff zomon_v1 -j 1         # one spec at a time (the default: the CPUs but two side by side)
python3 build.py xdiff megaman_plain --story # MegaMan the subject: his timeline in both games
python3 tools/xproxy.py --mutations          # the Souls' proxy form against HeatCross (The proxy form, below)
python3 build.py shot --data-dir /src/.build/xdiff/data \
  --xdiff "foes=4F@62;mm=22;hp=100;frames=1200;out=/src/.build/xdiff/runs/s1"   # the traces alone
```

Issue #82 moves BN5's virus families into BN6's battle engine by their own
code ([Transplanted families](../multirom/families.md): CanGard, then Handi, the
first that draws random numbers and moves, then every family of BN5's that
BN6 lacks, 25 in all). The differential proves a moved family behaves as
it does in BN5's own engine, frame by frame: seven specs a family (V1
alone against MegaMan idle, V1 against `cangard_v1_keys`' 62 inputs, and
each other version alone: `NAME_v1`, `NAME_v1_keys`, `NAME_ex` to
`NAME_v3ex`). Local
only: it needs both ROMs (CI runs the unit tests of its parts).

The specs run side by side, as many as the machine's CPUs but two
(`-j N`; mutations too): each is one battle in a container of its own and
its comparison in a process of its own, forked, sharing only the boot
states in `.build/xdiff/data`, which the first run writes where they are
missing (that run goes alone) and every run after only reads. Each spec's
lines print whole as it ends, so their order is the order they end in;
the summary line keeps the specs' order. A spec takes about 9 seconds
(its battle about 5 in mGBA, its comparison about 3 in Python): the 325
specs take about 55 minutes one at a time (10 seconds each, measured on one) and took 2 minutes 44 seconds on 16 CPUs,
with the same verdicts at the same frames (`-j 1` keeps one at a time).

`--xdiff SPEC` (`src/dev/xdiff.c`, its items parsed in `xdiff_spec.c`, a
tool run in the game's place, as `--pacing` is) fights one battle twice
and traces both:

- **The reference**: BN5's battle on the guest core, as BN5 has it
  (`guest_dev_battle`, guest.c, the guest core's one use since it left
  runs in issue #82's last round): BN5's random battles' record with the
  foes named (BN5's ids at their panels), MegaMan at the spec's HP,
  everything else BN5's own (its folder, buster, mood and deck
  compaction). Its core boots once on the main thread and keeps its
  state as `xdiff-bn5.state` in the differential's data folder.
- **The subject**: BN6's battle with the families installed, the same
  foes by BN6's ids (BN5's 0x4F-0x54 are 0x200-0x205), forced as the foes
  step forces one (`emu_battle_force`), from BN6's boot state, with only
  the encounters' hooks and the install's (none of the run's director).
- **Alignment**: each battle's timer (`BN5_BATTLE_TIMER`,
  `BN6_BATTLE_TIMER`), never the cores' frames (BN5's Custom screen
  opens about 56 frames later). Each core runs to its fight's first frame,
  then by the battle's clock: a frame of each in turn while their timers
  read the same, the one whose timer has moved on waiting while the
  other's still holds (a turn's change takes BN6 four frames more than
  BN5, a Soul's crossing into the proxy six fewer than BN5's unison), so
  each game traces a timer value with the other at it too and the
  replay finds the reference's draws of it made. Each Custom screen is
  passed by its keys in each game, from the frame its window first reads
  open (a screen is counted there: the phase passes the Custom screen's
  for a frame as a battle begins): the spec's for that screen of that game
  (`cust5=N:KEYS.FRAMES/...`, `cust6=`; `-` no key), or else START 4
  frames, nothing 8, A 4 (OK, no chip), the same in both. The fight's keys
  go by the timer read before the frame (`keys=T:BUTTONS:N`, held for
  timer values T to T+N-1).
- **MegaMan's setup**, the same in both where a spec asks: event flags
  set in each game before its battle (`flags5=`, BN5's through its
  eToolkit +0x44: Double Soul's 0000, a Soul's 0008 + k, Chaos Unison's
  0236; `flags6=` BN6's), each game's first folder (`folder5=`,
  `folder6=`: chip | code << 9, the list repeated to 30, each chip marked
  owned, no Regular chip, no TagChips), each game's battle deck in an
  order (`deck5=`, `deck6=`, the list repeated to 30, written as the
  battle's phase first reads the Custom screen's: each game makes and
  shuffles its deck as the first turn begins, so without it each hand
  is its own game's shuffle's), the Souls in BN6's engine with MegaMan
  holding them (`souls6=MASK`, bit k BN5's Soul 7 + k: src/emu/soulunite.h
  over the proxy form, [The proxy form](proxy-form.md)), his buster's Attack, Speed and
  Charge (`buster=A:S:C`, levels less one), and `safe=1`, his collision
  data's `BN6_OBJ_INVULNERABLE` set before every frame of the battle (the
  same bit in BN5: no hit takes him, so a long spec keeps its course).
  What a spec does not set stays each game's boot state's.
- **Film**: a spec's `"film": [FROM, TO, STEP, PREFIX]` saves each
  game's picture at the battle's timer values FROM to TO, STEP apart, as
  `PREFIX_bn5_tNNNN.bmp` and `PREFIX_bn6_tNNNN.bmp`: the two battles side by
  side by the clock the comparison keeps (the Darkloids' pull request's
  pictures).
- **Seeding**: both RNG words written in both cores after the fight's
  first frame (`seed=P:S`); seeding as the battle begins does not work
  (BN5's opening draws 84 more). With `rng=follow`, BN6's words are
  written from BN5's after each frame both traced: BN5's buster draws one
  more number a shot than BN6's, so without it everything that draws
  later (a hit's spark) would part. Where the clock holds a number of
  frames each game's own (a turn's change, a Soul's crossing), each game
  steps its words by the three draws a frame of its own (legit.json's
  rng, per_frame), and the first frame traced after shows them apart:
  a mask reads them as the engine's where every other draw since the
  frame before is the same in number in both and each word is the
  other's stepped by exactly the per-frame draws one game made more.
- **The hooks** (`src/dev/xdiff_events.c`): each core carries hooks of
  its own (src/emu/hook.c keeps a table per core; the guest's are the
  differential's alone, `guest_hook`), on each game's own routines
  (bn5.h, bn6.h; [Virus families moved into BN6's battle engine](../rom/virus-families.md), The differential's hooks): its four
  RNG draws, every spawn of the first, third and fourth kinds (their
  common routine, entered and left), PlaySoundEffect, an effect's
  spawner, setAttack, a panel reserved, the blink of an object flashing
  and a sprite set up (with the CRC32 of the sprite's bytes, decompressed
  as BN6 draws it, read where the core lists it: `src/dev/xdiff_sprite.c`).
  Each logs an event with the core's frame, the
  battle's timer before it, its caller (a spawn's: its spawner's), the
  family's own code that led to it (the innermost return address in a
  family's moved code: BN6's copy, BN5's original; the Navis' moved code
  only where a foe is a moved Navi, as their copies hold engine routines
  BN5 runs for MegaMan too, a Soul's companion) and the outermost
  caller (the stack read word by word up to the object runner's return,
  each word that is a return address: after a BL, or after `mov lr,pc;
  bx rN`), and the object running then (the runner's word) or the one
  it concerns.
- **The replay** (`rng=replay`): BN5's draws are kept as they are made,
  each keyed by the family's code that led to it (its BN5 address) or
  else by its caller; each of BN6's is keyed the same way, its caller in
  a BN6 routine read as the same place in the routine's BN5 twin
  (`twin=BN6:BN5:LEN[:WORD]`, the twins of legit.json's rng, each
  checked byte for byte but for BLs and literal loads as the spec is
  made), and the k-th draw of a key in a frame (by the timer before it)
  is given the word BN5's k-th of that key drew from, written into the
  RNG word before it reads it: the family's code draws BN5's numbers
  whatever the engine drew around it. A twin may name the word BN5's
  draws take where they are not BN6's (the camera's shake: BN6's from its
  second word, BN5's from its first), so the shake is the same too. A
  draw with no twin in the other game's frame is reported, never forced:
  BN5's buster's extra draw is the engine's own (legit.json's rng, only,
  both routines cited); any other is a difference, the family's where its
  code led to it. Without the replay Handi parts at its first draw that
  BN5's buster or shake moved (`rng=follow`: t305, none: t35).
- **The traces** (`OUT.bn5`, `OUT.bn6`, their format in xdiff.c, version
  7): a record each time the timer moves while the battle fights: the
  objects of the first, third and fourth kinds in play (each from its list
  node on, its AI's data and its collision data, and in BN6's the byte
  the second-level dispatch's side table holds for its slot), the 40 panels, the order
  the objects ran in, the songs the music players began, both RNG words,
  MegaMan's side's player block (his battle NaviStats, the word of what
  was used this battle, BN5's Souls or BN6's Crosses, and his side's
  pending form: [Virus families moved into BN6's battle engine](../rom/virus-families.md), MegaMan as the differential's subject;
  the Souls' state in BN6's engine, `BN6_SOULS`' 0x80 bytes, zero in
  BN5's; the Custom screen's element 0xB, UNITE, as each game's last
  screen left it), what BN6's engine keeps apart of the objects (the sea's
  submerge's 0x90 bytes, `RAM_SEA`: the submerged by their collision
  data's bit and their markers; zero in BN5's, which keeps it in the
  collision data, read there as BN6 keeps it by legit.json's `layouts`
  `sea`: under or not, its marker compared as an object; the Dark Soul's
  chip AI's lists, BN5's from its own `0x02034C20`, compared while BN5's
  Dark Soul is in play, its chips as BN6's ids by the chip registry and
  a side of none alike in both; the Navis' own state, Colonel's sides'
  bits, BN5's from BattleState +0x5C; the Dark Soul's AI data past
  0x100, its chip ids BN6's), and the hooks' events since the record
  before (the first record's from the battle's start; a battle's end
  after its fight is no record's, its events counted lost).
- **The field**: `field=XX` fights both on BN5's battlefield XX, BN6's as
  `src/emu/xfields.c` installs it ([BN6's battles](../rom/battle.md), Battlefields and
  their panels' types), `field=XX:YY` BN6's own field YY against it; the
  specs `field_poison`, `field_grass` and `field_ice` fight CanGard on
  BN5's poison, grass and ice, every panel's type and flags compared, and
  `field_lava*` and `field_sea*` on BN5's lava and sea as BN6's engine
  runs them (`src/emu/xpanels.c`; [One net: the design](../multirom/one-net.md)): each of the
  eight fields, MegaMan stepping onto lava, a Fire CanGard on lava and on
  sea, an Aqua hit on lava, their timers running out; `toadman_sea`,
  `toadman_sea_elec` and `tomahawkman_sea` the sea's submerge and hold
  (`src/emu/xsea.c`: ToadMan under the sea, a buster's shot passing over
  him and an Elec one reaching him, MegaMan and TomahawkMan held as they
  come to sea under the splash).
- **A change in both games**: `both=A:V[:N]` writes into both games'
  copies alike, BN5's at its own address and BN6's at the moved copy of
  it (a family's element given, to put the engine's handling of it under
  test: CanGard made Fire, its reticle made Aqua), `each=A5:A6:V:N` writes
  V (N bytes) into a table of the engine's both games hold, each at its
  own address (BN5's at A5, BN6's at A6: the buster's shot made Elec in
  `toadman_sea_elec`), and `mm_element=N`
  gives MegaMan element N as the fight begins (his object's and his
  collision data's). `dschips=C,...` gives MegaMan's side's chip AI list
  in BN5 (BN5's chip ids, hex, as its battle makes one: in order, none
  after them, their count, no sequences), and with it or `dslist=1`
  BN6's lists are written from BN5's as BN6's fight begins, their chips
  BN6's by the chip registry (`darksoul_cannon`, `darksoul_sword`: the
  Dark Soul's chip AI takes them).
- **Breaking it on purpose**: `poke=A:V[:N]` writes into BN6's copy after
  the install (an address of BN5's in a moved block is written to its
  copy; one in RAM once the game has booted, over what it copied there,
  such as IWRAM's routines), `call=F:T` aims each family's veneer to
  BN6's routine F at T, and `unitebreak=N` breaks UNITE's rule in BN6's
  engine (`SOULUNITE_BREAK_*`: a Soul's turns 4, its used bit not set, a
  Soul lit by the next kind, Beast Out not greyed while united, a Soul
  kept past a Cross) and `soulbreak=N` a Soul's look, abilities or
  passives (`SOULBREAK_*` in soulunite.h: the low nibble the look's,
  another Soul's sprite 1, palette 1 2, TomahawkSoul's landing still 4,
  the buster's flash in a Soul BN6's object 8;
  the next the abilities', the charge 10 frames longer 0x10, the next
  Soul's family 0x20, no element 0x40; from bit 8 the passives', a hand
  of nine 0x100, no +10 0x200, no passives as it unites 0x400, and
  ColonelSoul's, `SOULCOLONEL_BREAK_*` in soulcolonel.h: his button lit
  for any pick 0x800, no obstacle strike 0x1000, B leaving the button's
  chip in it 0x2000, the button's chip left in the deck 0x4000, his
  charged shot ScreenDivide with a chip 0x8000; from bit
  16 the chips', `SOULCHIPS_BREAK_*` in soulchips.h: no chip charged
  0x10000, a charged chip not doubled 0x20000, TomahawkSoul's Wood chip
  not doubled on grass 0x40000, KnightSoul's front column unguarded
  0x80000, ShadowSoul's charged sword BN6's family 0x100000, his shadow
  kept 0x200000; from bit 24 BN5's mood's, `SOULMOOD_BREAK_*` in
  soulmood.h: no gain from a landed hit 0x1000000, no DarkChip dealt
  into a worried hand 0x2000000, a BN5 DarkChip used leaving him as he
  was 0x4000000, UNITE standing while he worries 0x8000000, the dark gate
  left out 0x10000000).
- **Comparing to a frame**: `compare_to=T` compares the two battles frame
  by frame to timer value T alone, the spec's `expect` holding both to
  their end: for a battle whose last part BN6's engine cannot fight the
  same yet (a BN5 DarkChip's own attack, the chips track's), what comes
  after it held by expectations (BN5's mood 0 to the end).
  `compare_skip=[[A, B], ...]` leaves the frames A to B out (the run
  prints them), for a part each game runs with its own code that the
  spec does not test (`soul_colonel_strike`: the RockCube's own use,
  each game's chip, ten frames), the frames before and after compared;
  `colonel_strike` compares to its cube's own use and holds the strike
  that follows by its expectations in both (BN5's RockCube object is the
  code Colonel's moved copy runs, the reference's family's, where BN6's
  is MegaMan's own chip's).
- **Effects by their copies**: `effect_copies` (and every spec with
  `souls6`) reads a copy a track made of a BN5 effect BN6's engine has
  its own of (legit.json's maps, effect, copies: the Navis' 0xA0, 0xA2,
  0xA3 and 0xAB, the Souls' 0xC0 and 0xC1) as BN5's effect and BN5's as
  itself, so an effect the moved code spawns and one BN6's engine spawns
  of the same number read alike (`colonel_strike`: the strike's hit, and
  BN6's own 0x01 and 0x02 beside Colonel's copies). The chips' units a
  spec's code runs another track's copy of are the Souls' routes
  (`xd_log_routes`, where the Souls are installed or a Colonel is a foe:
  the strike's shot is the chips' copy), named in the trace as the
  spec's own.
- **Pictures**: `shots=[[G, N, F, PATH], ...]` (xdiff.c's `shot=`) saves
  game G's (5 or 6) Custom screen N at its frame F (from the frame its
  window first reads open) as a BMP, the canvas as the core draws it; a
  path under `/src/` is the worktree's (its folder made), so both games'
  screens can be laid side by side (ColonelSoul's button in BN5 and in
  BN6's engine).

`tools/xdiff.py` copies both ROMs into `.build/xdiff/rom`, builds the host
binary, runs each spec of `tests/xdiff` (`*.json`: the foes, MegaMan's
panel and HP, the frames, the seed, the keys, `rng`, `natives` with
their `native_attacks` and `native_effects`, `"sprites": "bytes"` (every
moved family's spec: a sprite no pair of the registry names compared by
its bytes, as the generator's twins are), and what BN5's battle is known
to do) and compares the two traces (`tools/xdiff_snap.py` the
snapshot and its differences, `tools/xdiff_masks.py` whose each is and
the masks):

- **By role, never by slot**: the first kind's objects by their enemy id,
  the third and fourth by their type (an effect object, the fourth kind's
  type 0, by its effect id in BN6's numbers too, so one spawned with the
  wrong id is a role the other game lacks), each by the order it appeared.
  An object is the family's where its spawn's call chain passed through
  the family's code, or the object that ran as it was spawned is the
  family's (the hooks' spawn events): so an extra object the subject's
  family code spawns fails, whatever its +0x4C names. The family's code
  is the spec's families' own blocks (those its foes are versions of, and
  a family's whose moved type one of them runs as shared), in both traces:
  BN5's code another family copied (a helper of BN5's shared virus code, an
  object type's) is BN5's engine there, as BN6 runs its twin; and an event
  is the family's only where the object it ran for is the family's or
  none's (BN5's temp attack object, which WuNote spawns, is MegaMan's
  buster's in BN5's own engine). In a spec whose foe is a moved Navi the
  Navis' code is the foe's and no family's is (the families' copies of
  BN5's engine code are its engine there); the engine's objects moved
  once (`XN_OBJECTS`) are the engine's in every spec.
- **In BN6's numbers** (`tests/xdiff/legit.json`, maps, each with why):
  the enemy ids, MegaMan's NameID (0x180 against 0x1A0), CurAction (BN5's
  0-5 BN6's 0-5, 6 BN6's 8, 7 on two up; MegaMan's ready 6 against 8), an
  object of a dispatch type by the number its side table names (BN6's
  0xD2 with 0x25 beside it read as 0x25, CanGard's reticle: a
  translation, not a mask; the spawn hook reads the dispatch's spawner's
  frame the same way), the muzzle's effect (0x57 against
  0x6C), the AI index (14 against 32), a sound by its song (the player it
  plays on is each song table's own; a copy of BN5's in one of BN6's
  empty slots by BN5's number, and in a Navi's spec BN6's own song of a
  copied sound's number told apart, bit 16). The engine's own numbers too, the
  way an enemy id is read: a collision data's alliance words (+0x30 and
  +0x34, set from a table by alliance and region, each game its own) as
  the BN6 entry at the BN5 word's place in BN5's table, and a hit's or a
  touch's word (+0x70, +0x6C: the other's +0x30 ORed in) as the OR of
  those entries, where every way of making it of the table's words gives
  the same (`alliance`); the renderer's tile place and palette slot of a
  family object as their rank among those the family's objects drawn that
  frame take, in the order they ran (`renderer`: the renderer counts them
  over everything drawn before, so what else is drawn is no part of the
  family's; drawn is the renderer's own test, the object and its sprite's
  state shown; one not drawn keeps the rank of its last place, one never
  drawn has none); an object's slot (+3) counted from
  its own (`slot`); an attack of the third kind that runs a virus's states
  (`action`, `t3_types`: Handi's bomb) its CurAction as a virus's; a
  pointer into a table both games hold whose end the 32 bytes would run
  past (`rom_tables`: Handi's bomb's kinds, the status effect's) by its
  place there, the entry the same; a chip id (`chip_id`: an object's
  chip, +0x2A, and its AttackVars +0x14, BN5's by the chip registry's
  rows), and a virus's CurAction that is its chip's family as the chip
  writer returned it, BN6's family of BN6's chip; a panel's type, the
  type it is drawn as and its flags' low nibble (`panel_type`: BN5's lava
  8 and sea 0xA BN6's 13 and 14, its holy 9 BN6's 5); a status's saved
  state word (+0x5C, its CurAction at +0x5D: `action`). The spec's
  families' own numbers come from `tests/xdiff/families.json`, which
  `tools/xrecipe.py` writes with the recipes: their ids and AI indexes,
  the effects their code spawns, their sounds copied or played by another
  number, the engine's routines their code keeps as callbacks and their
  twins (`code_twins`), the effect ids their code keeps in their AI data
  for the engine's teleport (`effect_bytes`: read as BN6's, and their word
  compared as bytes, never as a pointer), their attacks that run a
  virus's states (`t3_actions`) and the sprites they copied (`art`: BN6's
  list word, BN5's list and number). A word ORed of the alliance table's
  words keeps the hit pass's own bits as they are (bits 0x1, 0x40 and
  0x20000, none a table word's). A difference prints BN5's value as read
  so.
- **Pointers by what they name**: an object by its role and part (its
  node, its AI's data, its collision data), a panel, ROM that was moved by
  where it came from (BN5's into a family's code where it is: a callback
  kept in an AI's data), a pointer into each game's own ROM by the data
  there (32 bytes the same), a sprite's frames from its sprite's own data,
  a sprite itself by the registry's pair (`sprites`: MegaMan, his flashes,
  the sparks, the charge glow, a status's effect) or, in a spec with
  `"sprites": "bytes"`, by its bytes where both games' are in their ROMs
  or the one is a family's copy (`families.json`'s `art`): a type of
  BN6's drawing other art than BN5's is a difference.
- **Art by its bytes**: what an install chose to draw (an object of a
  moved type, a foe's own body, an effect of a family's or a foe's own
  ids, a companion overlay of a Navi's kind) is compared by the bytes of
  the sprite it last set up, never by its number: the two games number
  their sprites otherwise (a copy in BN6's grown lists, a twin at another
  number), and a twin that draws other art fails as `sprite bytes` at
  its sprite's word (+0x18 of its sprite state). The engine's and
  MegaMan's own draw each game's art and are not compared so.
- **Read in BN6's terms** (`tools/xdiff_events.py`, from BN5's own
  events): an object that has reserved a panel reads BN6's bit `0x20` of
  its +0 (BN6 sets it at the first reservation and never clears it while
  the object lives; BN5 has none); an object the blink ran on reads its
  visible bit (`0x02`) in BN6's phase (the two hide a flashing object on
  opposite frames of the same timer). Neither is a mask: each is the
  object's own record read as BN6 would have written it, and the family's
  records are compared through it like any other's. An object spawned
  while the hooks watched that has set up no sprite since holds the
  slot's last occupant's sprite state (the spawn clears the record only
  to there): its sprite state is compared from its set-up on, and one
  game's setting one up where the other's does not is a difference.
- **The family's calls by their call sites**: in each frame, the calls
  its code made (the events whose chain passed through it: spawns by
  kind and type, sounds, effects, setAttack, reservations, draws), each
  with its call site in BN5's numbers, in the order made, in BN6's
  numbers: the same in both, or the family's difference.
- **Each structure through its own layout** (legit.json, layouts): the
  collision data (0xA0 bytes in BN5, 0xA8 in BN6, laid out otherwise from
  +0x14) and the panels (0x24 and 0x20), each segment with the paired
  routines that show it. Where BN5 keeps in two places what BN6 keeps in
  one, BN5's parts are held to their rule (`bn5_or`: a hit's result by the
  hitter's side, +0x6C and +0x70, ORed into +0x68; its modifier, +0x18 and
  +0x19, into +0x0F), the word itself compared with BN6's. Any other byte
  outside the segments must be 0 in both (BN6's own +0x74-0x7B, the
  multiplier and elements of a hit, among them). The flag words are
  compared as bytes, never read as addresses. A panel's timers
  (`timers`): BN5 counts lava down at +0x10 and the sea at +0x14, BN6 one
  timer at +0x12; where one of BN5's is not idle it is compared with
  BN6's (the frame one runs out, BN5's 0 and BN6's 0x708, the same).
- **Every difference explained** (legit.json, masks): an engine difference
  that is not the family's code, with why and the evidence (both games'
  routines by address), or it fails the spec at the first frame it shows.
  A mask names its place and may pin it further: its values (`cells`), a
  bit with its direction (`direction`: BN6 has it, BN5 not), a condition
  (`when`), whose records (`whose`). No mask applies to the family's own
  records (its objects, what they own, its attacks and effects): there
  every difference that is left after the maps fails the spec. The tool
  prints the masks it applied, none of them on the family's.
- **Whose a difference is**, from both traces: the family's where the
  reference (BN5) says the object is the family's, where the two traces
  disagree, or where only the subject has it and calls it the engine's
  (the subject's numbers are the transplant's own: a wrong id must not be
  its alibi). An object owned by another (+0x4C) is its owner's, but the
  engine's spark over an actor, lava's as it cools under it, is the
  panels' (BN6's engine's port of BN5's lava, under test as a family is:
  its differences fail the spec, but for its sprite's, each game's own);
  the spark where the family's attack or body struck (spawned by the
  engine's spark spawner, which the hit's object_spawnCollisionEffect
  calls with the attack's hit effect) is the spark's, and an effect object
  whose id the engine's code gave (the effect spawner's caller a routine
  of the engine's BN6 runs as its twin, or a type of BN6's own the
  family's code spawns as its twin) the effect's: their id, place and
  owner the family's and failing the spec where they differ, their
  sprite each game's own art (legit.json's masks).
- **The stop rule**: where the first unexplained difference is MegaMan's
  own state (his object, or one he owns: his shots, his buster's flash,
  their sparks) or an effect of the engine's, the spec is INCONCLUSIVE from
  there, not failed: the battle is no longer the same battle for the
  family. A native enemy's own differences (a spec's `natives`, their
  attacks and effects: each game runs its own code for them) do not stop
  it: the family is compared on while its world (MegaMan, the RNG, the
  songs, the run order, the panels) stays the same, and the spec stops at
  the first difference there. After a stop the family's objects are still
  compared, and the tool says how far they stayed the same.
- **What BN5's battle is known to do** (a spec's `expect`): CanGard's
  CurActions at given frames, its reticle's appearances, path and damage,
  the sound as it fires, MegaMan's HP lost when, the virus hit only in its
  firing state, holding its guard while MegaMan is out of its row, a
  panel's type at a frame (`panels`, in BN6's numbers), the virus's HP at
  a frame (`foe_hp`); checked on both traces.
- **MegaMan as the subject** (`"subject": "megaman"`, the Souls track's
  S0): the spec proves MegaMan, not a family. His object, what he owns
  (his shots, his buster's flashes, their sparks, his chips' attacks) and
  the engine's effects are the subject: their first unexplained difference
  FAILS the spec, where a family's spec stops there INCONCLUSIVE (the stop
  rule flipped). He is compared in BN6's numbers: his CurAction (BN5's
  ready 6 BN6's 8; BN5's 0x1A, a Soul's unison and its end, BN6's 0x1C, a
  Cross's; the step 0x10, the buster 0x11 and a chip's 0x14 the same),
  his NameID (0x180 against 0x1A0), his loadout (his AI data's +4 to +8
  and +0x11, power attacks by each game's table: legit.json's
  `power_attack`), his sprite by a registry (`sprites`: pairs of each
  game's sprite lists' entries, MegaMan's own, his buster's flashes and
  charge glow, the sparks; his frames are each game's own, the animation
  his code picks, his palette and the sprite itself are compared), his
  form and pending form (`form`), and his side's player block byte for
  byte. BN5 raises the mood of the side that lands a hit (no counter),
  BN6 has no such gain: BN5's mood is read less what its gains added
  since it was last set outright (a translation from BN5's own events,
  not a mask: its gains, `BN5_MOOD_GAIN`, and its set and calm,
  `BN5_MOOD_SET` and `BN5_MOOD_CALM`, which BN6's twins write alike;
  before the set reset it, a calmed 0x80 read as 0x08 in
  `chaos_shadow_turn`, its recorded FAIL until the release candidate). BN5's plain charged shot fires at once where
  BN6's waits 5 frames ([Virus families moved into BN6's battle engine](../rom/virus-families.md), MegaMan as the differential's
  subject): an engine difference, so no spec fires one. `megaman_plain`
  (CanGard V1 against MegaMan at 2000 HP, his buster 1-1-1, a folder of
  Cannon in both, the first Custom screen's Cannon picked; buster shots,
  the Cannon while CanGard's guard is down, steps between the rows and
  columns) passes; its breaks (`--mutations`: his buster's first flash
  spawned as effect 7, the Cannon striking for 41) fail it, and the
  flash's break leaves `cangard_v1_keys`, a family's spec, INCONCLUSIVE
  at the same frame.
- **A chip as the subject** (the chips' differential, issue #82's C2 and
  C3; [The chips' code](../multirom/chips-code.md)): `chip_*.json` are MegaMan
  specs that use one chip. `folder5` and `folder6` give each game's
  folder (its entries chip | code << 9, repeated to 30; BN6's chips
  owned in its pack too, as its deal asks: a chip it does not own is
  dealt as its junk chip 389), `deck5` and `deck6` the battle's deck in
  that order (a Giga chip's spec: a folder of one Giga and 29 Cannon A,
  the Giga dealt first; a folder of Gigas alone never deals), and
  `hold: N` keeps the foes still: before each fight frame each one's
  paralysis timer is set to N (BN5's collision data +0x1E, BN6's +0x1C,
  each game's own status step counting it down) and, the first time, its
  struck word's paralysis bit (the same bit in both). The chip's objects
  are MegaMan's; `megaman_types` names those a spawner no hook sees makes
  (NrthWind's gusts, `T4:0x3A`). `"sprites": "bytes"` (every chip spec)
  compares an object's sprite of no registry pair by its bytes where both
  games' are in their ROMs (legit.json's `sprites` compares the pairs it
  names; another sprite is otherwise compared by its frames' offsets
  alone, which the same layout of another picture passes): BN6's code
  drawing another picture at the number BN5's draws is a difference. An
  effect object's (the fourth kind's 0) is its effect id's, compared as
  the id: each game's art for an effect id is the engine's (BN6 redrew
  some, effect 0x14's list 5 0xD among them). The
  chip's id is read in each game's
  numbers (legit.json's `chip_id`, from the registry's rows) and its
  family through the registry: a C1 chip's BN6 twin's, a moved chip's
  moved number (`src/core/xchips_moved.c`). A run that never reaches its
  fight says where each game stood ("never reached its fight: its game
  state, its battle's phase, Custom screen, frames"). A moved chip's
  copies are read as BN5's: a sprite BN6 draws from a unit's copy as the
  BN5 sprite it copies (`src/emu/xchips_bn5.c`'s `XvSprite` rows), an
  effect id as BN5's (its `XvId` rows, and the BN6 ids its immediates'
  and table rows' comments name), a song BN6 plays from a copy in one of
  its slots as BN5's number (`xsong_copy_of`), a muzzle flash the
  moved image object spawned by BN5's numbers, run by the dispatch
  (legit.json's `flash` pairs are BN6's own flashes'); an object's state word saved at +0x5C as a flinch
  or a status interrupts it has its action read as +0x09's. The chips'
  helpers' code (the buster arm, the shake, the computed powers) is no
  unit's: BN5's own buster runs it too, so its
  calls are compared as the engine's. Only the units of the chips BN6's
  folder plays are the family's code (`xd_log_units`): another unit's
  copy of BN5's code, which BN5's own code calls too (BoyBomb's explosion
  spawner, which a Handi's bomb calls in BN5), is BN5's own in BN5 and
  its twin in BN6, so it is no family's in either, and the trace names
  only those units' ranges (`xd_moved_kept`), and those of a unit whose
  copy of a type the spec's families run as theirs (`xd_log_shared`:
  Marina's water, MetFire's fire column, BomBoy's bomb, the chips' copies
  since the chips' code installs first), as a family's that moved a type
  another shares is; the hits' spark (the
  fourth kind's 4) the engine's hit area spawns is the engine's. A
  Program Advance or a DarkChip waiting for its rule is dealt straight
  into the deck (`deck5`, `deck6`: a code it has, else BN5's custom
  screen deals a blank). A spec whose
  target's own rule meets the chip (`hold: 0` where a flinching hit ends
  a paralysis in BN5's hit pass and not in BN6's: DrilArm's) says why in
  its `about`; one held to a recorded FAIL (`verdict`: Skully's
  confusion mark over a moved foe, the DarkChips' BN5 dark rule)
  has its `--survey` named there.
- **A Soul's life** (the Souls track's S2; [Transplanted Souls](../multirom/souls.md), Transplanted
  Souls): with the Souls installed in BN6's engine (`souls6=`), a Soul
  stands behind the proxy form, 8, where BN5's form byte names the Soul:
  his form is read in BN6's numbers (legit.json's `form`: BN5's 7-12 as
  8) and his NameID as BN6 writes it in that form (`name_id`'s
  `megaman_forms`: 0x1AB + the form, his role MegaMan's in any form). The
  Soul itself is compared every frame (`souls`): BN5's form byte and the
  places of its AI data where it keeps a Soul's state (its turns +0x0F,
  chaos +0x12, the chaos charge's cycle +0x1C and phase +0x1F,
  ColonelSoul's chip +0x32, the chaos level +0x6C: read 0 there as
  BN6's, whose AI data has its own fields at those places) against
  `BN6_SOULS`; BN5's used word against
  the Souls' block's, BN6's Crosses' used word none; the Custom screen's
  UNITE byte for byte. Nothing of a Soul is masked since S6 brought
  Chaos Unison's charge glow and its attack in the loadout (the chaos
  specs, `chaos_*`: each Soul's DarkChip attack at a hold of 45 frames,
  its failure at 60, and NumberSoul's holds from 75 to 210 besides; ShadowSoul's attack
  BN5's own DarkInvs, moved, which `chaos_shadow_45` holds frame for frame; `soul_mood_chaos`
  the way a run reaches it, a DarkChip kept while worried and given up once a landed chip
  calms him, as BN5 refuses UNITE while he worries). The expectations read `soul`, `chaos`, `soul turns`
  and `souls used` the same way in both games, and `unite` (UNITE's +7)
  and `unite stands` (its +0).
  `soul_knight` (KnightSoul: a deck of JustcOne J in both, the first
  Custom screen's chip given up, UNITE and OK; the Custom screens opened
  with L count its turns down; its end as the turn after its last starts,
  t1564 in both), `soul_replace` (KnightSoul, then ToadSoul over it on the
  second screen), `soul_grey` (a Cannon picked: UNITE grey, A on it
  refused) and `soul_chaos` (NumberSoul's Chaos Unison with DarkPlus: one
  turn) pass.
- **A Soul's look, abilities and passives** (the Souls track's S3-S5;
  [Transplanted Souls](../multirom/souls.md)): united, MegaMan is BN5's in both
  engines, compared every frame with nothing masked of it: his sprite's
  frames (both games draw him from BN5's sprite of the Soul, BN6's a copy
  in its grown list 0; `MegaMan's own sprite` masks his frames only while
  he is not united), his palette, his loadout in BN6's numbers
  (`power_attack`: BN5's ids as BN6's grown table's, 0x94 on), his
  element, the arm's flash (`flash`: BN5's own object, moved with its
  kinds table, its type read through the dispatch's side table, so its
  kind the same number in both), his charged shot's family (`action`'s `megaman`: BN5's 0x43,
  0x3A, 0x40, 0x50, 0x3D, 0x45 BN6's 0xC0-0xC5; `megaman_by_attack`:
  BN5's sword family 0x13 is BN6's 0xC6 where his AttackVars' sub-family
  is ShadowSoul's 2), its effects (`effect`: ColonelSoul's 0x48-0x4A as
  0xC0-0xC2, ShadowSoul's slash 0x17 as 0xC3, TomahawkSoul's swing 0x45
  as 0xC4), its sounds (`sound`'s `bn5_to_bn6`: BN5's 0x70, 0x117 and
  0x115 as BN6's slots 0x5F, 0x5E and 0x5A where BN6 played that slot;
  the trace finds a song through the core's song table), the objects it
  spawns and their hits (a status's timers, the collision layout's BN5
  +0x1E-+0x2B as BN6's +0x1C-+0x29; the CurAction a status saved, +0x5D,
  numbered as +0x09). The effects BN6 has of the same number but draws
  otherwise (BN5's 0x03, 0x14 and 0x02) are BN5's copies, 0xC5-0xC7,
  which `effect`'s `copies` reads back as BN5's numbers; a counter's two
  families (BN5's 0x2C and 0x2D, BN6's 0x47 and 0x48, started by BN5's
  `0x0800E338` and the Souls' Thumb in BN6) by `action`'s `megaman`; a
  chip MegaMan uses that the two folders name differently (BN5's Sword,
  BN6's of its name) as the folders pair them (`folder5[i]` with
  `folder6[i]`), its damage read as a share of each record's power, so
  BN5's and BN6's chips of one name compare by what each does.
  TomahawkSoul's landing shakes the camera in both,
  which draws from each game's own word (`rng`'s `shake`: counted as the
  per-frame draws are, as many in both). `soul_colonel`, `soul_shadow`,
  `soul_number`, `soul_tomahawk`, `soul_knight_shot` and `soul_toad`
  (each Soul united, B held for its charged shot against CanGard or
  Canlada), `soul_buster` (KnightSoul's plain buster: its arm flash
  BN5's object, kind 0x23 and the Soul's frame in both, drawn from BN5's
  sprite copied by its bytes, which `sprites`' `copies` names as BN5's)
  and `soul_number_plus` (NumberSoul's hand of ten on the second screen,
  a Cannon's 40 and its 10) pass; their breaks (`soulbreak`:
  another Soul's sprite, palette 1, TomahawkSoul's landing still, the
  charge 10 frames longer, the next Soul's family, no element, a hand of
  nine, no bonus, no passives as it unites, the buster's flash in a Soul
  BN6's object; a copied song's slot left empty) fail them. The chips'
  specs (the track's S5) pass the same way: `soul_knight_charge`
  (KnightSoul's JustcOne charged with A held, doubled), `soul_toad_charge`
  (ToadSoul's BblWrap), `soul_tomahawk_wood` (TomahawkSoul's Wood chip on
  grass, doubled, the panel plain after), `soul_tomahawk_heal` (BN6's
  grass healing him, a Wood actor), `soul_knight_front` (an attack chip
  from his front column: 50 frames unharmed) and `soul_shadow_left`
  (ShadowSoul's B+Left stance struck: the counter's first frame at once),
  their breaks failing them. `soul_knight_shot` stops at t278: the next frame the
  deleted CanGard's deletion effect is let go a frame apart in the two
  engines, BN6's own end of a family's virus. An enemy's deletion leaves
  each game's battle state's address in the effect it spawns (`battle
  state`, named alike).
- **BN5's mood and its DarkChip rule** (the Souls track's S7; docs/
  [Transplanted Souls](../multirom/souls.md)): BN5's mood as BN6's engine keeps it
  (the Souls' block's +0x0E) is compared with BN5's own (his battle
  NaviStats' +0x0E) wherever `souls6` installs the Souls (`soul`'s
  `mood`; the expectations' `soul mood`). The subject's BN6 has the chip
  registry installed as the game's own start has it (`xchips_install`), so
  a folder may name the registry's chips by their BN6 ids (BN5's DarkCirc
  R, 187, as 214). `soul_mood_dark` (KnightSoul held: CanGard's hits
  lower his mood to worried, the second screen deals DarkCirc into the
  hand's end in both games and both cursors stand on it, used: his mood 0
  to the end; `compare_to` its use, as BN6 runs the registry's DarkCirc
  by its twin's family), `soul_mood_unite` (worried, KnightSoul's Break
  chip picked last: UNITE does not stand in either), `soul_mood_gate`
  (BlakWing used while he is not dark: the chip of nothing in both) and
  `soul_mood_chaos` (DarkDril dealt worried and kept, a Cannon landed,
  KnightSoul's Chaos Unison on the calm screen after) pass; their breaks
  fail them. `tinhawk_megaman` holds MegaMan the subject against TinHawk:
  struck again the frame his flash ends, he blinks most of the battle in
  both games.
- **ColonelSoul's button, chip shot and strike** (the track's S5): his
  chip committed at OK (BN5's pending record's +6, 0xFFFF none; the
  Souls' block's +0x14, 0 none) and his chip for the turn (BN5's AI
  data +0x32; the block's +4) are compared as `soul`'s `next chip` and
  `chip` (the expectations' `soul next chip`, `soul chip`, in BN6's chip
  numbers by the folders' pairs). `soul_colonel_chip` (a Cannon picked
  on the second screen and loaded into the button, OK, B held: the chip
  shot fires the Cannon; the third screen's hand without it, spent),
  `soul_colonel_grey` (a RockCube picked: the button grey, ScreenDivide),
  `soul_colonel_undo` (the Cannon loaded, then B: back among the picks,
  held and used) and `soul_colonel_strike` (a RockCube on CanGard's
  area: BN5's soldier and its three shots) pass; their breaks fail them.
  `soul_shadow_sword` (ShadowSoul's charged sword, no enemy in reach) and
  `soul_shadow_slash` (CanGard in reach: his path to it, the slash, the
  afterimages) pass as well.
- **Reference specs** (`"kind": "reference"`): a battle BN6's engine
  cannot fight yet, fought in BN5 alone (`game=bn5`), MegaMan's timeline
  printed and held to the spec's `expect.megaman` (his action, form,
  pending form, turns, the used word, loadout, sprite, palette, mood and
  HP at given timer values, in BN5's numbers): REFERENCE, or FAIL where
  one does not hold. (The first, `soul_knight_ref`, BN5's own KnightSoul
  from UNITE to its end, became the two-game `soul_knight` with S2.)
- **Engine specs** (`"kind": "engine"`): what BN6's engine does that
  BN5's has no twin of, fought in BN6 alone (`game=bn6`), MegaMan's
  timeline held to the spec's `expect` in BN6's numbers (and `sounds`, a
  song each that must begin): PASS or FAIL. `soul_cross` (HeatCross chosen
  on CROSSSELECT while KnightSoul is united: HeatCross from the next turn,
  the Soul gone, its used bit kept) and `soul_beast` (Beast Out's flag
  set, KnightSoul united: on the next screen the emblem stands grey, A on
  it refused, MegaMan still in the proxy).

`--story` prints each game's battle, `--survey` every difference over
every frame by whose and where, `--no-run` compares the last traces,
`--poke` and `--call` break the subject for one run, and `--mutations`
runs `tests/xdiff/mutations.json`, the breaks a transplant bug would make
(CanGard's: an immediate, a state number and an offset left as BN5's, a
call aimed at nothing or at the wrong twin, a table's entry, a number in
its data, an effect id left as BN5's alone and beside a native, a
strike's effect id, a collision region; Handi's: an immediate, a call
rebased to its neighbour, its draws shifted onto the other word; the
dispatch's: a MOVED entry left the stray's, a TWIN entry made BN6's own
number, a spawn aimed at BN6's spawner as before the dispatch; each of
the 23 other families': an immediate left as BN5's and a call rebased;
the Navis': a state number left as BN5's (each Navi's), an effect left as
BN5's, an overlay spawned of BN6's kind, a battle sprite's number, a
sprite table's entry, a sprite left at BN5's number where BN6 lists the
same bytes at another, the moved image object's kind left at BN5's
sprite and its twin kind at another sprite, a sound left at BN6's
number, Colonel's side's bit not kept, the Dark Soul's record of a chip
holding BN6's byte for BN5's region or another approach, the Navis'
buster another family; the lava's
and the sea's: lava's flags, their hooks taken off, the panel lava's
cooling leaves, the random number its spark draws, the element hit's
test that lets Aqua cool it; the sea's submerge's: its hit gate letting
every attack through and stopping Elec, its marker not kept, its splash
another effect; MegaMan's: his buster's flash, a chip's
damage; the Souls': a Soul's turns 4, its used bit not set, UNITE lit
for a wrong kind, Beast Out lit while united, a Soul kept past a Cross,
each by `unitebreak`; another Soul's sprite, palette 1, TomahawkSoul's
landing still, a charge 10 frames longer, the next Soul's family, no
element, a hand of nine, no +10, no passives as it unites, the buster's
flash BN6's object, no chip charged in a Soul, a charged chip not
doubled, TomahawkSoul's Wood chip not doubled on grass, KnightSoul's
front column unguarded, TomahawkSoul of no element, no gain from a
landed hit, no DarkChip dealt into a worried hand, a BN5 DarkChip
leaving him as he was, UNITE standing while worried, the dark gate left
out, ShadowSoul's charged sword BN6's, ColonelSoul's button lit for any
pick, no obstacle strike, B leaving the button's chip in it, the
button's chip left in the deck, his charged shot ScreenDivide with a
chip, each by `soulbreak`; a copied song's slot left empty and a
counter's first frame left to the next, by `poke`; the chips' moved
code: a moved chip's family or subfamily in its record, a grown
family's or support routine's entry left at its stub, a number, an
object's type and a sound in a unit's code, these by `poke5`, a unit's
BN5 address written at its copy, as its BN5 addresses lie among BN6's
own attack families' code and a plain `poke` there is BN6's), each of
which must FAIL its spec.
[Transplanted families](../multirom/families.md), has what they showed.

The control (`tools/xcontrol.py`, BN6's own Mettaur moved) runs with the
replay too: `CYBERWORLD_XDRAWS=keep` keeps a run's battle draws in its
data folder (`xdraws.bin`), keyed as the replay keys them (the control's
code at BN6's own place and as its moved copy alike), and
`CYBERWORLD_XDRAWS=PATH` replays them from there; the native run keeps,
the two others replay each draw (none unmatched) and stay the same frame
for frame.
