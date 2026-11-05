---
type: Design
title: "The tournament"
description: "The tournament, an act's climax in BN4's manner: in some acts the arena holds three Navis in a row, two below the act's guardian band and the final at it, MegaMan's HP carried with the arena's heal before the first alone; where and how often, each round's band, a loss, the prize, the card, the entrants, the door's save, beside the rival and the Glitch layer, the owner's six calls, and as built (behind the faces for BN4's screen, `--dev tourney` till then)."
tags: [game, design, tournament, bn4]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-05T17:20:00Z }
sources:
  - id: owner
    resource: "The owner's call 2 of 4 November 2026, docs/multirom/bn4.md, The owner's calls"
    title: "The tournament as an act's climax"
    author: human:saschb2b
  - id: umami
    resource: "umami.saschb2b.com, site Cyberworld Endless: run-start, run-end and guardian events, 6 to 26 October 2026, as docs/game/core-loop.md reads them"
    title: "The players' own statistics"
  - id: code
    resource: "src/core/pacing.c (pacing_guardian_band, pacing_guardian_version, pacing_xguardian_hp), src/core/save.h (Profile.best_depth)"
    title: "The guardian bands and versions as built"
  - id: bn6f
    resource: "~/.cache/mmbn-ref/bn6f: asm/ searched for 'tourn' and 'bracket' (nothing); data/textscript/compressed/CompText87EBDDC.s (a tournament Netopia once held)"
    title: "bn6f, searched for a tournament"
  - id: built
    resource: "src/core/run_tourney.c, src/layer/bracket.c, src/director/boss.c, src/dev/pacing_bracket.c, src/dev/economy_bracket.c; build.py pacing and economy, autopilot runs, 5 November 2026"
    title: "The tournament as built"
---

# The tournament

BN4's tournaments as a run's set piece. The owner's call 2 (4 November
2026, [Battle Network 4 in a run](../multirom/bn4.md), The tournament):
in some acts the arena holds a bracket of three Navis instead of one
guardian, two rounds below the act's band and the final at it, MegaMan's
HP carried from round to round with the arena's heal before the first
alone; the act's card shows the bracket; the prize is the final's
Guardian Data. This page is its design, reasoned with the game-design
skill and checked against [the core loop's review](core-loop.md), with
the owner's calls at the end, and how it was built (As built, below).

## The experience

> "Three of them, one after another, and every hit I take now I'll still
> have taken in the final."

Challenge first (the act's last test, one that can go the other way),
fantasy second (a netbattler's tournament, Lan's own past in BN4, fought
in one sitting), and for a veteran the old guard: BN4's Navis as they
join. The dynamics that make them: one HP bar across three reads, a ramp
inside one climax, and a folder judged on breadth and endurance where a
single guardian judges it on one answer.

What players said of BN4, and what the bracket keeps of it
([Sources](../research/sources.md), Battle Network 4):

- **The fights liked, the pattern around each round not** (an opponent's
  trouble, Lan's errand, then the fight): only the fights come, one after
  another in the arena, with no scenario, no errand and no line between
  them.
- **Random brackets hid the Navis wanted**, and players reset to steer
  them: here the bracket is the run's seed's, fixed before the act
  begins and shown whole on its card; a reset or a CONTINUE changes
  nothing.
- **Three clears to see everything**: BN4's Navis join the bracket's pool
  as each is moved, behind no clear and no version (Red Sun's ROM fights
  Blue Moon's Navis too).

## The dialectic and the loop

- **The dialectic** is the run's, what you bring against what you find:
  one folder, and the HP walked in with, against three Navis shown
  before. It is restated at three scales: the act (the dealer's answer
  for the final against chips that keep HP through the rounds, an
  HPMemory bought or not), the bracket (every hit taken in round 1 is
  still taken in the final), the battle (finish fast and take a hit, or
  slow and whole).
- **The loop layer** is the act's: its climax, feeding the run's (the
  final's Guardian Data, as any guardian's) and the meta only by memory
  (each Navi met, named on later cards, his battle data and rival
  record), never by power.
- **Against the core loop**: it answers the review's second lack, a
  climax that bites (522 of 561 guardian battles won, a median 81 to 85%
  of MegaMan's HP left), and its third, danger in the right places
  (viruses end 23 of 45 lost short nets, spread over layers 1 to 9): the
  danger moves to the act's peak, where the player prepared for it. It
  risks the first, pace: about a minute more in an act that holds one, no
  walk and no word added (The numbers, below). It is not Phase 2's work on
  single guardians (their HP left brought to a band with BN6's own
  versions), which stands; it lands alone, as a change that tightens the
  run must, and its coin makes its own comparison (Measured, below).

## The patterns

- **Rest-point spacing as the dial** (the catalog's bonfire-rest-point:
  "spacing is the whole tuning problem"): the heal before the arena is
  the rest, the three rounds its spacing. One number tunes the bracket
  (the rounds' share of the band); a second (a heal before the final) is
  held in reserve.
- **A sawtooth inside one climax** (sawtooth difficulty): a warm-up, a
  rise, the peak. In act 3 it is the run's own ladder in a row: act 1's
  tier, act 2's, then act 3's.
- **The honest telegraph**: the three Navis and their order on the act's
  card before its first layer, "???" only for a Navi never met in any
  run, as for any guardian.
- **Two bosses as a peak**: Slay the Spire's twentieth Ascension ends its
  third act with two bosses in a row. Here not a rung of the staircase,
  which few climb (62 of 119 players never left threat 0), but an act the
  seed picks.
- **Meta as variety, not power**: the prize is a guardian's; the rounds
  pay their results screens and memory.

The smells, checked:

- *Dominant strategy:* stacking the final's weakness alone (the dealer's
  answer) leaves the rounds, drawn without regard to it, to the rest of
  the folder; a folder of Recovery chips fights each round longer and is
  hit more; a DarkChip in every round costs 60 max HP
  ([DarkChips](meta/darkchips.md): 20 for each battle a dark power ran
  in); a quit in the final restarts the bracket at round 1 (The arena's
  door, below).
- *No opportunity cost:* the folder's thirty chips are the budget, three
  reads to cover.
- *Hollow loop:* the final pays the run, the rounds the memory; nothing
  dead-ends.
- *Complexity over depth:* one rule ("three in a row, the HP carries,
  the heal before the first"); no new screen, item, currency or menu;
  BN6's battle untouched.
- *Treadmill:* the same prize as a guardian's, the bands the act's.
- *Ethics:* nothing to grind, nothing timed.

## The numbers

### Where, and how often

**Recommended (call 1):** the short net's act 3 in half the runs, by a
coin of the run's seed; in the endless net one act a cycle, act 3, 4 or
5, each as likely, by the seed. Never act 1 or 2, act 6 (the Graveyard,
before the Nest), the Nest, the Secret Area, a dark layer's board, or the
act of the run's Glitch layer (Beside the rival and the Glitch layer,
below): there the act keeps its single guardian.

- **Act 3 first**, because the short net's last act before the Nest is
  its peak, and its band is the first to leave room below it for two
  rounds of BN6's Navis at their own HP: act 2's band (600-700) would
  leave its rounds 300-420 and 420-560 HP, BlastMan and DiveMan alone of
  BN6's, the same two every time. Act 1 stays every run's gentle opening
  (its guardian falls 91 times in 95), where half the players decide.
- **Half the runs, not every one**: the bracket stays a peak, not the
  act's shape (the same guardian four runs running is a veteran's
  complaint), and the statistics see a bracket and a single guardian at
  the same act, from the same players.
- **Every endless cycle**: the endless net's runs are winners', whom
  guardians rarely end (ProtoMan ends 24 of 43, Gregar 3), so its deep
  climaxes need the bite most.
- With the Glitch layer's act left out (one run in three holds one, on
  act 2 or 3 in the short net), a short net holds a bracket in about five
  runs in twelve.

**And who (call 2):** a profile meets its first bracket once it has
reached the Nest in any run (`Profile.best_depth` 10 or more, an act 3
guardian beaten); until then act 3 keeps its single guardian. A new
system meets a player gentle or opt-in first (the core loop: the first
rival duel, KnightMan in act 1 and BN5's blank chips each cost a run
before they were tuned), and 63 of 119 players started one run only.

### Each round's band

From the act's guardian band, lo to hi (`pacing_guardian_band`):

| Round | Its band | Its share of the band's middle |
| --- | --- | --- |
| 1 | lo/2 to 3hi/5 | about 55% |
| 2 | 7lo/10 to 4hi/5 | about 75% |
| Final | lo to hi | the act's guardian, drawn as today |

| Act | Round 1 | Round 2 | Final | BN6's Navis that fit the rounds |
| --- | --- | --- | --- | --- |
| 3 | 400-600 | 560-800 | 800-1000, V1 | Round 1: BlastMan 400, DiveMan 500, SpoutMan 600 at V1 (act 1's band exactly). Round 2: SpoutMan 600, HeatMan and CircusMan 700, SlashMan, EraseMan, TenguMan and JudgeMan 800 |
| 4 | 500-780 | 700-1040 | 1000-1300, EX first | Round 1: DiveMan, SpoutMan, HeatMan and CircusMan at V1. Round 2: BlastMan EX 800, DiveMan EX 1000, and the V1s from 700 to 1000 whose EX is past the band |
| 5 | 550-900 | 770-1200 | 1100-1500, EX first | Round 1: BlastMan EX 800, the V1s from SpoutMan's 600 to the 900s. Round 2: BlastMan EX, DiveMan EX, the V1s from 800 to 1000 and Colonel 1200 |

- **Versions** as a guardian's (`pacing_guardian_version`): V1 through
  act 3, EX first from act 4 where it fits the round's band, else V1.
  BN5's Navis at the lowest version whose HP reaches the round's floor,
  held to its top as they spawn (`pacing_xguardian_hp`, as at a
  guardian's); BN4's by the same rule as they come. No Navi of BN6's has
  his HP cut: BN6 scales by choice, and only the other games' Navis,
  whose versions are coarser, are held to a top, as they are now.
- **Later cycles**, where every guardian is SP: the rounds a version
  under, EX (BN5's and BN4's V3).
- **A threat rung's rule** for guardians (EX from act 2) is the final's;
  the rounds keep their bands.
- **Where no Navi fits a round** (none of BN6's acts 3 to 5 today), the
  nearest by HP, as for a guardian's pool.

**What it makes**, projected for act 3 from the statistics (act 3's
guardian: about 900 HP fought in 32 seconds for 19% of MegaMan's HP; the
HP lost taken to grow with the seconds fought, assumed): round 1 about
500 HP, 18 seconds and 11%; round 2 about 680 HP, 24 seconds and 14%;
the final begun at about 75% of max HP and left at about 56%, where a
single guardian leaves 81%. About 2.3 guardians' HP in 74 seconds of
battle, 42 more than the guardian's, about a minute with the rounds'
log-ins. If what a player loses also grows with the HP fought through
(one guardian battle in fourteen lost at one band's worth), about one
bracket in five is lost, and a short net that holds one is won about 39
times in 100 rather than 45 (assumed; the statistics measure it, below).

**The target**, the band the statistics hold it to: the bracket won 75
to 90% of the time, its final begun at a median 60 to 80% of max HP.
Over 90%, the rounds' shares rise by a tenth (to about 65% and 85%);
under 75%, a heal of a quarter before the final (call 3's second answer)
before the shares come down. BN6's battle is never the lever.

### The HP carried, and no heal between

**Recommended (call 3): no heal between the rounds.** The arena's heal
(full, before the arena, as every act's third layer gives it) is the
bracket's one; MegaMan begins each round with the HP the last one's
results screen left him. The folder's Recovery chips, a Soul's UNITE, a
Cross and Beast Out come anew each round, as each round is BN6's battle
with its own Custom screen, so they are the heal the player brings. No
PET between the rounds: the next Navi logs in as the results screen
closes, and the bracket is fought with the folder walked in with.

A heal between every round makes each round a guardian fought at full
HP, three tests of the kind that has stopped testing (81 to 85% left
after each); a heal before the final alone makes the rounds a toll that
ends no run but through the final. Both stay levers if the statistics
ask for them.

### A loss

**Recommended (call 4):** a round lost ends the run, as a guardian lost
does (BN6's GAME OVER), whichever round. The bracket is the act's way on,
as a guardian is, and a climax that cannot end the run does not bite.
The rival's netbattle keeps its own rule (his deleting blow leaves 1 HP,
an optional fight: [The rival](rival.md)); here the stake is said by the
card, as a guardian's is.

### The prize

**Recommended (call 5):** the final's Guardian Data, whole and as any
guardian's: his Navi chip at the version fought, his Soul where BN5 gives
one (and Red Sun, once its Souls are moved), five HPMemory, the full heal
and the NaviCust's draft; a BN4 Navi beaten with dark power gives his DS
chip, as Battle Network 4 in a run plans.

The rounds pay their results screens as a guardian's battle does (his
row's zenny, his Navi chip's entry paid as the act's zenny,
`loot_guardian_zenny`: about 1,260 zenny a battle on average in act 1,
[Measuring it](economy/measuring.md)), by BN6's busting level, so a
clean round pays better and the carried HP is paid for twice. And they
pay memory: each round's Navi counts as met ([What MegaMan
knows](meta/what-megaman-knows.md)), named on later cards, his moves in
MegaMan's battle data, a rival record of his own. No Navi chip a round
(three a bracket would rival the Glitch copy's pick, and Navi chips are
Mega chips, five to a folder), and no cash-out between the rounds (the
Virus Battler's verb).

`build.py economy` runs when it is built: two guardian rows more in
about five act 3s of twelve. Zenny is in surplus for winners already (a
median 17,431 in a won short net), so if the rounds' rows put an act's
zenny out of its band, their chip entries pay the act's battle zenny
instead.

### The card

- **The act's card**, the reveal on the act's first layer as now: the
  area, and in the guardian's place the bracket, three lines in order
  (round 1, round 2, the final), each Navi's name with his version's
  mark, "???" for a Navi never met in any run. A CONTINUE shows it again,
  as the door does.
- **Lan's HP**: beside a portal whose act holds a bracket, MegaMan's read
  of a strong signal reads three, once, its words through
  [the voice rules](../voice/rules.md). The rounds are the act's, so both
  ways read it, and the home shop is the last decision before the act.
- **The dealer** names the final's weakness and stocks his answer, as for
  a guardian: one channel per fact. The rounds are not named again; a
  veteran knows BN6's Navis.
- **Each round's title card** (`cinema.c`'s) marks round 1, round 2 or
  the final over the Navi's name. Rounds 1 and 2 have no intro and no
  last word; the final has a guardian's (his intro keyed to the last
  fight, his last word). MegaMan says one box at the door at most.
- **The screen** (call 6): the card is ours, as the act's card already
  is, since BN6 has no bracket screen (bn6f's code names none: `asm/`
  searched for "tourn" and "bracket", nothing; its text names a
  tournament once, as one Netopia once held). BN4 has its own, being
  mapped beside this design ([BN4 Red Sun's tables](../rom/bn4.md)).
- **The music**: the area's boss theme; where BN4's ROM is read, its
  tournament song (0x17, copied into BN6 and proved with its other 33;
  its role assumed) for rounds 1 and 2, and the boss theme for the final.

### The entrants

- **The final** is the act's guardian as drawn today
  ([The structure](progression/structure.md), Choosing guardians; BN5's
  by his coin where BN5 dresses the area, [BN5's
  Navis](bosses/bn5-navis.md); BN4's as they come to guard BN4's areas),
  so the act's chain (the card, the dealer's answer, the way chosen at
  Lan's HP) is untouched.
- **The rounds** are drawn from the run's seed and the act, never the
  game's RAM, the same two whichever way is taken at Lan's HP, from every
  Navi moved of the games present: BN6's guardians (navis 1-16 and 18,
  ProtoMan's 11 apart), BN5's six (24-29) where its ROM is read, BN4's as
  each is moved and proved (GutsMan first, [BN4's Navis in BN6's
  engine](../multirom/bn4-navis.md)). Each Navi whose HP at a version
  fits the round's band is as likely as any other, as the deep's shared
  pool weighs them; the area's own are not favoured, as a tournament
  gathers Navis from all over.
- **Three apart**: none twice in a bracket, none the run has met as a
  guardian, and not the final, wherever another fits. A round's Navi
  stays free to guard a later act, where his Guardian Data and his Soul
  still come.
- **Never**: ProtoMan (the rival's, Chaud's own), Bass, the Cybeast,
  navi 17, the Darkloids, Duo.
- **With BN6 alone** every round has three Navis or more that fit (the
  table above); BN5's and BN4's widen the pool, and with BN4's eighteen
  moved about half the rounds' Navis would be BN4's.
- **Nothing saved**: the draw is the seed's, so `Run` does not change;
  the arena's layer is made as now, the final standing where the
  guardian stands (`LAYER_MAKE` unchanged, to be checked when built).

### The arena's door

The door is the bracket's only save ([The arena's door](meta/arena-door.md)):
the frame MegaMan steps in, after the heal and before round 1. A quit or
a crash anywhere in the bracket comes back "From the arena's door", with
the card, and round 1 with the HP and chips MegaMan walked in with.
Nothing between the rounds is saved: no `Run` change, and no moment where
a carried HP is frozen for a retry to start from. A crash in the final
costs the rounds' minute, not their HP; a quit wins nothing a loss would
not but the knowledge, as at a guardian, and replays the rounds with
their risk. Not chosen: a save after each round, which makes a quit in
the final a retry of the final at the HP carried, and needs the round in
`Run` (`RUN_MAGIC`).

### Beside the rival and the Glitch layer

| | The rival's duels | The Glitch layer's copy | The tournament |
| --- | --- | --- | --- |
| Its verb | a race against BN6's DeleteTime; at the top a netbattle | one battle, forms swapped mid-fight on one pool of HP | three battles, MegaMan's HP carried |
| Where | each act's duel layer, never a guardian's | a side layer behind a warp, an act's first layer from act 2 | the guardian's arena, acts 3 to 5 |
| Chosen | yes, its choice starting on No | yes, behind a Yes | no: the act's way on, shown on its card |
| Its Navis | ProtoMan, Chaud's | the run's beaten guardians, the last first | Navis the run has not met, the act's guardian last |
| A loss costs | the HP (the netbattle's blow leaves 1) | the run, as any battle | the run, as a guardian |
| It pays | standing and access | a pick of its forms' Navi chips | the final's Guardian Data |

- **The rival** ([The rival](rival.md)) keeps his verb and his layer.
  ProtoMan is never an entrant, so the rivalry's record stays the real
  one's; a netbattle on act 3's duel layer (layer 8) costs the bracket
  nothing, as the heal before the arena on layer 9 comes between.
- **The Glitch copy** ([The Glitch layer](glitch-layer.md)) is memory and
  chaos: the run's own guardians, swapped without a seam, on a chosen
  detour. The bracket is a ramp known before: Navis new to the run, three
  reads, a fresh hand for each. Two set pieces of several Navis in one act
  would be one idea twice, with two Navi chips' prizes in one act, so the
  bracket gives way where a Glitch layer stands. Its act is a fact of
  layer making (`run_glitch_at`) and the bracket's is not, so the bracket
  moves and no layer changes.
- **The Virus Battler** presses on or cashes out at home; the bracket has
  no cash-out. **A Server** is one fight a band up, and chosen.

## Measured

- `build.py pacing` ([Measuring it](progression/measuring.md)): 500
  drawn runs' brackets, each round's Navi in its round's band and the
  final in the act's (0 past), listed per act beside the guardians.
- The statistics: the guardian event with its round (1, 2, the final)
  and MegaMan's HP as each begins; the run-choices event with the
  brackets met and the rounds reached; a run's end naming the round that
  ended it. With the coin, act 3's brackets and act 3's single guardians
  are read side by side, among profiles past the gate, against the target
  above.
- `build.py economy` for the rounds' rows (The prize, above).

## How it would be built

Where it hooks in is built: `run_bracket_at` (`src/core/run_tourney.c`)
answers whether an act's arena holds the bracket, by calls 1 and 2 below
(the short net's act 3 by a coin, one of acts 3 to 5 each endless cycle,
never the Glitch layer's act, once the profile has reached the Nest), and
never in a run whose setup switched the tournament off
([the extras](meta/extras.md)); `test_extras` holds it to them. What
follows calls it.

1. The bracket on BN6's and BN5's Navis behind a dev switch (a bracket in
   every act from 3 that can hold one): the draw from the seed, the card,
   the rounds one after another in the guardian's sequence
   ([The sequence](bosses/sequence.md)), the pacing rows and the
   statistics' round. `build.py test`, `build.py pacing`, captures of each
   round's title card and of the final's Guardian Data, an autopilot run
   through a bracket.
2. Into players' runs, by the owner's call.
3. BN4's Navis join as each is moved and proved, and BN4's tournament
   song where its ROM is read.

## As built

Built on 5 November 2026 as the owner's six calls say, with the
deviations below; behind one gate the owner's call 6 sets, so no player
meets it yet.

**Waiting for the faces.** BN4's screen draws a plate from its own tables,
whose 31 entries end at MegaMan's `0x1E`: an entrant of the faces'
contract (`0x20` + the Navi's number, [Faces for every
entrant](../multirom/bn4-tournament.md)) reads past them, and the screen,
and the map entered again after it, are drawn in stripes (measured with
the dev step `bracket` on layer 9: BN4's own ids draw and return cleanly,
ids from `0x20` do not). So a bracket stands in a player's run only once
`src/emu/xbracket.h` defines `XBR_PLATE_NAVI` (the faces' work, built
apart), and only where BN4's ROM is read, as its screen shows every
bracket (`bracket_here`, `src/layer/bracket.c`). Until then the dev route
`--dev tourney` forces one in every act from the third that can hold one
(the gate and the coin aside, the switch and the Glitch layer's act
honoured: `run_debug_bracket`) and shows BN4's own plates in the places,
so the whole bracket can be played and checked.

**Where it hooks in.** `run_bracket_at` (`src/core/run_tourney.c`) as
before; the draw is `run_bracket_draw` beside it, from the seed, the act
and a pool the layer reads from the ROMs (`src/layer/bracket.c`: BN6's
navis 1-10, 12-16 and 18, BN5's 24-29 where they are fightable and the
older nets are on; BN4's proved Navis on the same terms, below). Each round's
Navi is drawn without regard to the final, then again without him where
the two met, so the rounds are the same whichever way the act took at
Lan's HP unless one was that way's guardian. The rounds' bands, versions
and HP are `pacing_round_band`, `pacing_round_version`,
`pacing_xround_version` and `pacing_xround_hp` (`src/core/pacing.c`).
`Run` and the profile are unchanged (no `RUN_MAGIC`), and so is what a
layer seed makes (no `LAYER_MAKE`): the bracket's two prize scripts join
the layer's text archive only where a bracket stands.

**The sequence** (`src/director/boss.c`; [The sequence](bosses/sequence.md)).
MegaMan steps in, the door saves, he steps up beside the final's place,
and BN4's screen opens with every entrant in. Back on the map: round 1's
title card ("Round 1" over his name), no words, his battle at once. Won:
his Navi chip in BN6's own "MegaMan got:" box (`ta_round_prize`,
`src/layer/reward_words.c`), the bracket with the round settled, round
2 the same way. After round 2's bracket the final logs in as a guardian
does (the prelude, his beam, his card marked "The final", his intro), and
after his battle BN4's screen shows MegaMan the winner of all three
before the final's last word, his log-out and his Guardian Data, whole.
Deviations: rounds 1 and 2 have no sprite on the map (the arena's one
actor is the final's, and only his logs in); the bracket is shown a
fourth time after the final.

**BN4's Navis** join through one list, `src/layer/xguardian_bn4.c`'s
`proved`: a line a Navi (his BN4 V1 id, his plate on BN4's bracket, his
name, epithet and colour), added once his spec PASSes with every mutation
caught ([BN4's Navis in BN6's engine](../multirom/bn4-navis.md)). GutsMan
is the first: navi 35 (`GUARDIAN_BN4_FIRST` 34 + his seat, `src/layer/guardians.h`),
in the pool where BN4's ROM is read, his four versions installed and the
older nets on (`pool_bn4`); BN4's HP ladder (V1 500, V2 900, V3 1300, SP
1700, read from BN4's records) held to a round's top as BN5's are, so V1
in round 1 and V2 held to the band's top in round 2. On BN4's bracket he
stands on BN4's own plate, `0x00`, not a composed face. His battle is his
moved id (0x384 on) at the round's version (`xguardian_bn4_encounter`),
his battle sprite filled into the room BN4's Navis share as its record is
written ([Battle sprites by battle](../multirom/bn4-navis.md#battle-sprites-by-battle)).
He is never a final: the final is the act's guardian as drawn
(`run_guardian`, by the act's area), and BN4's Navis guard no area yet
(BN4's viruses come first, [Battle Network 4 in a run](../multirom/bn4.md)),
so the guardian code draws from no pool he is in; he joins the finals when
BN4's areas bring their guardians.

**The bracket's eight.** BN4's three-round bracket: MegaMan first (`0x1E`),
round 1's Navi beside him, round 2's Navi in the next pair, the final's
in the other half; the four places left are four more of the pool, by
the seed, none of the three, who lose their matches (the second pair's
other to round 2's Navi, the other half's three to the final's and one
of them). Their flags are the rounds settled (`bracket_entrants`): a bit
a round won, in while still in, out the round he fell. The title is The
Den Battle Tournament in act 3, The Eagle Tournament in act 4, The Red Sun
Tournament in act 5.

**The screen's return to the map** (the move's report left it unrun):
it works. BN6's EnterMap puts MegaMan where GameState's PlayerX, Y and Z
say (bn6f `spawnOWPlayerObjectForEnterMap_809e064`), which the leave did
not set, so he came back at the layer's arrival; the bracket now writes
his place there before it opens and puts the old values back after
(`BN6_ENTER_X` to `BN6_ENTER_FACING`, `src/emu/bn6.h`). EnterMap also
clears BN6's no-battle flag (`BN6_FLAG_NO_ENCOUNTERS`) and starts the
map's theme: a random battle began on the first frame back in one run in
four, so random battles are held from the screen's opening to the round's
battle (`emu_encounters_hold`, `src/director/encounter.c`), and the theme
is hushed again.

**HP, a loss, the door.** No heal between rounds: each is BN6's battle
with the HP the last left. A round lost is BN6's GAME OVER and the run's
end, the round's Navi named as what ended it (`boss_foe`). Nothing of the
bracket is saved: a quit or a crash anywhere in it comes back at the
arena's door, round 1 again with the HP walked in with.

**What it pays.** Each round's results screen as a guardian battle's (his
Navi chip's entries the act's guardian zenny, `loot_guardian_zenny`), his
Navi chip at the version fought (`bracket_chip`), and his meeting
(`rival_met`, `rival_result`): named on later cards. Not given: a
first meeting's battle data for a round's Navi (Dad's mail comes with
the Guardian Data only).

**The card and the words.** The act's card lists "1: X   2: Y" and
"Final: Z" in the guardian's place, "???" for a Navi never met (the
final's "?" where a Navi on the net named him), a third line
(`cinema_card_more`). MegaMan's read at a portal whose act holds one:
"Three strong signals,Lan!" and the final by name where he knows him,
else "Whoa... Three strong signals! One after another,Lan..."
(`src/director/home_words.c`). The dealer is unchanged: he names the act's
guardian's weakness, the final's.

**Not built.** BN4's tournament song for the rounds (they play the boss
theme, as a guardian's battle); the statistics' round (the guardian
event names the round's Navi, not the round); the faces (built apart).
For BN4's Navis: a Navi chip of theirs (no BN4 Navi chip plays in BN6's
battles, so GutsMan's round pays its results screen alone,
`bracket_chip`), and a meeting kept (`rivals.sav` keeps navis 0-31, so the
card names him "???" in every run until it grows, which changes its
size).

**Measured.** `build.py pacing`: 0 past their band, the rounds included
(each round's Navis over 500 drawn runs, the short net's act 3 and the
endless net's acts 3 to 5 on two cycles); with GutsMan in the pool (5
November 2026) still 0, he drawn in every round's band: the short net's
act 3 round 1 at V1 63 times in 500, round 2 at V2 37 times. A forced
bracket (`--dev tourney,god,onehit`, seed 6, autopilot from layer 9): BN4's
screen with his plate beside MegaMan's, round 1's card "GutsMan, The
Pounding Fist", his battle (enemy 0x384, 500 HP, his sprite put in the
room), won; round 2 ShadowMan V2, the final GroundMan, won. `build.py economy` (300 runs, a
profile past the Nest, the plan in play): the short net's act 3 14.5 to
15.4 chips and 6,971 to 8,391 zenny a run, the endless net's act 3 12.5
to 13.2 and 5,860 to 6,910, act 4 12.6 to 13.1 and 7,226 to 8,247, act 5
16.7 to 17.3 and 10,751 to 12,101; the bands' centres set to them
(`src/core/economy.c`), the short net's act 3 zenny past its old band.
Autopilot runs (`CYBERWORLD_AUTOPILOT=1`, fresh data folders, the short
net from layer 9 unless said): with the tournament not rolled (a fresh
profile, from layer 7, 9,000 frames, seeds 1 to 4) this build and
`01cb5284` draw the same frames at 3,000, 6,000 and 8,999 and log the same
frame lines; switched off (`--setup short,0,0,0,0,2`) under `--dev
tourney`, no bracket on seeds 1 to 4, act 3's single guardian fought;
forced (`--dev tourney,god,onehit`), seed 1 won all three rounds and went
on to the Nest, seed 3 won both rounds and was deleted in the final
(GAME OVER, the run ended), seed 2's act 3 holds its Glitch layer and kept
its guardian, seed 4's autopilot stuck in a random battle before the arena
(as seed 2's does with the tournament off); placed at the arena with
`fragile`, round 1 lost and the run ended. The door: round 1 won at 70 HP
of 100, round 2 begun at 70; a CONTINUE from the title came back "From the
arena's door", the card ("1: ShadowMan", now met), the bracket, round 1
at 100.

## The owner's calls

Decided, 5 November 2026:

1. **How often: the short net's act 3 in half the runs**, by a coin of
   the seed, and in the endless net one of acts 3 to 5 each cycle; over
   every run's act 3 and over runs with BN4's ROM alone.
2. **From a profile's first run: no, once the profile has reached the
   Nest** in any run; over from the first run.
3. **A heal between the rounds: none**, the full heal before the arena
   the only one; over a quarter or a full heal before the final. Should
   the bracket bite past its target (75 to 90% won), a quarter heal before
   the final is the lever, the owner's to pull.
4. **A round lost: the run ends**, as at a guardian; over Lan pulling
   MegaMan out at 1 HP.
5. **What the rounds pay: each round's Navi chip as well**, beside its
   results screen and memory, the final's Guardian Data the prize; over
   the results screens alone. Three Navi chips an act where a bracket
   stands: `build.py economy` and the Navi chips' prices are to be checked
   against it when the tournament is built, its band in `economy.c` set
   with it.
6. **The bracket's screen: BN4's own, and no tournament until it can show
   every entrant.** BN4's bracket (main mode 0x24, [The tournament's
   screens in BN4](../multirom/bn4-tournament.md)) is moved into BN6 and
   shown for every bracket; its plates hold BN4's own Navis and MegaMan
   alone, so BN6's and BN5's Navis' faces for it are researched first
   (their own games' art, read from the player's ROMs), and the tournament
   waits for them. Over the act's card in our PET look for every bracket,
   and over BN4's screen for BN4's Navis with our card for the rest.
