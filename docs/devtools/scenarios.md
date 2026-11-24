---
type: Tool
title: "Scenarios: the game played through a course, its state checked"
description: "`python3 build.py scenarios` plays each file of `tests/scenarios/` headless (`tools/scenarios.py`, on `tools/cw.py`'s games): a scripted course with the state in words checked on the way, so a bug that only a sequence of play shows stays fixed."
tags: [devtools, tests]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-23T23:30:00Z }
---

# Scenarios: the game played through a course, its state checked

```bash
python3 build.py scenarios                 # every scenario, four games at a time
python3 build.py scenarios liberation      # those a name starts
python3 build.py scenarios routes --jobs 8 # every route of tools/cw_routes.py
python3 tools/scenarios.py --list          # each with what it guards
python3 tools/scenarios.py --check         # all read without the game (build.py test does)
python3 tools/scenarios.py --stop          # the games a run that was cut off left playing, stopped
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

A run stops every game and server it started as it ends, and at the
signal that ends it early (a terminal closed, a tool's time up): the
first day's sessions left a hundred games playing on for good, each a
run cut off before its own stop. `tools/play.py`'s `stop` asks a game to
quit and kills it where it has not within a second, since a quit
signalled between two batches is not heard (A quit at any frame, below).
`--stop` ends the games an earlier run left under `.build/scenarios`.

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
| `('moved', WORD, BY)` | ... or the kept line WORD's number moved by BY since: a number (`1`, `-1`), or a kept word with a sign (`'-price'`: down by the price kept), so a purchase is checked against the price the shop showed and not against today's balance |
| `('restart', START)` | the game quit as a player's system quits it (SIGTERM, the port's close: it saves and leaves), then started again on the same saves |
| `('server', 'stop')`, `('server', 'start')` | the scenario's server gone at once, and back on its port with its state ([A server and other games](#a-server-and-other-games)) |
| `('sleep', SECONDS)` | the clock's seconds: what the server holds for a time (a crew formed after a second near, issue #714) sees it pass, where a dive's layer runs its frames faster than the clock; with a server, no game held meanwhile, each online one running on at the clock's pace and answering the others (a busy crewmate's answer, issue #715) |
| `('@beta', 'do', COMMANDS)` | any step, in the game `beta` of the scenario's `GAMES` |

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

## What stands on a layer

The layer's scenarios (`layer_*.py`: its keepers, Mystery Data, locks and
set pieces, gates, the way on, the buttons that must not fall through)
walk up to a thing and press A, as a player does, and read what MegaMan is
left with. Two dev steps and the state's lines below make that the same
on every seed ([tools/play.py](play.md) has the steps):

- `by NAME` puts MegaMan beside the layer's first NAME, facing it, on
  floor he can walk to: a `--talk` kind (`shop`, `heal`, `programs`,
  `npc`, `vault`, `navigate`, `official`, `challenge`, `duel`, `trader`,
  `bugtrader`, `undernet`, `gate`, `gift`), a Mystery Data by its colour
  (`green`, `blue`, `purple`) or by the key item in it (`scrtdata`,
  `regup`), a lock (`block`), the Navi who knows its P-Code (`teller`) or
  hints at an invisible path (`hinter`); a number after a kind or a colour
  is a later one (`npc2`). A pad is stepped on, so `exit`, `return`,
  `teleport`, `gap`, `lane` and `path` put him on it (`exitside`: two
  panels off it), and `arena` on the guardian's way in, two panels short
  of his arena, facing it. `--talk` runs a chat's script with no one walked up to;
  `by` and `press A` go through BN6's own check and the director's keys.
- `hold AHEAD 40` holds the d-pad the way MegaMan faces: over Rush's
  back, onto an arrow lane, along an invisible path, wherever the seed
  laid it.

| The state says | Where | Written by |
| --- | --- | --- |
| `savemasks N` (the masks BN6's own save has made since the start: one a save, where nothing wrote into the RAM while it was masked; a write fails the save's sum, and BN6 masks and saves it all again), `savemasked yes` or `no` (the RAM masked now) | a run, at home too | `save_mask_describe`, `src/emu/save_mask.c` |
| `chips N` (the chips owned, the Folder's and the Pack's: BN6's pack counts both), `programs N`, `unlockers N`, `rushfood N`, `regmemory N` | a run, at home too | `holds_describe`, `director_folder.c` |
| `map open`, `hidden` (opened by a press, under a chat for now) or `closed`: the layer's map over the picture (issue #840) | a layer | `director_describe`, `director_describe.c` |
| `choice K/N` (the cursor on option K of the N a chat box asks) | while BN6's box shows a question | `talk_describe`, `talk.c` |
| `pickat K/N` the cursor's row as a player counts it, `pickof hp`, `chip`, `program` or `skip` what that row is (with the pick's other lines, `pick`, `offers`, `pickrows`, `pickrow`, `pickready`, below) | a reward pick's screen | `director_pick_describe`, `reward_pick.c` |
| `shoprows N`, and of the row under the cursor `price N` (as shown, zenny or BugFrags) and `stock N` | BN6's shop screen | `shop_describe`, `shop.c` |
| `exitpad shut` or `open`, `heard yes` once a Navi has named the act's guardian, `greentaken N`, `bluetaken N`, `purpletaken N`, `locks N` (still standing), `pcode told`, `gate HeatMan sealed` or `earned`, `on floor`, `teleport1`, `teleport2`, `gap`, `lane` or `path` | a layer | `layer_describe`, `layer_describe.c` |

With `CYBERWORLD_STATE_POS=1` first in its START a scenario also reads
`held`: `held=none` is MegaMan free, so
`('repeat', 'press A; wait 30', 'held=none')` pages a chat to its end
whatever its boxes. A question is waited for with
`('repeat', 'press A; wait 30', 'choice=2/2')` and its cursor moved with
`('repeat', 'press LEFT; wait 10', 'choice=1/2')`: a d-pad press as the
options appear is lost. On BN6's shop screen `zenny` is the game's
working copy and moves once the screen is left; on its trader's screen
the pack's counts are the spare chips alone.

## A server and other games

Online play is played against a Net Square server of this machine, and
mostly by two games ([The Net Square: two games at once](net-square.md)).
A scenario asks for them (`tools/scenarios_online.py`, the runner's cast):

```python
import scenarios_online as online
from scenarios_online import soon

NEEDS = ('bn5', 'bn4')                 # a trade, a NetBattle and the dive take the NetPass's full set
START = online.HOME                    # Lan's HP after act 2, the gate at {server} as {name}
SERVER = True                          # or dict(seed=120, weeks=[(name, depth, won, minutes[, pulled])], settings=dict(capacity=2))
GAMES = {'beta': online.HOME}          # other games, started beside the first
FILES = {'netpass.ini': online.SEEN_FULL}   # into each game's data folder before it starts
STEPS = online.course(
    online.into_square(), online.into_square('beta'),
    soon('peers~Beta'),                # the first game's state: it sees him
    ('@beta', 'do', 'place 116 -52 5; wait 10'),
    soon('peer~Beta at 116 -52 in plaza'),
    ('server', 'stop'),
    soon('visit=waiting', '', 600),
)
```

- **The server** is the checkout's (`server/main.py`), on a free port of
  127.0.0.1 with a state folder of its own, started before the games and
  killed after them; `('server', 'stop')` kills it mid-course, as a
  server that fell over, and `('server', 'start')` brings it back on the
  same port with the same state. Two scenarios never share one, so they
  play side by side.
- **Places in braces** in a `START`, a game's or a file's text name what
  only the run knows: `{server}` its server's address, `{nowhere}` an
  address of this machine nothing answers on (a port bound and never
  listened on: a call there is refused at once), `{name}` the game's name
  (`Alpha` the first, then its key in `GAMES`), `{identity}` a private
  identity made of it, for a `network.ini` (the gate as a player's, which
  asks "To the Net Square,Lan?"; `--dev online={server}` goes at once).
- **No scenario reaches a host but this machine.** A scenario that names
  an address of its own (`ws://`, `wss://`, `--dev online=` anything but
  `{server}` or `{nowhere}`) does not read (`build.py test`), and every
  game the runner starts, whatever its scenario, runs with
  `CYBERWORLD_ONLINE_LOCAL`, under which the game's socket refuses every
  host but 127.0.0.1 (`online_socket.c`): a course that pages the gate's
  keeper to his official Square ends at the gate with "no answer".
- **A game not being stepped keeps running while it is online** (its
  state's `visit` not `home`, or `divelink on`), a twelfth of a second a
  batch: a visit plays at the clock's pace, and a game that stood still
  would drop off its server. A step in it waits for its batch; a check
  reads its state as that batch left it. Each game keeps what it kept
  (`keep`, `same`) for itself.
- **A visit costs its seconds.** Sixty frames are a second there, so an
  `until`, which looks once in sixty frames, costs a second each:
  `soon(COND)` is `('repeat', 'wait 10', COND, 30)`, and the common steps
  (`into_square`, `home_again`, `yes`, `pick`, `paged`, `at`, `course`)
  are in the cast's file. A chat's question is answered once its cursor
  is set (`choice=2/2`: BN6 shows the options a few frames before), an
  option taken with `pick(EFFECT)`, A until its effect shows: a box
  counts its options as it types them, a moment before it takes a key.
  A NetBattle from the Square is BN6's own link battle on two cores a
  game and takes half a minute whatever is done; the others stay under
  thirty seconds.
- **What stays apart from the layout.** The Square's places are its
  seed's (120's: `tests/scenarios/online_square.py` lists them); the
  week's net is the server's own (its key and the calendar week), so a
  dive's layer is another every run, and its courses read nothing of it.
  A NetBattle between equals is decided by the link's timing (9 to 0 one
  run, 0 to 4 the next): the one whose result is read gives its MegaMen
  unequal HP (`arenahp`).

The state says online play in lines of its own (`online_describe`,
`src/online/online_describe.c`; a feature's own function each): `visit
home|calling|square|battle|watching|waiting|pet`, `visits N`, `visitended
left|unreachable|lost|maintenance|full|name|dive|...`, `netpass
square|trade|full`, `stamps N`, `spares N: 1Ax2 20F` (the chips a trade
could give, id and code), `pending 20F for 411M` (a swap kept for home),
`dive on|none`, `divelink on|off`; in the Square `chat`, `held`, `choice
K/N`, `squarex`, `squarey`, `facing`, `part plaza|trade|arena|stage`,
`channel K/N`, `peers N: NAMES`, `peer NAME at X Y in PART free|battle|
watch|trade`, `sentlast emote|phrase K (N)`, `heardlast emote|phrase K from NAME
(N)`, `link up|quiet|reconnecting`, `resumes N`, `notice CODE`, `trade
none|offered|waiting|answered|accepted|done|cancelled` with `tradewith`,
`gives`, `gets`, `tradescreen none|list|deal` with `listrow K/N`,
`listchip`, `deal ask|wait|swap|swapped`, `dealcursor`
(`trade_screen_describe`), `tradeboard N` with `tradeoffer NAME has 20F wants W` and
`myoffer`, `challenge none|to NAME|from NAME`, `arena idle|sent|...|
battle|over` with `arenacard won|lost|stopped`, `arenahp MINE HIS`, `rematch`,
`arenalooks on|none` (the bout's looks, [Looks in a NetBattle](../online/arena/looks.md))
and `arenastop apart|left|...` (why the last match stopped) (`square_arena_describe`), `bouts N`, `bout won against NAME won 1 lost 0`,
`weekoffer none|new|again|on|trade|stamp`, `week N`, `weekboard WORDS`; on
a dive with its link `divers N: NAMES`, `diver NAME at X Y`, `tags N:
NAMES` (the names shown over the divers' heads, `NAME+K` where K more
stand under one, `NAME*` a crewmate's; `dive_people_describe`), `rings
N: NAMES` (the crew's rings under the divers shown, `NAME(forming)` a
dotted one, about to be), `door none|alone|queued: NAMES|full|started` (a
guardian's door, its party leader first; `dive_party_describe`), `doorwait S for N`
(the leader's: the seconds the door still waits for N crewmates), `crew N: NAMES`
(`dive_crew_describe`), `join none|calling|joined|fighting` (a crew's
battle: MegaMan's own held and offered, a crewmate's joined, the one
field fought; `dive_join_describe`); on that field `partycustom
chips|NN` (the Custom screen this game's pad drives, the field's for
the lead and his own for a partner: `chips` where it takes keys, else
BN6's mode in hex; `party_play_describe`), on which a course presses
rather than on a count of frames, the field running at the clock's
pace ([The shared dive](../online/shared-dive.md), The screen that
takes his keys).

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
  goes back to the last checkpoint, the battle and its reward lost. It
  stood in the fuzz's `KNOWN`, below, until the board saved the run as
  its Navi begins to speak (`liberation_save.c`, with the board's own
  scenarios): `--fuzz 3/12` passes, and the entry is gone.
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
| `battle random`, `guardian`, `duel`, `gate`, `copy`, `server` or `floor`; `phase custom`, `fight`, `results` or `other`; `form none`, `HeatCross` ... `BeastOut`; `busterattack N` (the battle's Attack level, a buster shot's damage before a form's bonus: BusterUp's +1); `foestatus none`, `paralysed`, `dragged`, `frozen` or `bubbled`; `megamanfaces right` or `left` (turned round on a Surrounded field's L or R) and `foeshurt none` or the columns of the foes that have lost HP, left to right (`foeshurt 2`: where an attack struck, issue #838); in a swap's battle (the copy's, the `swap` step's) `swapform N` (the form fighting, 1 first) and `leftovers N` (the objects bound to the last form taken off still at work, 0 where they ended with it: issue #733); at its results `result won`, `lost`, `ran` or `draw` and `reward zenny N`, `chip ID CODE`, `hp N`, `bugfrags N`; on the map after one `lastbattle won` ... | `battle_state_describe` (`battle_state.c`) | a battle: what it is, where it stands, how it ended and what its RESULT paid, MegaMan's form and buster, the statuses its enemies are held in, the way he faces and where an attack struck, a swap's form and what the last left behind |
| `version N`, `known yes` or `no`, `data hidden`, `fresh`, `waiting` or `taken` (the exit pad's line is the layer's, above); in his fight `guardianhp N` | `boss_describe` (`boss.c`), beside `guardian NAME waiting` ... | the guardian's course: his version, whether MegaMan has battle data on him, his Guardian Data (`fresh` its first second, when no A takes it) and the exit pad |
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

## A dark layer's board

A liberation's board says itself in the state
(`lib_describe`, `src/director/liberation_describe.c`, whose head lists
every line): its step (`board opening`, `move`, `menu`, `battle`,
`darkloids`, `done`; `board none` on a layer with no board), `boardphase`
and `par`, `orderpoints`, the panels dark and `freed`, the DarkHoles and
guardians standing, the Darkloid and his `darkloidhp`, each Navi of the
team (`mate0 MegaMan 100/100 inplay`), what the Navi in play `faces`, the
window's `command`, how the last battle ended (`lastfight won`, `oneturn`,
`timeup`, `lost`) and the frames its fight ran on under its closing banners
(`foughton 0`: paused under TIME UP! and LIBERATE FAILED!, issue #794),
the `prize`'s tier and `boardexit`. A battle of its own
adds `foehp`, `soul` and `cross`.

A board is its seed's, so a scenario does not name its panels. The dev
step `board KIND` ([tools/play.py](play.md)) stands the Navi in play
beside the nearest panel of a kind his walk reaches, facing it, where a
player would have walked him: `dark`, `pair` (LongSwrd's two), `item`
(or what it holds: `orderpoints`, `heart`, `key`, `route`), `trap`,
`bonus`, `darkhole`, `guardian`, `sneak` (a guardian in SneakAttack's
reach), `ring`, `darkloid`, `mate`, `over` (BombLiberation's flier over a
dark panel), and `next`, the panel the board's autopilot would take
(`liberation_auto.c`). The command is then taken by the pad. Its battle is
ended from the dev menu (hold SELECT, press R; "Win this battle", its
fifth row, deletes the enemies and BN6's own loop ends the battle), in
the turn the scenario waits for: the first for a one-turn win, the second
for a plain one, none for TIME UP!. `tests/scenarios/liberation_board.py`'s
`Course` builds those steps, and `liberation_team.py` takes it for BN5's
team.

They found two bugs as they were written. With BN6's ROM alone a dark
layer's panels were drawn 256 pixels off or not at all on eight layers of
nine: the recolouring read BN6's scroll as wrapping at 512, where BN6
scrolls a screen map of 256 (`liberation-board/shown` reads the picture's
own count, `darkshown` and `facedshown`). And START on the team's pick,
the board's own team taken, went on to BN6 and opened the PET over the
briefing (`liberation-team/pick/kept`). A third the fuzz of quits found
on the board, and its fixed course is here: MegaMan's words after a won
battle began in the fade back from it, BN6 still holding him, so no save
was held back before them and a quit under them lost the battle
(`liberation-board/continue/under-the-words`; the words now wait for
`talk_due_now()`, as the director's other talks do).

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

## The older nets in the state

The `xrom_*.py` scenarios play with BN5's and BN4's ROMs beside BN6's
(`NEEDS`) and read these, from `older_describe`
(`src/director/older_describe.c`, one call in `director_describe`) but
the title's:

| Lines | Say |
| --- | --- |
| `acts bn6 bn5 bn4 ...`, `olderacts N`, `olderguards N`, `soulsheld XX` | the run, at home too: each act's area by its game in the run's order (three in the short net), how many are an older net's, how many end at one of its Navis, BN5's Souls held (hex, a bit each from Colonel's) |
| `areafrom bn6`, `bn5` or `bn4`; `guardianfrom` the same | a layer: whose area it is drawn as, whose Navi guards it |
| `guardianstands ANIM shown` or `unseen`; one of BN4's `front-left`, `front-right`, `back-left` or `back-right` | his NPC's animation on the map and whether his sprite draws it; a BN4 Navi's the diagonal it shows him in (BN4 draws four, by its layout of pairs) |
| `copyform NAVI`, `copystands ANIM` | a Glitch layer's copy: its first body, its NPC's animation |
| `enemyhp N`, `enemynames CanGard+Mettaur`, `olderfoes N`, `olderrecord 811DE10` or `none` | a battle: its enemies' HP together, their names as the battle shows them (`KnightMn`, `GutsM`), how many are moved in from BN5 or BN4, and BN5's own record it was rolled from (how many stand is the tally's `foes` and `down`) |
| `drawn 204L 204L ...`, `handpicks N`; then `hand N`, `nextchip ID`, `nextchipfrom bn6`, `bn5` or `bn4`, `nextpower N` | the Custom screen's hand (id and code) and the chips picked; after OK the chips left to use, the next one, its game and its power |
| `unite none`, `grey`, `lit` or `chosen`; `unitesoul N`; `united N`, `unitedturns N`, `soulsused X` | where BN5's Souls stand in the engine: UNITE under OK and the Soul the last chip picked lights; the Soul MegaMan is united with (BN5's number, 0 none), its turns left, the Souls used this battle |
| `found BN5 + BN4 found: both join ours` or `nothing`; `foundnote due`, `up`, `gone` or `none` | the title (`title_found_describe`, `scene_title.c`): its note of the other games' ROMs, in its own words, and whether it shows |

A battle won by `--dev onehit` is over and back on the map inside one
look of an `until` (60 frames): its end is read from `lastbattle=won`,
which stays. `screen=battle` comes before the foes stand: a course waits
for `foes>=1` before it reads a battle's lines. An A pressed while BATTLE
START! shows is lost, so a chip is used once `custom>=20` (the gauge a
fifth full), with `('repeat', 'press A; wait 30', 'hand=0')`, which stops
before a buster shot; L on an open Custom screen is a try to run, pressed
once. The `chip` step wants a code the chip's record has (`chip 204 L`):
with another the hand holds chip 389, power 0. `--dev god` does not save
MegaMan from one blow of more than his HP: `hp=2000` beside it.

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

The battles' scenarios found one in the game: MegaMan deleted in a
battle, the next run begun without the game closed came onto its first
layer as if back from that battle, and its tally began with the viruses
deleted in it (`battle_deleted.py`; the battle now ends with the run).

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
