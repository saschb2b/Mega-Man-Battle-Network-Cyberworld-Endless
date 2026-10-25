---
type: Engine Mechanism
title: "The engine's own sound"
description: "The engine's own MP2K (`src/audio/mp2k*.c`, `gba_sound.c`): BN6's sound engine rewritten in C for the title and PET screens, proved byte for byte against the game by `tools/mp2k_verify.py`, and BN5's songs on it."
tags: [engine, emulation]
generated: { by: claude-code/unrecorded, at: 2026-10-23T01:11:15Z }
---

# The engine's own sound

Before a run and beside it (the title and its summary, the setup,
Options, SAVES, the PET's screens over the held game) the core is not
running, and the engine plays BN6's songs and sounds itself. It does so
with BN6's own sound engine's rules: MP2K, Nintendo's "m4a", read from
BN6's code in bn6f's disassembly (asm/libs.s; the routines and tables in
Gregar, [Sound and music](../../rom/sound.md), BN6's sound engine) and written again as the
engine's own C (`src/audio/mp2k*.c`), its data the player's ROM's:

- **the sequencer** (`mp2k_seq.c`, `mp2k_note.c`): 32 music players as
  BN6_MPLAYS has them, the music on player 31 and each sound on the
  player its song names; MPlayMain's ticks at the song's tempo against
  150, the commands of BN6's jump table (MEMACC and XCMD as BN6's start
  puts them in), a note given one of BN6's four DirectSound channels (or
  its PSG channel) by priority and track, the LFO, the fade;
- **the mixer** (`mp2k_mix.c`): SoundMainRAM at 10512 Hz, 176 samples a
  frame into a ring of nine frames, the song's reverb from the ring's
  next part, each channel's envelope stepped once a frame, its samples
  (interpolated, or one a step at fixed pitch) added into 8-bit bytes
  that wrap and never clip;
- **the PSG's driver** (`mp2k_cgb.c`): CgbSound's software envelope, its
  extra step every fifteenth frame, and its register writes, which set
  NRx2's start volume and its hardware step and retrigger the channel;
- **the GBA's sound after it** (`gba_sound.c`), as mGBA 0.10.5 plays it:
  the registers' writes as its io.c makes them, mGBA's own PSG (GBAudio,
  run on a clock of its own with the GBA's frame sequencer), the ring's
  samples held from one timer 0 overflow to the next, a frame after they
  were mixed, and the output mixed as GBAAudioSample mixes it into
  blip_buf, read at 48 kHz.

It was proved against the game itself (`tools/mp2k_verify.py`): the plain
ROM in libmgba (romlab, mGBA 0.10.5 as the engine's) with its title's
song-table entry pointed at a song, its MP2K state recorded each frame
(romlab's `sound`), laid beside the engine's `--mp2k-trace` of the same
song from the same phases (`tools/mp2k_diff.py`): the music player and
its eight tracks, the four DirectSound and the four PSG channels, the
reverb and both sides of the mix the same byte for byte in every one of
900 frames, for the title (0x01), the credits (0x1D), the net (0x13) and
seven of the menus' sounds; two bugs planted (the reverb's rounding, a
tie between channels) each failed it. The 48 kHz sound then: the
DirectSound part alone at a correlation of 0.997 with the game's, the
same gain (0.9996), what differs rising with frequency, as the GBA's
timer phase against its 65536 Hz output moves the steps (the phase is
the game's since its power-on; `CYBERWORLD_SOUND_PHASE` sets it, about
584 cycles matching this capture to -30 dB); each PSG channel alone at
the game's level (within 0.4%) and loudness (0.998-0.9999 by 100 ms),
its waveform apart by its square's and its frame sequencer's phases,
which carry over from whatever the game played before. Through the
title's own path the title's song is within 0.4 dB of the game's (the
plain ROM in mGBA at 48 kHz) in every third of an octave, its level
within 0.05 dB. The player it replaced was the engine's own
approximation (float mixing at 48 kHz, its own envelopes, no reverb, 32
channels): 1.6 dB quieter, up to 9 dB short in the deep bass and 1.4 to
2.5 dB short from 7 to 14 kHz, which the owner heard as a duller title
than the cartridge's in mGBA.

`audio_set_volume`'s music volume (8, the default, the game's level) and
settings.ini's `sound` (sound_filter.c) apply after it; on the PET's
screens over a run its sounds add to the core's. There the engine runs
only while it has something to play (`mp2k_busy`), and a song just begun
counts: MPlayStart leaves its player's status 0 till MPlayMain's first
tick, which counted as idle would never come, and the screens' sounds
would never start.

**Another game's songs.** A song of BN5's plays on the same code: BN5's
MP2K library is BN6's byte for byte and its tables are BN6's
([BN5's maps, songs, text and objects](../../rom/bn5-world.md), Another game's songs), so the engine runs BN6's
rules and tables on BN5's song data. Each music player keeps the ROM its
song is in, and each channel its player's at its note, and the part
running reads that ROM (`mp2k_read`): the sequence, the voices, the
samples and the PSG's waves; the library's tables are always BN6's
(`mp2k_lib_u8`). `mp2k_song_start_in` takes the game, `--render-song
x0/ID:SECONDS:PATH` renders BN5's song ID. BN6's songs stay byte for
byte the game's (`tools/mp2k_verify.py`, the title, the net and a menu
sound after the change); BN5's title is the plain BN5 ROM's within the
same bounds as BN6's ([Sources](../../research/sources.md#the-jukeboxs-songs), The Jukebox's songs).

**The Jukebox** ([The PET](../../game/pet.md)) holds the core still while it is up (no
frame of the game runs, so its music stops and its sound ring empties
to silence), and plays its songs on player 31 through `audio.c`'s
`audio_song_*`, which take the audio device's lock: the engine runs on
the sound's own thread, and a call into it from the frame's thread
waits for the mix in hand.
