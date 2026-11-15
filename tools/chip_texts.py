#!/usr/bin/env python3
"""Every chip's name and description against its own (docs/devtools/chip-texts.md).

The host build's --chip-texts writes, for each chip id 0-1023, the name and
description BN6's own routines find in the core's copy (the archives' words
their literals name, the id's high byte's, its low byte's entry), with all
three ROMs read, and the entry each should be: BN6's own id, a BN5 row's
or a BN4 row's id in its game (the chip registry). This compares them with
the ROMs' own archives, letters and digits alone (the codes below 0x40,
the same in all three games; punctuation and marks are transcoded, the
boxes' commands differ), and names the entry a wrong text was taken from.

  python3 tools/chip_texts.py            # dump with the host build, then compare
  python3 tools/chip_texts.py FILE.tsv   # compare a dump made before

Needs the ROMs (local only): CYBERWORLD_ROM_DIR, else ~/.cache/mmbn-ref/roms.
Exits 1 where a chip's text is another's or missing.
"""
import os
import re
import struct
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROM_DIR = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
ROMS = {6: 'bn6g.gba', 5: 'bn5c.gba', 4: 'bn4rs.gba'}
BUS = 0x08000000


def define(header, name):
    """A #define's number in src/ (the ROMs' offsets stay named in one place, docs/rom/)."""
    text = open(os.path.join(ROOT, header)).read()
    return int(re.search(r'#define\s+%s\s+(0x[0-9A-Fa-f]+|\d+)' % name, text).group(1), 0)


def layout():
    """Each game's names' and descriptions' archives (ROM offsets), and BN4's last id with a description."""
    rom_c = open(os.path.join(ROOT, 'src/core/rom.c')).read()
    gregar = rom_c[rom_c.index('[ROM_BN6_GREGAR_US]'):]
    names6 = [int(x, 16) for x in re.search(r'\.chip_names = \{ (0x\w+), (0x\w+) \}', gregar).groups()]
    descs6 = [int(x, 16) for x in re.search(r'\.chip_descs = \{ (0x\w+), (0x\w+) \}', gregar).groups()]
    return {
        6: (names6, descs6),
        5: ([define('src/emu/bn5.h', 'BN5_CHIP_NAMES_LOW'), define('src/emu/bn5.h', 'BN5_CHIP_NAMES_HIGH')],
            [define('src/emu/bn5.h', 'BN5_CHIP_DESCS_LOW'), define('src/emu/bn5.h', 'BN5_CHIP_DESCS_HIGH')]),
        4: ([define('src/emu/bn4.h', 'BN4_CHIP_NAMES') - BUS, define('src/emu/bn4.h', 'BN4_CHIP_NAMES_HI') - BUS],
            [define('src/emu/bn4_chips.h', 'BN4_CHIP_DESCS') - BUS, define('src/emu/bn4_chips.h', 'BN4_CHIP_DESCS_HI') - BUS]),
    }, define('src/emu/bn4_chips.h', 'BN4_CHIP_DESC_LAST')


# a game's text: its end, its new line and its commands' arguments (BN6's and BN5's alike; BN4's own)
RULES = {
    6: (0xE6, 0xE9, {0xE8: 3, 0xF1: 2, 0xE7: 1}),
    5: (0xE6, 0xE9, {0xE8: 3, 0xF1: 2, 0xE7: 1}),
    4: (0xE5, 0xE8, {0xE7: 1, 0xF0: 2, 0xE6: 1}),
}


def letters(b, game):
    """Its letters, digits and spaces, a new line a space: the codes below 0x40, the same glyphs in the three games (and
    BN5's DS mark as the core spells it)."""
    end, line, args = RULES[game]
    out, i = [], 0
    while i < len(b):
        c = b[i]
        i += 1
        if c == end:
            break
        if c >= 0xE5:
            if c == line:
                out.append(' ')
            i += args.get(c, 0)
        elif c == 0:
            out.append(' ')
        elif c == 0x9F and game == 5:
            out.append('DS')   # (BN5's DS mark, spelt D and S: BN6's font has none, src/core/rom.c's bn5_code)
        elif c <= 0x0A:
            out.append(chr(ord('0') + c - 1))
        elif c <= 0x24:
            out.append(chr(ord('A') + c - 0x0B))
        elif 0x26 <= c <= 0x3F:
            out.append(chr(ord('a') + c - 0x26))
    return ' '.join(''.join(out).split())


def entry(rom, archive, i, bounded):
    """Entry `i` of the text archive at `archive`, b'' past its count: to its end code as BN6's and BN5's readers take it
    (BN6's own name of id 284 has none, its reader running on into 285's, the core's copy too), or `bounded` to the next
    entry's start (BN4's descriptions 281-292 are empty boxes without one, which the core sets as empty boxes)."""
    count = struct.unpack_from('<H', rom, archive)[0] // 2
    if i >= count:
        return b''
    at = archive + struct.unpack_from('<H', rom, archive + 2 * i)[0]
    to = archive + struct.unpack_from('<H', rom, archive + 2 * i + 2)[0] if bounded and i + 1 < count else at + 200
    return rom[at:min(to, at + 200)] if to > at else rom[at:at + 200]


def dump(path):
    """The host build's --chip-texts into `path` (in .build/, where the build image sees it as /src)."""
    rel = os.path.relpath(path, ROOT)
    os.makedirs(os.path.join(ROOT, '.build', 'chip-texts', 'data'), exist_ok=True)
    cmd = [sys.executable, os.path.join(ROOT, 'build.py'), 'shot', '--data-dir', '/src/.build/chip-texts/data', '--chip-texts', '/src/' + rel]
    r = subprocess.run(cmd, cwd=ROOT, env=dict(os.environ, CYBERWORLD_ROM_DIR=ROM_DIR), capture_output=True, text=True)
    if r.returncode or not os.path.exists(path):
        sys.exit('chip_texts: the dump failed:\n' + r.stdout[-2000:] + r.stderr[-2000:])


def main():
    path = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, '.build', 'chip-texts', 'texts.tsv')
    if len(sys.argv) <= 1:
        dump(path)
    roms = {g: open(os.path.join(ROM_DIR, f), 'rb').read() for g, f in ROMS.items()}
    arch, desc4_last = layout()

    def source(game, index, desc):
        if game == 4 and desc and index > desc4_last:
            return ''
        a = arch[game][1 if desc else 0][index >> 8]
        return letters(entry(roms[game], a, index & 0xFF, game == 4), game)

    # (each game's descriptions by their letters, to name the entry a wrong one came from)
    known = {}
    for game in (6, 5, 4):
        for index in range(512):
            if index >> 8 < len(arch[game][1]):
                known.setdefault(source(game, index, True), (game, index))
    checked, wrong, readers = 0, [], []
    for line in open(path):
        cid, game, index, agree, name_hex, desc_hex = line.rstrip('\n').split('\t')
        cid, game, index = int(cid), int(game), int(index)
        if agree != '1':
            readers.append(cid)
        if not game:
            continue
        name, desc = letters(bytes.fromhex(name_hex), 6), letters(bytes.fromhex(desc_hex), 6)
        want_name, want_desc = source(game, index, False), source(game, index, True)
        # (an own row without its game's ROM shows the placeholder: a run reads them all here)
        checked += 1
        if name != want_name or desc != want_desc:
            wrong.append((cid, game, index, name, want_name, desc, want_desc, known.get(desc) if desc else None))
    for cid, game, index, name, want_name, desc, want_desc, came in wrong:
        print(f'id {cid} (BN{game} {index}): name "{name}" for "{want_name}", description "{desc}" for "{want_desc}"'
              + (f', BN{came[0]} {came[1]}\'s' if came else ''))
    if readers:
        print(f'{len(readers)} ids where the PET\'s readers of a description take another archive than BN6_CHIP_DESC_OF: {readers[:20]}')
    print(f'{checked} chips checked: {len(wrong)} with another\'s or a missing name or description')
    return 1 if wrong or readers else 0


if __name__ == '__main__':
    sys.exit(main())
