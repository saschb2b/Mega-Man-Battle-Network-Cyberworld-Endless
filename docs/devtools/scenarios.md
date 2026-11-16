---
type: Tool
title: "Scenarios: the game played through a course, its state checked"
description: "`python3 build.py scenarios` plays each file of `tests/scenarios/` headless (`tools/scenarios.py`, on `tools/cw.py`'s games): a scripted course with the state in words checked on the way, so a bug that only a sequence of play shows stays fixed."
tags: [devtools, tests]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-16T12:45:00Z }
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
and `STEPS` its course:

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
