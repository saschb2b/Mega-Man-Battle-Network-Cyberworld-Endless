/* BN6's AI data in a battle: the pool its actors' AI data comes from,
 * eight entries of BN6_AI_DATA_SIZE bytes (issue #182; docs/ROM_DATA.md,
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

#endif
