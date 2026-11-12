---
type: Design
title: "The Battle Lab"
description: "The Battle Lab, a place in Lan's HP to set up any battle and fight it without risk or reward, in the manner of Warframe's Simulacrum: foes opened by battle data gathered across runs (a family complete at 8 deletions), a builder for their versions, panels, field, background and song, the battle fought inside a snapshot of the game; designed with the game-design skill and the owner's eight calls of 11 November 2026."
tags: [game, design, meta]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-12T23:10:00Z }
sources:
  - id: owner
    resource: "The owner's request of 11 November 2026, given in the project's sessions"
    title: "The owner's request for a test-battle builder"
    author: human:saschb2b
  - id: calls
    resource: "The owner's answers of 11 November 2026 to this page's eight questions, given in the project's session"
    title: "The owner's calls"
    author: human:saschb2b
  - id: rom
    resource: "docs/rom/virus-families.md (BN6_DELETE_COUNTS 0x02000070, BN6_DELETE_COUNT 0x0802D644, the rare check at 0x080ABFA6), docs/rom/battle.md (Random battles, byte 7), docs/rom/virus-battler.md"
    title: "BN6's delete counts and its Virus Battler, as the ROM map has them"
  - id: bn6f
    resource: "~/.cache/mmbn-ref/bn6f, data/textscript: every script searched for simulat, virtual, practice, training, virus data"
    title: "bn6f, BN6's text scripts"
  - id: warframe
    resource: "https://wiki.warframe.com/w/Simulacrum and https://wiki.warframe.com/w/Sanctuary_(Cephalon_Simaris), read through a web search's excerpts on 11 November 2026"
    title: "The Warframe wiki on the Simulacrum"
  - id: runlogs
    resource: "1150 local runs that reached layer 9 with ten battles or more, from the run logs under .build/ of the main checkout (autopilot, persona and dev runs; not players')"
    title: "Deletions per run, counted from local run logs"
  - id: groups
    resource: "The Encounter groups concept (game/progression, The engine's room) as its session had written it on 11 November 2026, not yet merged"
    title: "The encounter groups' measured limits"
---

# The Battle Lab

The owner's request (11 November 2026): with 29 families of BN6's, 25 of
BN5's and BN4's growing share in one battle engine, a place to set up any
battle and test yourself against it. A good menu for the enemies, their
versions and positions, the field, the background and the music. Each
enemy opened by deleting it often enough, a gauge that fills across runs.
Inside, a test only: no real death, no rewards. Access earned in the
fiction, and a name of the game's own in place of the working one,
Warframe's "Simulacrum".

Nothing here is built yet. The owner decided its eight calls on 11
November 2026, each as recommended below; they are at the end. Its name
is the **Battle Lab**.

## What the games have

- **Warframe's Simulacrum** (the wiki, read through search excerpts): an
  arena that spawns copies of enemies whose Codex research is complete,
  bought from Cephalon Simaris for standing earned by scanning enemies in
  missions. The pattern is a bestiary that completes by play and then
  lends its entries to a sandbox.
- **BN6 counts deletions per family.** Verified in the ROM
  ([Virus families](../rom/virus-families.md)): a byte per AI index at
  `0x02000070`, raised by `0x0802D644` as a virus of versions 0 to 3 is
  deleted, to 255. Its one reader is the rare battles' check: a record of
  kind 1 waits for the BtlrCard's flag and 16 or 32 of the family deleted
  ([BN6's battles](../rom/battle.md), Random battles). So BN6 itself opens
  content by a family's deletions, at 16 and 32. The count lives in the
  game's save RAM, which a run starts afresh; ours must live in the
  profile.
- **BN6 stores virus data on a card.** Verified in bn6f's scripts: the
  BtlrCard's seller says "you can store virus data inside", and a Rare
  deleted with the card held ends its results with "You got new Virus
  Data" ([The Virus Battler](../world/level-design/virus-battler.md)). The
  card's viruses fight on their own, MegaMan off the field: not a test of
  the folder.
- **BN6 has no free battle.** Verified by a search that came up empty:
  none of bn6f's text scripts holds "simulat" or "virtual"; "practice" is
  a classmate's virus on a blackboard, Mr. Mach's Virus Busting class and
  the Comm menu's Practice NetBattle (a link battle, which
  [the Arena](../devtools/arena.md) runs). **Assumed**: that no other
  game of the series has one either (a web search for a practice or
  training mode found BN3's Virus Breeder in SciLab's Virus Lab, where
  found families are kept and fed, and nothing else).
- **The project already has the parts.** The profile keeps the Library's
  marks, the BtlrCard's families and the guardians' records
  ([What MegaMan knows](meta/what-megaman-knows.md), `rivals.sav`); the
  dev step `foes` starts a battle of named enemies at their panels on a
  chosen field ([A battle on demand](../devtools/battle-on-demand.md));
  ProtoMan's netbattle already ends at the blow that would delete MegaMan
  ([The rival](rival.md), `battle_spare.c`); the core's state can be kept
  and put back in memory (`emu_state_save`, `emu_state_load`).

## Through the game-design skill

- **The dialectic** is the meta layer's, what you bring against what you
  find ([The meta layer's design](meta/design.md)), turned round: here the
  player sets what is found, to learn what to bring.
- **The loop layer** is the meta loop. A run feeds it (deletions become
  battle data), and it feeds the run with nothing but the player's own
  skill and a folder's measure: no chip, zenny, HP or flag leaves it.
- **The patterns**: meta as variety, never power; a bestiary that
  completes by play and lends its entries to a sandbox (Warframe's Codex
  and Simulacrum, Hollow Knight's Hunter's Journal); a training room at
  home (Hades' Skelly); memory as the pull (the lab keeps count, as the
  guardians and Chaud's record do).
- **Against [the core loop](core-loop.md).** It serves lack 5, variety a
  veteran feels, and gives lack 2's "I got better" a test the player sets
  himself. It stands off the main path: no walk (it is at home, beside
  the arrival pad), no words on a layer, nothing a first run must learn.
  The page's rule "no gathering, no crafting gates, no meta currency" is
  the one it brushes: a gauge of deletions is a gate. It stays on the
  right side of it where the gauge fills by playing runs as they are
  played, no run needs what it opens, and nothing is spent.
- **The smells it must survive.** A treadmill: the gauge opens options,
  never numbers. Farming (the ethics test: would the player have more fun
  filling the gauge than not): the threshold is low enough that a family
  met in two or three runs opens by itself, so walking in circles on layer
  1 buys little. A dominant strategy: rehearsing the next guardian at
  home before every act costs only time, so an optimizing player might
  make a chore of it; only foes whose data is complete can be set, which
  are foes he has beaten before, and the statistics' guardian event will
  show whether win rates move. Complexity over depth: one rule for the
  gauge, one screen, BN6's own battle.
- **Resonance.** Knowledge here is data ([What MegaMan
  knows](meta/what-megaman-knows.md)): the PET's battle data on a virus
  exists once it has been fought, and Dad's lab already answers a first
  win over a guardian with a mail of battle data. A room that builds
  copies from complete battle data is that fiction's next step, and the
  run's guardians are copies already.

## The design

### The gauge: battle data

Weighed:

| Shape | For | Against |
| --- | --- | --- |
| **A family's gauge, the versions those met** (recommended) | One number to learn; a version never fought cannot be copied, as the fiction says; BN6 counts by family | A Rare opens with one deletion once the family is complete |
| A gauge per version | "Delete it N times" said of each virus as BN names them | 390 gauges for 65 families; Rares at 3% of battles take dozens of runs each |
| A family's gauge with marks that open V1, V2, V3 in order | The gauge itself shows the way | Fields a version MegaMan never met; four numbers to learn |

Recommended: **a family's battle data is complete at 8 deletions, of any
version, in any run. Complete, the lab can set every version of it
MegaMan has deleted at least once.** Higher versions need more only as
the run makes them: V2 and V3 come in later acts, Rares at 3% of battles
from act 3.

The number, from the local run logs (1150 runs that reached layer 9; a
floor, not players): such a run deletes a median 38 viruses of 11
families, 3 of a family, 9 of the one met most.

| Threshold | Families complete after one such run (median) | A family met in every run opens after |
| --- | --- | --- |
| 4 | 5 | 1 to 2 runs |
| **8** | **1** | **2 to 3 runs** |
| 16 (BN6's own) | 0 | about 5 runs |

BN6's own 16 would ground the number in the game, but no player knows
it: the rare battles' gate is nowhere said. At 8 the first family
completes in a first long run, most families a player keeps meeting in
his first five, and 65 families remain a long tail (520 deletions at the
least, a dozen full runs if every one counted evenly, far more as the
late areas' and the Rares come).

What counts: a virus deleted in a run's own battle (random, the layer's
floor, a Server's challenge, an encounter group's), whether the battle is
then won or lost. Not the Virus Battler's rounds, a NetBattle, a test
battle, nor the week's net (`meta_run_quiet`). Helpers count, as for
every unlock ([Decisions](meta/decisions.md)).

Where it is kept: the profile, a byte per family (to 255) and a byte of
versions met, by the engine's AI index (BN6's 1 to 31, the moved
families' from 32), appended as the Library's later pages were, so an
older profile reads zeros. A run's own deletions wait in the run's tally
beside its checkpoint and join the profile as the run ends, so a
CONTINUE from an older save counts no battle twice.

How a run shows it: not at all on a layer. The run's summary lists the
families whose data the run completed among what it opened (RECORDS,
marked NEW as the setup's options are), the key item's INFORMATION gives
the count ("Battle data on 12 virus families."), and the builder's list
shows every gauge.

### Navis

Recommended: **yes, by the records the profile already keeps, with no
new count.** A guardian is open once his NaviCode is held (deleted twice
as a guardian, [Phase two](meta/phase-two.md)), in each version MegaMan
has beaten; BN5's and BN4's Navis the same way by `rivals.sav`; Bass,
the Cybeast and ProtoMan after a first win. One Navi a battle and no
virus beside him at first: BN6's records never set a guardian beside
viruses, so whether its engine carries that is a thing to measure in the
build, not to promise here.

### Where it lives, and when it opens

Weighed:

| Place | For | Against |
| --- | --- | --- |
| **A keeper in Lan's HP** (recommended) | MegaMan's own home, reached at every run's start and after every act with no walk; the Square's gate already stands there with a keeper, so the HP is where the run's standing services grow | One more thing in a small room |
| A key item that opens it from the PET, anywhere | No place to build | A rehearsal at the arena's door, on any layer; a battle started from inside BN6's menu |
| Class 6-1's blackboard at the Academy | BN6's own place for practice: Mr. Mach's class, the classmate's practice virus | The longest walk at home, the core loop's first lack |
| The title's menu | Outside every run: nothing to protect | Outside the fiction, which the request rules out |

Recommended: **open whenever MegaMan stands in Lan's HP**, before the
first portal and after each act. Mid-run it tests the run's real folder,
which is the point of a benchmark; home is where nothing is at stake
(BN6 heals at the jack-in), so it takes no tension from a layer. What it
adds to a run is time the player chooses, and the chance to try the
run's folder on a guardian already known. The stricter alternative,
before the first portal alone, keeps a run sealed and leaves only the
starting folder to test.

### How it is earned

Weighed: Dad's lab, Chaud (training data for a rival who has earned his
respect), the Academy's NetBattle club (the blackboard's practice
viruses).

Recommended: **Dad's lab, once battle data on three families is
complete** (a second or third run at a threshold of 8). The lab already
reads MegaMan's battles: every guardian first beaten brings Dad's mail
with the battle data. At the next run's start the mail comes ("Lan,mail
from Dad!"), and a Mr.Prog of the lab stands in Lan's HP beside a
terminal. Who knows what: Dad knows what the PET sent him; the Mr.Prog
knows the lab's data and nothing of the Net ahead; MegaMan says only
what he can sense in his own HP, that the copies are data and cannot
delete him. A key item (LabPass, beside NetPass and DarkPass) says what
the lab holds. First meeting three or four boxes, each later talk one
line and the screen, as a shop's.

### The builder's screen

BN6 has no screen for this. Its nearest is the Virus Battler's VIRUS
LIST: a list of held families with their HP and attack, one picked, then
set down on a panel by a cursor, then "Fight with this set?". The
builder takes that flow and the PET's parts ([The PET's
screens](../engine/ui.md), `pet_look.c`); the VIRUS LIST itself cannot
be driven for it (27 of BN6's families, V1 alone, two at most, on
MegaMan's side).

Three pages as Options' tabs, L and R between them, the field always in
sight. As built the field is not drawn at all: it is BN6's own battle of
the set, held still with its field up (Built, The field is the battle's
own), and the sketch below is the first plan's:

```
+--------------------------------------------------+
| <NAME>                              [ 2/4 ]      |  band, the count in its pill
| L [ FOES ] [ FIELD ] [ STAGE ] R                 |  Options' tabs
| +-- the field, 6 x 3, in STATUS's slot ---------+ |
| |  MegaMan's 3 x 3      |   the foes' 3 x 3     | |  BN6's own panels and
| +-----------------------------------------------+ |  sprites, from the ROM
| +-- window: the page's list ------------------+   |
| | > Mettaur      < V2 >      HP  80           |   |  FOES: the families,
| |   Swordy       < V1 >      HP  90           |   |  an unfinished gauge
| |   CanGard      5/8                          |   |  as a count, a family
| |   ??                                        |   |  never met as ??
| +---------------------------------------------+   |
| INFORMATION: Elec. Deleted 23. Best 0:12.40      |  KeyItem's box
|                                    [> BATTLE! ]  |  NaviCust's RUN!
+--------------------------------------------------+
```

- **FOES.** Left and Right turn the version among those met. A lifts the
  foe onto the field, the cursor moves over the foes' nine panels, A
  sets it down, B puts it back; A on a foe set down lifts it again. Four
  at most (below).
- **FIELD.** The cursor walks all eighteen panels; A turns a panel
  through BN6's types (plain, cracked, broken, grass, ice, poison, holy,
  hole, volcano, the conveyors; BN5's lava and sea and BN4's with their
  ROMs), START takes one of the games' own fields whole as a start.
- **STAGE.** The background (BN6's areas', and the older nets' with
  their ROMs) and the song (the Jukebox's list), each a line turned by
  Left and Right.
- **BATTLE!** the one action. No A or B hints; `START` named on the
  band only where it does something unguessed.

The builder opens on the last set fought.

### The battle

- **Inside a snapshot.** As BATTLE! is taken the engine keeps the core's
  whole state in memory; the battle is BN6's own, started as a guardian's
  is forced; when it ends, the state is put back. The run's game is then
  byte for byte what it was: HP, the folder, zenny, flags, BN6's own
  delete counts, the Library, everything. Nothing is counted for it (the
  run log, the tally, the requests, the rival's record), as for a Virus
  Battler's round. The build proves it with hashes of the run's files
  and of the game's RAM before and after.
- **The folder and the NaviCust** are the ones MegaMan holds: the
  starting folder before the first portal, the run's own later, with his
  Cross, Souls and HP as they stand.
- **MegaMan deleted.** The blow that would delete him ends the battle as
  ProtoMan's netbattle ends (`battle_spare.c`); the result names it and
  the builder is back, the same set ready.
- **Leaving.** BN6's own way out: L on the Custom screen, which in a
  test never fails.
- **The result.** BN6's RESULT gives the delete time and the busting
  level; the reward's box stays empty (how BN6's RESULT is shown without
  a reward is a reverse-engineering step of the build). Back in the
  builder the information box holds the set's last and best: time, level,
  HP lost.
- **Records.** Kept for four saved sets in the profile (a set is its
  foes, panels, background and song; the folder is not part of it), so a
  player can come back to "three Swordy3 on ice" and beat his time. No
  online board.

### Limits

From the encounter groups' measuring ([Encounter
groups](progression/encounter-groups.md), The engine's room) and the dev
step:

- **Four foes a battle**: BN6's spawn takes the first four of a record.
- **Any mix of games** among them: every moved family's art is installed
  as a core starts, and four families of three nets were fought in one
  battle and drawn whole. **One of BN4's pooled families a battle**
  (Kilby or MagTect today) until they take fixed song slots, which that
  page's open call 3 proposes; the builder greys the second.
- **A family bound to special panels** is set only on a field that has
  them; the builder says so in the information box.
- **The ROM must be read**: a family of a game whose ROM is absent shows
  as locked by its game, its data kept. A New 3DS reads one extra ROM.
- **A Navi fights alone** until the build has measured otherwise.

### The statistics

One event as a run ends, with the choices: families complete, test
battles fought, sets saved. A course check can then see whether the lab
is used.

## Built

- **The deletions counted** (step 1). A hook on BN6's dying state
  (`BN6_FOE_DYING`, [Virus families](../rom/virus-families.md)) tells the
  director each virus deleted, by AI index, version and side
  (`src/director/events.c`); one of the enemies' in a run's own battle
  joins the run's tally (`src/core/battle_data.c`, `run.data` beside the
  checkpoint), which joins the profile's `data_count` and `data_versions`
  as the run ends (`meta_run_over`), or as the next run begins where a
  run was left without its end. Not counted: a Virus Battler's round, a
  dive of the week's net. The statistics' run-end event carries the
  families complete (`data`). No player sees it yet. An autopilot run
  (seed 3, 15000 frames) counted the eight viruses its run log names,
  BN5's moved families among them.

- **The test battle** (step 2), behind the dev step `lab`
  ([A battle on demand](../devtools/battle-on-demand.md);
  `src/director/lab_battle.c`). Asked for, it begins with the next
  frame's update: the core's whole state with the cart's flash save in
  it (`emu_snapshot`: the pack's entries for chips past 634 live there),
  the director's state and the run are kept, then BN6 fights the battle.
  Its frames are its own: nothing of the director's runs in them, so no
  count, log, record or save sees it. `battle_spare.c`'s blow ends it
  where MegaMan would be deleted, and a GAME OVER that began anyway ends
  it the same way. L on the Custom screen leaves at the first try: BN6's
  escape roll weighs the record's byte 1 against a fifth of MegaMan's max
  HP, and the test's record has 0 there ([BN6's battles](../rom/battle.md),
  Running from a battle). When BN6 is on its map again everything is put
  back, and the update the battle took the place of runs.
  - *Proved headless* (seed 15, Central Area, `--dev quiet`): a battle
    won by the autopilot (96 frames, level 9), one lost to two Mettaurs
    by standing still (deleted at 1966 frames, 99 HP lost, no GAME OVER,
    the run going on) and one left by L. In each the work RAM's hash is
    the same before and after. Against the same run without the step,
    stopped the test's 2652 frames earlier: the same last picture, and
    14 of the 15 save files byte for byte the same (`profile.sav`,
    `run.sav`, `run.data`, `run.tally` among them). `run.state` differs
    in 18 bytes of the emulator's own header (mGBA's PSG channels' event
    timing at 0x130-0x163 and 0x390-0x39F, which any state load derives
    again, a CONTINUE's too); its work RAM, IO, video RAM, palette, OAM
    and flash save are the same.
  - *Not a proof*: an autopilot run with a test battle in it rolls the
    same battles in the same order, but fights them otherwise, since the
    autopilot presses by the real frame's count.
  - The reward's box on BN6's RESULT is not emptied yet: what it shows
    is given and then put back with the rest.

- **The builder's screen** (step 3), behind the dev steps `lab open` and
  `lab all` (`src/director/lab.c`, its picture `lab_draw.c`, its words
  `lab_text.c`). Three pages as Options' tabs, L and R between them:
  - *FOES*: the families MegaMan has battle data on, a complete one in a
    version Left and Right turn, the rest with their gauge ("5/8"). A
    lifts the family onto the foes' nine panels, A sets it down, A on a
    foe set takes it off, B goes back to the list. Four at most; the
    count stands in the band.
  - *FIELD*: the cursor walks the eighteen panels and A turns one
    through BN6's types (plain, cracked, hole, poison, holy, grass, ice,
    volcano, the four conveyors); the row under them takes one of BN6's
    237 fields whole as a start. No hole under MegaMan.
  - *STAGE*: the background, an area of the net's, and the song, BN6's
    from the Jukebox's list or the battle's own.
  - BATTLE! at the foot (Down past the list, or START) fights the set
    as step 2's test battle: its panels written over BN6's last field in
    the core's copy while it is on, then put back. The screen returns
    with the result in the information box (time, busting level, HP
    lost).
  - Not yet: Navis, the foes' sprites on the field (a number marks each),
    BN5's and BN4's panels, backgrounds and songs, the saved sets (all but
    the panels built since, below).

- **The keeper and the LabPass** (step 4), behind `--dev lab`
  (`src/director/lab_keeper.c`, his lines `lab_words.c`, his place
  `src/map/lanhp.c`). A Mr.Prog of Dad's lab stands in Lan's HP (first
  across the room's front from the gate's keeper; moved since, below). His first talk is three boxes, once a
  profile (`Profile.lab_met`); every later one a single box. As the chat
  box closes the builder opens over the held game, and its battle is
  fought there, in Lan's HP: seen headless from the town's jack-in, a
  battle set, left by L, the builder back with the result, the work RAM's
  hash the same before and after. KeyItem lists the LabPass (`0x4C`) with
  the count of families whose data is complete. Still to come: what
  makes him appear for a player (three complete families and Dad's mail).

- **Turned on** (step 5).
  - *The condition.* The Lab is sent (`Profile.lab_sent`) as a run ends
    with the data on three families complete, and the summary says "the
    Battle Lab" among what the run opened; a run left without its end
    sends it as the next begins. The keeper stands in Lan's HP from then
    on. A later run's complete families show on the summary by their
    count.
  - *Dad's mail* ("Battle Lab", `pet_text.c`) comes with the run's other
    mails on its first layer, not in Lan's room as first designed: a
    run's mails are made as a layer begins. It is NEW again with each
    family whose data is complete since, so E-Mail shows the gauge's
    progress in a run.
  - *The sets* (`Profile.lab_sets`, `src/core/lab_set.h`): a fourth
    page, SETS, lists four; A takes one in hand, the one in hand kept as
    a battle starts and as the screen leaves. A win that beats a set's
    best (a higher busting level, or the same in less time) is its best;
    a set changed has none.
  - *The statistics*: the run's choices count the test battles fought
    (`labs`).
  - *Seen headless*, with no dev switch: a run that deleted eight each
    of three families, then a new run begun on the setup screen: the
    left run's data in the profile, the keeper in Lan's HP, his talk,
    the builder listing the three families in the version met, a battle
    won (the result in the information box, the set's best in the
    profile), the SETS page, and Dad's mail in BN6's E-Mail on the first
    layer.
  - *The lab's station* (the owner, 12 November 2026: the keeper stood
    behind a link square that becomes a portal, hard to reach once it is
    one, and looked like any Mr.Prog). It stands in the room's back left
    corner now, the first thing in sight on the way in from the blue pad,
    across the walk from the courier and clear of every link square: the
    keeper at its front, facing the room, behind him BN6's own glass
    capsule (list 7's 61) and its orange monitor of green bars (149).
    Each of the three opens the Lab.
  - *The keeper is a Navi* (the owner's call, 12 November 2026): BN6's
    teal Navi (list 6's `0x41`, by BN6's own variation of that sprite),
    with that sprite's mugshot and lines in a Navi's voice. Chosen over
    BN6's orange armoured Navi, its pink Navi and a Mr.Prog in one of the
    Virus Battlers' colours. A Mr.Prog in the sprite's unused mint
    palette was tried first and read as frozen ("what navi is that? is
    he frozen?"). No one else in a run has this look.
  - A run started by a dev scene named no seed for its tally, so its
    data was lost to the next run; the tally now follows the run's seed
    on every path.

- **Navis** (`src/director/lab_navis.c`), after the virus families on
  the FOES page. A guardian is listed once MegaMan has beaten him and
  open at two wins, as the NaviCode; Bass and the Cybeast at one;
  ProtoMan at a first netbattle won (every third duel won, `duel_won`).
  A Navi not yet open shows his wins, "1/2". His versions are those
  beaten as a guardian, noted from this build on
  (`Profile.navi_beaten`, as a guardian falls); a Navi beaten before
  that note was kept has his first. BN6's guardians come as V1, EX and
  SP, BN5's and BN4's as V1 to SP where their ROM is read, a super boss
  in his forms. One Navi a battle, alone: setting him takes the foes set
  off the field, and no virus joins him ("A Navi fights alone."). His
  battle is his own record's (`make_boss`, `xguardian_encounter`) on the
  set's panels, background and song, the boss theme where none is
  chosen. Seen headless: BlastMan EX set and fought from `lab all`; Bass
  and the Cybeast listed and Bass fought for a profile with a win over
  each (`--dev bass=1,beast=1`); ElecMan V1 noted in the profile after a
  guardian's battle won. Not listed: BN5's Darkloids and Team ProtoMan.

- **The field is the battle's own** (the owner's call, 12 November
  2026: "why not using the actual field and spawn them in? doing nothing
  until the battle starts. then we have the real thing"). The builder
  draws no field. As it opens, and each time the set changes (a foe set
  down or taken off, a panel painted, a
  field taken whole, the background, a set loaded), BN6's own battle of
  the set is begun inside the snapshot and run to the frame before its
  Custom screen opens (`BN6_CUSTOM_OPENING`,
  [The battle](../rom/battle.md)): every foe in at its panel with its
  HP, the panels and the background BN6's, MegaMan on his side. There it
  is held: the scene runs no frame of it (`lab_battle_show`,
  `lab_battle_held`), and BN6's opening is run through at 24 of its
  frames to one shown, so a change stands in about a sixth of a second.
  A set with nobody on it is its field alone: a battle needs a foe, so
  its record names a Mettaur, and it is held 104 frames in, before he
  comes in. The builder's pieces lie over that picture
  (`src/director/lab_draw.c`): the tabs across the top; on FOES, STAGE
  and SETS a panel over MegaMan's side, where BN6's Custom screen will
  stand (the list, five rows; what the cursor's item is, two lines;
  BATTLE! and the count), the foes' side clear; on FIELD a strip under
  the tabs alone, all eighteen panels in sight. The cursor's corners
  mark BN6's own panel (40 by 24 on its screen, the first row's top at
  72). BATTLE! lets the held battle run on (`lab_battle_go`): the next
  thing seen is BN6's Custom screen sliding in over the foes just
  placed. Leaving the builder puts the game back without a fight
  (`lab_battle_leave`). The background is the area's first, not one of
  its two by the dice, so the stage holds still while the set changes;
  the engine's dice are put back as the game is. STAGE's rows name what
  they turn and the information box holds the value, an area's or a
  song's name being wider than a row. Seen headless (`0:lab all`, seed
  15): an empty field; a Mettaur and a Piranha2 set down, each standing
  with its HP at once; a poison panel painted under MegaMan's side; the
  four pages; START into BN6's Custom screen with both foes; L out of
  it, the builder back with "Left at 0:00:00."; B out to the layer. The
  work RAM's hash after leaving is the one before opening (45e79931),
  the run untouched, after the fight and after leaving without one.
  Against the same run without the Lab, stopped at the same count of the
  act's frames: `run.sav`, `run.act`, `run.data` and the layer's files
  the same; `profile.sav` holds the set, `run.tally` the choice counted,
  `run.state` differs in 21 bytes of the core's audio timing in its
  header, its RAM, video and registers the same, and `battle.sav` in one
  byte of its structure's padding, which no build sets. The drawn field with
  the foes' sprites read from the ROM, tried the day before, was dropped
  for this.

- **The stage takes every game's** (`src/director/lab_stage.c`). STAGE's
  background row lists each background once: BN6's areas' first and
  then their second ("Central", "Central 2"; 21 of BN6's 22, an area's
  two were one by the dice before, and several areas share one), then
  the areas of the other games read that a run draws, by their game
  ("BN5 Oran Area", "BN4 Town Area"), their battle backgrounds the ones
  a run's layers there bring into BN6
  ([One net](../multirom/one-net.md)). The song row lists BN6's songs
  and then those of the other games that stand in BN6's song table: the
  themes of their areas, their virus and boss battles and BN5's
  missions ([BN5's music in a run](../multirom/music.md)); a song of
  theirs no run plays is not in the table, so not offered. A set keeps
  the area by its number and the song as its game over its number
  (`src/core/lab_set.h`), so a set kept reads the same whichever ROMs
  are read; with its game's ROM away it is fought on BN6's background
  of the area's place, to the battle's own song. BN6's names are BN6's
  own here, whatever draws the area in the run (the row said "Oran
  Area" for Central's background in a run that dressed Central so).
  Seen headless (`0:lab all`, seed 15, BN5's and BN4's ROMs read): the
  row turned through BN4's and BN5's areas, each background standing
  behind the held field at once; Central's second and BN4's "The Net"
  chosen, START: the battle ran on with background 9 and BN6's song
  0x28 playing, the slot BN4's song 0x12 was copied to; the work RAM's
  hash after leaving the one before opening (45e79931), the run
  untouched. Not here: BN5's and BN4's panels that BN6's engine has no
  type for (sand, metal, magnet, lava), which are a move of their own
  into BN6's battle engine.

## Build order

Each a PR to `dev`, the next begun when the last is merged:

1. The deletions counted: the hook's events, the run's tally, the
   profile's bytes, the summary's line, the statistics' count, the tests.
2. A test battle from a fixed set behind a dev switch: the snapshot, the
   spared blow, the way out, the hashes.
3. The builder's screen.
4. The keeper, the mail, the key item and their lines
   ([Voice](../voice/rules.md)).
5. Turned on; the records and the saved sets.

A sandbox folder from the Library (every chip a profile has held, in
BN6's own folder editor) is the step after, not this build's: it is a
second screen and a second kind of state.

## The owner's calls (11 November 2026)

1. **The name: Battle Lab**, its key item the LabPass (chosen over Test
   Comp, Copy Lab and Mirror Net). Asked again the same day, with names
   tied to the gauge and checked against the series
   ([Sources](../research/sources.md), The Battle Lab): kept, over
   EchoComp (a deleted virus leaves an echo, eight rebuild a copy),
   CopyComp and TraceComp. Not offered: a name with "Ghost" (BN4's ghost
   Navis), with "Mirror" (BN4's Mirror of Truth) or with "Sim" (no word
   of BN6's).
2. **In Lan's HP, open at every visit**: before the first portal and
   after each act (over before a run alone, the PET anywhere, and the
   Academy's blackboard).
3. **Dad's lab gives it by mail**, once battle data on three families is
   complete (over Chaud and the NetBattle club).
4. **A gauge per family, complete at 8 deletions**, the versions those
   MegaMan has deleted at least once (over BN6's own 16, a gauge per
   version, and marks by version).
5. **Navis by the records the profile keeps**: the NaviCode and the
   versions beaten, the super bosses and ProtoMan after a first win; one
   Navi a battle, alone, until the build has measured more (over a gauge
   of their own, and viruses first).
6. **The folder MegaMan holds**, a sandbox folder from the Library the
   step after this build (over the sandbox from the start, and the held
   folder for good).
7. **Four saved sets, each with its best** time, busting level and HP
   lost; no online board (over the last result alone, and none).
8. **Panels painted one by one, the games' own fields as a start** (over
   the games' fields alone, and painting alone).
