"""Sketch A for 0.19.0, "Hometown": nine bars, not a cut. Original, in the
manner of BN5's ACDC Town (song 0x03; its habits in sketch_town_lib.py),
the owner's reference for its happy, upbeat rhythm; none of its notes,
nor its chords in their order. The nearest of the sketches to the song:
its bounce, its call and answer, its chords struck on the off-beats.

    tools/trailer_music.py's render_any(OUT, trailers.sketch_town_a), in
    docker/Dockerfile.music's image

Here, in B-flat major at 112.5:
- Bar 0 the count-in: the backing's blips alone, thrown left and right,
  the hats rising, the lead's pickup.
- Bars 1-4 the A, 0x03's four-bar breath: the call (B-flat struck three
  times, up through C to E-flat struck again, F held, a turn on G) over Ebsus4 Fsus2, IV to
  the suspended V; the busy answer of repeated sixteenths over Dm7 Gm7
  (iii up a fourth to vi); the call's rhythm again lower over Cm7 Fsus4;
  a held bar over Bbsus2 Bb7, the Mixolydian I7 leading on. The electric
  piano's chord on 0x03's places (the beat, its fourth and third
  sixteenths), the bass's sixteenths high with its octave on the kick,
  the kit as 0x03's; the brass's low answer by step in bars 3 and 4.
- Bars 5-6 the B: the backing in 32nds, the low answer out, the clap on
  10 and 11; the lead climbing in sequence over Ebmaj7 Fm7 (the
  Mixolydian v) | Bb7 Gb7 (the bVI7 fake-out).
- Bar 7 the call back over Ebmaj7 Fsus4; bar 8 B-flat ringing.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from trailer_music import Score  # noqa: E402
from trailers import sketch_town_lib as L  # noqa: E402
from trailers.sketch_town_lib import (BPM, CHORD_SAMPLES, DS_ENV, DS_GAIN, DS_LEVEL,  # noqa: E402,F401
                                      HARDWARE, MIX, REVERB, SAMPLED_LEADS, SAMPLED_PARTS, SOUNDFONT, TAIL,
                                      instruments)

BARS = 9
BANNER = (1 * 4 * 60 / BPM, 2.9)

CHORDS = {0: ('Bbsus2', 'Fsus2'), 1: ('Ebsus4', 'Fsus2'), 2: ('Dm7', 'Gm7'), 3: ('Cm7', 'Fsus4'), 4: ('Bbsus2', 'Bb7'),
          5: ('Ebmaj7', 'Fm7'), 6: ('Bb7', 'Gb7'), 7: ('Ebmaj7', 'Fsus4'), 8: ('Bb',)}

CALL = [(0, 6, 'Bb4'), (6, 6, 'Bb4'), (12, 6, 'Bb4'), (18, 6, 'C5'), (24, 12, 'Eb5'), (36, 6, 'Eb5'), (42, 6, 'Eb5'),
        (48, 18, 'F5'), (66, 6, 'F5'), (72, 6, 'G5'), (78, 18, 'F5')]
LINES = {
    1: CALL,
    2: [(0, 6, 'D5'), (6, 6, 'D5'), (12, 6, 'D5'), (18, 6, 'C5'), (24, 6, 'D5'), (30, 6, 'F5'), (36, 6, 'F5'),
        (42, 6, 'D5'), (48, 6, 'D5'), (54, 6, 'D5'), (60, 6, 'C5'), (66, 6, 'D5'), (72, 12, 'G5'), (84, 12, 'F5')],
    3: [(0, 6, 'G4'), (6, 6, 'G4'), (12, 6, 'G4'), (18, 6, 'Bb4'), (24, 12, 'C5'), (36, 6, 'C5'), (42, 6, 'C5'),
        (48, 18, 'Bb4'), (66, 6, 'Bb4'), (72, 6, 'C5'), (78, 6, 'C5'), (84, 12, 'Eb5')],
    4: [(0, 24, 'D5'), (24, 12, 'D5'), (36, 12, 'D5'), (48, 24, 'C5'), (72, 12, 'F5'), (84, 12, 'F5')],
    5: [(0, 6, 'G5'), (6, 6, 'G5'), (12, 6, 'G5'), (18, 6, 'F5'), (24, 12, 'G5'), (36, 12, 'G5'), (48, 6, 'Ab5'),
        (54, 6, 'Ab5'), (60, 6, 'Ab5'), (66, 6, 'G5'), (72, 12, 'Ab5'), (84, 12, 'C6')],
    6: [(0, 6, 'Ab5'), (6, 6, 'Ab5'), (12, 6, 'Ab5'), (18, 6, 'G5'), (24, 12, 'Ab5'), (36, 12, 'D6'), (48, 6, 'Bb5'),
        (54, 6, 'Bb5'), (60, 6, 'Bb5'), (66, 6, 'Ab5'), (72, 12, 'Bb5'), (84, 12, 'Db6')],
    7: CALL,
}
ANSWER = {
    3: [(0, 12, 'Eb3'), (12, 12, 'F3'), (24, 12, 'G3'), (48, 12, 'F3'), (60, 12, 'Eb3'), (72, 24, 'D3')],
    4: [(0, 12, 'D3'), (12, 12, 'C3'), (24, 24, 'Bb2'), (48, 12, 'C3'), (60, 12, 'D3'), (72, 12, 'F3'), (84, 12, 'Ab3')],
}


def nxt(bar):
    return CHORDS.get(bar + 1, (None,))[0]


def arrangement():
    s = Score()
    # bar 0: the blips alone, the hats rising, the pickup
    L.blips(s, 0, CHORDS[0], vel=-10)
    for p in range(16):
        s.note('noise', 0, 6 * p, 3, 0, 24 + 4 * p, **L.V.HAT)
    L.lead(s, 0, [(72, 6, 'F4'), (78, 6, 'G4'), (84, 12, 'A4')], vel=100)
    # bars 1-4: the A
    for bar in range(1, 5):
        names = CHORDS[bar]
        L.blips(s, bar, names)
        L.bass(s, bar, names, nxt(bar))
        L.stabs(s, bar, names)
        L.lead(s, bar, LINES[bar])
        L.kit(s, bar, crash=bar == 1, until=13 if bar == 4 else 16)
        if bar in ANSWER:
            L.answer(s, bar, ANSWER[bar])
    L.toms(s, 4)
    # bars 5-6: the B, the backing in 32nds
    for bar in (5, 6):
        names = CHORDS[bar]
        L.blips(s, bar, names, double=True)
        L.bass(s, bar, names, nxt(bar))
        L.stabs(s, bar, names, vel=100)
        L.lead(s, bar, LINES[bar])
        L.kit(s, bar, crash=bar == 5, clap=True, until=13 if bar == 6 else 16)
    L.toms(s, 6)
    # bar 7: the call back
    names = CHORDS[7]
    L.blips(s, 7, names)
    L.bass(s, 7, names, nxt(7))
    L.stabs(s, 7, names, vel=100)
    L.lead(s, 7, CALL, vel=118)
    L.kit(s, 7, crash=True, until=13)
    L.toms(s, 7)
    # bar 8: B-flat ringing
    L.ring(s, 8, 'Bb', 'D5')
    return s
