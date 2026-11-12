"""0.21.0's music, "One Run": 29 bars at 144, E minor lifting to F#
minor, an original piece grown from the sketch the owner chose
(sketch_mix_a.py, 12 November 2026) in the manner of BN6's title theme
(0x01), the game whose run the other two join; none of its notes, nor its
chords in their order. Its habits, read from BN6's ROM, are in
sketch_mix_lib.py: the lead's long notes moving by step, a 32nd arpeggio
thrown hard left and right, the bass's sixteenths jumping octaves, a
march in the kit with its toms, maj7, m7 and sus2 colours over roots that
move by fourths and falling semitones. Every melodic part plays FluidR3
GM (the saw over square 1, the xylophone over square 2, the picked bass
over the wave).

The form, on the cut's slots (v0_21_0.py):
- Bars 0-1, the cold open: the 32nd pairs over the strings alone, then
  the bass and a half-time kit under them, the snare rolling in.
- Bars 2-3, the title: the hook and its cadence, the whole band.
- Bars 4-7, every act has its virus: the A (hook, answer, hook again,
  cadence), the brass on the guide tones.
- Bars 8-12, Navis and chips: the B climbing in sequence with both
  arpeggios and the violin, then the hook and its answer lifted a tone,
  the bell over them; toms.
- Bars 13-16, the Library: the drop, the hook in the first key over the
  pairs and a half-time kit, the band back for the cadence.
- Bars 17-22, the Battle Lab: the A lifted a tone, the bell on the hook,
  then the B's last two bars; toms.
- Bars 23-24, the recap: the hook and the cadence once more.
- Bar 25, the stinger: the lifted tonic ringing.
- Bars 26-28, the end card: the pairs and the strings alone, a last
  chord.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from trailer_music import Score  # noqa: E402
from trailers import sketch_mix_a as SEED  # noqa: E402
from trailers import sketch_mix_lib as M  # noqa: E402
from trailers.sketch_mix_lib import (CHORD_SAMPLES, DS_ENV, DS_GAIN, DS_LEVEL,  # noqa: E402,F401
                                     HARDWARE, MIX, REVERB, SAMPLED_LEADS, SAMPLED_PARTS, SOUNDFONT,
                                     instruments)

L = M.L
BPM = SEED.BPM
BARS = 29
TAIL = 1.2
LIFT = 2                                         # the key's lift, in semitones
BANNER = (17 * 4 * 60 / BPM, 2.9)                # the lifted hook, the 3DS HOME Menu's
BANNER_AUDIO = '3ds/banner-0.21.0.wav'

FLAT = ('C', 'Db', 'D', 'Eb', 'E', 'F', 'F#', 'G', 'Ab', 'A', 'Bb', 'B')
ST = SEED.STYLE
C = SEED.CHORDS


def shift(names, n=LIFT):
    """chords moved by n semitones"""
    return tuple(FLAT[(L.parse(nm)[0] + n) % 12] + L.parse(nm)[1] for nm in names)


def phrase(first, up=0):
    """the A's four bars from bar `first`: {bar: (chords, line)}"""
    lines = (SEED.HOOK, SEED.ANSWER, SEED.AGAIN, SEED.CADENCE)
    return {first + i: (shift(C[1 + i], up), L.up(lines[i], up)) for i in range(4)}


def layout():
    """{bar: (chords, the lead's line or None)}"""
    home, away = C[0], (shift(C[1])[0],)
    bars = {0: (home, None), 1: (home, None),
            2: (C[1], SEED.HOOK), 3: (C[4], SEED.CADENCE)}
    bars.update(phrase(4))
    bars.update({8 + i: (C[5 + i], SEED.CLIMB[5 + i]) for i in range(3)})
    bars[11] = (shift(C[1]), L.up(SEED.HOOK, LIFT))
    bars[12] = (shift(C[2]), L.up(SEED.ANSWER, LIFT))
    bars.update(phrase(13))
    bars.update(phrase(17, LIFT))
    bars[21] = (shift(C[6]), L.up(SEED.CLIMB[6], LIFT))
    bars[22] = (shift(C[7]), L.up(SEED.CLIMB[7], LIFT))
    bars[23] = (shift(C[1]), L.up(SEED.HOOK, LIFT))
    bars[24] = (shift(C[4]), L.up(SEED.CADENCE, LIFT))
    for b in (25, 26, 27, 28):
        bars[b] = (away, None)
    return bars


BARS_OF = layout()
CHORDS = {b: names for b, (names, _) in BARS_OF.items()}


def plan():
    A, B, key = ST['A'], ST['B'], ST['lead']
    line = {b: ln for b, (_, ln) in BARS_OF.items()}
    violin = L.guide({b: CHORDS[b] for b in (8, 9, 10, 21, 22)}, ST['violin'], rhythm=((0, 36), (36, 12), (48, 24), (72, 24)))
    p = {0: dict(ST['intro']),
         1: dict(ST['intro'], bass2=A.get('bass2'), bass=A.get('bass'), kit='half', fill='snare')}
    p[2] = dict(A, **{key: line[2]}, crash=True, hit=True)
    p[3] = dict(A, **{key: line[3]}, fill='toms', side=1)
    for b in (4, 5, 6, 7):
        p[b] = dict(A, **{key: line[b]}, crash=b in (4, 6), hit=b == 4, fill='snare' if b == 7 else None,
                    side=-1 if b % 2 == 0 else 1)
    for b in (8, 9, 10):
        p[b] = dict(B, **{key: line[b]}, counter=('vln', violin[b]), crash=b == 8, hit=b == 8, side=1)
    p[11] = dict(B, **{key: line[11]}, bell=True, crash=True, hit=True, lead_vel=118)
    p[12] = dict(B, **{key: line[12]}, fill='toms', side=1)
    # the drop: the hook over the arpeggio and a half-time kit, then the band back
    p[13] = dict(ST['drop'], **{key: line[13]}, thin=True, lead_vel=104, pad_vel=64, hit=True)
    p[14] = dict(ST['drop'], **{key: line[14]}, thin=True, lead_vel=104, pad_vel=64, side=1)
    p[15] = dict(A, **{key: line[15]}, crash=True)
    p[16] = dict(A, **{key: line[16]}, fill='snare', side=1)
    for b in (17, 18, 19, 20):
        p[b] = dict(B, **{key: line[b]}, bell=b in (17, 19), crash=b in (17, 19), hit=b == 17, lead_vel=120,
                    fill='snare' if b == 20 else None, side=-1 if b % 2 else 1)
    for b in (21, 22):
        p[b] = dict(B, **{key: line[b]}, counter=('vln', violin[b]), crash=b == 21, hit=b == 21,
                    fill='toms' if b == 22 else None, side=1, lead_vel=120)
    p[23] = dict(B, **{key: line[23]}, bell=True, crash=True, hit=True, lead_vel=120)
    p[24] = dict(A, **{key: line[24]}, fill='snare', side=1, lead_vel=120)
    p[25] = dict(ring=(CHORDS[25][0], L.key(ST['top']) + LIFT))
    p[26] = dict(ST['intro'], pad_vel=58)
    p[27] = dict(ST['intro'], pad_vel=54)
    p[28] = dict(pad_vel=60)
    if ST['counter']:
        brass = L.guide({b: CHORDS[b] for b in (4, 5, 6, 7, 17, 18, 19, 20)}, 'G4')
        for b in brass:
            p[b]['counter'] = (ST['counter'], brass[b])
    return p


def arrangement():
    s = Score()
    for q in range(16):
        s.note('noise', 0, 6 * q, 3, 0, 24 + 4 * q, **L.V.HAT)
    M.play(s, CHORDS, plan())
    # the last chord: the bell on its top note
    s.hit('bell', 28, 0, 48, L.key(ST['top']) + LIFT + 12, 76, 0.0)
    s.hit('sfbass', 28, 0, 60, L.bass_root(CHORDS[28][0]), 100, 0.0)
    return s
