---
type: Tool
title: "Agents' tools: build, play, capture and compare in a call"
description: "The `cyberworld` MCP server (`tools/cw_mcp.py`; `tools/cw.py` from a shell), whose `shot`, `play` and `build` capture a screen by route, keep a playtest, compare commits or build in one call, answering in words and a picture."
tags: [devtools]
generated: { by: claude-code/unrecorded, at: 2026-10-23T17:18:21Z }
---

# Agents' tools: build, play, capture and compare in a call

An agent checking a change used to spend several calls on each look: the
build (its every compiler line in the answer), `build.py shot` with input
written as frames, a PIL script to turn the BMP into a PNG, a Read of it;
in a playtest, `play.py do` and a Read for every step, the picture 768x576
whatever was asked. The project's MCP server, `cyberworld` in the
repository's `.mcp.json` (`tools/cw_mcp.py`), does each in one call and
answers short: a line of state, what changed, and the picture inline at
the size asked. Claude Code asks once whether to use the project's server
(or `"enabledMcpjsonServers": ["cyberworld"]` in
`.claude/settings.local.json`); the same tools run from a shell as
`tools/cw.py`, which prints the picture's path instead.

| Tool | Does | Answers |
| --- | --- | --- |
| `shot` | a fresh headless game of one batch: `start`, then `do`; `ref` the same by another commit's build beside this one's | the state in words, one picture (a sheet with `every`); with `ref`, what differs, framed and counted |
| `play` | `tools/play.py`'s game NAME kept between calls: `start` (re)starts it, `do` runs input, `do: "stop"` ends it | the frame and screen, the state's lines that changed, a picture |
| `build` | `build.py host`, `linux`, `test` or `lint` | `host ok in 0.8 s, 1 files compiled`, or the complaints alone |

```bash
python3 tools/cw.py shot options/audio                      # a route: Options' Audio section
python3 tools/cw.py shot "jukebox --seed 4" --do "press A; wait 300" --every 60
python3 tools/cw.py shot "CYBERWORLD_AUTOPILOT=1 layer --seed 3" --until "layer>=2"
python3 tools/cw.py shot jukebox --ref v0.13.0                # this build beside v0.13.0's
python3 tools/cw.py play kai --start "--scene emu --run-depth 3" --do "hold UP+RIGHT 40"
python3 tools/cw.py build test
```

**Start** is a route, the game's options, or both, the options after the
route's: `"jukebox --seed 4"`, `"--scene emu --seed 7 --net-biome 3 --talk
shop:430"`; words of the form `VAR=VALUE` first set the game's environment
(`CYBERWORLD_AUTOPILOT=1`, `CYBERWORLD_STATE_POS=1`). A shot's seed is 1
unless given, so two shots, or the two sides of a comparison, are the same
game. The routes (`tools/cw_routes.py`) reach the screens agents look at
most, each checked against the state's `screen` line by `tests/test_cw.py`'s
route reading and by `tools/cw.py shot ROUTE`, which says so where a route
missed its screen:

| Route | Arrives at |
| --- | --- |
| `title`, `options`, `controls`, `options/display`, `options/audio`, `setup`, `saves`, `launcher` | the title and the engine's own screens over it |
| `room`, `pet`, `pet/folder`, `pet/subchip`, `pet/library`, `pet/megaman`, `pet/mail`, `pet/keyitem`, `pet/options`, `jukebox` | a new run in Lan's room (seed 2), Dad's call paged, and the PET's screens |
| `layer`, `battle` | a run's first layer, its arrival's words paged (`--dev quiet`); its first battle's Custom screen |
| `hp` | Lan's HP after act 2 (seed 3), its words paged |

**Do** is `play.py`'s commands (`press A; wait 60; hold UP+RIGHT 30; mash A
200; shot`, the dev steps among them). **until** runs on, 60 frames a
batch, until the state says so: a state line's first word, an operator
(`=`, `!=`, `>`, `>=`, `<`, `<=`, `~` contains) and a value, several apart
by `|` (`layer>=2`, `screen=battle|doing~battle`), at most `limit` frames
(20000): the autopilot to a layer's exit, a battle to its end.

**The state** names the screen: `screen` is ours over the game first
(`devmenu`, `options`, `options-display`, `options-audio`, `controls`,
`statistics-question`, `jukebox`, `netpass`), then the scene's name off the
run (`title`, `saves`, `launcher`), and in it BN6's (`map`, `town`,
`battle`, `pet`, `folders`, `folder-edit`, `navicust`, `status`, `library`,
`mail`, `keyitem`, `subchip`, `comm`, `save`, `shop`, `trader`, `square`,
`game` for its own cut scenes and fades, `other`), from the second screen's
reading of BN6's modes (`src/dev/screen_name.c`). Ask for no picture
(`picture: false`, `--no-picture`) when the screen and the state's other
lines answer the question: a call of words only costs a tenth of one with
a picture. A `play` call says only what changed (`doing battle (was
walking)`); `state: "full"` says it all.

**Pictures** are the game's 240x160 cut from the canvas, at 2x for one
frame and 1x on a sheet (`scale` 1-4; `full` the whole 256x192 canvas, the
touch controls' place). A picture costs about one token for each 750 of
its pixels: 205 at 2x, 51 at 1x, against 590 for `play.py`'s 768x576.
`second` in the input adds the second screen's.

**Comparing** with `ref` builds that commit once in a worktree of its own
under `.build/cw/ref/` (about 10 seconds; `tools/cw.py prune` removes them),
runs the same game on both builds side by side, and answers with each
frame's pixels that differ and where, old beside new with a red frame round
the difference on the new side.

**Builds** come first by themselves: a `shot` or a `play` start rebuilds
the host binary where a source is newer than it (a stale build checked
twice for a fix that was there), and says so in a line. The game runs from
`build/host/cyberworld` on this machine, not in a container (half a second
saved a run): the build image's SDL2 is the system's here; elsewhere,
`CYBERWORLD_CW_BIN` names another build, the Linux one with its own SDL2.

Nothing in the server keeps state: the games are `play.py`'s folders in
`.build/play/NAME` (its `replay` and the playtest's scripts read them, and a
`play` call's history is `play.py`'s), a shot's game a folder under
`.build/cw/g/` removed after it, and the pictures are kept in
`.build/cw/pictures/` and the games' `shots/`. It reads the ROMs from
`~/.cache/mmbn-ref/roms` (`CYBERWORLD_ROM_DIR`), writes only under `.build/`,
and opens no connection: the server can be restarted, or two run, at any
time.

## What it saves

`tools/cw_bench.py` runs five tasks both ways, each step as an agent's
call, and counts what comes back (2026-10-23; a token a picture's 750
pixels, a text's 4 bytes, a shell's answer cut at 30000 characters as
Claude Code cuts it):

| Task | Before: calls | seconds | text | pictures | tokens | After: calls | seconds | text | pictures | tokens |
| --- | ---: | ---: | ---: | --- | ---: | ---: | ---: | ---: | --- | ---: |
| Options' Audio from the title | 3 | 1.2 | 260 B | 768x576 | 654 | 1 | 0.2 | 114 B | 480x320 | 232 |
| The Jukebox in a new run, a song played | 14 | 5.8 | 4.5 KB | 6 x 768x576 | 4670 | 2 | 1.0 | 239 B | 480x320 | 263 |
| The autopilot walks a layer to its exit | 3 | 3.8 | 5.4 KB | 768x576 | 1941 | 1 | 1.7 | 235 B | 480x320 | 262 |
| A screen before and after (`d89dc612~1`) | 6 | 12.8 | 324 KB | 1032x384 | 10763 | 1 | 9.8 (1.0 built) | 196 B | 964x320 | 460 |
| A one-line change built and seen | 4 | 1.9 | 15 KB | 768x576 | 4437 | 1 | 0.9 | 149 B | 480x320 | 241 |
| **All five** | **30** | **25.5** | **349 KB** | **10** | **22465** | **6** | **13.6 (4.8)** | **0.9 KB** | **5** | **1458** |

Before is the way of the tools before these: `build.py shot` and a PIL
script to read its BMP, `play.py` and a Read of each picture (with the
Linux build it needs already made), `git worktree add` and a build for the
old commit, PIL to put two frames side by side. Both ways are an agent
that knows its way; a live one spends more finding it, more so without the
routes. The autopilot's before guessed its frames (2400), the run's log
saying nothing of the layer's end; its after stops at it (1790). Most of
the text before was builds' command lines: one compiled file and the link
are 15 KB, an old commit's whole build 313 KB. `python3 tools/cw_bench.py
[TASK ...]` runs it again (the ROM, Docker; `CW_BENCH_REF` the commit to
compare with).
