/* BN6's link battle as a second MegaMan a side meets it (epic #167's 2v2
 * spike; docs/ROM_DATA.md, Two MegaMen a side): where a side's keys reach
 * its player each iteration of the battle's loop, and the list the
 * Practice NetBattle spawns its field from. */
#ifndef CW_BN6_LINK_H
#define CW_BN6_LINK_H

#define BN6_KEY_COPY          0x08012DFCu /* the battle loop's copy of a side's keys (r0 the side) from BN6_BATTLE_KEYS into its
                                             player's AI data: side 0's, then side 1's, once an iteration of the loop (bn6f sub_8012DFC) */
#define BN6_BATTLE_KEYS       0x02036820u /* each side's keys as the link delivered them, BN6_BATTLE_KEYS_STEP bytes a side: +2 held */
#define BN6_BATTLE_KEYS_STEP  8
#define BN6_SPAWN_ENTITIES    0x08007368u /* the battle's list of what it spawns walked (r0 the list: BN6_ENTITY_SIZE bytes an entry, kind,
                                             panel row << 4 | column, u16 id; the kind's high nibble the spawner, 0 MegaMan, its bit 0
                                             the side; BN6_ENTITY_END the end) (bn6f SpawnBattleObjectUsingBattleEntityConfig_8007368) */
#define BN6_ENTITY_SIZE       4
#define BN6_ENTITY_MEGAMAN    0x00        /* an entry's kind: MegaMan on side 0 (| 1: on side 1), a NetBattle's two its first entries */
#define BN6_ENTITY_END        0xF0
#define BN6_SIDE_PARTNER      4           /* BN6_BATTLE_PLAYERS + a side's BN6_BATTLE_PLAYER_STEP: its second Navi's object (0 none), its
                                             first the player's */
#define BN6_AI_HELD           0x22        /* u16 a player's AI data: the keys held, BN6_KEYS_VALID set (the link's mark), */
#define BN6_AI_PRESSED        0x24        /* ... the keys pressed this iteration, */
#define BN6_AI_UP             0x26        /* ... let go this iteration, */
#define BN6_AI_LAST           0x28        /* ... and the last iteration's held (bn6f's JoypadReleased) */
#define BN6_KEYS_VALID        0xFC00u

#endif
