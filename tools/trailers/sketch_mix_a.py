"""Sketch A for 0.21.0, "One Run": ten bars at 144, E minor, not a cut.
Original, in the manner of BN6's title theme (0x01; its habits in
sketch_mix_lib.py); none of its notes, nor its chords in their order.

- Bar 0: the 32nd pairs over Esus2, the strings, the march's snare
  rising.
- Bars 1-4, the A: the hook (B struck twice, a scoop up a fourth to E,
  on by step to G) over Em9 Cmaj7, its answer over Dsus2 Bm7, the hook
  again climbing to B over Em7 Gmaj7, the cadence over Am7 Bsus4 Bm; the
  brass on the guide tones, the bass in sixteenths jumping octaves, the
  march in the kit.
- Bars 5-7, the B: long notes climbing in sequence over Cmaj7 D | Fmaj7 G
  (the borrowed bII's fake-out) | Am7 Bm7, the violin answering, the bass
  walking, both arpeggios; toms.
- Bar 8: the hook lifted a tone, to F# minor, the bell doubling it.
- Bar 9: F# minor ringing.
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

CHORDS = {0: ('Esus2',), 1: ('Em9', 'Cmaj7'), 2: ('Dsus2', 'Bm7'), 3: ('Em7', 'Gmaj7'), 4: ('Am7', 'Bsus4', 'Bm'),
          5: ('Cmaj7', 'D'), 6: ('Fmaj7', 'G'), 7: ('Am7', 'Bm7'), 8: ('F#m9', 'Dmaj7'), 9: ('F#m9',)}

HOOK = [(0, 6, 'B4'), (6, 6, 'B4'), (12, 24, 'E5', 2), (36, 12, 'F#5'), (48, 24, 'G5'), (72, 12, 'E5'), (84, 12, 'D5')]
ANSWER = [(0, 12, 'E5'), (12, 12, 'F#5'), (24, 24, 'A5', 2), (48, 12, 'F#5'), (60, 12, 'E5'), (72, 24, 'D5', -2)]
AGAIN = HOOK[:4] + [(48, 12, 'G5'), (60, 12, 'A5'), (72, 24, 'B5', -1)]
CADENCE = [(0, 12, 'A5'), (12, 12, 'C6'), (24, 24, 'B5'), (48, 12, 'A5'), (60, 12, 'E5'), (72, 24, 'F#5', 2)]
CLIMB = {
    5: [(0, 36, 'G5'), (36, 12, 'E5'), (48, 24, 'F#5'), (72, 24, 'A5')],
    6: [(0, 36, 'A5'), (36, 12, 'F5'), (48, 24, 'G5'), (72, 24, 'B5')],
    7: [(0, 12, 'C6'), (12, 12, 'B5'), (24, 24, 'E6', 2), (48, 6, 'D6'), (54, 6, 'D6'), (60, 12, 'B5'), (72, 24, 'F#6', -1)],
}
BRASS = L.guide({b: CHORDS[b] for b in (1, 2, 3, 4)}, 'G4')
VIOLIN = L.guide({b: CHORDS[b] for b in (5, 6, 7)}, 'B5', rhythm=((0, 36), (36, 12), (48, 24), (72, 24)))


# what the piece grown from this sketch keeps (v0_21_0_music.py): the A's
# and the B's parts, the lead's key in a plan, the A's counter-voice, the
# drop's thin band, the last chord's top note
STYLE = dict(A=dict(arp='pairs', bass='drive', kit2='march'), B=dict(arp='both', bass='walk', stabs=True, kit2='march'),
             lead='lead', counter='horn', drop=dict(arp='pairs', kit='half'), intro=dict(arp='pairs', pad_vel=64),
             violin='B5', top='B5', offset=84, source=('bn6', '0x01'))

def plan():
    A = dict(arp='pairs', bass='drive', kit2='march')
    p = {0: dict(arp='pairs', pad_vel=64)}
    for b, line in zip((1, 2, 3, 4), (HOOK, ANSWER, AGAIN, CADENCE)):
        p[b] = dict(A, lead=line, counter=('horn', BRASS[b]), crash=b in (1, 3), hit=b == 1,
                    fill='snare' if b == 4 else None, side=-1 if b % 2 else 1)
    for b in (5, 6, 7):
        p[b] = dict(arp='both', bass='walk', stabs=True, kit2='march', lead=CLIMB[b], counter=('vln', VIOLIN[b]),
                    crash=b == 5, hit=b == 5, fill='toms' if b == 7 else None, side=1)
    p[8] = dict(A, arp='both', lead=L.up(HOOK, 2), bell=True, crash=True, hit=True, lead_vel=120)
    p[9] = dict(ring=('F#m9', 'C#6'))
    return p


def arrangement():
    s = Score()
    for q in range(8, 16):
        s.hit('snare', 0, 6 * q, 6, vel=40 + 8 * (q - 8), pan=0.05)
    s.hit('sfbass', 0, 72, 6, 'B2', 110)
    s.hit('sfbass', 0, 84, 6, 'D3', 110)
    M.play(s, CHORDS, plan())
    return s
