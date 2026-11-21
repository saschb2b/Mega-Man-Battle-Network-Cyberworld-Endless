---
type: Tool
title: "tools/play.py: a game kept for a playtest"
description: "`tools/play.py`: one game kept per NAME under `.build/play` for a playtest, replayed from its seed and history, a pinned build, and the dev steps (place, flags, stuck, foes, xrecord, status, battle, sig, second, dump)."
tags: [devtools]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-16T16:30:00Z }
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
map (on BN5 territory one of its records, in BN6's engine), `guardian` takes the run on to its next guardian's layer (the dev menu's Next guardian), `sig` puts MegaMan in the layer's signature (the state
names it and where it lies), `board KIND` stands the Navi in play on a dark layer's board beside the nearest panel of that kind his walk reaches, facing it (`dark`, `item`, `darkhole`, `guardian`, `next` the board's autopilot's choice and more: [Scenarios](scenarios.md), A dark layer's board), `by NAME` beside the layer's first shop, heal, Mystery Data, lock, gate or pad, facing it (`by shop`, `by purple`, `by block`, `by exit`: [Scenarios](scenarios.md), What stands on a layer, lists the names) and `hold AHEAD 40` holds the d-pad the way he faces, `second` saves the second screen's picture beside the
top one's, `drop PATH` drops a file on the game's window as a player does (a ROM on the ROMs screen, a `.cwsave` on the title or SAVES: [Scenarios](scenarios.md) use it), and `dump NAME` the video memory in romlab's format (and the work RAM and the save RAM beside it, `NAMEe.bin`, `NAMEw.bin` and `NAMEs.bin`, for two dumps compared: what a screen of BN6's wrote) for
`tools/romlab/labtrace.py`.

## Sound in a session

A session's game runs headless as fast as it can, its batches apart,
while the sound it opens plays on the audio device's own clock. So the
engine's songs (the Jukebox's, BN6's NaviCust theme under a screen of
ours) trail the frames, and the Jukebox's lights with them: a song a
batch old is still near its start. In session 76 kai6 saw the lights of
BN4's Virus Busting dark at "0:02" and read the song as silent; the
song's first notes come 12 ticks (a fifth of a second) in, and with the
sound in step with the frames the lights are lit 6 frames after A
(replayed 15 November 2026). The log's `music: song` lines name the
engine's songs alone: BN6's own music plays in its core and prints none,
so the room's song come back as the Jukebox closes leaves no line. For
sound in step with the frames, set `CYBERWORLD_AUDIO_DUMP=FILE` and
`CYBERWORLD_AUDIO_OFFLINE=1` (a frame of sound a frame, raw 48 kHz s16
stereo, no device opened): two such dumps of one route, one with a
Jukebox song played and one without, are byte for byte the same from
ten frames after B, in Lan's room and on a layer, the game's own music
playing on. [The PET](../game/pet.md)'s Jukebox says what the player
hears there.
