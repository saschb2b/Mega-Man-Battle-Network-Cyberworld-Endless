/* BN5's Team ProtoMan moved into BN6's engine (docs/multirom/navis.md, NapalmMan; docs/rom/navis.md, Team ProtoMan's
 * NapalmMan; docs/game/team-protoman.md): what their moved code's proof rests on beyond the generator's own reads, named for
 * tools/bn5_navi_twins.py (COMPANIONS), tools/xnavirecipe.py (CHIP_OBJECT_USES, EFFECT_CALLS) and tools/bn5_twins.py's
 * TYPES. None of it run: the recipes (src/emu/xnavi_bn5.c) copy the code. */
#ifndef CW_BN5_TEAM_PROTOMAN_H
#define CW_BN5_TEAM_PROTOMAN_H

/* NapalmMan (AI 4, ids 0x113-0x116; BN6's Navi AI 29, ids 0x1FC-0x1FF) */
#define BN5_NAPALM_COMPANION     0x0800EE82u /* his companion's start: BN5's overlay spawner with kind 4 (movs r4,#4 at 0x0800EE84),
                                                a kind BN6 lacks: copied with him, the kind his own (XN_KIND_OF, 33) */
#define BN5_NAPALM_COMPANION_END 0x0800F00Eu /* ... and its end, the body of KnightMan's (0x0800F000), copied with him */
#define BN5_NAPALM_HALF_HP_CHIP  0x080F5F22u /* below half his HP (+0x24 under +0x26 / 2), the chip he holds next written for his
                                                attack: BN5's "the chip an object uses" (0x0800D010) its id, a version holding none
                                                skipping the call (0x080F5F2E) */
#define BN5_NAPALM_FLAME_SPAWN   0x080CB4FCu /* his third kind's 0x33 spawns the third kind's 0x00 (movs r4,#0x11 its kind, the
                                                spawner 0x080C4ABC at 0x080CB500): the type the Navis' copy now owns for every track */
#define BN5_NAPALM_PANEL_FLAMES  0x080CC2A2u /* his third kind's 0x38's flames over a pattern of panels: movs r2,#0x4B, handed to
                                                BN5's effects-over-panels routine 0x08018838 (BN6's twin 0x0801BD3C) at 0x080CC2AA */

/* GyroMan (AI 2, ids 0x107-0x10A; BN6's Navi AI 27, ids 0x1F4-0x1F7) */
#define BN5_GYRO_COMPANION       0x0800EE46u /* his companion's start: BN5's overlay spawner with kind 0 (movs r4,#0 at 0x0800EE48,
                                                the form of Colonel's 0x0800EE56), a kind BN6 lacks: copied with him, the kind his
                                                own (XN_KIND_OF, 31; its sprite BN5's 4:0x0A, list 2's 0x84 in BN6) */
#define BN5_GYRO_COMPANION_END   0x0800EFD6u /* ... and its end, the body of Colonel's (0x0800EFE4), copied with him */

/* SearchMan (AI 3, ids 0x10D-0x110; BN6's Navi AI 28, ids 0x1F8-0x1FB): no companion of his own (BN5's empty one, BN6's
 * twin 0x080114D5); his pre-step hook and status hooks his own code */
#define BN5_SEARCH_CODE          0x080F4EC0u /* his state table, struct2 and code, to 0x080F5C80 */
#define BN5_SEARCH_PARENT_STATE  0x080E7014u /* his fourth kind's 0x5A compares its parent's CurAction with 10 (movs r1,#0xA),
                                                BN6's 12; and with 11 at 0x080E70E0, BN6's 13 */

/* Meddy (AI 6, ids 0x11F-0x122; BN6's Navi AI 31, ids 0x3E8-0x3EB): no companion of her own; her code from */
#define BN5_MEDDY_CODE           0x080F73E8u /* her state table, struct2 and code, to 0x080F7FF8 */
#define BN5_MEDDY_EFFECT_KINDS   0x080C63ACu /* her object's spawn of the fourth kind's 0x00 (0x080DFFD2 at 0x080C6358): r4 its effect 0
                                                (movs r4,#0 at 0x080C6354) with this table's byte by its kind (+4) in bits 16-23
                                                (lsls #16; orrs r4,r0), a byte a kind: 0, 4, 1, 2, 3, 3, 0, 0 */

/* MagnetMan (AI 5, ids 0x119-0x11C): his magnet panel, BN5's own (none in BN6; docs/rom/bn5-world.md, BN5's magnet panel) */
#define BN5_PANEL_MAGNET  5           /* its panel type: magnet (its flags 0x10210, BN5_PANEL_FLAGS_OF's, BN6's conveyors' word; BN6's 5 its holy) */
#define BN5_MAGNET_ARRIVED 0x0801720Eu /* what a magnet panel does to an object coming onto it (from BN5_PANEL_ARRIVED at 0x080171AC): none
                                       * where its collision data's +0x38 is not 0, its flags (+0x3C) hold 0x24, or it is the player (+0 2) whose
                                       * NaviStats byte 0x2C (0x08010DEE) is 5; else its +0x40 ORed with 0x10 (0x08016C5C; BN6's BN6_STRUCK_SLIDE)
                                       * and the object's +0x0F made 3 (the push's kind) */
#define BN5_MAGNET_LAID   0x080E24CEu /* MagnetMan's state that lays them: sound 0xFC, then type 5 (movs r2,#5 at 0x080E24EE) by
                                       * setPanelType (0x080E24F0) on each of eight panels, */
#define BN5_MAGNET_PANELS 0x080E2518u /* ... a byte pair (column, row) each: columns 1, 3, 4, 6 of rows 1 and 3 */

#endif
