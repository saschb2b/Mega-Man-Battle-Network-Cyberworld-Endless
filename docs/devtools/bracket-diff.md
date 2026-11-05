---
type: Tool
title: "The bracket's differential"
description: "`tools/xbracket.py` runs BN4 Red Sun's tournament bracket in romlab and its move into BN6 in the host build from the same entrants, and compares their frames pixel for pixel on the screen's own clock, with mutations of the comparator and of the move."
tags: [devtools, multirom, bn4, differential]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-05T13:59:00Z }
sources:
  - id: tool
    resource: "tools/xbracket.py, tools/xrecipe_bracket.py, src/emu/xbracket.c"
    title: "The comparison, the recipe's generator and the move's installer"
---

# The bracket's differential

BN4's bracket screen (its main mode 0x24) is moved into BN6 by a recipe
(`tools/xrecipe_bracket.py`, [BN4's tournament screens](../multirom/bn4-tournament.md),
Moved into BN6). `tools/xbracket.py` is its proof: the oracle is BN4's own
screen, the subject BN6's copy, both opened from the same eight entrants and
compared frame by frame. It is the bracket's counterpart of
[the differential](differential.md), which proves a moved battle by its
trace; a screen is proved by its pictures.

```
python3 tools/xbracket.py bn4 [CASE ...]       # BN4's frames in romlab, .build/xbracket/bn4/CASE/
python3 tools/xbracket.py bn6 [CASE ...]       # BN6's in the host build (build.py linux or host first), .build/xbracket/bn6/CASE/
python3 tools/xbracket.py compare [CASE ...]   # PASS, FAIL or INCONCLUSIVE a case
python3 tools/xbracket.py compare --mutation palette|plate|late       # the comparator's own check: must FAIL
python3 tools/xbracket.py compare --move-mutation tournaments          # the move broken: must FAIL where the move passes
python3 tools/bn4_faces.py check [--mutation facepal]                  # the faces' plates the move installs, against tools/bn4_faces.py's byte for byte
```

Local only: both ROMs and the `cyberworld-romlab` image ([romlab](romlab.md)).

## The two sides

- **BN4** (`bn4`): `tools/bn4_lab.py boot`'s state in Lan's room, the
  case's entrants poked at `BN4_TOURNAMENTS` + t x 0x20, the bracket's
  three bytes (`BN4_BRACKET` +5 to +7) and the game state's +0 made
  `BN4_GAMESTATE_TO_BRACKET`, as BN4's scripts open it; then `rec` 240
  frames with the screen's state (`BN4_BRACKET`'s first 16 bytes) watched.
- **BN6** (`bn6`): `build.py shot` on a run's first layer at 240x160, the
  dev step `0:bracket T IIFFSS/...` at frame 600 (its eight entrants, each
  id, flags and seed in hex; `src/emu/xbracket.h`'s entry), the frames
  saved with `--shot-range` and the moved screen's state
  (`XBR_STATE`) with `--watch ADDR:LEN` ([headless shot](headless-shot.md)).

The clock is the screen's own: its first frame in state 4 (shown), then
every frame after it, on both sides.

## Cases

`t0` to `t3` each tournament's first round, MegaMan and seven of BN4's Navis
in, no round won; `settled` The Den Battle Tournament after its first round,
four through (their lines lit) and four out this round.

## Verdicts

PASS where every pixel of every frame matches; FAIL at the first frame
that differs, with how many frames matched and how many rows ever differed;
INCONCLUSIVE where a side never shows the screen. One difference is allowed,
and only where both frames are one colour: the first frame shown, the
backdrop of the map each was opened from (BN4's Lan's room, BN6's net
layer), every layer off by `BN4_MAP_LEAVE`'s DISPCNT 0x40 on BN4's side and
its copy in the entry on BN6's, until the screen's palette reaches PRAM at
the next VBlank (BN4's buffer `0x03002A50`, BN6's `0x03001960`).

## Mutations

The comparator's own (`--mutation`) break BN6's frames as they are read: a
palette colour changed, the first round's plates a pixel lower, the frames
a frame late; each must FAIL every case. The move's (`--move-mutation`) runs
BN6 again with the move broken (`CYBERWORLD_XBRACKET_MUTATE`, read by
`src/emu/xbracket.c`): `tournaments` gives the eToolkit +0x3C shims the next
tournament's record, a RAM word placed wrong; it must FAIL where the move
itself passes, every case but t3, whose plates come from the Free
Tournament's own list, not these records.
