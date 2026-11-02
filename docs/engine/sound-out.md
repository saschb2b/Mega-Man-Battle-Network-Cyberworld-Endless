---
type: Feature
title: "The sound out: levels, output and filters"
description: "Options' Audio sets how the game is heard on the player's speakers, handheld or headphones: Music and Effects in ten steps of 3 dB (`sound_heard.c`), Output as Speakers, Headphones (bs2b's crossfeed, `crossfeed.c`) or Mono, and Sound as GBA, Soft or mGBA's libretro Audio Filter step for step (`sound_filter.c`); what each does, where it runs, what a NetBattle keeps alike, and how each was checked against its source's own code and measured."
tags: [engine, sound, options]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-02T01:40:00Z }
sources:
  - id: mgba-libretro
    resource: "https://github.com/libretro/mgba/blob/master/src/platform/libretro/libretro.c"
    title: "mGBA's libretro core: _audioLowPassFilter and _loadAudioLowPassFilterSettings; libretro_core_options.h's mgba_audio_low_pass_filter and mgba_audio_low_pass_range (read on 2026-11-02, master at 7a12d6d4b9)"
  - id: mgba-libretro-history
    resource: "https://github.com/libretro/mgba/commits/master/src/platform/libretro/libretro.c"
    title: "The core's output rate by commit: 32768 Hz until 0f39405ece (2025-01-25, \"Use resample rate of 65536\"), the frontend's target rate from c758314a63 (2025-10-13), a fixed 65536 Hz again from 517e518c48 (2026-08-03)"
  - id: libbs2b
    resource: "https://github.com/alexmarsev/libbs2b/blob/master/src/bs2b.c"
    title: "libbs2b 3.1.0 (Boris Mikhaylov, MIT): init(), cross_feed_d() and bs2b.h's levels (read on 2026-11-02)"
  - id: gag-mono
    resource: "https://gameaccessibilityguidelines.com/provide-a-stereo-mono-toggle/"
    title: "Game Accessibility Guidelines: Provide a stereo/mono toggle (Hearing, Intermediate)"
  - id: gag-volumes
    resource: "https://gameaccessibilityguidelines.com/provide-separate-volume-controls-or-mutes-for-effects-speech-and-background-music/"
    title: "Game Accessibility Guidelines: Provide separate volume controls or mutes for effects, speech and background / music (Hearing, Basic)"
  - id: xag-105
    resource: "https://learn.microsoft.com/en-us/gaming/accessibility/xbox-accessibility-guidelines/105"
    title: "Xbox Accessibility Guideline 105: Audio accessibility (separate volumes; stereo to mono sent to both channels)"
---

# The sound out: levels, output and filters

Everything the device plays, BN6's core in a run and the engine's own
MP2K around it, passes one chain at 48 kHz stereo on the device's thread,
the same on every platform (`sound_filter_run`): the **Sound** line's
filter, then the **Output**'s layout, then the louder of the two
**levels** as a gain, rounded once at the end. With the defaults (GBA,
Speakers, Music and Effects at 10) the chain is skipped and every sample
leaves as it came: an autopilot run's sound dumped (`--seed 2`, 7000
frames) is the same byte for byte as the build before these lines, and so
is `--render-song` (which now plays through the chain as the device
would). Soft is the same byte for byte as before (the Legacy
Collection's balance, measured: [The core's sound](emulation/core-sound.md)).

The tension these serve is the cartridge's sound against where the
player listens; the defaults stay the cartridge's, and each line is a
choice for the player's own place. They follow the accessibility
guidelines' hearing items: separate volumes for music and effects (GAG,
Hearing, Basic; XAG 105) and a mono toggle (GAG, Hearing, Intermediate;
XAG 105), sources in [Sources](../research/sources.md), Sound settings.

## Music and Effects: ten steps

Each is a level from 10 (the game's own) down to 1, 3 dB a step (1 is
-27 dB), and Off. Left and Right stop at Off and 10, so neither leaps
from silence to the loudest; A or a tap goes round from 10 to Off.
`settings.ini` keeps the number (`music = 7`); an older file's `on` reads
as 10 and `off` as 0.

BN6 mixes its music and its effects into one 8-bit stream, so they can be
set apart only where BN6 works out each track's volume (TrkVolPitSet,
[Sound and music](../rom/sound.md)). The two are split so that the least
passes through BN6's 8 bits:

- **The louder of the two is the device's gain**, in doubles after
  everything is mixed: both turned down together is exact (music and
  effects at 7 is -9.0 dB in every third-octave band, at 2 -24.0).
- **The quieter is its share of the louder**, in BN6's own track
  volumes: the core's hook (`sound_quiet.c`) works the volume out as BN6
  does and keeps its share, in 256ths, rounded (music at 4 under effects
  at 10 keeps 32 of 256); the engine's MP2K does the same
  (`mp2k_shares_set`). At 0 the share is none, as the music and effects
  were silenced before.

What the share costs, measured on three songs' renders (the net, a virus
battle, the title) against the intended level: within -0.3 to +1.3 dB of
it for gaps of up to 12 dB, the instruments' balance moving 1 to 5 dB
between bands (BN6's volumes at 8 bits, the PSG's at 4); at 18 dB apart
and more the quietest parts fall silent, as in BN6's own fades, the music 3
to 7 dB under its level and its stereo narrower (music 4 under effects
10 in a run: 23 dB down where 18 was meant). Both at the same level never
pays it.

## Output

| Value | What it does |
| --- | --- |
| **Speakers** | The stereo as the game makes it. |
| **Headphones** | bs2b's crossfeed at its default level (700 Hz, 4.5 dB, `crossfeed.c`): each ear gets the other side's sound low-passed at 700 Hz, 4.5 dB under its own at the lowest notes and delayed by the filter as a head delays it, its own side's highs lifted; the whole 1.8 dB lower in the highs, as bs2b keeps a mono sound's level. BN6's music is mildly stereo (its side 8 to 10 dB under its mid; its effects are mono), so it narrows the image a little: the net's song's correlation 0.88 to 0.95. |
| **Mono** | Left and right halved and added, on both: for one speaker, one earbud, or hearing on one side. A GBA's own speaker is mono, its stereo only in headphones. BN6's sound loses 0.35 to 0.76 dB folded. |

bs2b is Boris Mikhaylov's (2005, MIT; the notice in `crossfeed.c`). Its
`init()` and `cross_feed_d()` are written there for one rate; against
libbs2b 3.1.0's own code built beside it (`bs2b_set_level` at
`BS2B_DEFAULT_CLEVEL`, 48 kHz), 15.6 million frames of eight songs and
noise came out the same to the last bit of a double.

## Sound: GBA, Soft, mGBA

**mGBA** is mGBA's libretro core's Audio Filter as RetroArch's players set
it (`mgba_audio_low_pass_filter`, its level `mgba_audio_low_pass_range`,
5 to 95 %, 60 by default; here `sound = mgba` and `low_pass = 60`):
`_audioLowPassFilter`, every step of it. One pole in 16.16 fixed point,
`factorA` the level's share of 0x10000 (`(level * 0x10000) / 100`),
`factorB` the rest; each sample `(prev * factorA + in * factorB) >> 16`,
the shift rounding down (a fall from below settles at -2, as mGBA's does),
the state kept between calls and across a change of level. Cut from
`libretro.c` into a scratch build beside `sound_filter.c` and fed the same
samples in mGBA's chunks of 1024 and a device's of 735, all 19 levels over
eight songs and noise gave the same 592,800,000 samples out.

A one-pole filter is defined per sample, so where it cuts follows the
rate it runs at. mGBA's libretro core has run it at 32768 Hz (until
January 2025), at 65536 Hz, at the frontend's own rate (October 2025 to
August 2026: 48 kHz on most RetroArch setups) and at 65536 Hz again
since. It runs here at the device's 48000, as that core did on a 48 kHz
RetroArch, so the same samples in give the same out; a player matching
today's core takes a lower level:

| Level | -3 dB at 32768 Hz | at 48000 (here) | at 65536 |
| --- | --- | --- | --- |
| 30 % | 7231 Hz | 10592 Hz | 14462 Hz |
| 40 % | 5155 | 7551 | 10310 |
| 50 % | 3769 | 5521 | 7538 |
| 60 % (default) | 2724 | 3990 | 5448 |
| 70 % | 1880 | 2754 | 3760 |
| 80 % | 1169 | 1712 | 2337 |
| 95 % | 268 | 392 | 535 |

Today's core's 60 % is about this one's 50 %. Against GBA on the same
renders, 60 % takes 1 dB off at 2 kHz, 5.4 at 6.3 kHz, 8.2 at 10 kHz and
11.7 at 20 kHz, a gentler slope than Soft's (-7.8 at 10 kHz, -32.6 at 20)
and none of Soft's bass cut; 30 % takes 2.8 dB at 10 kHz, 90 % 21.3.

## A compressor: not needed

Measured before building one: BN6's songs hold their loudness within 0.6
to 2.5 LU (EBU's loudness range; eight songs, 40 s each) at 11 to 14 dB
from peak to loudness, and an autopilot run's 116 s with battles within
5.9 LU, the effects lifting the momentary loudness 3 to 6 LU over the
music. The mix is already dense; the effects' jumps are what Effects'
level is for.

## A NetBattle

Each player's system runs both GBAs and hashes their states whole
([What runs](../online/arena/what-runs.md)), so nothing that writes the
game's memory may differ between them: the levels' shares, which write
BN6's track volumes, stay off the Arena's cores, as the silencing did,
and there the device hears both GBAs at the louder of the player's two
levels (nothing where both are 0). The filter, the output and that gain
come after the cores and are each player's own.

## In Options

The Audio section's lines, top to bottom: Music, Effects, Output, Sound,
then Vibration and Trigger effects where the controller has them (six at
most). Each change is heard at once, the cursor's sound passing through
the new chain. In a run the game waits under Options with its music
stopped, so a music level is heard on the title (its song plays there) or
back in the game. The browser player's Sound button turns GBA, Soft and
mGBA in turn, as Options does.
