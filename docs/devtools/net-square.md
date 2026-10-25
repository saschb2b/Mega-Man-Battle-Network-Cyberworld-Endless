---
type: Tool
title: "The Net Square: two games at once"
description: "Testing the Net Square on a local server: `python3 build.py server-run`, a data folder's `network.ini` or `--dev online=URL`, two `tools/play.py` games, dev steps (`emote`, `netpass`, `trade`, `reconnect`) and `tools/link_relay.py` for dropped links."
tags: [devtools]
generated: { by: claude-code/unrecorded, at: 2026-10-24T07:19:34Z }
---

# The Net Square: two games at once

The Square needs a server: `python3 build.py server-run --host
127.0.0.1` runs one on `ws://127.0.0.1:8765/presence`, this computer
alone (seed 120; `--seed`, or another `seed` in a `server.ini` from
`server/server.example.ini`, gives another Square). A game's
data folder points its gate at it with a `network.ini` of its own:

```
identity=0123456789abcdef0123456789abcdef
name=Alpha
named=1
selected=1
consent=2
server=wss://cyberworld.saschb2b.com/presence
server=ws://127.0.0.1:8765/presence
```

(the keeper's first-time questions answered: name chosen, the second
server picked and consented to), or `--dev online=URL,onlinename=NAME`
points the gate at a server and skips his questions for a session. In
the Square the dev step `emote K` sends emote K (0-11, the strip's order)
as the strip does, aimed at the MegaMan faced where it can be (issue #176;
`.build/`'s harness of three to five games on a local server, as
`tools/trailer_online.py` runs them, played every emote, a wave and its
answer, the cap, the burst, the quiet and a group's cheer with it), and
`stap X Y` taps the second screen at its pixel (X, Y), in the picture
`--second-shot` draws (the 3DS's 320 x 240, or `--second-size`'s): the
Square's palette (issue #178; `.build/strip/harness.py` ran a strip,
a palette at an Android display's size and a phone's touch flow in one
Square, `--taps` on SELECT's touch button and the strip's icons). Two
`tools/play.py` sessions, each with its own folder and identity, then play
it from `--scene home --run-depth 7`: `place 16 72 3` puts MegaMan on the
gate, LEFT and A answer Yes; the log names the Square's area and arrival
("online: square area 0, arrival at X,Y") and every visit's end and
trade. A visit plays at the clock's pace even headless, so the two meet.
A trade, a NetBattle, watching one and the week's dive take the
NetPass's full set ([The NetPass](../online/netpass.md)): BN5's ROM beside
BN6's in each game's `--rom-dir`; a game with BN6's alone tries the
Square stamp's side, the referee's and the clerk's word on a missing
stamp among it. Never point a test at the official server.

The NetPass ([The NetPass](../online/netpass.md)) opens over any screen with the
dev step `netpass` (`0:netpass` in a shot's `--input`, `netpass` in
`tools/play.py`), and its stamp moment with `netpass stamp`, which also
writes the data folder's `netpass.ini` as the greeter does. A folder of
BN6's ROM alone (`CYBERWORLD_ROM_DIR` naming one) gives the Square pass:

```bash
python3 build.py shot --scene emu --run-depth 2 --frames 420 --input "300:,0:netpass" --shot "360:/src/.build/np/full.bmp"
python3 build.py shot --scene emu --run-depth 2 --frames 560 --input "300:,0:netpass stamp" --shot-range 300:500:/src/.build/np/stamp
```

Trade Central's two screens ([Trade Central](../online/trade-central.md#the-two-screens), The two screens) open in a
run, out of the Square, with the dev step `trade`: `trade list` lists
the run's spares (the `chip` step gives some, each one written over the
folder's first entry, so the one before stays a spare), `trade list 1`
those that meet a want (online_board.h's numbers, 1 a Fire chip), and
`trade deal 20 5 411 12` shows the chip trade of FireBrn1 F for MrkCan1
M, `trade swap` then swapping its cards as a done trade does. The other
player is "Beta"; nothing is sent, and what is chosen is printed
("trade preview: picked 411 12"):

```bash
CHIPS="250:,0:chip 1 A 1,0:chip 20 F 1,0:chip 411 M 1,0:chip 413 E 1"
python3 build.py shot --scene emu --run-depth 2 --frames 420 --input "$CHIPS,0:trade list,30:,3:START,30:" --shot "300:/src/.build/tc/list.bmp"
python3 build.py shot --scene emu --run-depth 2 --frames 420 --input "$CHIPS,0:trade deal 20 5 411 12,30:,3:RIGHT,20:,3:A,30:,0:trade swap,60:" --shot-range 250:400:/src/.build/tc/deal
```

A dropped link ([A dropped link](../online/square/dropped-link.md)) is tried with
`tools/link_relay.py` between a game and a local server: `--listen 8931
--to 127.0.0.1:8930 --control 8932`, the game's gate at
`ws://127.0.0.1:8931/presence`, and a line on the control port (`echo
"cut 5" | nc 127.0.0.1 8932`): `cut N` holds every byte both ways for N
seconds (a stall: the sockets survive), `close N` closes every link and
refuses new ones for N seconds (a drop: the server sees it at once),
`status` says what is in force. A second game straight to the server, run
with `CYBERWORLD_ONLINE_TRACE=FILE`, writes the peers it sees every sixth
frame, which says whether it ever lost sight of the first. Its wait is
drawn over any screen, for a capture, by the dev step `reconnect N` (as
if the link went N seconds ago: the time left shows past five), `reconnect
back` (the moment it is back) and `reconnect off`; nothing is held or
sent:

```bash
python3 build.py shot --scene emu --run-depth 2 --frames 700 --input "300:,0:reconnect 12,300:" --shot "560:/src/.build/rc/240.bmp"
python3 build.py shot --scene emu --run-depth 2 --frames 700 --touch --size 1080x1920 --dpi 420 --input "300:,0:reconnect 12,300:" --screen-shot "560:/src/.build/rc/phone.bmp"
```

The gate's strip shows while MegaMan asks "To the Net Square,Lan?": a
data folder whose `network.ini` names a server and its consent (above),
the arrival's chats paged, `0:place 16 72 3` and a step off and on.
