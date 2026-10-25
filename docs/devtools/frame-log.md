---
type: Tool
title: "Frame log: pacing on a player's machine"
description: "`CYBERWORLD_FRAME_LOG=1` (on the 3DS `frame_log = on` in `settings.ini`) prints a line a second of frames shown and played, the gaps between them and each frame's update, drawing and present times, plus the 3DS's heap."
tags: [devtools]
generated: { by: claude-code/unrecorded, at: 2026-10-13T00:52:19Z }
---

# Frame log: pacing on a player's machine

`CYBERWORLD_FRAME_LOG=1` prints a line a second of how frames reached the
display: how many were shown, the game frames played so far, and the gaps
between shown frames (shortest, longest, and how many fell under 12.5, 20
and 30 ms and over), then a frame's update, drawing and present, and the
longest update and drawing of the second (a hitch the means hide: BN5's
boot, [Other games' ROMs](../multirom/overview.md), ran beside BN6's frames without one). 60 shown
with gaps near 16.7 ms is the GBA's pace;
fewer shown with more played is the loop catching up (a slow machine, a
missed refresh). On the Steam Deck OLED in Desktop Mode it showed 60 shown
and played a second, gaps 15-18 ms, after the first layer's first seconds
(27 shown, 52 played, while the layer is made). On the 3DS (`frame_log =
on` in `settings.ini`) each line ends with the heap: its use and its
high-water mark against its size ([The 3DS's memory](../engine/emulation/3ds-memory.md)).
