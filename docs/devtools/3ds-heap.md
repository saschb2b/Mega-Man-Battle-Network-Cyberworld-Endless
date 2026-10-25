---
type: Tool
title: "The 3DS's heap on a computer"
description: "`tools/heap3ds.py` plays long headless runs of the host build with every allocation counted as the New 3DS's 96 MB heap holds it, reporting the peak, refused allocations and run depth, BN6 alone or with BN5."
tags: [devtools]
generated: { by: claude-code/unrecorded, at: 2026-10-13T00:52:19Z }
---

# The 3DS's heap on a computer

```bash
python3 tools/heap3ds.py                     # seeds 1-12, the three cases below
python3 tools/heap3ds.py --seeds 1-24 --cases bn5 --no-build
python3 tools/heap3ds.py --cases bn5 --cap 90   # a heap 6 MB short: what a refusal does where fragmentation took that much
```

`tools/heap3ds.py` plays long runs of the host build headless (the test
autopilot with `--dev god,onehit`, 120000 frames: act after act, area after
area) with `tools/heap3ds.c` preloaded, which counts every allocation as
the New 3DS's heap holds it: 96 MB, the core's 16 MB ROM copy kept (a
computer's core copies it and frees it), mGBA's 16 MB ROM buffer held from
the start where the case says so, and an allocation past the heap refused
as the 3DS's malloc refuses it, its place named; a crash is named too. The
cases: `today`, the 3DS before issue #82's 3DS round (BN6's ROM alone,
the buffer held, every area's learned maps kept: the host build as it is);
`bn6` and `bn5`, the 3DS now (the buffer given back, the layer's own
area's learned maps kept alone: the host built with `CW_HEAP_3DS` into
`build/heap3ds`), BN6's ROM alone and with BN5's read. `--as-3ds PATH`
runs those two on another build of the host (`make TARGET=host
OUT=build/k2 "EXTRA_CFLAGS=-DCW_HEAP_3DS -DAREAS_KEPT=2"`: two areas
kept). Per run: the peak, the allocations refused,
how deep the run went, how often an area was learned again. It models the
bytes, not the 3DS's fragmentation: the device's own numbers are
`log.txt`'s heap lines.
