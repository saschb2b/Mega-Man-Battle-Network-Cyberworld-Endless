"""Sketch B for 0.20.0's second round, "Tournament Day": ten bars at 144,
F major, not a cut. Original, in the manner of BN4's tournament (0x17)
and its battles (0x14, 0x15) (their habits in sketch_red_lib.py); none
of their notes, nor their chords in their order. The bright side: a
netbattler walking into the arena.

- Bar 0: the electric piano's three-octave sweep over Fadd9, the hats
  rising.
- Bars 1-4, the A: a fanfare's hook (C, F, G, a scoop up to A) over Fmaj7
  Gm7, answered over Am7 Bbmaj7, again over Fmaj7 and the Lydian G7, the
  cadence over the borrowed Bbm7 to Csus4 C; the violin's counter on the
  guide tones, the bass walking, the brass struck on the syncopations,
  the kick on every beat as 0x17's; a snare roll.
- Bars 5-7, the B: the lead held and climbing over Dm7 Bbmaj7 | Gm7
  Ebmaj7 (the borrowed bVII) | Bbmaj7 Csus4, the brass answering, the
  bass in sixteenths jumping octaves, the 32nd pairs over the sweep; toms.
- Bar 8: the hook lifted a semitone, to F-sharp, the bell doubling it.
- Bar 9: F-sharp ringing.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from trailer_music import Score  # noqa: E402
from trailers import sketch_red_lib as L  # noqa: E402
from trailers.sketch_red_lib import (CHORD_SAMPLES, DS_ENV, DS_GAIN, DS_LEVEL,  # noqa: E402,F401
                                     HARDWARE, MIX, REVERB, SAMPLED_LEADS, SAMPLED_PARTS, SOUNDFONT, TAIL,
                                     instruments)

BPM = 144
BARS = 10
BANNER = (1 * 4 * 60 / BPM, 2.9)

CHORDS = {0: ('Fadd9',), 1: ('Fmaj7', 'Gm7'), 2: ('Am7', 'Bbmaj7'), 3: ('Fmaj7', 'G7'), 4: ('Bbm7', 'Csus4', 'C'),
          5: ('Dm7', 'Bbmaj7'), 6: ('Gm7', 'Ebmaj7'), 7: ('Bbmaj7', 'Csus4'), 8: ('F#maj7', 'G#m7'), 9: ('F#add9',)}

HOOK = [(0, 12, 'C5'), (12, 6, 'F5'), (18, 6, 'G5'), (24, 18, 'A5', 2), (42, 6, 'G5'), (48, 12, 'Bb5'),
        (60, 12, 'A5'), (72, 12, 'G5'), (84, 12, 'D5')]
ANSWER = [(0, 18, 'C6'), (18, 6, 'A5'), (24, 12, 'G5'), (36, 12, 'E5'), (48, 24, 'F5'), (72, 12, 'D5'),
          (84, 12, 'F5')]
AGAIN = HOOK[:5] + [(48, 12, 'B5'), (60, 12, 'A5'), (72, 12, 'G5'), (84, 12, 'F5')]
CADENCE = [(0, 24, 'Db6'), (24, 12, 'C6'), (36, 12, 'Bb5'), (48, 12, 'F5'), (60, 12, 'G5'), (72, 24, 'E5', -1)]
CLIMB = {
    5: [(0, 24, 'A5'), (24, 12, 'F5'), (36, 12, 'D5'), (48, 24, 'D6', 2), (72, 12, 'C6'), (84, 12, 'A5')],
    6: [(0, 24, 'Bb5'), (24, 12, 'G5'), (36, 12, 'D5'), (48, 24, 'G5'), (72, 12, 'Bb5'), (84, 12, 'D6')],
    7: [(0, 12, 'D6'), (12, 12, 'C6'), (24, 12, 'Bb5'), (36, 12, 'A5'), (48, 24, 'F5'), (72, 6, 'G5'), (78, 6, 'A5'),
        (84, 12, 'C6', 2)],
}
VIOLIN = L.guide({b: CHORDS[b] for b in (1, 2, 3, 4)}, 'A4', rhythm=((0, 30), (36, 12), (48, 30), (84, 12)))
BRASS = L.guide({b: CHORDS[b] for b in (5, 6, 7)}, 'F4', rhythm=((12, 24), (36, 12), (60, 24), (84, 12)))


def plan():
    p = {0: dict(arp='sweep', pad_vel=64, bass=None)}
    for b, line in zip((1, 2, 3, 4), (HOOK, ANSWER, AGAIN, CADENCE)):
        p[b] = dict(arp='sweep', bass='walk', stabs=True, kit='four', lead=line, counter=('vln', VIOLIN[b]),
                    crash=b in (1, 3), hit=b == 1, fill='snare' if b == 4 else None, side=-1 if b % 2 else 1)
    for b in (5, 6, 7):
        p[b] = dict(arp='both', bass='drive', power=True, kit='four', lead=CLIMB[b], counter=('horn', BRASS[b]),
                    crash=b == 5, hit=b == 5, fill='toms' if b == 7 else None, side=1, thin=True)
    p[8] = dict(arp='both', bass='walk', stabs=True, kit='four', lead=L.up(HOOK, 1), bell=True, crash=True, hit=True,
                lead_vel=120, counter=('vln', L.up(VIOLIN[1], 1)))
    p[9] = dict(ring=('F#add9', 'C#6'))
    return p


def arrangement():
    s = Score()
    for q in range(16):
        s.note('noise', 0, 6 * q, 3, 0, 24 + 4 * q, **L.V.HAT)
    L.play(s, CHORDS, plan())
    return s
