"""bn5_objects.py: BN5 Team Colonel's engine objects that BN6 Cybeast Gregar (USA) runs otherwise, each moved into BN6
once, whole, for every track that spawns one (issue #82's plan for its tracks: the Navis, the virus families, the chips),
and what each one's moved copy draws. Numbers only. tools/xnavirecipe.py builds the moved copies
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
}
