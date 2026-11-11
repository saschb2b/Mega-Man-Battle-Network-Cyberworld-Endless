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

#endif
