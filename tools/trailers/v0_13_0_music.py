"""0.13.0's trailer music: an original piece in the manner of Mega Man
Battle Network 6's "Surge of Power!" (Yoshino Aoki), played the way the
Game Boy Advance plays it (tools/trailer_music.py's model of its channels
and Direct Sound).

The song is BN6's 0x16, matched to the owner's recording (kept in .build/)
by its tempo (150 against the recording's 150) and its chroma frames, which
line up with the recording's at no transposition and a 0.2 s offset far
better than any other song's (0.84 against 0.77 at best). How it is
written, read from the ROM with an MP2K reader (its bar line 12 ticks into
the track); nothing of it is used here but these habits:
- 150 beats a minute, F minor, its bass leaning on the Phrygian half step
  above the root (F, G-flat, back) and the seventh under it; later
  sections move to the borrowed bVI and to a chord a half step up.
- The two squares trade one job for another. Its signature is a 32nd-note
  arpeggio, each sixteenth struck twice, sweeping a minor seventh chord up
  three octaves in a bar and turning down at its end, on a plucked 25%
  square. One square runs it alone over the band for four bars; then it
  passes to the other square while the first holds a 50% lead in long
  notes; then both are busy; later the arpeggio is in both squares at
  once, and one square is left alone over the band for eight bars.
- The wave channel plays the bass in eighths and sixteenths, root and
  octave, on the kick's syncopations.
- The kit: the kick on one and then off the beat (the and of two, its
  last sixteenth, the ands of three and four), a clap on two and four,
  closed hats on the first three sixteenths of every beat but the last,
  whose third is an open hat over a kick, a crash at the head of a phrase;
  toms falling into a new section. Direct Sound
  adds short low stabs in pairs of sixteenths and a few held chords and
  hits. Its echo is 40.
- The form: the arpeggio alone for four bars, eight bars of lead over it,
  four with both squares busy, eight on bVI with the lead in eighths, four
  bars of both squares in 32nds, eight of one square alone.

Here, at 150 (a bar is 48 frames of the trailer's 30) in F minor, the
squares are two players. Each is panned to its side and has its own duty:
the first square (left, 50%) calls, the second (right, 25%) answers. The
call and the answer share one rhythmic cell, an eighth, two sixteenths, a
dotted eighth, a sixteenth, an eighth and a held note (with two sixteenths
of pickup before it); the call rises to an open note, the answer falls to a
closed one, and the answer's pickup starts while the call still rings.
While one talks, the other runs the 32nd-note arpeggio low and quiet, a
wave up two octaves and back, so the two hand the jobs between them every
bar.

The form (the picture is cut to it):
- 0-1 the hook: the call over i, the answer over bVI, the beat from the
  first downbeat.
- 2 the gate: the arpeggio alone over an F pedal, a question on the
  second square. 3 connecting: the two squares ping back and forth,
  sixteenths then 32nds, climbing over bVI and bVII, a riser and a snare
  roll. 4 the log-in beam: a crash and a hit on the downbeat, a rising
  glide on the first square.
- 5 the NetPass: three stamps on beats one, two and three (a stamp, a
  kick, a chord hit and a ping climbing F, A-flat, C, the squares taking
  them in turn), beat four a snare pickup. 6 "Full pass": a bright bVI
  with pings, then toms and the call's pickup.
- 7-14 the theme over the full band, i bVI iv V, then i bVI bVII V: two-bar
  units, the call then the answer. The first four bars fall to the
  dominant (C5), the second four climb (the answers end on F6, then G6).
- 15 the drop: the first square alone, thin (12.5%), the call bent to the
  Phrygian over a bII pad on an F pedal, the bass that one note. 16 the
  climb: both squares in 32nds, a sixteenth apart, a snare roll and a
  riser over bVI and bVII, the pickup in octaves.
- 17-20 the climax: the theme with both squares in octaves at last,
  brass under them, the band fullest.
- 21-22 the end card: one last call (bVI) and answer (bVII), each player
  converging, and I in major on bar 22's downbeat (A and F, the squares a
  sixth apart), ringing, the two players' pings echoing left and right.

Its BANNER, the 3DS HOME Menu's sound while it is the current trailer: bar
21's last call and answer and the major chord ringing.

    python3 tools/trailer_music.py OUT.wav --trailer 0.13.0
"""
import math
import os
import random
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import trailer_music as tm  # noqa: E402
from trailer_music import Sample, Score, key, keys  # noqa: E402
from trailers import v0_3_0_music as first  # noqa: E402
from trailers import v0_12_0_music as last  # noqa: E402

BPM = 150
BARS = 23
TAIL = 2.2
MIX = (('call', ('sq1',), 0.24), ('answer', ('sq2',), 0.24), ('bass', ('wave',), 0.24),
       ('hats', ('noise',), 0.075), ('fx', ('fx',), 0.075))
DS_LEVEL = 0.68
REVERB = 40
BANNER = (21 * 4 * 60 / BPM, 2.9)   # (from bar 21's downbeat, seconds)

# ---- the voices ----

CALL = dict(duty=0.5, env=(0, 0, 13, 2), pan=-0.5, vib=(20, 12))
ANSWER = dict(duty=0.25, env=(0, 0, 13, 2), pan=0.5, vib=(20, 12))
VOICE = {'sq1': CALL, 'sq2': ANSWER}
# the arpeggio's pluck: each 32nd a step down from its strike
ARP = dict(duty=0.25, env=(0, 1, 9, 1))
SIDE = {'sq1': -0.5, 'sq2': 0.5}
THIN = dict(duty=0.125, env=(0, 0, 12, 3), pan=-0.2, vib=(34, 8))
PULSE = [15, 15, 15, 15, 15, 15, 15, 15, 13, 11, 9, 7, 5, 3, 1, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 2, 4, 6, 8, 10, 12, 14, 15]
BASS = dict(env=(0, 0, 15, 0), wave=PULSE)
BASS_HOLD = dict(env=(0, 0, 15, 3), wave=PULSE)
HAT = dict(first.HAT, pan=0.2)
OPEN = dict(first.OPEN, pan=0.25)
CRASH = dict(first.CRASH, pan=-0.15)

DS_ENV = dict(last.DS_ENV, stamp=(255, 0, 255, 170), ping=(255, 0, 255, 246), boom=(255, 0, 255, 244))
DS_GAIN = dict(last.DS_GAIN, kick=0.85, stamp=0.9, ping=0.32, boom=0.55, pluck=0.2, pad=0.14, brass=0.26)


def instruments():
    """0.3.0's and 0.12.0's kits and tones, and this piece's own: a rubber
    stamp, a network ping and a falling boom."""
    S = last.instruments()
    rnd = random.Random(13)
    sr = tm.SAMPLE_RATE

    def length(sec):
        return range(int(sec * sr))

    # the stamp: a low thump, a wooden knock and the paper's slap
    stamp, ph, lo = [], 0.0, 0.0
    for i in length(0.24):
        t = i / sr
        ph += 2 * math.pi * (70 + 110 * math.exp(-t / 0.012)) / sr
        n = rnd.uniform(-1, 1)
        lo += 0.35 * (n - lo)
        x = (math.sin(ph) * math.exp(-t / 0.05)
             + 0.6 * math.sin(2 * math.pi * 820 * t) * math.exp(-t / 0.012)
             + 1.2 * (n - lo) * math.exp(-t / 0.007))
        stamp.append(math.tanh(2.0 * x))
    S['stamp'] = Sample(stamp)

    # the ping: a sine chirping down onto its pitch, a little of its octave
    ping = []
    f0 = tm.hz(key('C6'))
    ph = 0.0
    for i in length(0.6):
        t = i / sr
        ph += 2 * math.pi * f0 * (1 + 0.5 * math.exp(-t / 0.006)) / sr
        ping.append((math.sin(ph) + 0.25 * math.sin(2 * ph) * math.exp(-t / 0.05)) * math.exp(-t / 0.16))
    S['ping'] = Sample(ping, base=key('C6'))

    # the boom: a sine falling an octave, with a rumble
    boom, ph, lo = [], 0.0, 0.0
    for i in length(1.2):
        t = i / sr
        ph += 2 * math.pi * (40 + 45 * math.exp(-t / 0.35)) / sr
        lo += 0.05 * (rnd.uniform(-1, 1) - lo)
        boom.append((math.sin(ph) + 1.5 * lo) * math.exp(-t / 0.45) * (1 - math.exp(-t / 0.004)))
    S['boom'] = Sample(boom)
    return S


# ---- the parts ----

# the cell both players speak in, (tick, ticks), and its pickup in the bar
# before
CELL = [(0, 12), (12, 6), (18, 6), (24, 18), (42, 6), (48, 12), (60, 36)]
PICKUP = [(84, 6), (90, 6)]


def say(s, ch, bar, line, pickup=None, vel=120, voice=None, octave=0, last_len=36, scoop=True):
    """A phrase in the cell on one square: its seven notes in the bar, its
    two pickup notes at the end of the bar before."""
    o = dict(voice or VOICE[ch])
    if pickup:
        for (t, n), k in zip(PICKUP, keys(pickup)):
            s.note(ch, bar - 1, t, n, k + octave, vel - 8, **o)
    for i, ((t, n), k) in enumerate(zip(CELL, keys(line))):
        extra = {}
        if i == 6:
            n = last_len
            if scoop:
                extra['scoop'] = 1.0
        s.note(ch, bar, t, n, k + octave, vel, **dict(o, **extra))


def both(s, bar, line, pickup=None, vel=118, last_len=36):
    """The climax: the line on both squares, an octave apart, nearer the
    middle."""
    say(s, 'sq2', bar, line, pickup, vel, dict(ANSWER, pan=0.25))
    say(s, 'sq1', bar, line, pickup, vel - 6, dict(CALL, pan=-0.25), octave=-12, last_len=last_len)


# the arpeggio: up two octaves of the chord in sixteenths and back, each
# struck twice as 32nds, the second softer
WAVE = [0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1]


def arp(s, ch, bar, t0, t1, voicing, vel=84, pattern=WAVE, pan=None, lift=0):
    v = keys(voicing)
    side = SIDE[ch] if pan is None else pan
    for i, t in enumerate(range(t0, t1, 6)):
        k = v[pattern[i % len(pattern)] % len(v)]
        a = min(127, vel + lift * i)
        s.note(ch, bar, t, 3, k, a, pan=side, **ARP)
        s.note(ch, bar, t + 3, 3, k, a - 22, pan=side, **ARP)


# the bass: (tick, ticks, tone) of the Root, its Octave, the Neighbour (the
# Phrygian half step on i, the fifth elsewhere) and an Approach to the next
# root
GROOVE = [(0, 12, 'R'), (12, 6, 'O'), (18, 12, 'R'), (30, 6, 'R'), (36, 12, 'N'), (48, 12, 'R'), (60, 6, 'O'),
          (66, 12, 'R'), (78, 6, 'R'), (84, 12, 'A')]
HALF = [(0, 12, 'R'), (12, 6, 'O'), (18, 12, 'R'), (30, 6, 'R'), (36, 12, 'A')]


def bass(s, bar, t0, root, nxt, pattern=GROOVE, minor_i=False, vel=124):
    r, n = key(root), key(nxt)
    tone = {'R': r, 'O': r + 12, 'N': r + 1 if minor_i else r + 7,
            'A': n - 1 if (n - r) % 12 else r + 7}
    for t, length, sym in pattern:
        s.note('wave', bar, t0 + t, length, tone[sym], vel, **BASS)


KICKS = (0, 18, 36, 66, 84)
PUSH_KICKS = (0, 18, 36, 60, 66, 84)


def hats(s, bar, until=96, soft=0, opens=(84,)):
    """The first three sixteenths of each beat, the last beat's third an
    open hat."""
    for t in range(0, until, 6):
        if t % 24 == 18:
            continue
        if t in opens:
            s.note('noise', bar, t, 6, 0, 100 - soft, **OPEN)
        else:
            s.note('noise', bar, t, 3, 0, (116 if t % 24 == 0 else 92 if t % 24 == 12 else 70) - soft, **HAT)


def kit(s, bar, push=False, until=96, soft=0):
    """The kick on one and off the beat, the snare and clap on two and four,
    hats in eighths, the open hat on the and of four over the kick."""
    for t in PUSH_KICKS if push else KICKS:
        if t < until:
            s.hit('kick', bar, t, 12, vel=124 - soft)
    for t in (24, 72):
        if t < until:
            s.hit('snare', bar, t, 12, vel=112 - soft, pan=0.05)
            s.hit('clap', bar, t, 6, vel=96 - soft, pan=-0.1)
    hats(s, bar, until, soft)


def crash(s, bar, tick, vel=127, length=72):
    s.note('fx', bar, tick, length, 0, vel, **CRASH)


def fill(s, bar, t0=72):
    for t, tom, pan in ((t0, 'tom_hi', -0.4), (t0 + 6, 'tom_hi', -0.4), (t0 + 12, 'tom_mid', 0.0),
                        (t0 + 18, 'tom_lo', 0.4)):
        s.hit(tom, bar, t, 6, vel=116, pan=pan)
    s.hit('kick', bar, t0, 12)


def stabs(s, bar, chord, ticks=(12, 18, 60, 66), vel=100):
    """Short low stabs in pairs of sixteenths, off the beat."""
    for t in ticks:
        s.chord('pluck', bar, t, 4, chord, vel, 0.4)


def downbeat(s, bar, root, vel=122):
    crash(s, bar, 0)
    s.hit('hit', bar, 0, 24, root, vel)


# the chords: (root, stab, the arpeggio's two octaves, the pad, i or not)
Fm = ('F2', 'F3 Ab3 C4', 'F3 Ab3 C4 Eb4 F4 Ab4 C5 Eb5 F5', 'F3 Ab3 C4 Eb4', True)
Db = ('Db2', 'F3 Ab3 Db4', 'Db3 F3 Ab3 C4 Db4 F4 Ab4 C5 Db5', 'Db3 F3 Ab3 C4', False)
Bbm = ('Bb1', 'F3 Bb3 Db4', 'Bb2 Db3 F3 Ab3 Bb3 Db4 F4 Ab4 Bb4', 'Bb2 Db3 F3 Ab3', False)
C = ('C2', 'E3 G3 C4', 'C3 E3 G3 Bb3 C4 E4 G4 Bb4 C5', 'C3 E3 G3 Bb3', False)
Eb = ('Eb2', 'G3 Bb3 Eb4', 'Eb3 G3 Bb3 Db4 Eb4 G4 Bb4 Db5 Eb5', 'Eb3 G3 Bb3 Db4', False)


def band(s, bar, chord, nxt, push=False, pad=0, stab_vel=100):
    root, stab, _, pad_chord, minor_i = chord
    bass(s, bar, 0, root, nxt, minor_i=minor_i)
    stabs(s, bar, stab, vel=stab_vel)
    if pad:
        s.chord('pad', bar, 0, 96, pad_chord, pad, 0.6)
    kit(s, bar, push)


# ---- the score ----

# the theme: (the chord, who speaks, the line, its pickup)
THEME = [
    (Fm, 'sq1', 'F5 F5 Ab5 G5 Eb5 F5 C6', 'C5 Eb5'),
    (Db, 'sq2', 'C6 C6 Bb5 Ab5 G5 Ab5 F5', 'Ab5 Bb5'),
    (Bbm, 'sq1', 'Bb5 Bb5 Db6 C6 Ab5 Bb5 F5', 'F5 Ab5'),
    (C, 'sq2', 'G5 G5 F5 E5 Db5 E5 C5', 'E5 F5'),
    (Fm, 'sq1', 'F5 F5 Ab5 G5 Eb5 F5 C6', 'C5 E5'),
    (Db, 'sq2', 'C6 C6 Db6 Eb6 C6 Db6 F6', 'Ab5 Bb5'),
    (Eb, 'sq1', 'G5 G5 Bb5 Ab5 F5 G5 Eb6', 'Eb5 F5'),
    (C, 'sq2', 'G5 G5 Bb5 C6 Db6 E6 G6', 'Eb5 F5'),
]
OTHER = {'sq1': 'sq2', 'sq2': 'sq1'}


def hook(s):
    """Bars 0-1: the call alone over i, the answer over bVI, the beat from
    the first downbeat."""
    downbeat(s, 0, 'F3')
    for i, (chord, who, line, pickup) in enumerate(THEME[:2]):
        nxt = THEME[i + 1][0][0] if i == 0 else 'F2'
        band(s, i, chord, nxt, push=i == 1)
        say(s, who, i, line, pickup if i else None)
    s.chord('pad', 1, 0, 96, Db[3], 80, 0.6)
    s.hit('tom_hi', 1, 84, 6, vel=110, pan=-0.4)
    s.hit('tom_mid', 1, 90, 6, vel=116, pan=0.2)


def going_online(s):
    # bar 2, the gate: the arpeggio alone over an F pedal, a question
    s.note('wave', 2, 0, 12, 'F2', 124, **BASS)
    for t in range(12, 96, 12):
        s.note('wave', 2, t, 6 if t % 24 else 12, 'F2' if t % 24 == 0 else 'F3', 116, **BASS)
    arp(s, 'sq1', 2, 0, 96, Fm[2], vel=96)
    s.chord('pad', 2, 0, 96, 'F3 Ab3 C4 G4', 96, 0.6)
    s.hit('kick', 2, 0, 12)
    s.hit('kick', 2, 36, 12, vel=96)
    s.hit('clap', 2, 72, 6, vel=90)
    for t, v in ((84, 70), (90, 86)):
        s.hit('snare', 2, t, 6, vel=v, pan=0.05)
    hats(s, 2, soft=26, opens=())
    for t, n, k in ((48, 12, 'C6'), (60, 6, 'C6'), (66, 30, 'Eb6')):
        s.note('sq2', 2, t, n, k, 104, **dict(ANSWER, scoop=1.0) if n == 30 else ANSWER)
    # bar 3, connecting: the squares ping back and forth, sixteenths then
    # 32nds, climbing over bVI and bVII; a riser, a snare roll
    s.note('fx', 3, 0, 92, 0, 110, sweep=(2500, 240000), ramp=(2, 12))
    lows = keys('C5 C5 Db5 Db5 Eb5 Eb5 E5 E5')
    for i, t in enumerate(range(0, 48, 6)):
        ch = 'sq1' if i % 2 == 0 else 'sq2'
        k = lows[i] + (0 if ch == 'sq1' else 5)
        s.note(ch, 3, t, 6, k, 96 + 2 * i, pan=SIDE[ch], **ARP)
    run = keys('F5 Ab5 G5 Bb5 Ab5 C6 Bb5 Db6 C6 Eb6 Db6 F6 Eb6 G6 F6 G6')
    for i, t in enumerate(range(48, 96, 3)):
        ch = 'sq1' if i % 2 == 0 else 'sq2'
        s.note(ch, 3, t, 3, run[i], 104 + i, pan=SIDE[ch], **ARP)
    for t in (0, 24, 48, 72):
        s.hit('kick', 3, t, 12)
    roll = list(range(0, 48, 12)) + list(range(48, 72, 6)) + list(range(72, 96, 3))
    for i, t in enumerate(roll):
        s.hit('snare', 3, t, 6, vel=50 + 70 * i // len(roll), pan=0.05)
    s.chord('pad', 3, 0, 48, Db[3], 100, 0.6)
    s.chord('pad', 3, 48, 48, Eb[3], 108, 0.6)
    bass(s, 3, 0, 'Db2', 'Eb2', HALF)
    bass(s, 3, 48, 'Eb2', 'F2', HALF)
    for t in range(0, 96, 24):
        s.note('noise', 3, t, 3, 0, 90, **HAT)
    # bar 4, the log-in beam: the landing, a glide up an octave on the
    # first square, the arpeggio on the second
    downbeat(s, 4, 'F3', 127)
    s.hit('boom', 4, 0, 48, vel=110)
    s.note('sq1', 4, 0, 36, 'F5', 124, **dict(CALL, drop=-1200, vib=None))
    s.note('sq1', 4, 36, 30, 'F6', 116, **CALL)
    arp(s, 'sq2', 4, 0, 96, Fm[2], vel=88)
    band(s, 4, Fm, 'F2', pad=92)
    s.hit('tom_mid', 4, 84, 6, vel=112, pan=0.0)
    s.hit('tom_lo', 4, 90, 6, vel=118, pan=0.4)


def netpass(s):
    """Bar 5: three stamps, on beats one, two and three, the squares taking
    them in turn; a snare pickup on four. Bar 6: "Full pass", a bright bVI,
    then the toms and the call's pickup."""
    for i, (t, root, ping, ch, k) in enumerate(((0, 'F3', 'F5', 'sq1', 'F5'), (24, 'Ab3', 'Ab5', 'sq2', 'Ab5'),
                                                (48, 'C4', 'C6', 'sq1', 'C6'))):
        s.hit('stamp', 5, t, 12, vel=127)
        s.hit('kick', 5, t, 12)
        s.hit('clap', 5, t, 6, vel=100, pan=-0.1)
        s.hit('hit', 5, t, 18, root, 112 + 4 * i)
        s.hit('ping', 5, t + 3, 24, ping, 110, SIDE[ch])
        s.note(ch, 5, t, 14, k, 120, **VOICE[ch])
        s.note('wave', 5, t, 12, 'F2', 124, **BASS)
        crash(s, 5, t, 84 + 12 * i, 18)
    for i, t in enumerate(range(72, 96, 6)):
        s.hit('snare', 5, t, 6, vel=84 + 10 * i, pan=0.05)
    s.note('wave', 5, 72, 12, 'C2', 120, **BASS)
    s.note('wave', 5, 84, 12, 'C3', 120, **BASS)
    # bar 6: Full pass
    downbeat(s, 6, 'Db4', 127)
    s.chord('pad', 6, 0, 72, 'Db3 F3 Ab3 C4 Eb4', 110, 0.6)
    for i, k in enumerate(keys('Db6 F6 Ab6 C7')):
        s.hit('ping', 6, 6 + 6 * i, 24, k, 104 - 8 * i, -0.6 + 0.4 * i)
    arp(s, 'sq2', 6, 0, 72, Db[2], vel=92)
    bass(s, 6, 0, 'Db2', 'C2', GROOVE[:7])
    s.note('wave', 6, 72, 24, 'C2', 124, **BASS)
    kit(s, 6, until=72)
    fill(s, 6)
    for (t, n), k in zip(PICKUP, keys('C5 Eb5')):
        s.note('sq1', 6, t, n, k, 112, **CALL)


def theme(s):
    """Bars 7-14: the call and the answer over the full band; while one
    speaks, the other runs the arpeggio."""
    for i, (chord, who, line, pickup) in enumerate(THEME):
        bar = 7 + i
        nxt = THEME[i + 1][0][0] if i + 1 < len(THEME) else 'Gb2'
        if i % 4 == 0:
            downbeat(s, bar, 'F3')
        band(s, bar, chord, nxt, push=i % 2 == 1, pad=88 if i >= 4 else 0, stab_vel=96 + 4 * (i >= 4))
        say(s, who, bar, line, pickup if i else None)
        other = OTHER[who]
        arp(s, other, bar, 0, 84 if i + 1 < len(THEME) else 72, chord[2], vel=78 + 3 * (i >= 4))
    fill(s, 10)
    fill(s, 14, 48)
    for i, t in enumerate(range(72, 96, 6)):
        s.hit('snare', 14, t, 6, vel=96 + 8 * i)


def drop(s):
    """Bar 15: the first square alone, thin, the call bent to the Phrygian
    over a bII pad and the bass's one F. Bar 16: the climb back, both
    squares in 32nds, a roll and a riser, the pickup in octaves."""
    s.hit('boom', 15, 0, 96, vel=88)
    s.hit('kick', 15, 0, 12, vel=110)
    s.note('wave', 15, 0, 96, 'F2', 72, **BASS_HOLD)
    s.chord('pad', 15, 0, 96, 'Gb3 Bb3 Db4 F4', 84, 0.6)
    say(s, 'sq1', 15, 'F5 F5 Gb5 F5 Db5 Eb5 Bb4', voice=THIN, vel=112, last_len=30)
    for t in (24, 48, 72):
        s.note('noise', 15, t, 3, 0, 64, **HAT)
    s.hit('ping', 15, 66, 24, 'Gb5', 70, 0.6)
    s.hit('ping', 15, 78, 24, 'Gb5', 50, -0.6)
    s.note('fx', 15, 48, 48, 0, 80, sweep=(4000, 90000), ramp=(1, 8))
    # bar 16: the climb
    s.note('fx', 16, 0, 90, 0, 116, sweep=(3000, 260000), ramp=(2, 13))
    for t in (0, 24, 48, 72):
        s.hit('kick', 16, t, 12)
    roll = list(range(0, 48, 12)) + list(range(48, 72, 6)) + list(range(72, 96, 3))
    for i, t in enumerate(roll):
        s.hit('snare', 16, t, 6, vel=56 + 66 * i // len(roll), pan=0.05)
    s.chord('pad', 16, 0, 48, Db[3], 108, 0.6)
    s.chord('pad', 16, 48, 48, Eb[3], 116, 0.6)
    bass(s, 16, 0, 'Db2', 'Eb2', HALF)
    bass(s, 16, 48, 'Eb2', 'F2', HALF)
    arp(s, 'sq2', 16, 0, 48, 'Db4 F4 Ab4 C5 Db5 F5 Ab5 C6 Db6', vel=92, lift=2)
    arp(s, 'sq2', 16, 48, 84, 'Eb4 G4 Bb4 Db5 Eb5 G5 Bb5 Db6 Eb6', vel=100, lift=2)
    arp(s, 'sq1', 16, 6, 48, 'Db4 F4 Ab4 C5 Db5 F5 Ab5 C6 Db6', vel=88, lift=2)
    arp(s, 'sq1', 16, 48, 84, 'Eb4 G4 Bb4 Db5 Eb5 G5 Bb5 Db6 Eb6', vel=96, lift=2, pattern=WAVE[1:] + WAVE[:1])
    for t in range(0, 84, 12):
        s.note('noise', 16, t, 3, 0, 80 + t // 4, **HAT)


def climax(s):
    """Bars 17-20: the theme with both squares in octaves, brass under
    them, the pluck's arpeggio, the fullest the band gets."""
    lines = [(Fm, 'F5 F5 Ab5 G5 Eb5 F5 C6', 'C5 Eb5'),
             (Db, 'C6 C6 Bb5 Ab5 G5 Ab5 F5', 'Ab5 Bb5'),
             (Bbm, 'Bb5 Bb5 Db6 C6 Ab5 Bb5 F6', 'F5 Ab5'),
             (C, 'G5 G5 Bb5 C6 Db6 E6 G6', 'Eb5 F5')]
    for i, (chord, line, pickup) in enumerate(lines):
        bar = 17 + i
        nxt = lines[i + 1][0][0] if i + 1 < len(lines) else 'Db2'
        if i % 2 == 0:
            downbeat(s, bar, 'F3' if i == 0 else 'Bb3')
        band(s, bar, chord, nxt, push=i % 2 == 1, pad=100, stab_vel=108)
        both(s, bar, line, pickup)
        for (t, n), k in zip(CELL, keys(line)):
            s.hit('brass', bar, t, n, key(k) - 24, 112, -0.2)
        v = keys(chord[2])
        for j, t in enumerate(range(0, 96, 6)):
            s.hit('pluck', bar, t, 4, v[WAVE[j]] + 12, 70 if j % 2 else 84, 0.45 if j % 4 < 2 else 0.2)
    fill(s, 20)


def ending(s):
    """Bar 21: the last call over bVI and the answer over bVII, each player
    converging; bar 22: I in major, ringing, their pings echoing."""
    crash(s, 21, 0)
    s.hit('hit', 21, 0, 24, 'Db4', 120)
    s.hit('hit', 21, 48, 24, 'Eb4', 124)
    s.chord('pad', 21, 0, 48, 'Db3 F3 Ab3 C4', 112, 0.6)
    s.chord('pad', 21, 48, 48, 'Eb3 G3 Bb3 Db4', 116, 0.6)
    s.chord('brass', 21, 0, 48, 'Db3 F3 Ab3 C4', 104, 0.5)
    s.chord('brass', 21, 48, 48, 'Eb3 G3 Bb3 D4', 108, 0.5)
    bass(s, 21, 0, 'Db2', 'Eb2', HALF)
    bass(s, 21, 48, 'Eb2', 'F2', HALF)
    kit(s, 21, push=True, until=72)
    for i, t in enumerate(range(72, 96, 6)):
        s.hit('snare', 21, t, 6, vel=100 + 7 * i)
    s.hit('kick', 21, 72, 12)
    for t, n, k in ((0, 12, 'F5'), (12, 6, 'F5'), (18, 6, 'Ab5'), (24, 24, 'C6')):
        s.note('sq1', 21, t, n, k, 120, **dict(CALL, scoop=1.0) if n == 24 else CALL)
    for t, n, k in ((84, 6, 'G5'), (90, 6, 'G#5')):
        s.note('sq1', 21, t, n, k, 116, **CALL)
    for t, n, k in ((48, 12, 'Bb5'), (60, 6, 'Bb5'), (66, 6, 'C6'), (72, 12, 'D6'), (84, 6, 'Eb6'), (90, 6, 'E6')):
        s.note('sq2', 21, t, n, k, 120, **ANSWER)
    # bar 22: F major, ringing past the bar
    ring = 180
    crash(s, 22, 0, 127, 140)
    s.hit('hit', 22, 0, 36, 'F4', 127)
    s.hit('kick', 22, 0, 12)
    s.hit('snare', 22, 0, 12, vel=110)
    s.hit('timp', 22, 0, 48, 'F2', 124)
    s.hit('boom', 22, 0, 96, vel=100)
    s.chord('brass', 22, 0, ring, 'F3 A3 C4 F4', 118, 0.5)
    s.chord('pad', 22, 0, ring, 'F3 A3 C4 G4', 116, 0.6)
    s.note('wave', 22, 0, ring, 'F2', 124, **dict(BASS_HOLD, ramp=(15, 0)))
    s.note('sq2', 22, 0, ring, 'F6', 120, **dict(ANSWER, pan=0.35, scoop=1.0, ramp=(14, 0)))
    s.note('sq1', 22, 0, ring, 'A5', 116, **dict(CALL, pan=-0.35, scoop=1.0, ramp=(13, 0)))
    for i, (t, k) in enumerate(((24, 'C6'), (36, 'F6'), (48, 'C6'), (60, 'F6'), (72, 'C6'), (84, 'F6'))):
        s.hit('ping', 22, t, 24, k, 112 - 14 * i, -0.7 if i % 2 == 0 else 0.7)


def arrangement():
    s = Score()
    hook(s)
    going_online(s)
    netpass(s)
    theme(s)
    drop(s)
    climax(s)
    ending(s)
    return s
