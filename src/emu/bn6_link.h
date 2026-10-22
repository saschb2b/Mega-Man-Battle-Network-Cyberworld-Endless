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
/* A link battle's packet, one a frame each way (docs/ONLINE_ARENA.md, The partners' words in BN6's packet; bn6f sub_801FF18,
 * sub_803EE98) */
#define BN6_LINK_OUT          0x02036780u /* this GBA's packet, eight u16 the battle writes each frame (bn6f eStruct2036780): +0 the
                                             mark (the pad's high byte, 0xFC) and +1 its sequence (the link's frame, 0x0803EE6C), +2 the
                                             pad held (BN6_KEYS_VALID set), +4 the word stream's index (0xFF none) and +8 its word, +6
                                             the battle's ready flags, +0xC the link's state, +0xE and +0xF the library's status */
#define BN6_LINK_IN           0x020399F0u /* both sides' packets of the exchange the link took this frame, BN6_LINK_PACKET apart, side
                                             0's first, every GBA alike; the battle takes each side's pad from its +2 where its mark is
                                             set (0x0801FF2A) */
#define BN6_LINK_PACKET       0x10
#define BN6_LINK_PAD          0x02
#define BN6_LINK_SPARE_LO     0x05        /* a packet's two bytes nothing of BN6's writes or reads (+4's and +6's other halves: every
                                             writer of BN6_LINK_OUT and reader of BN6_LINK_IN stores and loads those two as bytes) */
#define BN6_LINK_SPARE_HI     0x07
#define BN6_LINK_PAD_STORE    0x0801FF8Cu /* strh r1,[r5,#2]: sub_801FF18's store of this frame's pad into BN6_LINK_OUT (r5), both
                                             sides' packets read before it (0x0801FF2A-0x0801FF82) */
#define BN6_LINK_PAD_STORE_B  0x08020062u /* ... the same in sub_801FFD6, the other of the battle's link routines (0x0801FEEE's table),
                                             its argument at [sp,#4]: bit 3 set, the packets not read this frame */
/* A Navi's battle NaviStats (BN6_BATTLE_NAVI, BN6_BATTLE_NAVI_SIZE a side): every reader and writer takes its side's */
#define BN6_NAVI_STATS_ADDR   0x08013682u /* r0 the side: its battle NaviStats' address in r0, eBattleNaviStats0 in r1, by mov pc,lr, no push
                                             (bn6f GetBattleNaviStatsAddr) */
#define BN6_NAVI_BY_OBJECT    0x08013754u /* the five routines that call it with the side of the object in r5 (its BN6_T1_ALLIANCE) and
                                             read or write a field r1 bytes in: set a byte, set a halfword, get a byte, the same again, get
                                             a halfword (bn6f's *_AllianceFromBattleObject), to: */
#define BN6_NAVI_BY_OBJECT_END 0x0801379Eu /* ... the last one's end */
#define BN6_NS_ATTACK         0x01        /* u8, a NaviStats' buster: Attack, Speed and Charge, levels less one (bn6.h's BN6_NAVI_ATTACK's, */
#define BN6_NS_SPEED          0x02        /* ... the menu's NaviStats laid out alike; a battle reads its copy's: a shot's damage Attack + 1, */
#define BN6_NS_CHARGE         0x03        /* ... bn6f sub_801265A) */
#define BN6_NS_BASE_HP        0x3E        /* u16, a NaviStats' fields from its start: MaxBaseHP, */
#define BN6_NS_CUR_HP         0x40        /* ... CurHP (a MegaMan's object takes it as he is set up, bn6f sub_80141C8), */
#define BN6_NS_MAX_HP         0x42        /* ... and MaxHP */
#define BN6_HAND_ADDR         0x08010018u /* r0 the side: its hand's address (BN6_BATTLE_HAND, 0x50 a side) in r0, BN6_BATTLE_HAND in r1, by
                                             mov pc,lr, no push (bn6f getBattleHandAddr_8010018): the chip up next, a chip used, the
                                             Custom screen's picks; its callers pass a side, most of them the object's in r5 */
/* A hand's fields past bn6.h's (BN6_HAND_CHIPS, POWERS, BONUS: six u16 each), as the Custom screen writes a side's (bn6f sub_8029110,
 * the hand built at 0x020366C0 from its picks): */
#define BN6_HAND_SIZE         0x50        /* a side's hand's bytes */
#define BN6_HAND_EXTRA        0x26        /* ... six u16 the Custom screen zeroes */
#define BN6_HAND_ENTRIES      0x32        /* ... six u16, each chip's folder entry (chip | code << 9), 0xFFFF after them */
#define BN6_HAND_SCREEN       0x3E        /* ... six u8, each chip's Custom screen (BN6_BATTLE_SCREENS less one) */
#define BN6_HAND_MARKS        0x44        /* ... six u8, each chip's marks (its Custom screen element's +4; WhiCapsl's and Uninstll's bits) */
#define BN6_SPAWN_COMMON      0x08003278u /* every kind's spawn (BN6_SPAWN_1 and the rest): r0 the kind, r5 the object that spawns */
#define BN6_SPAWN_MADE        0x080032BEu /* ... the new object in r5, its words taken (its first store, of its index) */
#define BN6_T4_ACTIVE         0x02036710u /* u32 the fourth kind's slots in play (BN6_T4_OBJECTS), slot i bit 31 - i: a spawn takes the
                                             first clear one, an object let go clears its own, the battle's setup clears them all with the
                                             pool (bn6f eActiveT4BattleObjectsBitfield, the kind's record at 0x08003310) */
/* A partner's whole MegaMan (docs/ONLINE_ARENA.md, His NaviCust, compiled by BN6; His Cross) */
#define BN6_NAVICUST_RELOAD   0x0813E18Cu /* the PET's MegaMan's stats made again from the NaviCust's board (bn6f
                                             reloadCurNaviStatBoosts_813c3ac, Falzar 0x0813C3AC): his NaviStats (BN6_NAVI_STATS) put back to
                                             his own, each program's routine run over them (0x0813E464), the bugs, his HP by the map; no
                                             arguments, called from the menus (the NaviCust's RUN, a map's load) */
#define BN6_NS_CUSTOM         0x0A        /* u8 a NaviStats' Custom level: the chips a Custom screen deals (5, Custom1 6; 0x0802A408 reads
                                             the side's into BN6_HAND_SIZE_SET) */
#define BN6_NS_FLOAT_SHOE     0x1B        /* u8 a NaviStats' abilities the NaviCust's programs set (bn6f's NaviStats): FloatShoe, */
#define BN6_NS_AIR_SHOE       0x1C        /* ... AirShoe, */
#define BN6_NS_UNDER_SHIRT    0x1D        /* ... UnderShirt, */
#define BN6_NS_SUPER_ARMOR    0x23        /* ... SuperArmor (1 each where its program runs) */
#define BN6_NS_START_FORM     0x17        /* u8 a NaviStats' form he began the battle in, which CROSSSELECT leaves out (0x08029EF6); a link
                                             battle's start sets it in its packet (0xC, bn6f sub_800B144) */
#define BN6_NS_BEAST_TURNS    0x21        /* u8 a NaviStats' BeastOutCounter: 3 as a link battle with Beast Out starts (sub_800B144) */
/* BN6's crossing (bn6f dword_20367F0's controller, read in Gregar's code: its routines at bn6f's addresses) */
#define BN6_CROSS_SIDES       0x020367F0u /* the crossing controller: +0 its mode (0, 4, 8), +4 1 while it runs, then each side's record
                                             (BN6_CROSS_RECORD's layout, +8 the side's player's object) at +8 and +0x18, copied from the
                                             turn's packets with BN6_CROSS_PENDING (0x080147E4); only those two objects cross */
#define BN6_TURN_FORMS_SET    0x080148C0u /* movs r0,#8 at the end of BN6_TURN_FORMS: r4 1 where a side's record holds a Cross (the
                                             controller's next mode 4, the crossing; else 8) */
#define BN6_CROSS_ACTIVATE    0x08014944u /* push {lr}: the crossing's step 4 (bn6f activateCrossHappensHere_8014944, r5 the controller,
                                             +3 0 its first frame): each record's object that holds a Cross asked to cross (0x0801596E:
                                             BN6_AI_FLAGS | BN6_AI_CROSS_ASK), then waited for until none crosses (0x0801597C) */
#define BN6_CROSS_PENDING_GET 0x0801595Eu /* r0 a side: its pending record's form in r0, +1 in r1, +3 in r3 (mov pc,lr, no push), asked by
                                             the crossing's phases with the crossing object in r5 (0x08014A38 and the phases after it) */
/* A Program Advance on the Custom screen (OK's routine 0x08029110) */
#define BN6_CUSTOM_NEXT       0x020364CBu /* u8 the Custom screen's +0x0B, the mode OK goes on to once its show is over: BN6_MODE_WAIT (0x08029112),
                                             BN6_MODE_PA where its check (0x08029520) made a Program Advance (0x080291DE) */
#define BN6_MODE_PA           0x10        /* BN6_CUSTOM_MODE: a Program Advance shown, its chips joined, before BN6_MODE_WAIT */
#define BN6_CUSTOM_BUILT      0x020366C0u /* the hand OK builds from the picks (bn6f byte_20366C0, a hand's layout: BN6_HAND_CHIPS ...),
                                             the Program Advance in it once shown; a link battle's packet carries it (BN6_LINK_SEND) */
#define BN6_CROSS_SIDE_RECORD 0x08        /* the controller's side 0's record (BN6_CROSS_SIDES'), side 1's BN6_CROSS_PENDING_SIZE on */
#define BN6_CROSS_STEP        0x03      /* the controller's byte +3 (BN6_CROSS_SIDES'): its step's count, 0 a step's first frame */
#define BN6_BEAST_COMMIT      0x08029344u /* Beast Out's commit at OK (the emblem's pick): its form into BN6_CROSS_RECORD by 0x08015952 as
                                             CROSSSELECT's (0xB, a Cross's from 0xD, BeastOver 0x17), so it crosses as a Cross does */
#define BN6_FORM_TURN_DOWN    0x08015A16u /* r0 an object: his AI data's +0x0F counted down (neither 0 nor 0xFF, where his NaviStats' Navi
                                             is MegaMan), called as each Custom screen opens for each side's player alone (0x0800846E,
                                             0x08008478: the side-player getter 0x080103BC) */
#define BN6_FORM_END_CHECK    0x080159C6u /* r0 an object: true where he is in a Beast Out form (0xB-0x18) with its BeastOutCounter 0 (his
                                             AI data's +0x0F at 0 set to 2 on the way), asked in the turn's form check for the controller's
                                             records' objects alone; the end then asked by 0x08015994 */
#define BN6_FORM_TURN_DOWN_BODY 0x08015A1Cu /* ... its body past the object's test (movs r1,#0x29), r5 the object, */
#define BN6_FORM_TURN_DOWN_END 0x08015A36u /* ... its pop {r5,pc} */
#define BN6_BEAST_TURN        0x08015A38u /* r0 an object: his BeastOutCounter less one where he is in a Beast Out form (MegaMan, not begun
                                             in one), called as the crossing controller is done each turn for each side's player alone
                                             (0x0800840C on the side-player getter 0x080103BC); */
#define BN6_BEAST_TURN_BODY   0x08015A40u /* ... its body past the object's test (movs r1,#0x29), r5 the object, r7 0, */
#define BN6_BEAST_TURN_END    0x08015A7Eu /* ... its pop {r5,r7,pc} */
#define BN6_FORM_LEAVE_ASK    0x08015994u /* r0 an object: asked to leave his form (BN6_AI_FLAGS | BN6_AI_LEAVE_ASK) */
#define BN6_CROSS_RESTORE     0x08014A00u /* push {lr}: the crossing controller's mode 8 (bn6f sub_8014A00), reached by mov lr,pc; bx r1:
                                             it waits while a record's object leaves his form (0x080159A2: BN6_AI_LEAVING or
                                             BN6_AI_LEAVE_ASK), then ends the controller */
#define BN6_AI_LEAVE_ASK      0x40u       /* BN6_AI_FLAGS: asked to leave his form (0x08015994) */
#define BN6_AI_LEAVING        0x100u      /* BN6_AI_MODES: leaving his form (0x080159A2 waits while it is set) */
#define BN6_TURN_SIDES        0x0203C970u /* the controller set up as each Custom screen opens (0x0802D6A0): +4 1 while it runs, +8 and +0xC
                                             each side's player (0x080103BC), for whom alone it asks 0x0802DD1E, which answers 0 in Gregar
                                             (movs r7,#0 ... adds r0,r7,#0), so that its 0x0802DD10 never runs */
#define BN6_AI_CROSS_ASK      0x4000u    /* BN6_AI_FLAGS: asked to cross into his pending form (0x0801596E) */
#define BN6_AI_CROSSING       0x80u       /* BN6_AI_MODES: a crossing under way (0x0801597C waits while it is set) */

#endif
