---
type: Tool
title: "A controller in a test"
description: "`--pad KIND` (xbox, playstation, nintendo, generic) attaches an SDL virtual controller to a headless shot or `tools/play.py` game, and `pad.NAME` and `key.NAME` steps press its buttons and keyboard keys as a player's."
tags: [devtools]
generated: { by: claude-code/unrecorded, at: 2026-10-24T23:07:49Z }
---

# A controller in a test

```bash
python3 build.py shot --scene title --pad xbox --input "60:,4:START,60:,4:pad.back,20:,4:pad.dpdown" --shot 200:/src/.build/a.bmp
python3 build.py shot --scene emu --run-depth 1 --pad nintendo --input "420:,6:pad.a" --shot 460:/src/.build/a.bmp
python3 tools/play.py start NAME -- --pad playstation
python3 tools/play.py do NAME "press pad.back; press key.J"
```

`--pad KIND` (xbox, playstation, nintendo or generic) attaches one of
SDL's virtual controllers, named and numbered as that kind is (an Xbox 360
pad, a PS4 pad, a Switch Pro Controller), so the game opens and reads it
as a player's (`pads.c`): its family and labels, the controls screen,
`pad.ini`, which the data folder then holds headless too (written with
the defaults where missing, and when the controls screen keeps a map).
Steps of `--input` and play.py's commands take `pad.NAME`, that pad's input
by SDL's name held for the step (`pad.a`, `pad.dpup`, `pad.lefttrigger`,
`pad.-lefty`), and `key.NAME`, a key (`key.J`, `key.Return`), both through
SDL's events as a player's, past any map; the GBA's own names (`A`,
`UP+RIGHT`) still inject the buttons themselves, and `FAST` fast forward's
(`30:,4:FAST` turns it on for the rest of the run, at 3x headless, which
reads no `settings.ini`; [The player's guide](../GUIDE.md#fast-forward), Fast forward). It needs SDL 2.24 (every
build but the handhelds' PortMaster binary, linked against 2.0.14, where
`--pad` says it has none).
