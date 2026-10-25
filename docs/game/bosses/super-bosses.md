---
type: Feature
title: "Super bosses"
description: "Super bosses Bass and the Cybeast (issue #100): the Cybeast in every endless Net's Nest, Bass in the Secret Area once cleared, his form by record, their foreshadowing, grander staging (`src/core/super_boss.c`, `boss_grand.c`) and Giga rewards."
tags: [game, bosses]
generated: { by: claude-code/unrecorded, at: 2026-10-19T16:46:21Z }
---

# Super bosses (6 October 2026, issue #100)

A player missed super bosses, Bass and the Cybeasts above all. The owner:
not sprinkled in; each comes at a decided time and place in the layers,
with a presentation grander than a guardian's. What the ROMs hold, what
BN6 makes of them, the design reasoned with the game-design skill, and
what was built.

## What the ROMs hold

Verified in BN6 Gregar's ROM (its enemy id table and stats read for every
Navi, its battle records and random tables walked, its sprites drawn with
`--sheet`, its sounds rendered with `--render-song`), in the bn6f
disassembly (Falzar's: its cutscene and map scripts, which start the same
records by index), and in the community's references ([Sources](../../research/sources.md):
MMKB, MMHP, the LP Archive; the Rockman EXE Zone and The Cutting Room
Floor through search excerpts only). Each row says how.

| What | BN6 Gregar holds | How it was verified |
| --- | --- | --- |
| Bass (AI 19) | Bass 1800 HP (enemy `0x16D`), Bass SP 2700 (`0x16F`, `0x170`), Bass BX 3400 (`0x16E`); a BassXX record of 2700 whose reward row pays 1 zenny, unused | The id table's type 1 and its stats rows; the names archive; MMKB's Bass.EXE and the Rockman EXE Zone's custom battle guide list the same ids and HP |
| His battles | Records `0x88` (Bass, music `0x16`), `0x89` (Bass SP, `0x16`), `0x8D` (Bass BX, `0x16`) of the story list at `0x0B06E0` (16 bytes each, the same index in Falzar); Graveyard 2's random table holds Bass BX as its SP Navi (byte 7 = 10, an event flag's) | The records read at `0x0B06E0` + 16 × index; bn6f's `cs_start_fixed_battle` with those indices in his cutscenes; the random tables as `formations.c` reads them |
| His reward | His rows pay 1000 to 3000 zenny by the busting level, no chip | The reward rows at `0x0AC718` + id × 0x28 |
| His look | Overworld sprite list 6 `0x5B`: cloaked, facing right through down-left (animations 2-5) and never away from the camera, 24 a pose, 25 the cloak off, 26 throwing it open; face `0x5B`; battle sprite list 0 `0x41` (bn6f's "Bass without cape") | Drawn with `--sheet`; his enemy traits name `0x41` (`0x13` + `0x2E`, as every Navi's) |
| Where BN6 sets him | Bass in Undernet Zero, optional from chapter 4, a dormant stone bearing his epithet that breaks as MegaMan nears; Bass SP at the Graveyard's end, behind gates for 100 and 200 Standard chips, paying the Bass Giga and the title's Bass mark; then in Underground 2 GBeast SP (a copy of MegaMan's beast he sets on him) and Bass BX, Bass with the Cybeast's power, paying ColForce; Bass BX roams the Graveyard after | MMKB (Bass.EXE, Graveyard, Undernet), MMHP and the LP Archive (updates 71, 103-105) agree; bn6f: the trigger in the Undernet's map 1 script (Undernet Zero), the Graveyard's and the Underground's scripts |
| How BN6 stages him | The music fades (`cs_sound_cmd_803810e 8, 0x1F`); the stone (sprite list 7 `0x9B`) shakes with sound `0xFE` and breaks (its animations 3, then 2); the screen shakes with sound `0xE3` every 32 frames; the music stops, sound `0x100` plays with a fade to white; out of the white the boss prelude (`0x1C`), then the battle; after it, sound `0xD7` with a white fade as he goes | bn6f's cutscenes before records `0x88`, `0x89` and `0x8D`, and the Undernet's NPC list; the stone drawn with `--sheet`; each sound rendered (`0xE3` a steady rumble of 0.8 s, `0xFE` a burst, `0x100` an impact, `0xD7` a fading tone) |
| The Cybeast Gregar (AI 20) | Gregar 2500 HP (`0x173`), Gregar SP 4000 (`0x175`; the others of 4000 named EX, RV, BX) | The id table and stats; MMKB's Gregar agrees |
| Its battles | Records `0x84` (Gregar) and `0x85` (Gregar SP), music `0x17`, the final battle's theme; BN6 picks SP once the title holds the six marks of `0xF6` | The records; bn6f `sub_8096B00`, which tests the marks (`GetTitleScreenIconCount`) before the cutscene starts one or the other; the Rockman EXE Zone's title mark page and LP107 |
| Its look | On the map list 6 `0x58`, a crouching beast (animations 25, 27, 28) that rears (26, 29); face `0x58`; battle sprite list 0 `0x42` | Drawn with `--sheet` |
| Falzar (AI 21) | Stats (2300, 3200) and no battle record in Gregar; its battle sprite list 0 `0x43` and map sprite `0x59` are white dots here | The records walked; bn6f names `0x43` "Falzar (Falzar version) or white dot"; drawn with `--sheet`. Not fightable in Gregar |
| GBeast and FBeast (AI 23, 24) | MegaMan's own beast form, berserk: GBeast 900, SP 1800 (`0x187`), its sprite list 0 `0x0B`; Gregar's record `0x8C`, and the SP Navi of Underground 1 and Graveyard 1 | The records and its traits; MMKB's GBeast |
| AI 17 and 22 | No name, 4000 HP at their first version; their traits draw list 0 `0x00`, MegaMan himself; the Immortal Area's SP random record (byte 7 = 8) holds AI 17 | The traits and tables. The research's lead, likely right: the Count of Groundsoaking Blood (the Boktai crossover), cut from the US version with the Immortal Area, an idle MegaMan left in his place (The Cutting Room Floor, an excerpt), and bn6f's battle modes `HAKUSHAKU_INVINCIBLE_MODE` and `HAKUSHAKU_DEFEAT` (the Count, in Japanese) |
| BN5 Team Colonel, beside BN6 | Bass (AI 19: 1500, 2000, 3000, SP 3500, DS 4000): story records `0x52`, `0x54`, `0x55`, and its Undernet map `0x94:05`'s SP and DS records behind conditions; Nebula Grey (AI 20), its final boss | BN5's id table, stats, story list `0x113CD8` and net tables read; MMKB and Wikibooks on BN5's Nebula Area and the Chaos Lord |

## The design

Reasoned with the game-design skill: the lens, the pattern catalog, the
critique smells, the transitions budget.

- **The experience**: dread, then the reveal of a legend; a fight beyond
  the act that the player chose, or that the whole dive led down to.
  Challenge and narrative, and a record of mastery.
- **The dialectic** is the run's, prepare or press on, at its two
  extremes. The Cybeast is where the endless net leads: everything
  prepared (the folder, the Crosses, the Net's clock paid for trips back)
  is tested at the bottom, and beaten, it pays its own power, BeastOut,
  for the Nets after (issue #109). Bass is
  pressing on in its purest form: a fight beyond the act behind a gate the
  player chooses to open, for a Giga chip and a record.
- **The loop layers**: the Cybeast feeds the progression and run loops,
  the end of every Net, whose fall rebuilds it; Bass the session (the
  Secret Area, a side trip) and the meta (his record across runs, his
  forms, the Library's Gigas).
- **The patterns**: a capstone at the end of a bounded arc inside an
  endless run (each Net a dive with its own last boss, BN6's own SP on
  the later ones); an optional superboss behind a key (Slay the Spire's
  Heart behind three keys, BN's Bass behind collection gates; here three
  ScrtData and a mark); bosses who remember (Hades), Bass growing with
  every defeat as BN6's Bass does; foreshadowing and payoff (the short
  net's last words, the Nest's growl, the Graveyard's call, the gate's
  signal); the honest telegraph (each gate says what waits before it
  opens, and a bigger arena with the music falling away says this is not
  a guardian); and the transitions budget: a long entrance because it is
  rare, a shorter one at a rematch.
- **The smells avoided**: sprinkling (BN6's own random records of Bass
  BX, GBeast and the cut Count rolled as Server challenges in the
  Graveyard and the Nest; they no longer do); power creep (what they pay
  is the run's, Gigas and HPMemory, and the Library's collection; across
  runs only the record and the knowledge, and the BeastOut helper the
  endless Nest's fall already opened, issue #99, a helper chosen each
  run); a hollow loop (the Cybeast's
  fall opens the next Net, Bass's defeat his next form); a lying
  telegraph (a rematch's form is said at the gate, before the choice);
  complexity (two super bosses, one place each, a sentence each).

## Where each stands

1. **The Cybeast Gregar, at the bottom of every endless Net.** The
   endless net's Nest (layer 19, and every nineteenth after it) is the
   Cybeast's den: Gregar there, not one of the Nest's copies, in BN6's
   own final battle (`0x84`). On the second Net and after, Gregar SP
   (`0x85`), which BN6 keeps for a player with six of its marks: the Net
   copied the last battle and came back stronger. Its fall rebuilds the
   Net, as the Nest's guardian's did, and earns the title's Bass mark
   ("the endless net's own Nest cleared") as before, and with it the
   BeastOut helper for the runs after (issue #99): MegaMan downloaded the
   Cybeast's data. In the run itself it gives BeastOut from layer 20 on:
   Dad's call after it unlocks the PET's CybeastButton, where the run did
   not bring it (issue #109). The short net keeps
   its Nest guardian: its last words already say something deeper is
   still awake, and the endless net, which its win opens, pays that off.
2. **Bass, in the Secret Area, once it has been cleared.** BN6 sets
   Bass's first battle in Undernet Zero, in a stone that wakes as MegaMan
   nears; the Secret Area is drawn in Undernet Zero's tiles, "where the
   strongest wait". Its first clear stays ProtoMan's copy, which earns the
   title's S; in every run after that, Bass waits there. Chaud's call
   after that first clear says officials picked up another signal down
   there, no copy's. Bass is real, the one thing in the Endless Net that
   is: he came to hunt its strongest copies. His form follows his record
   in any run, as BN6's chain does: Bass (1800) until MegaMan beats him,
   then Bass SP (2700), and once he has fallen twice and the Cybeast once
   (their records, `rivals.sav`), Bass BX (3400): he took the beast's
   data. The gate says which before it opens, and its question starts on
   No; the fight is a choice.

Neither comes at random: BN6's own records of Bass BX, GBeast SP and the
cut Count are taken out of the Servers' SP Navis, which keep the story's
SP Navis (BlastMan, DiveMan, CircusMan, JudgeMan, ElementMan, Colonel).

## How the run foreshadows them

The Cybeast, from the first run on:

- the short net's Nest growls as MegaMan arrives, and its last words say
  something deeper is still awake, the Nest only its den (as before);
- the endless net's Graveyard stirs the beast in MegaMan ("The Nest is
  calling to the Cybeast in me!"), unlocking nothing: BeastOut comes with
  the Cybeast's fall (issue #109), or as a run with the BeastOut helper
  begins;
- in Lan's HP, the Nest's portal: MegaMan reads something growling far
  down, like the beast in him;
- the endless Nest's arrival: the growl from below, and MegaMan feels the
  beast in him answer; a bystander there: the floor keeps shaking,
  something huge is waking up; L in the Nest: something huge waits at its
  end, growling like the beast in MegaMan; the PET's next step on the
  second screen, "Something huge waits at the end";
- in the room before the arena, MegaMan: the growling comes from just
  ahead;
- as MegaMan nears the arena, the music fades as BN6's own scenes fade
  it, and the floor shakes with BN6's rumble.

Bass, once the Secret Area is his:

- Chaud's call after the first clear: another signal, not a copy;
- the Endless Net BBS: a cloaked Navi seen in the Secret Area, deleting
  copies;
- at the golden gate: MegaMan senses a dark signal behind it, unnamed
  until they have met ([What MegaMan knows](../meta/what-megaman-knows.md)); after that,
  "Bass is in there", and his form;
- the Secret Area's arrival, L's word and the room before the arena: a
  dark signal at the end that does not feel like a copy; the second
  screen, "A dark signal waits at the end";
- its arena: BN6's dormant stone stands where a guardian would, its
  screen flickering.

## The presentation, above a guardian's

| | A guardian | A super boss |
| --- | --- | --- |
| Arena | 5 x 5 panels at the end of a bridge | 7 x 7, an octagon, where the layer has room: four more tries at the layout, at its smallest from the third, and a guardian's 5 x 5 in the last five (the Nest's crosses and slabs at their size seldom had room; so, nearly nine layers in ten) |
| Approach | The arena silences the area's theme | The theme fades out as MegaMan steps into the room before the arena, as BN6's own scenes fade it; before the Cybeast, the floor shakes now and then with BN6's rumble (sound `0xE3`); before Bass, his stone stands in the arena |
| Entrance | Bars, the prelude, a log-in with a shake, about 1.3 s | Bars and silence; the rumble every 32 frames, harder each time (BN6's own cadence); Bass's stone shakes and cracks (`0xFE`); the screen fades to white with sound `0x100` and holds; in the white the stone lies broken and the super boss stands; the prelude starts as the white lifts, as in BN6; once it has, Bass throws his cloak open, the Cybeast rears with a roar and a heavy shake, and the card follows. About 4.6 s, 3.3 s at a rematch, which skips the first two rumbles |
| Title card | A band, the area, the name, the epithet | A darkened picture, a taller band with double edges in its colour, the name larger and heavier, its form or the Net beneath, held longer |
| Words | The guardian's line, MegaMan's answer, the battle call | Bass speaks (his face, BN6's), by his record; the Cybeast roars (its face) while MegaMan, Lan and Dad speak; no word on how to fight them before the call (MegaMan never coaches, [Voice](../../voice/rules.md#what-nobody-says), What nobody says): MegaMan's battle data on their moves, once fought in any run, is the briefing's and the second screen's, watched in god mode (Bass's cape takes no damage and comes off as he attacks; every attack of the Cybeast's lights its panels first) |
| Fight | BN6's boss theme, the area's background | BN6's own record: Bass's boss theme (`0x16`), the Cybeast's final battle theme (`0x17`), the background of the map it stands on as BN6's records ask (`0xFF`), no running |
| After | Silence, last word, a flash, the log-out | Silence; Bass's last word, a white fade with BN6's `0xD7` as he goes, his stone's pieces fading with him; the Cybeast's last growl, a long white fade with the floor shaking, then the Net stays silent through Dad's call (the CybeastButton unlocked, where the fall gave the run BeastOut) |

## What they pay

The guardian's Guardian Data, and more: five HPMemory (four on threat
9), a full heal, BN6's own zenny from the results screen (1000 to 3000 by
the busting level), and a Giga chip of BN6's: Bass BassAnly (BN6's
other version's Bass Giga), Bass SP the Bass Giga (BN6 Gregar's for Bass
SP), Bass BX ColForce (Gregar's for Bass BX), the Cybeast BugRSwrd
(Gregar's own version Giga; the Cybeast was born of bugs). The Cybeast's
data on a normal layer also brings the NaviCust's draft, as the Nest's
guardian's did; its fall gives the run BeastOut for the Nets after
(issue #109) and opens the BeastOut helper (issue #99). The
Beast chips (Gregar, Falzar, DblBeast) are left out: the Japanese Beast
Link Gate's, unverified in the US version.

## As built

- `src/core/super_boss.c`: who they are (navi 19 and 20, their AI index
  in BN6's table), where each waits (`run_new` sets the endless Nest's
  master and, with the title's S, the Secret Area's), each form by record
  and Net, their records' music, names and Gigas, and the AIs no
  Server's challenge holds (`loot.c`, `navi_challenge`).
- `make_boss` gives a super boss his form, his record's music and the
  map's own background (`encounter.c`, `0xFF`, the record's background
  as BN6's story records have it).
- `src/net/net_gen.c`: his arena, 7 x 7 where the layer has room
  (`build_layout`); `LAYER_MAKE` 88.
- `src/layer/stage_npc.c`: `npc_super` (hidden until the white, his pose
  on flag `0x1454`, then what he stands in after it; a slow fade for the
  beast) and `npc_seal`, Bass's stone (BN6's own sprite, list 7 `0x9B`:
  standing, shaking with its crack on flag `0x1452`, in pieces once he
  stands, fading with him; nothing walks into it). `guardians.c` gives
  their bodies (`super_body`), `guardian_objs.c` their scripts: BN6's
  sounds (`0xE3`, `0x100`, `0xD7`) and the music's fade (`ts_sound_fade_out`
  on its slot, `0x1F`, eight sixteen-frame steps, as BN6's scenes fade it).
- `src/director/boss_grand.c`: the staging in boss.c's place, frame by
  frame; `cinema.c` its whiteout and the grand title card.
- Their words: `src/layer/super_lines.c` (meetings, last words, tips,
  data, card), with the foreshadowing in the words files beside each
  (`story_words.c`: arrival, Chaud's and Dad's calls; `briefing_words.c`:
  L's word, the room before the arena; `home_words.c`: the Nest's port;
  `service_words.c`: the gate; `layer_words.c`: the bystander;
  `pet_text.c`: the BBS; `second_text.c`: the PET's next step).
- For tests: `--dev side=2` starts the run in the Secret Area (`side=1`
  the Undernet, its gate), `--dev bass=N,beast=N` sets their records,
  `--talk scrtdata:FRAME` gives the three ScrtData a gate wants (docs/
  [Dev tools](../../devtools/overview.md)). `tests/test_core.c` (`test_super_bosses`): their forms,
  records, places and arenas (52 of 60 test layers at 7 x 7 beside issue #98's signature rooms, each reached
  from the start), every line of theirs within BN6's box, none naming
  them before they have met.

Checked by headless captures with BN6's ROM alone, the autopilot fighting
(god mode, MegaMan at 800 HP): the Cybeast at seed 7's layer 19, Bass at
depth 8 in the Secret Area with the S held, each from the approach
through the entrance, card, talk, battle, fall and data to the exit, and
the endless Nest's to Dad's call and the next Net; Bass BX's rematch
(`bass=2,beast=1`); the gate unmet and met; the music's state each second
(`CYBERWORLD_EMU_DEBUG`): faded on the approach, stopped through the last
word, back after Bass and silent after the Cybeast.

## Not built, and the owner's calls

Left for later: GBeast SP before Bass BX, as BN6 sets them back to back;
BN5's Bass and Nebula Grey in BN5's engine, where BN5 dresses the
Graveyard; the Beast chips. Where a choice reached past the super
bosses, the conservative one was taken, each one switch for the owner to
turn:

- the Cybeast is the endless Nest's own guardian, not a layer of its own
  below it (no change to the cycle's 19 layers);
- the short net keeps its Nest guardian (the Cybeast could be threat
  10's second guardian below its Nest);
- Bass takes the Secret Area after its first clear, not from the first
  visit;
- the title's Bass mark stays the endless Nest's (BN6 gives it for Bass
  SP);
- the endless Nest's master is BN6's own: Gregar at 2500 HP where the
  Nest's SP guardians stood at 1500 to 2000 (`build.py pacing`), and
  Gregar SP at 4000 on every Net after the first, the Net's clock adding
  its tenth a notch as to any guardian. Gregar on every Net, or the
  clock left off him, are the gentler switches.

## The owner's decisions (6 October 2026)

1. The Cybeast stays as built: Gregar on the first Net, Gregar SP on
   every Net after it, the Net's clock on top as on any guardian.
2. Silence after every guardian's battle stays ([the sequence](sequence.md)).
3. The framing: before the talk MegaMan steps up beside the boss, so the
   chat box no longer covers him wherever the arena is entered from
   above, a scripted move in the staging as BN6's cutscenes move him,
   for every guardian ([the sequence](sequence.md)).
