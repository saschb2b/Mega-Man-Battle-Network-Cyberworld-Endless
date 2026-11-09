"""Sketch A for 0.20.0's second round, "Battle Drive": ten bars at 150, D
minor, not a cut. Original, in the manner of BN4's virus battle (0x14)
and BN6's boss battle (0x16) (their habits in sketch_red_lib.py); none of
their notes, nor their chords in their order.

- Bar 0: the 32nd pairs and the electric piano's sweep over Dm9, the
  strings, the hats rising, the bass's pickup.
- Bars 1-4, the A: the hook (D struck twice, a scoop up to A, falling by
  step) over Dm9 Bbmaj7, its answer over Csus4 C, the hook again over
  Dm7 Fmaj7, the cadence over Gm7 Asus4 A; the brass's counter-line on
  the guide tones, the bass in sixteenths jumping octaves, the guitar's
  3-3-2 with chugs between, the syncopated kick; a snare roll into the B.
- Bars 5-7, the B: the lead in sequence over Bbmaj7 C | Ebmaj7 F (the
  borrowed bII's fake-out) | Gm7 Am7, the violin answering, the bass
  walking, the brass struck on the syncopations, both arpeggios; toms.
- Bar 8: the hook lifted a tone, to E minor, the bell doubling it.
- Bar 9: E minor ringing.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from trailer_music import Score  # noqa: E402
from trailers import sketch_red_lib as L  # noqa: E402
from trailers.sketch_red_lib import (CHORD_SAMPLES, DS_ENV, DS_GAIN, DS_LEVEL,  # noqa: E402,F401
                                     HARDWARE, MIX, REVERB, SAMPLED_LEADS, SAMPLED_PARTS, SOUNDFONT, TAIL,
                                     instruments)

BPM = 150
BARS = 10
BANNER = (1 * 4 * 60 / BPM, 2.9)

CHORDS = {0: ('Dm9',), 1: ('Dm9', 'Bbmaj7'), 2: ('Csus4', 'C'), 3: ('Dm7', 'Fmaj7'), 4: ('Gm7', 'Asus4', 'A'),
          5: ('Bbmaj7', 'C'), 6: ('Ebmaj7', 'F'), 7: ('Gm7', 'Am7'), 8: ('Em9', 'Cmaj7'), 9: ('Em9',)}

HOOK = [(0, 6, 'D5'), (6, 6, 'D5'), (12, 12, 'A5', 2), (24, 6, 'G5'), (30, 6, 'F5'), (36, 12, 'E5'),
        (48, 18, 'F5'), (66, 6, 'D5'), (72, 12, 'C5'), (84, 12, 'A5')]
ANSWER = [(0, 12, 'G5'), (12, 6, 'F5'), (18, 6, 'G5'), (24, 12, 'C6', 2), (36, 12, 'G5'), (48, 6, 'E5'),
          (54, 6, 'E5'), (60, 12, 'G5'), (72, 24, 'E5', -2)]
AGAIN = HOOK[:5] + [(36, 12, 'A5'), (48, 12, 'C6'), (60, 6, 'A5'), (66, 6, 'G5'), (72, 24, 'A5', -1)]
CADENCE = [(0, 18, 'Bb5'), (18, 6, 'A5'), (24, 12, 'G5'), (36, 12, 'F5'), (48, 12, 'E5'), (60, 12, 'D5'),
           (72, 24, 'C#5', 2)]
CLIMB = {
    5: [(0, 24, 'D5'), (24, 6, 'C5'), (30, 6, 'D5'), (36, 12, 'F5'), (48, 24, 'E5'), (72, 12, 'G5'), (84, 12, 'E5')],
    6: [(0, 24, 'G5'), (24, 6, 'F5'), (30, 6, 'G5'), (36, 12, 'Bb5'), (48, 24, 'A5'), (72, 12, 'C6'), (84, 12, 'A5')],
    7: [(0, 6, 'Bb5'), (6, 6, 'Bb5'), (12, 6, 'A5'), (18, 6, 'Bb5'), (24, 12, 'D6'), (36, 12, 'Bb5'), (48, 6, 'C6'),
        (54, 6, 'C6'), (60, 6, 'B5'), (66, 6, 'C6'), (72, 24, 'E6', -1)],
}
BRASS = L.guide({b: CHORDS[b] for b in (1, 2, 3, 4)}, 'F4')
VIOLIN = L.guide({b: CHORDS[b] for b in (5, 6, 7)}, 'A5', rhythm=((0, 36), (36, 12), (48, 24), (72, 24)))


def plan():
    A = dict(arp='pairs', bass='drive', power=True, kit='battle', counter=None)
    p = {0: dict(arp='both', pad_vel=64, bass=None)}
    for b, line in zip((1, 2, 3, 4), (HOOK, ANSWER, AGAIN, CADENCE)):
        p[b] = dict(A, lead=line, counter=('horn', BRASS[b]), crash=b in (1, 3), hit=b == 1,
                    fill='snare' if b == 4 else None, side=-1 if b % 2 else 1)
    for b in (5, 6, 7):
        p[b] = dict(arp='both', bass='walk', stabs=True, kit='battle', lead=CLIMB[b], counter=('vln', VIOLIN[b]),
                    crash=b == 5, hit=b == 5, fill='toms' if b == 7 else None, side=1)
    p[8] = dict(A, arp='both', lead=L.up(HOOK, 2), bell=True, crash=True, hit=True, lead_vel=120)
    p[9] = dict(ring=('Em9', 'B5'))
    return p


def arrangement():
    s = Score()
    for q in range(16):
        s.note('noise', 0, 6 * q, 3, 0, 24 + 4 * q, **L.V.HAT)
    s.hit('sfbass', 0, 72, 6, 'A2', 110)
    s.hit('sfbass', 0, 84, 6, 'C3', 110)
    L.play(s, CHORDS, plan())
    return s
