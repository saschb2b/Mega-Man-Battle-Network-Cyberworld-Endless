"""Sketch A for 0.17.0, "Busting": nine bars for the owner's ear, not a
cut. Original, in the manner of BN5's "Virus Busting" (song 0x15; its
habits in sketch_bust_lib.py), the song every layer of the run now plays
at least once; none of its notes, nor its chords in their order. The
nearest of the three to the song: its own drive, its stab, its kit.

    tools/trailer_music.py's render_any(OUT, trailers.sketch_bust_a), in
    docker/Dockerfile.music's image

Here, in F minor at 150:
- Bar 0 the count-in: the stab's bar alone (3+3+2 twice on the
  distortion guitar at one pitch, the palm-muted ghosts before the beat),
  the hats rising, the hook's pickup.
- Bars 1-4 the A: the hook (F held, F, E-flat struck twice, D-flat, a C
  held, a pickup back up) over Bbm7 Eb6, IV to bVII as 0x15 moves; its
  answer over Abmaj7 Dbmaj7 in another rhythm; the hook again with the
  same chord sample over G, now Gm7b5 (0x15's IIm7b5); the hook's head a
  semitone up over the Lydian Bmaj7, then held over Csus2. The strings a
  chord a half bar, the backing's sixteenths thrown left and right by the
  beat, the bass's sixteenths, the kit as 0x15's.
- Bars 5-6 the B: one figure climbing in sequence over Dbmaj7 Ebsus2 |
  Fm7 C, the major V once, its E held into the hook; the stab gives way
  to the chord held at the bar's head as 0x15's B does.
- Bar 7 the hook back over Dbmaj7 Eb6, the saw an octave under it; bar 8
  F minor with its ninth, ringing.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from trailer_music import Score  # noqa: E402
from trailers import sketch_bust_lib as L  # noqa: E402
from trailers.sketch_bust_lib import (BPM, CHORD_SAMPLES, DS_ENV, DS_GAIN, DS_LEVEL,  # noqa: E402,F401
                                      HARDWARE, MIX, REVERB, SAMPLED_LEADS, SOUNDFONT, TAIL)

BARS = 9
BANNER = (1 * 4 * 60 / BPM, 2.9)


def instruments():
    return L.V.instruments('F2')


CHORDS = {0: ('Fm9',), 1: ('Bbm7', 'Eb6'), 2: ('Abmaj7', 'Dbmaj7'), 3: ('Gm7b5', 'Eb6'), 4: ('Bmaj7', 'Csus2'),
          5: ('Dbmaj7', 'Ebsus2'), 6: ('Fm7', 'C'), 7: ('Dbmaj7', 'Eb6'), 8: ('Fm9',)}

HOOK = [(0, 18, 'F5'), (18, 6, 'F5'), (24, 6, 'Eb5'), (30, 6, 'Eb5'), (36, 12, 'Db5'), (48, 30, 'C5'),
        (78, 6, 'C5'), (84, 6, 'Bb4'), (90, 6, 'C5')]
LINES = {
    1: HOOK,
    2: [(0, 12, 'Eb5'), (12, 6, 'Eb5'), (18, 6, 'F5'), (24, 24, 'Ab5'), (48, 12, 'F5'), (60, 6, 'F5'), (66, 6, 'Eb5'),
        (72, 18, 'Db5'), (90, 6, 'C5')],
    3: HOOK,
    4: [(0, 18, 'F#5'), (18, 6, 'F#5'), (24, 6, 'D#5'), (30, 6, 'D#5'), (36, 12, 'C#5'), (48, 36, 'G5'),
        (84, 6, 'G5'), (90, 6, 'G5')],
    5: [(0, 12, 'Ab5'), (12, 6, 'Ab5'), (18, 6, 'G5'), (24, 24, 'F5'), (48, 12, 'Bb5'), (60, 6, 'Bb5'), (66, 6, 'Ab5'),
        (72, 24, 'G5')],
    6: [(0, 12, 'C6'), (12, 6, 'C6'), (18, 6, 'Bb5'), (24, 24, 'Ab5'), (48, 6, 'G5'), (54, 6, 'G5'), (60, 6, 'F5'),
        (66, 30, 'E5')],
    7: HOOK,
}


def nxt(bar):
    return CHORDS.get(bar + 1, (None,))[0]


def arrangement():
    s = Score()
    # bar 0: the stab's bar alone, the hats rising, the pickup
    L.stab(s, 0, CHORDS[0], vel=96)
    for p in range(16):
        s.note('noise', 0, 6 * p, 3, 0, 28 + 4 * p, **L.V.HAT)
    s.note('fx', 0, 0, 92, 0, 100, sweep=(1500, 120000), ramp=(1, 8))
    L.lead(s, 0, [(78, 6, 'C5'), (84, 6, 'Bb4'), (90, 6, 'C5')], vel=108)
    # bars 1-4: the A
    for bar in range(1, 5):
        names = CHORDS[bar]
        L.backing(s, bar, names)
        L.chords(s, bar, names)
        L.bass(s, bar, names, nxt(bar))
        L.stab(s, bar, names, until=14 if bar == 4 else 16)
        L.lead(s, bar, LINES[bar])
        L.kit(s, bar, crash=bar == 1, until=12 if bar == 4 else 16)
    L.toms(s, 4)
    # bars 5-6: the B, the chord held from the bar's head
    for bar in (5, 6):
        names = CHORDS[bar]
        L.backing(s, bar, names, rest=True)
        L.chords(s, bar, names, vel=100)
        L.held(s, bar, names, vel=84, up=0)
        L.bass(s, bar, names, nxt(bar))
        L.lead(s, bar, LINES[bar])
        L.kit(s, bar, crash=bar == 5, until=12 if bar == 6 else 16)
    L.power(s, 5, 0, 30, 'Db3', vel=112)
    L.power(s, 6, 48, 30, 'C3', vel=112, fall=True)
    L.toms(s, 6)
    # bar 7: the hook back, the saw under it
    names = CHORDS[7]
    L.backing(s, 7, names, vel=8)
    L.chords(s, 7, names, vel=100)
    L.bass(s, 7, names, nxt(7))
    L.stab(s, 7, names, vel=110)
    L.lead(s, 7, HOOK, vel=120)
    L.double(s, 7, HOOK)
    L.kit(s, 7, crash=True, until=12)
    L.toms(s, 7)
    # bar 8: F minor with its ninth, ringing
    L.ring(s, 8, 'Fm9', 'C5', 'F2')
    s.hit('pad', 8, 0, 150, 'F4', 92, 0.35)            # F G C: the ninth
    return s
