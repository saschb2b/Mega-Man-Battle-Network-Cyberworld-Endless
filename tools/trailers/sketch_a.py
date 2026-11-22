"""Sketch A, a battle theme's drive (for NetBattles, two a side): an
eight-bar sketch for the owner to judge by ear, not a cut. Original, in the
manner of BN6's battle themes and nearest the owner's reference for
0.13.0, "Surge of Power!" (song 0x16, his recording kept in .build/), by
the rules of .claude/skills/trailer/music_rules.md; none of its notes or
chords.

    python3 tools/trailer_music.py OUT.wav --piece sketch_a

What the reference does that 0.13.0's piece did not, and what this does:
- Punch: its kick and clap are short samples driven into the top (half
  the kick's 139 ms at full scale), its drum hits swing 18 dB inside a
  beat in the lows where ours swung 8, its notes stop. Here: the kit's
  punch_kick and punch_clap, the bass in gated sixteenths on the kick's
  places, chugs that stop dead.
- Body between 120 and 500 Hz: its low distorted stabs (a guitar's, with
  pitch bends) and chord samples fill it. Here: a power chord (the kit's
  power_stab) chugging with the kick, palm-muted between, diving at the
  phrase ends; a sus chord sample a half bar at a time, recoloured by the
  bass under it.
- Movement: its arpeggio ping-pongs hard left and right a note at a
  time; its bass falls and slides. Here: the 32nd-note arpeggio on the
  second square, each sixteenth struck twice (loud, then soft), panned
  hard to one side then the other; the bass's falls at the phrase ends.
- Harmony: two chords a bar, sevenths and suspensions, Aeolian and
  Dorian (bVII, bVI, the IV major, the minor v), a fake-out chord.
  Here, in G minor: i bVII | bVI IV | i bVII | bVI v | i bIII | bVI bVII
  | bII(maj7) bIII | IVsus4-IV Vsus4-V, then i: each suspension
  resolving on the beat, the Dorian IV's E natural in the melody.
- A hook that comes back: a one-bar motif (two hammered G's, a leap of a
  fifth, a turn back, a long note held with vibrato that pushes across
  the bar line), answered, stated again, cadenced; a B section that
  climbs in sequence on a hammered figure; the motif again, on the
  fake-out chord.

150 beats a minute (a bar is 48 frames at 30 fps), G minor. Bar 0 the
count-in (the arpeggio alone, the beat from beat three, a riser and the
pickup); bars 1-4 the hook, its answer, the hook, a cadence onto v; bars
5-6 the climb; bars 7-8 the hook on bII and the turnaround through IV
and V's suspensions; bar 9 i, hit and ringing, the power chord diving.

Instruments: all the kit's own (tools/trailer_music.py), the toms 0.3.0's.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import trailer_music as tm  # noqa: E402
from trailer_music import Score, key, keys  # noqa: E402
from trailers import v0_3_0_music as first  # noqa: E402

BPM = 150
BARS = 10
TAIL = 1.4
MIX = (('lead', ('sq1',), 0.11), ('arp', ('sq2',), 0.19), ('bass', ('wave',), 0.17),
       ('hats', ('noise',), 0.14), ('fx', ('fx',), 0.08))
DS_LEVEL = 1.0
REVERB = 40
HARDWARE = 32768
CHORD_SAMPLES = {'pad': (0, 2, 7, 12, 14), 'pad_m': (0, 3, 7, 12, 15), 'pad_M': (0, 4, 7, 12, 16)}
BANNER = (9 * 4 * 60 / BPM - 1.0, 2.9)

DS_ENV = {'kick': (255, 250, 190, 208), 'clap': (255, 250, 190, 208), 'tom_hi': (255, 0, 255, 230),
          'tom_mid': (255, 0, 255, 230), 'tom_lo': (255, 0, 255, 230), 'gtr': (255, 0, 255, 188),
          'mute': (255, 0, 255, 160), 'pad': (40, 0, 230, 200), 'pad_m': (40, 0, 230, 200),
          'pad_M': (40, 0, 230, 200),
          'impact': (255, 0, 255, 220), 'dlead': (255, 0, 224, 188)}
DS_GAIN = {'kick': 1.0, 'clap': 0.8, 'tom_hi': 0.6, 'tom_mid': 0.6, 'tom_lo': 0.65, 'gtr': 0.45, 'mute': 0.38,
           'pad': 0.9, 'pad_m': 0.9, 'pad_M': 0.9, 'impact': 0.7, 'dlead': 0.16}

LEAD = dict(duty=0.5, env=(0, 0, 11, 3), vib=(26, 10), pan=0.0)
ARP = dict(duty=0.25, env=(0, 7, 3, 1))
SQUARE = [15] * 13 + [8] + [0] * 17 + [8]
BASS = dict(env=(0, 0, 15, 1), wave=SQUARE)
HAT = dict(clock=262144, env=(0, 0, 11, 0), pan=0.25)
OPEN = dict(clock=262144, env=(0, 2, 10, 3), pan=0.3)
CRASH = dict(clock=80000, env=(0, 7, 0, 1), pan=-0.2)


def instruments():
    kit = first.instruments()
    S = {name: kit[name] for name in ('tom_hi', 'tom_mid', 'tom_lo')}
    S['kick'] = tm.punch_kick(low=74, high=160, drive=6.0, body=0.07)
    S['clap'] = tm.punch_clap()
    S['gtr'] = tm.power_stab(root=key('G2'))
    S['mute'] = tm.power_stab(root=key('G2'), seconds=0.12, mute=True)
    S['pad'] = tm.chord_pad(CHORD_SAMPLES['pad'])       # sus2: the third left to the bass and lead
    S['pad_m'] = tm.chord_pad(CHORD_SAMPLES['pad_m'])   # minor, to sit a third over a bass (maj7)
    S['pad_M'] = tm.chord_pad(CHORD_SAMPLES['pad_M'])   # major, to sit a third over a bass (m7)
    S['impact'] = tm.impact()
    S['dlead'] = tm.cycle_lead()
    return S


# ---- the harmony: two chords a bar ----
# (bass root, arpeggio's two octaves, the pad's sample and its root): a
# triad sample a third over the bass makes the seventh chord
H = {
    'Gm': ('G2', 'G3 Bb3 D4 F4 G4 Bb4 D5 F5 G5', 'pad_M', 'Bb3'),       # Bb over G: Gm7
    'F': ('F2', 'F3 A3 C4 F4 G4 A4 C5 F5 G5', 'pad', 'F3'),             # Fsus2
    'Eb': ('Eb2', 'Eb3 G3 Bb3 D4 Eb4 G4 Bb4 D5 Eb5', 'pad_m', 'G3'),    # Gm over Eb: Ebmaj7
    'C': ('C3', 'C3 E3 G3 Bb3 C4 E4 G4 Bb4 C5', 'pad_M', 'C4'),         # the Dorian IV, major
    'Dm': ('D2', 'D3 F3 A3 C4 D4 F4 A4 C5 D5', 'pad_M', 'F3'),          # F over D: Dm7
    'Bb': ('Bb2', 'Bb2 D3 F3 A3 Bb3 D4 F4 A4 Bb4', 'pad_m', 'D4'),      # Dm over Bb: Bbmaj7
    'Ab': ('Ab2', 'Ab2 C3 Eb3 G3 Ab3 C4 Eb4 G4 Ab4', 'pad_m', 'C4'),    # Cm over Ab: Abmaj7
    'Csus': ('C3', 'C3 F3 G3 C4 F4 G4 C5 F5 G5', 'pad', 'F3'),          # F G C over C: Csus4
    'Dsus': ('D2', 'D3 G3 A3 D4 G4 A4 D5 G5 A5', 'pad', 'G3'),          # G A D over D: Dsus4
    'D': ('D2', 'D3 F#3 A3 C4 D4 F#4 A4 C5 D5', 'pad_M', 'D4'),
}
CHORDS = [None, ('Gm', 'F'), ('Eb', 'C'), ('Gm', 'F'), ('Eb', 'Dm'), ('Gm', 'Bb'), ('Eb', 'F'), ('Ab', 'Bb'),
          ('Csus', 'C', 'Dsus', 'D')]

# ---- the hook and its answers: (tick, ticks, note) ----
HOOK = [(0, 6, 'G5'), (6, 6, 'G5'), (12, 12, 'D6'), (24, 6, 'C6'), (30, 6, 'Bb5'), (36, 12, 'C6'), (48, 36, 'D6')]
LINES = {
    1: HOOK + [(84, 6, 'C6'), (90, 6, 'D6')],
    2: [(0, 12, 'Eb6'), (12, 6, 'D6'), (18, 18, 'Bb5'), (36, 6, 'G5'), (42, 6, 'Bb5'), (48, 6, 'C6'), (54, 6, 'D6'),
        (60, 30, 'E6'), (90, 6, 'D5')],
    3: HOOK + [(84, 6, 'C6'), (90, 6, 'Bb5')],
    4: [(0, 18, 'G5'), (18, 6, 'F5'), (24, 12, 'G5'), (36, 12, 'Bb5'), (48, 12, 'A5'), (60, 30, 'D5'), (90, 6, 'C6')],
    5: [(0, 6, 'D6'), (6, 6, 'D6'), (12, 6, 'C6'), (18, 6, 'D6'), (24, 12, 'F6'), (36, 12, 'D6'), (48, 6, 'F6'),
        (54, 6, 'F6'), (60, 6, 'Eb6'), (66, 6, 'F6'), (72, 24, 'F6')],
    6: [(0, 6, 'Eb6'), (6, 6, 'Eb6'), (12, 6, 'D6'), (18, 6, 'Eb6'), (24, 12, 'G6'), (36, 12, 'Eb6'), (48, 6, 'F6'),
        (54, 6, 'F6'), (60, 6, 'Eb6'), (66, 6, 'F6'), (72, 18, 'A6'), (90, 6, 'D5')],
    7: [(0, 6, 'G5'), (6, 6, 'G5'), (12, 12, 'Eb6'), (24, 6, 'C6'), (30, 6, 'Bb5'), (36, 12, 'C6'), (48, 36, 'D6'),
        (84, 6, 'Eb6'), (90, 6, 'E6')],
    8: [(0, 12, 'F6'), (12, 12, 'G6'), (24, 12, 'E6'), (36, 12, 'G6'), (48, 12, 'G6'), (60, 12, 'A6'), (72, 12, 'F#6'),
        (84, 12, 'A6')],
}
# the long notes that fall away at their end (a phrase's last note)
FALLS = {(2, 60), (4, 60)}


def lead(s, bar, line, voice=LEAD, vel=118, octave=0):
    for t, n, k in line:
        o = dict(voice)
        if n >= 24 and (bar, t) in FALLS:
            o['bend'] = [(0, 0), (n - 8, 0), (n, -2)]
        s.note('sq1', bar, t, n, key(k) + octave, vel - (10 if n <= 6 and t % 12 else 0), **o)


FIGURE = [0, 2, 4, 3, 5, 4, 6, 5]   # three-note turns against the sixteenths


def arp(s, bar, t0, t1, voicing, vel=104, soft=54, figure=FIGURE, lift=0):
    """Each sixteenth struck twice as 32nds, loud then soft, and thrown
    hard left, then hard right."""
    v = keys(voicing)
    for i, t in enumerate(range(t0, t1, 6)):
        k = v[figure[i % len(figure)] % len(v)] + lift
        side = -0.9 if (t // 6) % 2 == 0 else 0.9
        s.note('sq2', bar, t, 3, k, vel, pan=side, **ARP)
        s.note('sq2', bar, t + 3, 3, k, soft, pan=side, **ARP)


KICK = (0, 6, 8, 11, 14)       # sixteenths of the bar
CLAP = (4, 12)
OCTAVES = (3, 9, 13)


def drums(s, bar, until=16, ghost=False, crash=False, open_at=(10,)):
    for p in KICK:
        if p < until:
            s.hit('kick', bar, 6 * p, 6, vel=120)
    for p in CLAP:
        if p < until:
            s.hit('clap', bar, 6 * p, 6, vel=118, pan=0.05)
    if ghost and until == 16:
        s.hit('clap', bar, 90, 6, vel=60, pan=-0.2)
    for p in range(until):
        if p in open_at:
            s.note('noise', bar, 6 * p, 6, 0, 96, **OPEN)
        else:
            s.note('noise', bar, 6 * p, 3, 0, (112, 40, 76, 40)[p % 4], **HAT)
    if crash:
        s.note('fx', bar, 0, 48, 0, 124, **CRASH)


def bass(s, bar, root, t0=0, t1=96, fall=False):
    """The root on the kick's places, its octave three times between, each
    held nearly to the next (BN6's bass is legato four notes in five); a
    fall at the phrase's end."""
    r = key(root)
    end = t1 - 12 if fall else t1
    places = [p for p in range(t0 // 6, end // 6) if p in KICK or p in OCTAVES]
    for i, p in enumerate(places):
        nxt = places[i + 1] if i + 1 < len(places) else end // 6
        n = 6 * (nxt - p) - 1
        if p in KICK:
            s.note('wave', bar, 6 * p, n, r, 124, **BASS)
        else:
            s.note('wave', bar, 6 * p, min(n, 5), r + 12, 92, **BASS)
    if fall:
        s.note('wave', bar, t1 - 12, 12, r + 12, 120, **dict(BASS, bend=[(0, 0), (4, 0), (12, -12)]))


def chugs(s, bar, root, t0=0, t1=96, dive=False):
    """The power chord with the kick, palm-muted on the sixteenths
    between, still on the backbeat."""
    k = key(root) + 12
    while k > key('C3'):
        k -= 12
    for p in range(t0 // 6, (t1 - 12 if dive else t1) // 6):
        if p in KICK:
            s.hit('gtr', bar, 6 * p, 6 if p in (6, 11) else 9, k, 112, -0.6)
        elif p not in CLAP and p % 2 == 0:
            s.hit('mute', bar, 6 * p, 4, k, 96, 0.6)
    if dive:
        s.hit('gtr', bar, t1 - 12, 12, k, 118, -0.35, bend=[(0, 0), (3, 0), (12, -5)])


def half(s, bar, t0, name, length=48, dive=False, fall=False):
    root, voicing, pad, pad_root = H[name]
    s.hit(pad, bar, t0, length - 2, key(pad_root) + 12, 100, -0.8 if t0 == 0 else 0.8)
    arp(s, bar, t0, t0 + length, voicing)
    bass(s, bar, root, t0, t0 + length, fall)
    chugs(s, bar, root, t0, t0 + length, dive)


def count_in(s):
    """Bar 0: the arpeggio alone on i, the beat from beat three, a riser,
    a tom pickup and the lead's two notes before the hook."""
    arp(s, 0, 0, 96, H['Gm'][1], vel=96, soft=48)
    s.note('fx', 0, 0, 92, 0, 110, sweep=(2500, 200000), ramp=(1, 11))
    s.hit('kick', 0, 48, 6)
    s.hit('clap', 0, 48, 6)
    for i, t in enumerate(range(48, 84, 6)):
        s.note('noise', 0, t, 3, 0, 60 + 8 * i, **HAT)
    for t, tom, pan in ((72, 'tom_hi', -0.4), (78, 'tom_mid', 0.0), (84, 'tom_lo', 0.4), (90, 'tom_lo', 0.4)):
        s.hit(tom, 0, t, 6, vel=116, pan=pan)
    s.note('wave', 0, 48, 36, 'G2', 120, **BASS)
    s.note('sq1', 0, 84, 6, 'D5', 108, **LEAD)
    s.note('sq1', 0, 90, 6, 'F5', 112, **LEAD)


def theme(s):
    for bar in range(1, 9):
        ch = CHORDS[bar]
        if len(ch) == 2:
            half(s, bar, 0, ch[0])
            half(s, bar, 48, ch[1], dive=bar in (4, 8), fall=bar in (4, 8))
        else:
            for i, name in enumerate(ch):
                half(s, bar, 24 * i, name, 24, dive=i == 3)
        drums(s, bar, until=12 if bar in (4, 8) else 16, ghost=bar % 2 == 0, crash=bar in (1, 5, 7))
        lead(s, bar, LINES[bar])
        if bar >= 5:   # the climb and the return thickened: the lead's tone an octave under
            for t, n, k in LINES[bar]:
                s.hit('dlead', bar, t, n, key(k) - 12, 100, 0.25)
    s.hit('impact', 1, 0, 24, vel=120)
    s.hit('impact', 5, 0, 24, vel=110)
    # the fills: toms into bar 5, a 32nd-note clap roll into the end
    for t, tom, pan in ((72, 'tom_hi', -0.4), (78, 'tom_hi', -0.4), (84, 'tom_mid', 0.0), (90, 'tom_lo', 0.4)):
        s.hit(tom, 4, t, 6, vel=118, pan=pan)
    for i, t in enumerate(range(72, 96, 3)):
        s.hit('clap', 8, t, 3, vel=70 + 6 * i, pan=-0.3 + 0.08 * i)


def ending(s):
    """Bar 9: i hit and ringing; the power chord dives; the lead's G held,
    falling at its end."""
    s.note('fx', 9, 0, 96, 0, 127, **CRASH)
    s.hit('impact', 9, 0, 48, vel=127)
    s.hit('kick', 9, 0, 6)
    s.hit('clap', 9, 0, 6)
    s.hit('pad', 9, 0, 96, 'G3', 112, -0.5)
    s.hit('pad_m', 9, 0, 96, 'G3', 100, 0.5)
    s.hit('gtr', 9, 0, 90, key('G2'), 124, 0.0, bend=[(0, 0), (40, 0), (90, -12)])
    s.note('wave', 9, 0, 84, 'G2', 124, **dict(BASS, env=(0, 0, 15, 3), bend=[(0, 0), (48, 0), (84, -12)]))
    s.note('sq1', 9, 0, 84, 'G6', 120, **dict(LEAD, bend=[(0, 0), (60, 0), (84, -3)]))
    arp(s, 9, 0, 48, H['Gm'][1], vel=100, soft=50, figure=list(range(9)))


def arrangement():
    s = Score()
    count_in(s)
    theme(s)
    ending(s)
    return s
