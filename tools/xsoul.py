#!/usr/bin/env python3
"""xsoul.py [--check] [--report]: the code BN5's Team Colonel Souls run, moved into BN6's battle engine (issue #82, the
Souls track's S3-S5; docs/multirom/souls.md; docs/devtools/, The Souls' code), derived from the two ROMs.

What a Soul does in BN5 is its own code beside the engine's (docs/rom/souls.md, The Souls' code in BN6's engine): its
charged shot's power attack and attack family, the objects they spawn, its unison's image (BN5's phase 1 of the unison,
which BN6 replaced with a Cross's summon) and the routines those call that BN6 has no twin of. This walks each from its
roots (tools/xrecipe_thumb.py), pairs every call out with BN6's twin (tools/xrecipe_pairs.py and tools/bn5_twins.py),
and writes src/emu/soulcode_bn5.c: the blocks to copy and where they go in the Souls' room, every literal word of their
pools and tables and every call out of a block with BN6's value in its place, the immediates BN6 numbers otherwise (the
families' CurActions from 0xC0, the effects, sprites and songs BN5's ROM lends, the collision data's offsets), the object
types they spawn through the second-level dispatch, and the CRC32 of each range copied. Numbers only.

  python3 tools/xsoul.py            write src/emu/soulcode_bn5.c
  python3 tools/xsoul.py --check    write nothing: fail where src/emu/soulcode_bn5.c is not what it would write
  python3 tools/xsoul.py --report   also print each unit's routines, calls, numbers and what was decided where

Local only: both ROMs (found by their SHA-1s in CYBERWORLD_ROM_DIR or ~/.cache/mmbn-ref/roms); the symbol map of
tools/bn6f_match.py in .build/symbols/match where it was made. Every decision below names where it was read."""
import argparse
import hashlib
import os
import re
import sys
import zlib

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import bn5_objects  # noqa: E402
import bn5_twins  # noqa: E402
import xrecipe  # noqa: E402
import xrecipe_family  # noqa: E402
import xrecipe_thumb as T  # noqa: E402
from xrecipe_pairs import Pairs  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT_C = os.path.join(ROOT, 'src', 'emu', 'soulcode_bn5.c')
# The tracks installed before the Souls' code (src/emu/xinstall.c: the moved objects and the Navis, the chips' code, the
# families), whose copies the Souls' code runs where a type it spawns is theirs (tools/bn5_twins.py's TYPES: one copy of
# each moved type, the first track's, every later one spawning it and calling into it there) and whose effects it shares
# (one BN6 number a BN5 effect, issue #82's allocations' amendment), each read from the C its generator wrote
BEFORE = (('navis', os.path.join(ROOT, 'src', 'emu', 'xnavi_bn5.c')), ('chips', os.path.join(ROOT, 'src', 'emu', 'xchips_bn5.c')),
          ('families', os.path.join(ROOT, 'src', 'emu', 'xvirus_bn5.c')))
MATCH = os.path.join(ROOT, '.build', 'symbols', 'match')
WORK = os.path.join(ROOT, '.build', 'xrecipe')

# ---- the Souls' room (src/emu/soul_room.h) ----

CODE_ROOM = 0x3000          # SOULS_CODE's bytes
ART_ROOM = 0xE000           # SOULS_ART's bytes before the faces (soul_room.h's SOULS_FACE_TILES); past them, the
ART_ROOM2 = 0xF000          # ... granted region FREE_SOULS_ART (free_space.h), its bytes before ColonelSoul's button's art
                            # (soul_room.h's SOULS_COLONEL_ART, its last 0x1000)
BLOCK_STEP = 0x10           # each block begins at the next multiple of this past the last, its distance from a word kept
ART_STEP = 0x100            # each sprite at the next multiple of this

# ---- what is moved ----

# The units, each a Soul's (its BN5 number, Team Colonel's 7-12) or the unison's, with its roots in BN5: the routines its
# power attacks name (BN5's table 0x0800F368 by the ids its loadout row 0x0801CA18 names), the attack families those
# return (BN5's table 0x080EB514 by CurAction - 0x10), the routines its enter table names (0x0800F030), its passives'
# (the ready table 0x08011BA4 as he unites, the collision table 0x08011CCC as a form's state is set again: each not a
# mov pc,lr; src/emu/soulpassive.c routes BN6's twins of the tables to them; ColonelSoul's, 0x08011C3C, readies his
# obstacle strike: BN5's RAM words by side and his side's bit 0x10 or 0x20 of BattleState +0x5C, kept in BN6_COLONEL_SIDES
# as a Navi Colonel's is, STUB_CALLS, which BN6's obstacles strike by, src/emu/xstrike.c), and the unison's phase 1
# (0x08011F8C's entry 1, the Soul's image)
UNITS = (
    ('unite', 0, (0x08012008,)),
    ('colonel', 7, (0x0800F7DC, 0x0800F7D0, 0x080EED92, 0x080236BC, 0x080236E8, 0x08011C3C, 0x080CABCA)),
    ('shadow', 8, (0x0800F818, 0x0800FBE6, 0x080EBDC6, 0x080EEADC, 0x08011C78, 0x08011D2A)),
    ('number', 9, (0x0800F66A, 0x080EE864, 0x0800F074, 0x0800F0D6)),
    ('tomahawk', 10, (0x0800F864, 0x080EEF6C, 0x08011C90)),
    ('knight', 11, (0x0800F79E, 0x080EEBDE)),
    ('toad', 12, (0x0800F9E6, 0x080EF68A, 0x08011CAA)),
)

# The families moved: BN5's CurAction, BN6's in the grown table (xattack_grow.h's XATTACK_SOULS on), its entry in BN5
FAMILIES = ((0x43, 0xC0, 0x080EED92), (0x3A, 0xC1, 0x080EE864), (0x40, 0xC2, 0x080EEBDE), (0x50, 0xC3, 0x080EF68A),
            (0x3D, 0xC4, 0x080EEADC), (0x45, 0xC5, 0x080EEF6C), (0x13, 0xC6, 0x080EBDC6))

# The power attacks moved: BN5's id, its entry, and the Soul's role for it (its loadout row's byte); BN6's id is
# soul_room.h's SOUL_POWER_FIRST on in this order
POWERS = ((0x14, 0x0800F7DC, 'ColonelSoul\'s charged shot'), (0x13, 0x0800F7D0, 'ColonelSoul\'s chip shot'),
          (0x17, 0x0800F818, 'ShadowSoul\'s charged shot'), (0x45, 0x0800FBE6, 'ShadowSoul\'s B+Left'),
          (0x07, 0x0800F66A, 'NumberSoul\'s charged shot'), (0x1A, 0x0800F864, 'TomahawkSoul\'s charged shot'),
          (0x11, 0x0800F79E, 'KnightSoul\'s charged shot'), (0x2D, 0x0800F9E6, 'ToadSoul\'s charged shot'))

# BN5's routines a moved routine calls that BN6 answers otherwise, aimed at the Souls' stubs (soul_room.h: SOUL_STUB_*),
# which the engine's C answers (src/emu/soulcode.c): BN5's pending Soul (0x080123FA: r0 the Soul, r1 its chaos, r2
# ColonelSoul's chip, r3 its turns, where BN6's pending record holds the proxy form), and the panel highlights of BN5's
# fourth kind's 0x83 (0x080EA08C, 0x080EA0BA, over BN5's panel highlight tables 0x0203C51C-0x0203C59F and 0x02034B80,
# which BN6's engine lacks: Gregar searched for their reader 0x0802EA60 and their set-up 0x0802E8F4, byte for byte but
# for the literal, found none): no object, the panel left plain
STUB_CALLS = {0x080123FA: 'SOUL_STUB_PENDING', 0x080EA08C: 'SOUL_STUB_NONE', 0x080EA0BA: 'SOUL_STUB_NONE',
              # (a counter's attack begun, ShadowSoul's B+Left stance hit: BN6's twin 0x080105F2 takes the same steps, its two
              # families numbered 0x47 and 0x48 where BN5's 0x2C and 0x2D, but leaves the family's first frame to the next
              # frame where BN5's runs it at once, 0x0800E39A: the Souls' Thumb calls the twin, then the family once)
              0x0800E338: 'SOUL_STUB_COUNTER',
              # (Colonel's side's flag of BattleState +0x5C, 0x10 or 0x20 by side through 0x0800131E: BN6's own code sets and
              # reads those bits for its own, 0x080E0684, so ColonelSoul's is kept in the Navis' field a Navi Colonel's spawn
              # sets, BN6_COLONEL_SIDES, which the obstacle strike reads: src/emu/xstrike.c)
              0x080CAD06: 'SOUL_STUB_STRIKE'}
# BN5's battle NaviStats byte getters (0x08010DEE by object, 0x08010D64 by side), read with r1 0x2C, MegaMan's form: BN5's
# answer is the Soul's number (7-12), BN6's the proxy's 8; where a moved routine reads it so the Souls' getter answers
FORM_GETTERS = {0x08010DEE: 'SOUL_STUB_FORM', 0x08010D64: 'SOUL_STUB_FORM_SIDE'}
FORM_FIELD = 0x2C

# The Souls' own pairs, where the symbol map's is wrong or none (each body read in both games): BN5's chip writer's second
# entry (push; movs r4,#0; b to their common part) and BN6's, beside the first's pair 0x0800FE6A -> 0x080126E4
PAIRS = {0x0800FE70: (0x080126EA, 'the chip writer\'s second entry: push {r4,r6,r7,lr}; movs r4,#0; b to the common part, in both'),
         0x080258E8: (0x08029CD4, 'a pick\'s slot drawn on the Custom screen: the same steps, each game\'s own tiles and map place')}
# The Souls' audits of their calls' differing twins (as tools/bn5_twins.py's AUDITS for the families')
AUDITS = {
    (0x0800FE56, 0x08012642): "a charged shot's damage, base + its step x (his buster's Attack + 1): BN6's adds his Navi's and his form's "
                              "Attack too (0x0801265A), capped at 5, and 1 while he is dark; for MegaMan in a Soul, the form's 0 "
                              "(src/emu/soulpower.c's hook on BN6_FORM_BUSTER_BONUS's read), the same to Attack 4",
    (0x0800FE70, 0x080126EA): "a chip's numbers written for its attack (ColonelSoul's chip shot): BN6 ORs its chip record's icon element's "
                              "hit flag in (0x080129E4): right for a BN6 chip id, which the chip MegaMan picked is",
    (0x0800E510, 0x080107C0): "the collision update's fourth argument for the passives' (ShadowSoul's FloatShoe): 3 where the battle's "
                              "effects have flag 8, else 0 in BN6; BN5 0 only for an object whose AI data +0 is 2 and +1 not 0x17 as "
                              "well, which MegaMan, the one object a Soul's passive runs on, always is (+0 2, +1 his AI index 0): the "
                              "same for him",
    (0x080258E8, 0x08029CD4): "a pick's slot drawn on the Custom screen's BG (r0 its place, r1 0, r2 0: ColonelSoul's button's "
                              "phase 1): the same steps, the tiles each game's own (BN5's from 0x47, BN6's from 0x89) at its BG map's "
                              "place for the slot: BN6's layout of its own screen",
    (0x08024004, 0x080281D4): "a chip's icon copied into the picks column's tiles (r0 its chip, -1 none; r1 its place: ColonelSoul's "
                              "button's phase 1 blinking it): the same steps on each game's tiles for the column (BN5 0x06009C20, BN6 "
                              "0x0600A4A0, BN6_CUSTOM_PICK_ICONS) and their icon table, BN6's own",
    (0x08024A2A, 0x08028E32): "the lit rules run again as the button's phase ends: BN5's five (the chips', OK's, UNITE's, ColonelSoul's "
                              "button, MeddySoul's), BN6's four (the same three, then DustCross's button, the place src/emu/"
                              "soulcolonel.c answers with BN5's ColonelSoul rule; MeddySoul is Team ProtoMan's)",
    (0x08015140, 0x08018810): "an owner's arm point, by its NameID's sprite (the arm flash's set-up, a kind whose +4 is set): BN6 answers "
                              "(0, 7) for its own NameIDs 0xCD-0xFF and else takes the same steps, its calls their twins; the owner of "
                              "every flash the Souls spawn is MegaMan (0x1A0, a form's 0x1AB on), his sprite's arm row read from the rows "
                              "soullook.c grows with the Souls' BN5 rows",
}

# BN5's RAM its code names, and BN6's in its place, each read from both games' code
RAM = {
    0x02034000: (0x0203FF60, 'BN5\'s per-side words of ColonelSoul\'s obstacle strike (0x080CACE0, 8 bytes a side): the RAM plan\'s '
                             'RAM_COLONEL (free_space.h), which BN6 leaves alone'),
}

# BN5's spawners by kind; BN5's and BN6's types tables
SPAWNERS = {0x08003140: 1, 0x0800315C: 1, 0x080031E8: 1, 0x08003178: 3, 0x08003194: 3, 0x080031B0: 3, 0x080031CC: 4}
DISPATCH = {0x08003140: 'XV_SPAWN_1', 0x0800315C: 'XV_SPAWN_1_LAST', 0x080031E8: 'XV_SPAWN_1_LAST', 0x08003178: 'XV_SPAWN_3',
            0x08003194: 'XV_SPAWN_3_LAST', 0x080031B0: 'XV_SPAWN_3_FIRST', 0x080031CC: 'XV_SPAWN_4'}
TYPES5 = {1: (0x08003958, 95), 3: (0x08003B80, 204), 4: (0x08003F60, 145)}
TYPES6 = {1: (0x08003C9C, 95), 3: (0x08003EC4, 213), 4: (0x080042C8, 146)}

# The calls whose arguments are numbers BN6 may number otherwise: BN5's routine, the register, what it is
SOUND = 0x08000598          # PlaySoundEffect: r0 the song
EFFECT = 0x080DFFD2         # an effect object: r4's low byte the effect
SPRITE_LOAD = 0x08002650    # sprite_load: r1 the list's offset (list x 4), r2 the number
# ... and an object's sprite and collision set up from two packed words (r0: its sprite's list x 4 << 8 | its number, its
# animation << 16; r1 its collision's: BN6's twin 0x08011504 the same; the obstacle strike's soldier's, 0x080CA94E), or its
# sprite from one (r0 the same, a palette's << 24: BN6's twin 0x0801BE2A)
SPRITE_SETUPS = (0x0800F198, 0x08018926)

# Sprite lists, effects, songs: the numbers the Souls take, each from the tracks' binding allocations (issue #82):
# sprite list 0 the Souls' alone (its words as BN6 reads them through it to 0x44, the Souls' full sprites 0x45-0x4A, what
# their code draws from soul_room.h's SOUL_LIST0_ART_FIRST up), effects 0xC0-0xCF, and one BN6 song slot a BN5 sound
ART_LIST = 0                # the list the Souls' sprites BN6 has none of are listed in, ...
ART_FIRST, ART_LAST = 0x4B, 0x5F   # ... at these numbers (soul_room.h's SOUL_LIST0_ART_FIRST, SOULS_LIST0_ENTRIES - 1)
# ... the sprites copied so far in the order they took their numbers (BN5's list and number), placed first so that a
# number once given stays (tools/bn5_objects.py fixes the flash kinds' by them, and the tracks merge on them); a sprite a
# later walk meets is numbered after them
# (None: a number left empty, once BN5's 3:0x03 and 3:0x00, which the image object's kinds now draw at the Navis' numbers:
# tools/bn5_objects.py's IMAGE_COPIED)
ART_ORDER = ((4, 0x1F), None, None, (3, 0x1F), (3, 0x14), (4, 0x01), (3, 0x0A), (3, 0x15), (3, 0x05), (5, 0x03),
             (5, 0x0D), (5, 0x02), (4, 0x20), (5, 0x1C))
# Sprites BN6's own code draws for a Soul where BN5 draws another, copied with the Souls' though no moved routine loads them
# (src/emu/soulchaos.c hooks the load): Chaos Unison's charge glow, BN5's list 5's 0x1C where his charge glow is its 8 (BN5
# 0x080E0F9C, the fourth kind's 8 in Chaos Unison; BN6's list 5's 0x1C another picture)
EXTRA_ART = ((5, 0x1C),)
# ... and the effects so (their ids from 0xC0 in this order: ColonelSoul's ScreenDivide's three, ShadowSoul's slash,
# TomahawkSoul's swing, the three BN6 redrew, the obstacle strike's shot's hit), a later walk's numbered after them; an
# effect another track copied first is that track's number (BEFORE: the Navis' ScreenDivide, slash, swing, 0x14 and 0x02),
# so the Souls' own are 0x03 at 0xC0 and 0x20 at 0xC1 (tools/bn5_twins.py's SHARED_ROWS names that one in the chips' copy)
EFFECT_ORDER = (0x48, 0x49, 0x4A, 0x17, 0x45, 0x03, 0x14, 0x02, 0x20)
EFFECT_FIRST, EFFECT_LAST = 0xC0, 0xCF
# BN5's songs a Soul plays that Gregar has otherwise (xrecipe.py's song_sig: BN6's own 0x70 and 0x117 are none of them, its
# 0x115 another sound), each at the BN6 slot the allocations' amendment gives that sound whatever track copies it (the
# first install copies it, the others find it there: xsong.c's xsong_install_at); 0x129 (the Dark Soul's) at 0x5D and 0x16B (the
# chaos pick's) at 0x5C are Chaos Unison's, the Souls track's S6
SONG_SLOTS = {0x70: 0x5F, 0x117: 0x5E, 0x115: 0x5A, 0x129: 0x5D, 0x16B: 0x5C}

# Effects a moved routine makes of a base and a number: the instruction adding the base, and how many ids from it its
# callers use (ColonelSoul's family 0x43: 0x48 + its sub-family, AttackVars +3, 0-2 as BN5's power attacks set it)
EFFECT_BASES = {0x080EEE18: (0x48, 3)}
# Raw tables of effect ids a moved routine reads, each entry's address and the sub-family it serves (BN5's sword family's
# effect by sub-family, 0x080EC110: ShadowSoul's charged shot, sub-family 2, its slash 0x17; the others are the sword
# chips' own, whose entries BN5 and BN6 number alike, each the same sprite, animation and palette by its record, and
# which no Soul's power attack runs through this copy)
EFFECT_TABLES = {0x080EC112: 'ShadowSoul\'s charged shot (sub-family 2)',
                 0x080C6C52: 'the obstacle strike\'s shot (its parameter 12, 0x080CAB40: the third kind\'s 0x12\'s table 0x080C6C08, 6 '
                             'bytes an entry, +2 its effect)'}
# ... the effect calls that read an entry of one far from them, by the call
EFFECT_SITES = {0x080C6DD6: 0x080C6C52}
# Raw tables of sounds a moved routine plays (a song number read from data, 0 none), by the call that plays it: each
# entry the Souls' code reaches, its address (the strike's shot's table's +4 for the parameter it is spawned with)
SOUND_TABLES = {0x080C6DE2: ((0x080C6C54, 'the obstacle strike\'s shot (its parameter 12)'),)}
# The arm's flash, BN5's first kind's type 5 (the image object): moved, BN5's own object, as the allocations' amendment
# has every track run it (tools/bn5_twins.py's TYPES): its spawner and its kind's lookup walked (UNTWINNED) so that the
# spawn goes through the second-level dispatch, its kinds table (5 bytes a kind: list x 4, number, animation, height,
# whether it follows its owner's offset; 0x3A kinds) copied with the code. Its kinds the Souls' code names (ShadowSoul's
# sword family's 0x03, 0x0A and 0x32, ToadSoul's 0x31) and MegaMan's buster names in a Soul (0x23, 0x33 in chaos: BN5's
# 0x080EC9xx, soullook.c) keep BN5's numbers, their sprites given BN6's numbers; the other kinds are dead in this copy (no
# code of the Souls' spawns them), the one copy every track shares fixing up each kind
FLASH_KINDS5 = bn5_objects.IMAGE['kinds']
FLASH_KIND_COUNT = bn5_objects.IMAGE['count']
FLASH_KINDS = (0x23, 0x33, 0x03, 0x0A, 0x32, 0x31)   # (each fixed in tools/bn5_objects.py's per-kind table, the Souls' entries)
# Raw tables of the sprites a moved routine loads by a parameter: BN5's table (its entry's size; list x 4 and number first),
# the parameters the moved code gives (NumberSoul's enter 0x0800F074 its companion 1; no moved code spawns it with 0), whose
SPRITE_TABLES = {0x080C33A8: (2, (1,), 'NumberSoul\'s companion'),
                 FLASH_KINDS5: (5, FLASH_KINDS, 'the arm\'s flash kind')}
# Routines BN6 has the twin of by their bytes whose literals name data of their own that differs, or whose spawn must run
# BN5's object, walked and moved instead
UNTWINNED = {0x080C33CC: 'NumberSoul\'s companion\'s set-up (the object NumberSoul\'s enter spawns, parameter 1): BN6\'s twin '
                         '0x080C5968 loads its sprite from its own table 0x080C5944, list 0 0x00 (MegaMan\'s own) for that '
                         'parameter where BN5\'s 0x080C33A8 names list 4 0x01',
             0x080B9CAE: 'the arm flash\'s spawner (BN6\'s twin 0x080BA6A0 spawns BN6\'s type 5, whose kinds table is BN6\'s own: '
                         'moved, its spawn goes through the dispatch to BN5\'s object)',
             0x080B9CE0: 'a flash kind\'s sprite, its list and number (BN6\'s twin 0x080BA6D2 reads BN6\'s kinds table)',
             0x080C6E0E: 'the obstacle strike\'s shot\'s spawner (BN6\'s twin 0x080C834A spawns BN6\'s type 0x12, whose numbers by '
                         'its parameter are its own table\'s: moved, its spawn goes through the dispatch to BN5\'s object)'}
# BN5's routines the Souls' C reaches in another track's copy, its routes (src/emu/soulcode.c's soulcode_moved): the image
# object's spawner, which MegaMan's buster's flash in a Soul is sent to (src/emu/soullook.c)
ROUTED = (0x080B9CAE,)
# Data a moved routine reads near it that the walk sees only through a pointer table: BN5's ranges, copied with the code
DATA_RANGES = ((0x080EC05C, 0x080EC120), (0x080125B8, 0x080125D0), (0x080C33A8, 0x080C33AC), (0x080C6C08, 0x080C6C80),
               (FLASH_KINDS5, FLASH_KINDS5 + 5 * FLASH_KIND_COUNT))
# Tables a block holds as raw bytes between its code and its pool that no moved routine reads (another's, copied with the
# bytes between): each never read through the copy
DEAD_TABLES = {(0x08011CCC, 0x08011D00): "BN5's passives by form as a form's state is set again (BN5_SOUL_REAPPLY), read by "
                                         "0x08011144 alone, which nothing moved runs: it lies between ColonelSoul's ready "
                                         "(0x08011C48) and its pool (0x08011D48)"}
# BN5's AI data fields a Soul keeps its state in, which BN6's AI data holds otherwise (docs/rom/souls.md, The Souls' code):
# where a moved routine reads one through its AI data (ldr rX,[r5,#0x58]), the Souls' C answers from BN6_SOULS
SOUL_FIELDS = {0x0F: 'BN6_SOUL_TURNS', 0x12: 'BN6_SOUL_CHAOS', 0x1F: 'BN6_SOUL_PHASE', 0x32: 'BN6_SOUL_CHIP', 0x6C: 'BN6_SOUL_LEVEL'}

LISTS5, LISTS6 = 0x0803272C, 0x08031CC4
EFFECTS5, EFFECTS6, NEFFECTS = 0x080DFD7C, 0x080E16D4, 108


def rounded(a, step):
    return (a + step - 1) // step * step


def anim_life(b, anim):
    """an animation's frames' durations in a sprite's bytes (as xrecipe.sprite gives them, xrecipe.sprite_len's layout:
    20 bytes a frame, its duration at +16, +18's 0x80 its last), None where they cannot be read"""
    g32 = lambda o: int.from_bytes(b[o:o + 4], 'little')
    if not b or len(b) < 8 or anim >= g32(4) // 4:
        return None
    f, out = g32(4 + 4 * anim), []
    for _ in range(256):
        if 4 + f + 20 > len(b):
            return None
        out.append(b[4 + f + 16])
        if b[4 + f + 18] & 0x80:
            return tuple(out)
        f += 20
    return None


def before_tracks():
    """what the tracks installed before the Souls' code copied (BEFORE): their blocks, each (BN5's lo, hi, BN6's place as C
    (its base's name and offset), its track), their effects, BN5's id -> BN6's, and their sprites, BN5's (list, number) ->
    BN6's (the first track's where two name one)"""
    blocks, effects, sprites = [], {}, {}
    for track, path in BEFORE:
        with open(path, encoding='utf-8') as f:
            text = f.read()
        for lo, hi, base, off in re.findall(r'\{ 0x([0-9A-F]{8}), 0x([0-9A-F]{8}), (XN_OBJECTS|XN_CODE|XC_CODE|XV_CODE) \+ 0x([0-9A-F]+), '
                                            r'0x[0-9A-F]{8}u \}', text):
            blocks.append((int(lo, 16), int(hi, 16), base, int(off, 16), track))
        for body in re.findall(r'static const XvId \w+_effects\[\] = \{(.*?)\};', text, re.S):
            for e5, e6 in re.findall(r'\{ 0x([0-9A-F]{2}), 0x([0-9A-F]{2}) \}', body):
                effects.setdefault(int(e5, 16), int(e6, 16))
        for body in re.findall(r'static const XvSprite \w+_sprites\[\] = \{(.*?)\};', text, re.S):
            for l5, i5, l6, i6 in re.findall(r'\{ (\d), 0x([0-9A-F]{2}), (\d), 0x([0-9A-F]{2}), ', body):
                sprites.setdefault((int(l5), int(i5, 16)), (int(l6), int(i6, 16)))
    return blocks, effects, sprites


def shared_blocks(b5, blocks):
    """the blocks of another track's copy of a type the Souls' code spawns as theirs: each block of a track holding the
    routine of a type tools/bn5_twins.py's TYPES gives that track (the image object's module, the moved objects', among
    them); the Souls' code calls into one there, and copies none of it"""
    out = []
    for (kind, t), (dec, _, track, _) in sorted(bn5_twins.TYPES.items()):
        if dec != 'moved' or track == 'souls' or kind not in TYPES5:
            continue
        r5 = b5.u32(TYPES5[kind][0] + 4 * t) & ~1
        for blk in blocks:
            if blk[4] == track and blk[0] <= r5 < blk[1] and blk not in out:
                out.append(blk)
                break
    return out


class Deriv:
    """The units walked and every number of their move derived; `decisions` what is left open (none for a recipe
    written)"""

    def __init__(self, b5, b6, pairs):
        self.b5, self.b6, self.pairs = b5, b6, pairs
        self.walks, self.unit_of = {}, {}
        self.calls = {}         # BL site -> (BN5 target, BN6 value as C, why)
        self.words = {}         # word address -> (BN5 value, BN6 value as C, why)
        self.imms = []          # dict(at, form, rd, rn, was, now, why)
        self.types = {}         # (kind, BN5 type) -> BN6's type, or None: moved (its routine walked)
        self.tables = {}
        self.decisions, self.notes = [], []
        self.blocks = []
        self.sprites, self.effects, self.sprite_at = [], [], {}
        self.number_of = {}     # (list, index) -> BN6's (list, number)
        self.placed_by = {}     # (list, index) -> (BN6's list, number, track): a sprite another track lists (its copy here till the merge)
        self.effect_of = {}     # BN5 effect -> BN6 effect
        self.next_art = ART_FIRST
        self.next_effect = EFFECT_FIRST
        self.runs, self.bytes_, self.fields, self.sprite_tables = set(), [], [], set()
        self.deferred, self.late = [], False   # (the effect spawns of the effects BN6 redrew, numbered after the rest)
        self.songs = {}         # BN5's song -> BN6's slot (soulpower.c copies it there)
        self.art = self.art2 = 0
        self.six = {}
        blocks, self.others_effects, self.others_sprites = before_tracks()
        self.shared = shared_blocks(b5, blocks)
        self.routes = {}        # BN5's routine or data -> its place in another track's copy, as C: what the Souls' code runs there
        self.shared_effects = set()   # BN6's ids of the effects another track's install made, the Souls' too
        self.reserved = {}      # BN5's sprite -> its list 0 number (ART_ORDER: the numbers given once stay)

    def shared_at(self, a):
        """the block of another track's copy (shared_blocks) that holds BN5's address `a`, or None"""
        return next((b for b in self.shared if b[0] <= (a & ~1) < b[1]), None)

    def shared_expr(self, a):
        """BN5's address `a` (its Thumb bit kept) in another track's copy, as C"""
        lo, hi, base, off, track = self.shared_at(a)
        return f'{base} + 0x{off + (a - lo):X}'

    def decide(self, what):
        if what not in self.decisions:
            self.decisions.append(what)

    def twin(self, a):
        if (a & ~1) in UNTWINNED:
            return None
        if (a & ~1) in PAIRS:
            to, why = PAIRS[a & ~1]
            return to, 'soul:' + why[:40], 'DIFF', None
        tw = self.pairs.twin(a & ~1)
        if tw[0] and (self.pairs.audit(a & ~1, tw[0]) or ('',))[0] == 'copy':
            return None   # (its audit serves BN5's own body, every track's: copied, as the families' and the chips' code do)
        return tw if tw[0] else None

    # ---- the walk ----

    def find_tables(self, w):
        out = {}
        for lit, v in w.lits.items():
            if v is None or v & 3 or not self.b5.has(v, 4) or self.shared_at(v):
                continue
            if not (min(w.insns) - 0x40 <= v <= max(w.insns) + 0x400):
                continue
            ents = []
            for k in range(48):
                x = self.b5.u32(v + 4 * k)
                # (an entry far from the table only where it is a routine BN6 has the twin of: an object's common end)
                if not T.is_code_ptr(x, self.b5) or (abs((x & ~1) - v) > 0x4000 and not self.twin(x)):
                    break
                ents.append(x)
            if ents:
                out[v] = ents
        return out

    def walk(self, name, roots):
        todo = [(r, True) for r in roots]
        while todo:
            a, root = todo.pop()
            a &= ~1
            if a in self.walks or a in STUB_CALLS or self.shared_at(a):
                continue   # (another track's copy runs it: shared_blocks)
            if not root and self.twin(a):
                continue
            w = T.walk(self.b5, a)
            if w.bad:
                self.decide(f'{name}: {a:08X} walks into {w.bad}')
            if w.indirect:
                self.decide(f'{name}: {a:08X} jumps indirectly at {", ".join(f"{x:08X}" for x in w.indirect)}')
            self.walks[a] = w
            self.unit_of[a] = name
            w.tables = self.find_tables(w)
            for tab, ents in w.tables.items():
                self.tables[tab] = ents
                for x in ents:
                    todo.append((x, False))
            for lit, v in w.lits.items():
                if v is not None and v not in w.tables and T.is_code_ptr(v, self.b5):
                    todo.append((v, False))
            for site, t in w.bls.items():
                if t in SPAWNERS:
                    self.spawn(name, w, site, t, todo)
                elif t not in STUB_CALLS and t not in FORM_GETTERS:
                    todo.append((t, False))

    def spawn(self, name, w, site, spawner, todo):
        kind = SPAWNERS[spawner]
        v, chain = T.back_value(w, site, 0)
        if v is None:
            self.decide(f'{name}: {site:08X} spawns a type made otherwise ({chain})')
            return
        tab5, _ = TYPES5[kind]
        tab6, n6 = TYPES6[kind]
        r5 = self.b5.u32(tab5 + 4 * v)
        r6 = self.b6.u32(tab6 + 4 * v) if v < n6 else 0
        # (moved unless proven the same in BN6, its sprites by their bytes too: one decision per type, recorded in
        # tools/bn5_twins.py's TYPES; a type of BN6's same body is a decision until its art is proven and recorded)
        same = T.is_code_ptr(r6, self.b6) and self.pairs.deep_same(r5 & ~1, r6 & ~1)
        dec = bn5_twins.TYPES.get((kind, v), ('', None, None, None))
        if dec[0] != 'moved':
            self.decide(f'{site:08X} spawns BN5\'s type {kind}/{v:#x}{" (BN6\'s of the same body)" if same else ""}: '
                        f'not moved in tools/bn5_twins.py\'s TYPES')
        elif dec[2] != 'souls':
            # (another track's copy, installed before the Souls' code: spawned through the dispatch as theirs, XV_TYPE_SHARED)
            if not self.shared_at(r5):
                self.decide(f'{site:08X} spawns BN5\'s type {kind}/{v:#x}, the {dec[2]}\' copy (TYPES), which no block of theirs holds')
            self.types[(kind, v)] = 'shared'
            return
        self.types[(kind, v)] = None
        todo.append((r5, True))

    # ---- calls out ----

    def resolve_calls(self):
        for a, w in self.walks.items():
            for site, t in w.bls.items():
                if t in SPAWNERS:
                    self.calls[site] = (t, DISPATCH[t], 'the second-level dispatch\'s spawner')
                elif t in STUB_CALLS:
                    self.calls[site] = (t, STUB_CALLS[t], 'answered by the Souls\' C')
                elif t in FORM_GETTERS and T.back_value(w, site, 1)[0] == FORM_FIELD:
                    self.calls[site] = (t, FORM_GETTERS[t], 'MegaMan\'s form read: the Soul\'s number, as BN5 keeps it')
                elif self.shared_at(t):
                    self.calls[site] = (t, self.shared_expr(t | 1), f'the {self.shared_at(t)[4]}\' copy of a type\'s module (TYPES)')
                    self.routes[t & ~1] = self.shared_expr(t & ~1)
                elif t in self.walks:
                    self.calls[site] = (t, None, 'moved')
                else:
                    tw = self.twin(t)
                    if not tw:
                        self.decide(f'{site:08X} calls {t:08X}, which has no twin in BN6 and was not walked')
                        continue
                    a6, how, cls, diff = tw
                    why = f'{how}, {cls}'
                    if cls == 'DIFF':
                        audit = self.pairs.audit(t, a6)
                        if (t, a6) in AUDITS:
                            why += ' (the Souls\' audit)'
                        elif audit and audit[0] == 'ok':
                            why += ' (audited)'
                        else:
                            self.decide(f'{site:08X} calls {t:08X}, whose twin {a6:08X} differs unaudited')
                    self.calls[site] = (t, f'0x{a6:08X}u', why)

    # ---- words: literal pools and jump tables ----

    def resolve_words(self):
        for a, w in self.walks.items():
            for lit, v in w.lits.items():
                self.word(lit, v, w)
        for tab, ents in self.tables.items():
            for k, x in enumerate(ents):
                at = tab + 4 * k
                if (x & ~1) in self.walks:
                    self.words[at] = (x, None, 'a jump table\'s entry, moved')
                else:
                    tw = self.twin(x)
                    if tw:
                        self.words[at] = (x, f'0x{tw[0] | 1:08X}u', f'a jump table\'s entry, BN6\'s twin ({tw[1]})')
                    else:
                        self.decide(f'{at:08X}: a jump table\'s entry {x:08X} neither moved nor twinned')

    def word(self, lit, v, w):
        if v is None:
            return
        if 0x08000000 <= v < 0x0A000000 and self.shared_at(v):
            self.words[lit] = (v, self.shared_expr(v), f'the {self.shared_at(v)[4]}\' copy of a type\'s module (TYPES)')
            self.routes[v & ~1] = self.shared_expr(v & ~1)
            return
        if v in self.tables or (T.is_code_ptr(v, self.b5) and (v & ~1) in self.walks):
            self.words[lit] = (v, None, 'moved with the code')
            return
        if T.is_code_ptr(v, self.b5):
            tw = self.twin(v)
            if tw:
                self.words[lit] = (v, f'0x{tw[0] | 1:08X}u', f'a routine of BN6\'s ({tw[1]})')
            else:
                self.decide(f'{lit:08X}: a code pointer {v:08X} neither moved nor twinned')
            return
        if 0x02000000 <= v < 0x04000000:
            if v in RAM:
                self.words[lit] = (v, f'0x{RAM[v][0]:08X}u', RAM[v][1])
            else:
                use = xrecipe_family.literal_use(w, lit)
                if use == 'number':
                    self.words[lit] = (v, f'0x{v:08X}u', 'a number')
                else:
                    self.decide(f'{lit:08X}: BN5\'s RAM {v:08X}, no twin known')
            return
        if 0x04000000 <= v < 0x08000000:
            self.words[lit] = (v, f'0x{v:08X}u', 'I/O or video, the same')
            return
        if 0x08000000 <= v < 0x0A000000:
            inside = any(lo <= v < hi for lo, hi in self.blocks)
            if inside:
                self.words[lit] = (v, None, 'data moved with the code')
                return
            for lo, n, to, why in bn5_twins.DATA_TWINS:
                if lo <= v < lo + n:
                    self.words[lit] = (v, f'0x{to + v - lo:08X}u', why)
                    return
            self.decide(f'{lit:08X}: BN5\'s ROM {v:08X}, no twin known')
            return
        self.words[lit] = (v, f'0x{v:08X}u', 'a number')

    # ---- blocks ----

    def make_blocks(self):
        cov = set()
        for a, w in self.walks.items():
            for x, i in w.insns.items():
                cov.update(range(x, x + i.size))
            for lit in w.lits:
                cov.update(range(lit, lit + 4))
        for tab, ents in self.tables.items():
            cov.update(range(tab, tab + 4 * len(ents)))
        # (raw data a routine names by address near it: a table of numbers its code indexes)
        for a, w in self.walks.items():
            for lit, v in w.lits.items():
                if v is None or v & 1 or not self.b5.has(v) or v in self.tables or not (min(w.insns) - 0x40 <= v <= max(w.insns) + 0x200):
                    continue
                if self.shared_at(v):
                    continue
                if any(lo <= v < lo + n for lo, n, _, _ in bn5_twins.DATA_TWINS):
                    continue
                # (to the next routine's push {..., lr}, 0x40 bytes at most)
                k = 0
                while k < 0x40 and v + k not in cov and not (k and not (v + k) & 1 and self.b5.u16(v + k) & 0xFF00 == 0xB500):
                    cov.add(v + k)
                    k += 1
        for lo, hi in self.own_ranges():
            cov.update(range(lo, hi))
        out, run = [], None
        for x in sorted(cov):
            if run and x <= run[1] + 0x20:
                run[1] = x + 1
            else:
                if run:
                    out.append((run[0], (run[1] + 1) & ~1))
                run = [x & ~1, x + 1]
        if run:
            out.append((run[0], (run[1] + 1) & ~1))
        # (a pc-relative load or adr in the block of what it names, and a branch in the block of the instruction it
        # reaches, the bytes between kept as they are, never run: the move keeps distances within a block alone, a pool
        # more than 0x20 bytes past its code would be another block's, and a branch reaches no further than its own
        # bits: ShadowSoul's sword family's phase 0x0C ends through the pop of its phase 0x10, 0x080EC232, 0x2A bytes
        # past it across that phase, which is BN6's twin's)
        def run_of(a):
            return next((k for k, (lo, hi) in enumerate(out) if lo <= a < hi), None)
        joined = True
        while joined:
            joined = False
            for w in self.walks.values():
                for x, i in sorted(w.insns.items()):
                    reach = i.target if i.form in ('b', 'bcc') else None
                    pair = [(run_of(x), run_of(t)) for t in (i.lit, i.adr, reach) if t is not None]
                    pair = [(a, b) for a, b in pair if a is not None and b is not None and a != b]
                    if pair:
                        a, b = min(pair[0]), max(pair[0])
                        out[a:b + 1] = [(out[a][0], out[b][1])]
                        joined = True
                        break
                if joined:
                    break
        self.blocks = out
        self.cov = cov

    def own_ranges(self):
        """DATA_RANGES but those in another track's copy (its own data, fixed there)"""
        return [(lo, hi) for lo, hi in DATA_RANGES if not self.shared_at(lo)]

    def place_blocks(self):
        at, placed = 0, []
        for lo, hi in self.blocks:
            off = rounded(at, BLOCK_STEP) + (lo & 3)
            placed.append((lo, hi, off, zlib.crc32(self.b5.bytes(lo, hi - lo)) & 0xFFFFFFFF))
            at = off + (hi - lo)
        if at > CODE_ROOM:
            self.decide(f'the code takes {at:#x} bytes, past SOULS_CODE\'s {CODE_ROOM:#x}')
        self.placed = placed
        self.code_used = at

    def moved(self, a):
        for lo, hi, off, _ in self.placed:
            if lo <= (a & ~1) < hi:
                return off + (a - lo)
        return None

    def raw_pointers(self):
        """words of raw data near the code that point into the blocks (a table of tables): moved with them"""
        code = set()
        for w in self.walks.values():
            for x, i in w.insns.items():
                code.update(range(x, x + i.size))
        for lo, hi in self.own_ranges():
            for a in range(lo, hi, 4):
                v = self.b5.u32(a)
                if a not in self.words and any(b0 <= v < b1 for b0, b1 in self.blocks):
                    self.words[a] = (v, None, 'a pointer in a table of its data, moved')

    def data_ranges(self):
        """each block's bytes that are no code: pools and tables (words, each listed) and raw bytes"""
        code = set()
        for w in self.walks.values():
            for x, i in w.insns.items():
                code.update(range(x, x + i.size))
        words = set(self.words)
        out = []
        for lo, hi, _, _ in self.placed:
            a = lo
            while a < hi:
                if a in code:
                    a += 2
                    continue
                kind = 'XV_POOL' if a in words or (a - 2) in words else 'XV_RAW'
                b = a + 2
                if kind == 'XV_POOL':
                    b = a + 4
                    while b < hi and b in words:
                        b += 4
                else:
                    while b < hi and b not in code and b not in words:
                        b += 2
                out.append((a, b, kind))
                a = b
        self.data = out
        # (raw bytes that hold a word naming a routine or the code: a table the walk did not see, which would run BN5's
        # address in BN6)
        for a, b, kind in out:
            if kind != 'XV_RAW':
                continue
            for x in range((a + 3) & ~3, b - 3, 4):
                v = self.b5.u32(x)
                if any(lo <= x < hi for lo, hi in DEAD_TABLES):
                    continue
                if T.is_code_ptr(v, self.b5) and ((v & ~1) in self.walks or self.twin(v)):
                    self.decide(f'{x:08X}: raw data holds {v:08X}, a routine\'s address: a table the walk did not take whole')

    # ---- numbers BN6 has otherwise ----

    def scan_numbers(self, families):
        for k, kind in sorted(bn5_objects.IMAGE_COPIED.items()):   # (another track's numbers for a sprite: its kinds' place)
            if kind[2] != 'souls':
                e = self.b5.bytes(FLASH_KINDS5 + 5 * k, 2)
                self.placed_by[(e[0] // 4, e[1])] = kind
        # (each sprite's number as ART_ORDER gave it, kept whether or not the Souls copy it now: another track's copy may serve)
        for k, e in enumerate(ART_ORDER):
            if e is not None:
                self.reserved[e] = ART_FIRST + k
        self.next_art = ART_FIRST + len(ART_ORDER)
        for e in EFFECT_ORDER:
            self.bn6_effect(e)
        for a, w in self.walks.items():
            for site, t in w.bls.items():
                if t == SOUND:
                    self.number_sound(w, site)
                elif t == EFFECT:
                    self.number_effect(w, site)
                elif t == SPRITE_LOAD:
                    self.number_sprite(w, site)
                elif t in SPRITE_SETUPS:
                    self.number_setup(w, site)
            self.scan_collision(w)
            self.scan_fields(w)
        for tab in SPRITE_TABLES:
            if any(tab in w.lits.values() for w in self.walks.values()) and not self.shared_at(tab):
                self.number_sprite_table(tab)
        for lst, idx in EXTRA_ART:
            self.place_sprite(lst, idx)
        self.late = True
        for w, site in self.deferred:
            self.number_effect(w, site)
        # (the flash kinds as the one per-kind table of the image object has them, the moved objects' copy fixing each:
        # tools/bn5_objects.py; a kind of the Souls' own their sprite, copied here at the number that table gives it)
        for kind in FLASH_KINDS:
            e = self.b5.bytes(FLASH_KINDS5 + 5 * kind, 2)
            fixed = bn5_objects.IMAGE_TWIN.get(kind) or bn5_objects.IMAGE_COPIED.get(kind, (None, None))[:2]
            if fixed[0] is None:
                self.decide(f'the image object\'s kind {kind:#x}: unfixed in tools/bn5_objects.py')
            elif bn5_objects.IMAGE_COPIED.get(kind, (0, 0, ''))[2] == 'souls':
                got = self.place_sprite(e[0] // 4, e[1])
                if got != tuple(fixed):
                    self.decide(f'the image object\'s kind {kind:#x}: {got} here, {fixed} in tools/bn5_objects.py')
        # (the rows of other tracks' copies the Souls' code spawns them with, fixed there to the Souls' numbers: they agree)
        for r in bn5_twins.SHARED_ROWS:
            if r['track'] != 'souls':
                continue
            got = self.bn6_effect(self.b5.u8(r['at'])) if r['kind'] == 'effect' else \
                self.place_sprite(self.b5.u8(r['at']) // 4, self.b5.u8(r['at'] + 1))
            if got != r['to']:
                self.decide(f'{r["at"]:08X}: {got} here, {r["to"]} in tools/bn5_twins.py\'s SHARED_ROWS')
        for site, (fam5, fam6) in families.items():
            self.imms.append(dict(at=site, form='THUMB_MOVS', rd=0, rn=0, was=fam5, now=fam6,
                                  why=f'its family: CurAction {fam5:#x} in BN5, the Souls\' {fam6:#x} in BN6 (xattack_grow.h)'))

    def imm_for(self, chain, reg, was, now, why):
        for x, form, v in chain:
            if form in ('movs', 'adds8') and v == was:
                self.imms.append(dict(at=x, form='THUMB_MOVS' if form == 'movs' else 'THUMB_ADDS', rd=reg, rn=0, was=was, now=now, why=why))
                return True
        return False

    def number_sound(self, w, site):
        v, chain = T.back_value(w, site, 0)
        if v is None and site in SOUND_TABLES:
            for at, why in SOUND_TABLES[site]:
                s = self.b5.u16(at)
                if s and xrecipe.song_sig(self.b5, 0x08155BF4, s) != xrecipe.song_sig(self.b6, 0x08159F48, s):
                    self.decide(f'{at:08X}: sound {s:#x} for {why} is another song in Gregar')
            return
        if v is None:
            self.decide(f'{site:08X}: a sound made otherwise ({chain})')
            return
        same = xrecipe.song_sig(self.b5, 0x08155BF4, v) == xrecipe.song_sig(self.b6, 0x08159F48, v)
        if same:
            return
        if v not in SONG_SLOTS:
            self.decide(f'{site:08X}: sound {v:#x} is another song in Gregar, and no slot is allocated it')
            return
        now = SONG_SLOTS[v]
        self.songs[v] = now
        # (BN5's value made by movs and an adds of 0xFF: the low byte set by its first instruction, the songs past 0xFF
        # and BN6's slot under it would need both changed)
        if v < 0x100:
            if not self.imm_for(chain, 0, v, now, f'its sound: song {v:#x} in BN5, BN6\'s slot {now:#x} (soulpower.c copies it)'):
                self.decide(f'{site:08X}: sound {v:#x} made by {chain}')
        else:
            movs = [x for x in chain if x[1] == 'movs']
            adds = [x for x in chain if x[1] == 'adds8']
            if len(movs) == 1 and len(adds) == 1 and adds[0][2] == 0xFF:
                self.imms.append(dict(at=movs[0][0], form='THUMB_MOVS', rd=0, rn=0, was=movs[0][2], now=now,
                                      why=f'its sound: song {v:#x} in BN5 (movs, adds 0xFF), BN6\'s slot {now:#x}'))
                self.imms.append(dict(at=adds[0][0], form='THUMB_ADDS', rd=0, rn=0, was=0xFF, now=0,
                                      why=f'... its adds made none (soulpower.c copies it)'))
            else:
                self.decide(f'{site:08X}: sound {v:#x} made by {chain}')

    def bn6_sprite_twin(self, lst, idx):
        p, b = xrecipe.sprite(self.b5, LISTS5, lst, idx)
        if not b:
            return None
        key = (lst, len(b))
        if key not in self.six:
            base, out = self.b6.u32(LISTS6 + 4 * lst), {}
            for k in range(256):
                q = self.b6.u32(base + 4 * k)
                if not 0x08000000 <= q & 0x7FFFFFFF < 0x08800000:
                    break
                _, c = xrecipe.sprite(self.b6, LISTS6, lst, k)
                if c:
                    out.setdefault(hashlib.sha1(c).hexdigest(), k)
            self.six[key] = out
        return self.six[key].get(hashlib.sha1(b).hexdigest())

    def place_sprite(self, lst, idx):
        """BN5's sprite (list, index) in BN6: its own number where BN6 has the same bytes, else a copy in the Souls' room
        listed in sprite list 0, the Souls' own, from ART_FIRST up"""
        if (lst, idx) in self.number_of:
            return self.number_of[(lst, idx)]
        tw = self.bn6_sprite_twin(lst, idx)
        if tw is not None:
            self.number_of[(lst, idx)] = (lst, tw)
            return (lst, tw)
        if (lst, idx) in self.placed_by and self.placed_by[(lst, idx)][2] != 'souls' or (lst, idx) in self.others_sprites:
            # (another track's copy at its number, listed by its install before the Souls': it serves, one BN6 number a
            # BN5 sprite)
            to = self.placed_by[(lst, idx)][:2] if (lst, idx) in self.placed_by else self.others_sprites[(lst, idx)]
            self.number_of[(lst, idx)] = tuple(to)
            return tuple(to)
        if (lst, idx) in self.reserved:
            to_list, num = ART_LIST, self.reserved[(lst, idx)]
        else:
            to_list, num = ART_LIST, self.next_art
            if num > ART_LAST:
                self.decide(f'BN5\'s sprite {lst}:{idx:#x}: list 0 past its Souls\' numbers ({ART_LAST:#x})')
            self.next_art += 1
        p, b = xrecipe.sprite(self.b5, LISTS5, lst, idx)
        if not b:
            self.decide(f'BN5\'s sprite {lst}:{idx:#x} cannot be read')
            return None
        # (in SOULS_ART while it holds the sprite, else in FREE_SOULS_ART: each region filled in the recipe's order)
        room = rounded(len(b), ART_STEP)
        at = rounded(self.art, ART_STEP)
        if at + room <= ART_ROOM:
            self.art = at + len(b)
            base = 'SOULS_ART'
        else:
            at = rounded(self.art2, ART_STEP)
            self.art2 = at + len(b)
            base = 'FREE_SOULS_ART'
        self.sprites.append(dict(list=lst, index=idx, to_list=to_list, to_index=num, at=at, room=room, base=base))
        self.number_of[(lst, idx)] = (to_list, num)
        return (to_list, num)

    def number_sprite(self, w, site):
        l, lchain = T.back_value(w, site, 1)
        v, vchain = T.back_value(w, site, 2)
        if l is None or v is None:
            tabs = [t for t in SPRITE_TABLES if t in w.lits.values()]
            if not tabs:
                self.decide(f'{site:08X}: a sprite loaded by a number made otherwise')
            for tab in tabs:
                self.number_sprite_table(tab)
            return
        lst = l // 4
        to = self.place_sprite(lst, v)
        if to is None:
            return
        if to[0] != lst and not self.imm_for(lchain, 1, l, to[0] * 4, f'its sprite\'s list: {lst} in BN5, {to[0]} in BN6'):
            self.decide(f'{site:08X}: the sprite\'s list made by {lchain}')
        if to[1] != v and not self.imm_for(vchain, 2, v, to[1], f'its sprite: list {lst} {v:#x} in BN5, list {to[0]} {to[1]:#x} in BN6'):
            self.decide(f'{site:08X}: the sprite\'s number made by {vchain}')

    def number_setup(self, w, site):
        """a sprite set up from a packed word (SPRITE_SETUPS), its list and number BN6's: the word's low half in its literal"""
        v, chain = T.back_value(w, site, 0)
        if v is None:
            self.decide(f'{site:08X}: a sprite set up from a word made otherwise ({chain})')
            return
        lst, idx = (v >> 8 & 0xFF) // 4, v & 0xFF
        to = self.place_sprite(lst, idx)
        if to is None or to == (lst, idx):
            return
        lit = w.insns[chain[0][0]].lit if chain and chain[0][1] == 'ldr_lit' else None
        if lit is None:
            self.decide(f'{site:08X}: the packed word of sprite {lst}:{idx:#x} made otherwise ({chain})')
            return
        # (and every other word of the routine's that r0 takes of the same sprite: another path to the call, its animation
        # another, the soldier's by its kind at 0x080CA942 and 0x080CA94A)
        for i in w.insns.values():
            x = w.lits.get(i.lit) if i.form == 'ldr_lit' and i.f and i.f[0] == 0 else None
            if x is not None and (x & 0xFFFF) == (v & 0xFFFF):
                now = (x & ~0xFFFF) | (4 * to[0]) << 8 | to[1]
                self.words[i.lit] = (x, f'0x{now:08X}u', f'its sprite: list {lst} {idx:#x} in BN5, list {to[0]} {to[1]:#x} in BN6 '
                                                         f'(a packed word)')

    def number_sprite_table(self, tab):
        if tab in self.sprite_tables:
            return
        self.sprite_tables.add(tab)
        step, used, whose = SPRITE_TABLES[tab]
        for k in used:
            at = tab + step * k
            l5, i5 = self.b5.u8(at), self.b5.u8(at + 1)
            to = self.place_sprite(l5 // 4, i5)
            if to is None:
                continue
            if to[0] * 4 != l5:
                self.bytes_.append(dict(at=at, was=l5, now=to[0] * 4, why=f'{whose}\'s sprite\'s list ({k}): {l5 // 4} in BN5, {to[0]} in BN6'))
            if to[1] != i5:
                self.bytes_.append(dict(at=at + 1, was=i5, now=to[1], why=f'{whose}\'s sprite ({k}): list {l5 // 4} {i5:#x} in BN5, '
                                                                          f'list {to[0]} {to[1]:#x} in BN6'))

    def bn6_effect(self, e5):
        """BN6's effect of the same sprite (by its bytes), animation and palette, else a new id with BN5's sprite copied"""
        if e5 in self.effect_of:
            return self.effect_of[e5]
        if e5 in self.others_effects:
            # (another track copied it first: its number, its record that track's install's, one BN6 number a BN5 effect)
            self.effect_of[e5] = self.others_effects[e5]
            self.shared_effects.add(self.others_effects[e5])
            return self.others_effects[e5]
        r5 = self.b5.bytes(EFFECTS5 + 4 * e5, 4)
        tw = self.bn6_sprite_twin(r5[0] // 4, r5[1])
        if tw is not None:
            for e6 in range(NEFFECTS):
                q = self.b6.bytes(EFFECTS6 + 4 * e6, 4)
                if q[0] == r5[0] and q[1] == tw and q[2] == r5[2] and q[3] == r5[3]:
                    self.effect_of[e5] = e6
                    return e6
        to = self.place_sprite(r5[0] // 4, r5[1])
        e6 = self.next_effect
        self.next_effect += 1
        if e6 > EFFECT_LAST:
            self.decide(f'BN5\'s effect {e5:#x}: past the Souls\' effect ids ({EFFECT_LAST:#x})')
        self.effects.append(dict(from_=e5, to=e6, list=to[0], index=to[1], anim=r5[2], pal=r5[3]))
        self.effect_of[e5] = e6
        return e6

    def number_effect(self, w, site):
        v, chain = T.back_value(w, site, 4)
        if v is None:
            # (r4 its flip shifted in from another register, then its effect added: the adds' immediate)
            if len(chain) == 2 and chain[0][1] == 'adds8' and chain[1][1] == 'lsls':
                v, chain = chain[0][2], chain[:1]
            else:
                for at, (base, n) in EFFECT_BASES.items():
                    if at in w.insns:
                        self.effect_run(at, base, n)
                        return
                for at, why in EFFECT_TABLES.items():
                    if EFFECT_SITES.get(site, at) == at:
                        lo = min(w.insns)
                        if site in EFFECT_SITES or lo - 0x100 <= at <= max(w.insns) + 0x200:
                            e = self.b5.u8(at)
                            e6 = self.bn6_effect(e)
                            if e6 != e:
                                self.bytes_.append(dict(at=at, was=e, now=e6, why=f'its effect for {why}: {e:#x} in BN5, {e6:#x} in BN6'))
                            return
                self.decide(f'{site:08X}: an effect made otherwise ({chain})')
                return
        e = v & 0xFF
        if not self.late and self.redrawn(e):
            self.deferred.append((w, site))
            return
        e6 = self.bn6_effect(e)
        if e6 != e and not self.imm_for(chain, 4, e, e6, f'its effect: {e:#x} in BN5, {e6:#x} in BN6'):
            self.decide(f'{site:08X}: effect {e:#x} made by {chain}')

    def redrawn(self, e5):
        """whether BN6 has an effect of BN5's record (its list, number, animation, palette) whose sprite's bytes differ, its
        animation as long: one BN6 redrew, copied with BN5's sprite all the same (the tracks' allocations: drawn as BN5's),
        its id and sprite numbered after the others' (FREE_SOULS_ART takes what SOULS_ART cannot)"""
        r5 = self.b5.bytes(EFFECTS5 + 4 * e5, 4)
        if self.bn6_sprite_twin(r5[0] // 4, r5[1]) is not None or not any(self.b6.bytes(EFFECTS6 + 4 * e6, 4) == r5 for e6 in range(NEFFECTS)):
            return False
        # (one whose animation lasts as long in both, frame for frame: an effect BN6's engine spawns of its own too, the
        # same role; one whose animation lasts otherwise was copied from the first, as the engine's own would end apart)
        _, s5 = xrecipe.sprite(self.b5, LISTS5, r5[0] // 4, r5[1])
        _, s6 = xrecipe.sprite(self.b6, LISTS6, r5[0] // 4, r5[1])
        return anim_life(s5, r5[2]) is not None and anim_life(s5, r5[2]) == anim_life(s6, r5[2])

    def effect_run(self, at, base, n):
        """an effect id of a base and a number from 0 to n - 1: BN6's ids one after another, BN5's copied as a run"""
        if at in self.runs:
            return
        self.runs.add(at)
        # (a run EFFECT_ORDER numbered already, one after another: its ids kept)
        given = [self.effect_of.get(base + k) for k in range(n)]
        if None not in given and given == list(range(given[0], given[0] + n)):
            first = given[0]
        else:
            first = self.next_effect
            self.next_effect += n
            if self.next_effect - 1 > EFFECT_LAST:
                self.decide(f'BN5\'s effects {base:#x}-{base + n - 1:#x}: past the Souls\' effect ids ({EFFECT_LAST:#x})')
            for k in range(n):
                r5 = self.b5.bytes(EFFECTS5 + 4 * (base + k), 4)
                to = self.place_sprite(r5[0] // 4, r5[1])
                self.effects.append(dict(from_=base + k, to=first + k, list=to[0], index=to[1], anim=r5[2], pal=r5[3]))
                self.effect_of[base + k] = first + k
        self.imms.append(dict(at=at, form='THUMB_ADDS', rd=self.b5.u16(at) >> 8 & 7, rn=0, was=base, now=first,
                              why=f'its effects {base:#x}-{base + n - 1:#x} in BN5, {first:#x}-{first + n - 1:#x} in BN6'))

    def scan_fields(self, w):
        """loads and stores of BN5's AI data's Soul fields (a register loaded from [r5,#0x58]): the Souls' C answers"""
        ai = set()
        for a in sorted(w.insns):
            i = w.insns[a]
            if i.form == 'ldr' and len(i.f) > 2 and i.f[1] == 5 and i.f[2] == 0x58:
                ai.add(i.f[0])
                continue
            if i.form in ('ldrb', 'ldrh', 'ldr', 'strb', 'strh', 'str') and len(i.f) > 2 and i.f[1] in ai and i.f[2] in SOUL_FIELDS:
                if i.form.startswith('st'):
                    self.decide(f'{a:08X}: a store to BN5\'s AI data +{i.f[2]:#x} ({SOUL_FIELDS[i.f[2]]})')
                else:
                    self.fields.append(dict(at=a, form=i.form, rd=i.f[0], field=SOUL_FIELDS[i.f[2]], off=i.f[2]))
            if i.form == 'bl':
                ai -= {0, 1, 2, 3}
            elif i.f and T.writes(i, i.f[0]) and i.f[0] in ai and not (i.form == 'ldr' and i.f[2] == 0x58):
                ai.discard(i.f[0])

    def scan_collision(self, w):
        forms = {'ldr': 'THUMB_LDR', 'str': 'THUMB_STR', 'ldrb': 'THUMB_LDRB', 'strb': 'THUMB_STRB', 'ldrh': 'THUMB_LDRH', 'strh': 'THUMB_STRH'}
        coll = set()
        for a in sorted(w.insns):
            i = w.insns[a]
            if i.form == 'ldr' and len(i.f) > 2 and i.f[2] == 0x54:
                coll.add(i.f[0])
                continue
            if i.form in forms and i.f[1] in coll:
                off = i.f[2]
                to = xrecipe_family.coll_to6(off)
                if to is None:
                    self.decide(f'{a:08X}: its collision data\'s +{off:#x}, which BN6 has no place for')
                elif to != off:
                    self.imms.append(dict(at=a, form=forms[i.form], rd=i.f[0], rn=i.f[1], was=off, now=to,
                                          why=f'collision data +{off:#x} in BN5, +{to:#x} in BN6'))
            if i.form == 'bl':
                coll -= {0, 1, 2, 3}
            elif i.form in forms and i.f[0] in coll:
                coll.discard(i.f[0])
            elif i.f and T.writes(i, i.f[0]) and i.f[0] in coll:
                coll.discard(i.f[0])


def derive(b5, b6, pairs):
    d = Deriv(b5, b6, pairs)
    for name, soul, roots in UNITS:
        d.walk(name, roots)
    # (each power attack returns its family by its last movs r0 before pop {pc})
    families = {}
    fam6 = {f5: f6 for f5, f6, _ in FAMILIES}
    for id5, entry, why in POWERS:
        w = d.walks.get(entry)
        last = None
        for a in sorted(w.insns):
            i = w.insns[a]
            if i.form == 'movs' and i.f[0] == 0:
                last = (a, i.f[1])
            if i.form == 'pop' and i.f[1]:
                break
        if last and last[1] in fam6:
            families[last[0]] = (last[1], fam6[last[1]])
    d.resolve_calls()
    for a in ROUTED:
        if d.shared_at(a):
            d.routes[a] = d.shared_expr(a)
        else:
            d.decide(f'{a:08X}: the Souls\' C reaches it in another track\'s copy, which holds none of it')
    d.make_blocks()
    d.resolve_words()
    d.raw_pointers()
    d.place_blocks()
    d.data_ranges()
    d.scan_numbers(families)
    if d.art > ART_ROOM or d.art2 > ART_ROOM2:
        d.decide(f'the sprites take {d.art:#x} and {d.art2:#x} bytes, past SOULS_ART\'s {ART_ROOM:#x} before the faces or '
                 f'FREE_SOULS_ART\'s {ART_ROOM2:#x}')
    return d


# ---- the C file ----

def c_value(d, bn5, now):
    """a word's or call's BN6 value as C: a moved address (SOULS_CODE + its offset, its Thumb bit kept), or as given"""
    if now is not None:
        return now
    off = d.moved(bn5)
    return f'SOULS_CODE + 0x{off | (bn5 & 1):X}u'


def write_c(d):
    L = ['/* soulcode_bn5.c: the code BN5\'s Team Colonel Souls run, moved into BN6\'s battle engine, written by tools/xsoul.py from',
         ' * the two ROMs (python3 tools/xsoul.py; --check says whether this is what it writes). Do not edit: change the generator',
         ' * and write it again. Numbers alone (docs/multirom/souls.md): addresses in BN5 Team Colonel (USA) and BN6',
         ' * Cybeast Gregar (USA), counts and the CRC32s of the ranges src/emu/soulcode.c copies from the player\'s ROM. */',
         '#include "soulcode.h"', '', '#include <stddef.h>', '', '#include "bn6.h"', '#include "soul_room.h"', '#include "xchips_code.h"',
         '#include "xnavi_recipe.h"', '#include "xvirus_dispatch.h"', '']
    L.append('static const XvBlock blocks[] = {')
    for lo, hi, off, crc in d.placed:
        L.append(f'\t{{ 0x{lo:08X}, 0x{hi:08X}, SOULS_CODE + 0x{off:X}, 0x{crc:08X}u }},   /* {d.unit_of.get(lo, "data")} */')
    L.append('};')
    L.append('static const XvData data[] = {')
    for a, b, k in d.data:
        L.append(f'\t{{ 0x{a:08X}, 0x{b:08X}, {k} }},')
    L.append('};')
    L.append('/* every word of the pools and tables: BN5\'s, and BN6\'s in its place */')
    L.append('static const XvNumber words[] = {')
    for at in sorted(d.words):
        v, now, why = d.words[at]
        L.append(f'\t{{ 0x{at:08X}, 0x{v:08X}u, {c_value(d, v, now)} }},   /* {why} */')
    L.append('};')
    L.append('/* every call out of its block: its site, BN5\'s routine, BN6\'s in its place */')
    L.append('static const XvNumber calls[] = {')
    for site in sorted(d.calls):
        t, now, why = d.calls[site]
        if now is None and d.moved(t) is not None and d.moved(site) is not None and \
                any(lo <= site < hi and lo <= t < hi for lo, hi, _, _ in d.placed):
            continue   # (a call within its block: it holds as the block moves)
        L.append(f'\t{{ 0x{site:08X}, 0x{t:08X}u, {c_value(d, t | 1, now)} }},   /* {why} */')
    L.append('};')
    L.append('static const XvImm imms[] = {')
    for m in sorted(d.imms, key=lambda m: m['at']):
        L.append(f'\t{{ 0x{m["at"]:08X}, {m["form"]}, {m["rd"]}, {m["rn"]}, 0x{m["was"]:X}, 0x{m["now"]:X} }},   /* {m["why"]} */')
    L.append('};')
    L.append('/* the object types its code spawns through the second-level dispatch: BN6\'s twin, moved (its routine BN5\'s), or another')
    L.append(' * track\'s copy, installed before (shared: tools/bn5_twins.py\'s TYPES) */')
    L.append('static const SoulType types[] = {')
    for (kind, v), to in sorted(d.types.items()):
        if to is None:
            r5 = d.b5.u32(TYPES5[kind][0] + 4 * v)
            L.append(f'\t{{ {kind}, 0x{v:02X}, XV_TYPE_MOVED, 0x{r5:08X}u }},')
        elif to == 'shared':
            L.append(f'\t{{ {kind}, 0x{v:02X}, XV_TYPE_SHARED, 0 }},   /* the {bn5_twins.TYPES[(kind, v)][2]}\' */')
        else:
            L.append(f'\t{{ {kind}, 0x{v:02X}, 0x{to:02X}, 0 }},')
    L.append('};')
    L.append('/* BN5\'s routines and data its code runs in other tracks\' copies (those of the types it shares), each\'s place there: what')
    L.append(' * src/emu/soulcode.c\'s soulcode_moved answers for them */')
    L.append('static const SoulRoute routes[] = {')
    for a in sorted(d.routes):
        L.append(f'\t{{ 0x{a:08X}u, {d.routes[a]} }},')
    L.append('};')
    L.append('/* the effects it spawns of another track\'s install, one BN6 number a BN5 effect: present before the Souls\' install */')
    L.append('static const uint8_t shared_effects[] = {')
    if d.shared_effects:
        L.append('\t' + ', '.join(f'0x{e:02X}' for e in sorted(d.shared_effects)) + ',')
    L.append('};')
    L.append('/* BN5\'s sprites its code draws that BN6 has none of by their bytes: copied into SOULS_ART (past it FREE_SOULS_ART), listed at a number of a grown list')
    L.append(' * (list 0 the Souls\' own; another\'s list a number of the track tools/bn5_objects.py names, whose copy serves where it stands first) */')
    L.append('static const XvSprite sprites[] = {')
    for s in d.sprites:
        L.append(f'\t{{ {s["list"]}, 0x{s["index"]:02X}, {s["to_list"]}, 0x{s["to_index"]:02X}, {s["base"]} + 0x{s["at"]:X}, 0x{s["room"]:X} }},')
    L.append('};')
    L.append('/* BN5\'s effects its code spawns that BN6 has none of: at an id of the grown table, BN5\'s entry with its sprite as listed above */')
    L.append('static const SoulEffect effects[] = {')
    for e in sorted(d.effects, key=lambda e: e['to']):
        L.append(f'\t{{ 0x{e["from_"]:02X}, 0x{e["to"]:02X}, {e["list"]}, 0x{e["index"]:02X}, 0x{e["anim"]:02X}, 0x{e["pal"]:02X} }},')
    L.append('};')
    L.append('/* bytes of its raw tables BN6 numbers otherwise */')
    L.append('static const XvByte bytes[] = {')
    for y in sorted(d.bytes_, key=lambda y: y['at']):
        L.append(f'\t{{ 0x{y["at"]:08X}, 0x{y["was"]:02X}, 0x{y["now"]:02X} }},   /* {y["why"]} */')
    L.append('};')
    L.append('/* its reads of BN5\'s AI data\'s Soul fields, which BN6\'s AI data holds otherwise: the Souls\' C answers from BN6_SOULS */')
    L.append('static const SoulField fields[] = {')
    for f in sorted(d.fields, key=lambda f: f['at']):
        L.append(f'\t{{ 0x{f["at"]:08X}, {f["rd"]}, {f["form"] == "ldrh" and 2 or f["form"] == "ldr" and 4 or 1}, {f["field"]} }},   /* AI data +{f["off"]:#x} */')
    L.append('};')
    L.append('/* the power attacks a Soul\'s loadout names, BN5\'s ids, each its entry in BN5 (moved) */')
    L.append('static const SoulPower powers[] = {')
    for id5, entry, why in POWERS:
        L.append(f'\t{{ 0x{id5:02X}, 0x{entry | 1:08X}u }},   /* {why} */')
    L.append('};')
    L.append('/* BN5\'s songs its code plays that Gregar has otherwise: copied into an empty slot (soulpower.c) */')
    L.append('static const SoulSong songs[] = {')
    for v, now in sorted(d.songs.items(), key=lambda x: -x[1]):
        L.append(f'\t{{ 0x{v:X}, 0x{now:02X} }},')
    L.append('};')
    L.append('/* the families moved: BN5\'s CurAction, BN6\'s, BN5\'s entry */')
    L.append('static const SoulFamily families[] = {')
    for f5, f6, entry in FAMILIES:
        L.append(f'\t{{ 0x{f5:02X}, 0x{f6:02X}, 0x{entry | 1:08X}u }},')
    L.append('};')
    L.append('')
    # (a table with no entries left out: C has no array of none)
    out, empty, k = [], set(), 0
    while k < len(L):
        line = L[k]
        if line.startswith('static const ') and line.endswith('[] = {') and k + 1 < len(L) and L[k + 1] == '};':
            empty.add(line.split()[3][:-2])
            k += 2
            continue
        out.append(line)
        k += 1
    L = out
    names = ('blocks', 'data', 'words', 'calls', 'imms', 'types', 'sprites', 'effects', 'bytes', 'fields', 'powers', 'songs',
             'families', 'routes', 'shared_effects')
    L.append('#define N(a) (int)(sizeof (a) / sizeof *(a))')
    L.append('const SoulCode soulcode_bn5 = {')
    L.append('\t' + ', '.join('NULL, 0' if n in empty else f'{n}, N({n})' for n in names) + ',')
    L.append('};')
    return '\n'.join(L) + '\n'


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--check', action='store_true')
    ap.add_argument('--report', action='store_true')
    a = ap.parse_args()
    b5, b6 = xrecipe.find_roms()
    pairs = Pairs(b5, b6, MATCH, WORK)
    d = derive(b5, b6, pairs)
    if a.report:
        for e in sorted(d.walks):
            w = d.walks[e]
            print(f'{d.unit_of[e]:9} {e:08X}-{max(w.insns) + 2:08X}')
        for n in d.notes:
            print('note:', n)
        print(f'code {d.code_used:#x} bytes in {len(d.placed)} blocks, art {d.art:#x} + {d.art2:#x} bytes, {len(d.sprites)} sprites, {len(d.effects)} effects')
    for x in d.decisions:
        print('OPEN:', x)
    text = write_c(d)
    if a.check:
        with open(OUT_C, encoding='utf-8') as f:
            if f.read() != text:
                sys.exit('xsoul: src/emu/soulcode_bn5.c is not what tools/xsoul.py writes')
        print('xsoul: src/emu/soulcode_bn5.c is what it writes')
        return
    if d.decisions:
        print(f'xsoul: {len(d.decisions)} decisions open: written all the same, for the report')
    with open(OUT_C, 'w', encoding='utf-8') as f:
        f.write(text)
    print(f'xsoul: wrote {os.path.relpath(OUT_C, ROOT)}')


if __name__ == '__main__':
    main()
