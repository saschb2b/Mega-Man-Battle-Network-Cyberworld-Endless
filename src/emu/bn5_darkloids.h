/* BN5's Darkloids moved into BN6's engine (docs/multirom/navis.md, The
 * Darkloids; docs/rom/liberation.md, BN5's Darkloids): what their moved
 * code's proof rests on beyond the generator's own reads, named for the
 * differential's rules (tests/xdiff/legit.json, spawn_leftovers) and for
 * CloudMan's decisions and the generator's rules he brought
 * (tools/bn5_twins.py, tools/bn5_navi_twins.py, tools/xnavirecipe.py).
 * None of it run: the recipes (src/emu/xnavi_bn5.c) copy the code. */
#ifndef CW_BN5_DARKLOIDS_H
#define CW_BN5_DARKLOIDS_H

/* CosmoMan (AI 16, ids 0x15B-0x15E) */
#define BN5_COSMO_SPAWN_6C   0x080D3828u /* the third kind's 0x6C spawned (r0-r2, r5 kept): X and Y from r1 and r2, Z's upper half 0x10 written
                                            (movs r2,#0x10; strh r2,[r0,#0x3E] at 0x080D383C), Z's low half the caller's r3, never set */
#define BN5_COSMO_SPAWN_6D   0x080D3964u /* the third kind's 0x6D spawned by a 0x6C: its X, Y and Z the 0x6C's (0x080D3974-0x080D397E) */
#define BN5_COSMO_SPAWN_79   0x080D56E0u /* the third kind's 0x79 spawned: +4 the caller's r1 as a byte, a count its routine (0x080D5634) runs down */
#define BN5_COSMO_HALF_HP    0x080FF87Au /* his attack below half his HP (+0x24 under +0x26 / 2): setAttack0 with CurAction 11 at 0x080FF8AC,
                                            BN6's 13 in his copy; no spec of the differential reaches it (docs/multirom/navis.md) */

/* CloudMan (AI 15, ids 0x155-0x158): his decisions and rules (docs/multirom/navis.md, CloudMan) */
#define BN5_CLOUD_FIND_37    0x080CBFBCu /* the third kind's objects of type 0x37 (his clouds) on side r1 but slot r2: r0 the list (each
                                            object's address less 0x10), returned their count; reads BN5_T3_OBJECTS - 0x10 (0x0203CA30): in
                                            BN6's engine the Navis' stub XN_STUB_CLOUDS (src/emu/xnavi_room.c) */
#define BN5_CLOUD_PANEL      0x080CBEF6u /* a new cloud's panel, away from his others' (BN5_CLOUD_FIND_37's list) */
#define BN5_CLOUD_SPAWN_37   0x080CBEB4u /* a cloud (the third kind's 0x37) spawned, by the 0x5F (0x080D1F86): +0x4C the caller's r3, the
                                            0x5F's own parent, CloudMan */
#define BN5_CLOUD_5F_PARENT  0x080D1EE4u /* the 0x5F's state: its parent's CurAction compared with 8 (0x080D1EEA) and then 9 (0x080D1EF0), both
                                            his own states (BN6's 10 and 11 in his copy); else its countdown +0x64 run */
#define BN5_CLOUD_STATES_37  0x080CBC50u /* the cloud's sub-states by +9, its table read and branched to by bx r1 (0x080CBC54): r1 the
                                            sub-state's own address as it begins */
#define BN5_CLOUD_ROW_LEFTOVER 0x080CBD9Au /* a cloud's sub-state writes r1, never set there, as its panel row (+0x13: strb r1,[r5,#0x13]),
                                            so the low byte of its own address (0x080CBD7D); again at 0x080CBDE4: an
                                            off-field row, the same in BN6 only where the copy keeps the block's low byte
                                            (tools/xnavirecipe.py LOW_BYTE_KEPT) */
#define BN5_CLOUD_KINDS_0B   0x080CBD78u /* the kinds of the third kind's 0x0B his code spawns by his object's +4: 2, 3, 4, 5 (the
                                            records at 0x080C5DF4, whose rows xchiprecipe.py's TABLE_SPRITES gives BN6's sprites) */

#endif
