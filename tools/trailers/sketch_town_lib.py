"""What 0.19.0's sketches share (sketch_town_a and _b; not a piece of its
own): the parts of BN5's home town theme, ACDC Town (BN5 Team Colonel's
song 0x03, the one its map-music lists give ACDC Town's maps,
docs/rom/bn5-world.md; BN5's score is Akari Kaida's, music_rules.md's
sources), the owner's reference for this release for its happy, upbeat
rhythm, as habits, never its notes, played on the approved palette
(sketch_vb_lib.py: FluidR3 GM's samples).

What 0x03 does, read from its ROM with song_habits.py --offset 12
--detail --theory --bars (the bar line 12 ticks in: the kick and crash on
one, the snare on two and four), numbers only:
- 112 beats a minute, A major. Its chords as degrees: Vsus2 19% of its
  half bars, IIm 13, VIm 13, IVsus4 9, the Mixolydian Vm 7, I5 5, Vsus4
  5, bVI7 5, I7 3; qualities m7 32%, sus2 21, sus4 17, 7 13, bare fifths
  7, maj7 3; the roots up a fourth 26%, a tone 17%, a fifth 15%, down a
  tone 13%; a third of its chords hold a tone outside the key; 1.84 chord
  changes a bar.
- The lead, square 2 (50%, some 75%; envelope 0 3 11 1, vibrato 6 at
  speed 31 after 14 ticks), held legato (96%): sixteenths most, 32nds,
  eighths, dotted eighths and quarters; half its notes repeat the one
  before (52%), a quarter step, few leap (7%); accents 100 on the beat,
  60 and 92 between. Its four-bar phrase breathes: a call of about 8
  notes, a busy answer of 15, the call again of 11, a held bar of 4.
- The backing, square 1 (50%), 32nd-note blips on every sixteenth thrown
  hard left and right note by note (pan 0 and 127, a hundred times
  each), a third of its moves an octave's leap, a third the same note,
  accents 100, 60 and 32; in the B every 32nd (32 a bar); a 25% voice in
  plain sixteenths beside it.
- The bass, the wave plucked (0 5 4 1): sixteenths, 14 a bar (one note a
  bar held a dotted eighth), the root repeated (60% of its moves), an
  octave's jump (15%), steps into the next root, 127 and 88; it sits
  high (B2 up), so the town is light; bent 22 times.
- Two Direct Sound tracks: a mellow chord sample decaying (sustain 69 of
  255) struck four to six times a bar, on the beat and its third and
  fourth sixteenths, swung a little left and right (40 and 87, not hard);
  in the C a sampled melody of repeated sixteenths (67% the same note,
  A4 to A5) while the square holds back; a low sampled answer (D3 to
  B3, 9 notes, by step) in the second half of each A phrase, soft.
- The kit: the kick on 0 7 10 14, the snare (a bright sample) on 4 and
  12 with a ghost on 13, a clap on 10 and 11 in a few bars, closed hats on
  every sixteenth at four levels, the open hat on 14, a crash on the
  phrase's one, toms falling on 13 14 15 into a section.
- The form: an A (eight bars: the call, the low answer), a B (eight: the
  backing in 32nds, the low answer out), a C (nine: the sampled melody
  leads), one bar of the bass and drums thinned, then all again.

What these sketches do with those jobs: every part on FluidR3 GM's
instruments (the owner's gate 3, SKILL.md), never the squares' or the
wave's own sound: the lead on 81 Saw Wave, the blips on 12 Marimba, the
bass on 38 Synth Bass 1 (SAMPLED_PARTS); the noise channel keeps the hats.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import trailer_soundfont as sf  # noqa: E402
from trailer_music import key, keys  # noqa: E402
from trailers import sketch_vb_lib as V  # noqa: E402
from trailers.sketch_vb_lib import HARDWARE, REVERB, SOUNDFONT, halves  # noqa: E402,F401

BPM = 112.5                      # (112 in the game; 112.5 makes a bar 64 video frames)
TAIL = 2.0
DS_LEVEL = 1.0
MIX = (('lead', ('sq1',), 0.15), ('arp', ('sq2',), 0.16), ('bass', ('wave',), 0.15),
       ('hats', ('noise',), 0.08), ('fx', ('fx',), 0.07))
# the chord samples: the strings' (the approved palette) and the
# electric piano's (FluidR3 GM's program 4), one each of major, minor,
# suspended and diminished, recoloured by the bass (Kaida's way)
CHORD_SAMPLES = dict(V.CHORD_SAMPLES, ep=(0, 2, 7, 12), ep_m=(0, 3, 7, 12), ep_M=(0, 4, 7, 12), ep_d=(0, 3, 6, 12))
DS_ENV = dict(V.DS_ENV, ep=(255, 238, 80, 200), ep_m=(255, 238, 80, 200), ep_M=(255, 238, 80, 200),
              ep_d=(255, 238, 80, 200), low=(255, 0, 230, 200),
              sflead=(255, 0, 230, 200), mallet=(255, 0, 255, 170), sfbass=(255, 0, 240, 190))
DS_GAIN = dict(V.DS_GAIN, kick=0.7, snare=0.6, ep=0.75, ep_m=0.75, ep_M=0.75, ep_d=0.75, dlead=0.34, low=0.42,
               pad=0.6, pad_m=0.6, pad_M=0.6, pad_d=0.6, sflead=0.32, mallet=0.3, sfbass=0.55)
SAMPLED_LEADS = ('dlead',)
# every melodic part on FluidR3 GM's instruments (the owner, 2 November
# 2026: the squares and the wave alone are "just not on the quality we
# want"): the lead the saw lead's, the backing's blips the marimba's, the
# bass the synth bass's; music_check.py reads them by these roles and
# fails a piece whose lead, backing or bass is not the soundfont's
SAMPLED_PARTS = {'lead': ('sflead',), 'backing': ('mallet',), 'bass': ('sfbass',)}



def instruments():
    S = V.instruments('Bb2')
    for name in ('ep', 'ep_m', 'ep_M', 'ep_d'):
        S[name] = sf.instrument(4, [60 + i for i in CHORD_SAMPLES[name]], 0.9, base=60, fade=0.25)
    # (the low answer: the brass section's one note, an octave down)
    S['low'] = sf.instrument(61, [48], 0.5, fade=0.12)
    # the lead (F4 to D6), the backing's blips (F4 to B-flat 5) and the bass
    # (B-flat 2 to G4), each recorded in the middle of its range
    S['sflead'] = sf.instrument(81, [74], 2.4, fade=0.2)
    S['mallet'] = sf.instrument(12, [74], 0.35, fade=0.05)
    S['sfbass'] = sf.instrument(38, [55], 2.4, fade=0.2)
    return S


# name: (the bass's root, the backing's four notes, the electric piano's
# sample and key, the strings' sample and key); in B-flat major
H = {
    'Bbsus2': ('Bb2', 'Bb4 C5 F5 Bb5', 'ep', 'Bb3', 'pad', 'Bb3'),
    'Bb': ('Bb2', 'Bb4 D5 F5 Bb5', 'ep_M', 'Bb3', 'pad_M', 'Bb3'),
    'Bb7': ('Bb2', 'Ab4 Bb4 D5 F5', 'ep_d', 'D3', 'pad_d', 'D3'),       # D F Ab over Bb
    'Cm7': ('C3', 'G4 Bb4 C5 Eb5', 'ep_M', 'Eb3', 'pad_M', 'Eb3'),      # Eb over C
    'Dm7': ('D3', 'A4 C5 D5 F5', 'ep_M', 'F3', 'pad_M', 'F3'),          # F over D
    'Ebsus4': ('Eb3', 'Ab4 Bb4 Eb5 Ab5', 'ep', 'Ab3', 'pad', 'Ab3'),    # Ab Bb Eb over Eb
    'Ebmaj7': ('Eb3', 'G4 Bb4 D5 Eb5', 'ep_m', 'G3', 'pad_m', 'G3'),    # Gm over Eb
    'Eb': ('Eb3', 'G4 Bb4 Eb5 G5', 'ep_M', 'Eb3', 'pad_M', 'Eb3'),
    'Fsus2': ('F3', 'F4 G4 C5 F5', 'ep', 'F3', 'pad', 'F3'),
    'Fsus4': ('F3', 'F4 Bb4 C5 F5', 'ep', 'Bb3', 'pad', 'Bb3'),        # Bb C F over F
    'F': ('F3', 'F4 A4 C5 F5', 'ep_M', 'F3', 'pad_M', 'F3'),
    'Fm7': ('F3', 'Ab4 C5 Eb5 F5', 'ep_M', 'Ab3', 'pad_M', 'Ab3'),      # Ab over F: the Mixolydian v
    'Gm7': ('G2', 'G4 Bb4 D5 F5', 'ep_M', 'Bb3', 'pad_M', 'Bb3'),       # Bb over G
    'Gb7': ('Gb2', 'Gb4 Bb4 Db5 E5', 'ep_d', 'Bb3', 'pad_d', 'Bb3'),     # Bb Db E over Gb: the bVI7
    'Abmaj7': ('Ab2', 'G4 Ab4 C5 Eb5', 'ep_m', 'C3', 'pad_m', 'C3'),    # Cm over Ab: the borrowed bVII
}

FLOOR = key('Bb2')               # (the bass sits high, as 0x03's)


def at(names, p):
    """the chord sounding on sixteenth p"""
    return next(nm for t0, n, nm in halves(names) if t0 <= 6 * p < t0 + n)


def root(name):
    r = key(H[name][0])
    while r < FLOOR:
        r += 12
    return r


# ---- the parts ----

def lead(s, bar, line, vel=112, shift=0):
    """the lead's line (the square's part in 0x03, here the saw lead's),
    legato; between the beats a little softer (0x03's 100 on the beat, 60
    and 92 between)"""
    for t, n, k in line:
        soft = t % 24 != 0
        s.hit('sflead', bar, t, n, key(k) + shift, vel - (18 if soft and n <= 6 else 0))


BLIP_VEL = (100, 32, 60, 32)     # a beat's four sixteenths
# the backing's figure over a chord's four notes (the fifth index its
# top an octave up): the bottom note bouncing to its octave as 0x03's
FIG = (0, 0, 4, 0, 1, 1, 2, 1)


def blips(s, bar, names, until=16, double=False, vel=0, register=0):
    """32nd-note blips on every sixteenth (with `double` every 32nd, the
    second soft), thrown hard left and right note by note"""
    n = 0
    for p in range(until):
        v = [k + register for k in keys(H[at(names, p)][1])]
        v.append(v[0] + 12)
        k = v[FIG[p % 8]]
        for half in ((0, 1) if double else (0,)):
            side = -0.9 if n % 2 == 0 else 0.9
            level = BLIP_VEL[p % 4] if not half else 32
            s.hit('mallet', bar, 6 * p + 3 * half, 3, k, max(24, level + vel), side)
            n += 1


def bass(s, bar, names, nxt=None, until=16):
    """sixteenths, the root repeated, its octave once with the kick on
    10, the e's softer; the note on 13 held a dotted eighth, sliding into
    the next bar's root"""
    for p in range(min(until, 14)):
        r = root(at(names, p))
        k = r + 12 if p == 10 else r
        if p == 13:
            target = root(nxt) if nxt else r
            k = target - 2 if target > r else target + 2 if target < r else r
            bend = [(0, 0), (12, 0), (18, (target - k) * 1.0)] if target != r else None
            s.hit('sfbass', bar, 6 * p, 18, k, 127, 0.0, bend)
            continue
        s.hit('sfbass', bar, 6 * p, 6, k, 88 if p in (1, 5, 9, 11) else 127)


STAB = (0, 3, 6, 10, 12)         # the beat, then its fourth and third sixteenths


def stabs(s, bar, names, inst=2, vel=96, until=16, places=STAB):
    """the electric piano's chord, struck on 0x03's places, swung a little
    left and right, stopped where its chord gives way; inst 2 the electric
    piano, 4 the strings"""
    for i, p in enumerate(places):
        if p < until:
            name = at(names, p)
            end = next(t0 + n for t0, n, nm in halves(names) if t0 <= 6 * p < t0 + n)
            s.hit(H[name][inst], bar, 6 * p, min(16, end - 6 * p), key(H[name][inst + 1]),
                  vel if p % 4 == 0 else vel - 14, -0.35 if i % 2 == 0 else 0.35)


def strings(s, bar, names, vel=86, up=12):
    """the strings, a chord a half bar, held"""
    for t0, n, name in halves(names):
        s.hit(H[name][4], bar, t0, n - 2, key(H[name][5]) + up, vel, -0.35 if t0 < 48 else 0.35)


def answer(s, bar, line, vel=84):
    """the low sampled answer, soft, by step"""
    for t, n, k in line:
        s.hit('low', bar, t, n, key(k), vel, 0.2)


def melody(s, bar, line, vel=100):
    """the sampled melody of the C: the saw lead (the approved palette's)"""
    for t, n, k in line:
        s.hit('dlead', bar, t, max(4, n - 1), key(k), vel if t % 24 == 0 else vel - 16, -0.15)


def kit(s, bar, crash=False, until=16, clap=False, ghost=True):
    """0x03's kit: the kick on 0 7 10 14, the snare on 4 and 12 (a ghost
    on 13), closed hats on every sixteenth at four levels, the open hat on
    14"""
    for p in (0, 7, 10, 14):
        if p < until:
            s.hit('kick', bar, 6 * p, 6, vel=110 if p == 0 else 96)
    for p in (4, 12):
        if p < until:
            s.hit('snare', bar, 6 * p, 6, vel=106, pan=0.05)
    if ghost and 13 < until:
        s.hit('snare', bar, 6 * 13, 6, vel=40, pan=0.05)
    if clap:
        for p in (10, 11):
            if p < until:
                s.hit('clap', bar, 6 * p, 6, vel=70 if p == 10 else 56, pan=-0.1)
    for p in range(until):
        if p == 14:
            s.note('noise', bar, 6 * p, 6, 0, 76, **V.OPEN)
        else:
            s.note('noise', bar, 6 * p, 3, 0, (80, 40, 60, 46)[p % 4], **V.HAT)
    if crash:
        s.note('fx', bar, 0, 64, 0, 104, **V.CRASH)


def toms(s, bar, vel=104):
    """falling on 13 14 15 into a section"""
    for p, tom, pan in ((13, 'tom_hi', -0.3), (14, 'tom_mid', 0.0), (15, 'tom_lo', 0.3)):
        s.hit(tom, bar, 6 * p, 6, vel=vel, pan=pan)


def ring(s, bar, name, top, vel=112):
    """the last chord ringing: the crash, the kick, the electric piano and
    the strings held, the bass held, the lead's long note"""
    s.note('fx', bar, 0, 96, 0, 116, **V.CRASH)
    s.hit('kick', bar, 0, 6)
    s.hit(H[name][2], bar, 0, 90, key(H[name][3]), 104, -0.35)
    s.hit(H[name][4], bar, 0, 150, key(H[name][5]) + 12, 96, 0.35)
    s.hit('sfbass', bar, 0, 90, root(name), 127, 0.0, [(0, 0), (60, 0), (90, -2)])
    s.hit('sflead', bar, 0, 96, key(top), vel, 0.0, [(0, 0), (80, 0), (96, -1)])
