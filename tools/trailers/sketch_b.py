"""Sketch B, the Net Square's playful bounce: an eight-bar sketch for the
owner to judge by ear, not a cut. Original, in the spirit of the music the
Square plays, BN6's Navi Customizer theme (song 0x1E), by the rules of
.claude/skills/trailer/music_rules.md; none of its notes or chords.

    python3 tools/trailer_music.py OUT.wav --piece sketch_b

What the Navi Customizer theme does, read from the ROM (song_habits.py
--detail --theory): 160 beats a minute in a major key with the
Mixolydian's flat seventh (I7, bVII) and the borrowed minor iv;
suspensions (sus2 and sus4 a third of its chords) and major sevenths;
roots moving by fourths and fifths more than half the time, two chords a
bar. The first square ping-pongs hard left and right in sixteenths,
accented loud-soft; the lead plays short notes (sixteenths and eighths,
a quarter of them gated under half); the wave's bass is staccato, half
its notes cut short, with slides (a bend range of four) and vibrato. The
kit is busy and Latin-tinged: the kick on one and syncopated after it,
claps and rim on and around two and four, open hats on the off-beats,
two pitched hand drums answering each other late in the bar.

Here, at 160 (a bar is 45 frames at 30 fps) in E-flat major, the Square
as a place where people talk: the square calls, an electric piano's bell
answers, and at the return they speak together in thirds. The hook skips:
a rising arpeggio of three notes, a turn, a fall, every note shorter
than its step. Chords on the off-beats as one short chord sample (a
skank), the bass bouncing root-octave with slides up into its octaves,
conga answers in the bar's second half.

The harmony, two chords a bar: I IVmaj7 | I7 iv6 | iii7 vi7 | ii7
V7sus4-V7 | I IVmaj7 | bVIIsus2 IV | iii7 bIIImaj7 (the chromatic
mediant, the fake-out) | ii7 V7sus4-V7 | I6/9. The fifths of iii vi ii V
walk home; the minor iv (its C-flat) is the wink.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import trailer_music as tm  # noqa: E402
from trailer_music import Score, key, keys  # noqa: E402

BPM = 160
BARS = 9
TAIL = 1.3
MIX = (('lead', ('sq1',), 0.14), ('arp', ('sq2',), 0.12), ('bass', ('wave',), 0.21),
       ('hats', ('noise',), 0.14), ('fx', ('fx',), 0.08))
DS_LEVEL = 1.0
REVERB = 40
HARDWARE = 32768
CHORD_SAMPLES = {'skank': (0, 4, 7, 12), 'skank_m': (0, 3, 7, 12), 'pad': (0, 4, 7, 12), 'pad_m': (0, 3, 7, 12)}
BANNER = (8 * 4 * 60 / BPM - 1.2, 2.9)

DS_ENV = {'kick': (255, 250, 190, 208), 'clap': (255, 250, 190, 208), 'rim': (255, 0, 255, 200),
          'skank': (255, 215, 0, 170), 'skank_m': (255, 215, 0, 170), 'pad': (30, 0, 200, 220),
          'pad_m': (30, 0, 200, 220), 'bell': (255, 0, 255, 230),
          'conga_hi': (255, 0, 255, 220), 'conga_lo': (255, 0, 255, 220)}
DS_GAIN = {'kick': 0.9, 'clap': 0.7, 'rim': 0.42, 'skank': 0.56, 'skank_m': 0.56, 'pad': 0.3, 'pad_m': 0.3,
           'bell': 0.5, 'conga_hi': 0.6,
           'conga_lo': 0.62}

LEAD = dict(duty=0.25, env=(0, 0, 12, 2), vib=(18, 12), pan=-0.3)
ARP = dict(duty=0.5, env=(0, 2, 6, 1))
SAW = [round(15 * (31 - i) / 31) for i in range(32)]
BASS = dict(env=(0, 0, 15, 1), wave=SAW)
HAT = dict(clock=262144, env=(0, 0, 10, 0), pan=-0.25)
OPEN = dict(clock=262144, env=(0, 2, 10, 3), pan=-0.3)
CRASH = dict(clock=80000, env=(0, 7, 0, 1), pan=0.2)


def instruments():
    return {
        'kick': tm.punch_kick(seconds=0.24, low=62, high=140, drive=3.5, body=0.09, hold=0.1),
        'clap': tm.punch_clap(seed=12, drive=2.0, body=230),
        'rim': tm.conga(root=key('E6'), seconds=0.05, slap=1.2, seed=9),
        'skank': tm.chord_pad(CHORD_SAMPLES['skank'], attack=0.004, bright=2600),
        'skank_m': tm.chord_pad(CHORD_SAMPLES['skank_m'], attack=0.004, bright=2600),
        'pad': tm.chord_pad(CHORD_SAMPLES['pad'], bright=1200),
        'pad_m': tm.chord_pad(CHORD_SAMPLES['pad_m'], bright=1200),
        'bell': tm.fm_bell(root=key('C5')),
        'conga_hi': tm.conga(root=key('D4')),
        'conga_lo': tm.conga(root=key('G3'), seed=10),
    }


# (bass root, arpeggio's notes, the skank's root: one major triad sample
# over the bass makes the seventh and ninth chords, as Kaida's did)
H = {
    'Eb': ('Eb2', 'Eb4 G4 Bb4 D5 F5', 'Eb4'),
    'Abmaj7': ('Ab2', 'Ab3 C4 Eb4 G4 C5', 'Cm4'),      # (C minor over Ab: Abmaj7)
    'Eb7': ('Eb2', 'Eb4 G4 Bb4 Db5 F5', 'Eb4'),
    'Abm6': ('Ab2', 'Ab3 Cb4 Eb4 F4 Ab4', 'Cb4'),     # (C-flat over Ab: Abm7)
    'Gm7': ('G2', 'G3 Bb3 D4 F4 G4', 'Bb3'),           # (Bb over G: Gm7)
    'Cm7': ('C3', 'C4 Eb4 G4 Bb4 C5', 'Eb4'),          # (Eb over C: Cm7)
    'Fm7': ('F2', 'F3 Ab3 C4 Eb4 F4', 'Ab3'),          # (Ab over F: Fm7)
    'Bb7sus': ('Bb2', 'Bb3 Eb4 F4 Ab4 Bb4', 'Ab3'),    # (Ab over Bb: Bb11)
    'Bb7': ('Bb2', 'Bb3 D4 F4 Ab4 Bb4', 'Bb3'),
    'Dbsus2': ('Db3', 'Db4 Eb4 Ab4 Db5 Eb5', 'Db4'),
    'Ab': ('Ab2', 'Ab3 C4 Eb4 Ab4 C5', 'Ab3'),
    'Gbmaj7': ('Gb2', 'Gb3 Bb3 Db4 F4 Gb4', 'Bbm3'),   # (B-flat minor over Gb: Gbmaj7)
}
CHORDS = [('Eb', 'Abmaj7'), ('Eb7', 'Abm6'), ('Gm7', 'Cm7'), ('Fm7', 'Bb7sus', 'Bb7'), ('Eb', 'Abmaj7'),
          ('Dbsus2', 'Ab'), ('Gm7', 'Gbmaj7'), ('Fm7', 'Bb7sus', 'Bb7')]

# the square's calls and the bell's answers: (tick, ticks, note)
HOOK = [(0, 5, 'G5'), (6, 5, 'Bb5'), (12, 9, 'Eb6'), (24, 5, 'D6'), (30, 5, 'Eb6'), (36, 9, 'Bb5'), (48, 5, 'C6'),
        (54, 5, 'Bb5'), (60, 9, 'G5'), (72, 5, 'Ab5'), (78, 5, 'G5'), (84, 9, 'Eb5')]
SQUARE = {
    0: HOOK,
    2: [(0, 5, 'D5'), (6, 5, 'F5'), (12, 9, 'Bb5'), (24, 5, 'G5'), (30, 5, 'Bb5'), (36, 9, 'F5'), (48, 5, 'G5'),
        (54, 5, 'Eb5'), (60, 9, 'C5'), (72, 5, 'D5'), (78, 5, 'Eb5'), (84, 9, 'G5')],
    4: HOOK,
    5: [(0, 5, 'Ab5'), (6, 5, 'Bb5'), (12, 9, 'Eb6'), (24, 5, 'F6'), (30, 5, 'Eb6'), (36, 9, 'Db6'), (48, 5, 'Eb6'),
        (54, 5, 'C6'), (60, 9, 'Ab5'), (72, 5, 'Bb5'), (78, 5, 'C6'), (84, 9, 'Eb6')],
    6: [(0, 5, 'D6'), (6, 5, 'Eb6'), (12, 9, 'F6'), (24, 5, 'G6'), (30, 5, 'F6'), (36, 9, 'D6'), (48, 5, 'Db6'),
        (54, 5, 'F6'), (60, 9, 'Bb5'), (72, 5, 'Db6'), (78, 5, 'F6'), (84, 12, 'Gb6')],
    7: [(0, 5, 'F6'), (6, 5, 'Eb6'), (12, 9, 'C6'), (24, 5, 'Ab5'), (30, 5, 'C6'), (36, 9, 'Eb6'), (48, 10, 'Eb6'),
        (60, 10, 'D6'), (72, 5, 'F6'), (78, 5, 'D6'), (84, 5, 'Bb5'), (90, 5, 'D6')],
}
BELL = {
    1: [(0, 5, 'Db6'), (6, 5, 'C6'), (12, 9, 'Bb5'), (24, 12, 'G5'), (36, 9, 'Bb5'), (48, 5, 'B5'), (54, 5, 'Bb5'),
        (60, 9, 'Ab5'), (72, 18, 'F5')],
    3: [(0, 5, 'Ab5'), (6, 5, 'G5'), (12, 9, 'F5'), (24, 12, 'C6'), (36, 9, 'Ab5'), (48, 10, 'Eb6'), (60, 10, 'D6'),
        (72, 5, 'C6'), (78, 5, 'Bb5'), (84, 9, 'F5')],
}


def call(s, bar, line, octave=0, vel=116):
    for t, n, k in line:
        o = dict(LEAD)
        if n >= 9 and t % 24 == 12:
            o['scoop'] = 1.0          # the skip's long note slides in from under
        s.note('sq1', bar, t, n, key(k) + octave, vel - (12 if t % 12 else 0), **o)


def answer(s, bar, line, transpose=0, vel=112, pan=0.45):
    for t, n, k in line:
        s.hit('bell', bar, t, n + 6, key(k) + transpose, vel - (14 if t % 12 else 0), pan)


def arp(s, bar, t0, t1, notes, vel=92, soft=40):
    """Sixteenths up and down the chord, accented loud-soft, thrown hard
    left and right."""
    v = keys(notes)
    order = [0, 2, 1, 3, 2, 4, 3, 1]
    for i, t in enumerate(range(t0, t1, 6)):
        side = -0.9 if (t // 6) % 2 == 0 else 0.9
        s.note('sq2', bar, t, 5, v[order[i % len(order)]], vel if (t // 6) % 2 == 0 else soft, pan=side, **ARP)


def bass(s, bar, t0, t1, root):
    """Bouncing root and octave, the octaves slid into from a tone under."""
    r = key(root)
    for p, n, tone in ((0, 9, 0), (3, 3, 12), (5, 3, 0), (6, 4, 12)):
        t = t0 + 6 * p
        if t + n > t1:
            break
        o = dict(BASS)
        if tone and p == 6:
            o['bend'] = [(0, -2), (3, 0)]
        s.note('wave', bar, t, n, r + tone, 124 if tone == 0 else 108, **o)


def skank(s, bar, t0, t1, root):
    """The chord on the off-beats, short ('Cm4': the minor triad's)."""
    inst = 'skank_m' if root[-2] == 'm' else 'skank'
    root = root.replace('m', '') if inst == 'skank_m' else root
    for t in range(t0 + 12, t1, 24):
        s.hit(inst, bar, t, 5, root, 104, 0.7 if (t // 24) % 2 else -0.7)


KICK = (0, 7, 10)
CLAP = (4, 12)


def drums(s, bar, last=False, crash=False):
    for p in KICK + ((14,) if bar % 2 else ()):
        s.hit('kick', bar, 6 * p, 6, vel=120)
    for p in CLAP:
        s.hit('clap', bar, 6 * p, 6, vel=112, pan=0.1)
    for p in (3, 15) if not last else (3,):
        s.hit('rim', bar, 6 * p, 3, key('E6'), 96, -0.4)
    for p in range(16):
        if p in (6, 14):
            s.note('noise', bar, 6 * p, 6, 0, 92, **OPEN)
        else:
            s.note('noise', bar, 6 * p, 2, 0, (104, 44, 80, 44)[p % 4], **HAT)
    congas = ((9, 'conga_hi'), (11, 'conga_hi'), (13, 'conga_lo')) if bar % 2 else ((10, 'conga_lo'), (13, 'conga_hi'))
    for p, drum in congas:
        high = drum == 'conga_hi'
        s.hit(drum, bar, 6 * p, 4, key('D4') if high else key('G3'), 104, 0.6 if high else -0.2)
    if crash:
        s.note('fx', bar, 0, 48, 0, 116, **CRASH)


def harmony(s, bar):
    names = CHORDS[bar]
    spans = [(0, 48), (48, 96)] if len(names) == 2 else [(0, 48), (48, 72), (72, 96)]
    for name, (t0, t1) in zip(names, spans):
        root, notes, sk = H[name]
        arp(s, bar, t0, t1, notes)
        bass(s, bar, t0, t1, root)
        skank(s, bar, t0, t1, sk)
        minor = sk[-2] == 'm'
        s.hit('pad_m' if minor else 'pad', bar, t0, t1 - t0 - 2, sk.replace('m', '') if minor else sk, 76, 0.0)


def arrangement():
    s = Score()
    for bar in range(8):
        harmony(s, bar)
        drums(s, bar, crash=bar in (0, 4))
        if bar in SQUARE:
            call(s, bar, SQUARE[bar])
        if bar in BELL:
            answer(s, bar, BELL[bar])
    # the return: the bell under the square in thirds, both talking at once
    answer(s, 4, HOOK, transpose=-4, vel=96, pan=-0.35)
    answer(s, 7, SQUARE[7], transpose=-3, vel=96, pan=-0.35)
    # bar 8: the six-nine chord, struck, then struck again on the and
    for t, vel in ((0, 112), (12, 124)):
        s.hit('kick', 8, t, 6, vel=vel)
        s.hit('clap', 8, t, 6, vel=vel)
        s.hit('skank', 8, t, 6 if t == 0 else 60, 'Eb4', vel, 0.0)
        s.hit('pad', 8, t, 6 if t == 0 else 90, 'Eb4', vel - 30, 0.0)
        s.hit('pad_m', 8, t, 6 if t == 0 else 90, 'C4', vel - 40, 0.0)
    s.note('fx', 8, 12, 60, 0, 120, **CRASH)
    s.note('sq1', 8, 0, 6, 'Eb6', 116, **LEAD)
    s.note('sq1', 8, 12, 72, 'Eb6', 120, **dict(LEAD, vib=(24, 14)))
    s.hit('bell', 8, 12, 72, 'G5', 110, 0.35)
    s.hit('bell', 8, 12, 72, 'C6', 100, -0.35)
    s.note('wave', 8, 0, 6, 'Eb2', 124, **BASS)
    s.note('wave', 8, 12, 60, 'Eb2', 124, **dict(BASS, env=(0, 0, 15, 3)))
    arp(s, 8, 12, 48, 'Eb5 G5 Bb5 C6 F6', vel=80, soft=36)
    return s
