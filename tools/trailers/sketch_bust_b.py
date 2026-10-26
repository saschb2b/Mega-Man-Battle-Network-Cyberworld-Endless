"""Sketch B for 0.17.0, "Break": ten bars for the owner's ear, not a cut.
Original, in the manner of BN5's "Virus Busting" (song 0x15; its habits
in sketch_bust_lib.py); none of its notes, nor its chords in their order.
It opens on what 0x15 does in its break (the drums and the stab alone,
the bass out) and lets the band in on the hook: the trailer's cold open
and its title card, in sound. Darker than A: 0x15's IIm7b5, its power
fifths and a bII where the ear expects home.

    tools/trailer_music.py's render_any(OUT, trailers.sketch_bust_b), in
    docker/Dockerfile.music's image

Here, in F minor at 150:
- Bars 0-1 the break: 0x15's kit and the stab on the distortion guitar,
  nothing else; the toms into bar 2, the hook's pickup.
- Bars 2-5 the A, everything in at once on the crash: the hook (C struck
  twice, a leap to G, F struck twice, E-flat, F held, a pickup of A-flat
  struck twice) over Fm9 Dbmaj7, i to bVI, a fall of a third as 0x15's
  roots fall; its answer over Bbm7 Eb7, up a fourth twice; the hook
  again; the cadence over Gm7b5 and the bare fifth on C, held.
- Bars 6-7 the B: the stab out, the chord struck at each bar's head, the
  lead on long notes over Dbmaj7 Eb7 | Gbmaj7 (the bII, the fake-out)
  C5, the toms into the return.
- Bar 8 the hook back, the saw an octave under it; bar 9 F minor with
  its ninth, ringing.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from trailer_music import Score  # noqa: E402
from trailers import sketch_bust_lib as L  # noqa: E402
from trailers.sketch_bust_lib import (BPM, CHORD_SAMPLES, DS_ENV, DS_GAIN, DS_LEVEL,  # noqa: E402,F401
                                      HARDWARE, MIX, REVERB, SAMPLED_LEADS, SOUNDFONT, TAIL)

BARS = 10
BANNER = (2 * 4 * 60 / BPM, 2.9)


def instruments():
    return L.V.instruments('F2')


CHORDS = {0: ('Fm9',), 1: ('Fm9',), 2: ('Fm9', 'Dbmaj7'), 3: ('Bbm7', 'Eb7'), 4: ('Fm9', 'Dbmaj7'),
          5: ('Gm7b5', 'Csus2'), 6: ('Dbmaj7', 'Eb7'), 7: ('Gbmaj7', 'Csus2'), 8: ('Fm9', 'Dbmaj7'), 9: ('Fm9',)}

HOOK = [(0, 6, 'C5'), (6, 6, 'C5'), (12, 12, 'G5'), (24, 6, 'F5'), (30, 6, 'F5'), (36, 12, 'Eb5'), (48, 30, 'F5'),
        (78, 6, 'Ab5'), (84, 6, 'Ab5'), (90, 6, 'G5')]
LINES = {
    2: HOOK,
    3: [(0, 12, 'Db5'), (12, 6, 'Db5'), (18, 6, 'F5'), (24, 24, 'Bb5'), (48, 12, 'G5'), (60, 6, 'G5'), (66, 6, 'F5'),
        (72, 24, 'Db5')],
    4: HOOK,
    5: [(0, 18, 'Db5'), (18, 6, 'Db5'), (24, 24, 'Bb4'), (48, 36, 'C5'), (84, 6, 'C5'), (90, 6, 'Eb5')],
    6: [(0, 24, 'F5'), (24, 6, 'Eb5'), (30, 6, 'Eb5'), (36, 12, 'Db5'), (48, 24, 'G5'), (72, 6, 'G5'), (78, 6, 'F5'),
        (84, 12, 'Eb5')],
    7: [(0, 36, 'Db5'), (36, 6, 'Db5'), (42, 6, 'F5'), (48, 36, 'G5'), (84, 6, 'Ab4'), (90, 6, 'Bb4')],
    8: HOOK,
}


def nxt(bar):
    return CHORDS.get(bar + 1, (None,))[0]


def arrangement():
    s = Score()
    # bars 0-1: the break, the drums and the stab alone
    for bar in (0, 1):
        L.stab(s, bar, CHORDS[bar], vel=100)
        L.kit(s, bar, crash=bar == 0, until=12 if bar == 1 else 16)
    L.toms(s, 1)
    L.lead(s, 1, [(84, 6, 'Ab4'), (90, 6, 'Bb4')], vel=104)
    # bars 2-5: the A, the band in on the crash
    for bar in range(2, 6):
        names = CHORDS[bar]
        L.backing(s, bar, names)
        L.chords(s, bar, names)
        L.bass(s, bar, names, nxt(bar), fall=bar == 5)
        L.stab(s, bar, names, until=14 if bar == 5 else 16)
        L.lead(s, bar, LINES[bar])
        L.kit(s, bar, crash=bar == 2, until=14 if bar == 5 else 16)
    L.power(s, 2, 0, 30, 'F2', vel=118)
    L.power(s, 5, 48, 40, 'C3', vel=114, fall=True)
    # bars 6-7: the B, the chord at each bar's head
    for bar in (6, 7):
        names = CHORDS[bar]
        L.backing(s, bar, names, rest=True)
        L.chords(s, bar, names, vel=100)
        L.held(s, bar, names, vel=90)
        L.bass(s, bar, names, nxt(bar))
        L.lead(s, bar, LINES[bar])
        L.kit(s, bar, crash=bar == 6, until=12 if bar == 7 else 16)
    L.power(s, 7, 0, 30, 'Gb2', vel=112)
    L.toms(s, 7)
    # bar 8: the hook back, the saw under it
    names = CHORDS[8]
    L.backing(s, 8, names, vel=8)
    L.chords(s, 8, names, vel=100)
    L.bass(s, 8, names, nxt(8))
    L.stab(s, 8, names, vel=110)
    L.lead(s, 8, HOOK, vel=120)
    L.double(s, 8, HOOK)
    L.kit(s, 8, crash=True, until=12)
    L.toms(s, 8)
    # bar 9: F minor with its ninth, ringing
    L.ring(s, 9, 'Fm9', 'F5', 'F2')
    s.hit('pad', 9, 0, 150, 'F4', 92, 0.35)
    return s
