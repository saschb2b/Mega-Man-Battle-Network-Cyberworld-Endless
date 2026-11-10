---
type: Tool
title: "The differential: a moved family against its own game"
description: "`python3 build.py xdiff` (`tools/xdiff.py`) fights each `tests/xdiff` spec in BN5's engine and BN6's with the moved family, Navi, chip or Soul, comparing traces frame by frame to PASS or FAIL; `--mutations` tests the comparator."
tags: [devtools]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-12T02:00:00Z }
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
build/host/cyberworld --headless --rom-dir DIR --dev god,onehit \
  --xdiff "game=bn4;foes=01@52;hp=60"        # BN4 Red Sun's own battle alone: won or lost, the HP left
```

BN4: with BN4 Red Sun's ROM beside BN6's, `game=bn4` fights BN4's own
battle alone on the guest core ([Guest battles](../multirom/guest-battles.md)),
`foes=` BN4's enemy ids (hex; Mettaur 01) at their panels, `mm=`, `hp=`
and `seed=` as above, and says how it ended: won or lost, MegaMan's HP at
the result and in his NaviStats back on the map (`src/dev/xdiff_bn4.c`). Its
keys are its own (each Custom screen START, then A; the buster while it
fights; A through the results); `--dev god` and `onehit` hold MegaMan's HP
and the enemies' at 1 in its battle; `film=` keeps its frames by the
guest's frame.

A BN4 chip's spec (unit 3 of [BN4's chips in a run](../multirom/bn4-chips.md)):
`folder4=` gives the guest's first folder (hex entries, chip | code << 9,
as `folder5`), written after its battle is asked, which sets its own, with
each chip's byte of the save's copy that BN4's gate reads against its key
(`BN4_CHIP_GATE`, `xd4_folder_put`); `cust4=` scripts its Custom screens
as `cust5` does BN5's. BN4's cursor slides to OK, so its keys are spaced:
`cust4=1:-.10/A.2/-.18/START.2/-.18/A.2` picks the first chip and ends
the screen. `bn4_chip_cannon_control` is that battle fought twice, the
same: MegaMan holds BN4's Cannon, fires it at fight frame 30 and deletes
the Mettaur (seen by `film=`, which in the trace's mode took BN6's core's
picture for a BN4 reference until this). `bn4_chip_cannon_broken` is
its check: the second run is given no folder and must part as the chip
is used (t32). It was added after the control had first been merged with
`tools/xdiff_bn4.py` passing neither `folder4` nor `cust4`: the control
then fought BN4's own folder twice and passed all the same. `karma4=` sets MegaMan's
karma in his NaviStats before the battle copies them (`BN4_NAVI_KARMA`):
BN4 plays a dark chip only for a dark MegaMan (`BN4_CHIP_ALIGN`,
[BN4's chips in a run](../multirom/bn4-chips.md), BN4's light and dark
chips), so `bn4_chip_hole_control` fights Hole with karma 0, twice the
same, and `bn4_chip_hole_light` is its check, the second battle at 600,
where the dud plays. A BN4 spec takes `film` as BN5's do.
A chip whose code is BN4's moved (a unit of `tools/xchiprecipe_bn4.py`,
its rows `XREG_MOVED`: `bn4_chip_panlsht1` to `3`) is compared the same
way, with what `legit.json`'s `bn4` section adds for it: his attack's
subfamily read as the unit's number in BN6's grown support routines,
BN6's second wheel's flag in the attack's element (by the record's icon,
as BN5's chips have it), its support object's chip and spawn leftovers by
its type, and each copied sound paired with its slot. A support chip runs
inside its time freeze, between two fight frames, so the traces meet at
what it leaves: the foe's HP, the panels, the frame's sounds. Give the
spec a foe the chip does not delete (`04@52`, BN4's MettaurSP, 140 HP):
a deletion parts at its own effect, each engine's art. A mutation of a
unit's code names BN4's address, which the runner pokes at its copy. The
trace keeps up to 128 songs begun between two fight frames (ShotStar's
freeze begins sixty-three). A chip that needs another on the field first
takes both in one hand: `deck6` gives BN6's deck its order, BN4's keys
pick its own shuffle's (`bn4_chip_repair`, after RockCube); and a chip
both games name is a control spec of its own first (`bn4_chip_rockcube`:
its object each engine's own, by `actor_objects`, the record's copies
and the cube's entries).

Against BN6 a chip's spec says `"subject": "megaman"`: a difference of
MegaMan's or the world's no rule explains fails it, as in the BN5
differential. `tools/xdiff_bn4.py` reads his numbers as BN6's: his
CurAction by the families both games pair (`PLAYER_FAMILY`), the chip's
id where his attack and a support chip's object keep it (`legit.json`'s
`bn4` `chip_id`: the registry's rows, then the pairs the spec's folders
name), and his attack's subfamily by the two chips' records. What BN6
alone has is allowed by name, each from both games' code: its count of
chips held, its attack's entry number, the frame its support run holds
him after the chip's routine (BN4 ends the attack in the routine's
frame), a support object's spawn's leftover r3. `bn4_chip_areagrab` is the
first to pass, BN4's AreaGrab against BN6's: after BN6's support run has
held MegaMan a frame his own record is read that frame later
(`megaman_clock`), until the world gives him a state (a hit lands at the
battle's frame in both) and he is read with the battle's offset again;
a panel's +4 is the side of the field's start and its +7 the side each
frame, as BN6's field update writes them. Its three mutations break
BN6's AreaGrab (one panel, two rows, another sound) and are caught. Two
specs record where a chip parts, each with its verdict and why:
`bn4_chip_cannon` (t33) and `bn4_chip_timebomb` (t32).

A support chip's object is read by one rule (`legit.json`'s `bn4`
`support_objects`, `tools/xdiff_bn4.py` `support_spawners`): every
support subfamily's routine is a spawner of one shape in both games,
reached by `bx r3`, so the object's +4, +0x2C, +0x30 and +0x3C hold the
attack's parameters, its power word, the chip and the spawner's own
address. The tool finds the type's spawner in each game's table and
walks the type's routines for a load of +0x3C at every run, and only
then reads BN4's copies as BN6's; a type it cannot show so is compared
as it is. For a chip both games name, which a run plays as BN6's, the
copies of its record are read as BN6's record's (`chip_id`'s `record`):
the two records hold other parameters for one chip. `bn4_chip_panlgrab`
and `bn4_chip_antinavi` are the controls (AntiNavi's trap is set and
never sprung there). `bn4_chip_lifeaura` records where the aura stands
(t34): the harness places BN4's barrier fields in BN6's layout
(`xdiff_bn4_trace.c`), and the kind's number, where MegaMan keeps the
aura's object, and that object, each game's own code, are open.

[The BN4 trace](#the-bn4-trace) below traces that battle and compares it
frame by frame: `game=bn4:trace` traces BN4's battle alone (`OUT.bn4`) and
`game=bn4:bn6` traces it against BN6's (`OUT.bn4` and `OUT.bn6`), and a
spec whose `"game"` is `"bn4"` runs through `tools/xdiff_bn4.py`.

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
  owned, no Regular chip, no TagChips; a BN6 chip past nine bits is
  written hex chip:code, `209:5` chip 521 in F, in `folder6=` and
  `deck6=` alike), each game's battle deck in an
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
  copy, to each copy where more than one family copies the block, as
  Mettaur and Melody both copy BN4's summoned appear; one in RAM once the
  game has booted, over what it copied there,
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
the masks; a BN4 spec's, `tools/xdiff_bn4.py`: The BN4 trace, below):

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
runs `tests/xdiff/mutations.json` (with specs named, their mutations
alone, by their numbers in the file), the breaks a transplant bug would make
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

## A program on each game's NaviCust

`ncp5=VARIANT:COL:ROW` and `ncp6=` (hex; `src/dev/xdiff_ncp.c`) place one
NaviCust program on each game's own board, alone, and run each game's own
compile before the battle is asked, so that what the program sets in the
NaviStats is what each game's own routine set, not a byte the spec wrote.
The variant is program x 4 + colour; its shape's cells go onto the board
from its record (BN5's 5 x 5 two left of and above the column and row,
BN6's 7 x 7 three), its entry into the placed list. BN6's compile is
`BN6_NAVICUST_RELOAD` by `game_call`, whose two hooks the differential's
core takes only for such a spec; BN5's is `BN5_NAVICUST_COMPILE` by
`guest_call` (`src/emu/guest.c`): a hook on its main loop's `bx r0` into
the mode's routine (`BN5_MAIN_CALL`) goes to the routine once, which
returns to the same instruction, where the loop's own r0 and lr are put
back. Each prints what its game made of it (`xdiff: BN5's NaviCust:
variant 0x98 at column 1, row 1, compiled by BN5: its extra turns 1`).

`soul_soult` is `soul_knight` with BN5's SoulT+1 so placed in both (BN5's
program 38, BN6's 48, [The NaviCust's addresses](../rom/navicust.md),
Program 48): KnightSoul lasts four turns in both where it lasts three, its
end a Custom screen later (t2084), every frame compared. Its three
mutations: the engine's routine adding none, adding two, and the grown
effects' table's entry naming a routine that sets nothing.

`beast_out` and `beast_soult` (kind `engine`: BN6's battle alone) hold a
Beast Out to its three turns, without the program and with it placed and
compiled (`ncp6`): its form 0x0B to fight frame 1563, MegaMan's own from
1564, in both. Their mutations are pokes, which an engine spec takes
since them (`tools/xdiff.py` engine_only passed none before, so a poke on
one was a mutation that could not be caught): the program's routine
aimed at the Beast Out counter (NaviStats +0x21), which gives a fourth
turn, and BN6's count-down held.

## Holds

Through a hold (the battle's timer at one value for many frames: a Navi
chip's, a turn's change) the two games run a frame each in turn, the
reference up to eight frames ahead (`src/dev/xdiff.c`, `fight`'s `LEAD`;
each side's frames at its timer counted from its seed, `still`), so the
reference's draws are made before the subject's replay asks for them. In
lockstep a frame the reference spends without its game's loop left the
subject a frame ahead for the rest of the hold: BN5 queues ShadeMan's and
BlizzardMan's LZ77 sprites as their Navi chips come and loses three
frames, so 609 of BN6's 936 per-frame draws in `chip_shademan`'s hold
went unmatched though their numbers were BN5's, a FAIL of the
comparator's own. With the lead the suite PASSes as before (every spec
of `tests/xdiff`, the four held verdicts held, the mutations caught; [The
chips' code](../multirom/chips-code.md), the Darkloids' first batch).
## A spawn's leftovers (BN5)

A word of a moved object that its spawn took from a register its caller
never set is each engine's own: legit.json's layouts' `spawn_leftovers`
names it by kind, type and word, with its evidence read in BN5's ROM
(the object's code and the engine's it calls, none reading the word).
A rule's `bn6` reads BN5's as an AI data's start and BN6's as that number
(BlizzardMan's 0x2F's Z). Where the spawner or its caller then writes
part of the word itself, the rule names the bytes it `compares` and lets
the rest be each engine's (`tools/xdiff_snap.py` `obj_cells`): CosmoMan's
third kind's 0x79's +4, a count's byte over each engine's r4 (`[0]`), his
0x6C's and 0x6D's Z, its upper half 0x10 over each engine's r3 (`[2, 3]`).
The engine's spark where an attack struck (the fourth kind's 0x04) takes
its place from that attack, so its Z is read by the struck attack's rule
(`struck_leftovers`) and by no rule of its own. Each rule's compared bytes
have a mutation that breaks them (`cosmoman_v1`'s height and count:
[Transplanted Navis](../multirom/navis.md), CosmoMan).

## The trace's moved ranges

Every range the installs move (a family's, a Navi's, a chip's, a Soul's
code, BN4's bracket, BN6's grown tables) reaches the traces, which read
them as code: `src/emu/xvirus_moved.c` keeps them on the heap, as many as
there are, and only where a reader asked before the installs (`--xdiff`,
`CYBERWORLD_BTRACE`, `CYBERWORLD_XDRAWS`), so a player's run keeps none.
A reader gets the whole list or none: a list never kept, or a range the
heap could not hold, refuses the battle (`xdiff: a battle could not be
begun`) or the trace, said on stderr; a log with no room for a block of
the spec's code (`FAMILY_MAX`, 4096) is refused the same way. The run
prints the count (`xdiff: N ranges moved`: 1054 in `melody4_v1`'s run,
8 November 2026). The list it replaced held 1024: once the BN4 Navis and
families pushed the count past it, the last ranges installed (in that run
30: most of the Souls' code, BN4's bracket screen and BN6's chip records
moved) dropped out of the traces with only a line on stderr, which
`--quiet` hides. `test_xvirus_moved` in `tests/test_core.c` holds 3000.

A BN5 address is read as moved code by the rooms the comparator knows
(`tools/xdiff_snap.py`: `XV_CODE`, `XN_ROOM`, `XC_CODE`, `XN_BN4_ROOM`,
and Team ProtoMan's `XN_TEAM_ROOM`, `FREE_TEAM_NAVI_CODE` to
`FREE_TEAM_NAVI_ART`): a word of BN5's naming its own copied code (the
fourth kind's 0x12's +0x34, NapalmMan's data) is read as BN6's word naming
the copy only where the copy's room is one of them.

A Navi's copy of a type both engines also run of their own (the third
kind's 0x00, NapalmMan's flames and MegaMan's buster's hit alike) is keyed
as his code in BN5's trace by its BN5 addresses, so the hit's draws and
spawns there would read as his where BN6's own type 0 runs them. An
object no family's code spawned (its spawn's family none, the first kind's
aside) runs no Navi's code as the family's (`src/dev/xdiff_events.c`,
`megamans_objects`, its draws too): both games then key those events by
their callers, through the twins (`napalmman_v1_keys` at t217).

## The BN4 trace

```bash
python3 build.py xdiff bn4_mettaur_control bn4_mettaur_broken   # BN4's battle twice, the same; then broken, parting
build/host/cyberworld --headless --rom-dir DIR --xdiff \
  "game=bn4:bn6;foes=01@52;hp=200;frames=600;out=PATH"   # BN4's Mettaur against BN6's own, both traced
```

BN4 Red Sun's viruses come next (BN4's Billy first,
[BN4 Red Sun](../multirom/bn4.md)); the differential's reference for them is
BN4's own battle on the guest core, traced as BN5's is (`src/dev/xdiff.c`'s
format, its game 4: the trace's header names it):

- **Its units**: each part read from BN4's layout ([BN4 Red Sun](../rom/bn4.md),
  The battle in RAM, The battle's objects, BN4 on the guest core) and
  written in BN6's (`src/dev/xdiff_bn4_trace.c`): an object of the first
  or third kind with its element (BN4's +0x17) at BN6's +0x0E and its
  enemy id (a byte at BN4's +0x0F) at BN6's NameID; its collision data's
  side, panel, statuses' timers, flash, invulnerability, final damage
  and two flag words at BN6's offsets; its AI data's attack variables
  (BN4's +0x70) at BN6's +0xA0, and MegaMan's fields paired by code
  (below, MegaMan's AI data and his object); each panel's type, side,
  flags and reserver at BN6's. BN4's other bytes keep their own offset
  where BN6's is free there and else fill the places left in order (an
  AI data's from BN6's +0xF0, past its attack variables, which BN6's AI
  take never lays out), so nothing of BN4's is dropped and two BN4
  traces part where their battles do. The
  collision data's and panels' sizes in its header are BN6's; what BN6's
  engine keeps apart (the sea, the Dark Soul's lists, the Navis' state)
  is zero; the player block is BN4's battle NaviStats (0x40 a side), its
  used word, pending form, Souls and UNITE zero.
- **Its timing**: BN4's battle timer (`BN4_BATTLE_TIMER`, BattleState
  +0x40 as BN5's: 0 through BATTLE START!, held on the Custom screen) is
  the clock, its main mode read as BN6's sub-mode (`xd4_sub`), its Custom
  screens passed from the frame its window (`BN4_CUSTOM_WINDOW`) first
  reads open by START 2 frames and A 2 frames 18 later, again every 40
  frames (BN4 takes A only after its cursor's slide to OK: BN5's 4-8-4
  left its first screen open), the spec's `keys=` by the timer, and both
  RNG words written after the fight's first frame, as BN5's specs seed
  theirs. `safe=1` and `hold=` take BN4's collision data's offsets;
  `mm_element=` and `deck6=` do not yet (BN4's collision element and
  deck unmapped).
- **Its events** (`src/dev/xdiff_events.c` on the guest core, BN4's
  routines given by `xdiff_bn4_trace.c`'s `xd4_routines`: [BN4 Red Sun](../rom/bn4.md),
  The differential's hooks): its four RNG draws, every spawn (its
  SpawnBattleObjectCommon, entered and left), PlaySoundEffect, the effect
  spawner, a panel reserved, the blink and a sprite set up (its CRC32 from
  BN4's own sprite lists), each BN6's twin by its body; its setAttack is
  not hooked (BN4's viruses keep no AI data). Each event's caller, family
  and outermost caller are read as BN5's are, BN4's moved family's code by
  its BN4 addresses (a spec's BN4 families, `xd4_families`), so `rng=replay`
  keys BN6's draws by BN4's places as it keys them by BN5's. Every BN4 run
  also watches `0x020015E1` (`xd4_watch`), which Billy's recipe takes for
  0: it read 0 at every frame of `billy_v1`'s battle.
- **Its numbers as BN6's** (`tools/xdiff_bn4.py`, against BN6's only): a
  moved family's enemy ids from its recipe (`src/emu/xvirus_bn4.c`'s
  `.names` and `.first_id`: Billy's 13-18 BN6's 0x2C0-0x2C5) and its
  object type (its `.src_ai`, BN4's own type, BN6's virus type 0); the
  CurActions, BN4's generic 0-5 BN6's 0-5 and its own from 6 BN6's from 8
  (the recipe's state table and immediates; MegaMan's ready 6 BN6's 8, as
  BN5's); a pointer into a moved family's sprite by its offset in it
  (BN4's own sprite, the recipe's copy in BN6's free space). A sprite
  BN4 keeps LZ77'd (its list word's bit 31, the recipe's `(LZ77 in BN4)`:
  Spikey's, Shrimpy's, Walla's, Elmperor's and CirKill's battle sprites)
  is drawn from RAM: the battle's preload (`0x08002AEC`, from the setup's
  `0x08006C9A`) decompresses each foe's into its sprite cache
  (`BN4_SPRITE_CACHE`, `0x0200F720`: eight keys, list offset << 8 \|
  number, and their sprites, each the buffer + 4, past the LZ77 data's
  size word), and `sprite_load` takes it from there (its look-up
  `0x08002ACA`), as BN6's battle cache serves BN6's compressed ones. The
  trace carries the cache's 0x38 bytes each frame in the block BN6 keeps
  apart, zeros else for BN4 (`src/dev/xdiff.c`, `apart4_out`), and
  `bn4_cache` reads it: the pointer is read by its offset from the
  cache's sprite for the row's key, its room the LZ77 header's size less
  that word; a key not in the cache maps nothing. The recipe's copy is
  the same bytes, its size word dropped too (`tools/xrecipe.py`'s
  `sprite`), so Spikey's +0xA8 reads ('sprite', 2, 4) in both. Its
  mutation (tests/xdiff/mutations.json, on `spikey_v1`) aims BN6's list
  1's 0xC1 at Shrimpy's copy, its +0xA8 then BN6's ('sprite', 5, 4):
  `spikey_v1` fails at t1 without it, so the line it adds is the check.
  The five's shared opening, settled since, each from both games' code:
  - **The setup's draws** (`xd_replay_setup`, `src/dev/xdiff_events.c`; a
    BN4 spec's alone): a family draws before the fight is seeded, in the
    battle's paused opening, where BN6's replay gave nothing (its draws
    keyed only from the seed on): Spikey's init (`BN4_SPIKEY_INIT`,
    `0x080BA428`) stores its moves before an attack, 4 plus a draw & 3
    (`BN4_SPIKEY_MOVES`, `0x080BA5BC`, from `0x080BA46C`), BN4's 4 against
    BN6's 7, and Shrimpy's init its first way at random (`0x080BA11A`: its
    row and panels apart). BN4's draws of the family's code before its seed
    are kept as its setup's, in order, and BN6's k-th setup draw of a key
    is given BN4's k-th of that key, whatever the frame (each game's opening
    its own length; the battle's offset is of the fight's frames), one with
    none drawn as it is and said so. Spikey's one and Shrimpy's one each
    replayed: +0x88 and the row alike. Its mutation (tests/xdiff/mutations.json,
    `spikey_v1`): the init's `adds r0,#4` made 5 in BN6's copy, caught at t1.
  - **The opening's idle animation, ported** (`XVirusFamily.present`,
    `src/emu/xvirus_shim.h`'s paused present; [BN4 Red Sun](../rom/bn4.md), A
    virus's after-state ported through BN6's paused opening): both games
    update a virus's sprite in its after-state routine, twins but for one
    test: BN4's (`BN4_VIRUS_PRESENT`, `0x08010736`, at the tail of each
    family's own type routine, `0x080BA410` in Spikey's) runs the sprite
    update (`BN4_SPRITE_UPDATE`, `0x0800285C`) whenever the object runs;
    BN6's (`BN6_VIRUS_PRESENT`, `0x08016E64`, at its virus type's tail,
    `BN6_VIRUS_TYPE` `0x0810A284`, every virus's) skips it while the battle
    is paused (`0x08016E84`-`0x08016E90`: the BattleSettings' mode,
    `BN6_SETTINGS_MODE_READ`, not 1, then the pause test, `BN6_PAUSE_TEST`).
    The object runner runs a paused object while its +0 has 0x04, so before
    the port BN4's virus animated from its spawn through the opening and
    BN6's not once before its first fight frame. The port, in the recipe and
    never in BN6's routine: `tools/xrecipe_bn4.py` (`after_of`) copies BN4's
    after-state with each family (`tools/bn4_twins.py` COPIED, its calls
    BN6's twins), and the installer (`src/emu/xvirus.c` `paused_put`) puts a
    paused present in the family's state 0 (its appear: BN6's state step,
    `0x0801B1C4`, runs a CurAction 0 while paused and no other) and its
    hook 5 (spawn done): the state or the hook, then, where the mode is not
    1 and the battle is paused, the copy. A probe of hooks on both games
    (temporary, not kept) in `spikey_v1` counts what is left: BN4's
    after-state ran 53 times before its first fight frame (its generic state
    0, `0x0800FCAC`, 52 times, the first in its init's frame, its init
    calling its update; its state 1 once, `0x0801013A`, which clears the
    0x04, so none after), BN6's copy 51 (its spawn's, then each of the 50
    runs of BN6's twin, `0x08016380`, whose appear waits on `0x0800139A`
    then blinks 2 x 16 frames; none in the 154 paused frames after its
    appear) and BN6's state 1 at its first fight frame (the battle's
    offset). So BN4's virus ran its appear, and the sprite update after it,
    once more than BN6's: at t1 BN4's Spikey was on its idle animation's
    frame 2 with 5 left and BN6's on the same record with 6, one update
    behind where it was 152 behind (frame 0 at the first record), Shrimpy's
    and Elmperor's alike, and `shrimpy_v3` failed at t65, where Shrimpy's
    hook (`0x080BA13E`: its animation 1 cleared where the animation in play
    has ended, `0x08002E56` bit 0x80) read its idle animation's last frame
    in BN4 a frame before BN6 (its +0x10, +0x11 and +0x90). That update is
    the engines' openings', not the after-state's nor the appear's code
    (the next item).
  - **The opening, a frame longer in BN6's** (the owner's call 7, revised:
    [Battle Network 4 in a run](../multirom/bn4.md), The owner's calls;
    `legit.json`'s `bn4` `align`, "BN6's battle opening a frame longer";
    `tools/xdiff_bn4.py` `alignments`). Both appears run the same steps:
    BN4's generic state 0 (`BN4_STATE_GENERIC_0`) takes its plain appear
    (`BN4_APPEAR`, `0x0800FE00`) in a plain battle, BN6's twin's steps
    (`BN6_APPEAR`, `0x08016396`: into the appear queue, wait for the battle
    state's appear bit 0 and the queue's head, a 2 x 16 frame blink, wait
    for bit 1) but for BN4's battle kind 0x44 and its sound ([BN4
    Red Sun](../rom/bn4.md), A virus's appear and the engine's appear bits).
    Each opening sets bit 0 from the same step, once its fade ends (BN4's
    `bl BN4_APPEAR_BITS_SET` at `0x080E1F0E`, BN6's `bl BN6_APPEAR_BITS_SET`
    at `0x080E1A0E`), and a probe of hooks on both (temporary, not kept) in
    `spikey_v1` and `shrimpy_v3` saw it 18 frames after the virus's init in
    BN4 (its step 0 run 19 times from its init's frame) and 17 in BN6 (17
    runs from the frame after), the blinks the same 32 frames. So the
    harness runs BN6's opening a frame longer in every BN4 spec against
    BN6's (`game=bn4:bn6`), as it runs BN6's battle a fight frame behind
    (`lag`): BN6's first blink step (`BN6_APPEAR_BLINK_FIRST`,
    `0x080163FC`, `movs r0,#2` into +0x20) counts 3. `alignments` reads the
    place by its name from `src/emu/bn6*.h`, checks that BN6's ROM holds
    `was` (0x2002) there, and writes its immediate plus `frames` (1) into
    BN6's copy as the spec's first `poke=` (the run prints
    `xdiff: poke 080163fc (080163fc) = 2003`); BN4's side, the controls'
    two BN4 runs and every BN5 spec are left alone, and the game too (a
    BN4 visitor keeps BN6's appear and opening). The frame is one more run
    of the appear where a bit 0 a frame later would put it: the virus runs
    its appear and the paused present after it as often as BN4's, the
    queue empties and bit 1 is set a frame later, and MegaMan's appear (his
    side, the battle state's +0xD against his +0x16 at
    `0x080163C0`-`0x080163D4`, skips the queue and the blink and waits for
    bit 1) ends with it; the fight's frames count as before. The blink and
    not the bit, because it is one immediate of BN6's own appear, where a
    bit a frame later needs a count the opening's step has not (it sets
    the bit the frame the fade ends). Its limit: the queue blinks its
    objects one after another, so a longer first blink step would delay a
    spec's k-th foe k frames where BN4's bit delays each one a frame; a
    spec of more than one foe is refused (every BN4 spec has one) until the
    bit itself is delayed. With it every BN4 spec PASSES and nothing in
    the opening needs allowing: the allowance it replaces ("the opening's
    idle animation, BN4's appear a run longer", which held a family's
    sprite place to BN4's one update ahead until an animation was set:
    `opening_animation`, `OPENING_LEAD`, `anim_steps`, `bn4_sprite_data`)
    is gone with its code. Its mutation (`shrimpy_v3`, "BN6's opening two
    frames longer": the step made `movs r0,#4`, written after the
    alignment's) is caught at t1 (+0x91 BN4 5 | BN6 4), and without the
    alignment (`--poke 080163FC:2002:2`) `spikey_v1` and `shrimpy_v3` fail
    at t1 (+0x91 BN4 5 | BN6 6). The two the allowance's own checks caught
    (`spikey_v1`): BN6's copy's first idle frame given the next frame's
    mini-animation offset, caught at t12 on its +0xB0 (BN4 11112 | BN6
    11124, the first frame compared on that record); the port's own,
    Spikey's copy's sprite update aimed at BN6's family nullsub
    (`call=080026C4:0810A952`), its place 51 updates behind again, caught
    at t1 on its +0xAC and +0xB0 (all 181 caught).
  - **A diagonal step's direction** (`tools/xdiff_bn4.py`
    `direction_as_bn6`): BN6's rule (`0x0800E994`) gives 5 for a move of two
    or more toward column or row 7 or a diagonal one, BN4's (`0x0800AF90`)
    never; both collision panels' updates (`0x08012D9A` / `0x0801A04C`)
    store the new panel over the old as they write it, so BN4's direction is
    read as BN6's from its panel at the record before and at this one (where
    BN4's own rule gives BN4's value from them), kept while its value and
    panel stay: Spikey's first step, (5,2) to (4,1) at t4, BN4's 3 and
    BN6's 5. Its mutation (`spikey_v1`): Spikey's collision panels' update
    aimed at BN6's family nullsub, caught at t4.
  - **A pointer into BN4's moved code** (`tools/xdiff_bn4.py` `one`): BN4's
    trace's pointers are named by the BN6 trace's moved blocks, as BN5's
    are (`xdiff_snap.Frame.ptr`'s `rom`), so CirKill's shot's record (+0x4C,
    its kind × 16 from the table at the literal `BN4_SHOT_KINDS_LIT`,
    `0x080CE83C`) reads ('rom', `0x080CE650`) in both, and a flags word of
    0x08000000 alike (Spikey's and Shrimpy's third kind's 0x03). Its
    mutation (`cirkill_v1`): the copy's literal left at BN4's address,
    BN6's shot reading BN6's own ROM, its +0x4C ('raw', `0x080CE650`)
    where both had read raw alike before, caught at t1.
  - **Where the fifteen stand**: `spikey_v1`-`v3` and `shrimpy_v1`, `v2`
    INCONCLUSIVE, the family's own records the same in all 1200 frames,
    the world parting where MegaMan is struck (his +0 0x13 against BN6's
    0x15: t277, t207, t170, t790, t222); `shrimpy_v3` FAIL at t65 (above: PASS since, BN6's opening a frame longer);
    `walla_v1`-`v3` FAIL at t72, t68 and t66 and `cirkill_v1`-`v3` at t32,
    t26 and t23 on the hit's spark (the fourth kind's 0x04, +0 and +8), each
    after MegaMan struck (since PASS: CirKill's direction, below); `elmperor_v1`-`v3` FAIL at t1 on its collision data
    +6 and +8 alone (BN4's 0xFF and 0, BN6's 0 and 0xFF). The
  collision data's layout from code: the collision setup (BN4
  `0x080129AC`, BN6's twin `0x08019FB4`) and the pool's maker
  (`0x08012470`, `0x0801986C`) paired store by store, BN4's +0x05, +0x0C,
  +0x1E, +0x58-+0x5F, +0x60 and +0x78 at BN6's +0x07, +0x0E, +0x2E,
  +0x30-+0x37, +0x38 and +0x44 (bn4.h, `BN4_COLL_*`); then its +0x0A and
  +0x0B at BN6's +0x0C and +0x0D (`billy_v1`, below). The alliance words,
  a move's direction, the slot, the reservation bit and the moved
  effects' ids are read as BN6's (`billy_v1`, below); sounds are BN4's
  own still.
- **The stop rule against BN6's**: the family's own records (its
  enemies, and what its code spawned by the spawn events) fail the spec
  at their first unexplained difference; one in the world's alone
  (MegaMan, the engine, the RNG, the songs, the panels) makes it
  INCONCLUSIVE from there, the family compared on, as BN5's specs stop.
- **The comparison** (`tools/xdiff_bn4.py`): the objects matched by role
  as BN5's are (BN4's MegaMan, enemy id 0, read as BN6's 0x1A0; BN4's
  other numbers its own, no map to BN6's yet), their bytes, AI data and
  collision data compared as BN6 lays them out, a pointer by what it
  names (an object by its role and part, a panel; a ROM pointer as it
  is), the panels, both RNG words, the songs begun and the run order each
  frame; the first frame with a difference no mask explains fails the
  spec. The masks are the fields BN4 has none of (`BN4_LACKS`, each with
  why), applied only where BN4's trace holds nothing there and the other
  is BN6's: a virus's AI data and its pointer, the AI index, the freeze's
  and the bubble's timers.
- **The control** (`"kind": "control"`): `bn4_mettaur_control` fights
  BN4's Mettaur at (5,2) against MegaMan at (2,2) with 200 HP, 35 buster
  shots and five steps by the timer, twice, each run in a process of its
  own: 900 fight frames, the same in every byte (PASS); MegaMan steps
  through four panels, his shots and the Mettaur's waves come and go, the
  Mettaur loses 20 HP and MegaMan 30, and the RNG's first word draws. Its
  broken twin, `bn4_mettaur_broken` (`"again"`: its second run steps up
  at t201, not t200), must part and does, at t202, where MegaMan's AI
  data holds the pressed UP in one and not the other (PASS). Two BN5
  specs, `cangard_v1` and `handi_v1`, PASS as before, their traces the
  same byte for byte.
- **The BN4 trace for a Navi** (`bn4_gutsman_control`): BN4's GutsMan V1
  (BN4's 0x84, a Soul Navi) alone at (5,2) against MegaMan idle at (2,2)
  with 2000 HP, fought twice: 1200 fight frames the same in every byte
  (PASS), his objects from t265 and his machine gun's shots every 11
  frames in them. A Navi's object carries an AI data of its own (his
  side's, BN4's 0xC0 bytes), which the trace lays out as MegaMan's: a Soul
  Navi's own block (BN4's +0x64, 12 bytes, `BN4_AI_NAVI`) at BN6's +0x80
  (`BN6_AI_NAVI_STATE`), where his port keeps it, the attack's variables
  (+0x70) at BN6's +0xA0, the bytes no part names from BN6's spare (+0xF0)
  on; MegaMan's AI data shares the layout, and every BN4 family spec
  PASSes with it. `tools/xdiff_bn4.py` reads the Navis' recipes
  (`src/emu/xnavi_bn4.c`) beside the families': his ids (BN4's 0x84-0x87
  BN6's 0x384-0x387, `src/dev/xdiff_bn4.c` `xd4_bn6_id` too), his effects,
  sprites and moved types; his object type and CurActions read by the
  families' rule (his type BN6's 0, his own from 6 BN6's from 8, MegaMan's
  67-70 among them as his 7-10). `tools/xdiff_snap.py` reads a pointer
  into a BN4 AI data by BN4's 0xC0 bytes a side (a Navi's after MegaMan's,
  not within his), and a pointer into BN4's Navis' moved code
  (`FREE_BN4_NAVI_CODE`) as the ROM's. The virus masks (`BN4_LACKS`), whose
  reasons are a virus's (BN4 keeps no AI data for one), never hold for a
  Navi's AI data or its pointer (`NAVI_NAMES`, his recipe's ids): his AI
  data both games keep is compared whole (his AI index read by the
  comparator's own map). A BN4 spec may record its verdict while its
  port is under way (`"verdict"`, as the chips' specs: held where the run
  ends the same at the same frame, never for a mutation's run). An object
  spawned while the hooks watched that has set up no sprite since holds
  the slot's last occupant's sprite state in each game, compared from its
  sprite's set-up on (`leftovers`, as BN5's `Obj.leftover`), and a moved
  object's word its spawn took from a register its caller never set is
  each game's leftover, allowed by its rule with both paths
  (`spawn_leftover`: GutsMan's attack's 0x1C's +0x3C).
  A first-kind object of a type a Navi's recipe moves (its `XvType` of kind 1: FireMan's companion
  0x57, an object of no enemy id) is told by its type, BN6's dispatched number read back as BN4's
  (`xdiff_snap.py` `T1_MOVED`, `xdiff_bn4.py` `t1_moved`), not by its NameID, which BN4's 0 would read
  as MegaMan's; its +9 and its id are its own code's, kept, and its spawn word's sprite number
  (+5) is read as the recipe's copy (`fireman_v1`, [BN4's Navis in BN6's engine](../multirom/bn4-navis.md),
  FireMan built). A Navi's companion whose own animation, set in the paused opening, its after-step
  runs at every fight frame (AquaMan's 0x5C, kind 1) is compared at the same fight frame until its sprite
  state first steps with the battle's offset, exactly from then (legit.json's bn4 `companion_clock`,
  `tools/xdiff_bn4_opening.py` `CompanionClock`; `aquaman_v1`, [BN4's Navis in BN6's
  engine](../multirom/bn4-navis.md), AquaMan built).
  `gutsman_v1` ([BN4's Navis in BN6's engine](../multirom/bn4-navis.md),
  GutsMan built) PASSes over its 1200 frames (his own collision argument
  ported, each game's own AI bookkeeping allowed on a Navi, `navi_ai`, his
  opening count by the frame's clock, his draws keyed by his code's room,
  his rocks' read of a side's player made BN6's). What it took of the
  comparator: the camera's shake's two draws a frame paired as BN5's are
  (legit.json's bn4 rng_twins take a `word`: BN4's draws there from its
  first RNG word, BN6's from its second, `tools/xdiff_bn4.py` rng_twins);
  a cracked panel's flags of the last frame mapped (BN4's +8, BN6's
  +0x18: `BN4_PANEL_FLAGS_LAST`, `BN6_PANEL_FLAGS_LAST` in the trace's
  panel layout); the panel crack's sound and his copies at the pool's
  slots paired by the routines that play them; a pointer into a moved
  sprite read as the sprite's whose start is nearest below it (a copy's
  room is rounded up, so in BN4's ROM it may reach into the next sprite:
  list 4's 0x0D into 0x0E, his rocks'); and the rocks' debris, the fourth
  kind's 0x09 whose twin inits set up each game's own rock art, allowed on
  its sprite state alone ("the rock's debris, each engine's own art").
  `thunderman_v1` ([BN4's Navis in BN6's engine](../multirom/bn4-navis.md),
  ThunderMan built) PASSes over its 1200 frames once his companion and cloud,
  which run no state, are gated a frame into BN6's fight with his first own
  state (`XV_FIGHT_FLAGS`), so his whole battle keeps the one offset. What it
  took of the comparator: an animation run through the paused opening
  compared by the two openings' lengths (legit.json's bn4
  `opening_animation`, `tools/xdiff_bn4_opening.py`: his cloud's, BN4's 227
  frames from its spawn to the fight against BN6's 206, its place in its
  18-frame loop as far apart, modulo the loop, until an animation is set in
  the fight); a sound row that holds in one Navi's battles alone (`foes`: his
  0x70's BN4 0xAD at the pool's 0x32, where Billy's thunder's 0xAD is BN6's
  own 0xBA); a spawn's leftover read as a named pointer or by its bytes
  (`spawn_leftover`'s `bn6` a list, or `bytes`: his 0x70's +4, his 0x71's +5
  and +6); a hit's spark taking no rank in the family's renderer places, its
  life each engine's art (`bn4_renderer`); a frame of hits by element laid
  out in the BN4 trace at BN6's places (BN4's +0x2A and +0x3C, BN6's +0x82
  and +0x94, five u16 each: `src/dev/xdiff_bn4_trace.c`, [BN4 Red
  Sun's tables](../rom/bn4.md), What a hit leaves the struck, by element);
  and BN6's own Elec tally for its freeze (+0x78), allowed on MegaMan's
  collision data where BN4's holds nothing.
  `roll_v1` ([BN4's Navis in BN6's engine](../multirom/bn4-navis.md), Roll
  built) PASSes over its 1200 frames: a moved fourth-kind object (her
  flash, the fourth kind's 0x17) is told by its type read back as BN4's
  (`xdiff_snap.py` `identity`, `xdiff_bn4.py` `Bn4Canon.t4`), as a moved
  third-kind one is; her flash's +4, its spawn's r4, is a spawn leftover,
  BN4's her +0 byte as its object runner read it (0x13, or 0x11 where her
  turn began hidden); her pool sounds pair in her battles alone (`foes`).
  The families her summon brings (BN4's Mettaur, moved for her) are hers
  in the comparison: their code keyed as hers (`src/dev/xdiff_bn4.c`
  `xd4_families`), their effects mapped (`xdiff_bn4.py` `summoned_foes`),
  her summon's draw, its shim's, keyed as `BN4_SPAWN_ENEMY`'s
  (`xd4_summon_draws`, `XV_SHIM_SUMMON_DRAW`), and a summoned enemy's
  +0xF, BN4's +0xE as the trace moves it (its appear queue's number: 1
  her first summon, 2 her second), allowed against BN6's 0 alone, each
  summoned version's own pair (a Billy's read as the word at +0xC, as its
  value looks like a pointer). `mettaur4_v1` fights that Mettaur alone and
  PASSes. Her other versions (`roll_v2`, `roll_v3`, `roll_sp`; V2 over
  2400 frames, both her summons, in `roll_v2_summons`) add a Navi's version
  at AI data +0x16 and +0x17, BN6's (its actor setup's, from the id's
  triple), a word read whole held to its differing bytes (V3's 0x02020000
  at +0x14), and her flash's spawn's r4 for each version
  ([BN4's Navis in BN6's engine](../multirom/bn4-navis.md), Roll's
  versions).
- **Against BN6's own Mettaur** (`game=bn4:bn6`, foes by BN6's ids): both
  battles trace their 600 frames; they part at t1, as they must until
  BN4's numbers are mapped: BN4's object flags (+0 0x13, BN6's 0x17) and
  type (BN4's family's own type, BN6's 0), its CurActions (MegaMan's
  ready 6 against BN6's 8, as BN5's), BN6's own effect at the fight's
  start (the fourth kind's 0x08), the RNG words (BN6 draws through
  BATTLE START!, BN4 not), the border panels' side (BN6's 0xFF), and
  collision data bytes BN4 keeps elsewhere: at t1 BN4's +0x05, +0x0C,
  +0x1E, +0x58-+0x5F, +0x60 and +0x78 held BN6's +0x07, +0x0E, +0x2E
  (the damage), +0x30-+0x37 (the alliance words), +0x38 (its object) and
  +0x44's values (leads from one frame, since proved by pairing the two
  games' routines and joined to the layout, above).
- **BN4's Billy against its BN6 twin** (`billy_v1`: Billy V1, BN4's 0x0D,
  at (5,2) against MegaMan idle at (2,2) with 2000 HP, `--dev
  bn4families`, `rng=replay`, `lag: 1`): both battles trace their 1200
  frames, all 30 of Billy's own draws replayed from BN4's (the recipe
  giving struct2 BN4's attack word as it is and its flags byte from the
  family's init, `XvTraits.stats_flags`: the struct2 finding below; its
  thunder's hit bits once parted at t341: the thunder, below). The RNG
  words are not compared where BN6's draws
  are replayed (BN6 steps both three times a frame, its main loop's
  `0x080050FC`, `0x08005100` and `0x0800031C`, 1473 times each in the
  1200 frames; BN4 never does, its 177 draws there none a frame's own):
  each of the family's draws must be given BN4's word instead, one drawn
  as it is the family's difference. On the recipe that moves every BN4
  family's effects (Billy's 0x15 and 0x16 BN6's 0x90 and 0x91) and runs
  Billy's thunder from a copy of BN6's 0x2A whose init passes r3 = 0
  (`tools/xrecipe_bn4_copy.py`, `src/emu/xvirus_bn4_copy.c`: the hit's
  bits BN4's 0, the thunder finding below), the family's own records are
  BN4's in all 1200 frames, and `billy_v1` is INCONCLUSIVE at t1 on
  MegaMan alone (below). The comparator reads a dispatched object's type
  (its side-table number in the BN6 trace) back as BN4's own where the
  recipe moved it (`bn4_moved_types`, the recipe's `XvType` rows: the
  copy's 0x2A, Spikey's 0x30 at the dispatch's 0xE0), so a moved type
  meets BN4's at any number the dispatch gives it, and gives each family
  the effects its own struct names (`bn4_families`, by `.effects`, never
  by order: Elmperor names none).
  - **The world, mapped from both games' code** (each in
    `tools/xdiff_bn4.py` or `tests/xdiff/legit.json`'s `bn4`, `allowed`):
    the panels as BN6's field init holds them (BN6 `0x0800C51A`-
    `0x0800C566`: +0 and +0xC from its two tables by place, its column
    and row at +0xA and +0xB, its type again at +6 and its side at +4 and
    +7, a side by its column's, 0xFF on columns 0 and 7; BN4's
    `0x08009120`-`0x0800920C` writes its type, its side, 0 off the 18,
    and three u16), BN4's +0xC 0x258 BN6's +0xE 0x258 (each init's first
    store, `0x0800913A` / `0x0800C562`), the other timers idle in both
    read alike (BN4's +0xE 0x3C0 and +0x12 0x80BE, BN6's +0x10 0 and
    +0x12 0x708: only the flags and the reserver move in either game
    through `billy_v1`); MegaMan's own sprite (each game's art: BN4's
    list 0's entry 0, `0x08214000`, BN6's `0x081D8000`); his +0 bit 0x04
    (BN4's player state 1 `0x08010A88` clears it, BN6's `0x08017888`
    keeps it); BN6's MegaMan's companion, the fourth kind's 0x08 his
    player init spawns into his AI data +0x58 (`0x080173CA`-`0x080173CE`,
    `0x080E223E`), whose BN4 twin (`0x080E28EC`) nothing calls: its being
    in play, its place in the run order and his pointer to it. With the
    thunder's hit bits given BN4's (the raw-write experiment) the world
    parts at t1 on MegaMan alone: his object's +0xB, +0x20 and +0x2A and
    his AI data (BN4's 0xC0 bytes, laid out otherwise), since paired by
    their readers, the battle's one offset and the two clocks (below): it
    PASSES, the world's records and the family's the same in all 1200
    frames.
  - **MegaMan's AI data and his object, by their writers** (a probe of
    hooks after every call of both player inits, BN4 `0x0801079C` and
    BN6 `0x080172F0`, and of both updates, BN4 `0x080EAECC` and BN6
    `0x080EB7C4`, printing his AI data where it changed; not yet a map:
    the readers of each field are still to pair):
    - before the init: BN4's +0x50 a pointer (`0x02035A70`, the AI
      data's own setup); BN6's +0 2 (the index his type's routine
      `0x080B9A5C` dispatches by, `0x080B9A62`: 2 his routine
      `0x080EB7A1`; BN4's type routine `0x080B34D8` calls his own
      `0x080EAEA8` without one), +3 1, +0xE 0xFF, +0x7F 0x80.
    - the charged shot's setup, twins (both `movs r0,#0xa; strh
      r0,[r5,#0x2e]`, r6 a loadout record by side, BN4 `0x0800D742`,
      BN6 `0x08013682`; r7 the AI data): BN4's `0x0800D836` stores
      record +0xA into +0xF (1), +0xB into +8 (0), +0xC into +0 (0xFF)
      and 0 into +5 (`0x0800D8CE`-`0x0800D8DC`); BN6's `0x08013892` stores
      record +5 into +7 (1) and +7 into +8 (0xFF) (`0x08013918`-
      `0x0801391E`); both then set the same four object flags from the
      record (0x20, 0x10, bit 18, bit 17: BN4 its +2, +3, +4, +1, BN6 its
      +0x1B, +0x1C, +0x1D, +0x23). Paired by their readers, now in the
      trace's AI layout (`BN4_AI_POWER_*`, `BN6_AI_POWER_*`): each game's
      power attacks are read by small dispatchers into its table (r7 the
      attack variables, BN4's +0x70, BN6's +0xA0): BN4's `0x0800C9D2`
      (+0xD), `0x0800CA60` (+0xF) and `0x0800C9E8` (+0), called in that
      order by his attack state (`0x080EEC1E`, `0x080EEC3E`,
      `0x080EEC5E`), each after the same tests, as BN6's `0x08011764`
      (+6), `0x080117A4` (+7, the charged shot's) and `0x08011790` (+8)
      by his (`0x080F17CA`, `0x080F17F2`, `0x080F182E`): BN4's +0xD, +0xF
      and +0 are BN6's +6, +7 and +8 (0, 1 and 0xFF in both at t1). BN6's
      fourth, `0x0801177A` (+4, `0x080F185A`), has no BN4 twin there.
    - the loadout's clear and set, twins in shape (12 bytes cleared at
      the side's block +0x26, then a routine by the Navi's style): BN4's
      `0x0800E05C` writes +0xC 0xFF, +0xE 0xFF, +0x34 0xFF and +0x36
      0x78; BN6's `0x080144C0` writes +4, +5 and +0x11 0xFF (his power
      attacks but the charged shot's).
    - before the first update (the battle's opening, not the init): BN4's
      +0x1D and +0x23 0xFC, BN6's +0x23 and +0x29 0xFC (+0x25 a frame);
      in the first update, BN4's +0x10 0xFF (`0x080EAECE`-`0x080EAEDA`'s
      calls) and BN6's +0xF 1; from his state runner (BN4 `0x08013A48`
      with his table `0x080EAEFC`, his ready state 6 `0x080EB0CC`
      dispatching by his NaviStats' 0x23 and 0x26), BN4's +0x4C 0x10,
      his object's +0xB 4 and +0x20 9, counted down (BN6's +0x48 0x10 a
      frame later; his ready 8 at `0x080EBA74`, by his form's record,
      sets neither at t1 and both at t2: below).
    - his object's +0x2A, 0xFFFF a held chip none: BN4's player state 2
      (`0x080108A6`-`0x080108AA`) writes it once it runs, after t1; BN6's
      before his first update, not in his init (none of its stores).
    - allowed, each game's own (`legit.json`'s `bn4`): BN6's AI data +0,
      the index his type's routine picks his by (`0x080B9A62`; BN4's type
      routine calls his own, `0x080B34D8`), and +4, BN6's fourth power
      attack (`0x0801177A` at `0x080F185A`, where BN4's attack state calls
      three).
    - the 17 that were left, by their readers (`src/emu/bn4_battle.h` and
      `src/emu/bn6_ai.h`, [BN4 Red Sun](../rom/bn4.md), MegaMan's AI data
      against BN6's; every load and store of each offset whose base came
      from an object's +0x58, in both ROMs' first 2 MB). Mapped in the
      trace's layout (`src/dev/xdiff_bn4_trace.c`), the same field at
      another offset, each by twin routines: his keys (BN4's +0x1C to
      +0x2B BN6's +0x22 to +0x31, the update `0x0800BA78` / `0x08012DFC`:
      BN4's +0x1D 0xFC was his held keys' mark, BN6's +0x29 the last
      held's), the flags word (BN4's +0x4C BN6's +0x48, test, set and
      clear `0x0800C1D6` / `0x08010300`), the chip charge's power attack
      (BN4's +0xE BN6's +5, both read for its charge time), the HP bug's
      count (BN4's +3 BN6's +9), the damage taken (BN4's +6 BN6's +0x20)
      and the frames struck (BN4's +0x54 BN6's +0x4C), the last two
      parting after t431 only (below). BN4's bytes with no place of BN6's
      now go past BN6's attack variables (`BN6_AI_SPARE`, +0xF0, which
      BN6's AI take never lays out), so a field of BN6's meets BN4's
      nothing rather than another field. Allowed, each game's own, each
      only where the other game's side reads 0 there (`legit.json`'s `bn4`,
      its `direction`, `tools/xdiff_bn4.py` `holds_nothing`): BN4's +0xC
      (its loadout's first byte, written and read by no code), +0x34 to
      +0x37 (the charge times it caches, `0x0800BCF4`, where BN6 reads its
      table at each test) and +0x50 (its side's control block, `0x02035A70`,
      run by his ready `0x080EED48`; BN6's ready keeps none); BN6's +3 (a
      palette step its readers use only for viruses and Navis), +0xE (a
      mark whose two readers keep nothing), +0xF and +0x11 (Beast Out's
      gate and Beast chips' charge) and +0x7C to +0x7F (the pool's bit;
      BN4 keeps one AI data a side, no pool).
    - what that left at t1 (6 bytes), each settled since: his object's +0xB
      and +0x20 and his flags' 0x10 (BN4's +0x4C, BN6's +0x48) were his
      ready state's, the same values in both a fight frame apart (BN4's
      ready 6, `0x080EB0CC`, at t1 sets +0xB 4 and +0x20 9 and counts to
      +0xA 4 at t10; BN6's ready 8, `0x080EBA74`, the same from t2 to t11,
      the earlier "sets neither" its first frame alone): the state runners'
      difference on MegaMan, now the battle's one offset (below), under
      which they are the same. BN4's +0x10, the pending form (0xFF none,
      `BN4_AI_PENDING_FORM`), is BN6's `BN6_CROSS_PENDING` +0 (`0x0203A980`,
      `BN6_RECORD_FORM`), outside BN6's AI data: the trace's player block
      carries BN6's pending record, and BN4's trace writes his AI data's
      +0x10 at its +0 (`xdiff_bn4_trace.c`, `XD4_PENDING`), compared there
      (`tools/xdiff_bn4.py`, "pending form"; the record's other bytes BN6's
      own), both 0xFF all battle. BN4 writes it as his turn starts
      (`0x0800B8CC`) and his unison's states read it (`0x080EBAD4`,
      `0x080EBB1E`, `0x080EBB4E`); BN6's turn's form check (`0x0801486C`)
      and its crossing (`0x08014A82`) read its pending record. His object's
      +0x2A, the held chip (0xFFFF none), is read of him by neither battle
      (allowed, `legit.json`'s "MegaMan's held chip"): BN6's four readers
      (`0x0800EDC6`, `0x0800EE1A`, `0x0800EE8E`, `0x0800EF2A`) read it only
      for an object whose name's class (`0x0800F29C`) is not a MegaMan's,
      his chip read from his side's hand; BN4's one (`0x08014AFE`) reads the
      objects of its list at `0x0203BE40`, which only the enemies' inits
      fill (`0x080167C0`, 13 calls, none his); the search: every load at
      +0x2A in both battle engines and in his own code, every
      `ldrh [r5,#0x2a]` of the first 2 MB by its type. His saved state word
      (+0x5C, the +8 to +0xB a status keeps while it holds him: BN4
      `0x0800AE80`, BN6's twin `0x0800E760`) is read with the CurAction's
      map at +0x5D (`tools/xdiff_bn4.py`, `OBJ_SAVED_ACTION`: his ready, 6
      BN4's and 8 BN6's).
    - **Two clocks** (the lead's call, 2026-11-04, the owner's to overturn;
      the rule written once, `legit.json`'s `bn4` `clocks`): what a state
      drives is compared with the battle's offset (below), what the
      battle's timer or the frame counter drives at the same fight frame,
      each record's clock shown by the routine that writes it, never
      assumed; one whose clock is not shown is compared with the offset and
      keeps the spec INCONCLUSIVE where it differs there. Shown so far,
      the sounds (`legit.json`'s `bn4` `sounds`, `tools/xdiff_bn4.py`
      `sounds_diff`): each of BN4's as BN6's number by the routines that
      play it in both games, their songs sharing no sample (`song_sig`,
      and by their samples alone: each game's own recordings): the
      thunder's launch (BN4's 0xAD from its init `0x080D20AE`, BN6's 0xBA
      from its twin's `0x080CAD12`; a state's), MegaMan struck (0x6B in
      both, `0x08013A9C` / `0x0801BA5A`; a state's), and the Custom gauge
      full (BN4's 0x81, BN6's 0x8F, from the gauge's fill, twins
      instruction for instruction, `0x080147D4` / `0x0801C488`, which the
      main loop runs each fight frame, no state: the timer's, both at
      t512); a BN4 sound not listed is compared as its own number. Two
      records found by a probe of hooks on both cores (temporary, in
      `src/dev/xdiff.c`, not kept) and their writers: his sprite's +1
      (+0x91) is the count of its animation, which both games' animation
      setter (BN4 `0x08002E10`, BN6 `0x08002DA4`) starts in the opening
      (fight frame 0, before the first state) and the sprite update (BN4
      `0x0800285C`, BN6 `0x080026C4`, at every fight frame and none of
      the paused opening's) counts down, 2 at t1's update, 0 after t2: the
      opening's count by the fight's frames, so BN4's t1 and t2 are
      compared with BN6's of the same fight frame (`legit.json`'s
      `opening_count`), a count a state starts (his paralysis's end, t520
      and t521) with the offset; and his paralysis's flash, his colour
      shader (his sprite's +6, +0x96), set by twins (BN4 `0x08010408`,
      BN6 `0x080168C8`) while his collision flags hold 0x800 (a state's,
      the status routines' `0x0800AEB4` / `0x0800E798`, t431 and t432)
      from the battle timer's bit 1 (BN4 `0x08007BE4`, BN6 `0x0800A704`,
      the timer's): each game's compared with what that code gives, the
      gate BN6's flags of t + 1 and each game's own timer
      (`paralysis_flash`).
    - the hit's spark (the fourth kind's 0x04), each engine's own effect:
      its place was a draw of the engine's under the family's thunder
      (twins BN4 `0x08014620`, BN6 `0x0801BDDE`, GetRNG once, the
      striker's place moved by two nibbles), now given BN4's word
      (`legit.json`'s `bn4` `rng_twins`, checked byte for byte against
      both ROMs by `tools/xdiff_bn4.py`; with the battle's offset every
      keyed draw is given BN4's of the frame before, `xd_replay_lag`), so
      it stands where BN4's does; its sprite state and its life are its
      own art's (twin spawners `0x080E2140` / `0x080E1C00`, the same list
      and number in each game's art, ended by twin updates as its
      animation ends, `0x080E2122` / `0x080E1BE2`): allowed, that object
      alone ("the hit's spark, each engine's own").
    - MegaMan's collision flags while paralysed: 0x02000800 in BN4 and
      0x02400800 in BN6. Both games' status routines set 0x800 as the
      paralysis's timer runs (BN4 `0x0800AEB4` at t431, BN6 `0x0800E798` at
      t432 and t1022, the probe) and both its ends clear it; BN6's
      paralysis state (his state 4, entry 4 of `0x080EB86C`: `0x080175B8`)
      also sets 0x400000 as it begins (`0x080175C4`), cleared at
      `0x08017828` (in his state 7, `0x08017768`) and read by `0x08010162`
      (from `0x0801B12E`, clearing the flags' 0x04 each frame while it
      stands), where BN4's (`0x080109FA`) sets none. Allowed as BN6's own
      bookkeeping (the lead's call, 2026-11-04, the owner's to overturn;
      `legit.json`'s "BN6's MegaMan's paralysis state's flag 0x400000"),
      that bit alone, BN6's side holding it and BN4's not, read whole as
      the word or as its byte (`tools/xdiff_bn4.py` `raw_int`); every other
      bit, the 0x04 it acts on among them, compared as ever.
    - his anger, which the hits of `billy_v3` raise (a struck word's 0x200:
      flag 0x200000 and 600 frames), is mapped: BN4's +0x2E is BN6's +0x34
      (`BN4_AI_ANGER`, `BN6_AI_ANGER`; BN4 `0x0800C594`, BN6 `0x08014350`,
      instruction for instruction), and BN4's charge-time cache, which sat
      at +0x34 in the trace, kept in the spare's last word (+0xFC, still
      allowed as BN4's own).
    - **`billy_v1`, `billy_v2` and `billy_v3` PASS**: 1200 fight frames
      each, the family's own records and the world's the same in all of
      them under the battle's offset, the two clocks and the allowed
      differences above, each with its reason in `legit.json`.
  - **Its mutations** (`tests/xdiff/mutations.json`; `tools/xdiff.py`
    gives a BN4 spec a mutation's `poke` and `call`, and `call=` aims BN4's
    families' veneers too, `xd4_family`), each FAIL on the family: a state
    immediate off by one (`poke=080B9600:200A`, the recipe's CurAction 7
    written as BN6's 9 made 10: t1, its CurAction); a call aimed
    elsewhere (`call=0801BB1C:0810A952`, its panel reservation at BN6's
    family nullsub: t43, no 0x20 in its object); the thunder's spawner
    aimed at another type (`poke=08B7C184:2029`, the copy's spawner,
    BN6's `0x080CADBC` at `XV_CODE + 0x14180`, spawning 0x29: t341). A
    call aimed from GetPositiveSignedRNG to GetRNG
    (`call=08001532:0800151C`) is no break for Billy: its draws, BN4's
    words replayed, read alike with bit 31 set or not, and the family
    stays the same (INCONCLUSIVE).
  - **Its other versions**: `billy_v2` (BN4's 0x0F, BN6's 0x2C2) and
    `billy_v3` (BN4's 0x11, BN6's 0x2C4), as `billy_v1` else: each
    PASS (below), the family's own records the same in all 1200 frames.
  - **The thunder** (BN6's own third-kind 0x2A run for Billy's
    parameters: the recipe's audit, `tools/bn4_twins.py` AUDITS
    (0x080D2156, 0x080CADBC)), field by field at t341, each init (BN4
    `0x080D2040`, BN6 `0x080CAC7C`) and the hit pass (BN4's from
    `0x08012BC0`, BN6's twin from `0x081D7830`) read side by side, a probe
    of hooks through both inits giving each store's place:
    - its sprite (+0xA8 to +0xBB): each game's own art, by design (BN4's
      list 0x10 number 0x11 at `0x080D2044`, BN6's number 0x16 at
      `0x080CAC80`; other bytes, CRC32 `0x030B3B0A` and `0xAF2AFC99`); the
      frame pointers into it and the word +0xB8 copied from its data
      (`0x0800210E`-`0x08002120`), Billy's copied art holding the same in
      both: allowed (`tests/xdiff/legit.json`, `bn4`, "the thunder's
      sprite"), that object's sprite state alone.
    - coll +0x05 and +0x09: no difference but the trace's placement:
      BN4's +7 is BN6's +9, each set to 3 by the init (BN4 `0x08012F74` /
      BN6 `0x0801A140`, the same store); BN4's +0xE is BN6's +0x10, the
      status a hit gives, from the object's +6 (BN4 `0x08013212` / BN6
      `0x0801A258`) and passed by the hit pass to the struck's +0xF / +0x11
      (`0x08012BEC` / `0x081D784E`): both 0x10. With the hit pass's other
      stores (the hit bits, the moods, the hits taken, the alliance words,
      the push, the pairs met, `bn4.h` `BN4_COLL_*`) the layout takes them.
    - coll +0xE, the hit's bits: the inits' collision setups give BN4's
      thunder r3 = 0 (`0x080D208C`) and BN6's r3 = 1 (`0x080CACD6`, `movs
      r3,#1`), which the hit pass ORs into the struck's hit bits (BN4
      `0x08012C40`-`0x08012C46` into its +0xD, BN6 `0x081D78B2`-`0x081D78B8`
      into its +0xF). What the player feels in `billy_v1` is the same: each
      thunder takes 15 HP and paralyses MegaMan 89 frames, his state 4
      then his ready, in both (t431 and t1021 in BN4, a frame later in
      BN6). What it leaves differs: MegaMan's saved state word (+0x5C, the
      action a status interrupts) holds his ready (BN4's 04 06 04, BN6's
      04 08 04 with BN4's bits) where BN6's thunder makes it 04 03 00,
      generic state 3, the struck reaction its bit 0x01 starts and the
      paralysis cuts short. With BN6's thunder given BN4's bits (a raw
      write of `0x2300` at `0x080CACD6`, an experiment, not committed),
      the family's own records are BN4's in all 1200 frames and MegaMan's
      saved word is the only record of his it changes. So the verdict to
      keep BN6's thunder holds for its art, place, timing, damage and
      paralysis, and not for its hit's bits: a struck the paralysis does
      not take would start its struck reaction in BN6 and not in BN4 (not
      measured). The spawner's parameters cannot give it, `movs r3,#1`
      being BN6's init's own: the recipe moves BN4's third-kind 0x2A, as
      BN5's moved objects are, or runs a copy of BN6's 0x2A whose init
      passes r3 = 0 (never BN6's own, which its own users run).
  - **The struct2 finding** (fixed since): the recipe's struct2 (`src/emu/xvirus.c`,
    `stats_bn4`: "element << 12 | its attack's damage", its flags byte 0):
    BN6's virus init copies a version's struct2 word +4 to the object's
    +0x2C (`0x300F` for Billy at its spawn, BN6's own Mettaur `0x000A`),
    and the collision setup reads its top bits as the hit's properties,
    never an element: BN6 `0x08019F44` keeps 11 bits of damage (lsls #21;
    lsrs #21), bit 15 doubles it, bit 14 sets +0x10 0x10 and +0xE 1, bit
    13 calls `0x0801A4D0` with 0xF8 and returns (Billy's +0x12 and +0x13,
    written in its collision setup at `0x08016FE8`: a probe of hooks
    through the virus init), else bit 12 with 0xF7; BN4's twin
    `0x08012860` keeps 14 bits and reads bits 15 and 14 alike in its own
    fields. Elec's 3 << 12 sets bits 13 and 12, so Billy's body takes bit
    13's hit property, which BN4's has none of. And BN6's
    virus init sets the object's flags from struct2's byte +3
    (`0x08016FCE`-`0x08017030`: 0x10 its 0x08000000, 0x08 its
    0x02000000, 0x04 its 0x20, 0x02 its 0x10, 0x01 its 0x00020000), where
    BN4's Billy sets 0x02000000 in its own init (`0x080B9552`: `ldr
    r0,=0x02000000; bl 0x08012F82`, object_setFlag). With the version's
    struct2 given BN4's damage alone and byte +3 0x08 (an experiment,
    `poke=08B5804B:08:1` and `08B5804C:000F:2`, V1's struct2 in its home,
    not committed), Billy is the same as BN4's through t340, its moves,
    its effects and its draws among it; at t341 its thunder (the third
    kind's 0x2A, BN6's own object running Billy's parameters) differs
    (the thunder, above). The recipe now gives struct2 both
    (05016ae4), and the spec runs so.
  - **The allowed difference, the family's timeline a fight frame later**
    (the owner's call, 2026-11-04: the port keeps BN6's timing; its
    reason in `tests/xdiff/legit.json`, `bn4`, `allowed`): both games'
    pause byte (BN4's `[eToolkit+0x40]+9`, BN6's `[eToolkit+0x3C]+0xA`)
    reads 1 from the appear to the fight, and each object runner (BN4
    `0x0800314C`, BN6 `0x080031AC`) runs a paused object whose +0 has
    0x04. BN4's state runner (`0x08013C48`) has no pause test and calls
    the state (`0x08013D9A`), so BN4's generic state 1 (`0x0801013A`)
    runs in the opening, clears 0x04 and sets the family's first own
    state, which runs at the fight's first frame. BN6's (`0x0801B1C4`,
    from the virus dispatcher `0x0810A29C`) tests the pause byte first
    (`0x0800A03C`, at `0x0801B1CC`) and, for a CurAction not 0 whose flags
    (`0x0801A166`) have none of 0x40000100 and whose struck word
    (`0x0801A180`) has no 0x8000, returns at `0x0801B21A` (`b 0x0801B37A`)
    without calling the state: BN6's generic state 1 (`0x080165B8`) runs
    at the fight's first frame and the family's first own state at the
    second. BN6's own Mettaur starts the same way against BN4's. The
    family's +0 bit 0x04, which BN4's state 1 clears and BN6's keeps, is
    allowed beside it (its own entry there), on the family's objects
    alone, that bit alone.
  - **One offset for the whole battle** (the lead's call, 2026-11-04, the
    owner's to overturn; it replaces the family-only lag of the owner's
    earlier call, and its reason is written once, in `legit.json`'s
    `bn4` entry "BN6's battle a fight frame behind BN4's"): the state
    runners' difference above holds for every state of BN6's engine, so
    MegaMan's run a frame late too (his state 1, `0x08017888`, at t1 and
    his ready 8 from t2, where BN4's ready 6 ran at t1) and everything the
    states do follows: Billy's moves and reservations, its thunder
    striking him (t431 in BN4, t432 in BN6), the sounds they play. With
    `"lag": 1` (`tools/xdiff_bn4.py` `allowed_lag`, refused unless
    `legit.json` names that lag for the battle) every record of BN6's
    battle is compared at t + 1 against BN4's at t: the family's, his and
    the world's objects, the panels, the run order, the sounds, the
    events and the family's draws (given BN4's of the frame before,
    `xd_replay_lag`); BN6's battle is traced a fight frame longer and its
    keys by the timer taken a frame later (`src/dev/xdiff.c`,
    `Side.behind`), so all of BN4's 1200 frames are compared. Never a
    field or a role apart: what still differs under the shift fails the
    spec (the family's) or keeps it INCONCLUSIVE (the world's). What is
    fixed before the first state keeps its own handling: BN6's per-frame
    RNG steps (not compared where the draws are replayed) and BN6's
    MegaMan's companion. BN6's first fight frame, its states' 1, meets
    BN4's paused opening, which the trace does not keep, and is not
    compared.
  - **Maps added for it**, each from both games' code: the alliance words
    by their place in each game's table (BN4 `0x08012660`, 122 words, read
    by `0x08012EF6`; BN6's first 122 the same or with bit 0x80, 98 of
    them); the last move's direction (BN4's +0xA, BN6's +0xC, by
    `0x0800AF90` / `0x0800E994`: BN6's left and right by the object's
    side, a move of two panels its 5); the counter time (BN4's +0xB,
    BN6's +0xD, by `0x0800AFBC` / `0x0800E9DC`); a reservation's bit 0x20
    in BN6's terms from BN4's events (BN6's `0x0801BB36`, BN4's
    `0x080143A8` sets none); an object's slot (+3) counted from its own;
    the moved effects' ids by the recipe (BN4's 0x16 and 0x15, BN6's 0x98
    and 0x99); the hit pass's fields, store by store (BN4's from
    `0x08012BC0` and `0x08012AB2`, BN6's from `0x081D7830` and
    `0x081D7B1E`), the hit words read through the alliance tables too
    (BN6's +0x6C and +0x70) and BN6's +0x7C, the strikers' index bits, a
    field BN4 has none of (`BN4_LACKS`); and the family's renderer places
    as their ranks among the family's own drawn (BN5's map, legit.json's
    `renderer`, `bn4_renderer`): BN6's thunder's own art takes other
    tiles, which moves the places of what is drawn after it.
- **Walla's fight** (`walla_v1`-`walla_v3`: Walla V1-V3, BN4's 0x67,
  0x69, 0x6B, at (5,2) against MegaMan idle at (2,2) with 2000 HP, as
  Billy's specs else): each of Walla's rings strikes MegaMan (eight hits
  in 1200 frames: t68, t225 ... in `walla_v1`), his struck state 3 (BN4
  `0x08010960`, BN6 `0x080174FE`, `BN4_PLAYER_STRUCK`) runs 23 frames and
  he flashes after it. The specs parted on three things, none the port's:
  - **The blink's phase** (MegaMan's +0 0x13 against BN6's 0x15 at t68,
    the 0x02 BN6 lacked beside the 0x04 already allowed): both games'
    blink step (BN4 `0x08010440`, BN6 `0x08016944`, the same steps) hides
    an object whose flags hold 0x202 by its flash timer's bit 1 (BN4's
    +0x18, BN6's +0x24, mapped and the same count), BN4 where it is clear
    (`0x08010456`, bcs past the bics), BN6 where it is set (`0x0801695A`,
    bcc): opposite phases of one timer, as BN5's against BN6's. Read as
    BN5's is (`tools/xdiff_events.py` `Translations`), not masked: an
    object BN4's blink step ran on at a frame (its blink events at
    `BN4_BLINK_FLASHING`, `tools/xdiff_bn4.py` `blinked_roles`) has its
    visible bit read in BN6's phase. An experiment shows it hides nothing:
    BN6's step turned to BN4's phase (`--poke 0801695A:D203`, bcs) parts
    `walla_v1` at t68 on that bit (INCONCLUSIVE, the world's).
  - **The hit's spark, the family's**: Walla's ring calls the effect its
    hits make itself each frame (`BN4_HIT_EFFECT`, BN4 `0x080D4BEA`: bl
    `0x08012F44`; its copy's veneer to BN6's twin `0x0801A0D4`), so the
    spark it spawns (the fourth kind's 0x04, through the twin spawners
    `0x080E2140` / `0x080E1C00`) is counted the family's, and BN6's art
    ended it two frames sooner (t72, BN4's t74). It is the engine's
    effect still: legit.json's "the hit's spark, each engine's own" holds
    on the family's records too, that object's sprite state and life
    alone (`tools/xdiff_bn4.py` `compare`), its spawn, place and being in
    play before both had it compared as ever. Its mutation
    (`tests/xdiff/mutations.json`, "a ring's hit effect aimed elsewhere",
    `call=0801A0D4:0810A952`: no spark in BN6) fails `walla_v1` at t68.
  - **MegaMan's own art's animation** (his +0x91 BN4 1 against BN6's 3 at
    t80): his struck animation (1) runs 24 frames in both, cut into
    frames 4,2,2,2,2,2,4,4,2 by BN4's art (`0x08214000`) and 4,2,2,2,2,4,5,3
    by BN6's (`0x081D8000`; a frame's record 20 bytes, its duration at
    +16, +18's 0x80 its last, the sprite update `BN4_SPRITE_UPDATE` /
    `0x080026C4` loading a duration less 1 into the count). Allowed
    (legit.json's "MegaMan's own art's animation"): his count and his
    flags' last-frame bit (+0x92's 0x80, BN6's a frame sooner, t89) only
    where both are what their own art gives at the same frames run of the
    same animation, each walked from its own ROM by his frame's record
    (+0xAC) and count (`own_animation`, `anim_place`). An experiment
    shows the guard: BN6's sixth frame shortened to 2 in its ROM copy
    (`--poke 081D80EC:02:1`) parts `walla_v1` at t82 on the count.
  - BN6's struck state sets 0x400000 with its 0x400 and clears both (at
    `0x0801750A`, `0x0801758A`), where BN4's sets 0x400 alone
    (`0x0801096C`, cleared at `0x080109DE`): the paralysis's flag of
    BN6's own bookkeeping, its legit.json entry's reason extended so.
  - With these, **`walla_v1`, `walla_v2` and `walla_v3` PASS**: 1200 fight
    frames each, the family's records and the world's the same.
- **Elmperor's collision data** (`elmperor_v1`-`elmperor_v3`): its +0x06
  BN4 0xFF against BN6 0, +0x08 BN4 0 against BN6 0xFF at t1, was the
  poison timer at another place: Elmperor's pre-step holds it at 0xFF
  (BN4 `0x080C2870`: `movs r0,#0xff; strb r0,[r3,#6]`), the recipe writing the copy's
  at BN6's +8 (its immediate, `src/emu/xvirus_bn4.c`); the poison
  panel's step, twins (`BN4_POISON_STEP` `0x08013010`, BN6's
  `0x0801A1AA`), counts BN4's +6 and BN6's +8 alike (6 a point, 0 off
  poison). The trace's layout now writes BN4's +6 at BN6's +8
  (`src/dev/xdiff_bn4_trace.c` `coll_segs`, `BN4_COLL_POISON_TIMER` to
  `BN6_COLL_POISON_TIMER`). Its mutation ("Elmperor's poison timer left
  at BN4's offset", `poke=080C2872:7198`, the store at BN4's +6 again)
  adds both lines back at t1 (the spec fails there without it on its
  sprite fields, so those lines are the check). Elmperor's first
  differences are then its sprite fields alone (+0x91, +0x92, +0xAC and
  +0xB0 from t1 to t123, its idle animation's place: the opening's
  sprite updates, the parallel work's); with them passed over by a
  scratch reading, `elmperor_v1` and `elmperor_v3` would PASS and
  `elmperor_v2` would be INCONCLUSIVE at t515 on a sound: BN4's 0x9D
  (Elmperor's attack's, its code's PlaySoundEffect at `0x080DC704`)
  begun on BN4's player 9, where BN6's copy plays BN6's own song 0x9D
  (the families' sounds still BN4's numbers) on BN6's player 0, which
  still holds the Custom gauge's 0x8F begun at t513 (BN4's 0x81 on its
  player 20), so BN6 begins none.
- **The families' copied sounds** (4 November 2026): the families' 12
  sounds BN6 has no song of now play as BN4's own, each a copy at its
  slot of BN6's empty songs on BN4's player ([Battle Network 4 in a
  run](../multirom/bn4.md), Its sounds). BN6's trace names such a copy
  by its slot (`src/dev/xdiff_events.c` `xd_song_copy`, which names
  BN5's copies alone by their own numbers), so a copy aimed back at
  BN6's own song of BN4's number is told apart; `legit.json`'s `bn4`
  `sounds` pairs each BN4 sound with its slot by the routines that play
  it (BN4's `movs r0,#N` and its bl, the copy's `movs r0,#slot`, the
  recipe's immediate; a state's clock), as the thunder's are. A frame's
  sounds line is the family's where a sound it parts on was played by
  the family's code in either game (a sound event its code led to in
  the frame before, as a frame's sounds begun are those its trace's
  frame before began: `tools/xdiff_bn4.py` `family_sounds`), failing
  the spec; the rest stay the world's. In the specs' traces every copy
  is begun on BN4's player at BN4's frame (the lag of 1),
  `elmperor_v2`'s 0x9D at t515 on player 9 beside the gauge's 0x8F on
  player 0; no sound line is left in Shrimpy's, Walla's, Elmperor's or
  CirKill's v1-v3 but `cirkill_v3`'s 0x82 against 0x84 at t1107 (BN4's
  player 8, BN6's 30, played by no family's code), and Spikey's begin
  at other frames than BN4's, its fight parting from t1. Its mutation
  ("a copied sound aimed at BN6's own song of its number",
  `poke=080BEAE8:20B7`, Walla's 0xB7 named again: BN6 begins its own
  0xB7 on its player 18) is caught at t3.

- **CirKill's direction** (`cirkill_v1`-`cirkill_v3`: CirKill V1-V3,
  BN4's 0x5B, 0x5D, 0x5F, at (5,2) against MegaMan idle at (2,2) with
  2000 HP, as Billy's specs else): `cirkill_v1` parted at t43 on its
  collision data's +0xC, the direction of its last move (BN4's +0xA),
  BN4's 4 against BN6's 3 (`v2` at t35, `v3` at t29, the same field).
  CirKill walks its ring a panel at a time, and its move's end (BN4
  `0x080BD34E`, in its state 7's move) writes the direction from its old
  panel (+0x74) to its new (+0x14) through BN4's `0x0800AFB0`, which the
  recipe copies (`tools/bn4_twins.py` COPIED: BN6 has no routine that
  answers every panel) with its rule (`BN4_DIRECTION`, `0x0800AF90`):
  right 4 and left 3 by the screen. BN6's rule (`BN6_DIRECTION`,
  `0x0800E994`) numbers them by the object's side (r4, its +0x16: right
  4 and left 3 on side 0, 3 and 4 on side 1), as its collision panels'
  update passes it (`0x0801A05C`) and BN6's code reads the field
  (`0x0800E498`, by +0xC from `0x0800E468`), so the copy wrote BN4's 3
  for CirKill's step to the left on side 1, where BN6's own rule gives
  4: the port's, a number in BN4's terms in BN6's field (the comparator
  reads BN4's by the side, `direction_as_bn6`). The recipe now aims each
  call of a family's code to the rule at a shim of its home that calls
  the rule's copy and swaps its 3 and 4 where the object's side is 1
  (`bn4_twins.SIDED`, `XVirusFamily.sides`, `src/emu/xvirus_shim.h`'s
  direction), so its moves keep BN4's answer for every panel in BN6's
  numbers. Its mutation (`tests/xdiff/mutations.json`, "CirKill's
  direction left in BN4's numbers": the copy's call aimed at the rule's
  copy again) fails `cirkill_v1` at t43 on that field.
  - `cirkill_v3` was then INCONCLUSIVE at t1107 on a sound, BN4's 0x82
    against BN6's 0x84: MegaMan's HP low, played by the HP gauge's step
    in each game (`BN4_HP_LOW_STEP`, `0x0801472C`; `BN6_HP_LOW_STEP`,
    `0x0801C840`), twins from the HP test on (a quarter of his maximum or
    less counts 45 frames, then the sound), its count started by the hit
    that took him there (t1062 in BN4, t1063 in BN6): the state's clock,
    mapped as BN6's number (`legit.json`'s `bn4` `sounds`, "MegaMan's HP
    low").
  - With these, **`cirkill_v1`, `cirkill_v2` and `cirkill_v3` PASS**: 1200
    fight frames each, the family's records and the world's the same.
- **VolGear's readings** (`volgear_v1`-`volgear_v3`: VolGear V1-V3, BN4's
  0x07, 0x09 and 0x0B, BN6's 0x2F0, 0x2F2 and 0x2F4, 9 November 2026;
  [Transplanted families](../multirom/families.md), BN4's late families):
  the comparator reads a sprite in the late families' art room as one in
  the families' (`tools/xdiff_bn4.py` `XV_LATE_ART`, its rows'
  `XV_LATE_ART + n`). Two firsts settled in the generator, never in the
  comparison: the install refused its NameID range's call while BN4's
  look-up, lying in its region, was walked as its own; then `volgear_v1`
  failed at t214 on its flame's +0x4C (BN4's MegaMan, BN6's 0) and +0x64,
  its aim read at BN4's players' +0x44 in BN6's battle state ([BN4 Red
  Sun's tables](../rom/bn4.md), VolGear moved). Each PASSes over 1200
  fight frames, its flame (the third kind's 0x1A at the dispatch's 0xDE)
  from t214, no allowance added. Its four mutations (tests/xdiff/mutations.json):
  the aim's base left at +0x44 (t214), the flame's kinds' table byte left
  at BN4's list 4 0x0A (t214, its sprite state), the flame spawned as its
  0x19's number (t214, the run order) and its list 1 entry aimed at
  Billy's copy (t1). Not reached: its fire on a list of panels (the third
  kind's 0x19 at 0xDF, from its state `0x080B66E3`'s step at
  `0x080B67A0`), with MegaMan idle in row 2 or row 1 over 1200 to 3600
  frames, so its sprite's immediate (`0x080CFFAE`) is unproved.
- **Kilby's readings** (`kilby_v1`-`kilby_v3`: Kilby V1-V3, BN4's 0x43,
  0x45 and 0x47, BN6's 0x2F6, 0x2F8 and 0x2FA, 9 November 2026;
  [Transplanted families](../multirom/families.md), BN4's late families):
  the first family whose code lies in the late families' code room
  (`XV_LATE_CODE`), which the comparator counts as the families' code
  (`src/dev/xdiff_events.c`'s keys, `tools/xdiff_snap.py` `XV_LATE_CODE`:
  moved code, and none of a BN5 reference's), and the first family sound
  in the per-battle pool: `kilby_v1` failed at t163 on its sound (BN4's
  0xCD, BN6's 0x32) until `legit.json`'s `bn4` `sounds` named the pool's
  slot, a row whose `foes` are its ids 0x43-0x48, so the slot is read as
  its own in its battles alone (another owner's songs fill it in others').
  Each then PASSes over 1200 fight frames, its effects (0x8F, and Spikey's
  0x92 and 0x93) from t129, no allowance added. Its four mutations
  (tests/xdiff/mutations.json): its sound at the pool's second slot
  (t163), its effect's id left at BN4's 0x2D (t129, the objects in play),
  a CurAction left at BN4's (t42) and its list 1 entry aimed at VolGear's
  copy (t1).
- **MagTect's readings** (`magtect_v1`-`magtect_v3`: MagTect V1-V3, BN4's
  0x31, 0x33 and 0x35, BN6's 0x2FC, 0x2FE and 0x300, 9 November 2026;
  [Transplanted families](../multirom/families.md), BN4's late families):
  its recipe whole, each first FAILed at t1 on its +0x36, +0x64 and +0x9C.
  Its update's tail (`0x080BF064`, hook 3) shakes it whenever the battle
  is not paused, the can-move check's third form allows and its +0x20 is
  a multiple of 4, drawing a number for the shake's offsets each time; in
  the opening before the appear's bit 0 that is every frame, and the
  traces' setup draws show BN4's 18 (frames 132-149, the first in its
  init's frame) against BN6's 16 (frames 72-87): BN4's update runs in its
  init's frame and its bit 0 comes a frame later, the two frames the
  opening's alignment and the battle's offset name (The BN4 trace, above).
  The setup's replay (`xd_replay_setup`) gave BN6's k-th setup draw BN4's
  k-th, so BN6's last shake took BN4's sixteenth number where BN4's took
  its eighteenth. **The opening's draws paired from its end**
  (`legit.json`'s `bn4` `opening_draws`): where BN4's opening made more of
  a key's draws than BN6's, `tools/xdiff_bn4.py` `opening_skips` runs the
  battle again with `setupskip=KEY:N`, BN6's k-th setup draw of the key
  given BN4's (k + N)-th (`src/dev/xdiff_events.c` `replay_setup`), BN4's
  first N without a twin, and holds BN6's count the same in both runs
  (else INCONCLUSIVE); the run prints `xdiff: the opening's draws paired
  from its end`. N is bounded, never the gap of the counts alone: it must
  equal BN4's draws of the key in the two unpaused frames BN6's opening has
  no twin of, read from BN4's trace by frame (`twinless_draws`: the family
  object's spawn frame, and the frame before the appear's bit 0, the frame
  before `BN4_APPEAR`'s step 0 plays its sound); any other gap, a draw of
  BN6's dropped or added, pairs nothing (`not paired from its end`) and
  the spec fails where the family's state parts. Its mutation, MagTect's
  opening one shake short (BN6's opening fade begun with 0x12, its copy
  shaking 15 times), is caught at t1 with the pairing refused. Shrimpy's and Spikey's one init draw each, the same count
  in both, keep the k-th pairing. Then FAIL at t143 to t239 on its sounds
  until `legit.json`'s `bn4` `sounds` named the pool's 0x32 and 0x33 for
  its ids, and INCONCLUSIVE at t177 to t273 on MegaMan's collision word
  +0x0C, his pull's direction, which the slide's allowance explains,
  read whole as a pointer while his hit bits at +0x0F were 3
  (0x03000004): `slide_direction` holds such a word to the direction's
  byte, every other byte the same. Each PASSes over 1200 fight frames. Its
  four mutations (tests/xdiff/mutations.json): its 0xF8 at the pool's first
  slot (t239), a CurAction left at BN4's (t39), its shake every other frame
  (t1: its opening's draws more than BN4's, some without a twin, so no
  pairing holds) and its list 1 entry aimed at Kilby's copy (t1).
- **WindMan's readings** (`windman_v1`, 8 November 2026; [BN4's Navis in BN6's
  engine](../multirom/bn4-navis.md), WindMan built): what the comparator
  gained for his battle, each reading or allowance from both games' code
  in `legit.json`'s `bn4`; with them **`windman_v1` PASSES**, 1200 fight
  frames:
  - **The battle state by its place** (`bn4.battle_state`,
    `tools/xdiff_snap.py` `state_off`): a pointer into BN4's battle state
    read as BN6's where a segment maps it, the sides' winds and their levels
    (BN4 +0x88, BN6 +0xC0, 0x10 bytes); every other offset as it is.
  - **A spawner one engine keeps** (`spawner` rows, `spawner_kept`): a word
    of an engine's object where BN6's spawn stores its spawner and BN4's
    twin nothing, only BN4's 0 against a moved Navi's object in BN6.
  - **A native's own art** (`own_art`): a world object each engine runs as
    its own, its sprite state's words into its sprite read by their offset
    from the sprite's start, where both games' last sprite events name the
    same bytes, list and number (`arts_set`).
  - **An engine's own bit** (`allowed_engine`): a bit of a world object's
    byte one engine's code sets (the gust's sprite flag 0x80), by its kind,
    type, offset and direction.
  - **The renderer's ranks** (`bn4_renderer`, `ranks_into`): a hit's spark
    takes none (its life is its own art's), and the world's natives
    (`is_native`: each engine's own third- and fourth-kind objects, not
    moved, not a spark, not BN6's MegaMan's companion) are ranked among
    themselves as the family is among its own.
  - **A slot let go, by its last occupant** (`let_go`, `occupants`): a
    pointer to a slot let go named by the role last in it, as each game
    numbers its slots by what else it holds.
  - **A slide's direction by each engine's clock** (`slide_direction`):
    MegaMan's collision's last move's direction where one side holds 0 and
    the other his last move's, read as BN6's (`MOVES`); with it MegaMan's
    step's kind (+0x0F, BN6's) and a blocked slide's step (+0x1B to +0x1D,
    BN4's) allowed by their directions.
- **FireMan's readings** (`fireman_v1`, 8 November 2026; [BN4's Navis in
  BN6's engine](../multirom/bn4-navis.md), FireMan built), with which
  **`fireman_v1` PASSes**, 1200 fight frames:
  - **A Navi's companion's opening count** (`companion_count`,
    `legit.json`'s `bn4` `companion_opening_count`): a companion his
    recipe moves (a first-kind role of a moved type: his 0x57) counts the
    animation its init set up in the paused opening by the fight's frames,
    as the Navi's own (`opening_count`), its mode's after-step (BN4
    `0x080143FC`, BN6's `0x0801BBAC`) running the sprite update at every
    fight frame and none of the opening's.
  - **The spark out of the ranks**: the same rule ThunderMan's and WindMan's
    battles found (`ranks_into`, `legit.json`'s `bn4` `spark_ranks`): his
    flame's hit's spark drawn on in BN4 where BN6's had ended moved his
    companion's rank at t271. The three branches each left the spark out;
    merged, one rule takes it out of the drawn a group ranks, the family's
    and the natives' alike.
- **SearchMan's readings** (`searchman_v1`, 8 November 2026; [BN4's Navis in
  BN6's engine](../multirom/bn4-navis.md), SearchMan built), with which
  **`searchman_v1` PASSes**, 1200 fight frames:
  - **The invisibility in the BN4 trace**: the collision data's frames
    (BN4's +0x1A, BN6's +0x26) and the hide step's marker (BN4's +0x7C,
    BN6's +0x50) laid out at BN6's places (`src/dev/xdiff_bn4_trace.c`,
    `BN4_COLL_INVISIBLE`, `BN4_COLL_HIDE_MARKER`).
  - **A moved fourth-kind type read back as BN4's** (`tools/xdiff_bn4.py`
    `t4`, called by `tools/xdiff_snap.py` `identity` for an object of the
    fourth kind that is no effect): his scope 0x2E, trails 0x31 and marker
    0x3E run at the dispatch's numbers 0xE0 on and meet BN4's own.
  - **His pool sounds** paired by slot (`legit.json`'s `bn4` sounds, his
    ids alone by `foes`), and one leftover allowed: his trail's begun byte
    (+9), what each game's PlaySoundEffect leaves in r0 (`spawn_leftover`'s
    bytes, BN4 0x28 and BN6 0x48 alone).
- **WoodMan's readings** (`woodman_v1`, 8 November 2026; [BN4's Navis in
  BN6's engine](../multirom/bn4-navis.md), WoodMan built): with them
  **`woodman_v1` PASSES**, 1200 fight frames:
  - **A pointer into a Soul Navi's AI data by its place**
    (`legit.json`'s `bn4` `ai_data`, `tools/xdiff_snap.py` `ai_off`): a
    pointer into BN4's AI data read as BN6's where his port moves the block
    (his own block, BN4 +0x64 at BN6 +0x80; his attack's variables, +0x70 at
    +0xA0), as the trace lays the fields out (`ai_segs`); his step's
    callback state keeps a pointer to his block's +0xB (BN4's +0x6F, BN6's
    +0x8B); every other offset as it is.
  - **His 0x94's spawn's r2** (`spawn_leftover`, its word +0x38): each
    game's camera, which the shake set before its spawner leaves in r2
    (BN4 0x02009B80, BN6 0x02009180); no code of the 0x94 reads it.
  - **His pool sounds** paired by slot in his battles alone (`foes`), his
    0xA2 the pool's second slot over GutsMan's first.
