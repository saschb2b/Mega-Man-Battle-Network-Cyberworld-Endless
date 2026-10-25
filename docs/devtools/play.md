---
type: Tool
title: "tools/play.py: a game kept for a playtest"
description: "`tools/play.py`: one game kept per NAME under `.build/play` for a playtest, replayed from its seed and history, a pinned build, and the dev steps (place, flags, stuck, foes, xrecord, status, battle, sig, second, dump)."
tags: [devtools]
generated: { by: claude-code/unrecorded, at: 2026-10-12T21:51:29Z }
---

# tools/play.py: a game kept for a playtest

`tools/play.py` keeps one game running per NAME (`.build/play/NAME`: its
saves in `data/`, every command in `history.txt`, the seed in its first
line), so a playtester, person or agent, plays it like the handheld,
reading each picture; a session replays from the seed, `data0/` and the
history. Each start copies the current build into the session's `bin/`;
with `NAME/bin.pin` present its restarts keep that copy, so a rebuild
during a playtest leaves its CONTINUE alone. Dev steps beside the
player's: `place X Y FACING` puts MegaMan somewhere, `flags FROM TO 1`
sets a block of event flags and `flags FROM TO 0` puts them back,
`stuck NAME` saves the run with BN6 holding MegaMan so (a CONTINUE's
recovery tested, issue #23), `foes 200@52 01@43` a battle of the foes named, `xrecord 811DE10` one of BN5's records in BN6's engine and `status freeze 90` a status on its enemies ([Dev tools](overview.md)), `battle` starts the layer's next random battle once he is free on its
map (on BN5 territory one of its records, in BN6's engine), `sig` puts MegaMan in the layer's signature (the state
names it and where it lies), `second` saves the second screen's picture beside the
top one's, and `dump NAME` the video memory in romlab's format for
`tools/romlab/labtrace.py`.
