---
type: Tool
title: "Frame log: pacing on a player's machine"
description: "`CYBERWORLD_FRAME_LOG=1` (on the 3DS `frame_log = on` in `settings.ini`) prints a line a second of frames shown and played, the gaps between them and each frame's update, drawing and present times, plus the 3DS's heap."
tags: [devtools]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-15T01:30:00Z }
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

Each line names a frame's dearest parts: "the parts a frame:", up to
eight of a tenth of a millisecond or more, dearest first, each a mean
over the frames played (the picture's conversion, the GBA's screen, the 3D's eyes, the
present's, a screen's own drawing), timed where the code marks them
(`frame_part_begin`/`frame_part_end` in `platform.h`, `frame_log_part`
for a time taken elsewhere). On the New 3DS, where the GBA core runs on
a core of its own, "reads of the game waited while drawing" counts the
reads of the game's memory a frame's drawing made, each of which waits
for the core's next frame, and "the last in" names the part that made
the last of them: a draw that reads the game belongs in the update
(the [3D depth](../engine/stereo.md)'s reads were moved to the frame's
end for it). Where depth lighting ran, the line says its time a frame
and its scale. The 3DS's line also says the GPU's time drawing a frame
(citro3d's `C3D_GetDrawingTime`, beside the main core's, not part of it),
and in 3D a frame's bodies and quads, so a frame held back by the GPU
tells itself from one held back by the main core; "show" is the canvas
put into the GPU's texture by SDL, before the present's own parts.

Each line also says how the core's sound fared: "the sound ran dry N
times, M samples of silence" where the device asked the ring
(`src/emu/emu_sound.c`) for more than it held and was given silence for
the rest, each one a crack heard, or "the sound's buffer N samples at
least", the least it held as a read began. The ring is kept near 1536
samples (32 ms) as a frame's sound is written and the 3DS's device reads
1024 at a time, so a frame late enough for two reads to fall before it
leaves the second short. Then "the longest present" of the second, and
what "the line before" took to say.

## The 3DS's lines are kept, not written

On the 3DS a line printed goes to the SD card at once, on the thread the
frames are paced on. Measured on a New 3DS (issue #897, 25 November
2026): 11 to 29 ms a line, 19 at the median. So with `frame_log = on`
every second of play showed 59 frames with one gap of 33 ms, and the sound
ran dry two or three times in it: the log drew a stutter and cracks of its
own making, heard by whoever tested with it and by no player. The title,
which waits for no refresh, showed 60 to 61 with gaps to 22 ms.

So the 3DS keeps its lines in memory (`say`, `src/core/frame_log.c`; 128
KB, two minutes of lines) and writes them where the loop is held anyway:
after a second with a frame of 100 ms or more in it (a layer made, a
start), as the room fills, and as the game ends (`frame_log_flush`, from
`platform_shutdown`). A game that stops without ending loses the lines
kept; the other lines of `log.txt` are written as they are printed, as
before, each one a stall of that size where it falls (a song's start, a
layer made). Elsewhere a frame line is printed as it is said, for the
tools that read the log as it grows.
