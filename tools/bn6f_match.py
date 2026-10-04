#!/usr/bin/env python3
"""bn6f_match.py --bn6f DIR [--roms DIR] [--out DIR] [--report FUNCTION]:
the functions of the bn6f disassembly (github.com/dism-exe/bn6f, built from
Cybeast Falzar) located in Cybeast Gregar (USA), and those BN5 Team
Colonel (USA) shares with it (docs/SYMBOLS.md).

Each function's bytes are made from bn6f's own source, as its assembler
(GNU as with agbasm's extensions, divided syntax) would make them at its
Falzar address, with every part that depends on where things are masked: a
bl's target, a branch out of the function, a load from a label outside it,
a literal word that names a label. The masked bytes are looked for in the
ROM: a function found once is located; and every located function's
calls, branches, literals and jump tables, read where it was found, name
where the functions and data they point at are: a function found in many
places takes the one they name, one whose bytes changed between the
versions is placed by them alone, the data between functions by its labels.
A function still unplaced takes the shift from Falzar its located
neighbours in its file share, where its bytes are there. RAM the literals
hold is checked against bn6f's addresses (Gregar's RAM is laid out as
Falzar's). In BN5, another game, only its bytes and the references count.

Writes .build/symbols/match/bn6f-gregar.csv and bn6f-bn5.csv (or into
--out): per function its bn6f name, its Falzar address, where it is in the
ROM, how it was located and how many references agree; then the data and
RAM labels the references locate. Reads the ROMs (found in --roms by their
SHA-1) and the disassembly, and keeps nothing of either but names and
addresses. bn6f states no license, so its names stay on this machine:
.build is not in git, and nothing this writes is committed
(docs/SYMBOLS.md). tools/symbols.py --full reads the tables; build.py
symbols --bn6f DIR runs both.

  python3 tools/bn6f_match.py --bn6f DIR                     # the ROMs from ~/.cache/mmbn-ref/roms
  python3 tools/bn6f_match.py --bn6f DIR --report FUNCTION   # one function's bytes against Gregar's, writes nothing
"""
import argparse
import csv
import hashlib
import io
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MATCH = os.path.join(ROOT, '.build', 'symbols', 'match')
# the ROMs, told by the SHA-1s src/core/rom.c checks, and their tables
ROMS = (('bn6-gregar', '89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6', 'bn6f-gregar.csv'),
        ('bn5-colonel', '5f472f78d8de2df01d5039e045c043cb40969a39', 'bn6f-bn5.csv'))
ANCHOR = re.compile(r'(sub|loc|locret|off|dword|word|byte|unk|asc|stru|jpt|jt)_([0-9A-Fa-f]{7})$')

REGS = {f'r{i}': i for i in range(16)}
REGS.update(sp=13, lr=14, pc=15, ip=12, fp=11, sl=10, sb=9)
CONDS = dict(eq=0, ne=1, cs=2, hs=2, cc=3, lo=3, mi=4, pl=5, vs=6, vc=7, hi=8, ls=9, ge=10, lt=11, gt=12, le=13, al=14)


# ---- bn6f's constants: .equ, enums, the structures' field offsets ----

class Constants:
    """The names bn6f's include/ and constants/ give numbers, evaluated as
    asked; a name not known (or a label) is None. A structure's field
    offsets are added up from its fields' types, as its macros lay them
    out (its loc= comments are not always right: ObjectSprite's Unk_10 says
    0x12, the code reads 0x10), the comments only where a field's type is
    not one of the plain ones."""

    def __init__(self, root):
        self.root, self.raw, self.cache, self.macros, self.prefix_of = root, {}, {}, {}, {}
        for d in ('constants', 'include'):
            for dirpath, _, files in os.walk(os.path.join(root, d)):
                for f in sorted(files):
                    if f.endswith(('.inc', '.s')):
                        self.read(os.path.join(dirpath, f))
        self.doubt = set()
        for macro, prefix in self.prefix_of.items():
            if macro in self.macros:
                laid, said = {}, {}
                self.struct(macro, prefix, laid, said)
                for name in set(laid) | set(said):
                    if name in laid and name in said and laid[name] != said[name]:
                        self.doubt.add(name)   # (the two disagree: the field's offset is masked)
                    else:
                        self.raw.setdefault(name, str(laid.get(name, said.get(name))))

    def struct(self, macro, prefix, laid, said, off=0, depth=0):
        """A structure's fields' offsets from off: added up from their types
        (laid) and as its loc= comments say (said); a structure in it (its
        macro called with \\label or \\label\\()_Name) laid out in its place.
        Returns the offset after it (None: not known)."""
        unions = []
        for line in self.macros[macro]:
            code = line.split('//')[0].strip()
            loc = re.search(r'loc=(0x[0-9A-Fa-f]+|\d+)', line)
            words = code.split(None, 1)
            if not words:
                continue
            t, arg = words[0], (words[1] if len(words) > 1 else '')
            inner = re.match(r'\\label(?:\\\(\))?(\w*)', arg)
            if t in self.macros and inner and depth < 4 and any('set_struct_label' in x for x in self.macros[t]):
                off = self.struct(t, prefix + inner.group(1), laid, said, off, depth + 1)
                continue
            if t == 'union':
                unions.append([off, off])
            elif t == 'nextu' and unions:
                top = unions[-1]
                top[1] = max(top[1], off) if off is not None and top[1] is not None else None
                off = top[0]
            elif t == 'endu' and unions:
                top = unions.pop()
                off = max(top[1], off) if off is not None and top[1] is not None else None
            elif t in ('space', '.space'):
                n = self.value(arg)
                off = off + n if off is not None and n is not None else None
            elif t == 'struct_org':
                off = self.value(arg)
            else:
                size = self.type_size(t, arg)
                field = arg.split(',')[0].strip()
                if not re.fullmatch(r'\w+', field or '-'):
                    continue
                name = f'{prefix}_{field}'
                if loc:
                    said.setdefault(name, int(loc.group(1), 0))
                if off is not None:
                    laid.setdefault(name, off)
                off = off + size if off is not None and size is not None else None
        return off

    def type_size(self, t, arg):
        sizes = dict(u0=0, u8=1, s8=1, enum8=1, flags8=1, bool=1, bool8=1, u16=2, s16=2, enum16=2, flags16=2,
                     u32=4, s32=4, ptr=4, enum32=4, flags32=4)
        if t in sizes:
            return sizes[t]
        if t == 'u8_arr':
            parts = split_ops(arg)
            return self.value(parts[1]) if len(parts) > 1 else None
        body = self.macros.get(t, [])
        if any('set_struct_label' in line for line in body):
            return None   # (a structure in the structure: laid out by its loc= comments after it)
        for line in body:   # (a type of its own: PETNavi is an enum8)
            w = line.split('//')[0].split()
            if w and w[0] in sizes:
                return sizes[w[0]]
        return None

    def read(self, path):
        enum, inc = 0, 1
        macro = None
        text = open(path, encoding='utf-8', errors='replace').read()
        for m in re.finditer(r'def_struct_offsets\s+(\w+)\s*,\s*(\w+)', text):
            self.prefix_of[m.group(1)] = m.group(2)
        for m in re.finditer(r'^\s*(\w+)\s+(\w+)\s*,\s*offset_struct_entry\b', text, re.M):
            self.prefix_of.setdefault(m.group(1), m.group(2))   # (battle_object_struct oBattleObject, offset_struct_entry, ...)
        for line in text.split('\n'):
            code = line.split('//')[0].strip()
            mm = re.match(r'\.macro\s+(\w+)', code)
            if mm:
                macro = mm.group(1)
                self.macros.setdefault(macro, [])
                continue
            if code.startswith('.endm'):
                macro = None
                continue
            if macro:
                self.macros[macro].append(line)
                sm = re.match(r'struct_const\s+(\w+)\s*,\s*(.+)$', code)
                if sm:
                    self.raw.setdefault(sm.group(1), sm.group(2))
                continue
            em = re.match(r'\.(?:equ|equiv|set)\s+(\w+)\s*,\s*(.+)$', code) or re.match(r'(\w+)\s*=\s*(.+)$', code)
            if em and not em.group(1).startswith('__'):
                self.raw.setdefault(em.group(1), em.group(2))
                continue
            es = re.match(r'enum_start\s*(?:([^,]+?))?\s*(?:,\s*(.+))?$', code)
            if es and code.startswith('enum_start'):
                enum = self.value(es.group(1)) if es.group(1) else 0
                inc = self.value(es.group(2)) if es.group(2) else 1
                enum = enum or 0
                inc = inc or 1
                continue
            en = re.match(r'(enum|flag_enum)\s+(\w+)$', code)
            if en:
                self.raw.setdefault(en.group(2), str(enum if en.group(1) == 'enum' else 1 << enum))
                enum += inc
                continue
            sk = re.match(r'enum_skip\s+(.+)$', code)
            if sk:
                enum += self.value(sk.group(1)) or 0

    def value(self, expr, depth=0):
        """expr as a number, or None."""
        if expr is None or depth > 20:
            return None
        expr = expr.strip()
        if expr in self.cache:
            return self.cache[expr]
        self.cache[expr] = None
        names = re.findall(r'(?<![\w.])[A-Za-z_][\w]*', expr)
        text = expr
        for n in sorted(set(names), key=len, reverse=True):
            if n not in self.raw:
                return None
            v = self.value(self.raw[n], depth + 1)
            if v is None:
                return None
            text = re.sub(r'(?<![\w.])' + n + r'(?![\w])', str(v), text)
        v = evaluate(text)
        self.cache[expr] = v
        return v


def evaluate(text):
    """A GNU as expression of numbers and operators, or None."""
    t = re.sub(r'\b0b([01]+)\b', lambda m: str(int(m.group(1), 2)), text)
    t = t.replace('!', ' not ').replace('/', '//')
    if not re.fullmatch(r'[\s0-9a-fA-FxX()+\-*/%<>|&^~not]*', t) or not t.strip():
        return None
    try:
        return int(eval(t, {'__builtins__': {}}))   # (digits and operators alone)
    except Exception:
        return None


# ---- bn6f's source, line by line ----

DIRECTIVES = {'.thumb', '.arm', '.text', '.data', '.pool', '.ltorg', '.endm', '.endif', '.else', '.thumb_func',
              '.syntax', '.section', '.global', '.globl', '.type', '.size', '.align', '.balign', '.word', '.hword',
              '.short', '.2byte', '.4byte', '.byte', '.space', '.skip', '.asciz', '.ascii', '.string', '.incbin',
              '.include', '.equ', '.equiv', '.set', '.if', '.ifdef', '.ifndef', '.macro', '.fill', '.code', '.end',
              '.print', '.warning', '.error', '.rept', '.endr', '.irp', '.irpc', '.purgem', '.altmacro', '.noaltmacro',
              '.loc', '.file', '.org', '.zero', '.int', '.long', '.quad', '.bss', '.err'}


def logical_lines(path):
    """A source file's statements: (line number, text) with comments gone and
    agbasm's [ ... ] macro arguments over several lines joined."""
    text = open(path, encoding='utf-8', errors='replace').read()
    text = re.sub(r'/\*.*?\*/', lambda m: '\n' * m.group(0).count('\n'), text, flags=re.S)
    out, pending, start = [], None, 0
    for no, line in enumerate(text.split('\n'), 1):
        code = strip_comment(line).strip()
        if pending is not None:
            pending += ' ' + code
            if pending.count('[') <= pending.count(']'):
                out.append((start, pending))
                pending = None
            continue
        if not code:
            continue
        if code.count('[') > code.count(']') and not re.match(r'\s*(ldr|str|ldm|stm)', code):
            pending, start = code, no
            continue
        out.append((no, code))
    return out


def strip_comment(line):
    """A line without its // or @ comment (not inside a string)."""
    quote = False
    for i, ch in enumerate(line):
        if ch == '"':
            quote = not quote
        elif not quote and (line.startswith('//', i) or (ch == '@' and not line[:i].rstrip().endswith(','))):
            return line[:i]
    return line


def split_ops(text):
    """Operands at the top level, split at commas outside [] and {}."""
    out, depth, cur = [], 0, ''
    for ch in text:
        if ch in '[{(':
            depth += 1
        elif ch in ']})':
            depth -= 1
        if ch == ',' and depth == 0:
            out.append(cur.strip())
            cur = ''
        else:
            cur += ch
    if cur.strip():
        out.append(cur.strip())
    return out


class Function:
    """A function of bn6f's, or the data between two (kind 'data': jump
    tables, pools, records), as its source has it."""

    def __init__(self, name, scope, mode, path, line, align, kind='function'):
        self.name, self.scope, self.mode, self.path, self.line = name, scope, mode, path, line
        self.kind = kind
        self.align = align        # its start's alignment, a power of two (thumb_func_start's .align 1, arm's .align 2)
        self.items = []           # (label | mode | stmt | end, its data, line)
        self.anchors = []         # (label, the address its name gives)
        self.after_gap = True     # what stands between it and the one before is not laid out here


def source_files(root):
    """bn6f's source files with code, in the order rom.s includes them, then
    the IWRAM code (asm38.s, iwram.s)."""
    order = []

    def follow(path):
        for no, code in logical_lines(path):
            m = re.match(r'\.include\s+"([^"]+)"', code)
            if not m or m.group(1).startswith(('include/', 'constants/', 'data/textscript/')) or m.group(1) == 'charmap.inc':
                continue
            inc = os.path.join(root, m.group(1))
            if inc.endswith('.s') and os.path.exists(inc):
                order.append(inc)
                follow(inc)
    follow(os.path.join(root, 'rom.s'))
    order.append(os.path.join(root, 'asm', 'asm38.s'))
    return [p for p in order if re.search(r'(thumb|arm)_(func|local)_start', open(p, encoding='utf-8', errors='replace').read())]


def read_functions(root, consts):
    """Every function bn6f defines, in the order of its files, each with its
    statements and the labels in it; the constants the files set (.equ)
    go to consts."""
    functions = []
    for path in source_files(root):
        for no, code in logical_lines(path):
            m = re.match(r'\.(?:equ|equiv|set)\s+(\w+)\s*,\s*(.+)$', code)
            if m:
                consts.raw.setdefault(m.group(1), m.group(2))
        functions += parse_file(root, path)
    return functions


def parse_file(root, path):
    """A file's functions and the data between them, in order."""
    rel = os.path.relpath(path, root)
    out, cur, blk, mode = [], None, None, 'thumb'
    pending_local = None

    def here(no):
        """Where a statement outside a function goes: the data block it is in."""
        nonlocal blk
        if blk is None:
            blk = Function(None, 'data', mode, rel, no, 0, kind='data')
            blk.after_gap = not out
            out.append(blk)
        return blk
    for no, code in logical_lines(path):
        # labels: name: / name:: / a lone name (agbasm's colonless labels)
        while True:
            m = re.match(r'^([A-Za-z_.$][\w.$]*)::?(?=\s|$)', code)
            if not m:
                break
            label = m.group(1)
            if pending_local is not None:
                cur = Function(label, 'local', pending_local[0], rel, no, pending_local[1])
                cur.after_gap, blk, pending_local = not out, None, None
                out.append(cur)
            (cur if cur is not None else here(no)).items.append(('label', label, no))
            code = code[m.end():].strip()
        if not code:
            continue
        words = code.split(None, 1)
        op, rest = words[0], (words[1] if len(words) > 1 else '')
        if op in ('thumb_func_start', 'arm_func_start'):
            args = split_ops(rest)
            mode = 'thumb' if op.startswith('thumb') else 'arm'
            align = int(args[1], 0) if len(args) > 1 else (1 if mode == 'thumb' else 2)
            cur = Function(args[0], 'global', mode, rel, no, align)
            cur.after_gap, blk = not out, None
            out.append(cur)
            continue
        if op in ('thumb_local_start', 'arm_local_start'):
            mode = 'thumb' if op.startswith('thumb') else 'arm'
            args = split_ops(rest)
            pending_local = (mode, int(args[0], 0) if args else (1 if mode == 'thumb' else 2))
            cur, blk = None, None
            continue
        if op in ('thumb_func_end', 'arm_func_end'):
            if cur is not None:
                cur.items.append(('end', None, no))
            cur = None
            continue
        if op in ('.thumb', '.arm') or (op == '.code' and rest.strip() in ('16', '32')):
            mode = 'thumb' if op == '.thumb' or rest.strip() == '16' else 'arm'
            (cur if cur is not None else here(no)).items.append(('mode', mode, no))
            continue
        lone = len(words) == 1 and ((op.startswith('.') and op not in DIRECTIVES) or
                                    (not op.startswith('.') and op.lower() not in BARE))
        target = cur if cur is not None else here(no)
        if lone:
            target.items.append(('label', op, no))   # (agbasm's labels without a colon: .loop, loc_800387C)
        else:
            target.items.append(('stmt', (op, rest), no))
    for f in out:
        if f.kind == 'data':
            f.name = next((d for k, d, n in f.items if k == 'label'), f'({rel}:{f.line})')
            f.items.append(('end', None, 0))
        for kind, data, no in f.items:
            if kind == 'label':
                m = ANCHOR.match(data)
                if m:
                    f.anchors.append((data, address_of(m.group(2))))
    return out


def address_of(digits):
    """An IDA name's seven hex digits as the address they mean (8000344: 0x08000344)."""
    v = int(digits, 16)
    return v | 0x08000000 if digits[0] in '89' else v


# the instructions and macros that stand alone on a line (no operands)
BARE = {'nop', 'thumb_local_start', 'arm_local_start'}


# ---- the encoders: (value, mask) a halfword or word, and what it names ----

class Unknown(Exception):
    """An instruction this encoder does not make: its bytes are masked whole."""


def reg(text):
    t = text.strip().lower()
    if t not in REGS:
        raise Unknown(text)
    return REGS[t]


def reglist(text):
    """{r4-r7,lr} as a bit mask of registers."""
    bits = 0
    for part in text.strip().strip('{}').split(','):
        part = part.strip()
        if not part:
            continue
        if '-' in part:
            a, b = part.split('-')
            for r in range(reg(a), reg(b) + 1):
                bits |= 1 << r
        else:
            bits |= 1 << reg(part)
    return bits


class Ctx:
    """What an instruction's encoding needs: bn6f's constants, the addresses
    of the function's own labels where it is laid out."""

    def __init__(self, consts, local):
        self.consts, self.local = consts, local

    def imm(self, text):
        """An immediate (#...) as a number, None where it names what is not a constant."""
        t = text.strip()
        t = t[1:] if t.startswith('#') else t
        return self.consts.value(t)


def thumb(op, rest, pc, ctx):
    """One Thumb instruction (divided syntax, as GNU as makes it): a list of
    (halfword, mask) and the references in it: (offset, kind, name)."""
    ops = split_ops(rest)
    o = op.lower()
    refs = []

    def imm(i, bits, shift=0, signed=False):
        v = ctx.imm(ops[i])
        if v is None:
            return 0, 0
        if signed and v < 0:
            v &= (1 << (bits + shift)) - 1
        return (v >> shift) & ((1 << bits) - 1), (1 << bits) - 1

    def branch_target(name):
        off = ctx.local.get(name)
        return None if off is None else off

    if o == 'bl':
        t = branch_target(ops[0])
        refs.append((0, 'bl', ops[0]))
        if t is not None and pc is not None:
            d = t - (pc + 4)
            return [(0xF000 | (d >> 12) & 0x7FF, 0xFFFF), (0xF800 | (d >> 1) & 0x7FF, 0xFFFF)], refs
        return [(0xF000, 0xF800), (0xF800, 0xF800)], refs
    if o == 'b' or (o.startswith('b') and o[1:] in CONDS and o not in ('bl', 'bic')):
        t = branch_target(ops[0])
        if o == 'b':
            if t is not None and pc is not None:
                return [(0xE000 | ((t - pc - 4) >> 1) & 0x7FF, 0xFFFF)], refs
            refs.append((0, 'b', ops[0]))
            return [(0xE000, 0xF800)], refs
        c = CONDS[o[1:]]
        if t is not None and pc is not None:
            return [(0xD000 | c << 8 | ((t - pc - 4) >> 1) & 0xFF, 0xFFFF)], refs
        refs.append((0, 'b', ops[0]))
        return [(0xD000 | c << 8, 0xFF00)], refs
    if o == 'bx':
        r = reg(ops[0])
        return [(0x4700 | (r >> 3) << 6 | (r & 7) << 3, 0xFFFF)], refs
    if o in ('svc', 'swi'):
        v, m = imm(0, 8)
        return [(0xDF00 | v, 0xFF00 | m)], refs
    if o == 'nop':
        return [(0x46C0, 0xFFFF)], refs
    if o in ('push', 'pop'):
        bits = reglist(ops[0])
        extra = (bits >> 14) & 1 if o == 'push' else (bits >> 15) & 1
        return [((0xB400 if o == 'push' else 0xBC00) | extra << 8 | bits & 0xFF, 0xFFFF)], refs
    if o in ('ldmia', 'stmia', 'ldm', 'stm'):
        rb = reg(ops[0].rstrip('!'))
        return [((0xC800 if o.startswith('ldm') else 0xC000) | rb << 8 | reglist(ops[1]) & 0xFF, 0xFFFF)], refs
    if o in ('lsl', 'lsr', 'asr') and len(ops) == 3:
        v, m = imm(2, 5)
        if ctx.imm(ops[2]) == 32:
            v, m = 0, 0x1F
        return [({'lsl': 0, 'lsr': 0x800, 'asr': 0x1000}[o] | v << 6 | reg(ops[1]) << 3 | reg(ops[0]), 0xF83F | m << 6)], refs
    alu = dict(and_=0, eor=1, lsl=2, lsr=3, asr=4, adc=5, sbc=6, ror=7, tst=8, neg=9, cmn=11, orr=12, mul=13, bic=14, mvn=15)
    key = 'and_' if o == 'and' else o
    if key in alu and len(ops) in (2, 3) and not ops[-1].startswith('#'):
        rd, rs = reg(ops[0]), reg(ops[1])
        if o == 'mul' and len(ops) == 3:
            if reg(ops[2]) == rd:
                pass
            elif rs == rd:
                rs = reg(ops[2])
            else:
                raise Unknown('mul')
        if rd > 7 or rs > 7:
            raise Unknown(o)
        return [(0x4000 | alu[key] << 6 | rs << 3 | rd, 0xFFFF)], refs
    if o in ('mov', 'cmp', 'add', 'sub'):
        return thumb_arith(o, ops, pc, ctx, imm, refs)
    if o in ('ldr', 'str', 'ldrb', 'strb', 'ldrh', 'strh', 'ldrsb', 'ldrsh'):
        return thumb_mem(o, ops, pc, ctx, refs)
    if o == 'adr':
        rd = reg(ops[0])
        t = branch_target(ops[1])
        if t is not None and pc is not None:
            return [(0xA000 | rd << 8 | ((t - ((pc + 4) & ~3)) >> 2) & 0xFF, 0xFFFF)], refs
        return [(0xA000 | rd << 8, 0xFF00)], refs
    raise Unknown(op)


def thumb_arith(o, ops, pc, ctx, imm, refs):
    if len(ops) == 2 and ops[1].startswith('#'):
        d = ops[0].lower()
        if d == 'sp' and o in ('add', 'sub'):
            v = ctx.imm(ops[1])
            if v is None:
                return [(0xB000 | (0x80 if o == 'sub' else 0), 0xFF80)], refs
            neg = (o == 'sub') != (v < 0)
            return [(0xB000 | (0x80 if neg else 0) | (abs(v) >> 2) & 0x7F, 0xFFFF)], refs
        rd = reg(ops[0])
        v, m = imm(1, 8)
        base = dict(mov=0x2000, cmp=0x2800, add=0x3000, sub=0x3800)[o]
        if rd > 7:
            raise Unknown(o)
        return [(base | rd << 8 | v, 0xFF00 | m)], refs
    if len(ops) == 3 and ops[2].startswith('#'):
        d, s = ops[0].lower(), ops[1].lower()
        if d == 'sp' and s == 'sp':
            return thumb_arith(o, ['sp', ops[2]], pc, ctx, imm, refs)
        if s in ('sp', 'pc') and o == 'add':
            v = ctx.imm(ops[2])
            rd = reg(d)
            base = 0xA800 if s == 'sp' else 0xA000
            if v is None:
                return [(base | rd << 8, 0xFF00)], refs
            return [(base | rd << 8 | (v >> 2) & 0xFF, 0xFFFF)], refs
        rd, rs = reg(d), reg(s)
        v = ctx.imm(ops[2])
        if v is not None and (v > 7 or v < 0) and rd == rs:
            if v < 0:
                o, v = ('sub' if o == 'add' else 'add'), -v
            return [(dict(add=0x3000, sub=0x3800)[o] | rd << 8 | v & 0xFF, 0xFFFF)], refs
        if v is not None and v < 0:
            o, v = ('sub' if o == 'add' else 'add'), -v
        base = 0x1C00 if o == 'add' else 0x1E00
        if v is None:
            return [(base | rs << 3 | rd, 0xFE3F)], refs
        return [(base | (v & 7) << 6 | rs << 3 | rd, 0xFFFF)], refs
    if len(ops) == 3:
        rd, rs, rn = reg(ops[0]), reg(ops[1]), reg(ops[2])
        if o in ('add', 'sub') and max(rd, rs, rn) < 8:
            return [((0x1800 if o == 'add' else 0x1A00) | rn << 6 | rs << 3 | rd, 0xFFFF)], refs
        if o == 'add' and rd == rs:
            return hireg(0x4400, rd, rn), refs
        if o == 'add' and rd == rn:
            return hireg(0x4400, rd, rs), refs
        raise Unknown(o)
    rd, rs = reg(ops[0]), reg(ops[1])
    if o == 'mov':
        if rd < 8 and rs < 8:
            return [(0x1C00 | rs << 3 | rd, 0xFFFF)], refs   # (GNU as: adds rd, rs, #0)
        return hireg(0x4600, rd, rs), refs
    if o == 'cmp':
        if rd < 8 and rs < 8:
            return [(0x4280 | rs << 3 | rd, 0xFFFF)], refs
        return hireg(0x4500, rd, rs), refs
    if o == 'add':
        if rd < 8 and rs < 8:
            return [(0x1800 | rs << 6 | rd << 3 | rd, 0xFFFF)], refs   # (adds rd, rd, rs)
        return hireg(0x4400, rd, rs), refs
    if o == 'sub' and rd < 8 and rs < 8:
        return [(0x1A00 | rs << 6 | rd << 3 | rd, 0xFFFF)], refs
    raise Unknown(o)


def hireg(base, rd, rs):
    return [(base | (rd >> 3) << 7 | (rs >> 3) << 6 | (rs & 7) << 3 | rd & 7, 0xFFFF)]


def thumb_mem(o, ops, pc, ctx, refs):
    rd = reg(ops[0])
    addr = ops[1].strip() if len(ops) > 1 else ''
    if o == 'ldr' and addr.startswith('='):
        refs.append((0, 'pool', addr[1:].strip()))
        return [(0x4800 | rd << 8, 0xFF00)], refs
    if o == 'ldr' and not addr.startswith('['):
        t = ctx.local.get(addr)
        if t is not None and pc is not None:
            return [(0x4800 | rd << 8 | ((t - ((pc + 4) & ~3)) >> 2) & 0xFF, 0xFFFF)], refs
        refs.append((0, 'ldr', addr))
        return [(0x4800 | rd << 8, 0xFF00)], refs
    inner = split_ops(addr.strip('[]'))
    rb = reg(inner[0])
    if len(inner) == 2 and not inner[1].startswith('#'):
        ro = reg(inner[1])
        base = dict(str=0x5000, strh=0x5200, strb=0x5400, ldrsb=0x5600, ldr=0x5800, ldrh=0x5A00, ldrb=0x5C00, ldrsh=0x5E00)[o]
        return [(base | ro << 6 | rb << 3 | rd, 0xFFFF)], refs
    v = ctx.imm(inner[1]) if len(inner) == 2 else 0
    if rb == 13:
        if o not in ('ldr', 'str'):
            raise Unknown(o)
        base = 0x9800 if o == 'ldr' else 0x9000
        return [(base | rd << 8 | ((v or 0) >> 2) & 0xFF, 0xFFFF if v is not None else 0xFF00)], refs
    if rb == 15 and o == 'ldr':
        return [(0x4800 | rd << 8 | ((v or 0) >> 2) & 0xFF, 0xFFFF if v is not None else 0xFF00)], refs
    if o in ('ldrsb', 'ldrsh'):
        raise Unknown(o)
    base, shift = dict(str=(0x6000, 2), ldr=(0x6800, 2), strb=(0x7000, 0), ldrb=(0x7800, 0), strh=(0x8000, 1),
                       ldrh=(0x8800, 1))[o]
    if v is None:
        return [(base | rb << 3 | rd, 0xF83F)], refs
    return [(base | ((v >> shift) & 0x1F) << 6 | rb << 3 | rd, 0xFFFF)], refs


# ARM: the data-processing, multiply, transfer, block, branch, status and
# software-interrupt forms the disassembly uses; the rest is masked
ARM_DP = dict(and_=0, eor=1, sub=2, rsb=3, add=4, adc=5, sbc=6, rsc=7, tst=8, teq=9, cmp=10, cmn=11, orr=12, mov=13, bic=14, mvn=15)
SHIFTS = dict(lsl=0, asl=0, lsr=1, asr=2, ror=3)


def arm_split(op):
    """An ARM mnemonic as (base, condition, the rest: s, b, h, sb, a block mode...)."""
    o = op.lower()
    for base in sorted(('and', 'eor', 'sub', 'rsb', 'add', 'adc', 'sbc', 'rsc', 'tst', 'teq', 'cmp', 'cmn', 'orr', 'mov',
                        'bic', 'mvn', 'mul', 'mla', 'umull', 'umlal', 'smull', 'smlal', 'ldr', 'str', 'ldm', 'stm',
                        'bx', 'bl', 'b', 'mrs', 'msr', 'swi', 'svc', 'adr', 'swp'), key=len, reverse=True):
        if o.startswith(base):
            rest = o[len(base):]
            cond = 14
            for c, v in CONDS.items():
                if rest.startswith(c) and (base not in ('bl',) or True):
                    cond, rest = v, rest[len(c):]
                    break
            if base == 'b' and rest == 'l':
                continue
            return base, cond, rest
    raise Unknown(op)


def arm_operand2(text, ctx):
    """An ARM operand 2: (bits, mask, I)."""
    ops = split_ops(text)
    first = ops[0]
    if first.startswith('#'):
        v = ctx.imm(first)
        if v is None:
            return 0, 0xFFFFF000, 1
        v &= 0xFFFFFFFF
        for rot in range(16):
            r = ((v << (2 * rot)) | (v >> (32 - 2 * rot))) & 0xFFFFFFFF
            if r < 256:
                return rot << 8 | r, 0xFFFFFFFF, 1
        raise Unknown('imm')
    rm = reg(first)
    if len(ops) == 1:
        return rm, 0xFFFFFFFF, 0
    sm = re.match(r'(lsl|asl|lsr|asr|ror|rrx)\s*(#?)\s*(.*)$', ops[1].strip().lower())
    if not sm:
        raise Unknown(text)
    kind = sm.group(1)
    if kind == 'rrx':
        return 3 << 5 | rm, 0xFFFFFFFF, 0
    if sm.group(2):
        n = ctx.imm(sm.group(3))
        if n is None:
            return SHIFTS[kind] << 5 | rm, 0xFFFFF07F, 0
        n = 0 if n == 32 else n
        return (n & 31) << 7 | SHIFTS[kind] << 5 | rm, 0xFFFFFFFF, 0
    return reg(sm.group(3)) << 8 | SHIFTS[kind] << 5 | 1 << 4 | rm, 0xFFFFFFFF, 0


def arm(op, rest, pc, ctx):
    """One ARM instruction: a list of one (word, mask) and its references."""
    base, cond, suffix = arm_split(op)
    ops = split_ops(rest)
    refs = []
    c = cond << 28
    if base in ('b', 'bl'):
        t = ctx.local.get(ops[0])
        link = 1 << 24 if base == 'bl' else 0
        if t is not None and pc is not None:
            return [(c | 0x0A000000 | link | ((t - pc - 8) >> 2) & 0xFFFFFF, 0xFFFFFFFF)], refs
        refs.append((0, base, ops[0]))
        return [(c | 0x0A000000 | link, 0xFF000000)], refs
    if base == 'bx':
        return [(c | 0x012FFF10 | reg(ops[0]), 0xFFFFFFFF)], refs
    if base in ('swi', 'svc'):
        v = ctx.imm(ops[0])
        return [(c | 0x0F000000 | (v or 0) & 0xFFFFFF, 0xFFFFFFFF if v is not None else 0xFF000000)], refs
    if base == 'mrs':
        return [(c | 0x010F0000 | (1 << 22 if 'spsr' in ops[1].lower() else 0) | reg(ops[0]) << 12, 0xFFFFFFFF)], refs
    if base == 'msr':
        m = re.match(r'(cpsr|spsr)(?:_(\w+))?', ops[0].lower())
        fields = m.group(2) or 'fc'
        mask = sum({'c': 1, 'x': 2, 's': 4, 'f': 8}[ch] for ch in fields if ch in 'cxsf')
        r = 1 << 22 if m.group(1) == 'spsr' else 0
        if ops[1].startswith('#'):
            bits, mk, _ = arm_operand2(ops[1], ctx)
            return [(c | 0x0320F000 | r | mask << 16 | bits, mk)], refs
        return [(c | 0x0120F000 | r | mask << 16 | reg(ops[1]), 0xFFFFFFFF)], refs
    if base in ('mul', 'mla'):
        s = 1 << 20 if 's' in suffix else 0
        rd, rm, rs = reg(ops[0]), reg(ops[1]), reg(ops[2])
        rn = reg(ops[3]) << 12 if base == 'mla' else 0
        return [(c | (1 << 21 if base == 'mla' else 0) | s | rd << 16 | rn | rs << 8 | 0x90 | rm, 0xFFFFFFFF)], refs
    if base in ('umull', 'umlal', 'smull', 'smlal'):
        s = 1 << 20 if 's' in suffix else 0
        lo, hi, rm, rs = (reg(x) for x in ops[:4])
        u = dict(umull=0, umlal=1, smull=2, smlal=3)[base]
        return [(c | 0x00800090 | u << 21 | s | hi << 16 | lo << 12 | rs << 8 | rm, 0xFFFFFFFF)], refs
    if base in ('ldm', 'stm'):
        mode = suffix or 'ia'
        load = base == 'ldm'
        pu = {'ia': (0, 1), 'ib': (1, 1), 'da': (0, 0), 'db': (1, 0),
              'fd': (0, 1) if load else (1, 0), 'ed': (1, 1) if load else (0, 0),
              'fa': (1, 0) if load else (0, 1), 'ea': (0, 0) if load else (1, 1)}[mode]
        rn = ops[0].strip()
        w = 1 << 21 if rn.endswith('!') else 0
        rl = ops[1].strip()
        sbit = 1 << 22 if rl.endswith('^') else 0
        bits = reglist(rl.rstrip('^'))
        return [(c | 0x08000000 | pu[0] << 24 | pu[1] << 23 | sbit | w | (1 << 20 if load else 0) | reg(rn.rstrip('!')) << 16 | bits,
                 0xFFFFFFFF)], refs
    if base in ('ldr', 'str'):
        return arm_mem(base, suffix, c, ops, pc, ctx, refs)
    if base == 'adr':
        rd = reg(ops[0])
        t = ctx.local.get(ops[1])
        if t is None or pc is None:
            refs.append((0, 'ldr', ops[1]))
            return [(c | 0x020F0000 | rd << 12, 0xFE0FF000)], refs
        d = t - (pc + 8)
        bits, mk, _ = arm_operand2(f'#{abs(d)}', ctx)
        return [(c | 0x020F0000 | (4 if d >= 0 else 2) << 21 | rd << 12 | bits, 0xFFFFFFFF)], refs
    key = 'and_' if base == 'and' else base
    if key in ARM_DP:
        opc = ARM_DP[key]
        s = 1 << 20 if suffix == 's' or opc in (8, 9, 10, 11) else 0
        if suffix not in ('', 's'):
            raise Unknown(op)
        if opc in (13, 15):
            rd, rn, rest_ops = reg(ops[0]), 0, ','.join(ops[1:])
        elif opc in (8, 9, 10, 11):
            rd, rn, rest_ops = 0, reg(ops[0]), ','.join(ops[1:])
        else:
            rd = reg(ops[0])
            if len(ops) == 2:
                rn, rest_ops = rd, ops[1]
            else:
                rn, rest_ops = reg(ops[1]), ','.join(ops[2:])
        bits, mk, i = arm_operand2(rest_ops, ctx)
        return [(c | i << 25 | opc << 21 | s | rn << 16 | rd << 12 | bits, mk)], refs
    raise Unknown(op)


def arm_mem(base, suffix, c, ops, pc, ctx, refs):
    load = base == 'ldr'
    rd = reg(ops[0])
    half = suffix in ('h', 'sh', 'sb')
    byte = suffix == 'b'
    if suffix not in ('', 'b', 'h', 'sh', 'sb', 't', 'bt'):
        raise Unknown(base + suffix)
    addr = ','.join(ops[1:]).strip()
    if addr.startswith('='):
        refs.append((0, 'pool', addr[1:].strip()))
        return [(c | 0x059F0000 | (1 << 22 if byte else 0) | rd << 12, 0xFFFFF000)], refs
    if not addr.startswith('['):
        t = ctx.local.get(addr)
        if t is None or pc is None:
            refs.append((0, 'ldr', addr))
            return [(c | 0x051F0000 | (1 << 20 if load else 0) | (1 << 22 if byte else 0) | rd << 12, 0xFF7FF000)], refs
        d = t - (pc + 8)
        return [(c | 0x051F0000 | (1 << 23 if d >= 0 else 0) | (1 << 20 if load else 0) | (1 << 22 if byte else 0) | rd << 12 | abs(d),
                 0xFFFFFFFF)], refs
    m = re.match(r'\[([^\]]*)\](!?)\s*(?:,\s*(.*))?$', addr)
    inner = split_ops(m.group(1))
    rn = reg(inner[0])
    pre = m.group(3) is None
    w = 1 << 21 if m.group(2) else 0
    off = inner[1] if pre and len(inner) > 1 else (m.group(3) if not pre else None)
    p = 1 << 24 if pre else 0
    l_ = 1 << 20 if load else 0
    if half:
        sh = dict(h=0xB0, sb=0xD0, sh=0xF0)[suffix]
        if off is None or off.strip().startswith('#'):
            v = ctx.imm(off) if off else 0
            if v is None:
                return [(c | p | 1 << 22 | w | l_ | rn << 16 | rd << 12 | sh, 0xFF7FF0F0)], refs
            v &= 0xFFFFFFFF
            if v >= 0x80000000:
                v = v - (1 << 32)
            u = 1 << 23 if v >= 0 else 0
            v = abs(v)
            return [(c | p | u | 1 << 22 | w | l_ | rn << 16 | rd << 12 | (v >> 4) << 8 | sh | v & 15, 0xFFFFFFFF)], refs
        u = 0 if off.strip().startswith('-') else 1 << 23
        return [(c | p | u | w | l_ | rn << 16 | rd << 12 | sh | reg(off.strip().lstrip('-')), 0xFFFFFFFF)], refs
    b = 1 << 22 if byte else 0
    if off is None or off.strip().startswith('#'):
        v = ctx.imm(off) if off else 0
        if v is None:
            return [(c | 0x04000000 | p | b | w | l_ | rn << 16 | rd << 12, 0xFF7FF000)], refs
        v &= 0xFFFFFFFF
        if v >= 0x80000000:
            v = v - (1 << 32)
        u = 1 << 23 if v >= 0 else 0
        return [(c | 0x04000000 | p | u | b | w | l_ | rn << 16 | rd << 12 | abs(v) & 0xFFF, 0xFFFFFFFF)], refs
    neg = off.strip().startswith('-')
    bits, mk, _ = arm_operand2(off.strip().lstrip('-'), ctx)
    return [(c | 0x06000000 | p | (0 if neg else 1 << 23) | b | w | l_ | rn << 16 | rd << 12 | bits, mk | 0xFFFFF000)], refs


# ---- a function's bytes at an address ----

THUMB_OPS = {'adc', 'add', 'and', 'asr', 'b', 'bic', 'bl', 'bx', 'cmn', 'cmp', 'eor', 'ldmia', 'ldr', 'ldrb', 'ldrh', 'ldrsb',
             'ldrsh', 'lsl', 'lsr', 'mov', 'mul', 'mvn', 'neg', 'orr', 'pop', 'push', 'ror', 'sbc', 'stmia', 'str', 'strb',
             'strh', 'sub', 'svc', 'swi', 'tst', 'nop', 'adr', 'ldm', 'stm'} | {'b' + c for c in CONDS}
DATA = {'.word': 4, '.4byte': 4, '.long': 4, '.int': 4, '.hword': 2, '.short': 2, '.2byte': 2, '.byte': 1}
QUIET = {'.global', '.globl', '.type', '.size', '.thumb_func', '.syntax', '.equ', '.equiv', '.set', '.text', '.section',
         '.data', '.loc', '.file', '.print', '.warning'}


class Layout:
    """A function's bytes as bn6f's assembler makes them with its start at
    `base`: a (value, mask) pair a byte (mask 0 where the byte depends on
    where something else is); its labels' offsets; the references in it
    (offset, kind, name, addend); the data among its code (literal pools,
    tables) as offset ranges; its size, or None where a statement this
    reader does not make cuts it short (the bytes up to it are kept)."""

    def __init__(self):
        self.bytes, self.labels, self.refs, self.data = [], {}, [], []
        self.size, self.stop, self.unknown = None, None, 0


def unescape(s):
    return s.encode('latin-1', 'replace').decode('unicode_escape').encode('latin-1', 'replace')


def strings(rest):
    return [unescape(m.group(1)) for m in re.finditer(r'"((?:[^"\\]|\\.)*)"', rest)]


def word_ref(expr):
    """A literal naming a label: (label, addend), or None."""
    m = re.fullmatch(r'\s*([A-Za-z_][\w]*)\s*(?:([+-])\s*(0x[0-9A-Fa-f]+|\d+))?\s*', expr)
    if not m:
        return None
    add = int(m.group(3), 0) if m.group(3) else 0
    return m.group(1), -add if m.group(2) == '-' else add


def sizes(f, consts, base):
    """First pass: each statement's offset, kind, size and instruction set,
    the labels' offsets and the literal pools' contents. Returns the
    statements, the labels and the offset it ends at (None: cut short)."""
    out, labels, pending = [], {}, []
    off, mode = 0, f.mode
    for kind, data, no in f.items:
        if kind == 'label':
            labels.setdefault(data, off)
            continue
        if kind == 'mode':
            mode = data
            continue
        if kind == 'end':
            return out, labels, off
        op, rest = data
        o = op.lower()
        if o in DATA:
            n = DATA[o] * len(split_ops(rest))
            out.append((off, 'data', (o, rest), n, mode))
            off += n
        elif o in ('.align', '.balign', '.p2align'):
            args = split_ops(rest)
            k = consts.value(args[0]) if args else None
            if k is None:
                return out, labels, None
            pad = (-(base + off)) % (k if o == '.balign' else 1 << k)
            out.append((off, 'pad', args[1] if len(args) > 1 else None, pad, mode))
            off += pad
        elif o in ('.asciz', '.string', '.ascii'):
            n = sum(len(s) + (0 if o == '.ascii' else 1) for s in strings(rest))
            out.append((off, 'text', (o, rest), n, mode))
            off += n
        elif o in ('.space', '.skip', '.zero'):
            n = consts.value(split_ops(rest)[0])
            if n is None:
                return out, labels, None
            out.append((off, 'pad', '0', n, mode))
            off += n
        elif o == '.incbin':
            args = split_ops(rest)
            path = os.path.join(consts.root, args[0].strip('"'))
            if not os.path.exists(path):
                return out, labels, None
            n = os.path.getsize(path)
            if len(args) > 1:
                n -= consts.value(args[1]) or 0
            if len(args) > 2:
                n = min(n, consts.value(args[2]) or n)
            out.append((off, 'blob', None, n, mode))
            off += n
        elif o in ('.pool', '.ltorg'):
            pad = (-(base + off)) % 4
            out.append((off, 'pad', None, pad, mode))
            off += pad
            for expr in dict.fromkeys(pending):
                out.append((off, 'literal', expr, 4, mode))
                off += 4
            pending = []
        elif o in QUIET:
            continue
        elif o == 'movflag' and mode == 'thumb':
            out.append((off, 'insn', ('mov', f'r0, #({rest}) >> 8'), 2, mode))
            out.append((off + 2, 'insn', ('mov', f'r1, #({rest}) & 0xFF'), 2, mode))
            off += 4
        elif mode == 'thumb' and o in THUMB_OPS:
            m = re.match(r'\s*(r\d+)\s*,\s*=\s*(.+)$', rest) if o == 'ldr' else None
            if m:
                v = consts.value(m.group(2))
                if v is not None and 0 <= v <= 0xFF:   # (GNU as makes it a mov in divided syntax)
                    out.append((off, 'insn', ('mov', f'{m.group(1)}, #{v}'), 2, mode))
                    off += 2
                    continue
                pending.append(m.group(2).strip())
            n = 4 if o == 'bl' else 2
            out.append((off, 'insn', (op, rest), n, mode))
            off += n
        elif mode == 'arm' and not o.startswith('.'):
            m = re.match(r'\s*(\w+)\s*,\s*=\s*(.+)$', rest) if o.startswith('ldr') else None
            if m:
                pending.append(m.group(2).strip())
            out.append((off, 'insn', (op, rest), 4, mode))
            off += 4
        else:
            return out, labels, None
    return out, labels, off


def make_layout(f, consts, base):
    """f's bytes with its start at base (only base's place in a word
    matters: its alignments and pc-relative loads)."""
    stmts, labels, end = sizes(f, consts, base)
    lay = Layout()
    lay.labels, lay.size = labels, end
    ctx = Ctx(consts, {name: base + o for name, o in labels.items()})
    pool_at = {}
    for off, kind, data, n, mode in stmts:
        if kind == 'literal':
            pool_at.setdefault(data, off)
    for off, kind, data, n, mode in stmts:
        b = []
        if kind == 'insn':
            op, rest = data
            try:
                words, refs = (arm if mode == 'arm' else thumb)(op, rest, base + off, ctx)
                width = 4 if mode == 'arm' else 2
                for v, m in words:
                    b += [((v >> (8 * j)) & 0xFF, (m >> (8 * j)) & 0xFF) for j in range(width)]
                for ro, rkind, name in refs:
                    if rkind == 'pool':
                        r = word_ref(name) if name in pool_at else None
                        if r:
                            lay.refs.append((pool_at[name], 'word', r[0], r[1]))
                        continue
                    lay.refs.append((off + ro, rkind, name, 0))
            except (Unknown, KeyError, ValueError, IndexError, AttributeError, TypeError):
                b = [(0, 0)] * n
                lay.unknown += 1
        elif kind == 'data':
            o, rest = data
            width = DATA[o]
            lay.data.append((off, off + n))
            for k, expr in enumerate(split_ops(rest)):
                v = consts.value(expr)
                if v is None:
                    r = word_ref(expr) if width == 4 else None
                    if r:
                        lay.refs.append((off + k * width, 'word', r[0], r[1]))
                    b += [(0, 0)] * width
                else:
                    v &= (1 << (8 * width)) - 1
                    b += [((v >> (8 * j)) & 0xFF, 0xFF) for j in range(width)]
        elif kind == 'literal':
            lay.data.append((off, off + 4))
            v = consts.value(data)
            b = [(0, 0)] * 4 if v is None else [(((v & 0xFFFFFFFF) >> (8 * j)) & 0xFF, 0xFF) for j in range(4)]
        elif kind == 'text':
            lay.data.append((off, off + n))
            o, rest = data
            for s in strings(rest):
                b += [(c, 0xFF) for c in s] + ([] if o == '.ascii' else [(0, 0xFF)])
        elif kind == 'pad':
            fill = consts.value(data) if data is not None else None
            b = [(fill & 0xFF, 0xFF) if fill is not None else (0, 0)] * n
        elif kind == 'blob':   # (an included file: data, its bytes not kept from here on)
            lay.data.append((off, off + n))
            if lay.stop is None:
                lay.stop = len(lay.bytes)
        if lay.stop is None:
            lay.bytes += b
    if end is None and lay.stop is None:
        lay.stop = len(lay.bytes)
    return lay


def anchor_start(f, layouts, after=None):
    """f's Falzar address from the addresses in its labels' names, as each
    alignment of its start would lay it out: the start most of them agree
    on (where both alignments agree with themselves, the one that follows
    the function before, `after`), and how many disagree."""
    votes = {}
    for r, lay in layouts.items():
        for name, addr in f.anchors:
            if name in lay.labels:
                s = addr - lay.labels[name]
                if s % 4 == r:
                    votes[s] = votes.get(s, 0) + 1
    if not votes:
        return None, 0
    top = max(votes.values())
    best = [s for s, n in votes.items() if n == top]
    start = after if after in best else min(best)
    return start, sum(votes.values()) - top


# ---- bn6f's functions, laid out at their Falzar addresses ----

class Fn:
    """A bn6f function: where it is in Falzar, its bytes for each place in a
    word its start may take, and where the searches found it."""

    def __init__(self, f, index):
        self.f, self.index = f, index
        self.name, self.falzar, self.layouts = f.name, None, {}
        self.hits = None      # where the last search found its bytes (None: too little known to look)

    def layout_at(self, address):
        """Its layout for a start at address (its place in a word)."""
        return self.layouts.get(address % 4, self.layouts[0])


def lay_out(functions, consts):
    """Each function's layouts and Falzar address: from its labels' names,
    else right after the function before it where nothing stands between."""
    fns, prev_end = [], None
    for k, f in enumerate(functions):
        fn = Fn(f, k)
        rs = (0, 1, 2, 3) if f.kind == 'data' else (0, 2) if f.mode == 'thumb' and f.align < 2 else (0,)
        fn.layouts = {r: make_layout(f, consts, 0x08000000 + r) for r in rs}
        after = None
        if prev_end is not None and not f.after_gap:
            step = 1 << f.align
            after = (prev_end + step - 1) // step * step
        start, fn.disagree = anchor_start(f, fn.layouts, after)
        fn.falzar = start if start is not None else after
        lay = fn.layout_at(fn.falzar) if fn.falzar is not None else None
        prev_end = fn.falzar + lay.size if lay is not None and lay.size is not None else None
        fns.append(fn)
    return fns


# ---- looking for the bytes in a ROM ----

class Signature:
    """A layout's bytes with their masks, as runs to compare: the longest run
    of whole bytes is the one looked for."""

    def __init__(self, lay):
        bs = lay.bytes[:lay.stop] if lay.stop is not None else lay.bytes
        while bs and not bs[-1][1]:
            bs = bs[:-1]
        self.length = len(bs)
        self.known = sum(bin(m).count('1') for v, m in bs)
        self.runs, self.partial = [], []
        k = 0
        while k < len(bs):
            if bs[k][1] == 0xFF:
                j = k
                while j < len(bs) and bs[j][1] == 0xFF:
                    j += 1
                self.runs.append((k, bytes(v for v, m in bs[k:j])))
                k = j
            else:
                if bs[k][1]:
                    self.partial.append((k, bs[k][0] & bs[k][1], bs[k][1]))
                k += 1
        self.anchor = max(self.runs, key=lambda r: len(r[1])) if self.runs else None

    def at(self, rom, start):
        if start < 0 or start + self.length > len(rom):
            return False
        for off, run in self.runs:
            if rom[start + off:start + off + len(run)] != run:
                return False
        return all(rom[start + off] & m == v for off, v, m in self.partial)

    def find(self, rom, step, cap=64):
        """Where in rom the bytes are (each start a multiple of step), at most
        cap of them; None where too little of it is known to look."""
        if self.anchor is None or len(self.anchor[1]) < 4 or self.known < 48:
            return None
        a, needle = self.anchor
        out, pos = [], rom.find(needle)
        while pos != -1:
            start = pos - a
            if start % step == 0 and self.at(rom, start):
                out.append(start)
                if len(out) > cap:
                    break
            pos = rom.find(needle, pos + 1)
        return out


def search(fns, rom):
    """Each function's places in rom, for each layout (a layout made for one
    place in a word counts only where its start takes that place)."""
    for fn in fns:
        if fn.f.kind == 'data':
            continue
        found = set()
        sigs = {}
        for r, lay in fn.layouts.items():
            sig = Signature(lay)
            key = (tuple(sig.runs), tuple(sig.partial), sig.length)
            sigs.setdefault(key, []).append((r, sig))
        anywhere = len(sigs) == 1
        looked = False
        for key, variants in sigs.items():
            r, sig = variants[0]
            step = 4 if fn.f.mode == 'arm' or fn.f.align >= 2 else 2
            hits = sig.find(rom, step)
            if hits is None:
                continue
            looked = True
            for h in hits:
                if anywhere or any(h % 4 == rr for rr, _ in variants):
                    found.add(0x08000000 + h)
        fn.hits = sorted(found) if looked else None


# ---- what the located functions name: calls, branches, literals ----

def u16(rom, a):
    return rom[a] | rom[a + 1] << 8


def u32(rom, a):
    return rom[a] | rom[a + 1] << 8 | rom[a + 2] << 16 | rom[a + 3] << 24


def sext(v, bits):
    return v - (1 << bits) if v & (1 << (bits - 1)) else v


def decode(rom, pc, kind, mode):
    """The address a reference at pc names, read from the ROM; None where
    the bytes there are not that kind of reference."""
    a = pc - 0x08000000
    if a < 0 or a + 4 > len(rom):
        return None
    if mode == 'arm':
        w = u32(rom, a)
        if kind in ('bl', 'b'):
            return pc + 8 + (sext(w & 0xFFFFFF, 24) << 2) if (w >> 25) & 7 == 5 else None
        if kind == 'ldr':
            if w & 0x0F7F0000 != 0x051F0000:
                return None
            return pc + 8 + (w & 0xFFF if w & (1 << 23) else -(w & 0xFFF))
        return u32(rom, a) if kind == 'word' else None
    h = u16(rom, a)
    if kind == 'bl':
        h2 = u16(rom, a + 2)
        if h & 0xF800 != 0xF000 or h2 & 0xF800 != 0xF800:
            return None
        return pc + 4 + sext((h & 0x7FF) << 12 | (h2 & 0x7FF) << 1, 23)
    if kind == 'b':
        if h & 0xF800 == 0xE000:
            return pc + 4 + sext((h & 0x7FF) << 1, 12)
        if h & 0xF000 == 0xD000 and (h >> 8) & 0xF < 14:
            return pc + 4 + sext((h & 0xFF) << 1, 9)
        return None
    if kind == 'ldr':
        return ((pc + 4) & ~3) + ((h & 0xFF) << 2) if h & 0xF800 == 0x4800 else None
    if kind == 'word':
        return u32(rom, a)
    return None


def references(fn, rom, at):
    """What fn's references name, read where it was found: (name, address)."""
    lay = fn.layout_at(at)
    out = []
    for off, kind, name, addend in lay.refs:
        mode = fn.f.mode
        v = decode(rom, at + off, kind, mode)
        if v is None:
            continue
        out.append((name, v - addend if kind == 'word' else v))
    return out


def ram_labels(root):
    """bn6f's EWRAM and IWRAM labels with the addresses its comments give
    (where a name ends in another address, the name's: eStructArr2008450's
    comment says 0x2008453, its code reads 0x2008450)."""
    out = {}
    for name in ('ewram.s', 'iwram_data.s'):
        path = os.path.join(root, name)
        for line in open(path, encoding='utf-8', errors='replace'):
            m = re.match(r'^([A-Za-z_]\w*)::?\s*//\s*(0x[0-9A-Fa-f]+)', line.strip())
            if m:
                own = re.search(r'([23][0-9A-Fa-f]{6})$', m.group(1))
                out.setdefault(m.group(1), int(own.group(1), 16) if own else int(m.group(2), 16))
    return out


# ---- where each function is: its bytes, what names it, its neighbours ----

# how a function was located, the surest first: 'bytes' (found once),
# 'calls+bytes' (its bytes at the place the references name),
# 'neighbours' (its bytes at the shift its neighbours share), 'calls'
# (named by references alone: its bytes differ), 'shift NN%' (estimated).
# The first three find its bytes there as bn6f lays them out:
VERIFIED = ('bytes', 'calls+bytes', 'neighbours')


def locate(fns, rom, neighbours=True):
    """Each function's place in rom (locate_once), again without the finds
    by bytes that more located references name elsewhere than there (two at
    least): another function's bytes, the same as its."""
    banned = set()
    for _ in range(4):
        out, votes = locate_once(fns, rom, neighbours, banned)
        wrong = {k for k, (a, how, agree, other) in out.items() if how in VERIFIED and other >= 2 and other > agree}
        if not wrong - banned:
            return out, votes
        banned |= wrong
    return out, votes


def locate_once(fns, rom, neighbours, banned):
    """Each function's place in rom: found once by its bytes (where several
    functions are found at one place, the one its neighbours agree with;
    with neighbours, none found where both of its located neighbours say
    it is not); where the located functions call, branch to and point at,
    all agreeing; at the shift its neighbours in its file share, its bytes
    there as bn6f lays them out, or, failing that, most of its known bits.
    neighbours=False (another game, laid out otherwise): bytes and calls.
    banned: functions not to be placed by their bytes alone.
    Returns {index: (address, how, references that agree, that do not)}
    and the references' votes ({name: {address: count}})."""
    by_name = {}
    for fn in fns:
        if fn.f.kind == 'function':
            by_name.setdefault(fn.name, []).append(fn)
    where = {fn.index: (fn.hits[0], 'bytes') for fn in fns
             if fn.hits is not None and len(fn.hits) == 1 and fn.index not in banned}
    one_each(fns, where)
    if neighbours:
        out_of_place(fns, where)
    votes, done = {}, set()
    for _ in range(10):
        for fn in fns:
            if fn.index in where and fn.index not in done and (where[fn.index][1] in VERIFIED or fn.f.kind == 'data'):
                done.add(fn.index)
                for name, v in references(fn, rom, where[fn.index][0]):
                    votes.setdefault(name, {}).setdefault(v, 0)
                    votes[name][v] += 1
        spans = Spans(fns, where)
        changed = False
        for fn in fns:
            if fn.index in where or fn.f.kind != 'function' or len(by_name[fn.name]) != 1:
                continue
            v = votes.get(fn.name)
            if v and len(v) == 1:
                a = next(iter(v)) & ~1
                if 0x08000000 <= a < 0x08000000 + len(rom) and spans.free(a):
                    # (its bytes there as bn6f lays them out: as sure as a find)
                    sig = Signature(fn.layout_at(a))
                    same = sig.known >= 16 and sig.at(rom, a - 0x08000000)
                    where[fn.index] = (a, 'calls+bytes' if same else 'calls')
                    lay = fn.layout_at(a)
                    spans.add(a, a + ((lay.size or lay.stop or 2) if same else 2))
                    changed = True
        changed |= by_tables(fns, rom, where, votes)
        if neighbours:
            changed |= by_neighbours(fns, rom, where, spans)
        if not changed:
            break
    if neighbours:
        by_shift(fns, rom, where, Spans(fns, where))
    untangle(fns, where)
    out = {}
    for fn in fns:
        if fn.index in where:
            a, how = where[fn.index]
            v = votes.get(fn.name, {}) if len(by_name.get(fn.name, ())) == 1 else {}
            run = a if fn.falzar is None or fn.falzar >> 24 != 3 else None
            agree = sum(n for addr, n in v.items() if run is not None and addr & ~1 == run)
            other = sum(n for addr, n in v.items() if run is not None and addr & ~1 != run)
            out[fn.index] = (a, how, agree, other)
    return out, votes


def by_tables(fns, rom, where, votes):
    """The data between functions (jump tables, pools, records) at the place
    the located functions' references give its labels, all agreeing, its
    known bytes there: then its own references name more."""
    changed = False
    for fn in fns:
        if fn.f.kind != 'data' or fn.index in where:
            continue
        starts = set()
        for r, lay in fn.layouts.items():
            for label, off in lay.labels.items():
                v = votes.get(label)
                if v and len(v) == 1:
                    s = next(iter(v)) - off
                    if s % 4 == r:
                        starts.add(s)
        if len(starts) != 1:
            continue
        s = starts.pop()
        sig = Signature(fn.layout_at(s))
        if 0x08000000 <= s < 0x08000000 + len(rom) and (not sig.length or sig.at(rom, s - 0x08000000)):
            where[fn.index] = (s, 'calls')
            changed = True
    return changed


class Spans:
    """The bytes the located functions take in the ROM: where one starts no
    other may start or lie."""

    def __init__(self, fns, where):
        import bisect
        self.bisect, self.items = bisect, []
        for fn in fns:
            if fn.index in where:
                a, how = where[fn.index]
                lay = fn.layout_at(a)
                size = (lay.size or lay.stop or 2) if how in VERIFIED else 2
                self.items.append((a, a + size))
        self.items.sort()

    def free(self, a):
        k = self.bisect.bisect_right(self.items, (a, 1 << 40)) - 1
        return k < 0 or self.items[k][1] <= a

    def add(self, a, end):
        self.bisect.insort(self.items, (a, end))


def one_each(fns, where):
    """Where the bytes of several functions were found at one place (bn6f has
    them twice, Gregar once): the one whose neighbours moved as it did,
    else none."""
    at = {}
    for k, (a, how) in where.items():
        at.setdefault(a, []).append(k)
    clash = {k for ks in at.values() if len(ks) > 1 for k in ks}
    if not clash:
        return
    calm = {k: v for k, v in where.items() if k not in clash}
    for a, ks in at.items():
        if len(ks) < 2:
            continue
        keep = [k for k in ks if fns[k].falzar is not None and neighbour_shift(fns, k, calm) == a - fns[k].falzar]
        for k in ks:
            if keep != [k]:
                del where[k]


def out_of_place(fns, where):
    """A function found once by its bytes, but where both of its located
    neighbours, moved alike, say it is not: another function's bytes, the
    same as its (it changed between the versions); not taken."""
    wrong = []
    for k in where:
        d = neighbour_shift(fns, k, where) if fns[k].falzar is not None else None
        if d is not None and where[k][0] - fns[k].falzar != d:
            wrong.append(k)
    for k in wrong:
        del where[k]


def by_neighbours(fns, rom, where, spans):
    """A function its bytes did not place: at the shift its located
    neighbours in its file share, where its bytes are as bn6f lays them
    out (of several finds, or too few known bits to look for)."""
    changed = False
    for k, fn in enumerate(fns):
        if fn.index in where or fn.falzar is None:
            continue
        d = neighbour_shift(fns, k, where)
        if d is None:
            continue
        a = fn.falzar + d
        sig = Signature(fn.layout_at(a))
        if sig.known >= 16 and spans.free(a) and sig.at(rom, a - 0x08000000):
            where[fn.index] = (a, 'neighbours')
            lay = fn.layout_at(a)
            spans.add(a, a + (lay.size or lay.stop or 2))
            changed = True
    return changed


def neighbour_shift(fns, k, where):
    """How far the located functions before and after fns[k] in its file
    moved from Falzar, where the two agree (their bytes found there)."""
    path = fns[k].f.path
    shifts = []
    for step in (-1, 1):
        j = k + step
        while 0 <= j < len(fns) and fns[j].f.path == path:
            if fns[j].index in where and fns[j].falzar is not None and where[fns[j].index][1] in VERIFIED:
                shifts.append(where[fns[j].index][0] - fns[j].falzar)
                break
            j += step
    if len(shifts) == 2 and shifts[0] == shifts[1]:
        return shifts[0]
    return None


def by_shift(fns, rom, where, spans):
    """A function whose bytes are nowhere, between two located neighbours
    that moved alike: at that shift, where at least 85% of its known bits
    agree with the ROM's (how many, its evidence; it changed a little)."""
    for k, fn in enumerate(fns):
        if fn.index in where or fn.f.kind != 'function' or fn.falzar is None or fn.falzar >> 24 != 8:
            continue
        d = neighbour_shift(fns, k, where)
        if d is None or not spans.free(fn.falzar + d):
            continue
        a = fn.falzar + d
        lay = fn.layout_at(a)
        bs = lay.bytes[:lay.stop] if lay.stop is not None else lay.bytes
        bits = same = 0
        for i, (v, m) in enumerate(bs):
            if not m or a - 0x08000000 + i >= len(rom):
                continue
            bits += bin(m).count('1')
            same += bin(~(rom[a - 0x08000000 + i] ^ v) & m & 0xFF).count('1')
        if bits >= 64 and same >= 0.85 * bits:
            where[fn.index] = (a, f'shift {100 * same // bits}%')
            spans.add(a, a + 2)


def untangle(fns, where):
    """Two functions whose bytes lie over each other: the one whose shift
    from Falzar a located neighbour shares stays, else neither."""
    spans = []
    for k, (a, how, *_) in where.items():
        lay = fns[k].layout_at(a)
        spans.append((a, a + ((lay.size or lay.stop or 2) if how in VERIFIED else 2), k))
    spans.sort()
    drop = set()
    for (a0, e0, k0), (a1, e1, k1) in zip(spans, spans[1:]):
        if a1 < e0:
            for k in (k0, k1):
                if not any(abs(j - k) <= 2 and j not in (k0, k1) and j in where and fns[j].falzar is not None and
                           fns[k].falzar is not None and where[j][0] - fns[j].falzar == where[k][0] - fns[k].falzar
                           for j in range(k - 2, k + 3)):
                    drop.add(k)
    for k in drop:
        del where[k]
    return len(drop)


# ---- the tables ----

def iwram(fns, where):
    """The IWRAM code (asm38.s) is copied from the ROM to IWRAM in one piece:
    each function runs at its copy's place less the shift of the piece's
    first function (Gregar's is 4 bytes shorter from 0x03006F20 on, so
    those run 4 bytes before Falzar's). Returns {index: run address} and
    the shift."""
    first = min(((fns[k].falzar, a) for k, (a, how, *_) in where.items()
                 if fns[k].falzar is not None and fns[k].falzar >> 24 == 3 and how in VERIFIED), default=None)
    if first is None:
        return {}, None
    d = first[1] - first[0]
    return {k: a - d for k, (a, *_) in where.items() if fns[k].falzar is not None and fns[k].falzar >> 24 == 3}, d


COLUMNS = ('kind', 'name', 'falzar', 'address', 'size', 'mode', 'scope', 'source', 'confidence', 'refs', 'against', 'pools')


def tables(fns, where, votes, ram, rom_key, commit):
    """The CSV: the bn6f commit it was made from (a row of kind "# bn6f"),
    every function (for Gregar, located or not), then the data labels of
    the data located between functions and the ones only the references
    name, then the RAM labels the literals hold as Falzar's. Returns its
    text and what it holds."""
    out = io.StringIO()
    w = csv.writer(out, lineterminator='\n')
    w.writerow(COLUMNS)
    w.writerow(('# bn6f', commit) + ('',) * (len(COLUMNS) - 2))
    run, shift = iwram(fns, where) if rom_key == 'bn6-gregar' else ({}, None)
    told = dict(functions=0, located=0, data=0, ram=0, ram_other=0, iwram_shift=shift)
    defined = set()
    for fn in fns:
        f = fn.f
        for label in fn.layout_at(0).labels:
            defined.add(label)
        if f.kind != 'function':
            continue
        told['functions'] += 1
        hit = where.get(fn.index)
        name = fn.name
        if hit is None:
            if rom_key == 'bn6-gregar':
                w.writerow(('function', name, hx(fn.falzar), '', '', f.mode, f.scope, f'{f.path}:{f.line}', '', '', '', ''))
            continue
        a, how, agree, other = hit
        lay = fn.layout_at(a)
        size = lay.size if how in VERIFIED and lay.size is not None else ''
        if size and rom_key != 'bn6-gregar':
            # (another game: its literal pools may be shorter than Falzar's, so up to its last known byte)
            size = max((i + 1 for i, (v, m) in enumerate(lay.bytes) if m), default=0)
        pools = ';'.join(f'{p:X}-{q:X}' for p, q in lay.data if q <= size) if size else ''
        if fn.falzar is not None and fn.falzar >> 24 == 3:
            if fn.index not in run:   # (its copy not laid out as Falzar's: where it runs is not known)
                w.writerow(('function', name, hx(fn.falzar), '', '', f.mode, f.scope, f'{f.path}:{f.line}', '', '', '', ''))
                continue
            a = run[fn.index]   # (it runs from IWRAM, copied there from the ROM)
        told['located'] += 1
        w.writerow(('function', name, hx(fn.falzar), hx(a), size, f.mode, f.scope, f'{f.path}:{f.line}', how,
                    agree or '', other or '', pools))
    for fn in fns:
        if fn.f.kind != 'data' or fn.index not in where:
            continue
        a, how, *_ = where[fn.index]
        lay = fn.layout_at(a)
        for label, off in sorted(lay.labels.items(), key=lambda kv: kv[1]):
            if label.startswith('.') or not 0x08000000 <= a + off < 0x0A000000:
                continue
            told['data'] += 1
            w.writerow(('data', label, hx(fn.falzar + off if fn.falzar is not None else None), hx(a + off), '', '', '',
                        f'{fn.f.path}:{fn.f.line}', how, sum(votes.get(label, {}).values()) or '', '', ''))
    for label in sorted(votes):
        v = votes[label]
        if len(v) != 1 or label in defined:
            continue
        a, n = next(iter(v.items()))
        if label in ram:
            told['ram' if a == ram[label] else 'ram_other'] += 1
            w.writerow(('ram', label, hx(ram[label]), hx(a), '', '', '', 'ewram.s', 'calls' if a == ram[label] else 'moved', n, '', ''))
        elif 0x08000000 <= a < 0x0A000000:
            m = ANCHOR.match(label)
            told['data'] += 1
            w.writerow(('data', label, hx(address_of(m.group(2))) if m else '', hx(a), '', '', '', '', 'calls', n, '', ''))
    return out.getvalue(), told


def hx(v):
    return '' if v is None else f'0x{v:08X}'


def report(fns, consts, rom, name, where):
    """One function: its statements' bytes and masks against the ROM's where
    it was found, else at the shift its located neighbours share, else at
    its Falzar address."""
    for fn in fns:
        if fn.name != name:
            continue
        hit = where.get(fn.index)
        d = neighbour_shift(fns, fn.index, where) if not hit and fn.falzar is not None else None
        a = hit[0] if hit else fn.falzar + d if d is not None else fn.falzar
        print(f'{fn.name} ({fn.f.scope}, {fn.f.mode}) {fn.f.path}:{fn.f.line}: Falzar {hx(fn.falzar)}, '
              f'{"found at " + hx(hit[0]) + " by " + hit[1] if hit else "not found, shown at " + hx(a)}, its bytes found at '
              f'{", ".join(hx(h) for h in (fn.hits or [])[:8]) or "no place"}')
        if a is None:
            continue
        stmts, labels, end = sizes(fn.f, consts, a)
        lay = fn.layout_at(a)
        at = {o: n for n, o in labels.items()}
        for off, kind, data, n, mode in stmts:
            want = ' '.join(f'{v:02X}' if m == 0xFF else ('..' if not m else f'{v:02X}/{m:02X}') for v, m in lay.bytes[off:off + n])
            got = ' '.join(f'{rom[a - 0x08000000 + off + j]:02X}' for j in range(n)) if 0x08000000 <= a < 0x08800000 else ''
            same = all(rom[a - 0x08000000 + off + j] & m == v & m for j, (v, m) in enumerate(lay.bytes[off:off + n])) if got else True
            print(f'  {"  " if same else "!="} +{off:04X} {at.get(off, ""):>16} {kind:7} {str(data)[:44]:44} {want:24} {got}')


def commit_of(root):
    """The commit a git checkout of bn6f is at (from its .git, without git),
    else ''."""
    git = os.path.join(root, '.git')
    try:
        with open(os.path.join(git, 'HEAD')) as f:
            head = f.read().strip()
        if not head.startswith('ref: '):
            return head
        ref = head[5:]
        if os.path.exists(os.path.join(git, ref)):
            with open(os.path.join(git, ref)) as f:
                return f.read().strip()
        with open(os.path.join(git, 'packed-refs')) as f:
            return next((line.split()[0] for line in f if line.rstrip().endswith(' ' + ref)), '')
    except OSError:
        return ''


def find_rom(folder, sha1):
    """The ROM in folder with this SHA-1 (any .gba of 8 MB), or None."""
    for name in sorted(os.listdir(folder)) if os.path.isdir(folder) else []:
        path = os.path.join(folder, name)
        if name.lower().endswith('.gba') and os.path.getsize(path) == 0x800000:
            with open(path, 'rb') as f:
                data = f.read()
            if hashlib.sha1(data).hexdigest() == sha1:
                return data
    return None


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('--bn6f', required=True, help='a checkout of github.com/dism-exe/bn6f')
    ap.add_argument('--roms', default=os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms')))
    ap.add_argument('--out', default=MATCH, help='where the tables go (.build/symbols/match)')
    ap.add_argument('--report', metavar='FUNCTION', help='print one function against the Gregar ROM, write nothing')
    a = ap.parse_args()
    if not os.path.isfile(os.path.join(a.bn6f, 'rom.s')):
        sys.exit(f'{a.bn6f}: not a bn6f checkout (no rom.s)')
    consts = Constants(a.bn6f)
    fns = lay_out(read_functions(a.bn6f, consts), consts)
    ram = ram_labels(a.bn6f)
    commit = commit_of(a.bn6f)
    os.makedirs(a.out, exist_ok=True)
    for key, sha1, out in ROMS:
        rom = find_rom(a.roms, sha1)
        if rom is None:
            print(f'{key}: no ROM of SHA-1 {sha1} in {a.roms}, its table kept as it is')
            continue
        search(fns, rom)
        where, votes = locate(fns, rom, neighbours=key == 'bn6-gregar')
        if a.report:
            report(fns, consts, rom, a.report, where)
            return 0
        text, told = tables(fns, where, votes, ram, key, commit)
        with open(os.path.join(a.out, out), 'w', encoding='utf-8', newline='\n') as f:
            f.write(text)
        hows = {}
        for k, (addr, how, *_) in where.items():
            if fns[k].f.kind == 'function':
                hows[how.split()[0]] = hows.get(how.split()[0], 0) + 1
        print(f'{key}: {told["located"]} of {told["functions"]} bn6f functions located '
              f'({", ".join(f"{n} by {h}" for h, n in hows.items())}), {told["data"]} data labels, '
              f'{told["ram"]} RAM labels as Falzar\'s ({told["ram_other"]} not); {os.path.join(a.out, out)}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
