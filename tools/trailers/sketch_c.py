"""Sketch C, a heroic opening (a net area's or the title's): an eight-bar
sketch for the owner to judge by ear, not a cut. Original, in the manner
of BN6's and BN5's title and net themes (BN6's 0x01 and 0x13, BN5's
0x01), by the rules of .claude/skills/trailer/music_rules.md; none of
their notes or chords.

    python3 tools/trailer_music.py OUT.wav --piece sketch_c

What those themes do, read from the ROM (song_habits.py --detail
--theory): 142 to 146 beats a minute; seventh and suspended chords far
more than plain triads (BN6's title: m7 a third, maj7 a quarter, sus2 a
seventh of its chords), two chords a bar, roots stepping down by a tone or
a semitone (IV iii ii) more than leaping; a major key lit by the Lydian's
raised fourth (BN5's title) and the borrowed bVI and bVII. Their drums are
a broken beat, not a march: the kick syncopated (the net theme's on one,
the and of one, the and of two, three and its and, the and of four), the
snare on two and four with ghosts around it, open hats on the off-beats,
a crash at the phrase heads, toms falling into a section.

Here, at 144 (a bar is 50 frames at 30 fps) in D major: a theme of open
intervals and long notes held with vibrato. The hook: a fourth up to a
long D, then a climb through E and F-sharp to the Lydian G-sharp over E/D,
the lift every BN opening has somewhere. Bars 4-7 state it again an
octave up, the second square under it an octave lower, the brass
answering.

The harmony, two chords a bar: Dmaj9 E/D | Gmaj9 F#m7 | Em7 Asus4-A |
Bbmaj9 Cmaj9 | Dmaj9 E/D | Gmaj9 F#m7 | Bm7 G | Bb C Dsus4-D | D. One
major triad of brass and one sus chord of strings make all of them over
the bass (F#m7 is A over F#, Gmaj9 D over G, Bbmaj9 F over Bb: Kaida's
way of getting many chords out of few samples). The borrowed bVI-bVII
into I at bar 3 and again at the end is the heroic lift.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import trailer_music as tm  # noqa: E402
from trailer_music import Score, key, keys  # noqa: E402
from trailers import v0_3_0_music as first  # noqa: E402

BPM = 144
BARS = 9
TAIL = 1.8
MIX = (('lead', ('sq1',), 0.12), ('second', ('sq2',), 0.1), ('bass', ('wave',), 0.2),
       ('hats', ('noise',), 0.11), ('fx', ('fx',), 0.09))
DS_LEVEL = 1.0
REVERB = 50
HARDWARE = 32768
CHORD_SAMPLES = {'brass': (0, 4, 7, 12), 'strings': (0, 4, 7, 12, 19)}
BANNER = (8 * 4 * 60 / BPM - 1.0, 2.9)

DS_ENV = {'kick': (255, 250, 190, 208), 'snare': (255, 250, 190, 208), 'brass': (255, 0, 230, 190),
          'strings': (20, 0, 230, 225), 'impact': (255, 0, 255, 225), 'tom_hi': (255, 0, 255, 230),
          'tom_mid': (255, 0, 255, 230), 'tom_lo': (255, 0, 255, 230)}
DS_GAIN = {'kick': 0.9, 'snare': 0.75, 'brass': 0.7, 'strings': 0.7, 'impact': 0.7, 'tom_hi': 0.6,
           'tom_mid': 0.6, 'tom_lo': 0.65}

LEAD = dict(duty=0.5, env=(0, 0, 13, 2), vib=(28, 14), pan=-0.25)
SECOND = dict(duty=0.25, env=(0, 0, 11, 2), vib=(22, 16), pan=0.55)
ARP = dict(duty=0.25, env=(0, 3, 5, 1))
SAW = [round(15 * (31 - i) / 31) for i in range(32)]
BASS = dict(env=(0, 0, 15, 1), wave=SAW)
HAT = dict(clock=262144, env=(0, 0, 10, 0), pan=0.25)
OPEN = dict(clock=262144, env=(0, 2, 10, 3), pan=0.3)
CRASH = dict(clock=80000, env=(0, 7, 0, 1), pan=-0.2)


def instruments():
    kit = first.instruments()
    S = {name: kit[name] for name in ('tom_hi', 'tom_mid', 'tom_lo')}
    S['kick'] = tm.punch_kick(low=70, high=150, drive=4.5, body=0.065)
    S['snare'] = tm.punch_clap(seed=21, drive=2.2, body=185)
    S['brass'] = tm.brass_section(CHORD_SAMPLES['brass'])
    S['strings'] = tm.chord_pad(CHORD_SAMPLES['strings'], attack=0.2, bright=1300, detune=2)
    S['impact'] = tm.impact(seed=22)
    return S


# (bass root, the brass's triad root over it, the arpeggio's notes)
H = {
    'Dmaj9': ('D2', 'A3', 'D4 A4 C#5 E5 F#5'),
    'E/D': ('D2', 'E4', 'D4 G#4 B4 E5 G#5'),
    'Gmaj9': ('G2', 'D4', 'G3 D4 F#4 A4 B4'),
    'F#m7': ('F#2', 'A3', 'F#3 C#4 E4 A4 C#5'),
    'Em7': ('E2', 'G3', 'E3 B3 D4 G4 B4'),
    'Asus4': ('A2', 'D4', 'A3 D4 E4 A4 D5'),
    'A': ('A2', 'A3', 'A3 C#4 E4 A4 C#5'),
    'Bbmaj9': ('Bb2', 'F4', 'Bb3 F4 A4 C5 D5'),
    'Cmaj9': ('C3', 'G3', 'C4 G4 B4 D5 E5'),
    'Bm7': ('B2', 'D4', 'B3 F#4 A4 D5 F#5'),
    'G': ('G2', 'G3', 'G3 B3 D4 G4 B4'),
    'Bb': ('Bb2', 'Bb3', 'Bb3 D4 F4 Bb4 D5'),
    'C': ('C3', 'C4', 'C4 E4 G4 C5 E5'),
    'Dsus4': ('D2', 'G3', 'D4 G4 A4 D5 G5'),
    'D': ('D2', 'D4', 'D4 F#4 A4 D5 F#5'),
}
CHORDS = [('Dmaj9', 'E/D'), ('Gmaj9', 'F#m7'), ('Em7', 'Asus4', 'A'), ('Bbmaj9', 'Cmaj9'), ('Dmaj9', 'E/D'),
          ('Gmaj9', 'F#m7'), ('Bm7', 'G'), ('Bb', 'C', 'Dsus4', 'D')]
SPANS = {2: [(0, 48), (48, 72), (72, 96)], 3: [(0, 24), (24, 48), (48, 72), (72, 96)]}

HOOK = [(0, 12, 'A4'), (12, 36, 'D5'), (48, 12, 'E5'), (60, 12, 'F#5'), (72, 24, 'G#5')]
LINES = {
    0: HOOK,
    1: [(0, 36, 'A5'), (36, 6, 'G5'), (42, 6, 'F#5'), (48, 24, 'E5'), (72, 12, 'F#5'), (84, 12, 'A5')],
    2: [(0, 24, 'B5'), (24, 12, 'A5'), (36, 12, 'G5'), (48, 24, 'D5'), (72, 24, 'C#5')],
    3: [(0, 12, 'D5'), (12, 12, 'F5'), (24, 24, 'A5'), (48, 12, 'G5'), (60, 36, 'D6')],
    4: [(t, n, k[:-1] + str(int(k[-1]) + 1)) for t, n, k in HOOK],
    5: [(0, 36, 'A6'), (36, 6, 'G6'), (42, 6, 'F#6'), (48, 24, 'E6'), (72, 12, 'D6'), (84, 12, 'E6')],
    6: [(0, 24, 'F#6'), (24, 12, 'E6'), (36, 12, 'D6'), (48, 24, 'B5'), (72, 24, 'D6')],
    7: [(0, 24, 'F6'), (24, 24, 'G6'), (48, 24, 'G6'), (72, 24, 'F#6')],
}


def lead(s, bar, line, ch='sq1', voice=LEAD, octave=0, vel=118):
    for t, n, k in line:
        o = dict(voice)
        if n >= 24:
            o['scoop'] = 0.5
        s.note(ch, bar, t, n - 1, key(k) + octave, vel, **o)


def arp(s, bar, t0, t1, notes, vel=80):
    v = keys(notes)
    order = [0, 1, 2, 3, 4, 3, 2, 1]
    for i, t in enumerate(range(t0, t1, 6)):
        s.note('sq2', bar, t, 5, v[order[i % len(order)]], vel if i % 2 == 0 else vel - 30,
               pan=-0.6 if i % 4 < 2 else 0.6, **ARP)


def bass(s, bar, t0, t1, root, nxt):
    """Eighths, root and octave, a half step into the next chord."""
    r, n = key(root), key(nxt)
    for t in range(t0, t1, 12):
        last = t + 12 >= t1
        k = (n - 1 if n > r else n + 1) if last and n != r else (r + 12 if (t // 12) % 2 else r)
        s.note('wave', bar, t, 10, k, 124 if t == t0 else 112, **BASS)


KICK = (0, 3, 6, 10, 11)
SNARE = (4, 12)
GHOSTS = (7, 15)


def drums(s, bar, until=16, crash=False):
    for p in KICK:
        if p < until:
            s.hit('kick', bar, 6 * p, 6, vel=118 if p in (0, 10) else 100)
    for p in SNARE:
        if p < until:
            s.hit('snare', bar, 6 * p, 6, vel=118, pan=0.05)
    for p in GHOSTS:
        if p < until:
            s.hit('snare', bar, 6 * p, 3, vel=58, pan=-0.2)
    for p in range(until):
        if p in (2, 10):
            s.note('noise', bar, 6 * p, 6, 0, 92, **OPEN)
        else:
            s.note('noise', bar, 6 * p, 2, 0, (108, 48, 78, 48)[p % 4], **HAT)
    if crash:
        s.note('fx', bar, 0, 60, 0, 124, **CRASH)


def fill(s, bar):
    for t, tom, pan in ((72, 'tom_hi', -0.5), (78, 'tom_hi', -0.5), (84, 'tom_mid', 0.0), (87, 'tom_mid', 0.0),
                        (90, 'tom_lo', 0.5), (93, 'tom_lo', 0.5)):
        s.hit(tom, bar, t, 3, vel=118, pan=pan)


# the brass's rhythm a bar: (tick, ticks, velocity): a long first hit, an
# answer pushed onto the and of three, a pickup into the next bar
BRASS = [(0, 22, 116), (60, 8, 104), (84, 10, 110)]


def harmony(s, bar):
    names = CHORDS[bar]
    spans = SPANS.get(len(names) - 1, [(0, 48), (48, 96)])
    flat = [n for b in CHORDS for n in b] + ['D']
    i0 = sum(len(b) for b in CHORDS[:bar])
    for j, (name, (t0, t1)) in enumerate(zip(names, spans)):
        root, triad, notes = H[name]
        nxt = H[flat[i0 + j + 1]][0]
        bass(s, bar, t0, t1, root, nxt)
        s.hit('strings', bar, t0, t1 - t0 - 2, key(triad) - 12, 92, -0.6 if (t0 // 48) % 2 == 0 else 0.6)
        if bar < 4:
            arp(s, bar, t0, t1, notes)
        for t, n, vel in BRASS:
            if t0 <= t < t1:
                s.hit('brass', bar, t, n, triad, vel, -0.55 if t else 0.55)


def arrangement():
    s = Score()
    for bar in range(8):
        harmony(s, bar)
        drums(s, bar, until=12 if bar in (3, 7) else 16, crash=bar in (0, 4))
        lead(s, bar, LINES[bar])
        if bar >= 4:
            lead(s, bar, LINES[bar], ch='sq2', voice=SECOND, octave=-12, vel=104)
    s.hit('impact', 0, 0, 30, vel=124)
    s.hit('impact', 4, 0, 30, vel=124)
    fill(s, 3)
    fill(s, 7)
    # bar 8: D, the brass and strings ringing, the squares on F-sharp and D
    s.note('fx', 8, 0, 96, 0, 127, **CRASH)
    s.hit('impact', 8, 0, 48, vel=127)
    s.hit('kick', 8, 0, 6)
    s.hit('snare', 8, 0, 6)
    s.hit('brass', 8, 0, 72, 'D4', 124, 0.3)
    s.hit('brass', 8, 0, 72, 'A3', 110, -0.3)
    s.hit('strings', 8, 0, 110, 'D3', 104, 0.0)
    s.note('wave', 8, 0, 90, 'D2', 124, **dict(BASS, env=(0, 0, 15, 3)))
    s.note('sq1', 8, 0, 90, 'F#6', 120, **dict(LEAD, scoop=0.5, vib=(32, 10)))
    s.note('sq2', 8, 0, 90, 'D5', 108, **dict(SECOND, vib=(26, 12)))
    return s
