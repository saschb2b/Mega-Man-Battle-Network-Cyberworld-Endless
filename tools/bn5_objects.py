"""bn5_objects.py: BN5 Team Colonel's engine objects that BN6 Cybeast Gregar (USA) runs otherwise, each moved into BN6
once, whole, for every track that spawns one (issue #82's plan for its tracks: the Navis, the virus families, the chips,
the Souls), and what each one's moved copy draws. Numbers only. The Souls' generator (tools/xsoul.py) reads it for the
kinds their code spawns and refuses a kind it fixes otherwise; the Navis' (tools/xnavirecipe.py, on their branch) installs
the module whole (docs/MULTIROM.md, Transplanted Souls; docs/ROM_DATA.md, The Souls' code in BN6's engine)."""

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
# TWIN: BN6 lists a sprite of the same bytes (each sprite compared whole): its list and number in BN6. (The Souls' kinds
# have none: the Navis' entries join this table at the merge.)
IMAGE_TWIN = {}
# COPIED: BN5's sprite copied into a track's art at a number of a list from the range of the track that spawns the kind:
# (BN6's list, number, track). Each sprite's bytes in one place (the coordinator's call, 2026-10-12): BN5's 3:0x00
# (ShadowMan's and Colonel's image; the Souls' kinds 0x03, 0x0A and 0x32, ShadowSoul's sword family's) at the end of the
# Navis' list 4 range, 0xB5; 3:0x03 (the Dark Soul's buster's arm; the Souls' 0x23 and 0x33, MegaMan's buster's flash in
# a Soul and in Chaos Unison) at list 5's 0xA6; 3:0x06 (the Navis' buster's muzzle) at list 5's 0xA5; 3:0x1F (ToadSoul's,
# 0x31) at list 0's 0x4E, the Souls' own. The Souls' branch copies the Navis' two as well until the merge (src/emu/
# soulcode.c lists a copy only where no install listed the same bytes there first)
IMAGE_COPIED = {
    0x03: (4, 0xB5, 'navis'),
    0x0A: (4, 0xB5, 'navis'),
    0x0D: (5, 0xA5, 'navis'),
    0x23: (5, 0xA6, 'navis'),
    0x31: (0, 0x4E, 'souls'),
    0x32: (4, 0xB5, 'navis'),
    0x33: (5, 0xA6, 'navis'),
}
