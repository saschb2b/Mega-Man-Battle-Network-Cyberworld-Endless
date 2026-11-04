---
type: Tool
title: "The engine's sound against the game's"
description: "`python3 tools/mp2k_verify.py [SONG...]` lays the engine's MP2K beside BN6's own sound in romlab frame by frame, comparing SoundInfo, channels, registers and the sound for PASS or each first difference, with mute and phase switches; `xN/SONG` lays another game's song, copied into BN6 by xsong.c, beside that game playing it."
tags: [devtools]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-04T02:10:00Z }
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

## Another game's song copied into BN6

```bash
python3 tools/mp2k_verify.py x1/12        # BN4 Red Sun's net theme
python3 tools/mp2k_verify.py x1/all x0/all   # every song of BN4's music (0x00-0x21) and BN5's (0x01-0x24)
```

`xN/SONG` (x0 BN5, x1 BN4, as `--render-song` names them) proves
[`src/audio/xsong.c`'s copy](../multirom/music.md) against the song's own
game: that game in romlab, its title's song entry pointed at the song
(BN4's entry 0 and BN5's 1, their songs' first engine frame 198), recorded
by `sound N PREFIX SI PLAYERS` from its own SoundInfo and player table;
beside it BN6 in romlab with the copy written where the copies go
(`FREE_SONGS`, romlab's `pokef ADDR FILE`; mGBA's patch grows the ROM past
its 8 MB), its title's entry pointed at the copy, and at frame 294 given the
other game's phases (SoundInfo +0x0A and +0x04) and master volume (+0x07)
as they stood the frame before its song. The copy is the engine's own:
`--xsong-pack xN/SONG:PATH` writes the song as `xsong_pack` lays it out
from `FREE_SONGS` and prints its header's address.

`tools/mp2k_diff.py ... --map MAP` then compares the two as it compares
the engine's trace, the other game's pointers moved first: its SoundInfo,
PSG channels, music player and tracks to BN6's places, and each piece of
the song (its sequence, voice group, samples, waves, key splits' tables
and sub-voices, drum kits) to where the copy has it, the map walked
through the original and the copy side by side, each piece's bytes
compared on the way. A pointer the copy did not move, a sample copied
wrong or a note played otherwise is a FAIL. Into `.build/mp2k/check/`
(`xN_SONG.pack`, `.map`, `_game.*`, `_ours.*`).

