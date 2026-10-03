/* Battle Network 5: Team Colonel (USA): the addresses the guest core uses
 * (guest.c; docs/ROM_DATA.md, BN5 guest battles). BN5 shares BN6's engine:
 * its toolkit, game state and battle records are laid out as BN6's are. */
#ifndef CW_BN5_H
#define CW_BN5_H

#define BN5_TOOLKIT       0x0200A440u /* eToolkit: +0 points at the main mode's index (4 the game, 0x14 its GAME OVER) */
#define BN5_TOOLKIT_GAMESTATE 0x3C    /* eToolkit +0x3C: the game state */
#define BN5_GAMESTATE     0x02002940u /* +0 the sub-mode: 4 on the map, 8 a battle beginning, 0xC in battle; +0x1C the battle's record */
#define BN5_ROLL          0x0810F6B4u /* the encounter roll the check calls each frame on a net map: returns a BattleSettings* (0 none) */
#define BN5_BATTLE_TABLES 0x0801C968u /* two pointers, the real world's map groups and the net's (group - 0x80): per group a pointer per map,
                                       * per map 16-byte BattleSettings records, byte 0 0xFF ending them, +7 a condition */

#define BN5_MODE_GAME      0x04
#define BN5_MODE_GAME_OVER 0x14
#define BN5_SUB_MAP        0x04
#define BN5_SUB_BATTLE_INIT 0x08
#define BN5_SUB_BATTLE     0x0C

#endif
