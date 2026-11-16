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

### A game's own places

A scenario of the screens outside a run needs more of its machine than a
run does: a second data folder to import into, a Downloads of its own, a
ROM folder with nothing in it. In a `START` and in `COMMANDS` the runner
puts them in:

| Written | Is |
| --- | --- |
| `{dir}` | the game's own folder under `.build/scenarios/`, new each time: `--data-dir {dir}/other` is a second device's saves, `HOME={dir}` gives the game a Downloads of its own (SAVES writes its file there) and keeps the ROMs screen from finding this machine's ROMs where downloads and front ends keep theirs |
| `{bn6}`, `{bn5}`, `{bn4}` | a ROM's file, alone in a folder (one dropped with another game's beside it brings that along) |
| `{roms}` | the folder of BN6's ROM and the scenario's `NEEDS` together |
| `{wrong}`, `{zip}`, `{damaged}` | files made here: a `.gba` whose header names no game of ours, an archive, a `.cwsave` cut short |
| `{update}` | the address of a made-up newer version's `update.json`, which the runner serves on this machine while the scenarios play (`CYBERWORLD_UPDATE_URL={update}`: [The game's own updates](../engine/updates.md)) |

`ROMS` names the files in the game's ROM folder where they are not BN6's
and its `NEEDS`: `ROMS = ()` none at all, `('wrong',)` the wrong file
alone, `('bn6',)` with `NEEDS = ('bn5',)` BN6 while BN5 is dropped in
later. That folder is then the game's own, as the ROMs screen keeps a
copy there of what it is given.

Three things of the game go with them. `drop PATH` among
[`tools/play.py`](play.md)'s commands drops a file on the game's window
as a player does (an SDL event: a ROM on the ROMs screen, a `.cwsave` on
the title). `--dev settings` has a headless start read the data folder's
`settings.ini`, `keys.ini`, `pad.ini` and `touch.ini` as a player's start
does (it reads none of them otherwise, so that a capture is the same
whatever its folder holds): a restart then finds what Options kept.
`press KEY.M` and `press PAD.x` (with `--pad xbox`, a virtual controller)
are a key and a controller's button through SDL, for the controls
screen. `GAMESCOPE_WAYLAND_DISPLAY=none` first in a `START` is Steam's
big screen, where SAVES has no dialog to wait in and writes and reads
its file in Downloads at once.

Nothing a scenario does leaves the machine: a start that is no player's
sends no statistics, the one that holds an answer across a restart is
given this machine's own closed port (`--dev
statsurl=http://127.0.0.1:9/api/send`, [Statistics](statistics.md)), and
the update box's Yes, which opens a browser, is never pressed.

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
   `leaving`; `cursor new`, `continue`, `options`), which every course that
   crosses the title uses. [The screens' lines](#the-screens-lines) lists
   what the engine's own screens say.
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

## The screens' lines

The engine's own screens say themselves in the state, each from a
function of its own (`title_describe`, `title_setup_describe`,
`options_describe`, `controls_describe`,
`analytics_ask_describe`, `news_box_describe`, `saves_describe`,
`launcher_describe`, and `meta_setup_describe` for a run's setup), called
from `write_state` in `src/dev/capture.c` (the title's there, the others
through `screens_describe` in `src/dev/screen_name.c`); they read and
change nothing.

| Where | Lines |
| --- | --- |
| The title | `title` its step, `cursor` new, continue or options, `saved` the layer of the run CONTINUE takes (0 none), `asks` typing, no or yes while NEW GAME asks, `runs`, `best` and `marks` (hex) of the profile |
| The JACK-IN SETUP | `tab` run, help or extras; `row` net, folder, cross, threat, a helper's or an extra's number, or foot; the choice as `net` short or endless, `startfolder`, `startcross`, `threat`, `helpers` and `extrasoff` (bits); what is open as `endless` open or shut, `folders`, `crosses`, `threatmax`, `helpersshown` |
| A run | the same six lines of the setup it began with (`net` ... `extrasoff`): kept on the setup with `keep`, they are the `same` in the run |
| A run's summary | the battles' scenarios' lines (`title_summary_describe`): `page` result or records, `ended` won or deleted, `cause`, `reached`, `viruses`, `navis`, `learned`, `newbest`, `opened` (what it opened, apart by commas; nothing) |
| Options | `line` the line under the cursor (scale, smooth, colour, blend, lcd3x, depth, music, effects, output, sound, vibration, controls, fast, statistics, update, updates) and `value` as the page shows it; `defaults` no or yes while START asks |
| The controls screen | `device` controller or keyboard, `button` the row (a, b, dpad, ... or setall, defaults, done), `slot` main or also, `bound` what is on it, `asking` none, slot, dpad, all or keep, `said` its passing word |
| The statistics | `statistics` on, off or unasked, where the session may hold an answer; `answer` wait, no or yes while the question is up |
| The update | `update` none, found, getting, failed or ready, where this copy looks; `newversion` |
| SAVES | `savesview` home, compare, result, send or receive; `savespage` cloud, code or file; `savesfocus` the line or button; `here` this device's saves' hash (none), `herelayer`, `hereruns`; `theirs`, `theirlayer`, `theirstatus` ok or refused; `undo` yes or no; `autocopy`; `savesasks`; `savesnote` |
| The ROMs | `roms` those read (`bn6 bn5`, none) on every screen; on the ROMs screen `cartridges`, `romfocus` bn6, bn5, bn4, alt or play, `romnote` and `romnotekind` info, good or bad |

## What they found at once

The first run of `routes` failed three: `layer` and `battle` stopped on
the run's opening gift, which had come to stand before them, and
`pet/options` named a screen since renamed. Every capture made from those
routes had been of the wrong screen, said only by a line of the answer
nobody read. The routes are fixed, and stay so.

Written for the screens outside a run (16 November 2026), they found two
more. Options' LCD 3x was off again at every start: `settings.ini`'s keys
were read by their letters alone, and `lcd3x` has a digit, so its line
was never found again and each turn added one. And a profile's first run,
which jacks in with no setup, never wrote its short net down: won, the
setup it opened started on the endless net.

## Beside it

[An old checkpoint](old-checkpoint.md) is a scenario of its own shape:
the last releases' own builds save a run, this build continues it. It
stays the Release row's, as it builds other commits. The scenarios play
one build.
