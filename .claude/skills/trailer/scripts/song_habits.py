#!/usr/bin/env python3
"""A song's habits, read from the developer's ROM with an MP2K reader, for an
original trailer piece written in its manner, never from its notes. It
prints numbers only and writes nothing: tempo, each track's role (its
voice's kind, envelope and sample, its note lengths and range), the key's
pitch classes, the drums' places in the bar and the form, bar by bar.

    python3 .claude/skills/trailer/scripts/song_habits.py GAME SONG [--offset TICKS] [--bars]

GAME is bn6 or bn5 (the ROMs told by their SHA-1 in build.py's ROM folder),
SONG the song's number in its game's table (docs/ROM_DATA.md: BN6's at
0x159F48, BN5's at 0x155BF4; a title's or battle's number from the
disassembly or by listening with `build.py shot --render-song`). A song
that starts on a pickup counts its bars late: --offset TICKS moves the bar
line (the first track's first downbeat, read off the first notes printed),
and the drums' places then read as the bar's. --bars prints how many notes
each track plays in each bar: the intro, a breakdown (a track alone) and
the main sections show as their densities change.
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
        R, a = self.R, self.vg + 12 * v
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
            else:
                arg, pc = R[pc], pc + 1
                if c == 0xBB:
                    tempo.append((t, arg * 2))
                elif c == 0xBD:
                    voice = arg
                elif c == 0xBC:
                    shift = arg if arg < 64 else arg - 256
                elif c == 0xCE and R[pc] < 0x80:
                    pc += 1
        return notes, tempo


def main():
    args = sys.argv[1:]
    offset = 0
    if '--offset' in args:
        i = args.index('--offset')
        offset = int(args[i + 1], 0)
        del args[i:i + 2]
    bars = '--bars' in args
    args = [a for a in args if a != '--bars']
    if len(args) != 2 or args[0] not in TABLES:
        sys.exit(__doc__)
    data, table = rom(args[0])
    song = Song(data, table, int(args[1], 0))
    print(f'{args[0]} song {args[1]}: {song.tracks_n} tracks, reverb {song.reverb & 0x7F}')
    walked = [song.walk(pc) for pc in song.tracks]
    tempos = sorted({bpm for _, tempo in walked for _, bpm in tempo})
    print('tempo', tempos or '(none set)')
    pitch = collections.Counter()
    density = []
    for i, (notes, _) in enumerate(walked):
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
            for k in sorted(places):
                print(f'     key {k}: {dict(sorted(places[k].items()))}')
        else:
            pitch.update(n[1] % 12 for n in notes)
        per_bar = collections.Counter(max(0, n[0] - offset) // 96 for n in notes)
        density.append(per_bar)
    if pitch:
        total = sum(pitch.values())
        print('\npitch classes (melodic tracks):', ', '.join(f'{NAMES[p]} {100 * c // total}%'
                                                             for p, c in pitch.most_common()))
    if bars:
        last = max(max(d) for d in density if d)
        print('\nnotes a bar, track by track:')
        for b in range(last + 1):
            print(f'  bar {b:2d}: ' + ' '.join(f'{d.get(b, 0):3d}' for d in density))


if __name__ == '__main__':
    main()
