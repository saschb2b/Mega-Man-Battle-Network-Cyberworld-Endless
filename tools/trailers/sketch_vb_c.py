"""Sketch C for 0.16.0, BN5's side of the team: nine bars for the owner's
ear, not a cut. Original, in the manner of BN5's virus battle theme (Team
Colonel's song 0x15, from Akari Kaida's score), for the commands the Cross
Navis bring from BN5's Team ProtoMan; none of its notes, nor its chords
in their order. Played on the approved palette's samples
(sketch_vb_lib.py).

    tools/trailer_music.py's render_any(OUT, trailers.sketch_vb_c), in
    docker/Dockerfile.music's image; on 26 October 2026 also through
    GeneralUser GS and MuseScore General (CW_SOUNDFONT_NAME), for the owner's
    comparison by ear

What BN5's 0x15 does, read from its ROM with song_habits.py --detail
--theory and the bars' rhythms in aggregate (the bar line 24 ticks in:
the crash on one, the snare on two and four), numbers only:
- 148 beats a minute in F minor that seldom sits on its tonic (i 4% of
  its half bars): bVIIsus2 12, the Lydian #IVmaj7 9, IIm7b5 9, IIdim 7,
  bVII7 7, bVI 7, iv 7; major sevenths 15%, suspensions 17%; roots up a
  minor sixth 17%, a tone 16%, a fifth 14%, a fourth 12%.
- Square 2 the lead: a 50% pulse that strikes (envelope 0, 3, 11, 1),
  sixteenths, repeats 37%, steps 29%, skips 16%; its commonest bars runs
  of four into the beat (3 4 5 6 11 12 13 14), long notes held between.
- Square 1 the backing: 75% and 50%, sixteenths through the bar (some
  bars resting on 6, 7, 14 and 15), skips 32%, steps 25%, thrown hard
  left and right about every five notes.
- The wave the bass: plucked (0, 5, 4, 1), sixteenths nearly all bar,
  steps 35%, repeats 27%, leaps 18%, bent 102 times (falls).
- Direct Sound: a short stab at one pitch (86% repeated), struck on 1 4 5
  6 9 12 13 14 of the bar, panned to 40 and 88 (BN5 swings its parts
  wide, not hard).
- The kit: the kick on 0, 2, 6, 8, 10 (14), the snare on 4, 7, 9 and 12,
  an open hat on every off-beat (2, 6, 10, 14), closed hats between, the
  crash on one.

Here, in F minor at 150:
- Bar 0 the count-in: the stab's bar alone on the palm-muted guitar,
  swung left and right, the hats rising, the lead's pickup.
- Bars 1-4 the A: the hook (a held D-flat, a C struck three times falling
  to the held A-flat, then B-flat and C struck twice each) over Dbmaj7 Ebsus2, its answer over Gm7b5
  Eb7 in another rhythm, the hook again, the Lydian turn over Bmaj7 and
  Eb; the strings a chord a half bar, the stab under them, the bass's
  sixteenths falling at each bar's end.
- Bars 5-6 the B: the melody to the sampled saw, one note repeated a
  half bar, over Bbm7 Eb7 | Dbmaj7 Csus4-C (the major V once); brass on the
  turnaround.
- Bar 7 the hook back over Dbmaj7 Ebsus2, the saw an octave under it;
  bar 8 F minor with its ninth, ringing: bVI, bVII, i.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from trailer_music import Score, key, keys  # noqa: E402
from trailers import sketch_vb_lib as L  # noqa: E402
from trailers.sketch_vb_lib import (CHORD_SAMPLES, DS_ENV, DS_GAIN, DS_LEVEL, HARDWARE, MIX, REVERB,  # noqa: E402,F401
                                    SAMPLED_LEADS, SOUNDFONT)

BPM = 150
BARS = 9
TAIL = 1.8
BANNER = (1 * 4 * 60 / BPM, 2.9)


def instruments():
    return L.instruments('F2')


LEAD = dict(duty=0.5, env=(0, 3, 11, 1), vib=(24, 14), pan=0.0)
ARP = dict(duty=0.75, env=(0, 1, 11, 1))
BASS = dict(env=(0, 5, 4, 1), wave=L.SAW)
HELD = dict(env=(0, 0, 15, 3), wave=L.SAW)

# (bass root, the backing's four notes, the chord sample and its root, the stab's note)
H = {
    'Dbmaj7': ('Db3', 'Ab4 C5 Db5 F5', 'pad_m', 'F3', 'F3'),        # Fm over Db
    'Ebsus2': ('Eb3', 'Bb4 Eb5 F5 Bb5', 'pad', 'Eb3', 'F3'),
    'Eb': ('Eb3', 'Bb4 Eb5 G5 Bb5', 'pad_M', 'Eb3', 'G3'),
    'Gm7b5': ('G2', 'G4 Bb4 Db5 F5', 'pad_m', 'Bb3', 'F3'),         # Bbm over G
    'Eb7': ('Eb3', 'G4 Bb4 Db5 Eb5', 'pad_d', 'G3', 'F3'),          # Gdim over Eb
    'Bmaj7': ('B2', 'A#4 B4 D#5 F#5', 'pad_m', 'D#3', 'F#3'),       # D#m over B: the Lydian #IV
    'Bbm7': ('Bb2', 'Ab4 Bb4 Db5 F5', 'pad_M', 'Db3', 'F3'),        # Db over Bb
    'Csus4': ('C3', 'G4 C5 F5 G5', 'pad', 'F3', 'G3'),              # F G C over C
    'C': ('C3', 'G4 C5 E5 G5', 'pad_M', 'C3', 'G3'),
    'Fm': ('F2', 'Ab4 C5 F5 Ab5', 'pad_m', 'F3', 'F3'),
}
CHORDS = {0: ('Fm',), 1: ('Dbmaj7', 'Ebsus2'), 2: ('Gm7b5', 'Eb7'), 3: ('Dbmaj7', 'Ebsus2'), 4: ('Bmaj7', 'Eb'),
          5: ('Bbm7', 'Eb7'), 6: ('Dbmaj7', 'Csus4', 'C'), 7: ('Dbmaj7', 'Ebsus2'), 8: ('Fm',)}

HOOK = [(0, 18, 'Db5'), (18, 6, 'C5'), (24, 6, 'C5'), (30, 6, 'C5'), (36, 30, 'Ab4'), (66, 6, 'Bb4'), (72, 6, 'Bb4'),
        (78, 6, 'C5'), (84, 12, 'C5')]
LINES = {
    1: HOOK,
    2: [(0, 12, 'Db5'), (12, 12, 'Db5'), (24, 6, 'Bb4'), (30, 6, 'Bb4'), (36, 24, 'G4'), (60, 6, 'G4'), (66, 6, 'Ab4'),
        (72, 12, 'Bb4'), (84, 12, 'Bb4')],
    3: HOOK,
    4: [(0, 24, 'D#5'), (24, 6, 'D#5'), (30, 6, 'D#5'), (36, 24, 'F#5'), (60, 12, 'G5'), (72, 12, 'G5'), (84, 12, 'Eb5')],
    7: HOOK,
}
SAWLINE = {5: [(0, 12, 'F5'), (12, 12, 'F5'), (24, 24, 'F5'), (48, 12, 'Db5'), (60, 12, 'Db5'), (72, 24, 'Db5')],
           6: [(0, 12, 'C5'), (12, 12, 'C5'), (24, 24, 'C5'), (48, 24, 'C5'), (72, 24, 'E5')]}

FIG = (0, 1, 2, 1, 3, 2, 1, 2)
STAB = (1, 4, 5, 6, 9, 12, 13, 14)


def backing(s, bar, t0, t1, name, rest=False, vel=0):
    v = keys(H[name][1])
    for t in range(t0, t1, 6):
        p = t // 6
        if rest and p % 8 in (6, 7):
            continue
        side = -0.9 if (p // 4) % 2 == 0 else 0.9
        s.note('sq2', bar, t, 6, v[FIG[p % 8]], max(30, (100, 52, 92, 52)[p % 4] + vel), pan=side, **ARP)


def bass(s, bar, names):
    """sixteenths: the root struck again, its octave, a step down, the
    fifth, falling into the next bar"""
    for t0, n, name in L.halves(names):
        r = key(H[name][0])
        pat = (0, 0, 12, 12, 10, 10, 7, 5)[:n // 6]
        for i, up in enumerate(pat):
            t = t0 + 6 * i
            last = t + 6 >= 96
            voice = dict(BASS, bend=[(0, 0), (3, 0), (5, -2)]) if last else BASS
            s.note('wave', bar, t, 5, r + up, 127 if i % 2 == 0 else 84, **voice)


def chords(s, bar, names, vel=94):
    """the strings, one chord a half bar, held under the stab"""
    for t0, n, name in L.halves(names):
        pad, root = H[name][2], H[name][3]
        s.hit(pad, bar, t0, n - 2, key(root) + 12, vel, -0.3 if t0 < 48 else 0.3)


def stabs(s, bar, names, vel=92):
    """BN5's stab bar on the palm-muted guitar, at one pitch, swung wide"""
    for i, p in enumerate(STAB):
        name = next(nm for t0, n, nm in L.halves(names) if t0 <= 6 * p < t0 + n)
        s.hit('mute', bar, 6 * p, 5, L.low(H[name][4]) + 12, vel if p % 4 == 0 else vel - 18, -0.35 if i % 2 == 0 else 0.35)


def kit(s, bar, crash=False, until=16):
    for p in (0, 2, 6, 8, 10):
        if p < until:
            s.hit('kick', bar, 6 * p, 6, vel=104 if p in (0, 8) else 90)
    for p in (4, 7, 9, 12):
        if p < until:
            s.hit('snare', bar, 6 * p, 6, vel=104 if p in (4, 12) else 70, pan=0.05)
    L.hats(s, bar, opens=(2, 6, 10, 14), until=until, levels=(76, 52, 60, 52))
    if crash:
        s.note('fx', bar, 0, 48, 0, 116, **L.CRASH)


def lead(s, bar, line, vel=116):
    for t, n, k in line:
        s.note('sq1', bar, t, n, k, vel - (12 if n <= 6 and t % 12 else 0), **LEAD)


def arrangement():
    s = Score()
    # bar 0: the stab's bar alone, the hats rising, the pickup
    stabs(s, 0, CHORDS[0], vel=84)
    for p in range(16):
        s.note('noise', 0, 6 * p, 3, 0, 28 + 4 * p, **L.HAT)
    s.note('fx', 0, 0, 92, 0, 100, sweep=(1500, 120000), ramp=(1, 8))
    s.note('sq1', 0, 84, 6, 'C5', 104, **LEAD)
    s.note('sq1', 0, 90, 6, 'C5', 110, **LEAD)
    # bars 1-4: the A
    for bar in range(1, 5):
        names = CHORDS[bar]
        for t0, n, name in L.halves(names):
            backing(s, bar, t0, t0 + n, name, rest=bar % 2 == 0)
        chords(s, bar, names)
        bass(s, bar, names)
        stabs(s, bar, names)
        lead(s, bar, LINES[bar])
        kit(s, bar, crash=bar == 1, until=14 if bar == 4 else 16)
    L.power(s, 1, 0, 30, 'Db3', vel=112)
    s.hit('tom_mid', 4, 84, 6, vel=108, pan=0.2)
    s.hit('tom_lo', 4, 90, 6, vel=112, pan=0.2)
    # bars 5-6: the B, the saw's melody
    for bar in (5, 6):
        names = CHORDS[bar]
        for t0, n, name in L.halves(names):
            backing(s, bar, t0, t0 + n, name)
        chords(s, bar, names, vel=100)
        bass(s, bar, names)
        stabs(s, bar, names, vel=84)
        for t, n, k in SAWLINE[bar]:
            s.hit('dlead', bar, t, n - 1, k, 112, -0.25)
            s.hit('dlead', bar, t + 3, n - 1, k, 64, 0.4)
        kit(s, bar, crash=bar == 5, until=12 if bar == 6 else 16)
    L.power(s, 5, 0, 30, 'Bb2', vel=116)
    for t in (72, 78, 84):
        s.hit('brass', 6, t, 5, 'C4', 104 - (t - 72), 0.3)
    L.toms(s, 6)
    # bar 7: the hook back, the saw under it
    for t0, n, name in L.halves(CHORDS[7]):
        backing(s, 7, t0, t0 + n, name, vel=8)
    chords(s, 7, CHORDS[7], vel=100)
    bass(s, 7, CHORDS[7])
    stabs(s, 7, CHORDS[7])
    lead(s, 7, LINES[7], vel=120)
    for t, n, k in LINES[7]:
        s.hit('double', 7, t, n - 1, key(k) - 12, 92, 0.25)
    kit(s, 7, crash=True, until=12)
    L.power(s, 7, 0, 30, 'Db3', vel=118)
    L.toms(s, 7)
    # bar 8: F minor with its ninth, ringing
    s.note('fx', 8, 0, 96, 0, 124, **L.CRASH)
    s.hit('kick', 8, 0, 6)
    s.hit('snare', 8, 0, 6)
    L.power(s, 8, 0, 90, 'F2', vel=122, fall=True)
    s.hit('pad_m', 8, 0, 150, 'F3', 108, -0.35)        # F Ab C
    s.hit('pad', 8, 0, 150, 'F3', 92, 0.35)            # F G C: the ninth
    s.note('wave', 8, 0, 92, 'F2', 127, **dict(HELD, bend=[(0, 0), (54, 0), (92, -12)]))
    s.note('sq1', 8, 0, 96, 'C5', 118, **dict(LEAD, bend=[(0, 0), (76, 0), (96, -2)]))
    return s
