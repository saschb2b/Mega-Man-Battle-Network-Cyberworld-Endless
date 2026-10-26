---
type: Engine Mechanism
title: "Transplanted Souls"
description: "Issue #82's Souls track: BN5's six Team Colonel Souls in BN6's engine on the proxy form 8, milestones S0-S7 from UNITE on BN6's Custom screen to Chaos Unison and BN5's mood (`src/emu/soul*.c`, `tools/xsoul.py`)."
tags: [multirom, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-13T12:46:43Z }
---

# Transplanted Souls

Issue #82's fifth call ([One net](one-net.md)): Souls serve every battle, so
BN5's six Team Colonel Souls move into BN6's own engine as the families
do, a milestone at a time, each proved against BN5's own battle before
the next.

- **What a Soul is to BN6's engine**: BN6's Cross is BN5's Soul Unison
  renumbered (issue #82's research, each place read in both ROMs): one
  form byte in the battle NaviStats (+0x2C: a Soul by its Navi's AI index
  in BN5, Team Colonel's 7-12; a Cross 1-10 and Beast Out 0xB-0x18 in
  BN6), the same 16-byte pending form a side, taken as a turn starts, the
  same requests and phases of the change, the same six bytes of loadout
  in MegaMan's AI data. A Soul is a whole sprite where a Cross is MegaMan
  with a palette and an overlay; BN6 writes a Cross's turns and never
  counts them, and dropped Chaos Unison's charge.
- **S0, MegaMan as the differential's subject** (done; [The differential: a moved family against its own game](../devtools/differential.md),
  The differential): a spec can make MegaMan the subject, his setup the
  same in both games (BN5's event flags for the Souls, a folder in each,
  his buster, kept from harm) and each Custom screen passed by its own
  keys in each game, his object, his attacks, effects, sounds and panels,
  his player block (his battle NaviStats, the Souls or Crosses used, his
  pending form) compared in BN6's numbers, and the stop rule flipped: a
  difference of his fails the spec. `megaman_plain` (the buster, a
  Cannon, steps) passes; `soul_knight_ref`, BN5's own KnightSoul from
  UNITE to its fourth turn, was traced in BN5 alone until BN6's engine
  could unite (S2 made it the two-game `soul_knight`). What it found:
  BN5 raises the mood of a side that lands a hit, BN6 never (read as a
  translation until BN5's DarkChip rule is ported), and BN5's plain
  charged shot fires five frames sooner than BN6's ([Virus families moved into BN6's battle engine](../rom/virus-families.md),
  MegaMan as the differential's subject).
- **S1, the proxy form** (done; [The proxy form: where a Soul stands](../devtools/proxy-form.md)): the
  Souls will stand behind form 8, Falzar's TenguCross, which Gregar keeps
  in every table by form and never offers. `--dev soulproxy` makes it an
  exact copy of HeatCross (its eleven tables' entry 8, its NameID's object
  id, hooks on the three places Gregar's code reads the form itself), and
  `tools/xproxy.py` proves it: HeatCross committed as form 8 plays as
  HeatCross every byte of EWRAM and IWRAM, every frame, through the
  crossing, a charged shot, a Fire chip, the buster, a weakness hit that
  breaks it and the battle's end, but the form byte's copies and the
  NameID. Gregar does compare the form with 8 once (TenguCross's Wind
  bonus, `0x0800EFE6`, behind HeatCross's hooked compare), and from form
  8 Beast Out is TenguBeast's (0x14): the Souls grey Beast Out while one
  is united ([MegaMan's forms and BN5's Souls in BN6's engine](../rom/souls.md), MegaMan's forms).
- **S2, UNITE and a Soul's life** (done; in every battle of a run since
  the last round, wherever BN5's Navis are; [The proxy form: where a Soul stands](../devtools/proxy-form.md), The proxy
  form, and The differential; [MegaMan's forms and BN5's Souls in BN6's engine](../rom/souls.md), UNITE on BN6's Custom
  screen): UNITE on BN6's Custom screen by BN5's rule, and a Soul's life
  as BN5 runs it, each Soul behind the proxy (its look and abilities
  BN5's from S3 and S4, below). Element 0xB under OK is BN6's Beast Out
  emblem and BN5's UNITE button in one place (One net: the design's finer
  calls): UNITE while the last chip picked lights a held Soul (BN5's check:
  the chip's kind as BN5 has it, which for BN6's chips is BN5's chip of the
  same name, as the guest battles crossed them, or BN6's own kind by name; a
  DarkChip of it for Chaos Unison; each once a battle), the emblem by
  BN6's own rule otherwise where Beast Out stands, a grey UNITE where only
  UNITE stands. UNITE stands while the run holds a Soul and MegaMan is
  neither worried nor dark by BN5's mood (as BN6's battles keep it
  beside their own since S7, below), nor beasted out. A on it runs BN5's own code for it, moved into
  BN6's copy, which enters BN6's still-present mode 0x24 for a UNITE
  chosen: the last pick given up, UNITE first among the picks, the Soul's
  picture from BN5's ROM; B gives the chip back, as BN5's does; the chip
  given up leaves the deck, as BN5's (BN6 does so for its emblem in a
  link battle alone); OK commits the proxy form, the Soul's turns (3, 1
  for Chaos Unison) and its used bit. The Soul takes MegaMan as he enters
  the proxy, its turns count down as each Custom screen opens (BN5 counts
  them there, in the step that opens the screen, before its window slides
  in: its code and a frame's trace agree), and it ends as the turn after
  its last starts: BN6 checks a form's end before its Custom screen as
  well, for Beast Out, so the Soul's end waits for the turn's start after
  the screen, the one check BN5 makes in a battle not linked. Another Soul
  or a Cross chosen while one is united replaces it; Beast Out stays grey
  while one is (from the proxy it would be TenguBeast's); no weakness
  break for a Soul. The differential holds it to BN5's own battle frame
  by frame: `soul_knight` (KnightSoul united at the first turn, its turns
  3, 2, 1, 0, its end at t1564 in both engines), `soul_replace`
  (KnightSoul then ToadSoul), `soul_grey` (UNITE grey for a chip of no
  held Soul's kind) and `soul_chaos` (NumberSoul's Chaos Unison, one
  turn); `soul_cross` and `soul_beast` hold BN6's alone (a Cross over a
  Soul, Beast Out greyed); five mutations fail; and `tools/xproxy.py
  --souls` finds BN6's own battle the same every byte with the Souls
  installed and none held, the proxy HeatCross through their stubs.
  What it found: the given-up chip's leaving the deck and the end's order
  against the Custom screen both differ in BN6's code and are put right;
  a turn's change takes BN6 four frames more than BN5 (its form check
  before the screen), and a Soul's crossing six fewer than BN5's unison
  (the proxy's summon), frames whose draws the differential explains.
- **S3, a Soul's look** (done; [MegaMan's forms and BN5's Souls in BN6's engine](../rom/souls.md), A Soul's look): united,
  MegaMan is BN5's sprite of the Soul (BN5's sprite list 0, 7-12, copied
  into the Souls' room and BN6's list 0 grown with them at 0x45-0x4A,
  past the words BN6's code reads through list 0; list 0 the Souls'
  alone by the tracks' allocations), its
  palette 0 (2 in Chaos Unison); no Cross's overlay but what BN5's Soul
  spawns as he enters it (NumberSoul's companion); the unison's phase 1
  is BN5's, moved: the Soul's image (BN6's own dead copy of BN5's object,
  drawn from the Soul's Navi's battle sprite, which BN5's Navis' track
  copies into BN6's list 2, `xnavi_sprite_install`, 0x20-0x25), its
  sounds BN5's (0x8D, 0x143 for Chaos Unison); TomahawkSoul's landing
  shakes the camera as BN5's phase 2 does; his face in the emotion window
  and on the Custom screen BN5's of the Soul, its turns counted under it
  in BN5's digits; his arm's flash BN5's own object (the image object,
  the first kind's type 5, moved with its kinds table: the tracks'
  allocations have every track run BN5's, BN6's table grown by none),
  at its own height, and the arm's points of his sprite BN5's.
- **S4, a Soul's abilities** (done; [MegaMan's forms and BN5's Souls in BN6's engine](../rom/souls.md), The Souls' code in
  BN6's engine, A Soul's abilities, BN6's attack tables grown): the code
  each Soul's charged shot runs is BN5's, moved by a recipe
  (`tools/xsoul.py` writes `src/emu/soulcode_bn5.c`, numbers alone: 21
  blocks since the tracks' merge, their calls aimed at BN6's twins or at
  the other tracks' copies, BN5's numbers BN6 has otherwise renumbered,
  every block's CRC32 checked as it is copied from the player's ROM): its
  power attack in BN6's power-attack table grown (0x94 on) with BN5's
  charge times, its family in BN6's attack families grown once for every
  track (`xattack_grow.c`: 0x10-0xFF, the Souls' 0xC0-0xC6), the objects
  it spawns through the families' second-level dispatch (each BN5 type
  moved once, its decision and whose copy in `tools/bn5_twins.py`'s
  TYPES: the obstacle strike's soldier the Souls', the rest the Navis' or
  the chips', below), its effects (the Souls' 0xC0 on) and sprites (list
  0 from 0x4B) where BN6 has none of the same bytes (or whose own
  animates otherwise) and no other track copied them first, its sounds
  copied into BN6's empty song slots at the slot the allocations give
  each sound (0x70 at 0x5F, 0x117 at 0x5E, 0x115 at 0x5A, `xsong_install_at`,
  found there where another track copied it first). His loadout is the Soul's row in BN6's
  ids, his element the Soul's (TomahawkSoul's Wood, ToadSoul's Aqua),
  no weakness, no buster bonus. ColonelSoul's ScreenDivide, ShadowSoul's
  slash, NumberSoul's bomb, TomahawkSoul's swing, KnightSoul's wrecking
  ball (its confusion) and ToadSoul's sound waves (their paralysis) each
  play as BN5's frame for frame ([The differential: a moved family against its own game](../devtools/differential.md)).
- **S5, a Soul's passives** (done; [MegaMan's forms and BN5's Souls in BN6's engine](../rom/souls.md), A Soul's
  passives, A Soul's chips, ColonelSoul's button, chip shot and obstacle
  strike): the passives BN5 gives a Soul as it unites
  and as his form's state is set again (its two tables by form, the
  Souls' routines moved, the proxy's entries of BN6's twins routed to
  them): ShadowSoul's FloatShoe, TomahawkSoul's grass under MegaMan,
  ToadSoul's flag (BN5's sea, which ToadSoul stands on unharmed, harms in
  BN6's engine only a Fire actor as poison does: the lava and sea's own
  port, xpanels.c; its sinking, an object of that flag on the sea, comes
  with the Navi track's port of it); NumberSoul's hand of ten and its +10
  to an attack chip of no kind. The chips (`src/emu/soulchips.c`): the
  chip charge, A held with a chip of the Soul's kind (ShadowSoul's swords,
  KnightSoul's Break chips, ToadSoul's Aqua chips), the chip used charged
  where BN6 would run the power attack, KnightSoul's and ToadSoul's
  doubled; TomahawkSoul's Wood chip doubled on grass and the panel plain
  after, and his heal on grass (BN6's own for a Wood actor, his element
  BN5's); KnightSoul's 50 frames unharmed after an attack chip used from
  his front column; ShadowSoul's B+Left stance and its counter, the
  counter's first frame run at once as BN5's (the Souls' own Thumb);
  ShadowSoul's charged sword run by BN5's sword family, with an enemy in
  reach and without (its family's branch into the next routine's return
  kept in one block; BN6's reserve routine kept, the differential's
  evidence); his shadow none. ColonelSoul's three (`src/emu/
  soulcolonel.c`): his Custom screen's button (BN6's DustCross's elements
  8 and 9, BN5's set-up, rule, phases, draw and art: lit by a standard
  chip of no kind picked last, A drops that chip into it, B gives it
  back, OK keeps it and spends it), his chip shot (the turn's start makes
  his charged shot fire the button's chip, ScreenDivide without one) and
  his obstacle strike (while he is united, an obstacle on the enemy's
  area with an enemy before it, or one of his side behind it, turns into
  BN5's soldier, which fires three shots: BN5's third kind's 0x30 and
  0x12 moved, BN5's block in the obstacles' four states, his side's bit
  in `BN6_COLONEL_SIDES`, never BN6's bits of BattleState +0x5C; BN5's
  Colonel's table at `RAM_COLONEL`, laid out as the Navi track's Colonel
  has it; since the tracks' merge BN5's rule for either Colonel,
  `src/emu/xstrike.c`, below). Each held to BN5's battle frame by frame by a spec of its own
  (the strike's spec leaving out the RockCube's own use, each game's
  chip), a mutation of each part failing. The effects BN6 draws
  otherwise (0x02, 0x03, 0x14) are BN5's copies (0x02 and 0x14 the
  Navis', since the tracks' merge); the flashes' sprites
  BN5's 3:0x00 and 3:0x03 stand at the Navis' numbers (list 4's 0xB5,
  list 5's 0xA6: one place for each sprite's bytes once the tracks
  merge, tools/bn5_objects.py), and ColonelSoul's button's art in the
  room issue #82 granted (`FREE_SOULS_ART`).
- **S6, Chaos Unison** (done; issue #82's final engine round,
  `src/emu/soulchaos.c`, `soulchaos_thumb.c`; [MegaMan's forms and BN5's Souls in BN6's engine](../rom/souls.md), Chaos
  Unison): a Soul united by a DarkChip of its kind charges B as BN5's
  does, its time its chaos power's row by the chaos level (40 frames at
  level 0, to 150 at 4), then a cycle of phases by the level's row (BN5's
  `0x08010648`: 42 frames at level 0, phase 2 its first 21), shown by
  BN5's dark charge glow (the Souls' copy, its animation the phase).
  Released in phase 2 it is the Soul's DarkChip attack through the chip
  writer, BN6's chip use of the registry's id (ColonelSoul's DrkSonic,
  NumberSoul's DarkPlus, TomahawkSoul's DrkLance, KnightSoul's DarkDril,
  ToadSoul's DarkWide, each by BN5's code), the level one up (4 at
  most); released in another phase it fails as BN5's family 0x39 does:
  MegaMan out of the Soul and the Dark Soul (the moved Navi) on the
  other side, its chip lists, where the battle gave none, MegaMan's
  folder's chips, the most of a chip first (BN5 keeps the chips he uses
  most, BN6 no record of them). BN6 reads the chaos request 0x8000 as a
  counter's: its four tests of it drop that bit while the Souls' request
  stands. The power attacks are entries of the Souls' grown power table
  (BN6's 0x9F-0xA4), the failure the attack family 0xC7. Held to BN5's
  battle frame by frame by 19 specs (`chaos_*`: each Soul's success at a
  hold of 45 frames and failure at 60, NumberSoul's holds 75, 90, 105,
  120, 150, 180 and 210: 45, 90, 180 and 210 succeed, the rest fail, as
  BN5's), two mutations failing (the cycle a frame longer, the Dark Soul
  a frame later). ShadowSoul's attack is BN5's DarkInvs since 13 October
  2026 (The chips' code, DarkInvs, moved: MegaMan under BN5's chip AI,
  its arm's chaos flash answered from the Souls' block): `chaos_shadow_45`
  PASS to t500, its recorded FAIL at t107 gone.
- **S7, BN5's mood and its DarkChip rule** (done; [MegaMan's forms and BN5's Souls in BN6's engine](../rom/souls.md), BN5's
  mood in BN6's battles): `src/emu/soulmood.c` keeps BN5's mood in the Souls' block
  beside BN6's own, by BN5's rules (a hit taken lowers it, a hit landed
  raises it, a counter calms him to Full Synchro, BN6's twins of BN5's
  other writers mirrored, a Soul's unison and end calm him, the battle
  opens as worried as his HP says), never through BN6's emotion values.
  BN5's DarkChips (the registry's) come into the deck, shelved behind the
  padding until BN5's worried rule deals one into the hand's end as a
  Custom screen opens while he worries or is dark; used, he is dark for
  the battle (his mood 0, held there; BN6's own mood 0 with it: no Cross,
  no Beast Out, a weak buster). BN6's own DarkChips keep BN6's rule.
  UNITE stands only while he neither worries nor is dark. BN5's dark gate
  holds for the registry's chips that name it (BlakWing only while dark,
  HolyDrem and a Navi's V1 and SP chips only while not, the DS chips only
  while dark): refused, the chip is BN5's chip of nothing; the battle's
  dark stands for BN5's dark meter, which no run has. Held to BN5's own
  battle by `soul_mood_dark`, `soul_mood_unite`, `soul_mood_gate` and
  `soul_mood_chaos` (Chaos Unison as BN5's play reaches it: DarkDril
  offered worried, kept, a Cannon's landed hit lifting him by its 0x1E,
  BN5's gain, where the buster's hits lift none, and KnightSoul's Chaos
  Unison on the calm screen after), the mood compared in every Soul's
  spec, a mutation of each rule failing. Since
  issue #82's final engine round each DarkChip's after-effect on his
  battle NaviStats (`0x0802C938`, ported) is BN5's too (his dark palette
  BN6's own since 13 October 2026: BN6's MegaMan has none, BN5's numbers
  drew him tan; The chips' code, DarkInvs, moved), the worried
  rule held on the battle's first screen as well (`soul_mood_first`:
  the DarkChip into deck[5], past the five the screen deals, in both),
  and BN5's DarkChips' own attacks play by their code (The chips' code,
  Their rules in place).
- **At the tracks' merge** (issue #82's four tracks in one tree): one
  copy of each moved type, the first install's (`src/emu/xinstall.c`:
  the moved objects and the Navis, the chips' code, the families, the
  sea, then the Souls' code), so the Souls' code runs the others' copies
  of what it spawns and calls into (`tools/xsoul.py`'s BEFORE and
  shared_blocks, the C's routes): the image object the moved objects'
  (MegaMan's buster's flash in a Soul sent to its spawner there,
  `soullook.c`), NumberSoul's companion and bomb, KnightSoul's ball and
  ToadSoul's hit the Navis', the obstacle strike's shot and ShadowSoul's
  afterimage the chips' (the shot's row the strike spawns it with fixed
  in the chips' copy to the Souls' effect 0xC1: `tools/bn5_twins.py`'s
  SHARED_ROWS); one BN6 number a BN5 effect and a BN5 sprite, the
  first track's (ScreenDivide's three and TomahawkSoul's swing the
  Navis' 0xA8-0xAA and 0xA6, ShadowSoul's slash 0xA3, 0x14 and 0x02
  0xA0 and 0xA2; the strike's hit's sprite the chips'); BN6's
  collision update's audit serves BN5's own body, which the Souls copy
  as the families and the chips do. The obstacle strike's soldier draws
  BN5's sprite 4:0x20, set from a packed word (`0x0800F198`), which the
  Souls' generator now numbers: BN6's 4:0x20 is another picture (the
  differential's sprite bytes, soul_colonel_strike's t634). The obstacle
  strike is BN5's rule for either Colonel (`src/emu/xstrike.c`): the
  Souls' code and the strike are installed with the core where BN5's
  Navis are, sprite list 0 grown by the Souls' code for its own art, and
  a Navi Colonel's spawn arms his side in `BN6_COLONEL_SIDES` (his moved
  write, `XN_STUB_SIDES`) as ColonelSoul's ready does, one field the
  strike reads: `colonel_strike` holds a Navi Colonel's soldier to BN5's
  (MegaMan's RockCube before him, the three shots, frame for frame in
  both; its comparison stops at the cube's own use, BN5's RockCube
  object being the code Colonel's moved copy runs, its expectations held
  in both battles), a mutation of his side's bit failing it. BN5's
  DarkChips' specs hold the Souls (KnightSoul) so that BN5's mood and
  DarkChip rule are in place: since the final engine round they PASS,
  their recorded FAILs gone (each chip's after-effect and his dark
  palette ported), BlakWing's run dark in both (BN5's meter under 470,
  BN6's latch) and HolyDrem's moved.
- **Next**: the Souls' engine is whole, ShadowSoul's chaos attack
  (BN5's DarkInvs) since 13 October 2026; what is left of DarkInvs is The
  chips' code's (DarkInvs, moved: three of its exits). The masks the differential kept "until the Souls track's S6"
  are gone. The Souls' state in RAM is the engine's
  plan's (`0x0203FF70`-`0x0203FFEF`, `BN6_SOULS`), their code and data
  in `EMU_FREE` + `0x480000` to `+0x4C0000` and their art past it at
  `+0x700000` to `+0x710000` ([Running the game's own logic](../engine/emulation/overview.md); laid out in
  `src/emu/soul_room.h`).
