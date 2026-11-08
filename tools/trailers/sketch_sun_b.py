"""Sketch B for 0.20.0, "Final": nine bars, not a cut. Original, in the
manner of BN4 Red Sun's tournament theme (song 0x17; its habits in
sketch_sun_lib.py); none of its notes, nor its chords in their order. The
song's B as its own mood: the lead in six held notes a bar, falling at
their ends, over maj7 and suspended chords that lift where the A only
rocked; the kick and the arpeggio as before, the guitar's fifths out.

- Bar 0: the arpeggio and the strings, the hats alone.
- Bars 1-4: the anthem, a long note a half bar over Fmaj7 Gsus2 | A5 Bb5 |
  Fmaj7 Gsus2 | Dm7 E5.
- Bars 5-7: the anthem an octave's climb, the band whole, the toms into
  the last bar.
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

CHORDS = {0: ('Fmaj7', 'Gsus2'), 1: ('Fmaj7', 'Gsus2'), 2: ('A5', 'Bb5'), 3: ('Fmaj7', 'Gsus2'), 4: ('Dm7', 'E5'),
          5: ('Fmaj7', 'Gsus2'), 6: ('A5', 'C5'), 7: ('Bbmaj7', 'B5'), 8: ('A5',)}

ANTHEM = {
    1: [(0, 30, 'E5', -1), (30, 6, 'E5'), (36, 12, 'G5'), (48, 30, 'A5'), (78, 6, 'A5'), (84, 12, 'G5')],
    2: [(0, 36, 'E5', -2), (36, 6, 'E5'), (42, 6, 'D5'), (48, 30, 'F5'), (78, 6, 'F5'), (84, 12, 'D5')],
    3: [(0, 30, 'C5'), (30, 6, 'C5'), (36, 12, 'E5'), (48, 30, 'A5'), (78, 6, 'A5'), (84, 12, 'B5')],
    4: [(0, 36, 'C6', -1), (36, 12, 'A5'), (48, 36, 'B5', -2), (84, 12, 'E5')],
    5: [(0, 30, 'E6', -1), (30, 6, 'E6'), (36, 12, 'G6'), (48, 30, 'A6'), (78, 6, 'A6'), (84, 12, 'G6')],
    6: [(0, 36, 'E6', -2), (36, 6, 'E6'), (42, 6, 'D6'), (48, 30, 'F6'), (78, 6, 'F6'), (84, 12, 'D6')],
    7: [(0, 30, 'D6'), (30, 6, 'D6'), (36, 12, 'A5'), (48, 36, 'B5', -2), (84, 12, 'E5')],
}


def arrangement():
    s = Score()
    # bar 0: the arpeggio and the strings, the hats alone
    L.arp(s, 0, CHORDS[0], vel=-10)
    L.pad(s, 0, CHORDS[0], vel=70)
    for p in range(16):
        s.note('noise', 0, 6 * p, 3, 0, 24 + 4 * p, **L.V.HAT)
    for bar in range(1, 8):
        names = CHORDS[bar]
        fill = bar in (4, 7)
        until = 12 if fill else 16
        L.arp(s, bar, names, up=12 if bar >= 5 else 0, vel=-6)
        L.bass(s, bar, names, until=until)
        L.pad(s, bar, names, vel=88)
        L.kit(s, bar, crash=bar in (1, 5), until=until, ghosts=bar >= 5)
        if fill:
            L.fill(s, bar)
        L.lead(s, bar, ANTHEM[bar], vel=108, side=-1 if bar < 5 else 1)
    L.ring(s, 8, 'A5', 'A5')
    return s
