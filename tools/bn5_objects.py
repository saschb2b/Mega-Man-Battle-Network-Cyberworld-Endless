"""bn5_objects.py: BN5 Team Colonel's engine objects that BN6 Cybeast Gregar (USA) runs otherwise, each moved into BN6
once, whole, for every track that spawns one (issue #82's plan for its tracks: the Navis, the virus families, the chips,
the Souls), and what each one's moved copy draws: the image object, and the sea's marker. Numbers only.
tools/xnavirecipe.py builds the moved copies (src/emu/xnavi_bn5.c, xnavi_objects_bn5; installed by src/emu/xnavi_objects.c
before any Navi, chip's code, family or Soul), checks this file against both ROMs, and refuses a recipe that spawns a kind
this file leaves unfixed; the other tracks' generators read it for the kinds their code spawns and aim their calls into a
module at its copy (tools/xrecipe.py for the families: WuNote's image; tools/xchiprecipe.py for the chips' code: the muzzle
flashes; tools/xsoul.py for the Souls': the arm's flash in a Soul, ShadowSoul's sword, ToadSoul's charged shot;
docs/MULTIROM.md, The engine's objects moved once; docs/ROM_DATA.md)."""

# BN5's image object, the first kind's type 5 (an image an attack shows: a sprite by its kind, the object's +4). BN6's
# type 5 is another object (its own kinds table at 0x080BA444, 52 kinds, other sprites and numbers), so BN5's type 5 is
# moved: its whole module, which nothing outside it jumps into but its spawner and its helpers, copied as one block. Its
# parts: the kinds table (five bytes a kind: the sprite's list offset, its number, its animation, a palette's offset, its
# arm point or 0), the type's routine (its jump table at 0x080B9B94: init, update, and the engine's destroy), the spawner
# every attack calls (r4 its parameters, the kind the low byte; BN5's 0x08003140 with type 5 inside), and the helpers
# other code calls (a kind's sprite, the image's palette)
IMAGE = dict(
    kind=1, type=5,
    region=(0x080B9A5C, 0x080B9D10),   # the module: the kinds table to the next object's routine (0x080B9D10)
    kinds=0x080B9A5C, count=58,        # 0x00-0x39; the table ends at 0x080B9B7E, two bytes before the routine
    routine=0x080B9B80,                # BN5_T1_TYPES' entry 5 (0x08003958 + 4 * 5): 0x080B9B81
    spawner=0x080B9CAE,
    # every routine of the module, each a root of the walk (the helpers have callers outside: 0x080B9CE0, the sprite of a
    # kind, BN5's chips' code)
    roots=(0x080B9B80, 0x080B9BA0, 0x080B9C26, 0x080B9CAE, 0x080B9CD6, 0x080B9CE0, 0x080B9CEE, 0x080B9CFA),
)

# BN5's sea's marker, the fourth kind's type 0x3E: where an object stands submerged on sea (BN5's 0x0800DEAA, from the
# status step of MegaMan and the Navis, 0x08017DB4), a ripple following it (sprite list 3's 0x2D, a splash, effect 0x5D,
# its fourth frame), ended (+8 = 8) as it surfaces. BN6's type 0x3E is its invisibility's shadow, the same code drawing list
# 3's 0x00, and nothing of BN6's spawns it (its spawner 0x080E60CA has no caller): moved, its module whole, for the sea's
# submerge in BN6's engine (src/emu/xsea.c, which calls its spawner)
SEA_MARKER = dict(
    kind=4, type=0x3E,
    region=(0x080E4C4C, 0x080E4D3C),   # the type's routine to the end routine's last instruction
    routine=0x080E4C4C,                # BN5_T4_TYPES' entry 0x3E: 0x080E4C4D (its jump table 0x080E4C64: init, update, the engine's destroy)
    spawner=0x080E4D1E,                # r5 the object it follows: r0 the marker or 0
    end=0x080E4D36,                    # r0 the marker
    roots=(0x080E4C4C, 0x080E4C70, 0x080E4CB2, 0x080E4D1E, 0x080E4D36),
)

# The modules, in the order they are laid out at XN_OBJECTS (src/emu/xnavi_recipe.h), each a block of its own
MODULES = (IMAGE, SEA_MARKER)

# Where the moved copies stand in BN6: XN_OBJECTS (the Navis' code's room, FREE_NAVI_CODE, + 0x100) to XN_OBJECTS_END
# (+ 0x2000), each module one block in MODULES' order, each from a 0x100 boundary the same distance from a word's as in
# BN5. tools/xnavirecipe.py copies them there; a family's or a Navi's code that calls into one is aimed at its copy there
# (tools/xrecipe.py, tools/xnavirecipe.py), and installed after it (src/emu/xinstall.c, xinstall_core)
OBJECTS_AT, OBJECTS_END, OBJECTS_STEP = 0x08800000 + 0x590000 + 0x100, 0x08800000 + 0x590000 + 0x2000, 0x100


def module_places():
    """each module's region in BN5 and where its copy goes: (lo, hi, to)"""
    out, at = [], OBJECTS_AT
    for m in MODULES:
        lo, hi = m['region']
        to = (at + OBJECTS_STEP - 1) // OBJECTS_STEP * OBJECTS_STEP + (lo & 3)
        out.append((lo, hi, to))
        at = to + (hi - lo)
    return out


def in_objects(a):
    """an address of BN5's in one of the modules"""
    return any(lo <= a < hi for lo, hi, _ in module_places())


def objects_at(a):
    """where an address of a module's is in its copy, None for one of no module's"""
    return next((to + (a - lo) for lo, hi, to in module_places() if lo <= a <= hi), None)


def module_of(a):
    """the module an address of BN5's is in, None for none"""
    return next((m for m in MODULES if m['region'][0] <= a < m['region'][1]), None)

# BN5's engine effects BN6 has none of that the project's own code spawns where BN5's engine does: the splash of a
# non-Aqua object stepping onto sea, immobilized (BN5's 0x08017156 at 0x080171E0, src/emu/xsea.c's), effect 0x63 (list
# 5's 0x1F). Each copied with the moved objects, at a number of the Navis' range.
ENGINE_EFFECTS = (0x63,)

# Each kind's sprite in the moved copy, one table for every track: a kind not listed keeps BN5's two bytes (its list
# offset and number), drawn by nothing a track installs ('unfixed': a recipe whose code spawns it is refused until the
# track that needs it adds it here, its art from its own range of numbers).
#
# TWIN: BN6 lists a sprite of the same bytes (each sprite decompressed and compared whole, tools/xnavirecipe.py): its
# list and number in BN6, the kind's two bytes made those (the same as BN5's but for 0x11, 0x37 and 0x2F)
IMAGE_TWIN = {
    0x08: (3, 0x18), 0x0C: (3, 0x16), 0x11: (3, 0x59), 0x13: (3, 0x3B), 0x14: (3, 0x3B), 0x15: (3, 0x3B), 0x16: (3, 0x3B),
    0x17: (3, 0x16), 0x25: (3, 0x08), 0x26: (3, 0x0C), 0x2F: (3, 0x58), 0x37: (3, 0x59),
}
# COPIED: BN5's sprite copied into the moved copy's art (the Navis' shared art, src/emu/xnavi_recipe.h) at a number of a
# list BN6's room grows, from the range of the track that spawns the kind: (BN6's list, number, track)
IMAGE_COPIED = {
    # ShadowMan's and Colonel's image (BN5's sprite 3:0x00), at the end of the Navis' list 4 range (0xA0-0xB5)
    0x03: (4, 0xB5, 'navis'),
    # the Navis' buster's muzzle (BN5's family 0x16, the Dark Soul's: BN5's sprite 3:0x06, 903 bytes, none of BN6's the
    # same), at list 5's 0xA5, past the Navis' 0xA0-0xA4 (asked of the plan's owner with this change)
    0x0D: (5, 0xA5, 'navis'),
    # the Dark Soul's buster's arm (BN5's 0x080EBBA6 for its AI index: BN5's sprite 3:0x03, 8063 bytes, none of BN6's the
    # same), at list 5's 0xA6 (asked with this change: the Souls' branch copies the same bytes for its kinds 0x23 and 0x33 at
    # list 0's 0x4C; one place kept at the merge)
    0x23: (5, 0xA6, 'navis'),
    # WuNote's note (BN5 0x0810CFC4-0x0810CFC6: r4's low byte 0x30, its palette in the high one; BN5's sprite 4:0x21, none of
    # BN6's the same), at list 4's 0x6C of the families' range (issue #82's shared allocations: the families' copy of the
    # module merged into this one, its number kept)
    0x30: (4, 0x6C, 'families'),
    # the chips' muzzle flashes (BN5's chips' code: tools/xchiprecipe.py, FLASH_NUMBERS and its units' own numbers): the
    # swords' 0x0A and 0x32 the same sprite as 0x03 (3:0x00), the buster arm's 0x33 as 0x23 (3:0x03), at the Navis' places
    # (the Navis' places win for every kind both tracks fix); the rest, none of BN6's the same, at numbers of the chips' in
    # list 3 (downward from 0xFF, past their units' own; 3:0x02 the number a unit, the bombs', loads it at)
    0x0A: (4, 0xB5, 'chips'),
    0x32: (4, 0xB5, 'chips'),
    0x33: (5, 0xA6, 'chips'),
    0x04: (3, 0xF3, 'chips'),   # the bombs' (3:0x02, as 0x1A, 0x28 and 0x2A)
    0x1A: (3, 0xF3, 'chips'),
    0x28: (3, 0xF3, 'chips'),
    0x2A: (3, 0xF3, 'chips'),
    0x0B: (3, 0xEE, 'chips'),   # (3:0x1D, as 0x1F)
    0x1F: (3, 0xEE, 'chips'),
    0x2B: (3, 0xED, 'chips'),   # the buster arm's in a form (3:0x04)
    0x2C: (3, 0xEC, 'chips'),   # MrkCan's (3:0x1C, as 0x2D and 0x2E)
    0x2D: (3, 0xEC, 'chips'),
    0x2E: (3, 0xEC, 'chips'),
    0x34: (3, 0xEB, 'chips'),   # (3:0x33)
    0x35: (3, 0xEA, 'chips'),   # (3:0x38)
    0x36: (3, 0xE9, 'chips'),   # (3:0x39)
    # the Souls' (tools/xsoul.py: ShadowSoul's sword family's 0x03, 0x0A and 0x32, MegaMan's buster's flash in a Soul and
    # in Chaos Unison 0x23 and 0x33, at the places above), and ToadSoul's charged shot's 0x31 (BN5's sprite 3:0x1F, none of
    # BN6's the same) at list 0's 0x4E, the Souls' own list: the kind's bytes fixed in the one copy, its sprite copied and
    # listed by the Souls' install (src/emu/soulcode.c), list 0 grown by theirs alone (tools/xnavirecipe.py copies none for it)
    0x31: (0, 0x4E, 'souls'),
}
