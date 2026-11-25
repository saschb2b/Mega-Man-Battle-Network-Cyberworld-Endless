/* BN4 Red Sun's Souls' own code and art past bn4.h's tables by form
 * (docs/rom/bn4.md, Souls in battle; docs/multirom/bn4-souls.md): the
 * unison's picture, and what a Soul's routine as its form is taken does. */
#ifndef CW_BN4_SOULS_H
#define CW_BN4_SOULS_H

#define BN4_T4_SOUL_PICTURE 0x13      /* the fourth kind's: the unison's picture, spawned by its phase 1 (0x080EBB20, 0x080E37D8: its +4 the
                                       * pending form), its init 0x080E363C: */
#define BN4_SOUL_PICTURE_LIST 0x080E363Eu /* ... movs r1,#8: the picture's sprite list offset (list 2), */
#define BN4_SOUL_PICTURE_NUMBER 0x080E3640u /* ... ldrb r2,[r5,#4]: its number the form, so a Soul's picture is his Navi's battle
                                       * sprite (ThunSoul's list 2 number 6, 36,388 bytes; GutsSoul's 60,988 the largest) */
#define BN4_SOUL_PICTURE_DECOS 0x080E36ACu /* ... an arm object (BN4_ARM_SPAWN's kind) on the picture by form, 0xFF none: WindSoul's
                                       * 0x34, FireSoul's 0x12, NumSoul's 0x14, AquaSoul's 0x13 */
#define BN4_GUTS_ENTER    0x0800E0E4u /* GutsSoul's (form 2's): r0 his side, on to his hand's pass, */
#define BN4_GUTS_PASS     0x0800E1CAu /* ... which adds to the bonus of each chip of the side's hand from the one up next (0x0800BFEA's
                                       * hand: +0 the index, +2 the ids, their bonuses 0x24 on, +0x26: BN6's BN6_HAND_BONUS, the attack's
                                       * +6 as it is used) whose record's +9 holds 0x02 (its power shown): */
#define BN4_GUTS_PASS_KIND 0x0800E1F0u /* ... cmp r1,#0xc: of the kind none (BN4_CHIP_KIND), */
#define BN4_GUTS_PASS_KIND2 0x0800E1F6u /* ... cmp r1,#8: or panel-cracking, */
#define BN4_GUTS_PASS_ADD 0x0800E1FEu /* ... adds r1,#0x1e: 30 */
#define BN4_NUM_ENTER     0x0800E134u /* NumSoul's (form 8's): r0 his side, on to his hand's pass (0x0800E194), the same as GutsSoul's */
#define BN4_NUM_PASS_KIND 0x0800E1BAu /* ... but one kind: cmp r1,#0xc, none (BN4_CHIP_KIND), */
#define BN4_NUM_PASS_ADD  0x0800E1C0u /* ... adds r1,#0xa: 10 (BN5's NumberSoul's rule) */
/* GutsSoul's rapid buster: B pressed six times in his form (the key handler 0x0800BDE0, r4 his AI data, r6 his requests at
 * its start) asks his state 0x10, eight shots of his buster (soulrapid.c) */
#define BN4_SOUL_COMPANION 0x0800B7E8u /* r5 MegaMan, at the unison (0x080EBBBE) and as the Soul ends (0x080EBF34): his AI +0x48's
                                        * object told to end (0x080CC614, after 0x080EBBBA's and 0x080EBF30's the same), then
                                        * BN4_FORM_COMPANIONS by his form (0x0800D7CA's NaviStats +0x24) spawned where its word is not 0
                                        * (BN4_FIRE_COMPANION_SPAWN: r4 the word, its list's byte offset and number; no 0x10000, so his
                                        * animation its own) and kept there; told as his animation changes (0x0800DCE8 by NaviStats
                                        * +0x23, 0x080F1860) */
/* A Soul's chip charge (AquaSoul's, form 11; ProtSoul's, MetlSoul's and FireSoul's by the same code): */
#define BN4_POWER_EMPTY   0x0800CC2Cu /* mov pc,lr: the chip charge's power attack of forms 7, 9 and 11 (BN4_FORM_LOADOUTS +2), its charge
                                       * times (BN4_CHARGE_TIMES' row) the chip charge's alone */
#define BN4_LAVA_STEP     0x08013128u /* an actor on a panel of type 8 (lava; BN6's engine's XPANEL_LAVA, BN5's): */
#define BN4_LAVA_FIRE_FORM 0x08013142u /* ... cmp r0,#5: FireSoul's form (NaviStats +0x24), on to 0x080131B8, */
#define BN4_LAVA_FIRE_HEAL 0x080131B8u /* ... movs r0,#0x32: 50 healed (0x0800ABBA, BN6's 0x0800E2EC), the same added to his collision data's
                                        * Fire tally (+0x3E, BN6's BN6_COLL_FIRE_DAMAGE), the panel set plain (BN4_SET_PANEL_TYPE),
                                        * effect 7 at his place (movs r4,#7 at 0x080131D6) and sound 0x9A (0x080131DC) */
#define BN4_POWER_EMPTY2  0x0800CCD0u /* mov pc,lr: another, FireSoul's chip charge's (form 5's +2, power 0x66: 50 frames) */
#define BN4_CHIP_CHARGE_TEST 0x0800BC78u /* r0 the chip: 1 where his form charges it with A (r4 its kind, +7; r7 its flags, +9) */
#define BN4_AQUA_TEST_FORM 0x0800BCBAu  /* ... cmp r0,#0xb: AquaSoul's form, then not left alone (+9 bit 0), */
#define BN4_AQUA_TEST_KIND 0x0800BCC4u  /* ... cmp r4,#1: of the kind Aqua */
#define BN4_METL_TEST_FORM 0x0800BC98u  /* ... cmp r0,#9: MetlSoul's form, not left alone (+9 bit 0) and its power shown (bit 1), */
#define BN4_METL_TEST_KIND 0x0800BCA8u  /* ... cmp r4,#0xb: of the kind Break, */
#define BN4_METL_TEST_KIND2 0x0800BCACu /* ... cmp r4,#0xc: or none */
#define BN4_METL_BONUS_FORM 0x0800C486u /* BN4_CHIP_CHARGE_BONUS's cmp r2,#9: MetlSoul's, */
#define BN4_METL_BONUS_KIND 0x0800C48Cu /* ... cmp r1,#0xb, */
#define BN4_METL_BONUS_KIND2 0x0800C490u /* ... cmp r1,#0xc: those two kinds doubled (BN4_CHARGED_DOUBLE) */
#define BN4_FIRE_TEST_FORM 0x0800BCB0u  /* ... cmp r0,#5: FireSoul's form, no flag tested, */
#define BN4_FIRE_TEST_KIND 0x0800BCB4u  /* ... cmp r4,#0: of the kind Fire */
#define BN4_CHARGED_FORM  0x0800B776u   /* the chip use's (0x0800B6F8 on: r4 its flags, 8 a chip charged): cmp r0,#5, FireSoul's form, */
#define BN4_FIRE_CHARGED_CALL 0x0800B78Au /* ... where its bl to BN4_FIRE_CHARGED stands in for the chip's setup (0x0800D4B4) */
#define BN4_FIRE_CHARGED  0x0800D638u   /* FireSoul's attack for a chip he charged: family 0x19 (his flame) of 150, Fire, its hit word
                                         * 0x1E05, the chip's id 0 (tools/xchiprecipe_bn4.py SOUL_CHARGED) */
#define BN4_CHIP_CHARGE_BONUS 0x0800C47Au /* r0 the chip, r1 his form: 0x8000 to a charged chip's power (its damage doubled; BN6's twin
                                           * 0x08012C7C, BN6_CHARGE_BONUS), then sound 0x1BB (0x0800D536-0x0800D53C) */
#define BN4_AQUA_BONUS_FORM 0x0800C49Au /* ... cmp r2,#0xb: AquaSoul's, */
#define BN4_AQUA_BONUS_KIND 0x0800C4A0u /* ... cmp r1,#1: an Aqua chip */
#define BN4_CHARGED_DOUBLE 0x8000u      /* ... the bonus (ldr r4,=0x8000 at 0x0800C4A4) */
#define BN4_CHIP_BONUS    0x0800C4C4u   /* r0 the chip, r1 his form: what an uncharged chip's power takes (0x0800D544), ThunSoul's alone: */
#define BN4_THUN_BONUS_FORM 0x0800C4D0u /* ... cmp r2,#6: ThunSoul's form, its power shown and not left alone (+9 bits 1 and 0), */
#define BN4_THUN_BONUS_KIND 0x0800C4E4u /* ... cmp r1,#0xc, */
#define BN4_THUN_BONUS_KIND2 0x0800C4E8u /* ... cmp r1,#2: of those two kinds (BN4's numbering), */
#define BN4_PARALYSING    0x4000u       /* ... 0x4000 to its power (ldr r4,=0x4000 at 0x0800C4EE): the hit paralyses, the bit BN6's own
                                         * BN6_CHARGE_BONUS gives ElecCross's charged chips */
#define BN4_GUTS_RAPID_FORM 0x0800BE50u /* cmp r7,#2: his form (NaviStats +0x24) GutsSoul's, and his request 0x20 not standing
                                         * (tst r6, #0x20): then each B pressed (AI +0x1E bit 2) counts at his AI +0x12, */
#define BN4_GUTS_RAPID_WINDOW 0x0800BE68u /* ... movs r0,#0xa: its window at his AI +0x11, ten frames from a press, the count
                                           * 0 as it runs out (0x0800BE6C-0x0800BE7A), */
#define BN4_GUTS_RAPID_AT 0x0800BE7Eu /* ... cmp r0,#6: at six or more the window the count (0x0800BE84), request 0x20 asked
                                       * (0x0800BE8A), his kept charge levels set as B let go sets them (+0x14, +0x15), */
#define BN4_GUTS_RAPID_ASK 0x080EEC04u /* ... movs r0,#0x10: which his ready state's requests (0x080EEBEE, before a plain or charged
                                        * B's) answer with his state 0x10, */
#define BN4_GUTS_RAPID 0x080EBDC0u     /* ... its routine (BN4_PLAYER_STATES' 0x10): eight buster shots (0x0800B590) with sound
                                        * 0xA7, the count and window cleared at its end (0x080EBE84-0x080EBE88) */

#define BN4_WOOD_ENTER    0x0800E174u /* WoodSoul's (form 12's): r0 his side, on to his hand's merge (its bl at +4), */
#define BN4_WOOD_MERGE    0x0800E294u /* ... which, from the chip up next, joins each chip of the kind Wood (BN4_WOOD_MERGE_KIND) and
                                       * the chip after it (0x0800E2C4-0x0800E2D6: of the kind none, BN4_WOOD_MERGE_KIND2, its power
                                       * shown, +9's 0x02): that chip's power, bonus and extra (the hand's +0xE, +0x1A and +0x26 by
                                       * index, its layout BN6's) added to the Wood chip's, and
                                       * it taken out of the hand, the chips after it moved up a place (0x0800E2F0-0x0800E31C; its
                                       * two bytes' arrays, +0x3E and +0x44, moved by two bytes a place as the u16s are) */
#define BN4_WOOD_MERGE_KIND 0x0800E2B0u /* ... cmp r1,#3: the first chip's kind (BN4_CHIP_KIND), Wood, */
#define BN4_WOOD_MERGE_KIND2 0x0800E2CCu /* ... cmp r1,#0xc: the next's, none */
#define BN4_STATUS_TAKE   0x08013470u /* the status a hit gave the struck put on him (the player's frame's, 0x0801363C, 0x08013728,
                                       * 0x08013836, 0x0801390C): his collision data's +0xF (BN4_COLL_STATUS_TAKEN), its class the
                                       * high nibble (1-4) and its level the low, the flag set and its frames stored by the table
                                       * 0x08018550 (paralysis's 0x08 for 90, 120, 150 frames), BN6_STATUS_TAKEN's twin */
#define BN4_STATUS_GUARD_FORM 0x08013908u /* the player's frame's (0x08013860's) cmp r0,#0xc on his form (movs r1,#0x24; bl 0x0800D7CA:
                                           * NaviStats +0x24) and its beq past the next bl: WoodSoul's form, whom no hit's status
                                           * takes, */
#define BN4_STATUS_GUARD_SKIP 0x0801390Cu /* ... the bl BN4_STATUS_TAKE it passes over */

#define BN4_CHIP_ROUTE    0x0800D406u /* r0 a chip, r7 MegaMan's attack's variables: a power attack's shot that is a chip (WindSoul's
                                       * AirShot 0x6B and WindRack 0x09, ProtSoul's WideSwrd 0x3E): the variables set from the chip's
                                       * record (+0x14 the chip, +0xC, +8 its +0xA and damage, +5, +6 0, +3, +2 and +4), its
                                       * family returned (+0xB); the power attack's stores after it (r1 into +8, +0xA, +3) its own */

#define BN4_WIND_ENTER    0x0800E0F4u /* WindSoul's (form 3's): his collision flags ORed (BN4_WIND_FLAGS), his collision set up as at
                                       * every form (0x08012ED0: 0x10, 2, 0), a wind of his side (bl BN4_WIND_SPAWN at
                                       * BN4_WIND_SPAWN_CALL, kept at his AI +0x5C), then his hand's pass (0x0800E208), */
#define BN4_WIND_FLAGS    0x0800E0F6u /* ... movs r0,#0x30, ORed into his collision data's flags by 0x08012F82 (BN4_COLL_FLAGS, BN6's
                                       * bits: BN6_OBJ_FLOATS and 0x10), */
#define BN4_WIND_COLL     0x0800E0FCu /* ... movs r1,#0x10; movs r2,#2; movs r3,#0, bl 0x08012ED0 (BN6_COLL_SET_UP's twin): his collision's
                                       * regions set again, the masks of region 0x10 where his battle's start set his others, */
#define BN4_WIND_LEVEL    0x0800E106u /* ... movs r4,#1: the spawn's r4, its level (the wind's +4), */
#define BN4_WIND_SPAWN_CALL 0x0800E108u /* ... the bl BN4_WIND_SPAWN, */
#define BN4_WIND_PASS_KIND 0x0800E22Eu /* ... the pass: cmp r1,#0xa, the kind Wind (BN4_CHIP_KIND), its power shown, */
#define BN4_WIND_PASS_ADD 0x0800E236u /* ... adds r1,#0xa: 10 to its bonus, as GutsSoul's pass */
#define BN4_FORM_RESET    0x0800D8C2u /* BN4_FORM_STATE's first call, as every form is taken: his collision flags' mask cleared (the
                                       * word at 0x0800DB14), 1 set where NaviStats +0x28 is 1, his AI +0x5C's wind ended (BN4_WIND_END)
                                       * and the word cleared (0x0800D8F6-0x0800D902): WindSoul's taken away as he leaves */

#define BN4_PROT_TEST_FORM 0x0800BC88u /* BN4_CHIP_CHARGE_TEST's cmp r0,#7: ProtSoul's form, */
#define BN4_PROT_TEST_KIND 0x0800BC8Cu /* ... cmp r4,#6: a Sword chip (BN4_CHIP_KIND), not left alone (its flags' bit 0, 0x0800BC90) */
#define BN4_PROT_BONUS_FORM 0x0800C4A8u /* BN4_CHIP_CHARGE_BONUS's cmp r2,#7, */
#define BN4_PROT_BONUS_KIND 0x0800C4AEu /* ... cmp r1,#6: a Sword chip charged doubled (BN4_CHARGED_DOUBLE) */
#define BN4_PROT_ENTER    0x0800E12Cu /* ProtSoul's (form 7's): ldr r0,[r5,#0x58]; movs r1,#0x6a; strb r1,[r0,#0]: his AI data's +0,
                                       * BN6's BN6_AI_POWER_C, his B and left's power attack, */
#define BN4_PROT_LEFT     0x0800E12Eu /* ... the movs r1,#0x6a: his shield (0x0800D3C0: family 0x25, BN4's Guard state's subfamily 4,
                                       * power 50, the hit word 0x114) */

/* RollSoul's heal in MegaMan's chip use (0x080EED04-0x080EED32, after the chip's reload, BN6's at BN6_CHIP_USE_HEAL): in form 1,
 * a chip of another kind than its own, his max HP's tenth (swi 6) healed (0x0800ABBA, BN6_OBJECT_HEAL), the effect spawned at
 * him (BN4_SPAWN_EFFECT) and the sound played */
#define BN4_ROLL_HEAL_FORM  0x080EED0Au /* cmp r0,#1: NaviStats +0x24, his form, RollSoul's */
#define BN4_ROLL_HEAL_KIND  0x080EED10u /* ... cmp r0,#4: the chip's kind (BN4_CHIP_KIND) none of Recovery's */
#define BN4_ROLL_HEAL_SHARE 0x080EED16u /* ... movs r1,#0xa: his max HP divided by it */
#define BN4_ROLL_HEAL_EFFECT 0x080EED28u /* ... movs r4,#7: the effect (Repair's, xchips_bn4.c) */
#define BN4_ROLL_HEAL_SOUND 0x080EED2Eu /* ... movs r0,#0x9a: the sound (Repair's) */

#endif
