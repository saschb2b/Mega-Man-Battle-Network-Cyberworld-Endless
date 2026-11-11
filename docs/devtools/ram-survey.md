---
type: Tool
title: "The RAM survey: what BN6 leaves alone"
description: "`tools/ram_survey.py` runs a battery of headless autopilot games that mark every byte of the GBA's work RAM a frame ended with not zero (`CYBERWORLD_RAM_SURVEY`), prints the stretches that stayed zero, with `--canary` fills a stretch with a pattern and tells every byte a frame changed (`CYBERWORLD_RAM_CANARY`), and with `--watch` tells every read and write the game's code makes of a stretch, the save RAM's too, changing nothing (`CYBERWORLD_RAM_WATCH`): the measurement before the engine claims RAM."
tags: [devtools]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-10T21:45:00Z }
---

# The RAM survey: what BN6 leaves alone

```bash
python3 tools/ram_survey.py                               # 54 runs: the stretches of EWRAM and IWRAM that stayed zero
python3 tools/ram_survey.py --canary 0x02010CE0:0x444     # the same runs with a pattern over a stretch: who writes it
python3 tools/ram_survey.py --watch 0x0E000000:0x8000,0x0E006810:0x15F0   # ... with every read and write of a stretch told: who reaches it at all
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

**The watch.** A canary changes the run it watches (the game reads the
pattern where the bytes are live) and sees writes alone.
`CYBERWORLD_RAM_WATCH=ADDR:LEN[,ADDR:LEN...]` changes nothing: it puts
the engine's own calls under the eight the core's CPU reaches memory by
(a load and a store of each size, a load and a store multiple; DMA and
the BIOS's copies go through the same eight in mGBA), and each access
that touches a stretch is told on stderr once a place: `ram watch:
0e006810+15f0 written 0e006d1c, 1 byte, at 08021b6e lr 08021b1d frame
473` (the stretch, read or written, the first byte touched, the access's
size, the instruction and its link register), then each stretch's sums as
the game exits (reads, writes, the lowest and highest byte of each, how
many wider than a byte). A stretch is any of the bus: the work RAM, or
the cart's save RAM from `0x0E000000`; a mirror's address counts as its
first image's. Not seen: an instruction fetched (code run from the
stretch), and the engine's own reads and writes, which do not go through
the CPU. A NetBattle's pair cores are watched too (`arena_core_new`), and
a play session takes the variable as any (`tools/cw.py play NAME --start
"CYBERWORLD_RAM_WATCH=... layer"`, the lines in
`.build/play/NAME/log.txt`). `--watch` runs the battery so and sums the
runs: each place with the runs that met it.

**What it does not reach.** No NetBattle (the pair's cores are not the
game's), no shop bought from, no PET screen, no folder editor: a stretch
it finds is tried there by hand before it is claimed.

**What it found** (9 November 2026, for chip ids past 511: [The chip
registry and BN5's chips' code](../rom/chips-code.md), The pack past
511): no stretch of 1 KB or more that stayed zero and holds no symbol
but the persistent zone the engine already owns
(`0x02010CDC`-`0x020117FF`); in it, the pack's extension's entries of
ids 320-410 stayed zero, and their canary met the power-on clear alone.
For the ids from 635 (10 November 2026; the same page, The pack in the
save RAM) the watch was built and pointed at the cart's save RAM: over
the battery BN6 reads its own save's blocks as it boots and nothing
else, and the 5,616 bytes between them are reached by the pack's
routines alone; a Chip Trader's trade, the one place a run still runs
BN6's own save, writes its blocks and leaves the span alone.
