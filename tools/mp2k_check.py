#!/usr/bin/env python3
"""Walk MP2K tracks the way src/audio/audio.c does and report each track's length
in ticks up to its loop jump. Tracks of one song must agree."""
import struct, sys
rom = open(sys.argv[1], 'rb').read()
T = 0x159F48
W = lambda a: struct.unpack_from('<I', rom, a)[0]
LEN = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,28,30,32,36,40,42,44,48,52,54,56,60,64,66,68,72,76,78,80,84,88,90,92,96]

def walk(pc):
    ticks, running, stack, rept = 0, 0, [], {}
    for _ in range(200000):
        c = rom[pc]
        if c < 0x80:
            c = running
            if not c: return ticks, 'bad running status'
        else:
            pc += 1
            if c >= 0xBD: running = c
        if c <= 0xB0: ticks += LEN[c - 0x80]; continue
        if c >= 0xCF:
            if rom[pc] < 0x80:
                pc += 1
                if rom[pc] < 0x80:
                    pc += 1
                    if c != 0xCF and rom[pc] < 0x80: pc += 1
            continue
        if c == 0xB1: return ticks, 'end'
        if c == 0xB2: return ticks, 'loop'
        if c == 0xB3: stack.append(pc + 4); pc = W(pc) - 0x08000000; continue
        if c == 0xB4:
            if stack: pc = stack.pop()
            continue
        if c == 0xB5:
            cnt = rom[pc]; dst = W(pc + 1) - 0x08000000
            k = rept.get(pc, 0) + 1
            if cnt == 0 or k < cnt: rept[pc] = k; pc = dst
            else: rept[pc] = 0; pc += 5
            continue
        if c == 0xB9: pc += 3; continue
        if c == 0xCD: pc += 2; continue
        if c == 0xCE:
            if rom[pc] < 0x80: pc += 1
            continue
        if c in (0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC8): pc += 1; continue
        return ticks, f'unknown {c:#x} at {pc:#x}'
    return ticks, 'runaway'

bad = 0
for song in [int(x, 0) for x in sys.argv[2:]] or range(1, 0x26):
    h = W(T + song * 8) - 0x08000000
    n = rom[h]
    res = [walk(W(h + 8 + 4 * t) - 0x08000000) for t in range(n)]
    lens = {r[0] for r in res if r[1] == 'loop'}
    flag = '' if len(lens) <= 1 and all(r[1] in ('loop', 'end') for r in res) else '  <-- MISMATCH'
    bad += bool(flag)
    print(f'{song:#04x} tracks {n}: ' + ' '.join(f'{t}{"L" if s == "loop" else "E" if s == "end" else "!"}' for t, s in res) + flag)
print('mismatched songs:', bad)
