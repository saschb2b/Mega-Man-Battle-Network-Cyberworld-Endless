"""What 0.21.0's sketches share (sketch_mix_a, _b and _c; not a piece of
its own): three games meet in the release, so each sketch grows from one
game's song, its habits read from that game's ROM with song_habits.py
--detail --theory (numbers only, never its notes), and played with
sketch_red_lib's parts, every melodic one on FluidR3 GM.

- BN6's title (0x01, 142-146 beats a minute, E minor): the lead's long
  notes moving by step (53% steps, a quarter repeated, twelve ticks the
  median), a square's 32nd arpeggio at 75% thrown hard left and right 89
  times, the wave's sixteenths with octave jumps, a march in the kit (the
  low drum on 0 2 4 11, toms on 6 9 12 15, the crash on 2); maj7 23% of
  its half bars, m7 17, sus2 14; roots by fourths and falling semitones.
- BN5's boss battle (0x16, 152, D minor): half-diminished chords on 30%
  of its half bars, two chords in three with a tone outside the key, roots
  by minor thirds and semitones; the lead hammering a note and moving by
  step (41% repeated, 39 steps); a square's sixteenth arpeggio leaping
  wide (keys 38-94); a sampled voice on one note in a gallop (95%
  repeated, the beat's second and third sixteenths twice as often as its
  first and fourth); another pinging octaves (50% big leaps, 37 repeated);
  the wave's bass high (keys 48-64) in sixteenths, 41% repeated; the kick
  on 0 2 5 8 12, the snare on the off-beats 6 and 14.
- BN4's title (0x00, 146, C major with the borrowed iv and a
  half-diminished ii): one chord and a third a bar, the root held half the
  time; the lead on both squares, the second eight ticks behind the first,
  four notes in five by step, eleven ticks the median; a sampled voice on
  one note in eighths (92% repeated); a held chord sample; the wave's bass
  low (keys 38-48) in sixteenths; the kick on 0 2 4 8 10 12, the snare on
  6 and 14, open hats on the beat.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from trailer_music import key  # noqa: E402
from trailers import sketch_red_lib as L  # noqa: E402
from trailers.sketch_red_lib import (CHORD_SAMPLES, DS_ENV, DS_GAIN, DS_LEVEL,  # noqa: E402,F401
                                     HARDWARE, MIX, REVERB, SAMPLED_LEADS, SAMPLED_PARTS, SOUNDFONT, TAIL,
                                     instruments)

V = L.V
BLIP75 = dict(duty=0.75, env=(0, 1, 9, 1))

KICKS = {'march': ((0, 112), (2, 84), (9, 100), (12, 108)),
         'boss': ((0, 112), (2, 92), (5, 100), (8, 108), (12, 104)),
         'anthem': ((0, 112), (2, 88), (4, 100), (8, 108), (10, 88), (12, 100))}
SNARES = {'march': ((4, 104), (11, 60), (14, 96)), 'boss': ((6, 108), (14, 108)), 'anthem': ((6, 104), (14, 108))}
TOMS = {'march': ((6, 'tom_hi', 70), (15, 'tom_lo', 76)), 'boss': (), 'anthem': ()}
OPENS = {'march': (2, 10), 'boss': (2, 10), 'anthem': (0, 4, 8, 12)}


def kit(s, bar, style, until=16, crash=False, hit=False):
    """'march' BN6's title's, 'boss' BN5's 0x16's (the snare on the
    off-beats), 'anthem' BN4's title's (the kick in eighths, open hats on
    the beat); hats on every sixteenth at three levels"""
    for p, v in KICKS[style]:
        if p < until:
            s.hit('kick', bar, 6 * p, 6, vel=v)
    for p, v in SNARES[style]:
        if p < until:
            s.hit('clap', bar, 6 * p, 6, vel=v, pan=-0.05)
            s.hit('snare', bar, 6 * p, 6, vel=max(30, v - 10), pan=0.05)
    for p, tom, v in TOMS[style]:
        if p < until:
            s.hit(tom, bar, 6 * p, 6, vel=v, pan=-0.3 if tom == 'tom_hi' else 0.3)
    for p in range(until):
        if p in OPENS[style]:
            s.note('noise', bar, 6 * p, 6, 0, 76, **V.OPEN)
        else:
            s.note('noise', bar, 6 * p, 3, 0, (90, 44, 64, 44)[p % 4], **V.HAT)
    if crash:
        s.note('fx', bar, 0, 64, 0, 108, **V.CRASH)
    if hit:
        s.hit('hit', bar, 0, 24, 'D4', 110, 0.0)


def arp16(s, bar, names, until=16, lo=60, vel=0):
    """a sixteenth arpeggio leaping wide over two octaves of the chord
    (BN5's 0x16's square), the xylophone over square 2, thrown left and
    right note by note"""
    order = (0, 3, 1, 4, 2, 5, 3, 6, 4, 1, 5, 2, 6, 3, 4, 0)
    for p in range(until):
        nm = L.at(names, 6 * p)
        ts = sorted(L.near(t % 12, lo) for t in L.tones(nm)[:4])
        ladder = sorted(set(ts + [k + 12 for k in ts]))
        k = ladder[order[p] % len(ladder)]
        side = -0.9 if p % 2 == 0 else 0.9
        v = (100 if p % 4 == 0 else 80) + vel
        s.hit('mallet', bar, 6 * p, 5, k, v, side)
        s.note('sq2', bar, 6 * p, 5, k, max(16, v - 24), pan=side, **BLIP75)


def blips(s, bar, names, until=16, lo=72, vel=0):
    """one note in eighths, the chord's fifth (BN4's title's sampled
    voice), the xylophone over square 2, thrown left and right"""
    for p in range(0, until, 2):
        nm = L.at(names, 6 * p)
        ts = L.tones(nm)
        k = L.near(ts[2] % 12, lo)
        side = -0.9 if (p // 2) % 2 == 0 else 0.9
        v = (96 if p % 4 == 0 else 72) + vel
        s.hit('mallet', bar, 6 * p, 5, k, v, side)
        s.note('sq2', bar, 6 * p, 5, k, max(16, v - 24), pan=side, **L.BLIP)


def pulse(s, bar, names, until=16, vel=84):
    """the gallop on one note (BN5's 0x16): the palm-muted guitar on the
    chord's root, the beat's second and third sixteenths, the bar's one"""
    for p in range(until):
        if not (p % 4 in (1, 2) or p == 0 or p == 8):
            continue
        r = V.low(L.bass_root(L.at(names, 6 * p)) + 12) + 12
        s.hit('mute', bar, 6 * p, 5, r, vel + (12 if p % 4 == 1 else 0), 0.0)


def pings(s, bar, names, until=16, vel=72):
    """a voice pinging octaves in sixteenths (BN5's 0x16), the electric
    piano on the chord's fifth, wide"""
    fig = (0, 0, 12, 0, 12, 0, 0, 12)
    for p in range(until):
        ts = L.tones(L.at(names, 6 * p))
        k = L.near(ts[2] % 12, 62) + fig[p % 8]
        s.hit('ep2', bar, 6 * p, 5, k, vel - (14 if p % 2 else 0), -0.6 if p % 4 < 2 else 0.6)


def bass(s, bar, names, nxt=None, style='gallop', until=16):
    """'gallop' BN5's 0x16's high sixteenths, the root repeated with an
    octave and a step back into it; 'pedal' BN4's title's low sixteenths on
    the bar's first root, held under its second chord; the wave doubling it"""
    notes = []
    for t0, n, nm in L.halves(names):
        r = L.bass_root(nm) + (12 if style == 'gallop' else 0)
        steps = n // 6
        for i in range(steps):
            if style == 'gallop':
                k = (r, r, r, r + 12, r, r, r - 2, r - 1)[i % 8] if steps == 8 else (r, r, r + 12, r)[i % 4]
            else:
                k = L.bass_root(names[0])        # (the bar's first root held under its second chord)
            notes.append((t0 + 6 * i, 5, k, 118 if i % 2 == 0 else 96))
    for t, ln, k, v in notes:
        if t < 6 * until:
            s.hit('sfbass', bar, t, ln, k, v, 0.0)
            s.note('wave', bar, t, max(2, ln - 1), k, v, **L.WAVE)


def lead(s, bar, line, vel=112, side=-1, delay=8):
    """the lead on both squares, the second `delay` ticks behind the first
    from the other side (BN4's title); the saw over them"""
    for item in line:
        t, n, k = item[:3]
        k = key(k)
        fall = item[3] if len(item) > 3 else 0
        bend = None
        if fall > 0:
            bend = [(0, -fall), (min(4, n // 3), 0)]
        elif fall < 0:
            bend = [(0, 0), (n * 2 // 3, 0), (n, fall)]
        v = vel - (10 if t % 12 else 0)
        s.hit('sflead', bar, t, n, k, v, 0.3 * side, bend)
        s.note('sq1', bar, t, n, k, v - 16, pan=0.3 * side, **(dict(L.LEAD, bend=bend) if bend else L.LEAD))
        if t + delay < 96:
            s.hit('echo', bar, t + delay, max(2, min(n, 96 - t - delay)), k, v - 20, -0.8 * side, bend)


def play(s, chords, plan):
    """sketch_red_lib's play, and these parts beside it: kit2 (a style
    here), arp2 ('arp16', 'blips'), bass2 ('gallop', 'pedal'), pulse, pings,
    lead2 (the lead with its late twin)"""
    L.play(s, chords, plan)
    for bar, p in sorted(plan.items()):
        if p.get('ring') or p.get('brk'):
            continue
        names = chords[bar]
        until = 12 if p.get('fill') == 'toms' else 16
        if p.get('kit2'):
            kit(s, bar, p['kit2'], until=until, crash=p.get('crash', False), hit=p.get('hit', False))
        if p.get('arp2') == 'arp16':
            arp16(s, bar, names)
        elif p.get('arp2') == 'blips':
            blips(s, bar, names)
        if p.get('bass2'):
            bass(s, bar, names, style=p['bass2'], until=until)
        if p.get('pulse'):
            pulse(s, bar, names, until=until)
        if p.get('pings'):
            pings(s, bar, names)
        if p.get('lead2'):
            lead(s, bar, p['lead2'], vel=p.get('lead_vel', 112), side=p.get('side', -1))
            if p.get('bell'):
                for it in p['lead2']:
                    s.hit('bell', bar, it[0], min(it[1], 12), key(it[2]) + 12, 70, 0.5)
