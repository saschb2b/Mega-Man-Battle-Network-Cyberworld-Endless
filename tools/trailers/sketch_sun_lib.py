"""What 0.20.0's sketches share (sketch_sun_a and _b; not a piece of its
own): the parts of BN4 Red Sun's tournament theme (BN4's song 0x17, the
one docs/multirom/bn4.md names the tournament's; BN4's score is Toshihiko
Horiyama's, music_rules.md's sources), the release's own feature's theme,
as habits, never its notes, played on FluidR3 GM's samples (the owner's
gate 3).

What 0x17 does, read from BN4's ROM with song_habits.py bn4 0x17
--offset 12 --detail --theory --bars (the bar line an eighth in: the kick
on one, the snare on two and four), numbers only:
- 140 beats a minute (144 here, a bar 50 video frames). Pitch classes B
  23%, C 20%, then F#, D, C#, E: a minor home on B whose neighbour a
  semitone up leans on it. Its chords are bare fifths more than half the
  time (55% of its half bars), the rest maj7 (15%), sus4 (10%), sus2,
  m7b5; the roots rock by a semitone (up 37%, down 37%: B5 C5 C#5 C5),
  held into the next half bar 54% of the time; 1.28 chord changes a bar;
  38% of its chords hold a tone outside the key. A rock tournament's
  drive, not a town's song.
- The lead: both squares in unison, square 2 at 12.5% (and 25% in its
  second section), square 1 the same line four ticks later (an echo),
  the two thrown hard left and right and swapped (pan 0 and 127, a dozen
  times each); sixteenths and dotted eighths, 14 notes a bar in its A,
  six held notes a bar in its B; steps 37%, octave and bigger leaps 32%,
  the same note 9%; bent 36 times (range 6, falls of a few semitones),
  vibrato from 4 to 13. It waits four bars before it enters.
- The backing: a Direct Sound arpeggio (a short looped synth sample, pan
  95, right of centre), 784 notes of four ticks, 12 to 16 a bar, leaping
  69% of its moves across C3 to G#4, its accents on each beat's first and
  third sixteenths (272 and 272 against 106 and 134): the busy sampled
  track.
- The bass: the wave, staccato (five ticks of a sixteenth's six), ten
  notes a bar, B2 to G#3, one level, a slight vibrato.
- A held Direct Sound note, one a bar (F#3 to C4, looped), panned left
  (40): a pad under it all.
- The kit (Direct Sound): a kick on every beat (0 4 8 12) with a long low
  hit on 0 and 8, the bright snare on 4 and 12 with ghosts on 7 and 13,
  toms falling on 1-3, 4-7 and 8-11 in its fills; on the noise channel
  closed hats on every sixteenth but the snare's, at five levels, the
  open hat on 2 and 10, the crash on a phrase's one.
- The form: four bars without the lead, an A of eight (the lead busy),
  a B of seven (the lead held), a fill, a C, a bar of the drums alone,
  then all again.

What these sketches do with those jobs, every part on FluidR3 GM (gate
3), the squares doubling the lead and the backing for the top the
samples cannot reach: the lead on 81 Saw Wave over square 1 at 12.5%,
its echo four ticks later from the other side (the same saw, quieter);
the backing's arpeggio on 13 Xylophone over square 2 at 25%, right of
centre; the bass on 34 Electric Bass (pick), short; the pad on 48
Strings' chord samples, left; the power chords' bite on 30 Distortion
Guitar on the half bars; the kit 24's.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import trailer_soundfont as sf  # noqa: E402
from trailer_music import key, keys  # noqa: E402
from trailers import sketch_vb_lib as V  # noqa: E402
from trailers.sketch_vb_lib import HARDWARE, REVERB, SOUNDFONT, halves  # noqa: E402,F401

BPM = 144                        # (140 in the game; 144 makes a bar 50 video frames)
TAIL = 2.0
DS_LEVEL = 1.0
MIX = (('lead', ('sq1',), 0.10), ('arp', ('sq2',), 0.13), ('bass', ('wave',), 0.15),
       ('hats', ('noise',), 0.16), ('fx', ('fx',), 0.08))
CHORD_SAMPLES = dict(V.CHORD_SAMPLES)
DS_ENV = dict(V.DS_ENV, sflead=(255, 0, 230, 200), echo=(255, 0, 230, 200), mallet=(255, 0, 255, 150),
              sfbass=(255, 0, 200, 160))
DS_GAIN = dict(V.DS_GAIN, kick=0.86, snare=0.66, pad=0.5, pad_m=0.5, pad_M=0.5, pad_d=0.5, sflead=0.3, echo=0.17,
               mallet=0.32, sfbass=0.62, gtr=0.34)
SAMPLED_LEADS = ('dlead',)
# every melodic part on FluidR3 GM's instruments (gate 3), by role for
# music_check.py: the lead the saw, the backing the xylophone, the bass
# the picked bass
SAMPLED_PARTS = {'lead': ('sflead',), 'backing': ('mallet',), 'bass': ('sfbass',)}
# the squares doubling them at 0x17's duties: the lead's 12.5% (vibrato
# after 15 ticks), the backing's 25%
LEAD = dict(duty=0.125, env=(0, 2, 9, 2), vib=(24, 15))
BLIP = dict(duty=0.25, env=(0, 1, 9, 1))


def instruments():
    S = V.instruments('A2')
    # the lead (A4 to A6) and its echo, the backing's arpeggio (A3 to E5),
    # the bass (A2 to G3), each recorded in the middle of its range
    S['sflead'] = sf.instrument(81, [76], 2.0, fade=0.2)
    S['echo'] = S['sflead']
    S['mallet'] = sf.instrument(13, [64], 0.3, fade=0.05)
    S['sfbass'] = sf.instrument(34, [50], 0.6, fade=0.08)
    return S


# name: (the bass's root, the backing's four notes, the strings' sample
# and key); A is home, its neighbour B-flat leaning on it as 0x17's C
# leans on B (bare fifths, then maj7, sus and m7b5 colours)
H = {
    'A5': ('A2', 'A3 E4 A4 E5', 'pad_m', 'A3'),           # the bare fifth over the strings' A minor
    'Bb5': ('Bb2', 'Bb3 F4 Bb4 C5', 'pad', 'Bb3'),
    'B5': ('B2', 'B3 F#4 B4 C#5', 'pad', 'B3'),
    'C5': ('C3', 'C4 G4 C5 D5', 'pad', 'C3'),
    'Bbmaj7': ('Bb2', 'Bb3 D4 F4 A4', 'pad_m', 'D3'),     # Dm over Bb
    'Fmaj7': ('F2', 'F3 A3 C4 E4', 'pad_m', 'A3'),        # Am over F
    'Dsus4': ('D3', 'D4 G4 A4 D5', 'pad', 'G3'),          # G A D over D
    'Gsus2': ('G2', 'G3 A3 D4 G4', 'pad', 'G3'),
    'Em7b5': ('E3', 'E4 G4 Bb4 D5', 'pad_d', 'E3'),
    'Dm7': ('D3', 'D4 F4 A4 C5', 'pad_M', 'F3'),          # F over D
    'E5': ('E2', 'E3 B3 E4 F#4', 'pad', 'E3'),
}

FLOOR = key('F2')


def at(names, p):
    """the chord sounding on sixteenth p"""
    return next(nm for t0, n, nm in halves(names) if t0 <= 6 * p < t0 + n)


def root(name):
    r = key(H[name][0])
    while r < FLOOR:
        r += 12
    return r


# ---- the parts ----

def lead(s, bar, line, vel=112, side=-1, echo=True):
    """the lead (0x17's two squares in unison): the saw over square 1 at
    12.5%, its echo four ticks later from the other side; between the
    beats a little softer. A note may carry a fall (its fourth item, in
    semitones, over its last third)"""
    for item in line:
        t, n, k = item[:3]
        fall = item[3] if len(item) > 3 else 0
        bend = [(0, 0), (n * 2 // 3, 0), (n, fall)] if fall else None
        v = vel - (14 if t % 12 else 0)
        s.hit('sflead', bar, t, n, key(k), v, 0.55 * side, bend)
        s.note('sq1', bar, t, n, key(k), v - 18, pan=0.55 * side, **dict(LEAD, bend=bend) if bend else LEAD)
        if echo and t + 4 < 96:
            s.hit('echo', bar, t + 4, max(2, n - 4), key(k), v - 10, -0.85 * side, bend)


# the arpeggio's figure over a chord's four notes (an index a sixteenth,
# None a rest): eighths on the beat's first and third sixteenths, three
# of the four between, leaping across the octave as 0x17's
FIG = (0, None, 2, 3, 1, None, 3, 2, 0, 3, 2, None, 1, None, 3, 2)
ACC = (100, 54, 86, 60)


def arp(s, bar, names, until=16, vel=0, up=0):
    """the backing: four-tick notes over the bar's first chord (0x17's
    arpeggio keeps three or four pitches a bar while the chords above it
    turn), right of centre, accents on each beat's first and third
    sixteenths; the xylophone over square 2"""
    for p in range(until):
        idx = FIG[p]
        if idx is None:
            continue
        k = keys(H[names[0]][1])[idx] + up
        v = max(24, ACC[p % 4] + vel)
        s.hit('mallet', bar, 6 * p, 4, k, v, 0.45)
        s.note('sq2', bar, 6 * p, 4, k, max(16, v - 26), pan=0.45, **BLIP)


BASS = (0, 2, 4, 6, 7, 8, 10, 12, 14, 15)     # ten a bar, 0x17's count


def bass(s, bar, names, until=16, vel=0):
    """staccato, five ticks of a sixteenth's six, the bar's first root
    repeated under both its chords (0x17's bass: 88% of its moves the same
    note, 1.1 pitches a bar: the semitone's rock rides over it)"""
    r = root(names[0])
    for p in BASS:
        if p < until:
            s.hit('sfbass', bar, 6 * p, 5, r, (127 if p % 4 == 0 else 100) + vel, 0.0)


def pad(s, bar, names, vel=80):
    """the held chord sample, a chord a half bar, left (0x17's held note at
    pan 40)"""
    for t0, n, name in halves(names):
        s.hit(H[name][2], bar, t0, n - 2, key(H[name][3]) + 12, vel, -0.45)


def power(s, bar, names, vel=92):
    """the distortion guitar's fifth on each half bar's head, short (the
    bare fifths' bite, in the middle)"""
    for t0, n, name in halves(names):
        s.hit('gtr', bar, t0, 10, V.low(H[name][0]) + 12, vel, 0.0)


def kit(s, bar, crash=False, until=16, ghosts=True, snare=True):
    """0x17's kit: a kick on every beat, the snare on 4 and 12 with ghosts
    on 7 and 13; hats on every sixteenth but the snare's, the open hat on
    2 and 10"""
    for p in (0, 4, 8, 12):
        if p < until:
            s.hit('kick', bar, 6 * p, 6, vel=112 if p in (0, 8) else 96)
    if snare:
        for p in (4, 12):
            if p < until:
                s.hit('snare', bar, 6 * p, 6, vel=110, pan=0.05)
        if ghosts:
            for p in (7, 13):
                if p < until:
                    s.hit('snare', bar, 6 * p, 6, vel=46, pan=0.05)
    for p in range(until):
        if p in (4, 12) and snare:
            continue
        if p in (2, 10):
            s.note('noise', bar, 6 * p, 6, 0, 80, **V.OPEN)
        else:
            s.note('noise', bar, 6 * p, 3, 0, (86, 42, 0, 56)[p % 4] or 64, **V.HAT)
    if crash:
        s.note('fx', bar, 0, 64, 0, 104, **V.CRASH)


def fill(s, bar, vel=108):
    """toms falling on the last beat (0x17's fills: 8-11, then the bar's
    end)"""
    for p, tom, pan in ((12, 'tom_hi', -0.3), (13, 'tom_hi', -0.3), (14, 'tom_mid', 0.0), (15, 'tom_lo', 0.3)):
        s.hit(tom, bar, 6 * p, 6, vel=vel, pan=pan)


def ring(s, bar, name, top, vel=112):
    """the last chord ringing: the crash, the kick, the strings and the
    guitar's fifth held, the bass held, the lead's long note falling a
    little at its end"""
    s.note('fx', bar, 0, 96, 0, 116, **V.CRASH)
    s.hit('kick', bar, 0, 6)
    s.hit(H[name][2], bar, 0, 150, key(H[name][3]) + 12, 96, -0.35)
    s.hit('gtr', bar, 0, 80, V.low(H[name][0]) + 12, 100, 0.0, [(0, 0), (60, 0), (80, -3)])
    s.hit('sfbass', bar, 0, 90, root(name), 127, 0.0)
    bend = [(0, 0), (80, 0), (96, -1)]
    s.hit('sflead', bar, 0, 96, key(top), vel, 0.0, bend)
    s.note('sq1', bar, 0, 96, key(top), vel - 20, **dict(LEAD, bend=bend))
