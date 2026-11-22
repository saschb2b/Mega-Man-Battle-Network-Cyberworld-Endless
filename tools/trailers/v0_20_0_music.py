"""0.20.0's music: sketch A of the second round, "Battle Drive"
(sketch_red_a.py), grown into the cut's 25 bars. Original, in the manner
of BN4's virus battle (0x14), its boss battle (0x15) and tournament
(0x17), and BN6's boss battle (0x16); none of their notes, nor their
chords in their order. Their habits, read from the ROMs, are in
sketch_red_lib.py's docstring; the palette is FluidR3 GM's (SOUNDFONT),
rendered in docker/Dockerfile.music's image, its 3DS banner kept as our
own audio (BANNER_AUDIO).

The owner on the first piece (8 November 2026): "the music sounds flat
and not complex. I miss the mmbn dna and good use of the soundfont"; he
chose neither of its sketches. That piece had matched 0x17's calmest
numbers: a bass on one note a bar, one arpeggio, bare fifths. This one
is written to be at least as busy as BN4's battles.

Every part plays FluidR3 GM's instruments (the owner's gate 3, SKILL.md),
the GBA's channels doubling them for the top and the colour: the lead on
81 Saw Wave over square 1 (vibrato, scoops and falls, its echo three
ticks behind from the other side); the backing's 32nd pairs on 13
Xylophone over square 2, thrown hard left and right; the bass on 34
Electric Bass (pick) over the wave's pulse, sixteenths jumping octaves or
walking into the next chord (SAMPLED_PARTS); a counter-melody on the
chords' guide tones, 61 Brass Section in the A, 40 Violin in the B and
the tournament's anthem; 5 Electric Piano 2's sweep over three octaves;
48 Strings' chord samples recoloured by the bass, 61's brass chord
struck on the syncopations, 30 Distortion Guitar's 3-3-2 with 28's
chugs, 55's orchestra hit on a section's one, 9 Glockenspiel doubling
the lifted hook; the kit 24's, the noise channel's hats and crash.

The sketches went to the owner with this piece, unheard by him: the
agent's best guess is A, the battle drive nearest 0.14.0's piece he
called "awesome", with B's four-on-the-floor for the tournament.

The form, on the cut's slots (SKILL.md, The structure), 150 beats a
minute (a bar 48 video frames), D minor:
- Bars 0-1, the cold open: the 32nd pairs and the sweep over Dm9, the
  hats rising; then the band without its lead, a snare roll.
- Bars 2-3, the title card: the hook (D struck twice, a scoop up to A,
  falling by step) over Dm9 Bbmaj7, its answer over Csus4 C.
- Bars 4-7, BN4 joins: the hook again over Dm7 Fmaj7, the cadence over
  Gm7 Asus4 A, the hook and its answer once more; the brass's counter;
  toms.
- Bars 8-10, its viruses: the B, the lead in sequence over Bbmaj7 C |
  Ebmaj7 F (the borrowed bII's fake-out) | Gm7 Am7, the violin
  answering, the bass walking, both arpeggios; toms.
- Bars 11-14, the tournament: an anthem of held notes over Fmaj7 Gm7 |
  Am7 Bbmaj7 | Gm7 Ebmaj7 | Asus4 A, BN4's tournament's kick on every
  beat with the snare's ghosts, the violin's counter; toms.
- Bars 15-18, BN4's Navis: the hook lifted a tone, to E minor, the bell
  doubling it, the brass's counter, the whole band.
- Bars 19-20, the recap: a half-time bar (the drop, kept short), then a
  break of the drums alone and the hit.
- Bar 21, the stinger: E minor ringing.
- Bars 22-24, the end card: the 32nd pairs and the strings, softly.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from trailer_music import Score  # noqa: E402
from trailers import sketch_red_lib as L  # noqa: E402
from trailers.sketch_red_a import AGAIN, ANSWER, CADENCE, CLIMB, HOOK  # noqa: E402
from trailers.sketch_red_lib import (CHORD_SAMPLES, DS_ENV, DS_GAIN, DS_LEVEL,  # noqa: E402,F401
                                     HARDWARE, MIX, REVERB, SAMPLED_LEADS, SAMPLED_PARTS, SOUNDFONT, instruments)

BPM = 150
BARS = 25
TAIL = 2.0
BANNER = (2 * 4 * 60 / BPM, 2.9)         # (from bar 2's downbeat, seconds: the hook with the title)
BANNER_AUDIO = '3ds/banner-0.20.0.wav'

CHORDS = {0: ('Dm9',), 1: ('Dm9', 'Bbmaj7'), 2: ('Dm9', 'Bbmaj7'), 3: ('Csus4', 'C'),
          4: ('Dm7', 'Fmaj7'), 5: ('Gm7', 'Asus4', 'A'), 6: ('Dm9', 'Bbmaj7'), 7: ('Csus4', 'C'),
          8: ('Bbmaj7', 'C'), 9: ('Ebmaj7', 'F'), 10: ('Gm7', 'Am7'),
          11: ('Fmaj7', 'Gm7'), 12: ('Am7', 'Bbmaj7'), 13: ('Gm7', 'Ebmaj7'), 14: ('Asus4', 'A'),
          15: ('Em9', 'Cmaj7'), 16: ('Dsus4', 'D'), 17: ('Em7', 'Gmaj7'), 18: ('Am7', 'Bsus4', 'B'),
          19: ('Cmaj7', 'D'), 20: ('Em7',), 21: ('Em9',),
          22: ('Em9', 'Cmaj7'), 23: ('Am7', 'Bsus4'), 24: ('Em9',)}

ANTHEM = {
    11: [(0, 36, 'A5', 2), (36, 6, 'G5'), (42, 6, 'A5'), (48, 36, 'Bb5'), (84, 12, 'D6')],
    12: [(0, 36, 'C6', -1), (36, 12, 'A5'), (48, 24, 'F5'), (72, 12, 'A5'), (84, 12, 'D6')],
    13: [(0, 36, 'D6'), (36, 6, 'C6'), (42, 6, 'Bb5'), (48, 36, 'G5'), (84, 12, 'Bb5')],
    14: [(0, 24, 'D6'), (24, 12, 'E6'), (36, 12, 'D6'), (48, 36, 'C#6', -1)],
}
BRASS = L.guide({b: CHORDS[b] for b in (2, 3, 4, 5, 6, 7)}, 'F4')
VIOLIN = L.guide({b: CHORDS[b] for b in (8, 9, 10)}, 'A5', rhythm=((0, 36), (36, 12), (48, 24), (72, 24)))
ANTHEM_VLN = L.guide({b: CHORDS[b] for b in (11, 12, 13, 14)}, 'F5', rhythm=((12, 24), (36, 12), (60, 24), (84, 12)))
LIFT_BRASS = L.guide({b: CHORDS[b] for b in (15, 16, 17, 18)}, 'G4')
LINES = {2: HOOK, 3: ANSWER, 4: AGAIN, 5: CADENCE, 6: HOOK, 7: ANSWER}


def plan():
    A = dict(arp='pairs', bass='drive', power=True, kit='battle')
    p = {0: dict(arp='both', pad_vel=64, bass=None),
         1: dict(A, crash=True, hit=True, fill='snare')}
    for b in range(2, 8):
        p[b] = dict(A, lead=LINES[b], counter=('horn', BRASS[b]), crash=b in (2, 4, 6), hit=b in (2, 6),
                    fill='toms' if b == 7 else None, side=-1 if b % 2 == 0 else 1)
    for b, c in zip((8, 9, 10), (5, 6, 7)):
        p[b] = dict(arp='both', bass='walk', stabs=True, kit='battle', lead=CLIMB[c], counter=('vln', VIOLIN[b]),
                    crash=b == 8, hit=b == 8, fill='toms' if b == 10 else None, side=1)
    for b in (11, 12, 13, 14):
        p[b] = dict(arp='sweep', bass='walk', stabs=True, kit='four', lead=ANTHEM[b], counter=('vln', ANTHEM_VLN[b]),
                    crash=b in (11, 13), hit=b == 11, fill='toms' if b == 14 else None, side=-1, thin=True)
    for b, line in zip((15, 16, 17, 18), (HOOK, ANSWER, AGAIN, CADENCE)):
        p[b] = dict(A, arp='both', lead=L.up(line, 2), counter=('horn', LIFT_BRASS[b]), bell=b in (15, 17),
                    crash=b in (15, 17), hit=b in (15, 17), lead_vel=120, fill='toms' if b == 18 else None,
                    side=-1 if b % 2 else 1)
    p[19] = dict(arp='pairs', arp_vel=-10, bass='pump', kit='half', pad_vel=80)
    p[20] = dict(brk=True)
    p[21] = dict(ring=('Em9', 'B5'))
    p[22] = dict(arp='pairs', arp_vel=-30, pad_vel=60)
    p[23] = dict(arp='pairs', arp_vel=-36, pad_vel=56)
    return p


def arrangement():
    s = Score()
    for q in range(16):
        s.note('noise', 0, 6 * q, 3, 0, 24 + 4 * q, **L.V.HAT)
    s.hit('sfbass', 0, 72, 6, 'A2', 110)
    s.hit('sfbass', 0, 84, 6, 'C3', 110)
    L.play(s, CHORDS, plan())
    s.hit('pad_m', 24, 0, 150, 'G3', 64, -0.3)       # (Em9's last chord: G minor's sample over the E)
    return s
