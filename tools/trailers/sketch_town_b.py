"""Sketch B for 0.19.0, "Your way": nine bars, not a cut. Original, in the
manner of BN5's ACDC Town (song 0x03; its habits in sketch_town_lib.py),
from its C, where a sampled melody of repeated sixteenths takes the lead
and the square holds back; none of its notes, nor its chords in their
order. Brighter and fuller than A: the strings held under the electric
piano, the clap in every bar.

    tools/trailer_music.py's render_any(OUT, trailers.sketch_town_b), in
    docker/Dockerfile.music's image

Here, in B-flat major at 112.5:
- Bar 0 the count-in: the electric piano's chord on 0x03's places alone,
  the hats rising, the melody's pickup.
- Bars 1-4 the C's manner: the saw lead (the approved palette's sampled
  lead) in repeated sixteenths, four struck and a leap, over Gm7 Cm7 |
  Fsus4 F | Ebmaj7 Dm7 | Cm7 Fsus2 (vi up a fourth to ii, the suspended
  V, IV falling a tone to iii); the square under it in struck pairs of
  eighths, holding back as 0x03's does in its C; the
  strings a chord a half bar; the blips, the bass and the kit as 0x03's,
  the clap on 10 and 11.
- Bars 5-6 the B: the square takes the lead, climbing in sequence over
  the borrowed Abmaj7 Bbsus2 | Ebsus4 Fm7, the backing in 32nds.
- Bar 7 the melody's first bar back over Gm7 Fsus4; bar 8 B-flat
  suspended, ringing.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from trailer_music import Score  # noqa: E402
from trailers import sketch_town_lib as L  # noqa: E402
from trailers.sketch_town_lib import (BPM, CHORD_SAMPLES, DS_ENV, DS_GAIN, DS_LEVEL,  # noqa: E402,F401
                                      HARDWARE, MIX, REVERB, SAMPLED_LEADS, SOUNDFONT, TAIL, instruments)

BARS = 9
BANNER = (1 * 4 * 60 / BPM, 2.9)

CHORDS = {0: ('Bbsus2', 'Fsus2'), 1: ('Gm7', 'Cm7'), 2: ('Fsus4', 'F'), 3: ('Ebmaj7', 'Dm7'), 4: ('Cm7', 'Fsus2'),
          5: ('Abmaj7', 'Bbsus2'), 6: ('Ebsus4', 'Fm7'), 7: ('Gm7', 'Fsus4'), 8: ('Bbsus2',)}

TUNE = [(0, 6, 'D5'), (6, 6, 'D5'), (12, 6, 'D5'), (18, 6, 'D5'), (24, 6, 'F5'), (30, 6, 'F5'), (36, 6, 'F5'),
        (42, 6, 'D5'), (48, 6, 'Eb5'), (54, 6, 'Eb5'), (60, 6, 'Eb5'), (66, 6, 'Eb5'), (72, 6, 'G5'), (78, 6, 'G5'),
        (84, 12, 'F5')]
MELODY = {
    1: TUNE,
    2: [(0, 6, 'F5'), (6, 6, 'F5'), (12, 6, 'F5'), (18, 6, 'Eb5'), (24, 12, 'F5'), (36, 6, 'C5'), (42, 6, 'C5'),
        (48, 6, 'C5'), (54, 6, 'C5'), (60, 6, 'Bb4'), (66, 6, 'C5'), (72, 24, 'A4')],
    3: [(0, 6, 'G5'), (6, 6, 'G5'), (12, 6, 'G5'), (18, 6, 'G5'), (24, 6, 'Bb5'), (30, 6, 'Bb5'), (36, 6, 'Bb5'),
        (42, 6, 'G5'), (48, 6, 'A5'), (54, 6, 'A5'), (60, 6, 'A5'), (66, 6, 'A5'), (72, 6, 'F5'), (78, 6, 'F5'),
        (84, 12, 'D5')],
    4: [(0, 6, 'Eb5'), (6, 6, 'Eb5'), (12, 6, 'Eb5'), (18, 6, 'D5'), (24, 12, 'C5'), (36, 12, 'G5'), (48, 48, 'F5')],
    7: TUNE,
}
UNDER = {
    1: [(0, 12, 'Bb4'), (12, 12, 'Bb4'), (24, 12, 'D5'), (36, 12, 'D5'), (48, 12, 'C5'), (60, 12, 'C5'), (72, 24, 'Bb4')],
    2: [(0, 12, 'C5'), (12, 12, 'C5'), (24, 12, 'C5'), (36, 12, 'Bb4'), (48, 12, 'A4'), (60, 12, 'A4'), (72, 24, 'C5')],
    3: [(0, 12, 'D5'), (12, 12, 'D5'), (24, 12, 'Bb4'), (36, 12, 'Bb4'), (48, 12, 'A4'), (60, 12, 'A4'), (72, 24, 'F4')],
    4: [(0, 12, 'G4'), (12, 12, 'G4'), (24, 24, 'G4'), (48, 12, 'A4'), (60, 12, 'A4'), (72, 24, 'A4')],
    7: [(0, 12, 'Bb4'), (12, 12, 'Bb4'), (24, 12, 'D5'), (36, 12, 'D5'), (48, 48, 'C5')],
}
LINES = {
    5: [(0, 6, 'C5'), (6, 6, 'C5'), (12, 6, 'C5'), (18, 6, 'Bb4'), (24, 12, 'C5'), (36, 12, 'Eb5'), (48, 6, 'F5'),
        (54, 6, 'F5'), (60, 6, 'F5'), (66, 6, 'Eb5'), (72, 24, 'F5')],
    6: [(0, 6, 'Eb5'), (6, 6, 'Eb5'), (12, 6, 'Eb5'), (18, 6, 'D5'), (24, 12, 'Eb5'), (36, 12, 'Ab5'), (48, 6, 'Ab5'),
        (54, 6, 'Ab5'), (60, 6, 'G5'), (66, 6, 'Ab5'), (72, 12, 'C6'), (84, 12, 'C6')],
}


def nxt(bar):
    return CHORDS.get(bar + 1, (None,))[0]


def arrangement():
    s = Score()
    # bar 0: the electric piano alone, the hats rising, the pickup
    L.stabs(s, 0, CHORDS[0], vel=88)
    for p in range(16):
        s.note('noise', 0, 6 * p, 3, 0, 24 + 4 * p, **L.V.HAT)
    L.melody(s, 0, [(72, 6, 'Bb4'), (78, 6, 'C5'), (84, 12, 'C5')], vel=92)
    # bars 1-4: the melody over the square's long notes
    for bar in range(1, 5):
        names = CHORDS[bar]
        L.blips(s, bar, names, vel=-8)
        L.bass(s, bar, names, nxt(bar))
        L.stabs(s, bar, names, vel=86, places=(3, 6, 10))
        L.strings(s, bar, names)
        L.melody(s, bar, MELODY[bar])
        L.lead(s, bar, UNDER[bar], vel=96)
        L.kit(s, bar, crash=bar == 1, clap=True, until=13 if bar == 4 else 16)
    L.toms(s, 4)
    # bars 5-6: the B, the square leading, the backing in 32nds
    for bar in (5, 6):
        names = CHORDS[bar]
        L.blips(s, bar, names, double=True)
        L.bass(s, bar, names, nxt(bar))
        L.stabs(s, bar, names, vel=96)
        L.strings(s, bar, names)
        L.lead(s, bar, LINES[bar])
        L.kit(s, bar, crash=bar == 5, clap=True, until=13 if bar == 6 else 16)
    L.toms(s, 6)
    # bar 7: the melody's first bar back
    names = CHORDS[7]
    L.blips(s, 7, names)
    L.bass(s, 7, names, nxt(7))
    L.stabs(s, 7, names, vel=96, places=(3, 6, 10))
    L.strings(s, 7, names)
    L.melody(s, 7, TUNE, vel=108)
    L.lead(s, 7, UNDER[7], vel=100)
    L.kit(s, 7, crash=True, clap=True, until=13)
    L.toms(s, 7)
    # bar 8: B-flat suspended, ringing
    L.ring(s, 8, 'Bbsus2', 'F5')
    L.melody(s, 8, [(0, 60, 'D5')], vel=96)
    return s
