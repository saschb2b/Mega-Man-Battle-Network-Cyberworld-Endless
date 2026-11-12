"""Sketch B for 0.21.0, "Deep Signal": ten bars at 150, C minor, not a
cut. Original, in the manner of BN5's boss battle (0x16; its habits in
sketch_mix_lib.py); none of its notes, nor its chords in their order.

- Bar 0: the guitar's gallop on C alone, the wide arpeggio over Cm9, the
  hats rising.
- Bars 1-4, the A: the hook (G hammered, its upper neighbour, down to D,
  then up to C) over Cm9 Am7b5, its answer over Abmaj7 F7, the hook again
  over Cm7 Em7b5, the cadence over Abmaj7 Gsus4 G; the bass's high
  sixteenths, the gallop, the snare on the off-beats.
- Bars 5-7, the B: the lead in sequence over Am7b5 Abmaj7 | Dbmaj7 Eb
  (the bII's fake-out) | Fm7 Gsus4, the octaves pinging, the violin
  answering; toms.
- Bar 8: the hook lifted a tone, to D minor, the bell doubling it.
- Bar 9: D minor ringing.
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
BPM = 150
BARS = 10
BANNER = (1 * 4 * 60 / BPM, 2.9)

CHORDS = {0: ('Cm9',), 1: ('Cm9', 'Am7b5'), 2: ('Abmaj7', 'F7'), 3: ('Cm7', 'Em7b5'), 4: ('Abmaj7', 'Gsus4', 'G'),
          5: ('Am7b5', 'Abmaj7'), 6: ('Dbmaj7', 'Eb'), 7: ('Fm7', 'Gsus4'), 8: ('Dm9', 'Bm7b5'), 9: ('Dm9',)}

HOOK = [(0, 6, 'G5'), (6, 6, 'G5'), (12, 6, 'Ab5'), (18, 6, 'G5'), (24, 12, 'Eb5'), (36, 12, 'D5'), (48, 6, 'Eb5'),
        (54, 6, 'Eb5'), (60, 12, 'G5', 1), (72, 24, 'C6', -2)]
ANSWER = [(0, 12, 'C6'), (12, 6, 'Bb5'), (18, 6, 'Ab5'), (24, 24, 'G5'), (48, 6, 'A5'), (54, 6, 'A5'), (60, 12, 'C6'),
          (72, 24, 'A5', -2)]
AGAIN = HOOK[:6] + [(48, 6, 'E5'), (54, 6, 'E5'), (60, 12, 'G5'), (72, 24, 'Bb5', -1)]
CADENCE = [(0, 6, 'C6'), (6, 6, 'C6'), (12, 12, 'Eb6'), (24, 24, 'C6'), (48, 12, 'D6'), (60, 12, 'C6'), (72, 24, 'B5', 2)]
CLIMB = {
    5: [(0, 24, 'Eb5'), (24, 12, 'G5'), (36, 12, 'A5'), (48, 24, 'G5'), (72, 12, 'Eb5'), (84, 12, 'C5')],
    6: [(0, 24, 'F5'), (24, 12, 'Ab5'), (36, 12, 'C6'), (48, 24, 'Bb5'), (72, 12, 'G5'), (84, 12, 'Eb5')],
    7: [(0, 6, 'Ab5'), (6, 6, 'Ab5'), (12, 12, 'C6'), (24, 12, 'Eb6'), (36, 12, 'C6'), (48, 6, 'D6'), (54, 6, 'D6'),
        (60, 12, 'C6'), (72, 24, 'D6', -1)],
}
VIOLIN = L.guide({b: CHORDS[b] for b in (5, 6, 7)}, 'G5', rhythm=((0, 36), (36, 12), (48, 24), (72, 24)))


# what the piece grown from this sketch keeps (v0_21_0_music.py): the A's
# and the B's parts, the lead's key in a plan, no counter-voice in the A,
# the drop's thin band, the last chord's top note
STYLE = dict(A=dict(arp2='arp16', bass2='gallop', pulse=True, kit2='boss'),
             B=dict(arp2='arp16', bass2='gallop', pulse=True, kit2='boss', pings=True),
             lead='lead', counter=None, drop=dict(arp2='arp16', kit='half'), intro=dict(arp2='arp16', pulse=True, pad_vel=60),
             violin='G5', top='G5', offset=72, source=('bn5', '0x16'))

def plan():
    A = dict(arp2='arp16', bass2='gallop', pulse=True, kit2='boss')
    p = {0: dict(arp2='arp16', pulse=True, pad_vel=60)}
    for b, line in zip((1, 2, 3, 4), (HOOK, ANSWER, AGAIN, CADENCE)):
        p[b] = dict(A, lead=line, crash=b in (1, 3), hit=b == 1, fill='snare' if b == 4 else None,
                    side=-1 if b % 2 else 1)
    for b in (5, 6, 7):
        p[b] = dict(A, pings=True, lead=CLIMB[b], counter=('vln', VIOLIN[b]), crash=b == 5, hit=b == 5,
                    fill='toms' if b == 7 else None, side=1)
    p[8] = dict(A, pings=True, lead=L.up(HOOK, 2), bell=True, crash=True, hit=True, lead_vel=120)
    p[9] = dict(ring=('Dm9', 'A5'))
    return p


def arrangement():
    s = Score()
    for q in range(16):
        s.note('noise', 0, 6 * q, 3, 0, 24 + 4 * q, **L.V.HAT)
    M.play(s, CHORDS, plan())
    return s
