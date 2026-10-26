"""Sketch C for 0.17.0, "S rank": nine bars for the owner's ear, not a
cut. Original, in the manner of BN5's "Virus Busting" (song 0x15; its
habits in sketch_bust_lib.py); none of its notes, nor its chords in their
order. The battle won: 0x15's major sevenths and its Lydian #IVmaj7 taken
toward the relative major, the stab on the brass section, a hook that
sings over long notes, ending in A-flat major rather than at home.

    tools/trailer_music.py's render_any(OUT, trailers.sketch_bust_c), in
    docker/Dockerfile.music's image

Here, in F minor turning to A-flat at 150:
- Bar 0 the count-in: the stab's bar alone on the brass at one pitch,
  the hats rising, the pickup.
- Bars 1-4 the A: the hook (F struck twice, up to A-flat, B-flat held,
  G struck twice, A-flat, B-flat, E-flat struck twice) over Dbmaj7 Eb7,
  bVI to bVII; its answer falling from C over Cm7 Fm7 (the minor v to i,
  up a fourth); the hook again; the cadence on the Lydian Bmaj7 (a
  chromatic mediant where the ear expects home), then Ebsus4 resolving
  to E-flat on the beat. The backing's 50% voice, resting on the half
  bar's first two sixteenths, an octave up.
- Bars 5-6 the B: one figure climbing in sequence over Bbm7 Eb7 | Abmaj7
  Gm7b5 C, the stab out, the chord struck at each bar's head; the major V
  last, deceived into the hook's Dbmaj7.
- Bar 7 the hook back, the saw an octave under it; bar 8 A-flat major
  with its ninth, ringing.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from trailer_music import Score  # noqa: E402
from trailers import sketch_bust_lib as L  # noqa: E402
from trailers.sketch_bust_lib import (BPM, CHORD_SAMPLES, DS_ENV, DS_GAIN, DS_LEVEL,  # noqa: E402,F401
                                      HARDWARE, MIX, REVERB, SAMPLED_LEADS, SOUNDFONT, TAIL)

BARS = 9
BANNER = (7 * 4 * 60 / BPM, 2.9)


def instruments():
    return L.V.instruments('F2')


CHORDS = {0: ('Ab',), 1: ('Dbmaj7', 'Eb7'), 2: ('Cm7', 'Fm7'), 3: ('Dbmaj7', 'Eb7'), 4: ('Bmaj7', 'Ebsus4', 'Eb'),
          5: ('Bbm7', 'Eb7'), 6: ('Abmaj7', 'Gm7b5', 'C'), 7: ('Dbmaj7', 'Eb7'), 8: ('Ab',)}

# the brass's one major chord, played where it sits inside each chord
# (Db F Ab over Bb is Bbm7's top, over Eb an Eb11)
BRASS = {'Ab': 'Ab3', 'Dbmaj7': 'Db4', 'Eb7': 'Eb4', 'Cm7': 'Eb4', 'Fm7': 'Ab3', 'Bmaj7': 'B3', 'Ebsus4': 'Db4',
         'Eb': 'Eb4'}

HOOK = [(0, 12, 'F5'), (12, 6, 'F5'), (18, 6, 'Ab5'), (24, 24, 'Bb5'), (48, 12, 'G5'), (60, 6, 'G5'), (66, 6, 'Ab5'),
        (72, 12, 'Bb5'), (84, 6, 'Eb5'), (90, 6, 'Eb5')]
LINES = {
    1: HOOK,
    2: [(0, 18, 'C6'), (18, 6, 'Bb5'), (24, 6, 'Bb5'), (30, 6, 'G5'), (36, 12, 'G5'), (48, 24, 'Ab5'), (72, 12, 'F5'),
        (84, 6, 'Eb5'), (90, 6, 'Eb5')],
    3: HOOK,
    4: [(0, 18, 'F#5'), (18, 6, 'F#5'), (24, 24, 'D#5'), (48, 24, 'Ab5'), (72, 18, 'G5'), (90, 6, 'Db5')],
    5: [(0, 18, 'Db5'), (18, 6, 'Db5'), (24, 24, 'F5'), (48, 18, 'Eb5'), (66, 6, 'Eb5'), (72, 24, 'G5')],
    6: [(0, 18, 'Eb5'), (18, 6, 'Eb5'), (24, 24, 'Ab5'), (48, 12, 'G5'), (60, 12, 'G5'), (72, 18, 'E5'), (90, 6, 'E5')],
    7: HOOK,
}


def nxt(bar):
    return CHORDS.get(bar + 1, (None,))[0]


def arrangement():
    s = Score()
    # bar 0: the stab's bar alone on the brass, the hats rising, the pickup
    L.stab(s, 0, CHORDS[0], inst='brass', vel=96, ghosts=False, pan=0.25, pitch=BRASS)
    for p in range(16):
        s.note('noise', 0, 6 * p, 3, 0, 28 + 4 * p, **L.V.HAT)
    s.note('fx', 0, 0, 92, 0, 100, sweep=(1500, 120000), ramp=(1, 8))
    L.lead(s, 0, [(84, 6, 'Eb5'), (90, 6, 'Eb5')], vel=108)
    # bars 1-4: the A
    for bar in range(1, 5):
        names = CHORDS[bar]
        L.backing(s, bar, names, rest=True, register=12)
        L.chords(s, bar, names)
        L.bass(s, bar, names, nxt(bar))
        L.stab(s, bar, names, inst='brass', ghosts=False, until=14 if bar == 4 else 16, pan=0.25, pitch=BRASS)
        L.lead(s, bar, LINES[bar])
        L.kit(s, bar, crash=bar == 1, until=12 if bar == 4 else 16)
    L.toms(s, 4)
    # bars 5-6: the B, the chord at each bar's head
    for bar in (5, 6):
        names = CHORDS[bar]
        L.backing(s, bar, names, rest=True, register=12)
        L.chords(s, bar, names, vel=100)
        L.held(s, bar, names, vel=86)
        L.bass(s, bar, names, nxt(bar))
        L.lead(s, bar, LINES[bar])
        L.kit(s, bar, crash=bar == 5, until=12 if bar == 6 else 16)
    for t in (72, 78, 84):
        s.hit('brass', 6, t, 5, 'C4', 104 - (t - 72), 0.3)
    L.toms(s, 6)
    # bar 7: the hook back, the saw under it
    names = CHORDS[7]
    L.backing(s, 7, names, rest=True, register=12, vel=8)
    L.chords(s, 7, names, vel=100)
    L.bass(s, 7, names, nxt(7))
    L.stab(s, 7, names, inst='brass', vel=112, ghosts=False, pan=0.25, pitch=BRASS)
    L.lead(s, 7, HOOK, vel=120)
    L.double(s, 7, HOOK)
    L.kit(s, 7, crash=True, until=12)
    L.toms(s, 7)
    # bar 8: A-flat major with its ninth, ringing
    L.ring(s, 8, 'Ab', 'Ab5', 'Ab2')
    s.hit('pad', 8, 0, 150, 'Ab4', 92, 0.35)            # Ab Bb Eb: the ninth
    s.hit('brass', 8, 0, 60, 'Ab4', 108, -0.2)
    return s
