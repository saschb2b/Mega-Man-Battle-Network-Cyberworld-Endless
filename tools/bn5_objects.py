"""bn5_objects.py: BN5 Team Colonel's engine objects that BN6 Cybeast Gregar (USA) runs otherwise, each moved into BN6
once, whole, for every track that spawns one (issue #82's plan for its tracks: the Navis, the virus families, the chips),
and what each one's moved copy draws: the image object, and the sea's marker. Numbers only. tools/xnavirecipe.py builds the moved copies
(src/emu/xnavi_bn5.c, xnavi_objects_bn5; installed by src/emu/xnavi_objects.c before any Navi), checks this file
against both ROMs, and refuses a recipe that spawns a kind this file leaves unfixed; the other tracks' generators read it
for the kinds their code spawns (docs/MULTIROM.md, The engine's objects moved once; docs/ROM_DATA.md)."""

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
}
