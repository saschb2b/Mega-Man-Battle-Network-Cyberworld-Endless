---
type: Engine Mechanism
title: "The chips' code"
description: "Issue #82's C2 and C3: BN5's chips proved against BN5's own battle frame by frame, and BN5's code for those BN6 lacks moved in as units (`tools/xchiprecipe.py`, `src/emu/xchips_code.c`), DarkInvs and Program Advances among them."
tags: [multirom, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-22T13:45:57Z }
---

# The chips' code

Issue #82's C2 and C3: a chip of BN5's proven to play as BN5 plays it,
and BN5's own code moved into BN6's engine for the chips BN6 has no code
of. The registry's ids, records and art stay C1's (above).

- **The chips' differential (C2)** (`tests/xdiff/chip_*.json`; docs/
  [The differential: a moved family against its own game](../devtools/differential.md): a chip as the subject): MegaMan is the
  subject; each game gets a folder of the chip (BN5's id and the
  registry's, owned in BN6's pack as its deal asks), the first Custom
  screen's chip picked and OK, the chip used at t30 against BN5's
  CanladEX held paralysed (each game's own paralysis timer and struck
  bit, set each fight frame: BN5's collision data +0x1E, BN6's +0x1C).
  Compared frame by frame: MegaMan's CurAction (a C1 chip's family as
  its BN6 twin's, a moved chip's as its moved number), his attack
  variables and AI data, every object the chip spawns (by each game's
  type, through the dispatch's TWIN and MOVED, and the picture it draws
  by its sprite's bytes where both games' are in their ROMs: the same
  code drawing another picture at the same number is a difference, an
  effect object's picture its effect id's), effects, sounds, draws and
  the target's HP. What differs and is the engine's, not the chip's,
  is explained in `tests/xdiff/legit.json` with both games' routines
  (never on a chip's own records): BN6's chip use ORs into the attack's
  element a hit flag by its icon element (`0x080129E4`: Sword 0x80,
  Cursor 0x40, Wind 0x20, Break 0x10; BN5's has none), carried on by the
  chip's objects; BN6's support family ends its attack a frame later than
  BN5's; BN6's hit-clear keeps a wrong word in a time stop
  (`0x0801A022`); a sword's slash's last frame (each game's own sprite);
  each game's muzzle flashes by its own table (WideSht's flash 0x2A in
  BN5 and 0x59 in BN6 the same picture); the tables a chip's objects index
  by kind (the thrown bombs', the spread shot's, the pucks', the guards');
  NrthWind's gusts drawn by a routine the same in both (a twin of the
  replay). [The chip registry and BN5's chips' code](../rom/chips-code.md), BN5's chips' code in BN6's engine, has
  each with its addresses.
- **Its verdicts**: BN6's own chips in both games (Cannon, Sword,
  MiniBomb, AreaGrab) PASS, each game drawing its own chip with its own
  pictures (BN6 redrew MegaMan's sword, list 3's 0, and his bomb's hand,
  list 3's 2; there a sprite is compared by its frames' layout); of the
  C1 plays with BN6's code under BN5's records, WideSht1-3, Spreader,
  AirHoc and Geddon1-3 PASS, their pictures the same bytes (each
  difference the engine's: WideSht's wave handed another spawn word,
  Spreader's shot another collision region, AirHocky's wait counted
  from another frame to the same end, Geddon's subfamily BN6's). C2
  found BN6's DrilArm family (0x2E) winding up 9 frames longer than
  BN5's 0x4F and BN6's Z Saver family (0x5B) slashing a frame sooner with
  a collision flag and an AI field BN5's has not: decided (below), they
  now run BN5's own code and PASS.
- **The design calls, decided** (the owner's; issue #82): BN5's chips
  behave as BN5's. DrilArm1-3 and Z Saver are move-class (BN5's own
  code, moved: they sat out until it was, the registry's
  `XREG_PENDING`, now `XREG_MOVED`). BlakWing's and HolyDrem's dark gates
  (their records' +0x15: BN5's chip use lets BlakWing out only to a dark
  MegaMan, the dark meter under 470, and HolyDrem only to a light one)
  are BN5's own rule and came with the mood port (the Souls track's S7):
  both play, their units moved (Their rules in place, below). The
  plus chips stay BN6's rule (one chip per name: Attck+10, Navi+20 and
  Attck+30 are BN6's own ids, merged into the chip before them as BN5's
  merges its own, a puff used alone), but DarkPlus is BN5's: used alone,
  +50 to the next chip (BN5's misc family 0x1A's subfamily 3,
  `0x0800E190`): done (Their rules in place, below).
- **The units (C3)**: a unit is one of BN5's attack families (the
  routine a chip's CurAction runs) or one of its support or Navi chips'
  routines (by subfamily), with every routine and object it reaches that
  BN6 has no twin of. `tools/xchiprecipe.py` derives each from both ROMs
  on the families' generator ([The chip recipe generator](../devtools/chip-recipe-generator.md), The chip recipe
  generator) and writes `src/emu/xchips_bn5.c` (numbers only) and
  `src/core/xchips_moved.c` (each moved chip's id, family and
  subfamily); `src/emu/xchips_code.c` installs them on xvirus's
  machinery as the core starts, before the registry's records, all or
  nothing. A twin of an object's code counts only where it is the same
  code to its depth and draws the same pictures: the sprites both load
  by number must be each game's of the same bytes (BN6's own T3 0x45 is
  MetaGel's gel drawing another sprite, its animation 14 frames longer;
  BN6's T3 0x18, CrakOut's crack, and T4 0x3A, NrthWind's gust, draw
  others at the numbers BN5's do: the differential's runs found them);
  an object BN6's twin would spawn of a type BN6 runs other code for
  moves with its spawner; the hit area and the effect object are BN6's
  own, audited. Each object type a unit spawns through the second-level
  dispatch has one decision for every track, moved or twin, written in
  `tools/bn5_twins.py` (`TYPES`; the generator fails on a type it spawns
  that has none or another), and a moved type one copy, the first
  track's to install it (`src/emu/xinstall.c`: the moved objects and
  the Navis, then the chips' code, then the families): a unit spawns the
  Navis' copy of their attacks' objects (the Navi chips' NumberMan,
  KnightMan and ToadMan attacks run the Navis' own; a unit's call into
  that code, ToadMan's note spawner, goes to the Navis' copy too, one
  copy run by both) and of the moved objects', and the families spawn
  the chips' of the types both move
  (SideBub's water and bubbles Marina's, the crack bombs' Bugtank's,
  BoyBomb's bomb BomBoy's, the misc family's meteor MetFire's fire
  column, Pulsar's Batty's, the wide shot Lark's: each kind's row the
  chips' copy fixes).
- **What a unit's code is given**:
  - *Sprites*: one BN6 keeps another of at the number BN5's code loads is
    copied from BN5's ROM into a number of the chips' in BN6's grown list
    (lists 1-5, the chips' from 0xFF down, the families' from the bottom
    up, list 2's 0x20-0x2F the Navis'; list 3, the attacks', grown once
    for every track, `XV_LIST3`), the load set to it: by its immediate,
    the packed word of either packed setup (`0x0800F198`, `0x08018926`), or
    a kind table's row (`TABLE_SPRITES`: the bubbles', the obstacles', the
    shockwave's, the shield's, the geyser's); MegaMan's own sprite (the
    swords' afterimage, BN5's family 0x49's) is each game's own.
  - *Effects*: one whose entry differs takes BN6's of the same sprite
    bytes, animation and palette, else an id of the chips' (0xD0-0xFF,
    from the top down) whose entry names BN5's sprite, copied; by its
    immediate (a number added to a value shifted past its byte too:
    Slasher's), a byte table's row (`TABLE_EFFECTS`: Z Saver's, CustSwrd's,
    Katana's, ElcReel's, the crack bombs', the seeds', the spread shot's),
    an id handed to the engine's routine that spawns effects over a
    pattern of panels (`EFFECT_CALLS`, `0x08018838`), or a run of them by
    an object's kind (`EFFECT_RANGES`: Colonel's soldiers', base + kind,
    one run in BN6's ids).
  - *Sounds*: a song BN6 has another of at BN5's number is copied by
    `src/audio/xsong.c` (`xsong_install_at`) into one of BN6's empty song
    slots, the one fixed for that sound whichever track copies it (the
    shared allocations: 0xBC 0x55, 0xB7 0x61, 0x6F 0x54, 0xE4 0x53, 0x70
    0x5F, 0xBA 0x60, 0xC0 0x62; their second amendment: 0x105 0x59, 0x115
    0x5A, 0x117 0x5E, 0x119 0x4D, 0x8E 0x4C, the fixed ones from 0x40 up,
    the per-run themes 0x26-0x3F; the chips' own from 0x52 down: 0x12E 0x52,
    0xA6 0x51, 0xC6 0x50, 0xBD 0x4F, 0x10E 0x4E, 0xE1 0x4B, 0x4A next),
    taken only where the slot still holds BN6's empty header; the
    immediates name the slot, a table's row its bytes (`TABLE_SOUNDS`: the
    Anti chips' counter's, the seeds', the crack bombs', the spread
    shot's, DrkSonic's sonic's).
  - *Panels*: BN5's lava, sea and holy (8, 0xA, 9) are BN6's 13, 14 and 5
    (src/emu/xpanels.c brings lava and sea in), set or compared by
    immediate or a table's row (`TABLE_PANELS`: the panel changer's kinds,
    the seeds'; `PANEL_LOOKUPS`: ElemRage's element by MegaMan's panel);
    BN5's magnet panel (5), which BN6 lacks, is a type no panel has (0xFE)
    where a unit compares with it and left where no moved unit reads its
    row. A panel's fields past its first 0x10 bytes are read at BN6's
    place (the flags, BN5 +0x18, BN6 +0x14: legit.json's panel layout).
  - *Collision data and MegaMan's points*: an offset into an object's
    collision data is BN6's (legit.json's layout), the immediates of loads
    and stores through it and a number added to its address
    (`COLL_SUMS`: the Anti trap's damage by element, BN5 +0x7C, BN6
    +0x84). A point of MegaMan's sprite a unit asks for (`0x08015140`'s
    r1, `0x08015164`'s r2) is BN6's of the same number where his rows
    give the same, else BN6's that `POINTS` names (CusVolt's 0x1A as BN6's
    0x1B, the centre, 2 pixels further right as every point of his), else
    a stub answering BN5's numbers (`UNIT_STUBS`: ElemRage's 0x14, (-1,
    26) in every row of BN5's, none of BN6's).
  - *Helpers*: BN5's code several units call whose object one copy alone
    may move (the dispatch moves a type once) is installed once before
    the units, at `XC_HELPERS`, and the units' calls aimed at it: the
    buster arm (`0x080EBB96`, BN5's plain buster's too), the shake
    (`0x080D3286`, the third kind's 0x68: SpShake's and ShakPar's), and the
    computed powers' routines (below). The muzzle flash (MegaMan's arm as
    a chip takes it: its spawner `0x080B9CAE`, the first kind's 0x05, its
    58-entry table at `0x080B9A5C`) is BN5's image object, moved once for
    every track with the Navis' install (The engine's objects moved once):
    a unit's calls into its module go to that copy, and the entries the
    units name are fixed there (`tools/bn5_objects.py`: BN6's twin, or
    BN5's sprite copied at the Navis' place or a number of the chips' in
    list 3). The differential counts no helper's calls as a unit's.
  - *Computed powers*: a record's power of 1000 and up is an index into
    each game's table of routines that compute it (BN6's 45 at
    `0x080109DC`, BN5's 76 at `0x0800E728`). BN6's is grown once to 256 at
    `XC_POWERS` (its word `0x080109D4` pointed there), its own 45 kept,
    BN5's entry i at 0x80 + i where a helper moved its routine
    (CustSwrd's 1045 and CusVolt1-3's 1073-1075 by the Custom gauge, the
    Navi chips' SP by BN5's fixed link powers, `UNIT_STUBS` answering the
    link bit 1 as BN6 keeps no deletion times of BN5's Navis, and DS by the
    side's battle counter 3), the rest a stub (power 0); a playing chip's record
    takes BN6's entry (`xchips_power_of`), still computed and shown.
  - *Subfamilies a unit plays*: a family's table of routines by
    subfamily (`SUB_TABLES`: the bombs', the misc family's) keeps the
    entries of the subfamilies its moved rows play (moved, Program
    Advances and the DarkChips of `DARK_MOVED`, waiting for their rules);
    the rest, BN6's own chips' (`XREG_SAME`, run by BN6's code), aimed at
    a stub and not walked.
  - *Another family started*: a chip whose code starts another of BN5's
    families (Slasher's `setAttack0(0x49)`, the player's 0x80000 reaction)
    gives the immediate the started family's moved number (0x99).
  - *Stubs*: a panel claim's spawners (`0x080EA08C`, `0x080EA0BA`) answer
    r0 0. Proved inert in every battle a run fights: they spawn the
    fourth kind's 0x83 (the one object that uses BN5's panel-claim pool,
    `0x0802E920`-`0x0802EAB4`, and its RAM) only where the battle's bit
    0x40 is set (`0x080090E8`, BattleState +0x32); the moved spawner's
    gate is BN6's twin (`0x0800A8F8`), BN6's bit set only by `0x0802E112`
    for a link battle (the link's state `0x0200AD04`, or the Beast Link
    Gate's event flag 0x1722), as BN5 sets its own only at `0x0802D594`.
    No RAM is taken for the pool. Every stub returns by `mov pc, lr`, BN6's
    own idiom: the readers of the support, Navi and computed powers' tables
    call through `mov lr, pc` (BN6 `0x080ED100`, `0x080E2BE6`,
    `0x08017B1E`, `0x080109CA`), a return address without the Thumb bit,
    which a `bx lr` would go on from in ARM state (the differential's
    mutation of a computed power found it: BN6's Custom screen never
    opened).
- **The numbers, one table for every track**: BN6's attack families
  grown once to 240 entries, CurAction 0x10-0xFF (`src/emu/xattack_grow.c`:
  BN6's 0x10-0x5E, the chips' 0x60-0xBF, the Souls' 0xC0-0xCF, the rest
  spare, every entry not given a stub that ends the attack); BN5's
  family F is BN6's 0x50 + F. BN6's support and Navi chips' routines
  grown to 256 entries each, BN6's own first; BN5's support subfamily S
  is BN6's 0x2A + S, its Navi subfamily S BN6's 0x1D + S. Fixed by BN5's
  numbers, the same at every boot and in every build.
- **Moved** (94 chips, each family's spec PASS, its objects' pictures
  compared by their bytes, the registry's `XREG_MOVED`, playing where
  BN5's ROM is read): CrakOut, DublCrak, TripCrak (BN5's family 0x1B as
  0x6B: its crack moved with its sprite), HotBody1-3 (0x58 as 0xA8),
  Jealousy and NrthWind (support 0x13 and 0x15 as 0x3D and 0x3F),
  MetaGel (support 0x06 as 0x30: its gel's sprite into list 3, sound 0x105
  at slot 0x59), Blinder (0x08 as 0x32, sound 0xBC at 0x55), Skully1-3
  (0x55 as 0xA5, sound 0xB7 at 0x61), DrilArm1-3 (0x4F as 0x9F: its flash
  0x2F's drill arm copied), RainyDay (support 0x3D as 0x67: its effects
  0x61 and 0x29, its sea's compare BN6's 14), WoodNos1-3 (0x28), CactBal1-3
  (0x48), SpShake1-3 (0x51), Pulsar1-3 (0x52), each through the buster
  arm, MrkCan1-3 (0x4E: its mark's sprite by the second packed setup, its
  flashes 0x2C-0x2E, sound 0xBD), SideBub1-3 (0x1C: the bubbles' kind
  table), AqWhirl1-3 (0x47: its effect 0x4F, sound 0x70 at 0x5F, its sea
  BN6's 14, its claim spawner stubbed), Z Saver (0x34 as 0x84: the swords'
  reach `0x08012530` copied, its flashes and slashes' effects BN5's),
  Katana1-3 (0x54), ElcReel1-3 (0x23), BoyBomb1-3 (support 0x3A as 0x64),
  Guard1-3 and CrsShld1-3 (0x1F as 0x6F), MoonBld1-3 (0x53 as 0xA3),
  CusVolt1-3 (0x27 as 0x77, their powers computed), CustSwrd (BN5's sword
  family 0x13 as 0x63: BN6's own runs its subfamily 7 with its effect
  0x28, a spark, where BN5's is CustSwrd's slash), the Anti chips
  (support 0x0C as 0x36: a foe's strike made Fire or Elec in both games'
  copies springs the trap, the counter's sound 0x12E at 0x52), Slasher
  (0x29 as 0x79, and BN5's 0x49 as 0x99, the family it starts: a foe on
  MegaMan's side of the field), ElemRage (0x56 as 0xA6: on grass Wood, on
  lava Fire), the bombs (0x12 as 0x62: Quake1-3, CrakBom, ParaBom,
  ResetBom, LavaSeed, CannBall, Geyser, SeaSeed, SeaSeed's sound 0xE1 at
  0x4B) and the misc family (0x1A as 0x6A: RedFrut1-3, Voltz1-3,
  Astroid1-3, WavePit on the sea, RedWave on lava, MudWave, whose waves
  rise from holes, which no field of BN5's has: its use alone compared),
  and the Navi chips NumbrMan, ShadoMan, TmhwkMan, KnightMn and ToadMan
  (their V1s: BN5's Navi routines 0x00, 0x0B, 0x0C, 0x05 and 0x0D as
  0x1D, 0x28, 0x29, 0x22 and 0x2A, each drawing its Navi's own battle
  sprite, the Navis' list 2's `0x20` + AI - 7, and running the Navis'
  copies of their attacks' objects: NumberMan's dice, KnightMan's
  wrecking ball, ToadMan's notes; KnightMan's companion begun and ended
  by BN6's AI index for him, 0x24, `NAVI_AI_CALLS`, and his object's own
  states kept BN5's numbers, `OWN_STATES`). Skully's spec holds a
  recorded FAIL (`verdict`): the engine's status mark over the foe (the
  fourth kind's 0x06, the same code in both) drawn from each game's own
  list 5's 0x0B; `--survey`: nothing else differs. Blinder's recorded
  FAIL (its blind mark at CanladEX's arm point) no longer holds since the
  virus families bring their own arm points: it passes. Mutations (`--mutations`)
  break each kind of number a unit is given (a record's family or
  subfamily, a grown entry, a number in its code, an object's type, a
  sound and its slot, an effect's id, a copied sprite's row in the
  flash's and a kind table) and each fails its spec.
- **What the differential found, and why** (each its cause, the fix the
  generator's and the cause in `tools/xchiprecipe.py`'s tables):
  - The one copy of a type two tracks spawn: BomBoy's specs ran the
    chips' copy of the bomb (the third kind's 0x3E) once the chips' code
    installed first, and at a paralysis (bomboy_v1_keys) its states mixed
    two numberings: BN5's own actions table by the obstacle runner, whose
    generic states BN6's twins number as BN6's. The bomb's update is now
    BN6's twin (`FORCE_TWIN`: BN6's table, as the families' copy had it),
    its CurAction read as BN6's (legit.json, t3_types); a family's spec
    counts the chips' copy it runs as its own code (`xd_log_shared`).
    KnightMn's object froze BN6's battle (its own states' table by +9 read
    at 0xE: `OWN_STATES`) and summoned another AI's companion (BN6's AI
    index for him: `NAVI_AI_CALLS`); the SP and DS powers' routines read
    their records' powers less BN5's bases (`POWER_BASES`).
  - BoyBomb: moved BN6's battle stood still once its thrower's time stop
    ended. Its bomb's actions are a table of routines BN5's code hands to
    the obstacle's state runner (`0x08017FCC`), which the deriver cut at
    its first entry far from it: the rest BN5's addresses in BN6's ROM.
    Now a table handed in a register is walked whole (`TABLE_ARGS`), and a
    table whose next word is a routine too is a decision.
  - Guard: the shield's collision data recorded the reticle's strike with
    bit 0x80 more in BN6's: the comparator's own reading of an alliance
    byte dropped the rest bits (`tools/xdiff_snap.py`, alliance_or), not
    the chip's. CrsShld's shield kept another word at +0x64: BN6's own
    shield (the third kind's 0x2B, the same code) reads its kinds from its
    own table (BN6 `0x080CAED4`), BN5's (`0x080CA2D8`) another sprite at
    CrsShld3's row: moved with BN5's table (`FORCE_MOVED`, `FORCE_COPY`).
  - MoonBld: the foe's AI data apart from the blade's hit on, and 1 HP: the
    hit's stat effect 0x18 is each game's own rule (BN6's virus pre-step
    `0x08013B20` adds the parameter to the AI data's +0x12, BN5 none): the
    sweep copied, its hit's parameter given in BN6's terms (`HIT_STATS`);
    the blade's register its spawn keeps from the calls before it (+0x3C,
    its first frame alone, read by nothing) a mask.
  - CusVolt: the volt a panel ahead in BN6's: its place by MegaMan's
    point 0x1A, another point in BN6's rows (`POINTS`).
  - The Anti chips: no counter in BN6's: the trap reads the struck's
    damage by element at BN5's place in the collision data (`COLL_SUMS`);
    its counter's explosion spawner differs below (moved).
  - DrkSonic: its sonic's time read from a table its BN6 twin keeps
    otherwise (BN5's 60, 60, 60, 60, 120 by kind, BN6's 170, 60, 170, 30),
    the same code: a twin now also compares the data its literals name
    (`same_data`), which moved that phase and found the explosion's and
    MrkCan's, CannBall's and Astroid's tables (each audited or moved).
  - Geyser, CannBall: each game's own spark sprite (list 5's 5) lives
    shorter in BN6's (masks).
- **Their rules in place** (issue #82's final engine round; their code
  moved and installed, `xchips_moved.c` naming their moved family and
  subfamily so their records take it):
  - The Program Advances InfVulc1-3 (BN5's Vulcan family 0x19 as 0x69),
    SuprSpr1-3 (the spread family 0x25 as 0x75), Boxer1-3, ShakPar1-3
    and CacDanc1-3 (the misc family's subfamilies 0x15-0x17) are formed
    on BN6's Custom screen: BN6's list of recipes grown with BN5's
    (`src/core/xchips_pas.c`, written by `tools/xchiprecipe.py` from
    BN5's list `0x08028020` in BN6's ids; [The chip registry and BN5's chips' code](../rom/chips-code.md), BN5's Program
    Advances in BN6's lists), BN6's 43 first, so BN6's recipe wins where
    two are the same, the P.A. screen drawing every part's code letter.
    Their rows sit out of the deck as BN6's own Program Advances do (a PA
    is formed, never dealt). `pa_infvulc1`, `pa_suprspr1`, `pa_shakpar1`
    and `pa_cacdanc1` PASS: the same three picks form it in both games,
    the P.A. screen shown, used at t30 and played by its moved code frame
    for frame to the foe's deletion. Never formed in BN6's, and so they
    stay (the decision of issue #82's last integration round: BN6's codes
    and recipes come first, no rule of ours replaces BN5's; kept as BN5's
    recipes, `PA_UNFORMED`): InfVulc2 and InfVulc3, whose parts' codes in
    BN6's records (Vulcan2 D, F, L; Vulcan3 A, G, R) never make three in
    a row, and Boxer1-3, whose three FireHit are BN6's own FlmHook1-3's
    recipes, first in the list. Their rows stay registered (a save that
    names one reads the same chip), unformed. The Library's P.A. tab lists BN6's own alone (its
    builder bound to 349: the registry's 350 would land before its
    buffer); BN5's in BN6's Library are M9's.
  - The DarkChips DarkCirc (support 0x2A as 0x54), DrkSonic (support
    0x1B as 0x45), DarkWide (the spread family's subfamily 2), DarkMetr
    (the misc family's 0x11), DrkLance (over its twin's family), DarkDril
    (BN5's drill, 0x4F as 0x9F), DarkTorn (BN5's Tornado family 0x22 as
    0x72, its flash 0x17), BlakWing (support 0x20 as 0x4A) and DarkPlus
    play by BN5's dark rule with the Souls installed (S7: the mood, the
    gate, the use, each one's after-effect on his battle NaviStats
    `0x0802C938`, his dark palette): their specs PASS, their recorded
    verdicts gone, BlakWing's dark in both (BN5's meter under 470, BN6's
    latch). DarkPlus is BN5's: the registry's DarkPlus is BN6's own 290
    by name, whose record takes BN5's family and parameters where BN5's
    ROM is read (`SAME_MOVED`), so it stores its power, +50, in the
    hand's next attack chip and flashes him green 24 frames (the flash
    kept apart from his AIData +0x3C, BN6's resting height; [The chip registry and BN5's chips' code](../rom/chips-code.md),
    BN5's DarkPlus and its green flash), a puff with no attack after it
    (`chip_darkplus`, `chip_darkplus_merge` with Atk+10, `chip_darkplus_puff`).
  - The Navi chips' SP and DS (NumbrMan, ShadoMan, TmhwkMan, KnightMn
    and ToadMan's SP and DS, ColonelDS) play: an SP's power is BN5's
    computed one by its best deletion time, which BN6 keeps none of for
    BN5's Navis (`XC_SP_TIMES`, a table of none: BN5's power for a save
    without one, 40 for NumbrMn[SP]: `chip_numbrmnsp`, `chip_knigtmnsp`);
    a DS is dealt as any chip and BN5's dark gate lets it out to a dark
    MegaMan alone (refused, the chip of nothing in both: `chip_numbrmnds`,
    `chip_colonelds`; dark, NumberMan's and Colonel's attacks in both:
    `chip_numbrmnds_dark`, `chip_colonelds_dark`). ColonelDS's object
    calls the companion's triple with its third table, the same as the
    second in both games, which the Navis grow in BN6 (`NAVI_TRIPLES`).
  - HolyDrem (support 0x23 as 0x4D: its double of MegaMan drawn from
    his own sprite in each game, its shots from BN5's point 0x1D of his,
    answered with BN5's numbers), Poltrgst (support 0x31 as 0x5B: the
    obstacles it throws by its own table of kinds, MegaMan's double as
    HolyDrem's) and the Tornado family move whole: `chip_holydrem`,
    `chip_poltrgst` (a RockCube to throw), `chip_darktorn` PASS. A double
    of MegaMan is set up through his form's overlay entry with the double
    as its object: with a Soul united BN5 runs the Soul's own entry for
    it (`0x0800F01C`), where the proxy form's gave it HeatCross's overlay
    (the first kind's 0x56): `src/emu/soullife.c` routes it to the
    Soul's (`soul_knight_holydrem`: HolyDrem used in KnightSoul, FAIL at
    t624 before, PASS after).
- **DarkInvs, moved** (13 October 2026; the owner's call of that day:
  BN5's rule wherever its ROM is read). BN5's DarkInvs puts MegaMan under
  its chip AI for the player for 600 frames, invisible, the game driving
  him; BN6 dropped that AI (its twin of BN5's idle states, `0x080F1866`,
  has no path to it). Now, where BN5's ROM is read:
  - *The machine*: BN5's chip AI for the player (`0x0802B4B0`-`0x0802B804`
    and its own side's heads) is copied with the Dark Soul's chip AI,
    whose routines it shares (`tools/bn5_navi_twins.py`: COPIED,
    EXTRA_ROOTS, and two CURACTIONS, its Navis' buster `0x0802B62C` and
    its dead state `0x0802B6D2`): the Dark Soul's code 14 blocks, 8856
    bytes. `src/emu/darkinvs.c` runs it from BN6's idle state past the
    chip request's test (`0x080F1870`) while MegaMan's AI data's +0x48
    holds BN5's flag 0x200, through a stub (`FREE_CHIP_AI`) that leaves
    the state as BN5's does, and clears its state on every other idle
    frame as BN5's does (`0x0802C03E`).
  - *His AI data*: BN5's is 0x140 bytes, BN6's 0x100 with the next
    slot's after it, and BN6's +0xF0-+0xFF and +0x36 are its own (+0x36
    a drain of 1 HP a frame, `0x08014498`). Where BN5's ROM is read his
    AI data is moved into BN5's 0x140 bytes as he is set up
    (`RAM_PLAYER_AI`, `0x080172F6`, before anything of his names it: a
    move at DarkInvs's use left his charge glow's object, the fourth
    kind's 8, pointing at the old place), and BN6's drain is held back
    while the chip AI has him. In a NetBattle the other MegaMen (side
    1's player; two a side, the partners) each move into a room of
    BN5's size of his own as the chip AI first takes him, in the last
    entries of BN6's AI data's pool (issue #182: side 1's player had
    moved into side 0's block, both reading one AI data;
    `src/emu/darkinvs_room.h`, [Memory the engine writes](../engine/emulation/memory.md#bn6s-ram), BN6's RAM); the
    machine, its time and its idle frames run for whoever's AI data is
    of BN5's size, either side.
  - *The unit* (`tools/xchiprecipe.py` UNITS: BN5's support 0x3B as
    BN6's 0x65, 3 blocks, 860 bytes; its object, the fourth kind's 0x18,
    and his double, the first kind's 0x09, moved): BN5's calls with no
    twin go to three of the chips' stubs (`XC_STUB_DARKINVS`, UNIT_STUBS)
    that darkinvs.c hooks: the chip AI's state cleared (`0x0802C03E`),
    entered with its time (`0x0802C114`: AIData +0x36 600, +0xF0 1, the
    flag; his side's list from his folder, as a failed Chaos Unison's),
    his statuses cleared at BN6's places of the collision data
    (`0x08017060`); his calm (`0x08011A8C`) is BN6's twin `0x080143A6`
    (FORCE_TWIN; soulmood.c's keep_dark holds its mood back while he is
    dark, BN5's mood calmed with the statuses' stub); his double loads
    his own sprite (MEGAMAN_LOADS). SAME_MOVED makes BN6's record 289
    BN5's chip where BN5's ROM is read, as DarkPlus's 290, and the run's
    side follows it (`darkbn6_bn5_code`): BN5's worried rule deals it, it
    makes him dark, its art BN5's (src/core/chips.c), its flame's words
    BN5's rule's and what it does ("For ten seconds,the darkness takes
    over!"); without BN5's ROM it is BN6's own as before.
  - *Its frame*: BN6's support family ends its attack a frame after
    BN5's (legit.json's "BN6's support family's extra frame"), which put
    the machine's first run, and every draw after, a frame behind BN5's:
    for DarkInvs's subfamily the attack ends in BN5's frame (a hook at
    `0x080ED12A`).
  - *Its end*: BN5 counts the time down at the player's frame's end
    (`0x0800DBD8`), BN6 nowhere: a hook at BN6's (`0x0801AD10`) does, at 0
    his request 0x80000, which BN6 answers with its own family 0x49
    (another): where the chip AI has him, BN5's family 0x49 starts instead
    (`0x080EBA9C`), BN5's code moved as BN6's 0x99 (the chips'
    Reaction49), which clears the flag and the invisibility, its
    afterimage and its sound as BN5's.
  - *In a Chaos Unison*: ShadowSoul's chaos attack is BN5's DarkInvs, the
    machine firing the Navis' buster, whose arm reads his Chaos Unison
    from BN5's AIData +0x12 (`0x080EBBFE`): answered from the Souls'
    block (`BN6_SOUL_CHAOS`), the arm's flash BN5's chaos one.
  - *The proof*: `chip_darkinvs` (DarkInvs used at t30, the machine from
    t34 in both, Cannons and the Navis' buster, walking, to t355 before
    the foe falls), `chip_darkinvs_end` (its time made 240 in each game's
    copy, `each`: the end family at t297, MegaMan his own from t321, to
    t420) and `chaos_shadow_45` (its recorded FAIL at t107 gone: PASS to
    t500) PASS; `chaos_shadow_60` as before. The comparator learnt BN5's
    0x16 as the Navis' 0x66 under the chip AI and BN5's 0x49 as 0x99
    (maps, action), a moved type drawing a sprite both games name by one
    pair as each game's own (tools/xdiff_snap.py: his double's and the
    afterimage's MegaMan), DarkInvs's object's chip at +0x30 (maps,
    chip_id) and, with rng=follow, the emotion window's glitch's draw a
    frame apart (masks: "the emotion window's glitch's draw",
    tools/xdiff_masks.py). Mutations: the time a frame short, the end a
    frame long (FAIL). In play (`tools/play.py`, `--net-biome x0 --dev
    worried,god`, `chip 289 * 1`): DarkInvs dealt worried, its name over
    the frozen field, MegaMan grey and untouched, the game firing his
    buster and throwing his MiniBomb, the Mettaur deleted, max HP 140 to
    120 once, the next battle his own.
  - *Its exits*: BN5 takes him out of the chip AI (`0x0802C12A`: +0x36
    and +0xF0 cleared, the flag dropped) where a Soul's unison begins or
    ends (`0x0801214A`, `0x08012276`, beside its calm): `darkinvs_leave`
    from soullife.c's enter and leave. `chaos_shadow_turn` (ShadowSoul's
    one turn ending at the second Custom screen while the machine runs)
    PASS: the machine stops in both and every byte of his AI data and
    object is the same to t760. Its recorded FAIL at t524 (his battle
    NaviStats +0x0E read 0x08 in BN5 against BN6's 0x80 from the Soul's
    end) was the differential's own: its reading of BN5's mood in BN6's
    terms took DarkInvs's hits' gains off a mood BN5's calm had set to
    0x80; since the release candidate's fixes it takes them off only
    since the mood was last set (`BN5_MOOD_SET`, `BN5_MOOD_CALM`
    logged). Not ported:
    BN5's other three exits, `0x0802CCA4`, `0x0802CE3A` and `0x0802D028`,
    each in a routine that sets his HP from his NaviStats, clears his
    statuses, calms him and his invisibility (BN5's own resets of him; no
    run's battle seen to reach them). A battle that ends under the chip AI
    leaves the flag in `RAM_PLAYER_AI`, which the next battle's set-up
    overwrites with his fresh AI data (seen: the next battle his own).
    *His look*: under the chip AI both engines give him their palette 1
    (the same branch in BN5's 0x0800DD8C and BN6's 0x0801002C), each
    sprite's own grey. BN5's dark palettes (2 by the meter, 3 after a
    DarkChip) are its sprite's dark blues, BN6's MegaMan's 2 and 3 yellow
    and tan (his NaviCust's), read from both ROMs' MegaMan sprite: given
    BN5's numbers he was drawn tan after every BN5 DarkChip. Since 13
    October 2026 BN6's engine gives him his own (0, BN6 having no dark
    palette), never a colour BN5 does not show; legit.json's mask "BN5's
    dark palettes" cites both routines.
- **A moved virus's deletion**: the specs stop before a foe is deleted
  (Boxer1, InfVulc1, SuprSpr1, DarkMetr): a moved family's deleted virus
  spawns the fourth kind's 0x11 with a RAM pointer at +0x34 (BN5
  `0x02034A90`, BN6 `0x02034880`) the comparator does not read in BN6's
  terms, the virus families' track's.
