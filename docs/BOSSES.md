# Guardians and super bosses

How a guardian Navi is met, fought and left behind, from the approach to the
next area. The structure follows how Hades stages its bosses; everything on
screen is the game's own (NPC and text commands, mugshots, songs, sounds)
or drawn over its frame by `src/director/cinema.c`. The super bosses, Bass
and the Cybeast, have places and a staging of their own: [Super
bosses](#super-bosses-6-october-2026-issue-100), below.

## What Hades does, and what was taken

| Hades | Here |
| --- | --- |
| The room before a boss is safe: Charon's shop, often a fountain | The antechamber, the room the arena's bridge leaves from, holds a heal and the Net Dealer (or a room near it, where it has no counter's place); stepping in, MegaMan names those not yet used and which way each is, since the arrow leads past them. While the guardian waits no random battle starts there, on the bridge or a panel round the arena (a playtester met one on the bridge, right after MegaMan's "no running from a guardian", session 64) |
| The boss chamber is an arena of its own, locked until the fight is over | A square arena at the end of one bridge, drawn in the area's second floor; the exit pad inside it stays hidden and shut |
| The music drops to ambience on approach, the boss theme starts with the fight | Entering the arena silences the area's theme; the boss prelude (song 0x1C) plays through the intro, the game's boss theme in the battle |
| Short intro dialogue with portraits, chosen by history (first meeting, who won last) | Lines with the guardian's mugshot for a first meeting (Lan or MegaMan naming the copy first), a rematch, revenge after beating MegaMan, a stronger version, and grudging respect after many losses; MegaMan answers now and then; the Nest's own guardian says what it is; Lan's "Battle routine, set!" and MegaMan's "Execute!" close it |
| The boss's name and epithet on its health bar | A title card: the area it guards, its name large (with EX or SP for stronger versions), its epithet; the game's own battle shows its name too |
| No question before the fight | The battle starts after the last line |
| A defeat line, a flash, the boss leaves | A last word, and the guardian logs out as BN6's Navis do on the net: his sprite's own log-out beside BN6's beam and its sound |
| A reward to walk up to; the exit opens | Its Guardian Data materializes where it stood (its Cross with MegaMan's words, at the first Graveyard the beast stirring in him, five HPMemory, its own Navi chip at the version fought, a full heal); taking it makes the exit pad appear |
| Stairs into the next region, its name on screen | An area-clear card (guardian, viruses, time) over the jack-out, then the next area's title card |
| Bosses remember runs | `rivals.sav` counts meetings and who won each battle, per Navi |
| The Codex fills as you meet things | MegaMan briefs a guardian's moves, and when a hit lands, only once they have fought that copy in any run (its `rivals.sav` record); a first meeting says they have no battle data, and to watch the yellow panels (docs/META.md, what MegaMan knows) |

Not taken: arenas that change during the fight and extra phases, which
would mean changing the game's battles themselves.

## The sequence (`src/director/boss.c`)

| State | What happens |
| --- | --- |
| Wait | The guardian's NPC waits hidden in the arena's middle |
| Enter | MegaMan steps into the arena: bars slide in, the music stops, MegaMan steps up to his place beside the guardian's and turns to it; the boss prelude starts, the guardian logs in facing him as BN6's Navis do (BN6's beam rising, its sound 0x76, and his sprite's own log-in, `0x19` + his facing) and the screen shakes; the card waits for MegaMan to be there |
| Title | The title card, while Gregar's own Navis strike their signature pose (animation 24) |
| Talk | The intro lines; input is A and B only |
| Fight | The game's navi battle, forced at once (the random battle record is not re-rolled over it) |
| After | Back on the map: bars, silence |
| Last word | Its defeat line |
| Log out | His sprite's own log-out (`0x21` + his facing: a frame in his lighter colours, then gone) beside BN6's beam and its sound, about a quarter of a second, as BN6's Navis log out on the net; no flash and no fading by alpha, which BN6 never does (the owner saw the old fade as a "blup"); then a beat, 45 frames from its start |
| Reward | The Guardian Data shows (sound 0x94), BN6's Mystery Data crystal turning (its sprite's animation 0); the area's theme returns and MegaMan walks to it. Taken, it gives in BN6's own "MegaMan got:" boxes, MegaMan's one word on a Cross and on a first meeting's battle data, the heal, and the draft: one box and its menu |
| Open | Taken: the exit pad appears and its warp works |

Event flags `0x1448`-`0x144C` drive the NPCs: the guardian leaves, logs in,
the Guardian Data shows, it was taken, the exit shows. While the exit is
shut, flag `0x16F1` keeps its warp from starting.

The silence is BN6's own "no song", 0x63, which its `PlayMusic` stops
every song for, as its scripts hush a scene. Until 6 October 2026 the
hush played song 0xFF, which stopped nothing: the area's theme played on
as MegaMan stepped in, and after the battle, where the game starts the
map's theme a frame after the battle anyway, under the guardian's last
word (the music's status read each frame, `BN6_MUSIC_STATUS`). The hush
after the battle now runs while a song plays, in the 40 frames before the
last word.

MegaMan meets the guardian beside him (the owner's call, 6 October 2026):
BN6's camera follows MegaMan and the chat box fills the picture's lower
third, so where the arena lay below its way in, a guardian he walked
straight at stood half behind the box through the talk. Now he steps up
to a place at the guardian's height on the screen, on the side the
bridge comes in from, and turns to him (`guardian_stand`: 24 world units
off on both axes, 48 pixels across the picture, a super boss 28, his
sprite being wider); the guardian logs in facing him (BlastMan, ElementMan
and Bass, drawn only towards the camera, turned down-left where he stands
on their left). The steps are the staging's own, as BN6's cutscenes walk
MegaMan (`cs_move_player_in_facing_direction`: as if the player walked,
the pad the cutscene's): the player's pad is held from the step in to
the battle, as before, and his own walking is BN6's everywhere else. The
place is worked out from the arena's way in, so it holds whichever side
a layer brings MegaMan in from; `tests/test_core.c` checks it on every
generated arena (floor under his feet, the bridge's side). A layer without
an arena keeps the old meeting, straight at the guardian.

## Guardians

Mugshots share their index with the Navi's overworld sprite in list 6:
HeatMan 0x47, ElecMan 0x49, SlashMan 0x4B, ChargeMan 0x4F, EraseMan 0x50,
BlastMan 0x51, DiveMan 0x52, Colonel 0x53, CircusMan 0x54, JudgeMan 0x55,
ElementMan 0x56, ProtoMan 0x3B, MegaMan 0x37. Every guardian stands on
the net in his own shape (`guardian_body`):

- Gregar's overworld sprite where it has one that faces every way.
- BlastMan's and ElementMan's, which Gregar draws only facing right
  through down-left (animations 2-5; its story never turns them from the
  camera), turned down-right or down-left where the arena would face them
  up into the screen.
- Falzar's Navis (SpoutMan, TomahawkMan, TenguMan, GroundMan, DustMan),
  whom Gregar has no overworld sprite or face of, in their battle sprites
  (sprite list 0, `0x2E` + the navi), standing in animation 0, logging in
  by BN6's battle warp-in (their animation 3) and mirrored to face right.
  They speak with a portrait from MegaMan's battle data (`portrait.c`): a
  40 x 48 window of the same battle sprite, in the frame of a BN6 face, on
  a ground of their title card's colour.
- BN5's Navis (below), in their own BN5 overworld sprites and faces, copied
  in from the player's BN5 ROM at Gregar's list-6 and mugshot number 78,
  one guardian at a time (`xnavi_guardian`); their sprites lay out their
  animations as Gregar's Navis' do, so they log in by animation 25 and
  strike their pose, 24, for the title card.

A guardian's navi index is its ai in the enemy table: HeatMan 1 .. ElementMan
16, and Colonel 18. BN5's take 24-29 (Colonel, ShadowMan, NumberMan,
TomahawkMan, KnightMan, ToadMan: their AI index in BN5's table, 7-12, plus
17), each with a rival record of his own. Bass is 19 and the Cybeast
Gregar 20 ([Super bosses](#super-bosses-6-october-2026-issue-100)).
Index 17 (and 0, 22) is an unnamed navi with 4000 HP at V1, drawn as
MegaMan: the place of a Navi the US version cut (What the ROMs hold,
below), whose SP record a Server's challenge in the Graveyard could roll
until the super bosses held it back; runs saved before Colonel moved to 18
have their 17 changed on loading, and his rival record follows.

Which guardian an area gets depends on its act (docs/PROGRESSION.md): from
the area's pool the Navis whose HP at the act's version lies in the act's
band (400-600 in act 1 up to 1200-2000 in act 6), else another Navi in the
band, else the pool's nearest. Acts 1-3 fight V1, acts 4-6 EX where it
fits, the Nest, the Secret Area and later cycles SP.

## BN5's Navis (4 October 2026, issue #69)

Where Battle Network 5: Team Colonel dresses an area its random battles
are BN5's own, fought on the guest core (docs/MULTIROM.md, Guest
battles). The owner's call: BN5's Navis guard some of those acts, fought
in BN5's engine; beaten, a Navi gives his Soul for the run's BN5 battles
(docs/META.md, Souls in BN5 territory). Standing beside it: no Cross in
BN5's battles (MegaMan says so arriving), while a BN6 guardian's fight
there keeps the Cross; DarkChips cost 20 max HP per battle used; no power
creep across runs. What follows was reasoned with the game-design skill,
and the owner approved all of it (4 October 2026), the points first
marked as proposals among them.

- **The dialectic** is the meta layer's, what you bring against what you
  find, met at an act's end: the guardian's Soul is found, and the folder
  brought decides how much it is worth, as it unites with chips of one
  kind. It feeds the progression loop (the act's last fight, in the
  territory's own engine, so the territory stays whole from its first
  battle to its guardian) and the run loop (the Souls a run carries into
  its later territories).
- **The patterns:** nested progression, Souls being BN5's axis beside
  BN6's Crosses, each serving only its own engine's battles (augment, not
  replace); the honest telegraph, the territory's look, words and card
  already saying whose battles these are; meta as variety, nothing of it
  carried to the next run.
- **What to watch:** a Soul the folder cannot use (see the table: two
  kinds have few chips in BN6), answered by the chip his Guardian Data
  gives; a guardian fight that is only harder for a BN6 folder (chips sit
  out, no Cross), answered by the act's band and the Soul it pays; a Soul
  won with no BN5 battle left to serve, answered by where they stand.

**Which Navis, where**. BN5's own data places its Navis: its
net maps' battle records list each Navi's SP version on the maps he
roams once its story is over (docs/ROM_DATA.md, BN5's Navis). Of the six
whose Souls Team Colonel's MegaMan unites with, each guards the area BN5
sets him in:

| BN5's area (it dresses) | His record roams | Guardian | His Soul unites with (BN5's kind) |
| --- | --- | --- | --- |
| ACDC Area (Central Area) | ACDC Area 1 | KnightMan | Break chips |
| End Area (Seaside Area) | End Area 3, 4 | NumberMan or ToadMan (the run's seed) | Plus chips, Aqua chips |
| Oran Area (Green Area) | Oran Area 1 | ShadowMan | Invisible chips |
| SciLab (Sky Area) | SciLab 4 | TomahawkMan | Wood chips |
| Undernet (Undernet) | Undernet 3 | Colonel | Obstacle chips |
| Nebula Area (Graveyard) | (Bass) | none: BN6's guardians | |

Nebula Area keeps BN6's guardians: BN5 lists none of the six there, its
act is the last before the Nest (a Soul won there would serve no BN5
battle in the cycle), and the Graveyard's guardian is the one whose fall
stirs the Cybeast in MegaMan before the Nest.

**How often**: on the guardian layer of an act whose area
BN5 dresses, in half the runs (a coin of the run's seed for each area, of
its own beside the dress's), where the build can run the guest. Not the
short net's last act (its Nest is BN6's: no BN5 battle would follow the
Soul), the Nest, the Secret Area or an Undernet detour. Else BN6's
guardian, as before. Nothing is saved for it: a run continued without
BN5's ROM, or on a build without the second core, meets BN6's guardian
there, as its layers come in BN6's own tiles.

**His band**. BN5's story holds a record for each of them at
V1, V2 and V3, its net one at SP (docs/ROM_DATA.md, BN5's Navis). He is
taken by choice, as BN5's random battles are (their viruses taken up their
versions, docs/PROGRESSION.md): the lowest of V1 to V3 whose HP reaches his
band, V2 at least from act 4 (as BN6's guardians fight EX there) and from
act 2 on threat 4; SP on later cycles. His HP above the band's top is
capped there as he spawns, as the netbattle caps ProtoMan's (docs/
RIVAL.md), never raised: the version brings the attacks, the cap holds the
fight's length. His band is three quarters of BN6's guardians'
(`pacing_xguardian_band`): he is fought in his own engine, where the run's
Cross never comes and the buster is the NaviCust's alone, 10 a charged shot
in act 1. At BN6's band KnightMan, 600 HP behind armor that turns every
blow but while he swings or leaps, outlasted a playtester's whole act-1 kit
by half (session 68), where the act's BN6 guardian, fought with HeatCross,
had fallen in 28 seconds (session 67). The Net Dealer's word on the act's
layers names KnightMan's opening. So every first-cycle guardian lies in his
band (`build.py pacing`):

| Guardian | Act (band) | Version, BN5's HP | Fought at |
| --- | --- | --- | --- |
| KnightMan | 1 (300-450), 2 (450-525) | V1, 600 | 450 / 525 |
| NumberMan | 1, 2 | V1, 600 | 450 / 525 |
| ToadMan | 1 / 2 | V1, 700 | 450 / 525 |
| ShadowMan | 2 / 3 (600-750) | V1, 500 / V2, 700 | 500 / 700 |
| TomahawkMan | 3 / 4 (750-975) | V1, 700 / V2, 900 | 700 / 900 |
| Colonel | 5 (825-1125) | V2, 1200 | 1125 |

BN5's HUD names his version as BN5 does: Colonel's V2 is its DarkCol.

**The fight.** Staged as BN6's: the arena, the door's save, his log-in,
title card and intro in our own words (BN5's text is never copied), his
moves briefed from battle data once met in any run (docs/META.md, what
MegaMan knows), the rival record his own (beside BN6's Colonel and
TomahawkMan, who are other Navis' copies). He stands in his own BN5
overworld sprite and speaks with his own BN5 face, copied in from the
player's BN5 ROM as its bystanders are. The battle is BN5's, from BN5's
own record for him at that version, with its boss theme and the area's
background: MegaMan as the run has him (HP, folder in BN5's chips, buster,
DarkChips), no running, his results screen paying the zenny BN6's
guardians pay where their row holds their chip. Lost, the run ends as at
any guardian.

**His Guardian Data**: his Soul with MegaMan's words, five
HPMemory, a full heal and the NaviCust's draft, as BN6's;
and in his Navi chip's place (BN6 has none of his) a chip of his Soul's
kind that BN6 has, in the folder's code or *, so the Soul unites from
the next battle on: KnightMan's JustcOne, ToadMan's BblWrap, one of
ShadowMan's Invisibl, Mine or AntiDmg, of NumberMan's BusterUp, ColorPt or
DblPoint, of TomahawkMan's Boomer, Lance or Snake, of Colonel's obstacle
chips (RockCube, the TimeBoms, Fanfare and the rest).

## Super bosses (6 October 2026, issue #100)

A player missed super bosses, Bass and the Cybeasts above all. The owner:
not sprinkled in; each comes at a decided time and place in the layers,
with a presentation grander than a guardian's. What the ROMs hold, what
BN6 makes of them, the design reasoned with the game-design skill, and
what was built.

### What the ROMs hold

Verified in BN6 Gregar's ROM (its enemy id table and stats read for every
Navi, its battle records and random tables walked, its sprites drawn with
`--sheet`, its sounds rendered with `--render-song`), in the bn6f
disassembly (Falzar's: its cutscene and map scripts, which start the same
records by index), and in the community's references (docs/SOURCES.md:
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

### The design

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

### Where each stands

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

### How the run foreshadows them

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
  until they have met (docs/META.md, what MegaMan knows); after that,
  "Bass is in there", and his form;
- the Secret Area's arrival, L's word and the room before the arena: a
  dark signal at the end that does not feel like a copy; the second
  screen, "A dark signal waits at the end";
- its arena: BN6's dormant stone stands where a guardian would, its
  screen flickering.

### The presentation, above a guardian's

| | A guardian | A super boss |
| --- | --- | --- |
| Arena | 5 x 5 panels at the end of a bridge | 7 x 7, an octagon, where the layer has room: four more tries at the layout, at its smallest from the third, and a guardian's 5 x 5 in the last five (the Nest's crosses and slabs at their size seldom had room; so, nearly nine layers in ten) |
| Approach | The arena silences the area's theme | The theme fades out as MegaMan steps into the room before the arena, as BN6's own scenes fade it; before the Cybeast, the floor shakes now and then with BN6's rumble (sound `0xE3`); before Bass, his stone stands in the arena |
| Entrance | Bars, the prelude, a log-in with a shake, about 1.3 s | Bars and silence; the rumble every 32 frames, harder each time (BN6's own cadence); Bass's stone shakes and cracks (`0xFE`); the screen fades to white with sound `0x100` and holds; in the white the stone lies broken and the super boss stands; the prelude starts as the white lifts, as in BN6; once it has, Bass throws his cloak open, the Cybeast rears with a roar and a heavy shake, and the card follows. About 4.6 s, 3.3 s at a rematch, which skips the first two rumbles |
| Title card | A band, the area, the name, the epithet | A darkened picture, a taller band with double edges in its colour, the name larger and heavier, its form or the Net beneath, held longer |
| Words | The guardian's line, MegaMan's answer, the battle call | Bass speaks (his face, BN6's), by his record; the Cybeast roars (its face) while MegaMan, Lan and Dad speak; once fought in any run, MegaMan's battle data on their moves, watched in god mode (Bass's cape takes no damage and comes off as he attacks; every attack of the Cybeast's lights its panels first) |
| Fight | BN6's boss theme, the area's background | BN6's own record: Bass's boss theme (`0x16`), the Cybeast's final battle theme (`0x17`), the background of the map it stands on as BN6's records ask (`0xFF`), no running |
| After | Silence, last word, a flash, the log-out | Silence; Bass's last word, a white fade with BN6's `0xD7` as he goes, his stone's pieces fading with him; the Cybeast's last growl, a long white fade with the floor shaking, then the Net stays silent through Dad's call (the CybeastButton unlocked, where the fall gave the run BeastOut) |

### What they pay

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

### As built

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
  DEVTOOLS.md). `tests/test_core.c` (`test_super_bosses`): their forms,
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

### Not built, and the owner's calls

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

### The owner's decisions (6 October 2026)

1. The Cybeast stays as built: Gregar on the first Net, Gregar SP on
   every Net after it, the Net's clock on top as on any guardian.
2. Silence after every guardian's battle stays (the sequence, above).
3. The framing: before the talk MegaMan steps up beside the boss, so the
   chat box no longer covers him wherever the arena is entered from
   above, a scripted move in the staging as BN6's cutscenes move him,
   for every guardian (the sequence, above).
