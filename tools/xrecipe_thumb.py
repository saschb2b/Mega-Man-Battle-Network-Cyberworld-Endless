"""xrecipe_thumb.py: the ARMv4T Thumb code of a ROM read for tools/xrecipe.py (issue #82): each instruction decoded
from its bits, a routine walked from its entry (its branches followed, its calls, literal words and jump tables
noted), and a routine's body normalised so that two games' copies of one routine compare equal (its calls' targets,
its literals' values by their class, its branches by their place in it). Nothing here is the games': instructions are
decoded as the CPU reads them, and only addresses and numbers come out."""
import re
import struct

BUS = 0x08000000
ROM_END = BUS + 0x2000000


class Rom:
    """A ROM's bytes and its bus addresses"""

    def __init__(self, data, name):
        self.data, self.name = data, name

    def has(self, a, n=1):
        return BUS <= a and a + n <= BUS + len(self.data)

    def u8(self, a):
        return self.data[a - BUS]

    def u16(self, a):
        return struct.unpack_from('<H', self.data, a - BUS)[0]

    def u32(self, a):
        return struct.unpack_from('<I', self.data, a - BUS)[0]

    def bytes(self, a, n):
        return self.data[a - BUS:a - BUS + n]


def sext(v, bits):
    return v - (1 << bits) if v & (1 << (bits - 1)) else v


def is_code_ptr(v, rom=None):
    """a word that may be a Thumb routine's address in the ROM (bit 0 set)"""
    return v & 1 and BUS <= v < BUS + 0x800000 and (rom is None or rom.has(v & ~1, 2))


class Insn:
    """One instruction: its address, size, form, and fields (form-specific); `target` a branch's or call's, `lit` a
    pc-relative load's word address, `adr` an address add rd,pc gives"""
    __slots__ = ('at', 'size', 'form', 'f', 'target', 'lit', 'adr', 'text')

    def __init__(self, at, size, form, f=(), target=None, lit=None, adr=None, text=''):
        self.at, self.size, self.form, self.f, self.target, self.lit, self.adr, self.text = at, size, form, f, target, lit, adr, text


ALU = ('ands', 'eors', 'lsls', 'lsrs', 'asrs', 'adcs', 'sbcs', 'rors', 'tst', 'negs', 'cmp', 'cmn', 'orrs', 'muls', 'bics', 'mvns')
COND = ('eq', 'ne', 'cs', 'cc', 'mi', 'pl', 'vs', 'vc', 'hi', 'ls', 'ge', 'lt', 'gt', 'le')
MEM = {(0, 0): 'str', (0, 1): 'strb', (1, 0): 'ldr', (1, 1): 'ldrb'}
MEM_H = {0: 'strh', 1: 'ldsb', 2: 'ldrh', 3: 'ldsh'}


def rlist(bits, extra=None):
    regs = [f'r{i}' for i in range(8) if bits >> i & 1]
    return '{' + ', '.join(regs + ([extra] if extra else [])) + '}'


def decode(rom, at):
    """The instruction at `at` (an Insn); form 'bad' where the halfwords decode as none ARMv4T's Thumb has"""
    h = rom.u16(at)
    top = h >> 11
    if top == 0x1E:   # bl, its first half
        lo = rom.u16(at + 2) if rom.has(at + 2, 2) else 0
        if lo >> 11 != 0x1F:
            return Insn(at, 2, 'bad', text='half a bl')
        t = at + 4 + sext((h & 0x7FF) << 12 | (lo & 0x7FF) << 1, 23)
        return Insn(at, 4, 'bl', target=t, text=f'bl #{t:#x}')
    if top == 0x1F:
        return Insn(at, 2, 'bad', text='half a bl')
    if h >> 13 == 0 and top != 3:   # lsl/lsr/asr rd,rs,#imm
        op, imm, rs, rd = h >> 11 & 3, h >> 6 & 31, h >> 3 & 7, h & 7
        return Insn(at, 2, ('lsls', 'lsrs', 'asrs')[op], (rd, rs, imm), text=f'{("lsls", "lsrs", "asrs")[op]} r{rd}, r{rs}, #{imm}')
    if top == 3:   # add/sub reg or imm3
        i, sub, rn, rs, rd = h >> 10 & 1, h >> 9 & 1, h >> 6 & 7, h >> 3 & 7, h & 7
        name = ('adds', 'subs')[sub] + ('3' if i else '')
        return Insn(at, 2, name, (rd, rs, rn), text=f'{("adds", "subs")[sub]} r{rd}, r{rs}, {"#" if i else "r"}{rn}')
    if h >> 13 == 1:   # mov/cmp/add/sub imm8
        op, rd, imm = h >> 11 & 3, h >> 8 & 7, h & 0xFF
        name = ('movs', 'cmp', 'adds8', 'subs8')[op]
        return Insn(at, 2, name, (rd, imm), text=f'{("movs", "cmp", "adds", "subs")[op]} r{rd}, #{imm:#x}')
    if h >> 10 == 0x10:   # alu
        op, rs, rd = h >> 6 & 15, h >> 3 & 7, h & 7
        return Insn(at, 2, ALU[op], (rd, rs), text=f'{ALU[op]} r{rd}, r{rs}')
    if h >> 10 == 0x11:   # hi registers, bx
        op, h1, h2, rs, rd = h >> 8 & 3, h >> 7 & 1, h >> 6 & 1, h >> 3 & 7, h & 7
        rd |= h1 << 3
        rs |= h2 << 3
        name = ('add', 'cmp', 'mov', 'bx')[op]
        if name == 'bx':
            return Insn(at, 2, 'bx', (rs,), text=f'bx r{rs}')
        return Insn(at, 2, 'hi_' + name, (rd, rs), text=f'{name} r{rd}, r{rs}')
    if top == 9:   # ldr rd,[pc,#imm]
        rd, imm = h >> 8 & 7, h & 0xFF
        lit = ((at + 4) & ~3) + imm * 4
        return Insn(at, 2, 'ldr_lit', (rd,), lit=lit, text=f'ldr r{rd}, [pc, #{imm * 4:#x}]')
    if h >> 12 == 5:   # register offset
        ro, rb, rd = h >> 6 & 7, h >> 3 & 7, h & 7
        if h >> 9 & 1 == 0:
            name = MEM[(h >> 11 & 1, h >> 10 & 1)] + '_r'
        else:
            name = MEM_H[h >> 10 & 3] + '_r'
        return Insn(at, 2, name, (rd, rb, ro), text=f'{name[:-2]} r{rd}, [r{rb}, r{ro}]')
    if h >> 13 == 3:   # imm5 word/byte
        b, ld, imm, rb, rd = h >> 12 & 1, h >> 11 & 1, h >> 6 & 31, h >> 3 & 7, h & 7
        name = MEM[(ld, b)]
        off = imm if b else imm * 4
        return Insn(at, 2, name, (rd, rb, off), text=f'{name} r{rd}, [r{rb}, #{off:#x}]')
    if h >> 12 == 8:   # strh/ldrh imm
        ld, imm, rb, rd = h >> 11 & 1, h >> 6 & 31, h >> 3 & 7, h & 7
        name = ('strh', 'ldrh')[ld]
        return Insn(at, 2, name, (rd, rb, imm * 2), text=f'{name} r{rd}, [r{rb}, #{imm * 2:#x}]')
    if h >> 12 == 9:   # sp-relative
        ld, rd, imm = h >> 11 & 1, h >> 8 & 7, h & 0xFF
        name = ('str_sp', 'ldr_sp')[ld]
        return Insn(at, 2, name, (rd, imm * 4), text=f'{name[:3]} r{rd}, [sp, #{imm * 4:#x}]')
    if h >> 12 == 10:   # add rd, pc/sp, #imm
        sp, rd, imm = h >> 11 & 1, h >> 8 & 7, h & 0xFF
        if sp:
            return Insn(at, 2, 'add_sp_rd', (rd, imm * 4), text=f'add r{rd}, sp, #{imm * 4:#x}')
        adr = ((at + 4) & ~3) + imm * 4
        return Insn(at, 2, 'adr', (rd,), adr=adr, text=f'add r{rd}, pc, #{imm * 4:#x}')
    if h >> 8 == 0xB0:   # add sp, #+-imm
        s, imm = h >> 7 & 1, h & 0x7F
        return Insn(at, 2, 'add_sp', (-imm * 4 if s else imm * 4,), text=f'add sp, #{-imm * 4 if s else imm * 4}')
    if h >> 12 == 11 and h >> 9 & 3 == 2:   # push/pop
        ld, r, bits = h >> 11 & 1, h >> 8 & 1, h & 0xFF
        if ld:
            return Insn(at, 2, 'pop', (bits, r), text=f'pop {rlist(bits, "pc" if r else None)}')
        return Insn(at, 2, 'push', (bits, r), text=f'push {rlist(bits, "lr" if r else None)}')
    if h >> 12 == 12:   # stmia/ldmia
        ld, rb, bits = h >> 11 & 1, h >> 8 & 7, h & 0xFF
        name = ('stmia', 'ldmia')[ld]
        return Insn(at, 2, name, (rb, bits), text=f'{name} r{rb}!, {rlist(bits)}')
    if h >> 12 == 13:   # b<cond>, swi
        cond = h >> 8 & 15
        if cond == 15:
            return Insn(at, 2, 'swi', (h & 0xFF,), text=f'swi #{h & 0xFF:#x}')
        if cond == 14:
            return Insn(at, 2, 'bad', text='undefined')
        t = at + 4 + sext((h & 0xFF) << 1, 9)
        return Insn(at, 2, 'bcc', (cond,), target=t, text=f'b{COND[cond]} #{t:#x}')
    if top == 0x1C:   # b
        t = at + 4 + sext((h & 0x7FF) << 1, 12)
        return Insn(at, 2, 'b', target=t, text=f'b #{t:#x}')
    return Insn(at, 2, 'bad', text=f'.hword {h:#06x}')


def is_return(i):
    """bx lr, pop {..., pc}, mov pc, lr"""
    return (i.form == 'bx' and i.f[0] == 14) or (i.form == 'pop' and i.f[1]) or (i.form == 'hi_mov' and i.f == (15, 14))


class Walk:
    """A routine walked from its entry: its instructions (address -> Insn), its literal words (address -> value), its
    calls (site -> target), its indirect jumps and calls, the jump tables its literals name (table -> entries), what
    could not be decoded"""
    __slots__ = ('entry', 'insns', 'lits', 'bls', 'indirect', 'calls_ind', 'tables', 'bad', 'adrs')

    def __init__(self, entry):
        self.entry = entry
        self.insns, self.lits, self.bls, self.tables, self.adrs = {}, {}, {}, {}, set()
        self.indirect, self.calls_ind, self.bad = [], [], []


def walk(rom, entry, limit=0x2000, region=None):
    """The routine at `entry` (bit 0 ignored): every instruction reached by its branches from the entry (calls noted,
    not followed), stopping at returns and indirect jumps (mov lr,pc; bx rN is a call); `region` (lo, hi) the branches
    may not leave"""
    entry &= ~1
    w = Walk(entry)
    todo = [entry]
    while todo:
        pc = todo.pop()
        prev = None
        while True:
            if pc in w.insns or not rom.has(pc, 2) or abs(pc - entry) > limit:
                break
            if pc in w.lits or (pc & ~3) in w.lits:
                break
            if region and not region[0] <= pc < region[1]:
                w.bad.append(('leaves its region', pc))
                break
            i = decode(rom, pc)
            w.insns[pc] = i
            if i.lit is not None and rom.has(i.lit, 4):
                w.lits[i.lit] = rom.u32(i.lit)
            if i.adr is not None:
                w.adrs.add(i.adr)
            if i.form == 'bad':
                w.bad.append(('undecodable', pc))
                break
            if i.form == 'bl':
                w.bls[pc] = i.target
            elif i.form == 'b':
                todo.append(i.target)
                break
            elif i.form == 'bcc':
                todo.append(i.target)
            elif i.form == 'bx':
                if i.f[0] == 14:
                    break
                if prev is not None and prev.form == 'hi_mov' and prev.f == (14, 15):
                    w.calls_ind.append(pc)   # (mov lr,pc; bx rN: a call, on after it)
                else:
                    w.indirect.append(pc)
                    break
            elif is_return(i):
                break
            elif i.form in ('hi_mov', 'hi_add') and i.f[0] == 15:
                w.indirect.append(pc)
                break
            prev = i
            pc += i.size
    return w


def lit_class(v):
    """a literal word as two copies of a routine may differ in it and still be the same routine: a number as it is, an
    address by what it is"""
    if v is None:
        return 'none'
    if v < 0x10000:
        return v
    if 0x02000000 <= v < 0x04000000:
        return 'RAM'
    if 0x04000000 <= v < 0x05000000:
        return ('IO', v)
    if 0x05000000 <= v < BUS:
        return ('VRAM', v)
    if BUS <= v < ROM_END:
        return 'CODE' if v & 1 else 'ROM'
    return v


MEM_FORMS = {'ldr', 'str', 'ldrb', 'strb', 'ldrh', 'strh'}


def norm(rom, entry, limit=0x2000):
    """The routine's body as two games' copies of it compare: each instruction in address order, its call's target
    masked, its branches by the instruction they reach (its place among the routine's), its literal by its class; the
    immediate offsets of loads and stores kept apart (the third element) so a comparison may let them differ"""
    w = walk(rom, entry, limit)
    out = []
    addrs = sorted(w.insns)
    place = {a: k for k, a in enumerate(addrs)}
    for a in addrs:
        i = w.insns[a]
        if i.form == 'bl':
            out.append(('bl', None, None))
        elif i.form in ('b', 'bcc'):
            # (a branch by the instruction it reaches, its place among the routine's: the same where a copy's code moved)
            out.append((i.form, (i.f, place.get(i.target, ('out', i.target - w.entry))), None))
        elif i.form == 'ldr_lit':
            out.append(('ldr_lit', i.f, lit_class(w.lits.get(i.lit))))
        elif i.form == 'adr':
            out.append(('adr', i.f, i.adr - w.entry))
        elif i.form in MEM_FORMS:
            out.append((i.form, i.f[:2], ('off', i.f[2])))
        else:
            out.append((i.form, i.f, None))
    return out, w


def compare(rom5, a5, rom6, a6, limit=0x2000):
    """BN5's routine against BN6's: 'SAME' (equal once normalised), 'LAYOUT' (but for the offsets of loads and stores),
    'DIFF' (and where it first differs); with both walks"""
    n5, w5 = norm(rom5, a5, limit)
    n6, w6 = norm(rom6, a6, limit)
    if n5 == n6:
        return 'SAME', None, (w5, w6)
    if len(n5) == len(n6):
        diffs = [(k, x, y) for k, (x, y) in enumerate(zip(n5, n6)) if x != y]
        if all(x[0] == y[0] and x[1] == y[1] and isinstance(x[2], tuple) and x[2][0] == 'off' for _, x, y in diffs):
            addrs = sorted(w5.insns)
            return 'LAYOUT', [(addrs[k], x[2][1], y[2][1]) for k, x, y in diffs], (w5, w6)
    a5s, a6s = sorted(w5.insns), sorted(w6.insns)
    for k, (x, y) in enumerate(zip(n5, n6)):
        if x != y:
            return 'DIFF', (a5s[k], a6s[k], w5.insns[a5s[k]].text, w6.insns[a6s[k]].text), (w5, w6)
    return 'DIFF', ('length', len(n5), len(n6)), (w5, w6)


def listing(rom, w):
    """a walk's instructions as text, one per line (for the audit's notes)"""
    return '\n'.join(f'{a:08x}: {w.insns[a].text}' for a in sorted(w.insns))


def writes(i, reg):
    """whether the instruction writes register `reg` (r0-r7)"""
    f = i.form
    if f in ('str', 'strb', 'strh', 'str_r', 'strb_r', 'strh_r', 'str_sp', 'cmp', 'tst', 'cmn', 'push', 'stmia', 'b', 'bcc', 'bx', 'swi',
             'add_sp', 'hi_cmp'):
        return f == 'stmia' and i.f[0] == reg   # (stmia writes its base back)
    if f == 'pop':
        return bool(i.f[0] >> reg & 1)
    if f == 'ldmia':
        return i.f[0] == reg or bool(i.f[1] >> reg & 1)
    if f == 'bl':
        return reg <= 3
    return bool(i.f) and i.f[0] == reg


def back_value(w, site, reg, n=14):
    """The value `reg` holds at `site` from the instructions before it (address order, within the walk): a movs, then
    the adds and lsls after it, or a literal; (value or None, [(address, form, number)], the first instruction last)"""
    chain = []
    for a in sorted((x for x in w.insns if x < site), reverse=True)[:n]:
        i = w.insns[a]
        if i.form in ('b', 'bx') or is_return(i):
            break
        if not writes(i, reg):
            continue
        if i.form == 'movs':
            chain.append((a, 'movs', i.f[1]))
            return sum(x[2] for x in chain if x[1] in ('movs', 'adds8')), chain
        if i.form == 'adds8':
            chain.append((a, 'adds8', i.f[1]))
            continue
        if i.form == 'lsls' and i.f[1] == reg:
            chain.append((a, 'lsls', i.f[2]))
            adds = [x for x in chain if x[1] == 'adds8']
            return (sum(x[2] for x in adds) if adds else 0), chain
        if i.form == 'ldr_lit':
            chain.append((a, 'ldr_lit', w.lits.get(i.lit)))
            return w.lits.get(i.lit), chain
        return None, chain + [(a, i.form, None)]
    return None, chain
