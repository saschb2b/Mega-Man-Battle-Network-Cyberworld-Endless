/* The battle engine's Navis in BN6 Cybeast Gregar (USA), part of bn6.h's
 * addresses (it includes this): the tables a Navi is found through by its
 * AI index (AIData +1 of an enemy whose id's triple names actor type 1),
 * the engine's routines those tables name, and what a Navi draws by (its
 * arm points, its companion's overlay); what src/emu/xnavi_room.c grows
 * and points elsewhere and src/emu/xnavi_install.c fills, to move another
 * game's Navis into BN6's own battles (issue #82; docs/multirom/navis.md,
 * Transplanted Navis; docs/rom/navis.md, The Navis' tables). */
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
#define BN6_COLONEL_SIDES     0x0203FDA0u /* a Colonel's sides' bits, as BN5 keeps them in BN5_BATTLE_BITS (0x10 side 0, 0x20 side 1): the moved
                                             Navi's write aimed here (XN_STUB_SIDES), ColonelSoul's ready's too (xstrike.c), the obstacle strike
                                             reading them; BN6's own bits left alone; zeroed with the battle's RAM */
#define BN6_COLONEL_SIDE0     0x10u       /* ... side 0's bit, side 1's the next */
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

/* What counts an enemy, and how a Navi is deleted: what a Navi swapped for
 * another mid-battle stands on (src/emu/battle_swap.c; docs/rom/navis.md, A
 * Navi swapped mid-battle). BattleState (BN6_BATTLE_STATE) +4 and +5 count
 * each side's enemies of the battle's start (the spawn's 0 entry raises
 * them, 0x080077A0); an enemy's destroy lowers its side's (0x0800A104),
 * and the fight's frame calls the battle won at side 1's 0 (0x0800A152).
 * BN6_STATE_SIDE1_LEFT is lowered as its dying starts, where its Param2
 * is 0 (0x0800A11C) */
#define BN6_STATE_COUNT       0x04        /* BattleState +4 + alliance, u8: the side's enemies counted, the win at side 1's 0 */
#define BN6_STATE_ALIVE       0x80        /* BattleState +0x80: the objects alive, 4 words a side (side 1's from +0x90), the spawn's first free;
                                             the targeting's and the Custom screen's names' list (0x0800A998) */
#define BN6_STATE_SIDE_STEP   0x10        /* ... a side's words in BN6_STATE_ALIVE and BN6_STATE_ROSTER: side 1's this far on */
#define BN6_STATE_ROSTER      0xD0        /* BattleState +0xD0: the alive list as the battle began (0x08007396), 4 words a side, read by the
                                             foe finders (0x080103BC, 0x080103F8) that pass a destroyed one (CurState 8) by */
#define BN6_T1_PARAM2         0x05        /* Param2: 0 where its dying lowers BN6_STATE_SIDE1_LEFT (0x0801710E), 1 from the spawn's 1 entry */
#define BN6_T1_STATE          0x08        /* CurState, u8: 0 its init, 4 updated, BN6_STATE_DESTROYED its destroy */
#define BN6_STATE_DESTROYED   0x08        /* ... its destroy (BN6_NAVI_DESTROY) */
#define BN6_T1_PHASE          0x0A        /* CurPhase, u8: a state's phase (the dying's 0, then 4) */
#define BN6_T1_PHASE_INIT     0x0B        /* PhaseInitialized, u8: not 0 once its phase began */
#define BN6_T1_TIMER          0x20        /* Timer, u16 */
#define BN6_AI_ARRIVAL        0x02        /* AIData +2, u8, the spawn's entry: 0 an enemy of the battle's start (counted, its object kept at its
                                             destroy), 1 one arriving mid-fight (its arrival BN6_NAVI_PLATE's, uncounted, freed at its destroy) */
#define BN6_AI_DIE_SPARKS     0x74        /* AIData +0x74: the dying's sparks object (the fourth kind's 0x11, 0x080E2DAA), ended at the removal */
#define BN6_OBJ_DEAD          0x100u      /* BN6_COLL_FLAGS: deleted (set as the dying begins, 0x0801AF98) */
#define BN6_NAVI_DIE          0x080170E4u /* the Navi dying's phase 0 (BN6_NAVI_DYING's first, bn6f sub_80170E4; ProtoMan's 0x080FCFB3 calls
                                             it too): its collision, companion and HP's list cleared, BN6_STATE_SIDE1_LEFT lowered where
                                             BN6_T1_PARAM2 is 0, then phase 4 */
#define BN6_NAVI_DIE_FLASH    0x08017122u /* ... its phase 4 (bn6f sub_8017122): on its first frame the sparks and an explosion, then 0x5A frames
                                             flashing white, then off its panel and the alive list, and CurState BN6_STATE_DESTROYED */
#define BN6_NAVI_DESTROY      0x08016C4Fu /* the Navi object's destroy (BN6_NAVI_OBJECT's third): BN6_STATE_COUNT lowered where BN6_AI_ARRIVAL
                                             is 0 (0x0800A104), the object freed where it is not */
#define BN6_NAVI_DYING_PROTO  0x080FCFB3u /* ProtoMan's state 2 (his state table's), BN6_NAVI_DYING's caller */
#define BN6_SPRITE_MOSAIC_AT  0x11        /* an object's sprite (BN6_OBJ_SPRITE_AT) +0x11: its OAM attribute 0's high byte, */
#define BN6_SPRITE_MOSAIC     0x10        /* ... its mosaic bit (bn6f sprite_setMosaicSize 0x08002EF6), its size the render info's: */
#define BN6_TOOLKIT_RENDER    0x08        /* eToolkit RenderInfoPtr: the frame's display registers, +2 REG_MOSAIC (its high byte the sprites') */
#define BN6_RENDER_MOSAIC     0x02        /* ... that +2: REG_MOSAIC's value, written to it each frame by render_800172C (0x08001736), its high byte the sprites' size */
#define BN6_PANEL_RESERVER    0x1C        /* a panel's (BN6_FIELD_PANELS) ReserverObjectPtr: the object standing on it, 0 none (object_reservePanel 0x0801BB1C) */
#define BN6_HP_PLATES         0x0203EB50u /* the HP under each enemy: entries listed by 0x0801DC7C (BN6_NAVI_PLATE's call), dropped as its
                                             object's HP reads 0 (0x0801C168), */
#define BN6_HP_PLATE_COUNT    4           /* ... this many, */
#define BN6_HP_PLATE_SIZE     0x14        /* ... each this long: +0 its flags (bit 0 listed), */
#define BN6_HP_PLATE_SHOWN    0x02        /* ... u16 the HP it shows, counted toward the object's a frame at a time, */
#define BN6_HP_PLATE_OBJECT   0x0C        /* ... its object */

#endif
