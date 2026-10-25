---
type: Engine Mechanism
title: "The core's sound"
description: "How the core's sound reaches the device: `audio_pace.c` nudges mGBA's resampling by the ring's averaged fill, settings.ini's `sound` filters it, and a hook on TrkVolPitSet (`sound_quiet.c`) silences music or effects."
tags: [engine, emulation]
generated: { by: claude-code/unrecorded, at: 2026-10-23T20:47:59Z }
---

# The core's sound

mGBA's blip buffers resample the GBA's sound to the device's 48 kHz, a
frame at a time, into `emu.c`'s ring, which the audio callback reads. The
GBA runs 59.7275 frames a second and the loop 60, so the core makes 0.45%
more sound than the device plays: the rate the blip buffers resample to is
nudged by how full the ring is (`audio_pace.c`), and the game's sound
plays 0.45% high and fast, as every 60 Hz emulator plays it. The nudge
reads the ring's fill averaged over about two thirds of a second. It read
the fill itself before: the device takes the ring in blocks (512 samples
on a desktop, 1024 on a handheld, more on some sound servers), so the
fill after each frame swung by a block, and the pitch with it, 10 to 18
cents from one frame to the next in a model of those reads (40 with
2048-sample ones), a warble on held notes; averaged, under a cent
(`tests/test_core.c`'s `test_audio_pace` holds it there, and fails on
the old way). A ring past `LAT_MAX` is still cut to its target, the
average with it.

What the device hears then goes through settings.ini's `sound`
(`src/audio/sound_filter.c`, the engine's own sound too): `gba` as it is,
`soft` with the Legacy Collection's balance. That was measured, not
guessed: its net and virus battle themes found in a recording of it by
their notes (chroma matched against our captures, 35 chunks of 6 s), each
chunk's third-octave spectrum against ours on the same bars. Flat from
300 Hz to 5 kHz, it falls away above (-4.5 dB at 8 kHz, -10 at 10, -18 at
13, -29 at 16) and below (-3.5 from 80 to 160 Hz, -12 at 40), the two
songs within a dB of each other. Soft is three biquads fitted to it,
within 0.8 dB from 35 Hz to 17 kHz: a low-pass at 6750 Hz (Q 0.88), a
high-pass at 68 Hz and a low shelf of -2.5 dB at 320 Hz. It takes the
images of BN6's 10512 Hz mix ([Sound and music](../../rom/sound.md), The sound's mix) down
with the rest of the highs. Neither setting touches the game: a headless
capture's frames are the same byte for byte with either.

**The music and the effects** (settings.ini's `music` and `effects`,
Options' Audio; `src/audio/sound_heard.c`). BN6 plays its music on player
31 and each sound on a player of its own, and works each track's volume
out in one routine, TrkVolPitSet, from the track's own, the fade's and its
pan; each of its notes takes what it worked out ([Sound and music](../../rom/sound.md), The
music and the effects silenced). A hook there (`src/emu/sound_quiet.c`)
gives a silenced player's tracks none, right and left, and leaves the rest
as BN6 works it: the notes play on, silent, so the effects take the
channels they took before, and the game runs as it did. A change tells the
tracks that play, of the players it touches, to work their volume out
again before the next frame; so does a state's load, whose tracks hold
what the setting was when it was saved. The engine's own MP2K does the
same in its C (`mp2k_quiet_set`), but for the song the Jukebox plays,
which was asked for; with both off the device hears nothing at all. The
Arena's and the stands' cores take no hook: each player's system runs both
GBAs and hashes their states whole, sound and all (`arena_pair.c`), so a
NetBattle's music plays unless the effects are off too. Proved on the
core's sound dumped (`CYBERWORLD_AUDIO_DUMP`, a Central layer, `--seed 2`):
with the music off nothing from the moment the net's song begins, the
jack-in's effects alone before it; with the effects off nothing in the
jack-in's sound, and the song at the same level (within 0.02 dB) and
envelope (0.95 by 10 ms) as with both; with both on, the same byte for byte
as the build before the hook, and an autopilot run's battles and run log
the same with the effects off.
