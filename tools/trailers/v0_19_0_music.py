"""0.19.0's music: sketch A, "Hometown" (sketch_town_a.py), grown into the
cut's 23 bars. Original, in the manner of BN5's ACDC Town (BN5 Team
Colonel's song 0x03, its home town's theme), the owner's reference for
this release for its happy, upbeat rhythm; none of its notes, nor its
chords in their order. Its habits, read from the ROM, are in
sketch_town_lib.py's docstring; the palette is FluidR3 GM's (SOUNDFONT),
rendered in docker/Dockerfile.music's image, its 3DS banner kept as our
own audio (BANNER_AUDIO).

Every part plays FluidR3 GM's instruments (the owner's gate 3, SKILL.md):
the lead, 0x03's square, on 81 Saw Wave doubled by square 1 at 50%; the
backing's blips, its other square, on 13 Xylophone doubled by square 2
at 25%; the bass, its wave, on 34 Electric Bass (pick) (SAMPLED_PARTS);
the chords on 4 Electric Piano 1 and 48 Strings, the low answer on 61
Brass Section, the C's melody on 81, the kit 24's; and a counter-part of
our own in 0x03's busy sampled track's job (short notes from the chords,
from the other side note by note), 8 Celesta in the A and the recap, 11
Vibraphone in the B, 45 Pizzicato Strings in the C. The GBA's noise
channel keeps the hats and the crash. Its highs above 3 kHz match the
recording's (6.0%). (The first cut played the lead, the blips and the
bass on the squares and the wave, and the owner heard no soundfont in
it; the second on samples alone, and he heard it dull: 2 November 2026.)

The owner waived his pick of a sketch for this release (2 November 2026:
"Don't wait for my approvals"). Of the two written, A was chosen: every
part's motion near 0x03's (music_check.py: "busier than the source:
none", B's square too still under its melody), no lead note rubbing the
harmony, and its balance nearest the recording's.

What it keeps of 0x03: 112 beats a minute (112.5 here, a bar 64 video
frames); a major key on suspended Vs, ii and vi sevenths, the Mixolydian
v and I7, a bVI7 for colour, two chords a bar, roots up by fourths and
tones; the square's four-bar breath (a call, a busy answer of repeated
sixteenths, the call again, a held bar); the backing's 32nd blips on every
sixteenth thrown hard left and right, the B's in 32nds; the bass high,
its sixteenths repeating the root with an octave on the kick, one note a
bar held and sliding on; the mellow chord sample struck on the beat and
its third and fourth sixteenths; a low answer by step; the kit's kick on
0 7 10 14 and snare on 4 and 12, every sixteenth's hat, the open hat on
14; an A, a B and a C where a sampled melody of repeated sixteenths leads
and the square holds back.

The form, on the cut's slots (SKILL.md, The structure):
- Bars 0-1, the cold open: the blips and the electric piano's chord, the
  hats rising; then the band without its lead on Ebsus4 Fsus2, the
  call's pickup.
- Bars 2-3, the title card: the call (B-flat struck three times, up to
  E-flat, F held, a turn on G) over Ebsus4 Fsus2, the busy answer over Dm7
  Gm7.
- Bars 4-7, the new Options: the call lower over Cm7 Fsus4, the held bar
  over Bbsus2 Bb7 with the brass's low answer, the call again, the
  answer varied; the toms into the next.
- Bars 8-12, the GBA's screen: the B, the backing in 32nds and the clap,
  the lead climbing in sequence over Ebmaj7 Fm7 | Bb7 Gb7 | Cm7 Dm7 |
  Ebsus4 Fsus4; the call back over Ebmaj7 Fsus4 with the saw an octave
  under it, the climax.
- Bars 13-16, the sound: one bar thinned to the blips and the electric
  piano on the borrowed Abmaj7 Bbsus2 (the drop, kept short), then 0x03's
  C: the saw's melody of repeated sixteenths over Gm7 Cm7 | Fsus4 F |
  Ebmaj7 Dm7, the strings held, the square resting.
- Bars 17-18, the recap: the call over Ebsus4 Fsus2, then Cm7 Fsus4 held,
  E-flat falling to C, then to the stinger's D.
- Bar 19, the stinger: B-flat on the downbeat, ringing.
- Bars 20-22, the end card: the blips alone, softly, over Ebmaj7 Fsus2 |
  Gm7 Fsus4 | B-flat suspended, the strings' last chord under them.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from trailer_music import Score  # noqa: E402
from trailers import sketch_town_lib as L  # noqa: E402
from trailers.sketch_town_lib import (BPM, CHORD_SAMPLES, DS_ENV, DS_GAIN, DS_LEVEL,  # noqa: E402,F401
                                      HARDWARE, REVERB, SAMPLED_LEADS, SAMPLED_PARTS, SOUNDFONT, instruments)
from trailers.sketch_town_a import CALL  # noqa: E402

BARS = 23
TAIL = 2.0
# (the sketch's mix with the bass a little up: the recording's lows)
MIX = tuple((name, ch, 0.17 if name == 'bass' else g) for name, ch, g in L.MIX)
BANNER = (2 * 4 * 60 / BPM, 2.9)         # (from bar 2's downbeat, seconds: the call with the title)
BANNER_AUDIO = '3ds/banner-0.19.0.wav'

CHORDS = {0: ('Bbsus2', 'Fsus2'), 1: ('Ebsus4', 'Fsus2'), 2: ('Ebsus4', 'Fsus2'), 3: ('Dm7', 'Gm7'),
          4: ('Cm7', 'Fsus4'), 5: ('Bbsus2', 'Bb7'), 6: ('Ebsus4', 'Fsus2'), 7: ('Dm7', 'Gm7'),
          8: ('Ebmaj7', 'Fm7'), 9: ('Bb7', 'Gb7'), 10: ('Cm7', 'Dm7'), 11: ('Ebsus4', 'Fsus4'), 12: ('Ebmaj7', 'Fsus4'),
          13: ('Abmaj7', 'Bbsus2'), 14: ('Gm7', 'Cm7'), 15: ('Fsus4', 'F'), 16: ('Ebmaj7', 'Dm7'),
          17: ('Ebsus4', 'Fsus2'), 18: ('Cm7', 'Fsus4'), 19: ('Bb',),
          20: ('Ebmaj7', 'Fsus2'), 21: ('Gm7', 'Fsus4'), 22: ('Bbsus2',)}

ANSWER = [(0, 6, 'D5'), (6, 6, 'D5'), (12, 6, 'D5'), (18, 6, 'C5'), (24, 6, 'D5'), (30, 6, 'F5'), (36, 6, 'F5'),
          (42, 6, 'D5'), (48, 6, 'D5'), (54, 6, 'D5'), (60, 6, 'C5'), (66, 6, 'D5'), (72, 12, 'G5'), (84, 12, 'F5')]
LINES = {
    2: CALL,
    3: ANSWER,
    4: [(0, 6, 'G4'), (6, 6, 'G4'), (12, 6, 'G4'), (18, 6, 'Bb4'), (24, 12, 'C5'), (36, 6, 'C5'), (42, 6, 'C5'),
        (48, 18, 'Bb4'), (66, 6, 'Bb4'), (72, 6, 'C5'), (78, 6, 'C5'), (84, 12, 'Eb5')],
    5: [(0, 24, 'D5'), (24, 12, 'D5'), (36, 12, 'D5'), (48, 24, 'C5'), (72, 12, 'F5'), (84, 12, 'F5')],
    6: CALL,
    7: [(0, 6, 'D5'), (6, 6, 'D5'), (12, 6, 'D5'), (18, 6, 'C5'), (24, 6, 'D5'), (30, 6, 'F5'), (36, 12, 'G5'),
        (48, 6, 'F5'), (54, 6, 'F5'), (60, 6, 'F5'), (66, 6, 'D5'), (72, 24, 'Bb4')],
    8: [(0, 6, 'G5'), (6, 6, 'G5'), (12, 6, 'G5'), (18, 6, 'F5'), (24, 12, 'G5'), (36, 12, 'G5'), (48, 6, 'Ab5'),
        (54, 6, 'Ab5'), (60, 6, 'Ab5'), (66, 6, 'G5'), (72, 12, 'Ab5'), (84, 12, 'C6')],
    9: [(0, 6, 'Ab5'), (6, 6, 'Ab5'), (12, 6, 'Ab5'), (18, 6, 'G5'), (24, 12, 'Ab5'), (36, 12, 'D6'), (48, 6, 'Bb5'),
        (54, 6, 'Bb5'), (60, 6, 'Bb5'), (66, 6, 'Ab5'), (72, 12, 'Bb5'), (84, 12, 'Db6')],
    10: [(0, 6, 'G5'), (6, 6, 'G5'), (12, 6, 'G5'), (18, 6, 'F5'), (24, 12, 'Eb5'), (36, 12, 'C5'), (48, 6, 'F5'),
         (54, 6, 'F5'), (60, 6, 'F5'), (66, 6, 'D5'), (72, 12, 'F5'), (84, 12, 'A5')],
    11: [(0, 24, 'Bb5'), (24, 12, 'Bb5'), (36, 12, 'Ab5'), (48, 24, 'C6'), (72, 6, 'Bb5'), (78, 6, 'Bb5'),
         (84, 12, 'Bb4')],
    12: CALL,
    17: CALL,
    18: [(0, 24, 'Eb5'), (24, 12, 'Eb5'), (36, 12, 'C5'), (48, 24, 'C5'), (72, 12, 'C5'), (84, 12, 'Eb5')],
}
LOW = {
    4: [(0, 12, 'Eb3'), (12, 12, 'F3'), (24, 12, 'G3'), (48, 12, 'F3'), (60, 12, 'Eb3'), (72, 24, 'D3')],
    5: [(0, 12, 'D3'), (12, 12, 'C3'), (24, 24, 'Bb2'), (48, 12, 'C3'), (60, 12, 'D3'), (72, 12, 'F3'), (84, 12, 'Ab3')],
}
# 0x03's C: the saw's melody of repeated sixteenths, four struck and a leap
TUNE = [(0, 6, 'D5'), (6, 6, 'D5'), (12, 6, 'D5'), (18, 6, 'D5'), (24, 6, 'F5'), (30, 6, 'F5'), (36, 6, 'F5'),
        (42, 6, 'D5'), (48, 6, 'Eb5'), (54, 6, 'Eb5'), (60, 6, 'Eb5'), (66, 6, 'Eb5'), (72, 6, 'G5'), (78, 6, 'G5'),
        (84, 12, 'F5')]
MELODY = {
    14: TUNE,
    15: [(0, 6, 'F5'), (6, 6, 'F5'), (12, 6, 'F5'), (18, 6, 'Eb5'), (24, 12, 'F5'), (36, 6, 'C5'), (42, 6, 'C5'),
         (48, 6, 'C5'), (54, 6, 'C5'), (60, 6, 'Bb4'), (66, 6, 'C5'), (72, 24, 'A4')],
    16: [(0, 6, 'G5'), (6, 6, 'G5'), (12, 6, 'G5'), (18, 6, 'G5'), (24, 6, 'Bb5'), (30, 6, 'Bb5'), (36, 6, 'Bb5'),
         (42, 6, 'G5'), (48, 6, 'A5'), (54, 6, 'A5'), (60, 6, 'A5'), (66, 6, 'A5'), (72, 6, 'F5'), (78, 6, 'F5'),
         (84, 12, 'D5')],
}


def nxt(bar):
    return CHORDS.get(bar + 1, (None,))[0]


def band(s, bar, double=False, clap=False, crash=False, toms=False, stab_vel=96, chime=None):
    """the band under the lead: the blips, the bass, the chord struck, the
    counter-part in its section's voice (from the other side each bar),
    the kit (cut short for the toms where they fall)"""
    names = CHORDS[bar]
    L.blips(s, bar, names, double=double)
    L.bass(s, bar, names, nxt(bar))
    L.stabs(s, bar, names, vel=stab_vel)
    if chime:
        L.chime(s, bar, names, chime, until=13 if toms else 16, side=1 if bar % 2 else -1)
    L.kit(s, bar, crash=crash, clap=clap, until=13 if toms else 16)
    if toms:
        L.toms(s, bar)


def arrangement():
    s = Score()
    # bars 0-1, the cold open: the blips and the chord struck, the hats
    # rising; the band without its lead, the pickup
    L.blips(s, 0, CHORDS[0], vel=-10)
    L.stabs(s, 0, CHORDS[0], vel=84)
    for p in range(16):
        s.note('noise', 0, 6 * p, 3, 0, 24 + 4 * p, **L.V.HAT)
    band(s, 1, crash=True)
    L.lead(s, 1, [(72, 6, 'F4'), (78, 6, 'G4'), (84, 12, 'A4')], vel=100)
    # bars 2-7: the A (the title, the new Options)
    for bar in range(2, 8):
        band(s, bar, crash=bar in (2, 6), toms=bar == 7, chime='celesta')
        L.lead(s, bar, LINES[bar])
        if bar in LOW:
            L.answer(s, bar, LOW[bar])
    # bars 8-12: the B (the GBA's screen), the call back at its climax
    for bar in range(8, 12):
        band(s, bar, double=True, clap=True, crash=bar == 8, toms=bar == 11, stab_vel=100, chime='vibes')
        L.lead(s, bar, LINES[bar])
    band(s, 12, crash=True, toms=True, stab_vel=100, chime='vibes')
    L.lead(s, 12, CALL, vel=118)
    for t, n, k in CALL:
        s.hit('double', 12, t, max(4, n - 1), L.key(k) - 12, 84, 0.25)
    # bar 13, the drop: the blips and the electric piano alone, the
    # melody's pickup
    L.blips(s, 13, CHORDS[13], vel=-6)
    L.stabs(s, 13, CHORDS[13], vel=90)
    L.melody(s, 13, [(72, 6, 'Bb4'), (78, 6, 'C5'), (84, 12, 'C5')], vel=92)
    for p in range(8, 16):
        s.note('noise', 13, 6 * p, 3, 0, 30 + 6 * (p - 8), **L.V.HAT)
    # bars 14-16, 0x03's C: the saw's melody, the strings held, the
    # square resting
    for bar in range(14, 17):
        names = CHORDS[bar]
        L.blips(s, bar, names, vel=-8)
        L.bass(s, bar, names, nxt(bar))
        L.stabs(s, bar, names, vel=86, places=(3, 6, 10))
        L.strings(s, bar, names)
        L.melody(s, bar, MELODY[bar])
        L.chime(s, bar, names, 'pizz', vel=72, until=13 if bar == 16 else 16, side=1 if bar % 2 else -1)
        L.kit(s, bar, crash=bar == 14, clap=True, until=13 if bar == 16 else 16)
    L.toms(s, 16)
    # bars 17-18, the recap: the call, then the cadence held
    band(s, 17, crash=True, chime='celesta')
    L.lead(s, 17, CALL, vel=116)
    band(s, 18, toms=True, chime='celesta')
    L.lead(s, 18, LINES[18])
    # bar 19, the stinger: B-flat ringing
    L.ring(s, 19, 'Bb', 'D5')
    # bars 20-22, the end card: the blips alone, softly; the strings' last
    # chord
    for bar in (20, 21):
        L.blips(s, bar, CHORDS[bar], vel=-8)
    L.blips(s, 22, CHORDS[22], vel=-16, until=8)
    s.hit('pad', 22, 0, 150, 'Bb3', 70, 0.35)
    return s
