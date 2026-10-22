# Writing a Battle Network theme

What makes a piece sound like a Battle Network song rather than a generic
chiptune, written as rules to compose from. The numbers come from:

- **Fifteen songs read from the ROMs** with `scripts/song_habits.py GAME
  SONG --detail --theory --bars` (numbers only, nothing kept): BN6's title
  0x01, Central Town 0x03, Judge Tree comp 0x12, net 0x13, Undernet 0x14,
  virus battle 0x15, boss battle 0x16 ("Surge of Power!"), final battle
  0x17, Cybeasts 0x21, crossover battle 0x22 and Navi Customizer 0x1E;
  BN5's title 0x01, net 0x13 and battles 0x15 and 0x16. Their Direct
  Sound samples were measured in memory (length, loop, attack, decay,
  centroid, noisiness, clipping), never written out.
- **The owner's recording of "Surge of Power!"** (in `.build/` only)
  against 0.13.0's piece and the sketches, by numbers: band balance,
  the swing inside a beat per band, noisiness, how full the spectrum is,
  stereo width, loudness bar by bar.
- **The composers' own words**: Akari Kaida on how Battle Network's music
  was made (her 2019 thread, fan-translated by The Rockman EXE Zone:
  <https://www.therockmanexezone.com/general/2019/03/28/akari-kaida-comments-on-battle-networks-music-production/>)
  and Yoshino Aoki's 2009 interview with Chris Greening
  (<https://squareenixmusic.com/features/interviews/yoshinoaoki.shtml>).
  Kaida scored BN1 and BN5, Aoki BN2, BN3 and BN6, Toshihiko Horiyama BN4
  (<https://www.theongaku.com/posts/mega-man-battle-network-sound-box>).
  No written harmonic analysis of any BN track was found (Hooktheory,
  r/VGMtheory, the wikis, Japanese chord-progression pages searched on
  22 October 2026); the ROM is the evidence.

Every claim below marked *ROM* was read from the developer's ROMs with
the scripts here; *Kaida* and *Aoki* are their words (Kaida's through a
fan translation); *opinion* is a writer's.

## Why 0.13.0's piece read as "AI-ish"

The owner on 22 October 2026: "Its current music feels very ai-ish.
Doesn't truly feel like a MMBN theme." It was written from "Surge of
Power!"'s habits, so the gap was never which song, but how the habits
became sound and music. Side by side (the owner's recording, 0.13.0, and
sketch A after the rules below):

| Measure | "Surge of Power!" | 0.13.0 | Sketch A |
| --- | --- | --- | --- |
| Swing inside a beat (95th against 10th percentile of a 10 ms loudness) | 7.1 dB | 3.4 dB | 6.9 dB |
| ...in the lows / mids (500-2000 Hz) / highs (2-6 kHz) | 18 / 6.0 / 8.3 dB | 8 / 2.9 / 3.1 dB | 26 / 4.7 / 5.9 dB |
| Energy 60-120 / 120-500 / 500-2000 Hz | 49 / 24 / 18% | 44 / 15 / 25% | 42 / 30 / 17% |
| Noisiness (spectral flatness, 100-6000 Hz) | 0.075 | 0.022 | 0.054 |
| Tonal share of the energy | 84% | 93% | 80% |
| Spectrum filled (cells within 30 dB of the frame's peak) | 54% | 30% | 47% |
| Stereo width (side against middle) | -10.7 dB | -15.5 dB | -12.7 dB |
| A lead bar's rhythm heard again within 8 bars | 22% (BN battles 22-36%) | 48% | 22% |
| Chord changes a bar | 1.85 (BN 1.3-1.9) | 1.36 | 1.89 |

**The sound alone does not fix it.** 0.13.0's own notes rendered through
the new kit (its drums the kit's short clipped ones, its squares
unsmoothed, its arpeggio ping-ponged; `.build/trailer-sketches/`
`v0_13_0_new_kit`, never committed) measure nearly as before: a swing of
2.8 dB inside a beat, flatness 0.028, the spectrum 33% filled, the lows
above half their peak 73% of the time (the game's 43%). Sketch A, the
same kit written by the rules below, measures 6.9 dB, 0.054 and 47%. The
writing and the arrangement were most of the gap; the samples the rest.

Part by part, what read as generic:
- **A wall, not a beat.** Every band swung half as much inside a beat as
  the game's: squares held at sustain 13 with no gaps, the bass held
  under everything, pads on every bar. The game's notes stop: its kick
  and clap are short and loud, its stabs and chugs stop dead.
- **Too clean.** Pure tones in empty spectrum (flatness a third of the
  game's, the spectrum 30% filled against 54%): band-limited squares, soft
  synthesized drums, saw pads. The game's sound is 8-bit samples of a
  synthesizer's chord, guitar and drum sounds, clipped, with chorus and
  distortion baked in, and squares the console does not smooth.
- **Hollow between 120 and 500 Hz.** The game fills it with chord samples
  and a low distorted stab played with pitch bends; 0.13.0 had nothing
  there but the bass's harmonics.
- **The drums.** 0.13.0's kick was a 260 ms soft sine; BN6's is 139 ms
  with half its samples at full scale, 20 dB down 66 ms in (*ROM*). Its
  clap 82 ms, a quarter at full scale; 0.13.0's snare 220 ms.
- **Static stereo.** Each square sat at one pan. Every BN6 song read but
  the Cybeasts' has a track thrown hard left and right note by note (pan
  0 and 127: 0x16's arpeggio 212 times, 0x17 240, 0x1E 190); BN5's swing
  theirs wide (40 and 88: 0x13 281 times, 0x15 206), its final battle 0
  and 127 (*ROM*).
- **No bends.** 0x16's bass bends 25 times (range 12: falls), its low
  stabs 21 times; 0x1E's bass 37 (range 4: slides); the final battle 410,
  BN5's 0x15 102 (*ROM*). 0.13.0 bent nothing but a scoop at a phrase's
  end.
- **One rhythm cell everywhere.** Its call and answer spoke one 7-note
  cell in 15 of 23 bars, each bar a complete phrase ending on a long note.
  BN leads almost never repeat a bar in the next (0-2% of bars, BN5's
  0x15 14%) and come back at the phrase: the same bar 4 bars on in 7-32%
  (*ROM*).
- **Plain harmony at the cadences.** A major V (and V7) closed its phrases
  (12% of its half bars); BN6's battle themes close on bVII, the minor v
  or a suspension, and change chords twice a bar.

## Harmony

1. **Two chords a bar.** Every song read changes chord 1.3-1.9 times a
   bar (*ROM*); hold one a whole bar only at a section's head.
2. **Colour every chord.** As the script's chord matcher reads them
   (a template a half bar, so rough), plain major and minor triads are
   under a fifth of the chords in most songs (8-18% in BN6's title, virus
   and boss battles; up to 45% in its crossover battle); sevenths (m7,
   maj7, 7, m7b5) 40-60% in most (17-90% in all); suspensions 6-57% (BN5's
   net theme the most); bare fifths in the battle themes 12-17% (*ROM*).
   Resolve a sus4 onto the beat where the chord turns major.
3. **Minor battle themes are Aeolian and Dorian**: i, bVII, bVI, bIII, iv,
   the Dorian IV major, the minor v; a bIImaj7 for colour (4-8% in 0x15
   and 0x16). Roots step by a whole tone (0x16: up 21%, down 16%) or move
   by fourths and fifths (25%) (*ROM*). Keep the major V for one
   turnaround's surprise.
4. **Major themes** (net, Navi Customizer, town, title) lean on IVmaj7,
   ii7, iii7, the Mixolydian I7 and bVII, the borrowed iv; roots by
   fourths and fifths (Navi Customizer 52%, town 46%) or falling by steps
   (net: down a semitone 24%, a tone 20%), BN5's title on the Lydian's
   raised fourth (*ROM*).
5. **Dark places** (Undernet, final battle): diminished and
   half-diminished chords (46% and 32%), roots a tritone apart (31-56%)
   (*ROM*).
6. **Few samples, many chords.** BN5 had one major and one minor chord
   sample, recoloured by the bass: C over A is Am7, over F an F9 (*Kaida*).
   BN6's chord samples are suspended voicings (0x16's hold G-A-D and
   D-G-A) (*ROM*). In the kit: `chord_pad` and `brass_section` record one
   chord; play it over another bass (Gm over Eb is Ebmaj7, F over Bb is
   Bbmaj9).
7. **A fake-out** every eight bars or so: a chromatic mediant, a bII, a
   borrowed bVI where the ear expects home. A video analysis of "Surge of
   Power!" names fake-outs, unexpected chord changes and dissonant notes
   among its tools (8-bit Music Theory, <https://www.youtube.com/watch?v=H_a-78XJGl4>,
   chapter titles only) (*opinion*).

## Melody

1. **A hook of one bar, answered, stated again, cadenced** (four bars:
   A B A' C), then a B section that climbs in sequence, then the hook on a
   new chord. Never the same bar twice running (BN: 0-2%); the opening
   bar back four bars later (*ROM*).
2. **Vary the rhythm bar to bar.** A BN lead's bar rhythm comes back
   within eight bars in 22-36% of bars in the battle themes, up to 58% in
   the calmer ones (*ROM*). A one-bar cell in every bar is the AI tell.
3. **Long notes held with vibrato**, pushed across the bar line; BN6's
   leads set MOD 5-7 at LFO speed 28 after 14 ticks, its bass 10; BN5's
   3-12 (*ROM*): in the kit `vib=(20-30 cents, 10-14 frames)`. Battle themes hammer a note twice
   (two sixteenths) before a leap.
4. **Range**: leads from the fourth octave to the sixth (0x16's squares
   53-94) (*ROM*).
5. **The two squares are two jobs**: the lead on one, the backing as an
   arpeggio on the other, trading at section changes. Kaida: with melody,
   bass and noise drums taking three of the PSG's four channels, the
   whole accompaniment went into one channel's fast arpeggios, short
   blips so it came back quickly after a sound effect took it (*Kaida*).

## The backing squares

- **32nd-note pairs** in battle: each sixteenth struck twice, loud then
  soft (0x16: velocities 100 and 52), 25% duty (*ROM*); in bouncier songs
  sixteenths accented 92 and 40 (0x1E) (*ROM*).
- **Ping-pong**: thrown hard left and right note by note (*ROM*, above).
- **Accents, not humanising.** BN's sequences use 2-13 velocity levels a
  track in fixed accent patterns, and every onset on the 32nd-note grid
  (*ROM*): no random velocity, no timing jitter.

## Bass

- The wave channel: a square's table (odd harmonics) in 0x16, a saw's in
  0x15, 0x1E and 0x13 (*ROM*).
- Eighths and sixteenths (lengths 12 and 6), roots on the kick's places,
  octaves between, held nearly to the next note (0x16: legato 78%); in
  bouncy songs half the notes cut short (0x1E) (*ROM*).
- Never below D2 (0x16's bass 39-65, 0x1E's 38-63) (*ROM*): the sub
  stays clean and the kick owns 60-120 Hz.
- Bends: falls at phrase ends (range 12), slides up into notes (range 4);
  vibrato on held notes (MOD 10) (*ROM*). In the kit: `bend=[(tick,
  semitones), ...]` on any note.

## Drums

- **Samples short and loud** (*ROM*, 0x16): kick 139 ms, half its samples
  at full scale, 20 dB down by 66 ms, its energy at 72-115 Hz; clap and
  snare 82 ms, noisy, a quarter at full scale; toms 354 ms and dark. Hats,
  open hats and the crash on the noise channel (envelopes (0, 0, 11, 0),
  (0, 2, 11, 3), (0, 7, 0, 1)). In the kit: `punch_kick`, `punch_clap`.
- **Syncopated kicks** (sixteenths of the bar, *ROM*): the boss battle
  0, 6, 7, 10, 14; the net theme 0, 2, 6, 8, 10, 14, 15; the Navi
  Customizer 0, 6, 14 and around. Snare or clap on 4 and 12 with ghosts
  (the title's snare also on 2, 6, 13: a broken beat).
- Hats in sixteenths at two or three levels; open hats on off-beats; a
  crash at the phrase heads; toms falling into a section. Hand drums and
  shakers in the lighter songs (0x1E, 0x13) (*ROM*).

## Arrangement

- **Six tracks**: two squares (lead, arpeggio), the wave's bass, the kit
  (samples and noise), two Direct Sound tracks for stabs, chord samples
  and a lead's double (*ROM*). Music had two or three sample voices at a
  time under a game's load (*Kaida*): few at once, each doing a lot.
- **Direct Sound's jobs** (0x16, *ROM*): suspended chord samples with a
  slow attack (516 ms, looped), a low distorted stab with bends (316 ms,
  energy at 250-2000 Hz: the "guitar samples" players hear in it,
  <https://www.youtube.com/watch?v=8Px3cX9aci4>, *opinion*), a clean lead
  of a few cycles looped (1426 samples, loop 23), a noisy hit.
- **One loudness**: a section holds its level (the recording's bars
  vary by 0.65 dB, a standard deviation);
  contrast comes from the squares swapping jobs and parts dropping out,
  not from fades. A trailer's drop is the exception, kept short.
- **Each game its own atmosphere**: Aoki aimed each sequel to contrast
  with the last while keeping the series' feel, writing from what the
  player should feel, and pushed the GBA to sound powerful (*Aoki*).

## Mix and timbre

Aim for, against the owner's recording of "Surge of Power!" (the kit's
`music_check.py --ref`):
- Bass, body, mids and top (below 120 Hz, to 500, to 2000, above):
  about 57, 28, 13 and 1% at 8 kHz (sketch A: 54, 31, 13, 1).
- Punch, the swing inside a beat: 7-8 dB (0.13.0: 3.8).
- No bar near clipping; loudness steady bar to bar.

What the kit does for it (`tools/trailer_music.py`):
- `HARDWARE = 32768` on a piece: squares unsmoothed, their edges on the
  console's output rate, folding back as the console's do.
- Samples in the manner of BN6's, all synthesized here: `punch_kick`,
  `punch_clap`, `chord_pad` (one chord, looped, its attack swelling),
  `power_stab` (a distorted power chord, thinned under 260 Hz, `mute` for
  palm-muted chugs), `cycle_lead` (a few cycles looped), `slap_bass`,
  `fm_bell`, `brass_section` (one chord), `conga`, `impact`.
- `bend` on PSG and Direct Sound notes, followed a frame at a time.
- Hard pans per note are the piece's: alternate `pan` -0.9 and 0.9.

A soundfont's sampled instruments (FluidR3_GM, MIT; GeneralUser GS, free
for music) brought down to 10512 Hz and 8 bits would be nearer still to
the XV-1080 and SC-88 sounds BN was made from (*Kaida*). Not done: it
needs the owner's go-ahead to download one, and a trailer that used it
would need the 3DS build's banner rendered once and kept as our own audio,
since `build.py 3ds` renders the banner with the standard library.

## Checks before the owner listens

- `music_check.py --piece NAME --ref RECORDING`: the bars, the four-band
  balance, the punch, and `lead notes rubbing the harmony` (a piece names
  its `CHORD_SAMPLES`); read each rub and keep it only if it is a colour
  on purpose (a maj7 over its root).
- The roll (`roll.png`): the hook's returns visible, the lead's rhythm
  changing bar to bar, the bass in its octave.
- Sketch before the piece: three eight-bar sketches in different moods,
  rendered to `.build/trailer-sketches/` as WAV and MP3, for the owner to
  choose by ear before the full piece is written (owner, 22 October 2026).
