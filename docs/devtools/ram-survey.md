---
type: Tool
title: "The RAM survey: what BN6 leaves alone"
description: "`tools/ram_survey.py` runs a battery of headless autopilot games that mark every byte of the GBA's work RAM a frame ended with not zero (`CYBERWORLD_RAM_SURVEY`), prints the stretches that stayed zero, and with `--canary` fills a stretch with a pattern and tells every byte a frame changed (`CYBERWORLD_RAM_CANARY`): the measurement before the engine claims RAM."
tags: [devtools]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-09T21:07:22Z }
---

# The RAM survey: what BN6 leaves alone

```bash
python3 tools/ram_survey.py                               # 54 runs: the stretches of EWRAM and IWRAM that stayed zero
python3 tools/ram_survey.py --canary 0x02010CE0:0x444     # the same runs with a pattern over a stretch: who writes it
```

The engine keeps its own state in RAM BN6 leaves alone ([Memory the
engine writes](../engine/emulation/memory.md), the RAM's regions in
`src/emu/free_space.plan`). Before a new stretch is claimed it is
measured here, and read in the code.

**The survey.** With `CYBERWORLD_RAM_SURVEY=FILE` the core's frame step
(`src/emu/ram_survey.c`, called from `emu.c`) marks each byte of EWRAM
(`0x02000000`, 256 KB) and IWRAM (`0x03000000`, 32 KB) that holds
anything but zero as a frame ends, and writes the map as the game exits
(a map already there is read first, so runs add up). The tool runs the
battery side by side on every core (the autopilot's seeds 1-16, each of
BN6's and BN5's areas, deep runs, the town, home, BN5's guardians, the
dark layer, the tournament, the Glitch layer, visitors and a route),
merges the maps and prints the stretches no run touched, longest first.
Where a checkout of bn6f is at hand each stretch is crossed with its
`ewram.s`: a stretch with a symbol inside is a buffer the battery did
not use (the Request BBS's and the mail's, the decompression buffers',
the link library's globals), not free RAM.

**The canary.** A byte that stays zero may still be written with zeros.
`CYBERWORLD_RAM_CANARY=ADDR:LEN[,ADDR:LEN...]` fills each stretch with a
pattern once it is found all zero and tells on stderr every byte of it a
frame changed (the first few a stretch, then the count), filling it
again. The power-on clear shows at frames 14 and 15 of the boot, a clear
that runs across two frames; anything else is a writer. A reader does not
show: the runs' logs are compared with a run without the canary
(`runlog.txt` of each data folder), and the code that could reach the
stretch is read.

**What it does not reach.** No NetBattle (the pair's cores are not the
game's), no shop bought from, no PET screen, no folder editor: a stretch
it finds is tried there by hand before it is claimed.

**What it found** (9 November 2026, for chip ids past 511: [The chip
registry and BN5's chips' code](../rom/chips-code.md), The pack past
511): no stretch of 1 KB or more that stayed zero and holds no symbol
but the persistent zone the engine already owns
(`0x02010CDC`-`0x020117FF`); in it, the pack's extension's entries of
ids 320-410 stayed zero, and their canary met the power-on clear alone.
