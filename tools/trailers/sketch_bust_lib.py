"""What 0.17.0's three sketches share (sketch_bust_a, _b and _c; not a
piece of its own): the parts of BN5's virus battle theme, "Virus Busting"
(BN5 Team Colonel's song 0x15; BN5's score is Akari Kaida's,
music_rules.md's sources), as habits, never its notes, played on the
approved palette (sketch_vb_lib.py: FluidR3 GM's samples, 0.16.0's mix).

What 0x15 does, read from its ROM with song_habits.py --offset 12
--detail --theory --bars (the bar line 12 ticks in: the kick and crash on
one, the snare on two and four) and the bars' rhythms in aggregate,
numbers only:
- 148 beats a minute, F minor seldom on its tonic (i 4% of its half
  bars): bVII7 14, IIm7b5 12, IV5 9, IVm 9, the Lydian #IVmaj7 9, bVII 7;
  power fifths 17%, m7 17, maj7 14, 7 14, m7b5 14; roots up a fourth 19%,
  a fifth 17%, a minor sixth 17%, a tone 15%; 1.6 chord changes a bar.
- The stab (Direct Sound, a 0.3 s decaying sample) at one pitch, 86% of
  its notes the same, on 0 3 6 | 8 11 14 of the bar (3+3+2 twice), loud
  (120), with ghosts on 7 and 15 (32): the song's signature.
- The lead, square 2 (50% and 75%, envelope 0 3 11 1, vibrato 6 after 14
  ticks): sixteenths and held notes of 18 and 30 ticks, repeating 37-41%
  of its notes, stepping 29-43%; its commonest bars a note on one, a run
  from the third sixteenth to the half bar, a pickup on 13 14 15; accents
  100 on the beat, 60 and 32 on the e.
- The backing, square 1 (75%, envelope 0 1 11 1): sixteenths through the
  bar at one level (100), skips 32%, steps 25%, the same 15%, thrown hard
  left and right every few notes; its 50% voice rests on 0 1 and 8 9.
- The bass, the wave plucked (0 5 4 1): sixteenths in all but the sixth
  place, 127 with the e at 60, the root repeated, fifths below, octaves,
  steps into the next root, bent 102 times (falls).
- The kit: the kick on 0 2 6 8 10 (14 15), the snare on 4 7 9 12 with
  ghosts after, closed hats in pairs on each beat's first two sixteenths,
  an open hat on every "and" (2 6 10 14), the crash on one, toms into a
  section.
- The form: an A with the stab (bars 1-8), a B where the stab gives way to
  chords held at the bar's head (9-15), a break of the drums and the stab
  alone, the bass out (17-19), the A again.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from trailer_music import key, keys  # noqa: E402
from trailers import sketch_vb_lib as V  # noqa: E402
from trailers.sketch_vb_lib import (CHORD_SAMPLES, DS_ENV, DS_LEVEL, HARDWARE, REVERB, SOUNDFONT,  # noqa: E402,F401
                                    halves, low, power, toms)

BPM = 150                       # (148 in the game; 150 makes a bar 48 video frames)
TAIL = 1.8
# (0.16.0's mix moved toward 0x15's balance: its lows half the energy,
# its mids under 0x22's, so the bass up, the squares and strings down)
MIX = (('lead', ('sq1',), 0.13), ('arp', ('sq2',), 0.19), ('bass', ('wave',), 0.17),
       ('hats', ('noise',), 0.12), ('fx', ('fx',), 0.08))
DS_GAIN = dict(V.DS_GAIN, kick=0.95, pad=0.8, pad_m=0.8, pad_M=0.8, pad_d=0.8)
# (the stab is the sampled part music_check.py holds against 0x15's
# Direct Sound track, which is its stab and held chords)
SAMPLED_LEADS = ('gtr', 'brass')

LEAD = dict(duty=0.5, env=(0, 3, 11, 1), vib=(24, 14), pan=0.0)
ARP = dict(duty=0.75, env=(0, 1, 11, 1))
BASS = dict(env=(0, 5, 4, 1), wave=V.SAW)
HELD = dict(env=(0, 0, 15, 3), wave=V.SAW)

# name: (the bass's root, the backing's four notes, the chord sample and
# the key it is played at, the stab's one pitch)
H = {
    'Fm7': ('F2', 'Ab4 C5 Eb5 F5', 'pad_M', 'Ab3', 'F3'),          # Ab over F
    'Fm9': ('F2', 'Ab4 C5 Eb5 G5', 'pad_m', 'F3', 'F3'),
    'Bbm7': ('Bb2', 'Ab4 Bb4 Db5 F5', 'pad_M', 'Db3', 'F3'),       # Db over Bb
    'Eb7': ('Eb3', 'G4 Bb4 Db5 Eb5', 'pad_d', 'G3', 'G3'),         # Gdim over Eb
    'Eb6': ('Eb3', 'G4 Bb4 C5 Eb5', 'pad_M', 'Eb3', 'G3'),
    'Ebsus2': ('Eb3', 'Bb4 Eb5 F5 Bb5', 'pad', 'Eb3', 'F3'),
    'Ebsus4': ('Eb3', 'Ab4 Bb4 Eb5 Ab5', 'pad', 'Ab2', 'F3'),      # Ab Bb Eb over Eb
    'Eb': ('Eb3', 'G4 Bb4 Eb5 G5', 'pad_M', 'Eb3', 'G3'),
    'Abmaj7': ('Ab2', 'G4 Ab4 C5 Eb5', 'pad_m', 'C3', 'F3'),       # Cm over Ab
    'Dbmaj7': ('Db3', 'Ab4 C5 Db5 F5', 'pad_m', 'F3', 'F3'),       # Fm over Db
    'Gm7b5': ('G2', 'G4 Bb4 Db5 F5', 'pad_m', 'Bb3', 'F3'),        # Bbm over G
    'Bmaj7': ('B2', 'A#4 B4 D#5 F#5', 'pad_m', 'D#3', 'F#3'),      # D#m over B: the Lydian #IV
    'Gbmaj7': ('Gb2', 'F4 Gb4 Bb4 Db5', 'pad_m', 'Bb3', 'Gb3'),    # Bbm over Gb: the bII
    'Csus2': ('C3', 'G4 C5 D5 G5', 'pad', 'C3', 'G3'),
    'C': ('C3', 'G4 C5 E5 G5', 'pad_M', 'C3', 'G3'),
    'Cm7': ('C3', 'G4 Bb4 C5 Eb5', 'pad_M', 'Eb3', 'G3'),          # Eb over C
    'Ab': ('Ab2', 'Ab4 C5 Eb5 Ab5', 'pad_M', 'Ab3', 'Ab3'),
}

STAB = (0, 3, 6, 8, 11, 14)     # 3+3+2 twice
GHOST = (7, 15)
FIG = (0, 1, 2, 2, 3, 1, 2, 1)  # the backing's figure over a chord's four notes


def at(names, p):
    """the chord sounding on sixteenth p"""
    return next(nm for t0, n, nm in halves(names) if t0 <= 6 * p < t0 + n)


def stab(s, bar, names, inst='gtr', vel=104, ghosts=True, until=16, pan=0.3, pitch=None):
    """0x15's stab: one pitch on 3+3+2 twice, its top near F4 as the
    song's, the ghosts before the beat, swung a little left and right (BN5
    swings its parts wide, not hard); `pitch` a chord's key for another
    instrument than the guitar"""
    def k_at(p):
        return key(pitch[at(names, p)]) if pitch else low(H[at(names, p)][4]) + 24
    for i, p in enumerate(STAB):
        if p < until:
            s.hit(inst, bar, 6 * p, 10, k_at(p), vel if p in (0, 8) else vel - 10, -pan if i % 2 == 0 else pan)
    if ghosts:
        for p in GHOST:
            if p < until:
                s.hit(inst, bar, 6 * p, 5, k_at(p), 44, pan)


def backing(s, bar, names, vel=0, rest=False, until=16, register=0):
    """sixteenths over each chord's four notes at one level, thrown hard
    left and right by the beat; `rest` the 50% voice's rest on 0 1 and 8 9"""
    for p in range(until):
        if rest and p % 8 in (0, 1):
            continue
        v = [k + register for k in keys(H[at(names, p)][1])]
        side = -0.9 if (p // 4) % 2 == 0 else 0.9
        s.note('sq2', bar, 6 * p, 6, v[FIG[p % 8]], max(30, 100 - (8 if p % 4 == 1 else 0) + vel), pan=side, **ARP)


def bass(s, bar, names, nxt=None, until=16, fall=True):
    """sixteenths but the sixth place: the root on the kick's places, its
    soft repeat, a step above and back, the octave, the fifth below, a
    step into the next root; a fall at the bar's end"""
    for p in range(until):
        if p == 6:
            continue
        r = key(H[at(names, p)][0])
        q = p % 8
        if q == 3:
            k = r + 2
        elif q == 5:
            k = r + 12
        elif q == 6:
            k = r + 7 if r - 5 < key('D2') else r - 5
        elif q == 7:
            following = at(names, p + 1) if p < 8 else nxt
            target = key(H[following][0]) if following else r
            k = r if target == r else target + (2 if target < r else -2)
        else:
            k = r
        soft = p % 4 == 1
        last = p == until - 1 and fall
        voice = dict(BASS, bend=[(0, 0), (3, 0), (5, -2)]) if last else BASS
        s.note('wave', bar, 6 * p, 5, k, 60 if soft else 127, **voice)


def chords(s, bar, names, vel=94, up=0):
    """the strings, a chord a half bar"""
    for t0, n, name in halves(names):
        s.hit(H[name][2], bar, t0, n - 2, key(H[name][3]) + up, vel, -0.35 if t0 < 48 else 0.35)


def held(s, bar, names, vel=100, up=0):
    """0x15's B: the chord struck at the bar's head, held through its
    half, doubled a hair later on the other side"""
    name = names[0]
    s.hit(H[name][2], bar, 0, 46, key(H[name][3]) + up, vel, -0.35)
    s.hit(H[name][2], bar, 3, 43, key(H[name][3]) + up, vel - 30, 0.35)


def kit(s, bar, crash=False, until=16, ghosts=True, kick=(0, 2, 6, 8, 10)):
    """0x15's kit: the kick 0 2 6 8 10, the snare 4 7 9 12 with ghosts,
    closed hats in pairs, an open hat on every "and" """
    for p in kick:
        if p < until:
            s.hit('kick', bar, 6 * p, 6, vel=108 if p in (0, 8) else 92)
    for p in (4, 7, 9, 12):
        if p < until:
            s.hit('snare', bar, 6 * p, 6, vel=104 if p in (4, 12) else 76, pan=0.05)
    if ghosts and 13 < until:
        s.hit('snare', bar, 6 * 13, 6, vel=44, pan=0.05)
    hats(s, bar, until)
    if crash:
        s.note('fx', bar, 0, 48, 0, 116, **V.CRASH)


def hats(s, bar, until=16, level=0):
    for p in range(until):
        q = p % 4
        if q == 2:
            s.note('noise', bar, 6 * p, 6, 0, 80 + level, **V.OPEN)
        elif q in (0, 1):
            s.note('noise', bar, 6 * p, 3, 0, (72 if q == 0 else 52) + level, **V.HAT)


def lead(s, bar, line, vel=116, shift=0):
    """the square's line; sixteenths off the beat a little softer (0x15's
    100 on the beat, 60 on the e)"""
    for t, n, k in line:
        soft = n <= 6 and t % 24 in (6, 18)
        s.note('sq1', bar, t, n, key(k) + shift, vel - (14 if soft else 0), **LEAD)


def double(s, bar, line, vel=88, octave=-12, pan=0.25):
    """the saw doubling the square an octave down"""
    for t, n, k in line:
        s.hit('double', bar, t, max(4, n - 1), key(k) + octave, vel, pan)


def ring(s, bar, chord, top, root, vel=118):
    """the last chord ringing: the strings, the guitar's power chord
    falling, the bass held and falling an octave, the lead's long note"""
    s.note('fx', bar, 0, 96, 0, 124, **V.CRASH)
    s.hit('kick', bar, 0, 6)
    s.hit('snare', bar, 0, 6)
    power(s, bar, 0, 90, root, vel=vel, fall=True)
    s.hit(H[chord][2], bar, 0, 150, key(H[chord][3]) + 12, 108, -0.35)
    s.note('wave', bar, 0, 92, key(H[chord][0]), 127, **dict(HELD, bend=[(0, 0), (54, 0), (92, -12)]))
    s.note('sq1', bar, 0, 96, top, vel, **dict(LEAD, bend=[(0, 0), (76, 0), (96, -2)]))
