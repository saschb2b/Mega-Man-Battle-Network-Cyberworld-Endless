---
type: Tool
title: "Scenarios: the game played through a course, its state checked"
description: "`python3 build.py scenarios` plays each file of `tests/scenarios/` headless (`tools/scenarios.py`, on `tools/cw.py`'s games): a scripted course with the state in words checked on the way, so a bug that only a sequence of play shows stays fixed."
tags: [devtools, tests]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-16T13:02:00Z }
---

# Scenarios: the game played through a course, its state checked

```bash
python3 build.py scenarios                 # every scenario, four games at a time
python3 build.py scenarios liberation      # those a name starts
python3 build.py scenarios routes --jobs 8 # every route of tools/cw_routes.py
python3 tools/scenarios.py --list          # each with what it guards
python3 tools/scenarios.py --check         # all read without the game (build.py test does)
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

## The extras

The extras' scenarios (`extras_*.py`: the Glitch layer, the tournament,
the Virus Battler, the Undernet's door and the Secret Area, DarkChips,
and each of them switched off) reach what they play so, with the steps
there are:

- **A talk** is opened by the start's `--talk NAME:FRAME`
  ([a capture's options](headless-shot.md): `glitch`, `copy`, `battler`,
  `undernet`, `gate`, `dark`; `zenny`, `bugfrag` and `scrtdata` give what
  a talk wants), after the arrival's words are paged, and waited for with
  `('until', 'chat=open')`. A later talk on the layer is a second entry
  (`copy:800,copy:2600`).
- **Its answer.** The run's own risky questions start on No, so
  `('repeat', 'press LEFT; press A; wait 20', COND)` answers Yes and
  `press A` alone No, each until the state says what the answer did
  (`sidelayer=glitch`, `series=on`, `flame=taken`). The golden gate's
  starts on Yes until Bass is known behind it.
- **A battle won** by the dev menu's fifth row ([Dev menu](dev-menu.md)),
  once the Custom screen's OK has started the fight (`custom>0`); the
  Glitch copy's forms one opening each. The Virus Battler's rounds are
  fought by the viruses, the VIRUS LIST read from `vbscreen`, `vbpicked`
  and `vbplaced`.
- **The walk to an exit pad** is the autopilot's: the run quit and
  continued (`('restart', 'CYBERWORLD_AUTOPILOT=1 --scene title --dev
  quiet')`, CONTINUE), which also proves the side layer comes back from
  a quit as it was left.
- **A Glitch layer's data taken** are BN6's flags set by the `flags`
  step (`MAPSLOT_MD_FLAG`, 0x1400 on), as taking them sets them.
- **The tournament** needs BN4's ROM (`NEEDS = ('bn4',)`), whose screen
  shows every bracket; its scenarios switch the older nets off, so its
  Navis are BN6's.

| The state says | Where | Written by |
| --- | --- | --- |
| `sidelayer none`, `undernet`, `secret`, `back` or `glitch`; `darkwarp here` or `none`, `goldgate here` or `none` | a layer | `extras_describe.c`, which calls the four below |
| `glitchwarp here` or `none`; on a Glitch layer `glitchstage N` (0-4), `glitchshare N` (its data taken, in hundredths), `glitchsaid N`, `glitchlooks N`, `glitchflips N`, `glitchfx none`, `tear`, `slip`, `turn` or `flip`, `glitchlook own` or the look's number, `glitchbattles own`, `mixed` or `deeper`, `glitched on` or `settled`, `copy waiting`, `fighting`, `deleted` (its pick waits) or `taken` | a layer | `glitch_describe`, `glitch.c` |
| `bracket none`, `ahead`, `door` (MegaMan stepped in: its one save), `screen` (BN4's, off the map), `round` or `won`; `round N` (3 the final, 4 all won), `roundfoe NAME`, `roundchips N` (the two rounds' Navi chips held) | a layer | `boss_bracket_describe`, `boss.c` |
| `btlrcard held` or `none`, `virusdata N`; `battler N` or `none`, `stake N`, `vbrounds N`, `series none`, `on` or `over`, `vbround N`, `vbwins N`, `pot N`, `vblast none`, `won`, `lost` or `draw`, `vbprize open` or `taken`; after a round won `potchoice up`; in a round `vbscreen list`, `ok`, `placing`, `confirm` or `fight`, `vbpicked N`, `vbplaced N` | a layer | `battler_describe`, `director_battler.c` |
| `flame none`, `burning` or `taken`, `flamechip NAME`, `flamenet bn6` or `bn5`; `darkchips N` (kinds the run holds), `darkpack N` (their chips in the Pack and folder), `basehp N` (the max HP a DarkChip's price lowers), `darktaught none`, or `flame`, `price`, `bn5` joined by `+` (the profile's) | a layer | `dark_describe`, `director_dark.c` |

They found one bug at once: a Glitch layer whose copy was deleted
glitched again after a quit and CONTINUE, its flips and stronger
battles back for a copy that was gone (the settled state was the
session's alone; BN6's flag says it now, `glitch_update`).

## What they found at once

The first run of `routes` failed three: `layer` and `battle` stopped on
the run's opening gift, which had come to stand before them, and
`pet/options` named a screen since renamed. Every capture made from those
routes had been of the wrong screen, said only by a line of the answer
nobody read. The routes are fixed, and stay so.

## Beside it

[An old checkpoint](old-checkpoint.md) is a scenario of its own shape:
the last releases' own builds save a run, this build continues it. It
stays the Release row's, as it builds other commits. The scenarios play
one build.
