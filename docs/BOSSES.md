# Guardians

How a guardian Navi is met, fought and left behind, from the approach to the
next area. The structure follows how Hades stages its bosses; everything on
screen is the game's own (NPC and text commands, mugshots, songs, sounds)
or drawn over its frame by `src/director/cinema.c`.

## What Hades does, and what was taken

| Hades | Here |
| --- | --- |
| The room before a boss is safe: Charon's shop, often a fountain | The antechamber, the room the arena's bridge leaves from, holds a heal and the Net Dealer (or a room near it, where it has no counter's place); stepping in, MegaMan names those not yet used and which way each is, since the arrow leads past them. While the guardian waits no random battle starts there, on the bridge or a panel round the arena (a playtester met one on the bridge, right after MegaMan's "no running from a guardian", session 64) |
| The boss chamber is an arena of its own, locked until the fight is over | A square arena at the end of one bridge, drawn in the area's second floor; the exit pad inside it stays hidden and shut |
| The music drops to ambience on approach, the boss theme starts with the fight | Entering the arena silences the area's theme; the boss prelude (song 0x1C) plays through the intro, the game's boss theme in the battle |
| Short intro dialogue with portraits, chosen by history (first meeting, who won last) | Lines with the guardian's mugshot for a first meeting (Lan or MegaMan naming the copy first), a rematch, revenge after beating MegaMan, a stronger version, and grudging respect after many losses; MegaMan answers now and then; the Nest's own guardian says what it is; Lan's "Battle routine, set!" and MegaMan's "Execute!" close it |
| The boss's name and epithet on its health bar | A title card: the area it guards, its name large (with EX or SP for stronger versions), its epithet; the game's own battle shows its name too |
| No question before the fight | The battle starts after the last line |
| A defeat line, a flash, the boss leaves | A last word, a white flash, and the guardian logs out, fading away |
| A reward to walk up to; the exit opens | Its Guardian Data materializes where it stood (its Cross or BeastOut with MegaMan's and Dad's words, five HPMemory, its own Navi chip at the version fought, a full heal); taking it makes the exit pad appear |
| Stairs into the next region, its name on screen | An area-clear card (guardian, viruses, time) over the jack-out, then the next area's title card |
| Bosses remember runs | `rivals.sav` counts meetings and who won each battle, per Navi |
| The Codex fills as you meet things | MegaMan briefs a guardian's moves, and when a hit lands, only once they have fought that copy in any run (its `rivals.sav` record); a first meeting says they have no battle data, and to watch the yellow panels (docs/META.md, what MegaMan knows) |

Not taken: arenas that change during the fight and extra phases, which
would mean changing the game's battles themselves.

## The sequence (`src/director/boss.c`)

| State | What happens |
| --- | --- |
| Wait | The guardian's NPC waits hidden in the arena's middle |
| Enter | MegaMan steps into the arena: bars slide in, the music stops, MegaMan walks up; the boss prelude starts, the guardian logs in (its animation 25 and sound 0x77) and the screen shakes |
| Title | The title card, while Gregar's own Navis strike their signature pose (animation 24) |
| Talk | The intro lines; input is A and B only |
| Fight | The game's navi battle, forced at once (the random battle record is not re-rolled over it) |
| After | Back on the map: bars, silence |
| Last word | Its defeat line |
| Log out | A flash; the guardian plays the log-out sound (0x76) and fades by its alpha before it is freed |
| Reward | The Guardian Data shows (sound 0x94); the area's theme returns and MegaMan walks to it |
| Open | Taken: the exit pad appears and its warp works |

Event flags `0x1448`-`0x144C` drive the NPCs: the guardian leaves, logs in,
the Guardian Data shows, it was taken, the exit shows. While the exit is
shut, flag `0x16F1` keeps its warp from starting.

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
17), each with a rival record of his own. Index 17 (and 0, 22) is an unnamed navi with 4000 HP at
V1, which the Graveyard's SP battle uses; runs saved before Colonel moved
to 18 have their 17 changed on loading, and his rival record follows.

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
wakes the Cybeast in MegaMan for the Nest.

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
