#!/usr/bin/env python3
"""A song's habits, read from the developer's ROM with an MP2K reader, for an
original trailer piece written in its manner, never from its notes. It
prints numbers only and writes nothing: tempo, each track's role (its
voice's kind, envelope and sample, its note lengths and range), the key's
pitch classes, the drums' places in the bar and the form, bar by bar.

    python3 .claude/skills/trailer/scripts/song_habits.py GAME SONG [--offset TICKS] [--bars] [--detail] [--theory]

GAME is bn6 or bn5 (the ROMs told by their SHA-1 in build.py's ROM folder),
SONG the song's number in its game's table (docs/ROM_DATA.md: BN6's at
0x159F48, BN5's at 0x155BF4; a title's or battle's number from the
disassembly or by listening with `build.py shot --render-song`). A song
that starts on a pickup counts its bars late: --offset TICKS moves the bar
line (the first track's first downbeat, read off the first notes printed),
and the drums' places then read as the bar's. --bars prints how many notes
each track plays in each bar: the intro, a breakdown (a track alone) and
the main sections show as their densities change. --detail prints how
each track plays (velocities, the gate of its notes, its volume, pan,
bend and vibrato controls) and what the drum kit's keys are (a sample or
the noise channel); --theory the harmony and melody in aggregates: the
key, the chords as degrees and qualities, the roots' moves, the harmonic
rhythm, each part's intervals, syncopation and repetition.
"""
import collections
import glob
import hashlib
import os
import struct
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', '..'))
sys.path.insert(0, ROOT)
import build  # noqa: E402  (the ROM folder and the ROMs' SHA-1)

TABLES = {'bn6': (build.BN6_SHA1, 0x159F48), 'bn5': (build.BN5_SHA1, 0x155BF4)}
LEN = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 28, 30, 32, 36, 40,
       42, 44, 48, 52, 54, 56, 60, 64, 66, 68, 72, 76, 78, 80, 84, 88, 90, 92, 96]
KINDS = {0: 'Direct Sound', 8: 'Direct Sound (fixed)', 1: 'square 1', 2: 'square 2', 3: 'wave', 4: 'noise',
         9: 'square 1', 10: 'square 2', 11: 'wave', 12: 'noise', 0x40: 'key split', 0x80: 'drum kit'}
NAMES = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B']
CONTROLS = {0xBE: 'volume', 0xBF: 'pan', 0xC0: 'bend', 0xC1: 'bend range', 0xC2: 'LFO speed', 0xC3: 'LFO delay',
            0xC4: 'modulation', 0xC5: 'modulation type', 0xC8: 'tune'}


def rom(game):
    sha, table = TABLES[game]
    for path in sorted(glob.glob(os.path.join(build.default_rom_dir(), '*.gba'))):
        with open(path, 'rb') as f:
            data = f.read()
        if hashlib.sha1(data).hexdigest() == sha:
            return data, table
    sys.exit(f'no {game} ROM in {build.default_rom_dir()}')


class Song:
    def __init__(self, data, table, n):
        self.R = data
        hdr = self.word(table + 8 * n) - 0x08000000
        self.tracks_n, _, _, self.reverb = data[hdr:hdr + 4]
        self.vg = self.word(hdr + 4) - 0x08000000
        self.tracks = [self.word(hdr + 8 + 4 * i) - 0x08000000 for i in range(self.tracks_n)]

    def word(self, a):
        return struct.unpack_from('<I', self.R, a)[0]

    def voice(self, v):
        """A voice's kind, base key, envelope and, for Direct Sound, its
        sample's size, loop and rate; for a square its duty."""
        return self.voice_at(self.vg + 12 * v)

    def drum(self, v, k):
        """The voice a drum kit plays for key k."""
        return self.voice_at(self.word(self.vg + 12 * v + 4) - 0x08000000 + 12 * k)

    def voice_at(self, a):
        R = self.R
        kind, base = R[a], R[a + 1]
        adsr = tuple(R[a + 8:a + 12])
        info = {'kind': KINDS.get(kind, hex(kind)), 'base': base, 'adsr': adsr}
        if kind in (0, 8):
            sp = self.word(a + 4) - 0x08000000
            flags, freq, loop, size = struct.unpack_from('<IIII', R, sp)
            s = [x - 256 if x > 127 else x for x in R[sp + 16:sp + 16 + size]]
            crossings = sum(1 for i in range(1, len(s)) if (s[i - 1] < 0) != (s[i] < 0))
            info.update(size=size, loop=loop if flags & 0x40000000 else None, rate=round(freq / 1024),
                        brightness=round(1000 * crossings / max(1, len(s))))
        elif kind in (1, 2, 9, 10):
            info['duty'] = ('12.5%', '25%', '50%', '75%')[R[a + 4] & 3]
        return info

    def walk(self, pc, limit=60000):
        """A track's notes (tick, key, length, velocity, voice), its tempo
        changes, followed once round its loop."""
        R = self.R
        t = running = loops = 0
        stack, repeats, notes, tempo = [], {}, [], []
        self.controls = []
        voice, shift, key, vel = None, 0, 60, 100
        for _ in range(200000):
            if t > limit:
                break
            c = R[pc]
            if c < 0x80:
                c = running
            else:
                pc += 1
                if c >= 0xBD:
                    running = c
            if 0x80 <= c <= 0xB0:
                t += LEN[c - 0x80]
            elif c >= 0xCF:
                if R[pc] < 0x80:
                    key, pc = R[pc], pc + 1
                    if R[pc] < 0x80:
                        vel, pc = R[pc], pc + 1
                        if c != 0xCF and R[pc] < 0x80:
                            pc += 1
                notes.append((t, key + shift, 0 if c == 0xCF else LEN[c - 0xCF], vel, voice))
            elif c == 0xB1:
                break
            elif c == 0xB2:
                loops += 1
                if loops > 1:
                    break
                pc = self.word(pc) - 0x08000000
            elif c == 0xB3:
                stack.append(pc + 4)
                pc = self.word(pc) - 0x08000000
            elif c == 0xB4:
                if stack:
                    pc = stack.pop()
            elif c == 0xB5:
                count, dest = R[pc], self.word(pc + 1) - 0x08000000
                k = repeats.get(pc, 0) + 1
                if count == 0 or k < count:
                    repeats[pc], pc = k, dest
                else:
                    repeats[pc], pc = 0, pc + 5
            elif c == 0xB9:
                pc += 3
            elif c == 0xCD:
                pc += 2
            else:
                arg, pc = R[pc], pc + 1
                if c == 0xBB:
                    tempo.append((t, arg * 2))
                elif c == 0xBD:
                    voice = arg
                elif c == 0xBC:
                    shift = arg if arg < 64 else arg - 256
                elif c in CONTROLS:
                    self.controls.append((t, c, arg))
                elif c == 0xCE and R[pc] < 0x80:
                    pc += 1
        return notes, tempo, self.controls


def articulation(notes, controls):
    """How a track's notes are played: velocities (one level, a few, or
    many: humanised), the gate (a note's length against the time to the
    next: staccato or legato), and the controls it sets (volume swells,
    pans, bends, vibrato)."""
    vels = collections.Counter(n[3] for n in notes)
    gates = []
    for a, b in zip(notes, notes[1:]):
        gap = b[0] - a[0]
        if gap > 0 and a[2]:
            gates.append(min(1.0, a[2] / gap))
    gates.sort()
    out = [f'velocities {min(vels)}-{max(vels)}, {len(vels)} levels (commonest {vels.most_common(3)})']
    if gates:
        short = 100 * sum(g < 0.5 for g in gates) // len(gates)
        legato = 100 * sum(g >= 0.95 for g in gates) // len(gates)
        out.append(f'gate median {gates[len(gates) // 2]:.2f}, under half {short}%, legato {legato}%')
    by = collections.defaultdict(list)
    for _, c, arg in controls:
        by[CONTROLS[c]].append(arg)
    for name, args in by.items():
        out.append(f'{name} x{len(args)} {collections.Counter(args).most_common(4)}')
    return out


TEMPLATES = {'': (0, 4, 7), 'm': (0, 3, 7), 'dim': (0, 3, 6), 'aug': (0, 4, 8), 'sus4': (0, 5, 7), 'sus2': (0, 2, 7),
             '7': (0, 4, 7, 10), 'm7': (0, 3, 7, 10), 'maj7': (0, 4, 7, 11), 'm7b5': (0, 3, 6, 10), '5': (0, 7)}
DEGREES = ['I', 'bII', 'II', 'bIII', 'III', 'IV', '#IV', 'V', 'bVI', 'VI', 'bVII', 'VII']
MAJOR = (6.35, 2.23, 3.48, 2.33, 4.38, 4.09, 2.52, 5.19, 2.39, 3.66, 2.29, 2.88)
MINOR = (6.33, 2.68, 3.52, 5.38, 2.60, 3.53, 2.54, 4.75, 3.98, 2.69, 3.34, 3.17)


def tonic(weights):
    """The key (Krumhansl's profiles): (pitch class, 'major' or 'minor')."""
    def corr(profile, r):
        xs = [weights[(r + i) % 12] for i in range(12)]
        mx, my = sum(xs) / 12, sum(profile) / 12
        num = sum((x - mx) * (y - my) for x, y in zip(xs, profile))
        den = (sum((x - mx) ** 2 for x in xs) * sum((y - my) ** 2 for y in profile)) ** 0.5 or 1
        return num / den
    return max(((r, m) for r in range(12) for m in ('major', 'minor')),
               key=lambda rm: corr(MAJOR if rm[1] == 'major' else MINOR, rm[0]))


def chords_of(melodic, bass_tracks, offset, span):
    """The chord each window of `span` ticks: a template matched to the
    notes sounding, the bass's lowest note weighted as the root."""
    end = max(n[0] + (n[2] or 24) for ns in melodic.values() for n in ns)
    windows = [[0.0] * 12 for _ in range(end // span + 1)]
    lows = [None] * len(windows)
    total = [0.0] * 12
    for (ti, _), ns in melodic.items():
        for t, k, n, vel, _ in ns:
            n = n or 24
            for w in range(max(0, (t - offset) // span), min(len(windows), (t - offset + n) // span + 1)):
                a, b = max(t - offset, w * span), min(t - offset + n, (w + 1) * span)
                if b > a:
                    windows[w][k % 12] += (b - a) * vel / 127
                    total[k % 12] += b - a
                    if ti in bass_tracks and (lows[w] is None or k < lows[w][0]):
                        lows[w] = (k, k % 12)
    chords = []
    for w, pcs in enumerate(windows):
        if sum(pcs) < 1:
            continue
        best = None
        for r in range(12):
            for q, iv in TEMPLATES.items():
                fit = sum(pcs[(r + i) % 12] for i in iv) / len(iv) ** 0.5 - 0.35 * sum(
                    pcs[p] for p in range(12) if (p - r) % 12 not in iv) / 3
                if lows[w] and lows[w][1] == r:
                    fit *= 1.25
                if best is None or fit > best[0]:
                    best = (fit, r, q)
        chords.append((best[1], best[2]))
    return chords, tonic(total)


def theory(melodic, bass_tracks, offset, span=48):
    """Harmony and melody in numbers: the chords as degrees of the key with
    their qualities, the roots' moves, the chords outside the key, the
    harmonic rhythm; each melodic part's intervals, rhythm and repetition.
    Aggregates only: no sequence of the song's is printed."""
    chords, (home, mode) = chords_of(melodic, bass_tracks, offset, span)
    scale = (0, 2, 4, 5, 7, 9, 11) if mode == 'major' else (0, 2, 3, 5, 7, 8, 10)
    deg = collections.Counter(DEGREES[(r - home) % 12] + ('m' if q in ('m', 'm7') else q) for r, q in chords)
    qual = collections.Counter(q or 'major' for _, q in chords)
    moves = collections.Counter((b[0] - a[0]) % 12 for a, b in zip(chords, chords[1:]) if a[0] != b[0])
    held = sum(1 for a, b in zip(chords, chords[1:]) if a[0] == b[0])
    outside = sum(1 for r, q in chords if any((r - home + i) % 12 not in scale for i in TEMPLATES[q][:3]))
    n = len(chords) or 1
    print(f'\nkey: {NAMES[home]} {mode}; {n} half bars')
    print('  chords as degrees, %:', ', '.join(f'{d} {100 * c // n}' for d, c in deg.most_common(14)))
    print('  qualities, %:', ', '.join(f'{q} {100 * c // n}' for q, c in qual.most_common()))
    print(f'  the root held into the next half bar {100 * held // max(1, n - 1)}%; roots moving up by semitones, %:',
          ', '.join(f'+{m} {100 * c // max(1, sum(moves.values()))}' for m, c in sorted(moves.items())))
    print(f'  chords with a tone outside the key: {100 * outside // n}%')
    changes = sum(1 for a, b in zip(chords, chords[1:]) if a != b)
    print(f'  harmonic rhythm: {2 * changes / max(1, n - 1):.2f} chord changes a bar')
    for (ti, v), ns in sorted(melodic.items(), key=lambda kv: (kv[0][0], kv[0][1] or 0)):
        if len(ns) < 24 or ti in bass_tracks:
            continue
        ks = [k for _, k, *_ in ns]
        ivs = [b - a for a, b in zip(ks, ks[1:])]
        size = collections.Counter('same' if i == 0 else 'step' if abs(i) <= 2 else 'skip' if abs(i) <= 4 else 'leap'
                                   if abs(i) <= 7 else 'big leap' for i in ivs)
        up = sum(1 for i in ivs if i > 0)
        pos = collections.Counter(((t - offset) % 24) // 6 for t, *_ in ns if (t - offset) % 6 == 0)
        off = sum(1 for t, *_ in ns if (t - offset) % 6)
        lens = sorted(x[2] for x in ns)
        bars_ = collections.defaultdict(list)
        for t, k, *_ in ns:
            bars_[(t - offset) // 96].append(((t - offset) % 96, k))
        rhythms = collections.Counter(tuple(p for p, _ in b) for b in bars_.values())
        shapes = collections.Counter(tuple(b2[1] - b1[1] for b1, b2 in zip(b, b[1:])) for b in bars_.values())
        nb = len(bars_) or 1
        tot = len(ivs) or 1
        print(f'  track {ti} voice {v}: {len(ns)} notes, range {min(ks)}-{max(ks)},'
              f' median length {lens[len(lens) // 2]};'
              ' intervals %: ' + ', '.join(f'{k} {100 * c // tot}' for k, c in size.most_common()) +
              f'; up {100 * up // tot}%; onsets on the beat\'s sixteenths 1-4: '
              + '/'.join(str(pos.get(i, 0)) for i in range(4)) + (f' (+{off} off the grid)' if off else '') +
              f'; bars {nb}, distinct rhythms {len(rhythms)}, distinct shapes {len(shapes)},'
              f' the commonest rhythm in {100 * rhythms.most_common(1)[0][1] // nb}% of bars')


def main():
    args = sys.argv[1:]
    offset = 0
    if '--offset' in args:
        i = args.index('--offset')
        offset = int(args[i + 1], 0)
        del args[i:i + 2]
    flags = {a for a in args if a.startswith('--')}
    args = [a for a in args if not a.startswith('--')]
    bars, detail, do_theory = '--bars' in flags, '--detail' in flags, '--theory' in flags
    if len(args) != 2 or args[0] not in TABLES:
        sys.exit(__doc__)
    data, table = rom(args[0])
    song = Song(data, table, int(args[1], 0))
    print(f'{args[0]} song {args[1]}: {song.tracks_n} tracks, reverb {song.reverb & 0x7F}')
    walked = [song.walk(pc) for pc in song.tracks]
    tempos = sorted({bpm for _, tempo, _ in walked for _, bpm in tempo})
    print('tempo', tempos or '(none set)')
    pitch = collections.Counter()
    density = []
    melodic, bass_tracks = {}, set()
    for i, (notes, _, controls) in enumerate(walked):
        if not notes:
            print(f'track {i}: no notes')
            continue
        kinds = {v: song.voice(v) for v in {n[4] for n in notes if n[4] is not None}}
        drums = any(k['kind'] == 'drum kit' for k in kinds.values())
        keys = [n[1] for n in notes]
        lengths = collections.Counter(n[2] for n in notes).most_common(5)
        print(f'\ntrack {i}: {len(notes)} notes, keys {min(keys)}-{max(keys)}, lengths {lengths}, '
              f'first notes at {[n[0] for n in notes[:4]]}')
        for v, k in kinds.items():
            print(f'   voice {v} ({sum(1 for n in notes if n[4] == v)} notes): {k}')
        if detail:
            for line in articulation(notes, controls):
                print('   ' + line)
        if drums:
            places = collections.defaultdict(collections.Counter)
            for t, k, *_ in notes:
                places[k][((t - offset) % 96) // 6] += 1

            def score(off):
                at = collections.Counter((k, ((t - off) % 96) // 6) for t, k, *_ in notes if (t - off) % 6 == 0)
                return (at[(36, 0)] + at[(35, 0)] + at[(49, 0)] + at[(57, 0)]
                        + sum(at[(k, p)] for k in (38, 40) for p in (4, 12)))
            best = max(range(96), key=score)
            print(f'   the bar line, guessed (kick and crash on one, snare on two and four): --offset {best}')
            print('   drums by key, sixteenth of the bar: count')
            kit = next(v for v, k in kinds.items() if k['kind'] == 'drum kit')
            for k in sorted(places):
                extra = ''
                if detail:
                    d = song.drum(kit, k)
                    vel = collections.Counter(n[3] for n in notes if n[1] == k)
                    extra = (f'  [{d["kind"]}, adsr {d["adsr"]}' + (f', {d["size"]} samples at {d["rate"]} Hz,'
                             f' brightness {d["brightness"]}' if 'size' in d else '') +
                             f'; velocities {min(vel)}-{max(vel)} in {len(vel)} levels]')
                print(f'     key {k}: {dict(sorted(places[k].items()))}{extra}')
        else:
            pitch.update(n[1] % 12 for n in notes)
            if sum(keys) / len(keys) < 52:
                bass_tracks.add(i)
            for n in notes:
                melodic.setdefault((i, n[4]), []).append(n)
        per_bar = collections.Counter(max(0, n[0] - offset) // 96 for n in notes)
        density.append(per_bar)
    if pitch:
        total = sum(pitch.values())
        print('\npitch classes (melodic tracks):', ', '.join(f'{NAMES[p]} {100 * c // total}%'
                                                             for p, c in pitch.most_common()))
    if do_theory and melodic:
        theory(melodic, bass_tracks, offset)
    if bars:
        last = max(max(d) for d in density if d)
        print('\nnotes a bar, track by track:')
        for b in range(last + 1):
            print(f'  bar {b:2d}: ' + ' '.join(f'{d.get(b, 0):3d}' for d in density))


if __name__ == '__main__':
    main()
