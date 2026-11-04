/* BN6's AI data in a battle: the pool its actors' AI data comes from,
 * eight entries of BN6_AI_DATA_SIZE bytes (issue #182; docs/rom/navis.md,
 * BN6's AI data's pool). Not the first kind's 32 slots: an actor spawned
 * with AI (0x08007540, 0x080076C0) takes the first entry free, or none
 * and is let go; a MegaMan, a Navi, a virus each take one. And the getter
 * an AI asks a side's MegaMan of. */
#ifndef CW_BN6_AI_H
#define CW_BN6_AI_H

#define BN6_AI_POOL           0x02034080u /* the pool: entry i at + i x BN6_AI_DATA_SIZE, laid out as a battle sets up (0x0800ED00, from
                                             0x08007280), each entry's BN6_AI_POOL_BIT its own bit */
#define BN6_AI_POOL_ENTRIES   8           /* ... its entries (the take tries eight bits, 0x0800ED44) */
#define BN6_AI_POOL_USED      0x0203F6A0u /* u32 the entries taken, entry i bit 31 - i: the take (0x0800ED2C) sets the first clear one and
                                             lays the entry out, the let-go (0x0800ED80) clears the bit its AI data's +0x7C names; the
                                             battle's setup clears them all */
#define BN6_AI_POOL_BIT       0x7C        /* u32 an entry's own bit (a copy of the AI data carries it: its let-go clears the entry it came from) */
#define BN6_AI_TAKE           0x0800ED2Cu /* r0 the entry taken (laid out: +0 to +0x7B, +0x80 to +0x9F, +0xA0 to +0xEF zeroed), 0 none free */
#define BN6_AI_LET_GO         0x0800ED80u /* r0 the AI data: the entry its +0x7C names free again (mov pc,lr) */
#define BN6_SIDE_PLAYER_GET   0x080103BCu /* r0 a side: its player's object (BN6_BATTLE_PLAYERS' first word of it, where its name's class,
                                             0x0800F29C, is a MegaMan's), else 0: a side's player alone, never his partner. BN5's twin
                                             0x0800E140, which BN5's moved Navis (ShadowMan's, NumberMan's, TomahawkMan's AIs) call for the
                                             MegaMan they aim at; the five chips' copies of it (BN5's 0x080125EE, a side's player's
                                             AIData +0x12) are never called (issue #181's audit) */

/* An AI data's fields read in BN4's differential (docs/devtools/differential.md, MegaMan's AI data and his object; each
 * paired with BN4's by both games' code, bn4_battle.h) */
#define BN6_AI_PALETTE_STEP   0x03        /* u8 the palette's step: the battle actor's setup (0x0800768C) writes its enemy's record 1 +2
                                           * (1 MegaMan's), read only by the actor init's palette (0x0800F334, from 0x08016F90; palette =
                                           * 0x0800F354[version] x step) and a virus's version change (0x0810A90A): neither runs for MegaMan */
#define BN6_AI_POWER_CHIP     0x05        /* u8 a player's chip charge's power attack (0xFF none: his loadout 0x080144C0), read by the chip
                                           * charge's dispatcher (0x0800FB9A) and its charge time (0x08012F6C: r7 5 for A, 7 for B); for a
                                           * chip whose record +6 is 0xA both read BN6_AI_POWER_CHAOS (+0x11) instead (0x0800FB96, 0x08012F8E) */
#define BN6_AI_DAMAGE_TAKEN   0x20        /* u16 the damage he has taken, at most 0xFFFF (0x08010548 adds, 0x0801055E reads by side) */
#define BN6_AI_STRUCK_FRAMES  0x4C        /* u32 the frames his collision flags hold 0xC00 (0x080143FC: counted by 0x08014432, cleared by
                                           * 0x08014424 and with his anger by 0x080143B4, read by 0x0801442C) */
#define BN6_AI_SPARE          0xF0        /* the 16 bytes past the attack's variables (+0xA0 to +0xEF), which the take never lays out (its
                                           * three spans, BN6_AI_TAKE): BN4's trace keeps there what BN6 has no place for (xdiff_bn4_trace.c) */
#define BN6_AI_SPARE_LAST     0xFC        /* ... its last word, where BN4's trace keeps BN4's charge-time cache (BN4_AI_CHARGE_TIMES) */
#define BN6_AI_HP_BUG         0x09        /* u8 the HP bug's frames counted (0x08010230: HP above 1, its period by NaviStats +0x18, one HP lost
                                           * through 0x0800E2D8 at the period, then 0) */

#endif
