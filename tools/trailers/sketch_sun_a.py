"""Sketch A for 0.20.0, "Bracket": nine bars, not a cut. Original, in the
manner of BN4 Red Sun's tournament theme (song 0x17; its habits in
sketch_sun_lib.py); none of its notes, nor its chords in their order. The
nearest of the sketches to the song's A: the kick on every beat, the bare
fifths rocking a semitone, the busy lead with its echo from the other
side, the arpeggio running right of centre.

- Bar 0: the band without its lead (0x17 waits four bars; a sketch one),
  the kick alone under the arpeggio, the bass and the strings.
- Bars 1-4: the hook (E struck twice, a leap to the octave's A, falling
  back by step), its answer over the B-flat's maj7, the hook again turned
  up a tone onto B5, the cadence held and falling.
- Bars 5-7: 0x17's B, the lead in held notes climbing in sequence over
  Fmaj7 Dsus4 | Bbmaj7 C5 | Dm7 E5, the toms falling into the last bar.
- Bar 8: A5 ringing.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from trailer_music import Score  # noqa: E402
from trailers import sketch_sun_lib as L  # noqa: E402
from trailers.sketch_sun_lib import (BPM, CHORD_SAMPLES, DS_ENV, DS_GAIN, DS_LEVEL,  # noqa: E402,F401
                                     HARDWARE, MIX, REVERB, SAMPLED_LEADS, SAMPLED_PARTS, SOUNDFONT, TAIL,
                                     instruments)

BARS = 9
BANNER = (1 * 4 * 60 / BPM, 2.9)

CHORDS = {0: ('A5', 'Bb5'), 1: ('A5', 'Bb5'), 2: ('A5', 'Bbmaj7'), 3: ('A5', 'B5'), 4: ('Bb5', 'A5'),
          5: ('Fmaj7', 'Dsus4'), 6: ('Bbmaj7', 'C5'), 7: ('Dm7', 'E5'), 8: ('A5',)}

# the hook: E struck twice, the leap, the fall by step; then D struck and
# a long D falling (a fourth item: its fall in semitones)
HOOK = [(0, 6, 'E5'), (6, 6, 'E5'), (12, 6, 'A5'), (18, 6, 'G5'), (24, 12, 'E5'), (36, 6, 'E5'), (42, 6, 'D5'),
        (48, 12, 'D5'), (60, 6, 'D5'), (66, 6, 'C5'), (72, 18, 'D5', -3), (90, 6, 'C5')]
ANSWER = [(0, 12, 'C5'), (12, 6, 'E5'), (18, 6, 'C5'), (24, 6, 'C5'), (30, 6, 'D5'), (36, 12, 'E5'),
          (48, 6, 'F5'), (54, 6, 'F5'), (60, 6, 'A5'), (66, 6, 'A5'), (72, 24, 'F5', -2)]
HOOK_UP = HOOK[:6] + [(42, 6, 'E5'), (48, 12, 'C#5'), (60, 6, 'C#5'), (66, 6, 'E5'), (72, 18, 'F#5', -2), (90, 6, 'E5')]
CADENCE = [(0, 24, 'D5'), (24, 6, 'D5'), (30, 6, 'C5'), (36, 12, 'D5'), (48, 36, 'E5', -2), (84, 12, 'E5')]
CLIMB = {
    5: [(0, 18, 'E5'), (18, 6, 'E5'), (24, 12, 'A5'), (36, 12, 'G5'), (48, 18, 'A5'), (66, 6, 'A5'), (72, 24, 'D5')],
    6: [(0, 18, 'F5'), (18, 6, 'F5'), (24, 12, 'Bb5'), (36, 12, 'A5'), (48, 18, 'G5'), (66, 6, 'G5'), (72, 24, 'G5')],
    7: [(0, 18, 'A5'), (18, 6, 'A5'), (24, 24, 'D6'), (48, 24, 'B5', -2), (72, 12, 'E5')],
}
LINES = {1: HOOK, 2: ANSWER, 3: HOOK_UP, 4: CADENCE, **CLIMB}


def band(s, bar, crash=False, fill=False, power=True):
    names = CHORDS[bar]
    until = 12 if fill else 16
    L.arp(s, bar, names)
    L.bass(s, bar, names, until=until)
    L.pad(s, bar, names)
    if power:
        L.power(s, bar, names)
    L.kit(s, bar, crash=crash, until=until)
    if fill:
        L.fill(s, bar)


def arrangement():
    s = Score()
    # bar 0: the band without its lead, the kick alone
    L.arp(s, 0, CHORDS[0], vel=-10)
    L.bass(s, 0, CHORDS[0], vel=-20)
    L.pad(s, 0, CHORDS[0], vel=70)
    L.kit(s, 0, snare=False)
    # bars 1-4: the hook, answered, again, cadenced
    for bar in range(1, 5):
        band(s, bar, crash=bar == 1)
        L.lead(s, bar, LINES[bar], side=-1 if bar % 2 else 1)
    # bars 5-7: the B, held notes climbing
    for bar in range(5, 8):
        band(s, bar, crash=bar == 5, fill=bar == 7)
        L.lead(s, bar, LINES[bar], side=1)
    L.ring(s, 8, 'A5', 'E5')
    return s
