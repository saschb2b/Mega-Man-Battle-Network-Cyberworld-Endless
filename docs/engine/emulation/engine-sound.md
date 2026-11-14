---
type: Engine Mechanism
title: "The engine's own sound"
description: "The engine's own MP2K (`src/audio/mp2k*.c`, `gba_sound.c`): BN6's sound engine rewritten in C for the title and PET screens, proved byte for byte against the game by `tools/mp2k_verify.py`, BN5's and BN4's songs on it, and the game's own song carried on under the engine's screens over a run and given back."
tags: [engine, emulation]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-05T01:00:00Z }
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
BN4's songs play on it too (`--render-song x1/ID:SECONDS:PATH`, BN4's ROM
read beside BN6's), as BN6's engine plays their copies: BN4's library is
another build of BN6's, its tables BN6's byte for byte, its sound engine
otherwise in two ways a run would hear (its master volume 12, and a PSG
channel the hardware stopped kept: [BN4 Red Sun's tables](../../rom/bn4.md), Its sound engine
against BN6's).

**The Jukebox** ([The PET](../../game/pet.md)) holds the core still while it is up (no
frame of the game runs, so its music stops and its sound ring empties
to silence), and plays its songs on player 31 through `audio.c`'s
`audio_song_*`, which take the audio device's lock: the engine runs on
the sound's own thread, and a call into it from the frame's thread
waits for the mix in hand.

**The game's music under the engine's screens.** The other screens of
the engine's that hold the core still over a run (Options and its
Controls, the touch controls' menu and editor, the Library, a reward
pick, the Battle Lab's builder, the dev menu) stopped the game's music
dead with it, where BN6's own PET pages play on over the map's song.
They carry it on now, the owner's ask of 14 November 2026
(`src/audio/music_carry.c`, `mp2k_carry.c`): at the first frame held,
the game's music player (31) is read where BN6 keeps it (its
MusicPlayerInfo, its eight tracks, SoundInfo's four DirectSound
channels and the PSG's four, the reverb and the PSG's 15-frame count:
[Sound and music](../../rom/sound.md), The game's music carried) and
taken by the engine's player 31 field for field (`mp2k_layout.c`, the
layout `--mp2k-trace` writes): the song goes on from the tick where the
game was held, its notes sounding on, the PSG's set going again on the
engine's registers. Its data is read from the core's ROM copy, a byte
at a time (`emu_rom_byte`, gpSP's pages too), where another game's song
copied in lives. As the core runs again the song is given back the same
way: the game's RAM takes what the engine played since, the PSG's notes
set going on the game's own registers (`emu_io_write8`: mGBA's
`GBAIOWrite8`, gpSP's store, as the game's own store writes them), and
the game plays on from there. A channel sounding another's sound keeps
it, an effect of the engine's as the song is taken (Options' opening
sound), an effect of the game's as it is given back: the music's note
there is let go, and a channel given back that another track still
listed comes off that track's list as BN6's ClearChain takes it off.
Where the game plays nothing (between songs, its music stopped), the
screen plays BN6's Navi Customizer theme (0x1E, the NaviCust's screen's
own song), faded out as the game runs again. The Jukebox plays its own
songs, and a NetBattle from the Net Square its own cores' sound: there
nothing is carried. A song given back goes only to the game as it was
taken; one loaded meanwhile (a test battle's snapshot) keeps its own.

The two run the same code byte for byte, so the song comes back where
the game would have had it. Measured on 14 November 2026 with the
check `CYBERWORLD_CARRY_CHECK=N:M:HOW@FRAME` (at the engine's frame
FRAME the game's music handed to the engine for N frames and given back,
HOW 1, or left to the game, HOW 0, then M frames of the game; the
music's state and the PSG's registers 0x04000060-0x0400009F on stderr,
and the game ends): in the first battle of `cw.py shot battle`, at frame
900, `300:120:1@900` and `300:120:0@900` printed the same lines, the player, its
eight tracks, every channel and the PSG's registers byte for byte, a PSG
note of the song held through the hand-over and given back; in Lan's
room (`--scene town --seed 2`) over the room's song's loop,
`1500:200:1@560` against `1500:200:0@560`, the same but for the leftover
bytes of one silent channel of no track, where the game left to itself
had played a sound of its own in those 1500 frames. Heard: with
Options opened over Lan's room (`cw.py shot pet/options`, the sound
dumped with `CYBERWORLD_AUDIO_DUMP` and `CYBERWORLD_AUDIO_OFFLINE`), the
build before fell silent from Options' opening; this one holds the
room's song at its level all through, with no gap or doubling at either
seam.
