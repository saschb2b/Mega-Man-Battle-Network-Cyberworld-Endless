"""Sketch C for 0.21.0, "Every Page": ten bars at 144, C major, not a
cut. Original, in the manner of BN4's title theme (0x00; its habits in
sketch_mix_lib.py); none of its notes, nor its chords in their order.

- Bar 0: the one-note eighths over Csus2, the strings, the hats rising.
- Bars 1-4, the A: the hook (E, down a step and back, up to A) over Am7,
  its answer over the borrowed Fm and Csus2, the hook again reaching C
  over Am7 D7, the cadence over Dm7b5 Esus4; the lead on both squares,
  the second eight ticks behind; the bass's low sixteenths on the root,
  the kick in eighths, the snare on the off-beats.
- Bars 5-7, the B: long notes climbing by step over Abmaj7 | Dm7b5 Esus4
  | Am7 D7, the electric piano's arpeggio, the violin answering; toms.
- Bar 8: the hook over Fmaj7 Gsus4, the bell doubling it.
- Bar 9: Csus2 ringing.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from trailer_music import Score  # noqa: E402
from trailers import sketch_mix_lib as M  # noqa: E402
from trailers.sketch_mix_lib import (CHORD_SAMPLES, DS_ENV, DS_GAIN, DS_LEVEL,  # noqa: E402,F401
                                     HARDWARE, MIX, REVERB, SAMPLED_LEADS, SAMPLED_PARTS, SOUNDFONT, TAIL,
                                     instruments)

L = M.L
BPM = 144
BARS = 10
BANNER = (1 * 4 * 60 / BPM, 2.9)

CHORDS = {0: ('Csus2',), 1: ('Am7',), 2: ('Fm', 'Csus2'), 3: ('Am7', 'D7'), 4: ('Dm7b5', 'Esus4'),
          5: ('Abmaj7',), 6: ('Dm7b5', 'Esus4'), 7: ('Am7', 'D7'), 8: ('Fmaj7', 'Gsus4'), 9: ('Csus2',)}

HOOK = [(0, 18, 'E5'), (18, 6, 'D5'), (24, 12, 'E5'), (36, 12, 'G5'), (48, 24, 'A5', 1), (72, 12, 'G5'), (84, 12, 'E5')]
ANSWER = [(0, 12, 'F5'), (12, 12, 'G5'), (24, 24, 'Ab5'), (48, 12, 'G5'), (60, 12, 'F5'), (72, 24, 'D5')]
AGAIN = HOOK[:5] + [(72, 12, 'B5'), (84, 12, 'C6')]
CADENCE = [(0, 24, 'D6'), (24, 12, 'C6'), (36, 12, 'Ab5'), (48, 24, 'A5'), (72, 24, 'B5', 2)]
CLIMB = {
    5: [(0, 36, 'G5'), (36, 12, 'Ab5'), (48, 12, 'Bb5'), (60, 12, 'C6'), (72, 24, 'Eb6', 1)],
    6: [(0, 36, 'D6'), (36, 12, 'C6'), (48, 24, 'B5'), (72, 12, 'A5'), (84, 12, 'B5')],
    7: [(0, 12, 'C6'), (12, 12, 'B5'), (24, 24, 'A5'), (48, 12, 'A5'), (60, 12, 'B5'), (72, 24, 'C6', 2)],
}
VIOLIN = L.guide({b: CHORDS[b] for b in (5, 6, 7)}, 'C5', rhythm=((0, 48), (48, 24), (72, 24)))


def plan():
    A = dict(arp2='blips', bass2='pedal', kit2='anthem')
    p = {0: dict(arp2='blips', pad_vel=64)}
    for b, line in zip((1, 2, 3, 4), (HOOK, ANSWER, AGAIN, CADENCE)):
        p[b] = dict(A, lead2=line, crash=b in (1, 3), hit=b == 1, fill='snare' if b == 4 else None,
                    side=-1 if b % 2 else 1)
    for b in (5, 6, 7):
        p[b] = dict(A, arp='sweep', lead2=CLIMB[b], counter=('vln', VIOLIN[b]), crash=b == 5, hit=b == 5,
                    fill='toms' if b == 7 else None, side=1)
    p[8] = dict(A, arp='sweep', lead2=HOOK, bell=True, crash=True, hit=True, lead_vel=120)
    p[9] = dict(ring=('Csus2', 'G5'))
    return p


def arrangement():
    s = Score()
    for q in range(16):
        s.note('noise', 0, 6 * q, 3, 0, 24 + 4 * q, **L.V.HAT)
    M.play(s, CHORDS, plan())
    return s
