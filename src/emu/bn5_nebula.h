/* BN5's Nebula Navis of the story moved into BN6's engine, LarkMan (AI 17)
 * and GridMan (AI 18) (docs/multirom/navis.md, GridMan; docs/rom/navis.md,
 * Nebula's LarkMan and GridMan): what their moved code's proof rests on
 * beyond the generator's own reads, named for the differential's rules
 * (tests/xdiff/legit.json) and the generator's (tools/xnavirecipe.py). None
 * of it run: the recipes (src/emu/xnavi_bn5.c) copy the code. */
#ifndef CW_BN5_NEBULA_H
#define CW_BN5_NEBULA_H

/* GridMan (AI 18, ids 0x167-0x16A) */
#define BN5_GRID_SPAWN_C6    0x080DF7B4u /* the third kind's 0xC6 spawned (r0-r2, r5 kept): its panel from r0 and r1, its +0xE r2; its Z
                                            the caller's r3, which neither caller sets (0x08101414, 0x080BCDEA): a leftover */
#define BN5_GRID_ATTACK_POOL 0x081012A0u /* a pool word his state stores in AttackVars +0x28 (0x08100F84; 0x08101B61, as are the words
                                            at 0x08100F54 and 0x08102078 for 0x08101C1D and 0x08102209), called by bx (0x081012E6) */
#define BN5_GRID_EFFECT_5F   0x5Fu       /* his effect 0x5F (BN5's effects' row: list 2's 0x12, animation 0x11): his own battle sprite,
                                            an afterimage, which the recipe names at its place (xnavi_recipe.c own_sprite) */

/* LarkMan (AI 17, ids 0x161-0x164) */
#define BN5_LARK_SPAWN_B0    0x080DC4DEu /* the third kind's 0xB0 spawned (r1-r3 kept as +0x34, +0x38, +0x3C): its one caller
                                            (0x081008FA) sets no r3, its +0x3C's low half a leftover, its height at +0x3E its own */
#define BN5_LARK_SPAWN_BA    0x080DDB48u /* the third kind's 0xBA spawned: its caller's r3 a place in RAM, kept at +0x78 and as
                                            +0x3C's start, its height at +0x3E set by 0x080DDB7C */
#define BN5_LARK_AFTERIMAGE_WORD 0x08100DC8u /* the pool word ORed into r4 for his afterimage's spawn (0x08100D72): list 2's 0x11,
                                            his battle sprite, as list x 4 and number (BN5_AFTERIMAGE_SPAWN) */

/* The swords' afterimage (the fourth kind's 0x28), LarkMan's copy since his move: the chips' and ShadowSoul's spawns too */
#define BN5_AFTERIMAGE_SPAWN 0x080E36DCu /* its spawner: r4 the word its sprite is loaded by ([r5+4] list x 4, [r5+5] number) */
#define BN5_AFTERIMAGE_LOAD  0x080E3648u /* its init's load of that sprite (0x08002650): the spawner's word */

#endif
