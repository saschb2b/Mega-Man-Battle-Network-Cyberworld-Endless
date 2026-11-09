#!/usr/bin/env python3
"""chip_ids.py [--all] [--check]: every place in BN6 Cybeast Gregar's code that takes a chip's id in nine bits, for
ten-bit chip ids (docs/multirom/bn4-chips.md, Past 511; docs/rom/chips-code.md, Ten-bit chip ids).

A folder entry is a chip's id in its low 9 bits and its code from bit 9 (docs/rom/chips.md). The shapes that take one
apart or make one are looked for in the code (to 0x08150000): a shift pair keeping the low 9 bits (lsls #23, lsrs #23),
a shift right by 9 (the code), a shift left by 9 (a code put in), a literal whose low half is 0x1FF (the mask, or "no
chip"), a shift pair taking 6 or 7 bits from bit 9 (a code without the flag at bit 15). A shape counts where the code
reaches it: every routine a bl or a word of the ROM names is walked from its entry (tools/xrecipe_thumb.py), so a
shape inside data (map objects' scripts, tables) is left out. Each one reached has its verdict in SITES below, read in
the code: patched for ten bits (an entry's id or code, an id's mask, 0x1FF as no chip), kept (a reward entry, which
keeps nine bits; the link battle's, out of a run's reach) or no chip at all (what it is), or open (a chip's, not read
yet). Printed: the counts, then each site reached with its verdict, or without one.

Local only: the ROM (CYBERWORLD_ROM_DIR, ~/.cache/mmbn-ref/roms, told by its SHA-1). Nothing of it is written:
addresses and verdicts.

  python3 tools/chip_ids.py           # the counts and the sites without a verdict
  python3 tools/chip_ids.py --all     # every site reached, with its verdict
  python3 tools/chip_ids.py --check   # exit 1 while a site reached has no verdict
  python3 tools/chip_ids.py --table   # the changes for ten bits as src/emu/xchips_wide.c's table (was and now)
  python3 tools/chip_ids.py --moved [--check]   # the same shapes in BN5's code the moved units copy (BN5's ROM too)
"""
import argparse
import hashlib
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import xrecipe_thumb as T  # noqa: E402

ROM_DIR = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
GREGAR_SHA1 = '89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6'
COLONEL_SHA1 = '5f472f78d8de2df01d5039e045c043cb40969a39'
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# BN5's code the moved chips' and Souls' units copy into BN6's engine (src/emu/xchips_bn5.c, src/emu/soulcode_bn5.c: their
# blocks), its shapes and their verdicts: none of them splits an entry, the units handed ids by BN6's hand (--moved).
MOVED_SITES = {
    0x080C5E6A: 'SideBub\'s table of words, read as code', 0x080C5E7A: 'the same', 0x080C5E8A: 'the same', 0x080C5E9A: 'the same',
    0x080E2444: 'DarkInvs\'s time, the literal 600 (docs/rom/chips-code.md, its end)',
}
BUS = 0x08000000
CODE_END = 0x08150000

# The verdicts, by the shape's address: (verdict, what it is). Patched: 'entry' (a folder entry's id or code, taken in
# ten bits), 'id' (an id's mask, ten bits), 'none' (0x1FF as no chip: 0x3FF); 'make' (a code put into an entry at bit
# 10); kept: 'reward' (a reward entry, nine bits), 'link' (the link battle's, which no run reaches); 'other' (no chip);
# 'open' (a chip's, not read yet); 'keys' (the folder editor's items, id << 7 | code in a halfword, which hold ids to
# 511 whatever an entry's form: an id past 511 makes them id << 6 | code, with the editor's shifts by 7 and 0x7F masks,
# not read yet; the entries made from them are 'make').
SITES = {
    # the battle's deck: the folder shuffled into it, the marks, the Mega and Giga counts, the cheat test
    0x0800A584: ('entry', 'the deck shuffled from the folder: an entry\'s id, its record\'s library type'),
    0x0800A7F2: ('entry', 'the folder\'s marks applied at the battle\'s start: an entry\'s code'),
    0x0800A7F4: ('entry', 'the folder\'s marks applied: an entry\'s id'),
    0x0800A814: ('entry', 'a folder\'s Gigas counted: an entry\'s id'),
    0x0800AFD4: ('entry', 'a chip drawn tested (the cheat test, with the Mega and Giga counts): its code (27 and 28 the pseudo-chips\')'),
    0x0800AFDE: ('entry', 'the same: its id'),
    0x0800B00C: ('entry', 'the same: its id against 411 for the marks\' test'),
    0x0800B03C: ('entry', 'a chip drawn tested, the second form: its code'),
    0x0800B046: ('entry', 'the same: its id'),
    0x0800B074: ('entry', 'the same: its code for the code\'s test'),
    0x0800B076: ('entry', 'the same: its id'),
    # the Custom screen: its slots point at the deck's entries
    0x0801C050: ('id', 'a chip\'s icon in battle: its id\'s mask'),
    0x0801D348: ('id', 'a chip\'s icon and letter drawn: its id\'s mask'),
    0x0801D354: ('none', 'the same: 0x1FF draws no chip'),
    0x08021AE2: ('entry', 'the folder entry split (the folder editor, GiveFolder, the folder\'s Mega and Giga counts): its code'),
    0x08021AE4: ('entry', 'the same: its id\'s mask, a literal whose high half is the next routine\'s first instruction'),
    0x08021C2A: ('entry', 'a chip found in the folder by id and code: its id\'s mask'),
    0x08021C2E: ('entry', 'the same: its code'),
    0x0802696E: ('none', 'the Custom screen opened: no chip\'s icon drawn'),
    0x08027178: ('entry', 'the Custom screen\'s slots drawn: a slot\'s entry\'s id'),
    0x080274AE: ('none', 'a chip picked: no chip\'s icon in its place'),
    0x0802769A: ('entry', 'the Custom screen\'s slots drawn: a slot\'s entry\'s id'),
    0x080276BA: ('entry', 'the same, the cursor\'s slot'),
    0x080277CA: ('entry', 'the same'),
    0x08027D12: ('id', 'a chip\'s name: its id\'s mask, then the archive by id > 255'),
    0x08027D36: ('id', 'a chip\'s description: its id\'s mask, then the archive by id > 255'),
    0x08028084: ('entry', 'the Custom screen: a slot\'s entry\'s id, its record\'s flags'),
    0x080281AA: ('none', 'a chip\'s picture: its id\'s mask, 0x1FF no chip'),
    0x08028242: ('none', 'the picture\'s place cleared: no chip'),
    0x0802828C: ('none', 'a slot\'s picture, a Giga hidden: no chip'),
    0x080282A4: ('entry', 'a slot\'s code letter: an entry\'s code'),
    0x080282F0: ('none', 'the same, the second form: no chip'),
    0x08028308: ('entry', 'the same: an entry\'s code'),
    0x08028312: ('none', 'the picture cleared: no chip'),
    0x08028502: ('id', 'a chip\'s card drawn: its id\'s mask'),
    0x08028618: ('id', 'the same, the second form'),
    0x08028E78: ('entry', 'a chip picked: its entry\'s code'),
    0x08028E7A: ('entry', 'the same: its id'),
    0x08028EEE: ('entry', 'a chip given back: its entry\'s code'),
    0x08028EF0: ('entry', 'the same: its id'),
    0x08028FF4: ('entry', 'a slot\'s entry\'s id, its record\'s flags'),
    0x0802907C: ('none', 'a chip picked: no chip\'s icon in its place'),
    0x080290C8: ('none', 'the same, the second form'),
    0x08029168: ('entry', 'the hand made from the chips picked: an entry\'s code (the hand holds id and code apart)'),
    0x0802916A: ('entry', 'the same: its id'),
    0x0802953A: ('entry', 'the Program Advance check: the chips picked split, an entry\'s code'),
    0x0802953C: ('entry', 'the same: its id'),
    0x0802A00C: ('id', 'an id against a short list (the Custom screen\'s): its mask'),
    0x0802A3B2: ('entry', 'a slot\'s entry\'s id, its record\'s flags'),
    0x0802A516: ('entry', 'the chips picked counted by library type: an entry\'s id'),
    0x0802B706: ('make', 'the P.A. screen\'s parts: code 28 (movs #0x1C)'),
    0x0802B708: ('make', 'the same, put in at bit 9 (lsls #9)'),
    0x0802B846: ('id', 'the P.A. screen\'s parts: a part\'s id, its letter below BN6_PA_LETTERS'),
    0x0802B850: ('entry', 'the same: its code'),
    0x0802C558: ('reward', 'a battle\'s reward given: id, code from bit 9, kind from bit 14 (the wide rewards\' place)'),
    0x0802E2E0: ('link', 'the link battle\'s real-time chip (battle flag 0x40, the Arena\'s pair cores): an id BN6_LINK_CHIP_BOUND caps at 0x139 before this mask, 0 past it, nine bits or ten'),
    0x0802855C: ('entry', 'a chip\'s card: its entry\'s code (27 and 28 the pseudo-chips\')'),
    0x0802C556: ('reward', 'a battle\'s reward given: its code from bit 9'),
    0x081224F0: ('keys', 'the folder editor\'s list into the folder: an item\'s id (bits 7-15 of id << 7 | code)'),
    0x081225B4: ('keys', 'the same, the second form'),
    0x080AC4DA: ('link', 'the link battle\'s trade: an entry made, code << 9'),
    0x080AC564: ('link', 'the same, the second form'),
    # the chips' attacks: the chip in hand (an id, the hand keeps the code apart) or an object's own
    0x0810B466: ('id', 'a chip\'s attack: the chip in hand, its id\'s mask'),
    0x0810BA32: ('id', 'the same'), 0x0810C88A: ('id', 'the same'), 0x0810D102: ('id', 'the same'),
    0x0810D5C0: ('id', 'the same'), 0x0810DB5E: ('id', 'the same'), 0x0810E288: ('id', 'the same'),
    0x0810EE2E: ('id', 'the same'), 0x0810F688: ('id', 'the same'), 0x081104F4: ('id', 'the same'),
    0x081132EC: ('id', 'the same'), 0x0811393E: ('id', 'the same'), 0x08114516: ('id', 'the same'),
    # the link battle's copy of the pack (0x02033000), its rewards and trades: no run reaches it
    0x080AC628: ('link', 'the link battle\'s pack: an entry\'s code'),
    0x080AC62A: ('link', 'the same: its id'),
    0x080AC656: ('link', 'the same: an entry\'s code'),
    0x080AC658: ('link', 'the same: its id'),
    # the folder editor: its list's items are id << 7 | code (a halfword), an entry made from one with bit 15 set
    0x081224C2: ('keys', 'the folder editor\'s list: an item\'s code (bits 0-6 of id << 7 | code)'),
    0x081224F4: ('keys', 'the same, the list into the folder'),
    0x08122502: ('make', 'the folder editor\'s list into the folder: the entry made, code << 9 | id (the item\'s code from its low 7 bits)'),
    0x081225B8: ('keys', 'the same, the second form: an item\'s code'),
    0x081225C0: ('make', 'the same, the second form: the entry made'),
    0x0812EA88: ('entry', 'the folder editor\'s chip shown: an entry\'s id'),
    0x0812EAB2: ('entry', 'the same, the other side'),
    0x0812EB8E: ('entry', 'the folder editor: a chip moved, an entry\'s id'),
    0x0812EB96: ('entry', 'the same: its code, bits 9-14 (bit 15 the editor\'s mark)'),
    0x0812EBA4: ('entry', 'the same, the other side: its id'),
    0x0812EBAC: ('entry', 'the same: its code'),
    0x0812F3B4: ('entry', 'the folder editor: an entry\'s id'),
    0x0812F3BA: ('entry', 'the same: its code, bits 9-14'),
    0x0812F64C: ('make', 'the folder editor: an entry made from an item (id << 7 | code), code << 9, bit 15 its mark'),
    # no chip
    0x08001468: ('other', 'a coordinate wrapped to 9 bits'),
    0x0801C0F6: ('other', 'an object\'s screen x (OAM, 9 bits)'),
    0x08029C26: ('other', 'a screen x (9 bits)'),
    0x0802CA66: ('other', 'an OAM x (9 bits)'),
    0x08073B46: ('other', 'a random number to 511 and 600 more: a timer'),
    0x0800075C: ('other', 'the sound engine'),
    0x0802E18C: ('other', 'debug strings after a routine, read as code'), 0x0802E196: ('other', 'the same'),
    0x08037176: ('other', 'the cutscene camera\'s scroll'), 0x0803718C: ('other', 'the same'),
    0x08049BEA: ('other', 'graphics copied, 512 bytes a frame'), 0x08049BF0: ('other', 'the same'),
    0x0810658A: ('other', 'an object\'s offset, x 512'),
    0x08145E80: ('other', 'the wireless adapter\'s library'),
    0x0802D372: ('other', 'a flag, 0x200'), 0x0802D374: ('other', 'a flag, 0x200'),
    0x0802D51C: ('other', 'a flag, 0x200'), 0x0802D51E: ('other', 'a flag, 0x200'),
}
for _a in (0x0801431C, 0x08014354, 0x0801437A, 0x08014394, 0x0801A63A, 0x0801A674, 0x0801B32C, 0x0801E13A, 0x0801E146,
           0x0801E174):
    SITES[_a] = SITES[_a + 2] = ('other', 'a battle flag, 0x200')


# The folder editor's items (0x20 bytes each, their word at +0x1C: the low half id << 7 | code, the high half a count or
# stamp), read where its shifts by 7 and 0x7F masks are (a scan of the reached code from 0x0811F000 and 0x0812C000): what
# ids from 512 need, id << 6 | code (the codes to 28 fit six bits), not patched yet (the pack past 511 comes first, which a
# chip moved there needs). 'build' an item made (lsls #7 to #6), 'id' its id taken (lsrs #7 to #6; lsls #16, lsrs #23
# to #22), 'code' its code taken (0x7F to 0x3F; lsls #9, lsrs #25 to #10, #26), 'read' an id-like halfword at +0x1C of
# a list not read yet, 'other' no item (the input's repeat timers, sound 0x7F, a byte).
EDITOR_KEYS = {
    0x08121AC4: 'build', 0x08121CB4: 'build', 0x08121B00: 'read', 0x08121CC2: 'read',
    0x08121EE8: 'id', 0x08121F94: 'id', 0x08122192: 'id', 0x081221E6: 'id', 0x0812225A: 'id', 0x08122260: 'code',
    0x081222B4: 'id', 0x08122408: 'id', 0x081224BC: 'id', 0x081224C4: 'code', 0x081224F2: 'id', 0x081224F6: 'code',
    0x081225B6: 'id', 0x081225BA: 'code', 0x0812F396: 'id', 0x0812F398: 'code', 0x0812F646: 'id', 0x0812F648: 'code',
    0x08135EFA: 'id', 0x08135EFC: 'code', 0x081367DA: 'id', 0x08136936: 'id', 0x0813693C: 'id', 0x08136B8E: 'id',
    0x08136F24: 'id', 0x08136F7A: 'id', 0x08136FB2: 'id', 0x081372FC: 'id',
    0x0812160E: 'other', 0x08121624: 'other', 0x08121658: 'other', 0x0812166E: 'other', 0x08121728: 'other',
    0x08121762: 'other', 0x08120C1E: 'other', 0x08120C36: 'other', 0x081216CE: 'other', 0x081217C0: 'other',
    0x08121866: 'other', 0x08137E7C: 'other', 0x08137FD6: 'other', 0x0813835E: 'other', 0x081376B8: 'other',
    0x081394DE: 'other',
}


def find_rom(sha1=GREGAR_SHA1, name='BN6 Cybeast Gregar (USA)'):
    for fname in sorted(os.listdir(ROM_DIR)):
        path = os.path.join(ROM_DIR, fname)
        if fname.lower().endswith('.gba') and os.path.isfile(path):
            with open(path, 'rb') as f:
                data = f.read()
            if hashlib.sha1(data).hexdigest() == sha1:
                return T.Rom(data, name)
    sys.exit('%s not found in %s' % (name, ROM_DIR))


def moved_blocks():
    """the BN5 ranges the moved chips' and Souls' units copy: (unit, from, to)"""
    import re
    out = []
    for f in ('src/emu/xchips_bn5.c', 'src/emu/soulcode_bn5.c'):
        with open(os.path.join(ROOT, f)) as fh:
            text = fh.read()
        for m in re.finditer(r'static const XvBlock (\w+)\[\] = \{(.*?)\};', text, re.S):
            for b in re.finditer(r'\{ 0x([0-9A-F]{8}), 0x([0-9A-F]{8}),', m.group(2)):
                out.append((m.group(1), int(b.group(1), 16), int(b.group(2), 16)))
    return out


def moved(check):
    """the shapes in BN5's moved code, each with its verdict (MOVED_SITES)"""
    rom5 = find_rom(COLONEL_SHA1, 'BN5 Team Colonel (USA)')
    blocks = moved_blocks()
    hits = {}
    for unit, lo, hi in blocks:
        for s, k in shapes(rom5, lo, hi):
            hits[s] = (unit, k)
    loose = [s for s in hits if s not in MOVED_SITES]
    print('%d blocks of BN5\'s code (%d bytes) the moved units copy: %d shapes, %d without a verdict' % (
        len(blocks), sum(hi - lo for _, lo, hi in blocks), len(hits), len(loose)))
    for s in sorted(hits):
        print('%08x %-24s %-6s %s' % (s, hits[s][0], hits[s][1], MOVED_SITES.get(s, '-')))
    if check and loose:
        sys.exit(1)


def shapes(rom, lo=BUS, hi=CODE_END):
    """every shape from `lo` to `hi` (the code region): (address, shape)"""
    out = []
    for a in range(lo, hi, 2):
        h = rom.u16(a)
        if h & 0xF800 == 0x0000 and (h >> 6) & 31:   # lsls rd, rm, #n
            n, rd = (h >> 6) & 31, h & 7
            for k in (2, 4, 6):
                h2 = rom.u16(a + k)
                if h2 & 0xF800 == 0x0800 and (h2 >> 3) & 7 == rd and h2 & 7 == rd:
                    m = (h2 >> 6) & 31
                    if n == 23 and m == 23:
                        out.append((a, 'id23'))
                    elif m - n == 9:
                        out.append((a, 'code%d' % (32 - m)))
                    break
            if n == 9:
                out.append((a, 'shl9'))
        if h & 0xF800 == 0x0800 and (h >> 6) & 31 == 9:   # lsrs rd, rm, #9
            out.append((a, 'shr9'))
        if h & 0xF800 == 0x4800:   # ldr rd, [pc, #]
            lit = ((a + 4) & ~3) + (h & 0xFF) * 4
            if rom.has(lit, 4) and rom.u32(lit) & 0xFFFF == 0x01FF:
                out.append((a, 'mask'))
        if h & 0xF800 == 0x2000 and 0 < h & 0xFF < 64:   # movs rd, #c; lsls rd, rd, #9: a code made
            rd = (h >> 8) & 7
            h2 = rom.u16(a + 2)
            if h2 & 0xFFC0 == 0x0240 and h2 & 7 == rd and (h2 >> 3) & 7 == rd:
                out.append((a, 'code_k'))
    return out


def entries(rom):
    """every routine's entry the code names: a bl's target (sure), a word of the ROM with bit 0 set inside the code
    (a routine's address, or a number that looks like one)"""
    got, sure = set(), set()
    for a in range(BUS, CODE_END, 2):
        h = rom.u16(a)
        if h & 0xF800 == 0xF000 and rom.u16(a + 2) & 0xF800 == 0xF800:
            off = T.sext(((h & 0x7FF) << 12) | ((rom.u16(a + 2) & 0x7FF) << 1), 23)
            t = a + 4 + off
            if BUS <= t < CODE_END:
                sure.add(t)
    for a in range(0, min(len(rom.data), 0x800000) - 3, 4):
        v = struct.unpack_from('<I', rom.data, a)[0]
        if v & 1 and BUS <= v < CODE_END:
            got.add(v & ~1)
    return sure, got - sure


def reached(rom):
    code = set()
    sure, maybe = entries(rom)
    for e in sorted(sure) + sorted(maybe):
        if e in code:
            continue
        w = T.walk(rom, e, limit=0x4000, region=(BUS, CODE_END))
        if e in maybe and any(why == 'undecodable' for why, _ in w.bad):
            continue   # (a word that is not a routine's address)
        code.update(a for a, i in w.insns.items() if i.form != 'bad')
    return code


PATCHED = ('entry', 'id', 'none', 'make')


def changes(rom, found):
    """each patched site's halfword changes for ten bits: (address, was, now, the site's verdict and what): a shift's
    count (23 to 22, 9 to 10), the 0x01FF literal's low half (0x03FF, once a literal), BN6_ENTRY_CHEAT's entry"""
    out, lits = [], set()
    kinds = {}
    for a, k in found:
        kinds.setdefault(a, []).append(k)

    def imm(h, n):
        return (h & ~(31 << 6)) | (n << 6)
    for a in sorted(SITES):
        v, what = SITES[a]
        if v not in PATCHED or a not in kinds:
            continue
        h = rom.u16(a)
        for k in kinds[a]:
            if k == 'id23':
                out.append((a, h, imm(h, 22), v, what))
                for d in (2, 4, 6):
                    h2 = rom.u16(a + d)
                    if h2 & 0xF800 == 0x0800 and (h2 >> 6) & 31 == 23 and h2 & 7 == h & 7:
                        out.append((a + d, h2, imm(h2, 22), v, what))
                        break
            elif k in ('shr9', 'shl9'):
                out.append((a, h, imm(h, 10), v, what))
            elif k == 'code6':   # lsls #17 then lsrs #26: bits 9-14, bit 15 left out; ten bits: 10-14 (lsrs #27)
                for d in (2, 4, 6):
                    h2 = rom.u16(a + d)
                    if h2 & 0xF800 == 0x0800 and (h2 >> 6) & 31 == 26:
                        out.append((a + d, h2, imm(h2, 27), v, what))
                        break
            elif k == 'mask':
                lit = ((a + 4) & ~3) + (h & 0xFF) * 4
                if lit not in lits:
                    lits.add(lit)
                    out.append((lit, 0x01FF, 0x03FF, v, what))
    out.append((0x0802A55C, 0x3785, 0x6D85, 'entry', 'BN6_ENTRY_CHEAT: 389 in code 27'))
    return sorted(set(out), key=lambda c: c[0])


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--all', action='store_true', help='every site reached, with its verdict')
    ap.add_argument('--check', action='store_true', help='exit 1 while a site reached has no verdict')
    ap.add_argument('--table', action='store_true', help="the changes for ten bits as xchips_wide.c's table")
    ap.add_argument('--moved', action='store_true', help="BN5's code the moved chips' and Souls' units copy, its shapes (BN5's ROM)")
    a = ap.parse_args()
    if a.moved:
        return moved(a.check)
    rom = find_rom()
    code = reached(rom)
    found = shapes(rom)
    live = [(s, k) for s, k in found if s in code]
    if a.table:
        rows = changes(rom, found)
        for at, was, now, v, what in rows:
            print('\t{ 0x%08Xu, 0x%04X, 0x%04X },' % (at, was, now))
        print('/* %d changes */' % len(rows))
        return
    counts = {}
    loose = []
    for s, k in live:
        v = SITES.get(s, (None, ''))[0]
        counts[v or 'without a verdict'] = counts.get(v or 'without a verdict', 0) + 1
        if v is None:
            loose.append((s, k))
    print('%d shapes in the code region, %d where the code reaches' % (len(found), len(live)))
    for v in ('entry', 'id', 'none', 'make', 'keys', 'reward', 'link', 'other', 'open', 'without a verdict'):
        if v in counts:
            print('  %-18s %d' % (v, counts[v]))
    for s, k in (live if a.all else loose):
        v, what = SITES.get(s, ('-', ''))
        print('%08x %-7s %-7s %s' % (s, k, v, what))
    every = {x for x, _ in found}
    stale = [s for s in SITES if s not in every]
    for s in stale:
        print('%08x (a verdict, but no shape there)' % s)
    idle = sorted(s for s in SITES if s in every and s not in code)
    if idle:
        print('%d verdicts for shapes in routines nothing names (kept, patched alike): %s' % (
            len(idle), ' '.join('%08x' % s for s in idle)))
    if a.check and (loose or stale):
        sys.exit(1)


if __name__ == '__main__':
    main()
