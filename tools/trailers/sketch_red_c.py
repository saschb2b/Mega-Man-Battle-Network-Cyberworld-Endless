"""Sketch C for 0.20.0's second round, "Final Round": ten bars at 160, E
minor, not a cut. Original, in the manner of BN4's boss battle (0x15)
and BN6's boss battle (0x16) (their habits in sketch_red_lib.py); none
of their notes, nor their chords in their order. The dark side: the
final's Navi.

- Bar 0: the 32nd pairs alone over Em7, the hats rising, the bass's
  pickup.
- Bars 1-4, the A: a hammering hook of sixteenths (E struck twice, the
  leap to B, D, a scoop into B) over Em7 Cmaj7, answered over Am7 and
  the half-diminished F#m7b5, again, the cadence over Am7 Bsus4 and the
  augmented V (BN4's Vaug); the vibraphone's counter on the guide tones,
  the bass in sixteenths jumping octaves, the guitar's 3-3-2 diving at
  the cadence, the syncopated kick.
- Bars 5-7, the B: the lead climbing over Cmaj7 D | Bm7 Em7 | the bII's
  Fmaj7 to Baug, the brass answering, the bass walking with falls, both
  arpeggios; toms.
- Bar 8: a break, the drums alone but the bass's first beat and the hit.
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

BPM = 160
BARS = 10
BANNER = (1 * 4 * 60 / BPM, 2.9)

CHORDS = {0: ('Em7',), 1: ('Em7', 'Cmaj7'), 2: ('Am7', 'F#m7b5'), 3: ('Em7', 'Cmaj7'), 4: ('Am7', 'Bsus4', 'Baug'),
          5: ('Cmaj7', 'D'), 6: ('Bm7', 'Em7'), 7: ('Fmaj7', 'Baug'), 8: ('Em7',), 9: ('Em9',)}

HOOK = [(0, 6, 'E5'), (6, 6, 'E5'), (12, 6, 'B5'), (18, 6, 'E5'), (24, 6, 'D6'), (30, 6, 'B5'), (36, 12, 'A5'),
        (48, 6, 'G5'), (54, 6, 'G5'), (60, 6, 'E5'), (66, 6, 'G5'), (72, 12, 'B5', 2), (84, 12, 'G5')]
ANSWER = [(0, 12, 'C6'), (12, 6, 'B5'), (18, 6, 'A5'), (24, 12, 'E5'), (36, 12, 'G5'), (48, 6, 'A5'), (54, 6, 'A5'),
          (60, 6, 'C6'), (66, 6, 'A5'), (72, 24, 'F#5', -1)]
AGAIN = HOOK[:7] + [(48, 12, 'B5'), (60, 6, 'C6'), (66, 6, 'B5'), (72, 24, 'E6', -2)]
CADENCE = [(0, 18, 'C6'), (18, 6, 'B5'), (24, 12, 'A5'), (36, 12, 'E5'), (48, 12, 'E5'), (60, 12, 'F#5'),
           (72, 24, 'D#6', 2)]
CLIMB = {
    5: [(0, 24, 'G5'), (24, 6, 'E5'), (30, 6, 'G5'), (36, 12, 'B5'), (48, 24, 'A5'), (72, 12, 'F#5'), (84, 12, 'D5')],
    6: [(0, 24, 'D6'), (24, 6, 'B5'), (30, 6, 'D6'), (36, 12, 'F#6'), (48, 24, 'G6', -1), (72, 12, 'E6'),
        (84, 12, 'B5')],
    7: [(0, 12, 'A5'), (12, 12, 'C6'), (24, 12, 'E6'), (36, 12, 'F6'), (48, 12, 'D#6'), (60, 12, 'B5'),
        (72, 24, 'G5', 2)],
}
VIBES = L.guide({b: CHORDS[b] for b in (1, 2, 3, 4)}, 'G4', rhythm=((6, 18), (30, 12), (54, 18), (78, 12)))
BRASS = L.guide({b: CHORDS[b] for b in (5, 6, 7)}, 'B4')


def plan():
    A = dict(arp='pairs', bass='drive', power=True, kit='battle')
    p = {0: dict(arp='pairs', pad_vel=60, bass=None)}
    for b, line in zip((1, 2, 3, 4), (HOOK, ANSWER, AGAIN, CADENCE)):
        p[b] = dict(A, lead=line, counter=('vibes', VIBES[b]), crash=b in (1, 3), hit=b == 1,
                    fill='toms' if b == 4 else None, side=-1 if b % 2 else 1)
    for b in (5, 6, 7):
        p[b] = dict(arp='both', bass='walk', stabs=True, kit='battle', lead=CLIMB[b], counter=('horn', BRASS[b]),
                    crash=b == 5, hit=b == 5, fill='toms' if b == 7 else None, side=1)
    p[8] = dict(brk=True)
    p[9] = dict(ring=('Em9', 'B5'))
    return p


def arrangement():
    s = Score()
    for q in range(16):
        s.note('noise', 0, 6 * q, 3, 0, 24 + 4 * q, **L.V.HAT)
    s.hit('sfbass', 0, 72, 6, 'B2', 110)
    s.hit('sfbass', 0, 84, 6, 'D3', 110)
    L.play(s, CHORDS, plan())
    return s
