---
type: Tool
title: "The engine's sound against the game's"
description: "`python3 tools/mp2k_verify.py [SONG...]` lays the engine's MP2K beside BN6's own sound in romlab frame by frame, comparing SoundInfo, channels, registers and the sound for PASS or each first difference, with mute and phase switches."
tags: [devtools]
generated: { by: claude-code/unrecorded, at: 2026-10-23T00:30:46Z }
---

# The engine's sound against the game's

The engine's MP2K (src/audio/mp2k*.c; [The engine's own sound](../engine/emulation/engine-sound.md), The engine's own
sound) laid beside BN6's own, frame by frame:

```bash
python3 tools/mp2k_verify.py            # the title, the credits, the net and seven sounds; PASS or each first difference
python3 tools/mp2k_verify.py 15 6A --frames 1200
```

Each song: the plain ROM in romlab with the song table's entry 1 pointed
at it (BN6's title starts it at frame 294), romlab's `sound N PREFIX`
recording each frame's SoundInfo, its channels and ring, the PSG's
channels, the music player and its tracks (PREFIX.si), the registers by
words (PREFIX.io, masked as mGBA reads them) and the sound at 48 kHz
(PREFIX.wav); the engine's `--mp2k-trace SONG:FRAMES:PREFIX[:C15:COUNTER]`
writing the same records from silence, the game's two phases given (its
PSG envelope's 15-frame count and its ring's, as they stood at frame 293:
3 and 7); `tools/mp2k_diff.py` comparing every part each frame. Into
`.build/mp2k/check/`.

The sound's parts heard alone, on both sides: romlab's `mute MASK` and the
engine's `CYBERWORLD_SOUND_MUTE=MASK` (hex: 1 DirectSound A, 2 B, 4-0x20
the PSG's 1-4; `3C` DirectSound alone). `CYBERWORLD_SOUND_PHASE=CYCLES`
sets timer 0's first overflow in a frame (0 by default), the phase the
game's own power-on decides.
