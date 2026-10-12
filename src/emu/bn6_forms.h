/* MegaMan's forms in BN6's battle engine (bn6.h includes it): the tables
 * the engine reads by his form (BN6_BATTLE_FORM), the places Gregar's code
 * reads the form itself, and a Cross's pending record and used bits; at
 * Gregar's addresses (docs/ROM_DATA.md, MegaMan's forms; issue #82). A
 * table holds BN6_FORMS entries: 0 none, 1-10 the Crosses (Gregar's five,
 * then Falzar's SpoutCross to DustCross, 6-10, which Gregar keeps and never
 * offers), 0xB-0x16 Beast Out (BN6_FORM_BEAST, a Cross's from
 * BN6_FORM_CROSS_BEAST), 0x17-0x18 BeastOver. The Souls' proxy form
 * (src/emu/soulproxy.c) is form 8, Falzar's TenguCross. */
#ifndef CW_BN6_FORMS_H
#define CW_BN6_FORMS_H

#define BN6_FORMS             25          /* the entries of a table by form (0-0x18) */
#define BN6_FORM_HEAT_CROSS   1           /* BN6_BATTLE_FORM: HeatCross (BN6_FLAG_HEAT_CROSS's) */
#define BN6_FORM_PROXY        8           /* BN6_BATTLE_FORM: Falzar's TenguCross, which no Gregar CROSSSELECT offers: the Souls' proxy form */
#define BN6_FORM_NAME_BASE    0x1AB       /* MegaMan's NameID in a form: this + the form (0x08015B22 writes it; 0x1A0 in none), the object id table's entry */
/* The tables by form (each a pointer word's in the code names it, docs/ROM_DATA.md) */
#define BN6_FORM_SPRITES      0x0800FCBCu /* u8 a form: MegaMan's sprite, its number in his category (0x0800FC9E reads it; Crosses 0, his own) */
#define BN6_FORM_OVERLAYS     0x0801127Cu /* u32 a form: the routine that spawns a Cross's overlay as he crosses (0x08011268 calls it) */
#define BN6_FORM_OVERLAYS_END 0x08011398u /* u32 a form: the routine that takes it away as he leaves the form (0x08011384) */
#define BN6_FORM_ELEMENTS     0x080108B8u /* u8 a form: his element (0x0801086C, into his object +0x0E and collision data +2) */
#define BN6_FORM_WEAKNESSES   0x080108D1u /* u8 a form: the second wheel's elements he is weak to (0x0801086C, into collision data +0x18) */
#define BN6_FORM_PASSIVES     0x08014550u /* u32 a form: the routine giving his AI data the form's passive (0x08014536: an Airshoe, a SuperArmor) */
#define BN6_FORM_PASSIVES_COLL 0x080146B8u /* u32 a form: ... his collision data's (0x0801469C) */
#define BN6_FORM_LOADOUTS     0x08020354u /* 6 bytes a form: his AI data +4-+8 and +0x11 (his charged shot's power attack at +3; 0x0800FEEC) */
#define BN6_FORM_LOADOUT_SIZE 6           /* BN6_FORM_LOADOUTS: bytes a form's entry */
#define BN6_FORM_PALETTES     0x080203EAu /* u8 a form: his body's palette (0x0801002C, and four readers more) */
#define BN6_FORM_BUSTER_BONUS 0x080126B0u /* u8 a form: Attack added to his buster's (0x0801265A: HeatCross 1) */
#define BN6_FORM_MUGSHOTS     0x0801E700u /* u8 a form: his face in the emotion window (0x0801E660, 0x0801E6A8) */
#define BN6_OBJECT_IDS        0x080182C4u /* (version, actor type, AI index) per object id, 3 bytes (RomLayout.enemy_ids'): a form's NameID's
                                             (BN6_FORM_NAME_BASE + the form) gives MegaMan's arm, his records and struct1 as he crosses */
/* Gregar's code that reads the form itself, not through a table */
#define BN6_CHIP_BONUS_HEAT   0x0800EF62u /* cmp r0,#1 in the chips' bonuses by form (0x0800EF34), MegaMan's form in r0: HeatCross's +50 to Fire
                                             chips; Falzar's TenguCross's +10 to Wind chips stands further on (cmp r0,#8 at 0x0800EFE6) */
#define BN6_ARM_FLASH_FORM    0x080EC8DEu /* cmp r0,#0 after the routine that spawns MegaMan's arm flash (0x080EC8B2) reads his form into r0: in a
                                             Cross the flash (the first kind's type 5) takes the form as its animation, the Cross's arm */
#define BN6_CROSS_SUMMON      0x08014BB6u /* movs r3,#0x14 before the summon of the Cross's Navi behind MegaMan (0x080BE0B4) as he crosses, the form
                                             in r4: the summon draws the Navi of that number (phase 1 of the crossing, 0x08014B98) */
#define BN6_CROSS_COMMIT      0x08029394u /* movs r1,#0 before CROSSSELECT's commit writes the form chosen (r0: its row + 1) into
                                             BN6_CROSS_RECORD (0x08015952; the routine 0x0802937A, then the row's used bit) */
/* A Cross chosen, in RAM */
#define BN6_CROSS_RECORD      0x0203CED0u /* the Custom screen's record of the form chosen: +0 the form (0xFF none), +1, +3; copied to BN6_CROSS_PENDING */
#define BN6_CROSS_PENDING     0x0203A980u /* the form each side takes as its next turn starts, 16 bytes a side, laid out as BN6_CROSS_RECORD */
#define BN6_CROSS_PENDING_SIZE 0x10       /* BN6_CROSS_PENDING: bytes a side's */
#define BN6_CROSSES_USED      0x020349A0u /* u32: bit i as CROSSSELECT's Cross i is chosen (HeatCross bit 0), cleared as a battle begins */

#endif
