/* Battle Network 5: Team Colonel (USA): the Liberation board's commands, per
 * Navi (docs/LIBERATION.md, The Navis; docs/ROM_DATA.md, BN5's Liberation
 * board: the commands). Both versions share the code: Team Colonel's ROM
 * holds Team ProtoMan's commands whole (masks, routines, shapes, texts),
 * played in romlab with a unit's Navi id poked. Split from bn5_liberation.h,
 * which includes it. */
#ifndef CW_BN5_LIBERATION_CMDS_H
#define CW_BN5_LIBERATION_CMDS_H

/* A command's id: its bit in a Navi's mask, its entry of BN5_LIB_TEXT (its name) and its index in every table below */
#define BN5_LIB_CMD_LIBERATION 0         /* Liberation */
#define BN5_LIB_CMD_TWIN     1           /* TwinLiberation (Meddy); entry 2 the same name, in no Navi's mask */
#define BN5_LIB_CMD_LIFEMELODY 3         /* LifeMelody (ToadMan) */
#define BN5_LIB_CMD_BOMB     4           /* BombLiberation (GyroMan) */
#define BN5_LIB_CMD_SNEAK    5           /* SneakAttack (ShadowMan) */
#define BN5_LIB_CMD_SEARCH   6           /* PanelSearch (SearchMan) */
#define BN5_LIB_CMD_NUMBER   7           /* NumberCheck (NumberMan) */
#define BN5_LIB_CMD_NAPALM   8           /* Napalm (NapalmMan) */
#define BN5_LIB_CMD_TOMAHAWK 9           /* TomahawkSwing (TomahawkMan) */
#define BN5_LIB_CMD_MAGNET   10          /* MagnetBarrier (MagnetMan) */
#define BN5_LIB_CMD_LONGSWRD 11          /* LongSwrd (MegaMan) */
#define BN5_LIB_CMD_WIDESWRD 12          /* WideSwrd (ProtoMan) */
#define BN5_LIB_CMD_SCRENDIV 13          /* ScrenDiv (Colonel) */
#define BN5_LIB_CMD_SAVE     14          /* Save (ProtoMan, Meddy, Colonel, ToadMan) */
#define BN5_LIB_CMD_PASS     15          /* Pass */

/* Team ProtoMan's Navis (a board unit's +1, BN5_LIB_NAVI_HP's ids; their sprites, texts and masks all in this ROM) */
#define BN5_LIB_NAVI_PROTOMAN  0x01      /* ProtoMan: WideSwrd, Save */
#define BN5_LIB_NAVI_GYROMAN   0x02      /* GyroMan: BombLiberation */
#define BN5_LIB_NAVI_SEARCHMAN 0x03      /* SearchMan: PanelSearch */
#define BN5_LIB_NAVI_NAPALMMAN 0x04      /* NapalmMan: Napalm */
#define BN5_LIB_NAVI_MAGNETMAN 0x05      /* MagnetMan: MagnetBarrier */
#define BN5_LIB_NAVI_MEDDY     0x06      /* Meddy: TwinLiberation, Save */
#define BN5_LIB_UNIT_NAVI    0x01        /* a board unit's u8: its Navi id or enemy kind (+0: 1 a teammate, 2 an enemy) */
#define BN5_LIB_UNIT_FACING  0x05        /* ... its facing, 0-7 by 45 degrees: 1 toward x + 1, 3 y + 1, 5 x - 1, 7 y - 1 */
#define BN5_LIB_UNIT_FLAGS   0x0E        /* ... its flags: 0x40 MagnetBarrier's, 0x80 a BonusPanel's shield, 8 LifeMelody's (slot 4) */

/* The window */
#define BN5_LIB_CMD_MASKS    0x08050048u /* u16 per Navi id (13): his commands, bit n command n, read by 0x080556B6 */
#define BN5_LIB_CMD_WINDOW   0x0200A7D0u /* the window: +0 the command in hand, +4 the cursor, +8 the count, +0xC the mask, +0xE the usable */
#define BN5_LIB_CMD_OPEN     0x080552A4u /* the window drawn from r0 (the mask) and r1 (the usable, the rest greyed), bit by bit */
#define BN5_LIB_CMD_CHECK    0x0805571Cu /* as it opens, each of the Navi's commands checked by its routine in BN5_LIB_CMD_CHECKS: */
#define BN5_LIB_CMD_CHECKS   0x08055778u /* ... a u32 a command; each usable one sets its target and its panels: */
#define BN5_LIB_CMD_TARGETS  0x0200AAD0u /* ... 4 bytes a command (+0 1 usable, +1 the command, +2 u16 its panel, y << 8 | x), */
#define BN5_LIB_CMD_PANELS   0x0200FDB0u /* ... 32 bytes a command, its panels (u16 each, 0 ends) */

/* The command chosen */
#define BN5_LIB_CMD_STEPS    0x0804C428u /* a byte per command: the player's phase's step it goes on to (BN5_LIB_STATE +1), */
#define BN5_LIB_PHASE_ROUTINES 0x0804BF5Cu /* ... whose routine is this table's u32 (182 steps): Napalm 0x7C, MagnetBarrier 0x60, */
                                         /* ... BombLiberation 0x76, PanelSearch 0x97, the three swords 0xA8 */
#define BN5_LIB_CMD_BATTLE   0x10        /* ... the step a command's battle starts from, Liberation's (its point spent before) */
#define BN5_LIB_CMD_AFTER    0x0804D308u /* a byte per command: its step after its battle (0x0804D28C), 0 for one with none */
#define BN5_LIB_CMD_FREE     0x0804C57Cu /* the command's panels (BN5_LIB_CMD_PANELS) freed, after a won battle or BombLiberation */

/* The shapes: u8 w, u8 h, then four facings of w x h bytes; cell (c, r) the panel (x + c - w / 2, y + r - h / 2) of the Navi's own */
#define BN5_LIB_CMD_SHAPES   0x08050344u /* per Navi id nine u32, a shape each (a slot's offset its index x 4): */
#define BN5_LIB_SHAPE_SLOT   0x14        /* ... his command's (Liberation's the faced panel alone at slot 0, */
#define BN5_LIB_SHAPE_SLOT_ONE_TURN 0x18 /* ... 8 its eight round him on a one-turn win), this a sword's after a one-turn win */
#define BN5_LIB_CMD_APPLY    0x08052060u /* a shape applied: the Navi's panel r0, the faced r1, the slot r2, the kinds' values r3 */
#define BN5_LIB_CMD_FACING   0x08051BDAu /* ... its facing, r0 to r1: 0 y - 1, 1 x + 1, 2 y + 1, 3 x - 1 */
#define BN5_LIB_CMD_KINDS    0x080502C4u /* ... a panel kind's value (dark and item 1), 8 where an enemy stands (0x08052E10); */
                                         /* ... a cell takes a panel whose value is 1 to its own: the faced cell's 8, the rest 1 */
#define BN5_LIB_SHAPE_LONGSWRD 0x08050564u /* LongSwrd: the faced panel and the one beyond */
#define BN5_LIB_SHAPE_WIDESWRD 0x08050630u /* WideSwrd and ScrenDiv: the faced panel and its two sides */
#define BN5_LIB_SHAPE_TOMAHAWK 0x080506FCu /* TomahawkSwing and NumberCheck: two deep, three wide */
#define BN5_LIB_SHAPE_NAPALM 0x08050762u /* Napalm: four ahead, and the two beside the third */

/* PanelSearch (steps 0x97-0x9D) */
#define BN5_LIB_SEARCH_LINE  0x0805221Cu /* the line: from the faced panel on, away from him, while dark or item, no enemy on it */
#define BN5_LIB_SEARCH_KINDS 0x080502D4u /* ... (these values 1), at most BN5_LIB_SEARCH_MAX; steps of 0x08050D40 by the facing */
#define BN5_LIB_SEARCH_MAX   10
#define BN5_LIB_SEARCH_KEEP  0x080522CEu /* ... cut to its item and trap panels; none: no battle, the point spent */

/* MagnetBarrier (steps 0x60-0x63) and BombLiberation (0x76-0x7B), neither with a battle */
#define BN5_LIB_BARRIER_ON   0x08054518u /* each teammate's BN5_LIB_UNIT_FLAGS gains BN5_LIB_BARRIER: harm to him 0 (0x08053D32), */
#define BN5_LIB_BARRIER_OFF  0x0805455Cu /* ... till the Darkloids' phase's last step (0x0804E85A) clears it */
#define BN5_LIB_BARRIER      0x40
#define BN5_LIB_WALK_MASKS   0x0804FF5Cu /* u16 per Navi id, a unit's +0x0C (the kinds he walks): GyroMan's dark, item, BonusPanel */
#define BN5_LIB_BOMB_KINDS   0x080517E4u /* BombLiberation: the panel beneath him, a dark one (kind 1 alone), freed */

#endif
