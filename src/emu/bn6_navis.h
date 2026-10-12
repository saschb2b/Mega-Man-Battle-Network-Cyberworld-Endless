/* The battle engine's Navis in BN6 Cybeast Gregar (USA), part of bn6.h's
 * addresses (it includes this): the tables a Navi is found through by its
 * AI index (AIData +1 of an enemy whose id's triple names actor type 1),
 * the engine's routines those tables name, and what a Navi draws by (its
 * arm points, its companion's overlay); what src/emu/xnavi_room.c grows
 * and points elsewhere and src/emu/xnavi_install.c fills, to move another
 * game's Navis into BN6's own battles (issue #82; docs/MULTIROM.md,
 * Transplanted Navis; docs/ROM_DATA.md, The Navis' tables). */
#ifndef CW_BN6_NAVIS_H
#define CW_BN6_NAVIS_H

/* A Navi runs as the first kind's type 0 (BN6_T1_TYPES) by its actor type
 * (the id's triple +1: 1 a Navi): BN6_NAVI_OBJECT's init, per frame and
 * end. Its per frame (BN6_NAVI_FRAME) calls, by its AI index, its pre-step
 * (the status step and the engine's chores), hook A, its state through
 * the shared status dispatcher (0x0801AF44), then its post hook */
#define BN6_NAVI_OBJECT       0x080F3658u /* the Navis' object routine (the type 0 triple's Navi word at 0x080B9A78): its init 0x08016F57, */
#define BN6_NAVI_FRAME        0x080F367Cu /* ... its per frame, which reads the tables below by AIData +1, */
#define BN6_NAVIS             25          /* ... AI indexes 0 (a MegaMan of the story's) to 24 (FBeast); 17 and 22 empty (AI 0's entries) */
#define BN6_NAVI_TABLES       0x080F36D4u /* the thirteen per-AI tables, BN6_NAVI_TABLE_STEP apart, BN6_NAVIS words each, in this order: */
#define BN6_NAVI_TABLE_STEP   0x64        /* ... from one to the next */
#define BN6_NAVI_TABLE_COUNT  13          /* ... 0 the state tables (a Thumb pointer per CurAction, generic 0-7, its own from 8), 1 the pre-step,
                                             2 hook A, 3 struct1, 4 struct2, 5 the post hook, 6 spawn start, 7 spawn end, 8 paralysed, 9 frozen,
                                             10 bubbled, 11 flinching, 12 dragged; each named by one word (a literal, or a triple's Navi word): */
#define BN6_NAVI_REF_STATES   0x080F36C8u /* ... table 0's (BN6_NAVI_FRAME's literals 0x080F36C4-0x080F36D0) */
#define BN6_NAVI_REF_PRESTEP  0x080F36C4u /* ... table 1's */
#define BN6_NAVI_REF_HOOK_A   0x080F36CCu /* ... table 2's (BN6's alone: BN5's per frame has no hook between its pre-step and its state) */
#define BN6_NAVI_REF_STRUCT1  0x0800F234u /* ... table 3's: the struct1 triple's Navi word (its virus word RomLayout.enemy_traits) */
#define BN6_NAVI_REF_STRUCT2  0x0800F264u /* ... table 4's: the struct2 triple's (RomLayout.enemy_stats) */
#define BN6_NAVI_REF_POST     0x080F36D0u /* ... table 5's */
#define BN6_NAVI_REF_SPAWN    0x0800F370u /* ... table 6's, the Navi word of BN6_FAMILY_REF_SPAWN's triple; and so 7-12: */
#define BN6_NAVI_REF_SPAWNED  0x0800F38Cu /* ... table 7's (spawn end) */
#define BN6_NAVI_REF_PARALYSIS 0x0800F3A8u /* ... table 8's (paralysed) */
#define BN6_NAVI_REF_FREEZE   0x0800F3C4u /* (BN6's alone) */
#define BN6_NAVI_REF_BUBBLE   0x0800F3E0u /* (BN6's alone) */
#define BN6_NAVI_REF_FLINCH   0x0800F3FCu /* ... table 11's (flinching) */
#define BN6_NAVI_REF_DRAG     0x0800F418u /* ... table 12's (dragged) */
#define BN6_NAVI_NULLSUB      0x080F3BE9u /* the Navis' empty hook: mov pc, lr (Thumb) */
/* The engine's generic states a Navi's state table names (0-7) */
#define BN6_NAVI_SPAWN_STATE  0x08016381u /* 0: spawned (the viruses' too) */
#define BN6_NAVI_DECIDE       0x08017889u /* 1: on to its first own state, CurAction 8 (an actor of type 2 first its link step) */
#define BN6_NAVI_DYING        0x080170C5u /* 2: deleted */
#define BN6_NAVI_FLINCH       0x080174FFu /* 3: flinching */
#define BN6_NAVI_PARALYSED    0x080175B9u /* 4: paralysed */
#define BN6_NAVI_DRAGGED      0x080178B7u /* 5: dragged */
#define BN6_NAVI_FROZEN       0x08017689u /* 6: frozen (BN6's alone) */
#define BN6_NAVI_BUBBLED      0x08017769u /* 7: bubbled (BN6's alone) */
/* Their pre-steps (table 1): the status step and the chores of a frame, four variants */
#define BN6_NAVI_PRESTEP_A    0x0801A9B9u /* the Navis' own (NameIDs 0x125-0x12A, TomahawkMan's, skip a step) */
#define BN6_NAVI_PRESTEP_B    0x0801AA49u /* no knockback (ChargeMan's) */
#define BN6_NAVI_PRESTEP_C    0x0801AAC1u /* AI 12-17's: a step skipped in battle mode 10 */
#define BN6_NAVI_PRESTEP_D    0x0801AB41u /* the bosses' (Gregar's, Falzar's) */
/* The triples by actor type (virus, Navi, player words: the player's names the Navis' table too, read by player AI to 0x30): their
 * Navi words, each moved alone to a grown copy */
#define BN6_NAVI_ICE_REF      0x080EAF68u /* the ice block's size (BN6_ICE_SIZES_REF's triple): the Navis' bytes at 0x080EAF8E, a byte an AI */
#define BN6_NAVI_HELD_REF     0x080B05E8u /* the held chips' (BN6_HELD_CHIPS_REF's triple): the Navis' lists' words at 0x080B067C */
#define BN6_NAVI_COMPANION_REF 0x08010E10u /* the routine a Navi's companion begins by (BN6_AI_HOOKS_REF's triple): words at 0x08010EA4 */
#define BN6_NAVI_COMPANION_END_REF 0x08011060u /* ... the one it ends by (BN6_AI_HOOKS2_REF's): words at 0x080110F4 */
#define BN6_NAVI_HIDE_REF     0x0801146Cu /* the routine an attack of a Navi's hides its companion's overlay by, a word an AI (BN6_NAVIS):
                                             words at 0x08011470, read by AIData +1 for an actor type not 0 (0x08011450, from attack
                                             families' first steps; BN5's 0x0800F114, its words BN5_NAVI_HIDE) */
#define BN6_OVERLAY_HIDE      0x080C5D42u /* ... the overlay's hide (BN5's BN5_OVERLAY_HIDE), the other entries the empty companion's */
/* What a Navi draws by */
#define BN6_ARM_POINTS        0x08018810u /* an object's arm point: r0 its NameID (0xCD-0xFF none), r1 the point, r2 its alliance, r3 its flip; the
                                             record by its struct1's list and number (0x08018886): list 0 through the bytes at 0x080188B0 */
#define BN6_ARM_LISTS         0x080188A0u /* ... the records' tables by struct1's list offset (0, 4, 8): MegaMan's, the viruses', the Navis' */
#define BN6_ARM_NAVIS_REF     0x080188A8u /* ... the Navis' table's word (list 2's numbers): 0x080191C8, */
#define BN6_ARM_NAVIS_COUNT   36          /* ... its records, BN6_ARM_RECORD bytes each: */
#define BN6_ARM_RECORD        0x44        /* ... 34 points (x, y signed bytes); BN5's records 30 (0x3C): BN6's own 30-33 read by the fourth kind's
                                             0x3C (30), the third's 0x95 (31), the engine's 0x08010820 (32) and a freeze's ice block, the fourth
                                             kind's 0x89 (33) */
#define BN6_ARM_POINTS_BN5    30          /* ... the points BN5's records hold, the first BN6's too */
#define BN6_MEGAMAN_ARMS      0x080188C0u /* the records of list 0 (MegaMan's forms): record 0, MegaMan's own */
#define BN6_OVERLAY_MASKS_REF 0x080C5C30u /* the companion overlay (the first kind's type 0x56, its init 0x080C5BD8) by its kind (+4): the word naming
                                             its animations' masks (a word a kind, at 0x080C5B44), */
#define BN6_OVERLAY_SPRITES_REF 0x080C5C2Cu /* ... and its sprites' (two bytes a kind, list offset and number, at 0x080C5B90): */
#define BN6_OVERLAY_KINDS     19          /* ... its kinds */
#define BN6_NAVI_NAMES_REF    0x0800ECF0u /* BN6_NAME_OF's word naming the Navis' names (RomLayout.enemy_names[1]): an id past 0xFF its entry id & 0xFF, */
#define BN6_NAVI_NAMES_COUNT  184         /* ... its entries (to 0x1B7); its other reader 0x08139B70 (a list of the story's) */
#define BN6_NAME_OF_NAVIS     0x0800EC5Eu /* BN6_NAME_OF once its archive is chosen (movs r1,#0xFF): r0 the id, r2 the archive (the Navis' past 0xFF) */
/* An enemy spawned mid-battle (r0 its id, r1 and r2 its panel, r3 its side): BN5_CHAOS_FAIL_SPAWN's twin, the Souls
 * track's, the same body with its two entries in the other order */
#define BN6_SPAWN_ENEMY_1     0x08007696u /* ... its +5 and AIData +2 1 (BN5's 0x08006AAE), */
#define BN6_SPAWN_ENEMY_0     0x080076A0u /* ... 0 (BN5's 0x08006AA4) */
/* BN5's Colonel's table moved (issue #82's RAM plan, RAM_COLONEL): BN5 keeps two 8-byte entries by alliance at 0x02034000 */
#define BN6_BATTLE_BITS       0x5C        /* BattleState +0x5C (bits as BN5's but none a Colonel's: 1, 2, 4 and 8 the sides', 0x10 the field's
                                             slide as a battle ends, BN5's 0x40: set at 0x080E19CA, read at 0x08001A7E) */
#define BN6_COLONEL_SIDES     0x0203FDA0u /* the moved Colonel's sides' bits, as BN5 keeps them in BN5_BATTLE_BITS (0x10 side 0, 0x20 side 1):
                                             his write aimed here (XN_STUB_SIDES), BN6's own bits left alone; zeroed with the battle's RAM */
#define BN6_COLONEL_TABLE     0x0203FF60u /* the moved Colonel's two entries, zeroed with the battle's RAM as each battle is set up (0x080071F2) */
/* What BN5's engine does for the Dark Soul by its id (BN5_DARK_SOUL_ID) that BN6's twins do for no id: xnavi_dark.c makes it
 * for the moved one's */
#define BN6_NAVI_REGION       0x08016FE6u /* the actor's init (0x08016F57): movs r3,#3, before its collision set up (0x08019FB4) with r1
                                             its region (1, or 0x10 where its id's record's +3 has 4) and r2 2 (BN5's twin 0x08013C70,
                                             after BN5_DARK_REGION) */
#define BN6_NAVI_PLATE        0x08016594u /* its arrival into a fight under way (AIData +2 not 0, 0x08016380: the generic substate 0x0801657F):
                                             movs r0,#0, the call 0x0801DC7C(0, 0) that lists a foe and its HP, to BN6_NAVI_PLATE_END (BN5
                                             skips it for the Dark Soul: BN5_DARK_PLATE; the opening's arrival, 0x08016461, lists it in both) */
#define BN6_NAVI_PLATE_END    0x0801659Cu
#define BN6_DARK_REGION       0x13        /* the region BN5 gives the Dark Soul's collision (its +0x30 0x01010000, BN6's 0x01010080) */
#define BN6_DARK_LISTS        0x0203FBE0u /* the moved chip AI's lists (BN5_DARK_LISTS, laid out as BN5's, BN6's chip ids), two sides,
                                             zeroed with the battle's RAM (0 entries: none) */
#define BN6_DARK_AI           0x0203FDB0u /* the moved Dark Soul's AI data, BN5_AI_DATA_SIZE bytes (its chip AI's state from +0xF0 to +0x13F,
                                             past BN6's 0x100): its +0x58 pointed here as it is set up (xnavi_dark.c), its pool's 0x100
                                             bytes copied first; */
#define BN6_DARK_AI_OWNER     0x0203FDA4u /* ... the object it is now, 0 none; */
#define BN6_DARK_AI_POOL      0x0203FDA8u /* ... that object's own AI data in the pool, which a Dark Soul still out takes back as another
                                             is set up (BN5's failed Chaos Unison ends the one out first, 0x0810436C) */

#endif
