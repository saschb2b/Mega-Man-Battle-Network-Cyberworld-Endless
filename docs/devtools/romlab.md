---
type: Tool
title: "tools/romlab: the plain ROM for research"
description: "`tools/romlab` runs the plain ROM in libmgba for research (scripted input, memory peeks and pokes, states, recordings, probes on its code and watches on its stores), and `labtrace.py` traces captured tiles, palettes and OBJs back to ROM offsets."
tags: [devtools]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-04T01:01:51Z }
---

# tools/romlab: the plain ROM for research

`tools/romlab` runs the plain ROM in libmgba for research: scripted input,
memory peeks and pokes, states and recordings. `labtrace.py` traces captured
tiles, palettes and OBJs back to ROM offsets.

Two commands follow the game's code where a picture cannot (its script's
commands are listed at the top of `romlab.c`), each a line of output per
event:

- `probe ADDR [MEM LEN]` puts a BKPT on a Thumb instruction of the ROM
  copy; each time it runs, `probe ADDR fFRAME r0..r7 sp lr` and the bytes
  asked for, then the instruction as it was (fed to the CPU in the BKPT's
  place). Up to 256 at once: a routine's entry for whether it runs, the
  instruction after a call for what it gave back.
- `writes ADDR LEN` reports each store of the CPU into those bytes, its
  value, size, PC, LR and frame: what wrote a counter the picture shows. A
  store multiple (push, stmia) and DMA are not seen.

Both found how BN4 Red Sun takes a Soul ([BN4 Red Sun's tables](../rom/bn4.md),
Souls in battle): a probe after the pick's gate showed it refusing the
Soul's pseudo-chip, a watch on the emotion window's count led to the turns
in the AI data.
