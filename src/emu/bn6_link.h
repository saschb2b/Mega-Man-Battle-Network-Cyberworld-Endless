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
/* A Navi's battle NaviStats (BN6_BATTLE_NAVI, BN6_BATTLE_NAVI_SIZE a side): every reader and writer takes its side's */
#define BN6_NAVI_STATS_ADDR   0x08013682u /* r0 the side: its battle NaviStats' address in r0, eBattleNaviStats0 in r1, by mov pc,lr, no push
                                             (bn6f GetBattleNaviStatsAddr) */
#define BN6_NAVI_BY_OBJECT    0x08013754u /* the five routines that call it with the side of the object in r5 (its BN6_T1_ALLIANCE) and
                                             read or write a field r1 bytes in: set a byte, set a halfword, get a byte, the same again, get
                                             a halfword (bn6f's *_AllianceFromBattleObject), to: */
#define BN6_NAVI_BY_OBJECT_END 0x0801379Eu /* ... the last one's end */
#define BN6_NS_BASE_HP        0x3E        /* u16, a NaviStats' fields from its start: MaxBaseHP, */
#define BN6_NS_CUR_HP         0x40        /* ... CurHP (a MegaMan's object takes it as he is set up, bn6f sub_80141C8), */
#define BN6_NS_MAX_HP         0x42        /* ... and MaxHP */
#define BN6_HAND_ADDR         0x08010018u /* r0 the side: its hand's address (BN6_BATTLE_HAND, 0x50 a side) in r0, BN6_BATTLE_HAND in r1, by
                                             mov pc,lr, no push (bn6f getBattleHandAddr_8010018): the chip up next, a chip used, the
                                             Custom screen's picks; its callers pass a side, most of them the object's in r5 */
#define BN6_SPAWN_COMMON      0x08003278u /* every kind's spawn (BN6_SPAWN_1 and the rest): r0 the kind, r5 the object that spawns */
#define BN6_SPAWN_MADE        0x080032BEu /* ... the new object in r5, its words taken (its first store, of its index) */
#define BN6_T4_ACTIVE         0x02036710u /* u32 the fourth kind's slots in play (BN6_T4_OBJECTS), slot i bit 31 - i: a spawn takes the
                                             first clear one, an object let go clears its own, the battle's setup clears them all with the
                                             pool (bn6f eActiveT4BattleObjectsBitfield, the kind's record at 0x08003310) */

#endif
