"""0.16.0's trailer music: an original piece in the manner of BN6's
"Showdown" (song 0x22), grown from sketch B (tools/trailers/sketch_vb_b.py),
the one the owner chose by ear of three (26 October 2026: "i like b"),
written by .claude/skills/trailer/music_rules.md, its Discipline and its
measure (music Akari Kaida would be proud of). None of Capcom's notes,
chords in their order or samples. 0x22's habits, read from the ROM, are
in the sketch's docstring; this piece keeps the sketch's parts and sound
and adds what a trailer needs around them.

Played on FluidR3 GM's samples (the approved palette, sketch_vb_lib.py:
the electronic kit's kick, the snare, the toms, the strings' chords, the
distortion guitar's power chord, the brass section, the saw lead); the
squares, the wave and the noise are the Game Boy's own channels. The
sketches were heard through GeneralUser GS and MuseScore General too; the
owner heard no difference through MP2K's cut, and FluidR3 GM stays for its
licence (music_rules.md).

Here, in B-flat minor at 150 (a bar is 48 frames of the trailer's 30), 23
bars to the trailer's slots (SKILL.md, The structure), a section's idea
at its head and nowhere else:
- 0-1 under the cold open: the paired 12.5% backing alone, a riser; the
  strings and the bass held under Bbm Gdim, the kick's broken beat, the
  toms and the lead's pickup.
- 2-3 the title card: the hook arrives with the band (F held and struck
  again, a turn through G-flat, up to the D-flat held, over Bbm Gdim),
  then its answer over Absus2 Gb.
- 4-8 the Virus Battler's segment: the hook again, falling to the
  diminished E-natural (Bbm Bbdim), the cadence over Gb Fm; the B, the
  backing to the 75% voice's repeating 32nds and the melody's long notes
  on the sampled saw (Gb Absus2 | Ebm Fm), brass on its last beat; the
  hook back; the toms, the lift into the next.
- 9-12 the dark layer's segment: the drop (B-flat minor struck, the
  hook's first half alone at half speed on a thin square), the climb
  back (the 32nds rising, a snare roll), the B again, the lift.
- 13-16 the feel's segment, the climax: the A at full strength, the saw
  doubling the lead an octave down, the stab, brass on the cadence.
- 17-18 the recap: the hook back, the turnaround over Ebm and Fsus4 to
  F (the major V once, its fourth falling to the A-natural on the beat
  the chord turns major), the toms.
- 19 the logo's stinger: B-flat minor with its ninth struck on the
  cadence and ringing.
- 20-22 the end card held: the paired backing alone again, softly, over
  the strings, as the piece began, and the ninth ringing out.

The middle (500-2000 Hz) brought toward 0x22's (37% of its energy; the
sketch had 14-17%) without more parts: the strings' chord samples voiced
an octave higher and louder, the backing and the lead louder in the mix,
the bass and the kick softer.

Its BANNER, the 3DS HOME Menu's sound while it is the current trailer:
the climax's first bar and a half, the hook at full strength. The piece
plays a soundfont, so it renders in docker/Dockerfile.music's image
(tools/trailer_music.py's render_any) and keeps its banner as our own
audio in BANNER_AUDIO, written by
`python3 tools/trailer_music.py --write-banner --piece v0_16_0_music`.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from trailer_music import Score, key, keys  # noqa: E402
from trailers import sketch_vb_lib as L  # noqa: E402
from trailers.sketch_vb_lib import CHORD_SAMPLES, DS_ENV, DS_LEVEL, HARDWARE, REVERB, SAMPLED_LEADS, SOUNDFONT  # noqa: E402,F401

BPM = 150
BARS = 23
TAIL = 2.0
MIX = (('lead', ('sq1',), 0.15), ('arp', ('sq2',), 0.26), ('bass', ('wave',), 0.11),
       ('hats', ('noise',), 0.12), ('fx', ('fx',), 0.08))
DS_GAIN = dict(L.DS_GAIN, kick=0.8, pad=0.92, pad_m=0.92, pad_M=0.92, pad_d=0.92)
BANNER = (13 * 4 * 60 / BPM, 2.9)      # (from bar 13's downbeat, seconds: the climax's hook)
BANNER_AUDIO = '3ds/banner-0.16.0.wav'


def instruments():
    return L.instruments('Bb1')


LEAD = dict(duty=0.5, env=(0, 0, 10, 4), vib=(26, 14), pan=0.0)
THIN = dict(duty=0.125, env=(0, 0, 13, 2))
ALONE = dict(duty=0.125, env=(0, 0, 12, 4), vib=(30, 8), pan=0.0)     # the drop's square alone
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
    'Fsus4': ('F2', 'F3 Bb3 C4 F4 Bb4 C5', 'pad', 'Bb2'),        # Bb C F over F
    'F': ('F2', 'F3 A3 C4 F4 A4 C5', 'pad_M', 'F2'),
}
CHORDS = {0: ('Bbm',), 1: ('Bbm', 'Gdim'), 2: ('Bbm', 'Gdim'), 3: ('Absus2', 'Gb'),
          4: ('Bbm', 'Bbdim'), 5: ('Gb', 'Fm'), 6: ('Gb', 'Absus2'), 7: ('Ebm', 'Fm'), 8: ('Bbm', 'Gdim'),
          9: ('Bbm', 'Gdim'), 10: ('Gb', 'Absus2'), 11: ('Gb', 'Absus2'), 12: ('Ebm', 'Fm'),
          13: ('Bbm', 'Gdim'), 14: ('Absus2', 'Gb'), 15: ('Bbm', 'Bbdim'), 16: ('Gb', 'Fm'),
          17: ('Bbm', 'Gdim'), 18: ('Ebm', 'Fsus4', 'F'), 19: ('Bbm',),
          20: ('Bbm', 'Gdim'), 21: ('Gb', 'Absus2'), 22: ('Bbm',)}

# ---- the lines: (tick, ticks, note), the sketch's ----
HOOK = [(0, 24, 'F5'), (24, 6, 'F5'), (30, 6, 'Gb5'), (36, 12, 'F5'), (48, 36, 'Db6'), (84, 12, 'Bb5')]
A = [HOOK,
     [(0, 36, 'Bb5'), (36, 6, 'Bb5'), (42, 6, 'Ab5'), (48, 48, 'Gb5')],
     [(0, 24, 'F5'), (24, 6, 'F5'), (30, 6, 'Gb5'), (36, 12, 'F5'), (48, 36, 'E5'), (84, 12, 'Bb5')],
     [(0, 24, 'Db6'), (24, 12, 'Db6'), (36, 12, 'Bb5'), (48, 48, 'C6')]]
SAWLINE = {8: [(0, 36, 'Db6'), (36, 12, 'Db6'), (48, 36, 'Eb6'), (84, 12, 'Eb6')],
           9: [(0, 36, 'Gb5'), (36, 12, 'Gb5'), (48, 48, 'F5')]}
TURN = {18: [(0, 24, 'Gb5'), (24, 12, 'Gb5'), (36, 12, 'Eb5'), (48, 24, 'Bb5'), (72, 24, 'A5')]}

PAIRS = (0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15)
FIG = (0, 3, 3, 2, 1, 1, 2, 3, 3, 4, 3)       # tones by onset: an octave up, skips back, a note struck again


def backing(s, bar, t0, t1, name, vel=104):
    """the thin pulse in pairs (0x22's commonest bar), thrown left and right"""
    v = keys(H[name][1])
    for i, p in enumerate(PAIRS):
        t = 6 * p
        if t0 <= t < t1:
            side = -0.9 if (i // 4) % 2 == 0 else 0.9
            s.note('sq2', bar, t, 5, v[FIG[i]], vel if p % 3 == 0 else vel - 36, pan=side, **THIN)


def repeats(s, bar, t0, t1, name, vel=96, soft=52):
    """the 75% voice repeating 32nds on one of the chord's notes a beat"""
    v = keys(H[name][1])
    for t in range(t0, t1, 3):
        k = v[(3, 4, 3, 2)[(t // 24) % 4]]
        s.note('sq2', bar, t, 3, k, vel if t % 6 == 0 else soft, pan=-0.9 if (t // 24) % 2 == 0 else 0.9, **REP)


def bass(s, bar, names, vel=120):
    """straight sixteenths on the root, its octave on each beat's third"""
    for t0, n, name in L.halves(names):
        r = key(H[name][0])
        for t in range(t0, t0 + n, 6):
            s.note('wave', bar, t, 5, r + (12 if (t // 6) % 4 == 2 else 0), vel, **BASS)


def chords(s, bar, names, vel=96):
    """the strings, a chord sample a half bar, an octave over the sketch's"""
    for t0, n, name in L.halves(names):
        _, _, pad, root = H[name]
        s.hit(pad, bar, t0, n - 2, key(root) + 24, vel, -0.35 if t0 < 48 else 0.35)


def kit(s, bar, crash=False, until=16, half=False):
    """0x22's broken beat: the kick on 0, 3, 6, 8, the snare on 4, 7, 10, 12
    (in half: the kick on 0, 3, 6 and the snare on 8)"""
    for p in ((0, 3, 6) if half else (0, 3, 6, 8)):
        if p < until:
            s.hit('kick', bar, 6 * p, 6, vel=104 if p in (0, 8) else 92)
    for p in ((8,) if half else (4, 7, 10, 12)):
        if p < until:
            s.hit('snare', bar, 6 * p, 6, vel=104 if p in (4, 8, 12) else 80, pan=0.05)
    L.hats(s, bar, opens=() if half else (4, 12), until=until, levels=(80, 44, 64, 44), every=2 if half else 1)
    if crash:
        s.note('fx', bar, 0, 48, 0, 116, **L.CRASH)


def stab(s, bar, name):
    """the low dark stab on 14 and 15, into the next bar"""
    L.power(s, bar, 84, 5, H[name][0], vel=104)
    L.power(s, bar, 90, 6, H[name][0], vel=112)


def lead(s, bar, line, vel=116, voice=LEAD, fall_last=False):
    for i, (t, n, k) in enumerate(line):
        o = dict(voice)
        if fall_last and i == len(line) - 1 and n >= 24:
            o['bend'] = [(0, 0), (n - 6, 0), (n, -2)]
        s.note('sq1', bar, t, n, k, vel - (12 if n <= 6 and t % 12 else 0), **o)


def band(s, bar, vel=104):
    """the backing, the strings and the bass under a bar's chords"""
    names = CHORDS[bar]
    for t0, n, name in L.halves(names):
        backing(s, bar, t0, t0 + n, name, vel=vel)
    chords(s, bar, names)
    bass(s, bar, names)


# ---- the sections ----


# ---- the sections, to the trailer's slots (SKILL.md, The structure) ----

def b_section(s, first, brass_last=True):
    """two bars of the B: the 75% voice's repeating 32nds, the melody's
    long notes on the sampled saw, brass on the second bar's last beat"""
    for i, bar in enumerate((first, first + 1)):
        names = CHORDS[bar]
        for t0, n, name in L.halves(names):
            repeats(s, bar, t0, t0 + n, name)
        chords(s, bar, names, vel=100)
        bass(s, bar, names)
        for t, n, k in SAWLINE[8 + i]:
            s.hit('dlead', bar, t, n - 1, k, 112, -0.25)
            s.hit('dlead', bar, t + 3, n - 1, k, 64, 0.4)
        kit(s, bar, crash=i == 0)
    L.power(s, first, 0, 30, 'Gb2', vel=116)
    if brass_last:
        for t in (72, 78, 84):
            s.hit('brass', first + 1, t, 5, 'Ab3', 104 - (t - 72), 0.3)       # Ab C Eb over F: Fm7


def fill(s, bar):
    """the toms into the next section, its lift"""
    L.toms(s, bar)


def cold_open(s):
    """Bars 0-1, under the cold open: the paired backing alone, a riser;
    then the strings and the bass held, the stab, the toms and the
    lead's pickup into the title."""
    backing(s, 0, 0, 96, 'Bbm', vel=88)
    s.note('fx', 0, 0, 92, 0, 100, sweep=(1500, 120000), ramp=(1, 8))
    for p in range(8, 16):
        s.note('noise', 0, 6 * p, 3, 0, 30 + 6 * (p - 8), **L.HAT)
    for t0, n, name in L.halves(CHORDS[1]):
        backing(s, 1, t0, t0 + n, name, vel=98)
        s.note('wave', 1, t0, n - 2, H[name][0], 112, **HELD)
    chords(s, 1, CHORDS[1], vel=90)
    s.note('fx', 1, 0, 92, 0, 104, sweep=(2000, 180000), ramp=(1, 10))
    for p in (0, 3, 6, 8):
        s.hit('kick', 1, 6 * p, 6, vel=88)
    fill(s, 1)
    s.note('sq1', 1, 84, 6, 'Db5', 104, **LEAD)
    s.note('sq1', 1, 90, 6, 'Eb5', 110, **LEAD)


def title(s):
    """Bars 2-3, the title card: the hook arrives with the band, and its
    answer."""
    for i, bar in enumerate((2, 3)):
        band(s, bar)
        lead(s, bar, A[i])
        kit(s, bar, crash=bar == 2)
    L.power(s, 2, 0, 30, 'Bb2', vel=118)
    stab(s, 3, 'Gb')


def battler(s):
    """Bars 4-8, the Virus Battler's segment: the hook again and its
    cadence, the B, the hook back and the lift into the next."""
    for i, bar in enumerate((4, 5)):
        band(s, bar)
        lead(s, bar, A[2 + i])
        kit(s, bar, crash=bar == 4, until=14 if bar == 5 else 16)
    L.power(s, 4, 0, 30, 'Bb2', vel=118)
    s.hit('tom_mid', 5, 84, 6, vel=108, pan=0.2)
    s.hit('tom_lo', 5, 90, 6, vel=112, pan=0.2)
    b_section(s, 6)
    band(s, 8)
    lead(s, 8, HOOK, vel=120)
    kit(s, 8, crash=True, until=12)
    L.power(s, 8, 0, 30, 'Bb2', vel=116)
    fill(s, 8)


def crosses(s):
    """Bars 9-12, the dark layer's segment: the drop (B-flat minor struck,
    the hook's first half alone at half speed), the climb back, then the B
    over its own chords, the lift into the climax."""
    s.note('fx', 9, 0, 72, 0, 118, **L.CRASH)
    s.hit('kick', 9, 0, 6, vel=124)
    L.power(s, 9, 0, 40, 'Bb2', vel=120, fall=True)
    for t0, n, name in L.halves(CHORDS[9]):
        s.note('wave', 9, t0, n - 2, H[name][0], 112, **dict(HELD, bend=[(0, 0), (n - 12, 0), (n - 2, -1)]))
    chords(s, 9, CHORDS[9], vel=100)
    for t, n, k in ((0, 48, 'F5'), (48, 12, 'F5'), (60, 12, 'G5'), (72, 24, 'F5')):
        s.note('sq1', 9, t, n, k, 114, **ALONE)
    for t in (24, 72):
        s.note('noise', 9, t, 3, 0, 60, **L.HAT)
    # bar 10: the climb back
    s.note('fx', 10, 0, 90, 0, 112, sweep=(2500, 240000), ramp=(2, 13))
    for i, (t0, n, name) in enumerate(L.halves(CHORDS[10])):
        repeats(s, 10, t0, t0 + n, name, vel=72 + 24 * i, soft=40 + 12 * i)
    chords(s, 10, CHORDS[10], vel=100)
    bass(s, 10, CHORDS[10], vel=104)
    for t in (0, 24, 48, 72):
        s.hit('kick', 10, t, 6, vel=110)
    roll = list(range(0, 48, 12)) + list(range(48, 72, 6)) + list(range(72, 96, 3))
    for i, t in enumerate(roll):
        s.hit('snare', 10, t, 3, vel=52 + 66 * i // len(roll), pan=-0.2 + 0.4 * i / len(roll))
    b_section(s, 11)
    fill(s, 12)
    s.note('sq1', 12, 84, 6, 'Db5', 110, **LEAD)
    s.note('sq1', 12, 90, 6, 'Eb5', 116, **LEAD)


def climax(s):
    """Bars 13-16, the feel's segment: the A at full strength, the saw
    doubling the lead an octave down, the stab, brass on the cadence."""
    for i, bar in enumerate(range(13, 17)):
        band(s, bar, vel=110)
        lead(s, bar, A[i], vel=122)
        for t, n, k in A[i]:
            s.hit('double', bar, t, n - 1, key(k) - 12, 92, 0.25)
        kit(s, bar, crash=bar % 2 == 1, until=14 if bar == 16 else 16)
    stab(s, 14, 'Gb')
    L.power(s, 13, 0, 30, 'Bb2', vel=120)
    L.power(s, 15, 0, 30, 'Bb2', vel=118)
    for t in (72, 78, 84):
        s.hit('brass', 16, t, 5, 'Ab3', 108 - (t - 72), 0.3)


def rest_and_stinger(s):
    """Bars 17-18, the recap: the hook back, the turnaround over Ebm and
    Fsus4-F (the major V once), the toms. Bar 19, the logo's stinger:
    B-flat minor with its ninth, struck on the cadence and ringing."""
    band(s, 17, vel=110)
    lead(s, 17, HOOK, vel=122)
    for t, n, k in HOOK:
        s.hit('double', 17, t, n - 1, key(k) - 12, 92, 0.25)
    kit(s, 17, crash=True)
    L.power(s, 17, 0, 30, 'Bb2', vel=118)
    band(s, 18, vel=110)
    lead(s, 18, TURN[18], vel=122)
    for t, n, k in TURN[18]:
        s.hit('double', 18, t, n - 1, key(k) - 12, 92, 0.25)
    kit(s, 18, until=12)
    L.toms(s, 18)
    s.note('fx', 19, 0, 96, 0, 124, **L.CRASH)
    s.hit('kick', 19, 0, 6)
    s.hit('snare', 19, 0, 6)
    L.power(s, 19, 0, 90, 'Bb2', vel=122, fall=True)
    s.hit('pad_m', 19, 0, 150, 'Bb4', 108, -0.35)      # Bb Db F
    s.hit('pad', 19, 0, 150, 'Bb3', 92, 0.35)          # Bb C F: the ninth
    s.hit('brass', 19, 0, 60, 'Db4', 104, 0.3)         # Db F Ab over Bb: its seventh
    s.note('wave', 19, 0, 92, 'Bb2', 127, **dict(HELD, bend=[(0, 0), (54, 0), (92, -12)]))
    s.note('sq1', 19, 0, 96, 'F5', 120, **dict(LEAD, bend=[(0, 0), (76, 0), (96, -2)]))
    s.note('sq2', 19, 0, 96, 'Db5', 96, **dict(LEAD, pan=0.35, bend=[(0, 0), (76, 0), (96, -2)]))


def end_card(s):
    """Bars 20-22, under the end card held: the paired backing alone again,
    softly, over the strings, as the piece began; B-flat minor's ninth
    ringing out."""
    for bar in (20, 21):
        for t0, n, name in L.halves(CHORDS[bar]):
            backing(s, bar, t0, t0 + n, name, vel=76 - 8 * (bar - 20))
        chords(s, bar, CHORDS[bar], vel=76 - 8 * (bar - 20))
    s.hit('pad', 22, 0, 140, 'Bb3', 70, 0.0)            # Bb C F
    s.note('wave', 22, 0, 90, 'Bb2', 96, **HELD)


def arrangement():
    s = Score()
    cold_open(s)
    title(s)
    battler(s)
    crosses(s)
    climax(s)
    rest_and_stinger(s)
    end_card(s)
    return s
