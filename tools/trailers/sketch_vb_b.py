"""Sketch B for 0.16.0, the dark board's team: nine bars for the owner's
ear, not a cut. Original, in the manner of BN6's "Showdown" (song 0x22),
a battle theme of diminished chords and a broken beat, for the Crosses
fighting on a dark layer; none of its notes, nor its chords in their
order. Played on the approved palette's samples (sketch_vb_lib.py).

    tools/trailer_music.py's render_any(OUT, trailers.sketch_vb_b), in
    docker/Dockerfile.music's image; on 26 October 2026 also through
    GeneralUser GS and MuseScore General (CW_SOUNDFONT_NAME), for the owner's
    comparison by ear

What 0x22 does, read from the ROM with song_habits.py --detail --theory
--offset 12 and the bars' rhythms in aggregate, numbers only:
- 148 beats a minute in A-sharp (B-flat) minor, dark: i 24% of its half
  bars, the diminished VI 14 and I 9 (diminished chords 23% in all),
  bVIIsus2 12, bVI 10, the minor v 5; roots down a tone 25%, down a
  semitone 21%, up a tone 21%; 1.74 changes a bar.
- Square 1 the backing: a thin 12.5% pulse held at 13, struck in pairs of
  sixteenths with a rest between (its commonest bar 0 1 3 4 6 7 9 10 12 13
  15), leaping by octaves and skips (big leaps 49%, skips 31%), thrown
  hard left and right about every four notes; a 75% voice repeating
  32nds on one note in other sections.
- Square 2 the lead: a 50% voice of long notes (median a dotted eighth,
  struck on the beat and its eighth), steps 41%, repeats 20%, its
  vibrato after 14 ticks.
- The wave the bass: straight sixteenths at one velocity, every bar.
- The kit, a broken beat: the kick on 0, 3, 6, 8 (14 and 15 into a
  section), the snare on 4, 7, 10 and 12, an open hat on 4 and 12,
  closed hats between, a crash on one; toms only into a section.
- Direct Sound: chord samples twice a bar (0 and 8), a pad of slow
  attack, a low dark stab on 14 and 15 now and then.

Here, in B-flat minor at 150 (the trailer's bar of 48 frames):
- Bar 0 the count-in: the paired backing alone over Bbm, the kick's
  broken beat, a riser, the strings swelling.
- Bars 1-4 the A: the lead's long notes over Bbm Gdim | Absus2 Gb | Bbm
  Bbdim | Gb Fm, the strings on each half (the diminished ones a chord
  sample of their own), the bass's sixteenths, the broken beat, the
  guitar's low stab on 14 and 15 into bars 3 and 5.
- Bars 5-6 the B: the backing to the 75% voice repeating 32nds, the
  melody's long notes on the sampled saw, Gb Absus2 | Ebm Fm, brass on
  the turnaround.
- Bar 7 the A's first bar back, the saw an octave under the square.
- Bar 8 Gdim falling onto B-flat minor with its ninth, ringing.
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
    return L.instruments('Bb1')


LEAD = dict(duty=0.5, env=(0, 0, 10, 4), vib=(26, 14), pan=0.0)
THIN = dict(duty=0.125, env=(0, 0, 13, 2))
REP = dict(duty=0.75, env=(0, 0, 13, 2))
BASS = dict(env=(0, 0, 13, 1), wave=L.SQUARE)
HELD = dict(env=(0, 0, 15, 3), wave=L.SQUARE)

# (bass root, the backing's tones low to high, the chord sample and its root)
H = {
    'Bbm': ('Bb2', 'Bb3 Db4 F4 Bb4 Db5 F5', 'pad_m', 'Bb2'),
    'Gdim': ('G2', 'G3 Bb3 Db4 G4 Bb4 Db5', 'pad_d', 'G2'),
    'Absus2': ('Ab2', 'Ab3 Bb3 Eb4 Ab4 Bb4 Eb5', 'pad', 'Ab2'),
    'Gb': ('Gb2', 'Gb3 Bb3 Db4 Gb4 Bb4 Db5', 'pad_M', 'Gb2'),
    'Bbdim': ('Bb2', 'Bb3 Db4 E4 Bb4 Db5 E5', 'pad_d', 'Bb2'),
    'Fm': ('F2', 'F3 Ab3 C4 F4 Ab4 C5', 'pad_m', 'F2'),
    'Ebm': ('Eb3', 'Eb4 Gb4 Bb4 Eb5 Gb5 Bb5', 'pad_m', 'Eb3'),
}
CHORDS = {0: ('Bbm',), 1: ('Bbm', 'Gdim'), 2: ('Absus2', 'Gb'), 3: ('Bbm', 'Bbdim'), 4: ('Gb', 'Fm'),
          5: ('Gb', 'Absus2'), 6: ('Ebm', 'Fm'), 7: ('Bbm', 'Gdim'), 8: ('Bbm',)}

LINES = {
    1: [(0, 24, 'F5'), (24, 6, 'F5'), (30, 6, 'Gb5'), (36, 12, 'F5'), (48, 36, 'Db6'), (84, 12, 'Bb5')],
    2: [(0, 36, 'Bb5'), (36, 6, 'Bb5'), (42, 6, 'Ab5'), (48, 48, 'Gb5')],
    3: [(0, 24, 'F5'), (24, 6, 'F5'), (30, 6, 'Gb5'), (36, 12, 'F5'), (48, 36, 'E5'), (84, 12, 'Bb5')],
    4: [(0, 24, 'Db6'), (24, 12, 'Db6'), (36, 12, 'Bb5'), (48, 48, 'C6')],
}
SAWLINE = {5: [(0, 36, 'Db6'), (36, 12, 'Db6'), (48, 36, 'Eb6'), (84, 12, 'Eb6')],
           6: [(0, 36, 'Gb5'), (36, 12, 'Gb5'), (48, 48, 'F5')]}

PAIRS = (0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15)
FIG = (0, 3, 3, 2, 1, 1, 2, 3, 3, 4, 3)       # tones by onset: an octave up, skips back, a note struck again


def backing(s, bar, t0, t1, name, vel=100):
    """the thin pulse in pairs, leaping, thrown left and right"""
    v = keys(H[name][1])
    for i, p in enumerate(PAIRS):
        t = 6 * p
        if not t0 <= t < t1:
            continue
        side = -0.9 if (i // 4) % 2 == 0 else 0.9
        s.note('sq2', bar, t, 5, v[FIG[i]], vel if p % 3 == 0 else vel - 36, pan=side, **THIN)


def repeats(s, bar, t0, t1, name):
    """the 75% voice repeating 32nds on one of the chord's notes a beat"""
    v = keys(H[name][1])
    for t in range(t0, t1, 3):
        k = v[(3, 4, 3, 2)[(t // 24) % 4]]
        s.note('sq2', bar, t, 3, k, 96 if t % 6 == 0 else 52, pan=-0.9 if (t // 24) % 2 == 0 else 0.9, **REP)


def bass(s, bar, names, vel=120):
    for t0, n, name in L.halves(names):
        r = key(H[name][0])
        for t in range(t0, t0 + n, 6):
            up = 12 if (t // 6) % 4 == 2 else 0
            s.note('wave', bar, t, 5, r + up, vel, **BASS)


def chords(s, bar, names, vel=96):
    for t0, n, name in L.halves(names):
        _, _, pad, root = H[name]
        s.hit(pad, bar, t0, n - 2, key(root) + 12, vel, -0.35 if t0 < 48 else 0.35)


def kit(s, bar, crash=False, until=16):
    """0x22's broken beat"""
    for p in (0, 3, 6, 8):
        if p < until:
            s.hit('kick', bar, 6 * p, 6, vel=104 if p in (0, 8) else 92)
    for p in (4, 7, 10, 12):
        if p < until:
            s.hit('snare', bar, 6 * p, 6, vel=104 if p in (4, 12) else 80, pan=0.05)
    L.hats(s, bar, opens=(4, 12), until=until, levels=(80, 44, 64, 44))
    if crash:
        s.note('fx', bar, 0, 48, 0, 116, **L.CRASH)


def stab(s, bar, name):
    """the low dark stab on 14 and 15, into the next bar"""
    L.power(s, bar, 84, 5, H[name][0], vel=104)
    L.power(s, bar, 90, 6, H[name][0], vel=112)


def lead(s, bar, line, vel=116):
    for t, n, k in line:
        s.note('sq1', bar, t, n, k, vel - (12 if n <= 6 and t % 12 else 0), **LEAD)


def arrangement():
    s = Score()
    # bar 0: the paired backing alone, the kick's broken beat, the strings swelling
    backing(s, 0, 0, 96, 'Bbm', vel=84)
    for p in (0, 3, 6, 8):
        s.hit('kick', 0, 6 * p, 6, vel=86)
    s.hit('pad_m', 0, 0, 94, 'Bb3', 80, 0.0)
    s.note('fx', 0, 0, 92, 0, 100, sweep=(1500, 120000), ramp=(1, 8))
    for p in range(8, 16):
        s.note('noise', 0, 6 * p, 3, 0, 30 + 6 * (p - 8), **L.HAT)
    stab(s, 0, 'Bbm')
    # bars 1-4: the A
    for bar in range(1, 5):
        names = CHORDS[bar]
        for t0, n, name in L.halves(names):
            backing(s, bar, t0, t0 + n, name)
        chords(s, bar, names)
        bass(s, bar, names)
        lead(s, bar, LINES[bar])
        kit(s, bar, crash=bar == 1, until=14 if bar == 4 else 16)
    stab(s, 2, 'Gb')
    stab(s, 4, 'Fm')
    L.power(s, 1, 0, 30, 'Bb2', vel=118)
    s.hit('tom_mid', 4, 84, 6, vel=108, pan=0.2)
    s.hit('tom_lo', 4, 90, 6, vel=112, pan=0.2)
    # bars 5-6: the B, the saw's long notes, the repeating 32nds
    for bar in (5, 6):
        names = CHORDS[bar]
        for t0, n, name in L.halves(names):
            repeats(s, bar, t0, t0 + n, name)
        chords(s, bar, names, vel=102)
        bass(s, bar, names)
        for t, n, k in SAWLINE[bar]:
            s.hit('dlead', bar, t, n - 1, k, 112, -0.25)
            s.hit('dlead', bar, t + 3, n - 1, k, 64, 0.4)
        kit(s, bar, crash=bar == 5, until=12 if bar == 6 else 16)
    L.power(s, 5, 0, 30, 'Gb2', vel=116)
    for t in (72, 78, 84):
        s.hit('brass', 6, t, 5, 'Ab3', 104 - (t - 72), 0.3)       # Ab C Eb over F: Fm7
    L.toms(s, 6)
    # bar 7: the A's first bar, the saw under it
    for t0, n, name in L.halves(CHORDS[7]):
        backing(s, 7, t0, t0 + n, name, vel=108)
    chords(s, 7, CHORDS[7], vel=102)
    bass(s, 7, CHORDS[7])
    lead(s, 7, LINES[1], vel=120)
    for t, n, k in LINES[1]:
        s.hit('double', 7, t, n - 1, key(k) - 12, 92, 0.25)
    kit(s, 7, crash=True, until=12)
    L.power(s, 7, 0, 30, 'Bb2', vel=118)
    L.toms(s, 7)
    # bar 8: B-flat minor with its ninth, ringing
    s.note('fx', 8, 0, 96, 0, 124, **L.CRASH)
    s.hit('kick', 8, 0, 6)
    s.hit('snare', 8, 0, 6)
    L.power(s, 8, 0, 90, 'Bb2', vel=122, fall=True)
    s.hit('pad_m', 8, 0, 150, 'Bb3', 108, -0.35)       # Bb Db F
    s.hit('pad', 8, 0, 150, 'Bb2', 92, 0.35)           # Bb C F: the ninth
    s.note('wave', 8, 0, 92, 'Bb2', 127, **dict(HELD, bend=[(0, 0), (54, 0), (92, -12)]))
    s.note('sq1', 8, 0, 96, 'F5', 118, **dict(LEAD, bend=[(0, 0), (76, 0), (96, -2)]))
    return s
