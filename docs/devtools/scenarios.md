---
type: Tool
title: "Scenarios: the game played through a course, its state checked"
description: "`python3 build.py scenarios` plays each file of `tests/scenarios/` headless (`tools/scenarios.py`, on `tools/cw.py`'s games): a scripted course with the state in words checked on the way, so a bug that only a sequence of play shows stays fixed."
tags: [devtools, tests]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-16T13:35:00Z }
---

# Scenarios: the game played through a course, its state checked

```bash
python3 build.py scenarios                 # every scenario, four games at a time
python3 build.py scenarios liberation      # those a name starts
python3 build.py scenarios routes --jobs 8 # every route of tools/cw_routes.py
python3 tools/scenarios.py --list          # each with what it guards
python3 tools/scenarios.py --check         # all read without the game (build.py test does)
python3 build.py scenarios --fuzz [SEED]   # runs quit at frames a dice names and CONTINUEd (A quit at any frame)
```

The ROM-free tests reach what is a function of its inputs: a layer from
its seed, a save from its bytes. The run on the live game they do not
reach, and that is where the bugs players report are. Of 0.22.0's 21
fixes, 14 were of a sequence: six a run that came back changed from a
quit or a CONTINUE, four a button that fell through one screen into the
next, four an order between the engine and BN6. Issue #669 was one more:
MegaMan deleted in a liberation's battle, the next run's battles all ran
by its rules. Each was reproduced by hand, by a play scripted for the fix
and thrown away after it. A scenario is that play, kept.

It needs the ROM, so it is local, not CI's, like the captures and the
autopilot runs: the Engine code row of `AGENTS.md` runs it. The 27 that
BN6's ROM alone plays take six seconds here, six games at a time; a game
runs headless at about a thousand frames a second.

## A scenario

A file `tests/scenarios/NAME.py`. Its docstring says what it guards, the
issue first where there is one. `START` is the game's start as
[the agents' tools](agents-tools.md) take it (a route, the game's
options, `VAR=VALUE` first for its environment), `NEEDS` the other games'
ROMs it plays with beside BN6's (`'bn5'`, `'bn4'`; skipped without them),
`KNOWN` a bug it shows that is not fixed yet (below), and `STEPS` its
course:

| Step | Does |
| --- | --- |
| `('do', COMMANDS)` | [`tools/play.py`](play.md)'s commands: `press A; wait 60; hold UP 30`, the dev steps among them |
| `('until', COND, FRAMES)` | the game runs on until the state says so, 6000 frames at most where none is named; a failure where it never does |
| `('repeat', COMMANDS, COND, TIMES)` | `COMMANDS` again and again until `COND`, 12 times at most: a button pressed through screens whose timing is their own (the title's menu opens a moment after START) |
| `('expect', COND)` | the state says so now |
| `('keep', WORDS)`, `('same', WORDS)` | the state's lines of those first words kept, and the same later: what a CONTINUE must bring back |
| `('restart', START)` | the game quit as a player's system quits it (SIGTERM, the port's close: it saves and leaves), then started again on the same saves |

`COND` is an until's test of the agents' tools: a state line's first word,
an operator (`=`, `!=`, `>`, `>=`, `<`, `<=`, `~` contains) and a value,
several apart by `|` any of them. Issue #669's, whole:

```python
START = 'CYBERWORLD_AUTOPILOT=1 --scene emu --seed 5 --run-depth 5 --dev dark,fragile'

STEPS = [
    ('until', 'rules=liberation', 4000),      # the board's first battle, a liberation's
    ('until', 'scene=title', 8000),           # deleted in it: the run over
    ('expect', 'title=summary'),
    ('repeat', 'press A; wait 40', 'title=start'),
    ('repeat', 'press START; wait 40', 'title=menu'),
    ('expect', 'cursor=new'),
    ('repeat', 'press A; wait 40', 'title=setup|title=leaving|scene=emu'),
    ('repeat', 'press START; wait 60', 'scene=emu'),
    ('until', 'screen=battle', 20000),        # the next run's first battle
    ('expect', 'rules=plain'),
]
```

Each game is given a folder of BN6's ROM and those its scenario needs, no
other (links under `.build/`), so a scenario is the same game on every
machine: with BN5's ROM beside BN6's a run's areas change, and a dark
layer's board is BN5's with a team, where #669's course, written for
BN6's, never reached its deletion.

A file may hold many instead, `MANY = {name: dict(START=..., STEPS=...)}`:
`routes.py` makes a scenario of every route of `tools/cw_routes.py`, each
expected on the screen it names.

A failure names the step, says the state it found, and keeps that state,
the game's picture and its log in `.build/scenarios/NAME/`:

```
FAIL liberation-rules-end    3.4 s    4593 frames
     step 10 ('expect', 'rules=plain'): the state does not say so
     state: scene emu · screen battle · ... · turn 0 · rules liberation · layer 1 · area RoboDog Comp
     kept: .build/scenarios/liberation-rules-end/fail.txt, .../fail.png, .../log.txt
```

## Writing one for a fix

1. Reproduce the bug with the agents' tools' `play`, as before. The
   commands that got there are the scenario's steps.
2. Make what went wrong a line of the state where it is not one: a
   scenario reads words, not pictures. Issue #669 had none, so a battle's
   state now says `turn N` and `rules plain` or `rules liberation`
   (`lib_battle_describe`), and the title's its step and its menu's row
   (`title start`, `opening`, `menu`, `confirm`, `setup`, `summary`,
   `leaving`; `cursor new`, `continue`, `other`), which every course that
   crosses the title uses.
3. Run it on the code before the fix: it fails at the step that names the
   bug, not earlier. Then on the fix. A scenario never seen red guards
   nothing; #669's was run with both of its fix's disarms taken out.
4. Prefer `until` and `repeat` to counted waits: a wait counted in frames
   breaks when a fade grows by ten.

## A bug a scenario finds

A scenario written for a journey that turns out broken is the bug's
report and its test at once. Where the fix is small and sure it goes in
the same pull request, the scenario seen red before it. Where it is not,
the scenario is kept with `KNOWN = 'what is wrong (issue #N)'`: it is
played with the rest and reported `KNOWN` with that line, no failure of
the run, so the suite stays green and the bug stays in sight. Once the
bug is fixed the scenario passes, which fails the run until its `KNOWN`
is taken out: the fix's pull request does both.

```
KNOWN home-shop-buys-a-chip    1.2 s     640 frames
     the shop's last row sells at the first row's price (issue #700)
     step 7 ('expect', 'zenny=1200'): the state does not say so
```

## A quit at any frame

```bash
python3 build.py scenarios --fuzz              # 8 games of 4 quits each, from a seed drawn and printed
python3 build.py scenarios --fuzz 7            # the same games every time
python3 build.py scenarios --fuzz 7/3          # game 3 of them alone: a failure's line says this
python3 build.py scenarios --fuzz 7 --games 24 --quits 6 --jobs 6
```

Six of 0.22.0's fixes were one kind: a run that came back changed from a
quit and a CONTINUE, each found by a playtester, one at a time. Two
scenarios of fixed course guard two of them. `tools/scenarios_fuzz.py`
hunts the kind: a run is played by the autopilot, quit as a player's
system quits it (SIGTERM) at a frame a seeded dice names, started again
at the title, CONTINUE taken, and what came back is held to what the
state said at the quit; then it is played on to the next quit.
Everything comes from the fuzz's seed: each game's own seed, the layer it
starts on (1 to 9), its autopilot (`weak` twice in three: every battle
won at once, so more of the map, the guardians and the way home) and
each quit's frame. Half the quits wait for the next edge first (the
screen, a box, a battle's phase, the layer, a checkpoint saved, the
engine's word on the quit changing) and come up to 45 frames after it:
the bugs of this kind lived in the seconds around one. Eight games take
about twenty seconds here.

### What a CONTINUE brings back

A quit saves once more where it can (`scene_emu.c`'s `emu_keep_run`,
`director_save.c`'s `director_suspend`; [the layer](../engine/emulation/layer.md)
has the reasons), and where it cannot, the last checkpoint stands. So a
CONTINUE is not held to "every line as it was" but to the situation the
quit found the run in, told from the state as a player sees it:

| The quit finds | The engine keeps | A CONTINUE must bring back |
| --- | --- | --- |
| **map**: MegaMan on a layer's map, no box open | the run where he stands, once he is free; held by BN6 on a layer's exit (its warp, the jack-in, the arrival before the next layer's checkpoint), the game run on unseen to him free on the next layer | `layer`, `area`, `zenny`, `bugfrags`, `hp`, `scrtdata` and `guardian` as they were; the next layer where the exit was taken |
| **won**: a won battle, from its RESULT through the fade to the map | the game run on to him free, A paging the RESULT | the same layer (the next where the battle was fought on the exit), `zenny` and `bugfrags` no less: the reward may come after the quit's frame. Not `hp`: a battle's is its HUD's |
| **box**: a chat box, a shop, the PET, a reward's pick open | the run as it stood on the last frame MegaMan was free before it opened (the held set) | each line as it was at some moment between the map and the quit: what the box gave may be given again, nothing older is lost |
| **battle**: a battle under way, to its RESULT | nothing new | the last checkpoint saved, on this layer: the battle is not kept, and neither is what was won since that checkpoint |
| **guardian**: his staging, his fight and his end, to his Guardian Data | nothing new | the last checkpoint saved: the arena's door, or the Guardian Data once it stands |
| **home**: in the town | nothing new | the last checkpoint saved: Lan's HP's or the PET's Save's; before the HP's, the layer left, at its Guardian Data |

and after every CONTINUE the game goes on: MegaMan is free within 900
frames (issue #23's kind: a state that came back held), and the game's
own frame count never stands for 600 (issue #482's).

What may change, then, and is not a bug: a battle under way is fought
again, and the battles won before it on that layer since its checkpoint
with it (the quit prompt says "Run saved at the layer's start" there); a
guardian's fight starts again at his door; a talk is heard again; the
town before its save is walked to again. What the profile keeps a quit
does not take back, so a guardian met in a fight not kept is named after
it where he was `???` before. Two things the fuzz does not read: where
MegaMan stands, and the folder.

The situations are the fuzz's own reading (`situation`), not the
engine's word on what a quit keeps: 0.22.0's won battle was lost exactly
where the engine said "Run saved at the layer's start". Three state lines
were added for it:

| Line | Says |
| --- | --- |
| `quit here`, `after`, `talk`, `pet`, `checkpoint` | what a quit now would keep, by the engine (`director_quit_describe`): the run where MegaMan stands; after the game is run on (a won battle, a layer's exit); as it stood before the talk or the PET; the last checkpoint. Printed with a failure, the place to start reading from |
| `saves N` | the checkpoints saved since the game started: the fuzz keeps the states either side of the last, which is what "the last checkpoint" is held to |
| `battlestep start`, `custom`, `fight`, `won`, `over` | a battle's phase; `won` from its RESULT, where the engine's keeping begins. BN6's phase goes back under the Custom screen's as the battle fades out, so the fuzz remembers a `won` until MegaMan is free |

A failure says the quit, what was kept and found, and how to play that
game again; both states, the picture and each start's log stay in
`.build/scenarios/fuzz-SEED-GAME/`:

```
FAIL  3/11   5.6 s    4850 frames  --seed 52764 --run-depth 7, autopilot weak: map map won
     quit 3 (won, frame 2448 of its start; the engine: quit checkpoint): zenny 400 kept, 0 found
     kept:  screen battle · where layer · doing battle · ... · battlestep won · quit checkpoint · saves 0 · layer 8 · zenny 400 · ...
     found: screen map · where layer · doing map · ... · quit here · saves 0 · layer 8 · zenny 0 · ...
     files: .build/scenarios/fuzz-3-11/fail.txt, .../fail.png, .../log.txt (log-N.txt: each start's)
     again: python3 build.py scenarios --fuzz 3/11
```

That one is 0.22.0's quit after a won battle, with its fix taken out
(`director_settles` without its won battle): game 11 of each of seeds 1,
2 and 3 found it, as zenny lost on a RESULT and in the fade after it,
and all three pass with the fix in. A fuzz
never seen red guards nothing either.

### What it found

- **A quit as a layer's exit was taken lost the layer left.** From the
  exit pad's warp to the next layer's checkpoint, some two seconds, BN6
  holds MegaMan and no save could be made, so a quit there (the most
  natural place to stop) went back to the left layer's last checkpoint:
  its battles and what they paid gone. Found by the first run, seed 1's
  game 4, and by ten of the first 72 games. Fixed with it: the quit runs
  the game on to MegaMan free on the next layer, as it does after a won
  battle (`director_settle.c`'s `arriving`), and
  `tests/scenarios/quit_as_a_layer_is_left.py` keeps the play.
- **A liberation's words after a won battle have no save before them**
  (`--fuzz 3/12`): on a dark layer's board the Navi's line begins on the
  way back from the battle, while BN6 still holds MegaMan, where the run
  cannot be held back before it as it is before
  [a talk on the map](../engine/emulation/layer.md), and a quit under it
  goes back to the last checkpoint, the battle and its reward lost. Not fixed here: the board's steps are its own. It is
  in the fuzz's `KNOWN`, below.
- **A quit heard between two batches was not heard.** The headless
  game's wait for its next batch is a `read` the signal wakes; a signal
  that came after a batch's answer and before that `read` woke nothing,
  and the game waited on, one quit in some hundreds with the machine
  busy. A harness's race, not a player's: `quit_game` sends the signal
  again each second.

A bug shown and not fixed yet goes in `KNOWN` of
`tools/scenarios_fuzz.py`: the situation and the lines of the kept state
that tell it, and what is wrong. A CONTINUE that fails so is printed
`KNOWN` with the seed that shows it, fails no run, and the game is
played on; the entry goes with the fix. A new find is a seed: the line
`again:` in an issue is the whole report, and where its fix is small its
play becomes a scenario of fixed course.

## Many written side by side

Several sessions add scenarios at once, a claim each
([Claims](../project/claims.md): `scenarios-AREA.json`, its unit the
area). They stay out of each other's way so:

- A session's files carry its area's prefix (`home_*.py`, `layer_*.py`),
  and it writes no other's.
- No page lists the scenarios: `tools/scenarios.py --list` does, from
  their docstrings. A session's pull request adds its files and its
  `docs/log.d/` entry, and edits this page only for a new step or a new
  state line.
- A new state line is the feature's own: a function in its module that
  writes its lines (`lib_battle_describe` in `liberation_battle.c`),
  called from `director_describe` without a condition of its own, in one
  line; that function stands at lint's limit of branches, and two
  sessions' lines beside each other there merge by hand in a moment.
  The line's first word is new, its value one word where a test reads it.
- The runner's steps are what there is. A step the area cannot do without
  is added small, in a function of its own, and named in the pull
  request.
- Seeds and `--dev` switches make a course short and the same every time:
  a scenario that takes more than twenty seconds, or passes only most of
  the time, is not done. Ten runs in a row pass before it is committed.

## Battles and the run's spine in the state

The `battle_*.py` and `run_*.py` scenarios read these, each written by
its own part and called from `director_describe` (the summary's from
`capture.c`'s state, after the title's):

| Lines | From | Say |
| --- | --- | --- |
| `viruses N`, `navis N`, `battles N`; in a battle `foes N`, `down N` | `tally_describe` (`director_tally.c`) | the run's tally as RESULT will count it, the layer's random battles fought, a battle's enemies and those down |
| `battle random`, `guardian`, `duel`, `gate`, `copy`, `server` or `floor`; `phase custom`, `fight`, `results` or `other`; `form none`, `HeatCross` ... `BeastOut`; `foestatus none`, `paralysed`, `dragged`, `frozen` or `bubbled`; at its results `result won`, `lost`, `ran` or `draw` and `reward zenny N`, `chip ID CODE`, `hp N`, `bugfrags N`; on the map after one `lastbattle won` ... | `battle_state_describe` (`battle_state.c`) | a battle: what it is, where it stands, how it ended and what its RESULT paid, MegaMan's form, the statuses its enemies are held in |
| `version N`, `known yes` or `no`, `data hidden`, `fresh`, `waiting` or `taken`, `exitpad open` or `shut`; in his fight `guardianhp N` | `boss_describe` (`boss.c`), beside `guardian NAME waiting` ... | the guardian's course: his version, whether MegaMan has battle data on him, his Guardian Data (`fresh` its first second, when no A takes it) and the exit pad |
| `duel none`, `waiting`, `on` or `fought`; `rung N`; `duelswon N`, `duelslost N`, `clearance N`; `chaud new` or `known`; `verdict none`, `beat`, `slow`, `hit` or `lost` | `duel_describe` (`director_duel.c`) | the rival: the layer's duel and its rung, the record, what his respect has opened, whether Chaud has called before, the last verdict |
| `pick gift`, `draft`, `vault`, `order`, `official`, `server` or `copy`; `offers hpmemory:2 chip:137 program:1 [bugfrags]`; `pickrows N`, `pickrow N`, `pickready yes` or `no` | `director_pick_describe` (`reward_pick.c`) | a reward pick while it is up: its rows, the cursor's, whether an A would take one yet |
| `card title`, `area` or `grand`; `cardsays TOP / NAME / LINE / LINE / LINE` | `cinema_describe` (`cinema.c`) | the card that shows: a guardian's title, the act's ("Guardian: ???") |
| `page result` or `records`; `ended deleted` or `won`; `cause in RoboDog Comp`; `reached N`, `viruses N`, `navis N`; `learned NAME` or `none`; `newbest yes` or `no`; `opened the endless net, threat 1` or `nothing` | `title_summary_describe` (`title_summary.c`) | the finished run's summary over the title, both its screens' facts while it shows |

A drop-in start (`--scene emu`) is no player's arrival: the `battle` and
`foes` steps given in its first frames begin their battle during the
jack-in ([A battle on demand](battle-on-demand.md)), so a course pages
the arrival's words first, and with the autopilot waits for `chat=open`
and then `held=none` (the state's with `CYBERWORLD_STATE_POS=1`). A test
start's area and guardian (`--net-biome`, `--guardian`) are given again
at a `restart`: a CONTINUE from the title keeps the run's own, and
`--guardian`'s older-net Navi is no CONTINUE's.

## What they found at once

The first run of `routes` failed three: `layer` and `battle` stopped on
the run's opening gift, which had come to stand before them, and
`pet/options` named a screen since renamed. Every capture made from those
routes had been of the wrong screen, said only by a line of the answer
nobody read. The routes are fixed, and stay so.

The battles' scenarios found one in the game: MegaMan deleted in a
battle, the next run begun without the game closed came onto its first
layer as if back from that battle, and its tally began with the viruses
deleted in it (`battle_deleted.py`; the battle now ends with the run).

## Beside it

[An old checkpoint](old-checkpoint.md) is a scenario of its own shape:
the last releases' own builds save a run, this build continues it. It
stays the Release row's, as it builds other commits. The scenarios play
one build.
