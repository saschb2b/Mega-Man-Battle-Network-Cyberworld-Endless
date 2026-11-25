/* BN4 Red Sun's Souls in BN6's engine (docs/multirom/bn4-souls.md, ThunSoul
 * in BN6's engine): on the proxy form as BN5's six are, united on BN6's
 * Custom screen by the run's rule (BN5's), each with its charged shot's
 * power attack and attack family moved with the BN4 chips' units (its
 * family an attack unit's, its power attack copied with it:
 * tools/xchiprecipe_bn4.py SOUL_POWERS), its loadout, charge times and
 * element read from BN4's tables by its form. BN6's numbers for them: a
 * Soul's number in BN6_SOULS' fields past BN5's 1-12, its bit in a
 * MegaMan's Souls held past BN5's six. ThunSoul first; which of them a run
 * gives is the owner's call (none yet: the differential's souls6 alone). */
#ifndef CW_SOULBN4_H
#define CW_SOULBN4_H

#include <stdbool.h>
#include <stdint.h>

#define SOUL_BN4_FIRST 13   /* BN6's Soul number (BN6_SOUL_ID, the element's +BN6_ELEMENT_SOUL, a used bit) of BN4's first moved */
#define SOULS_BN4      12   /* BN4's Souls moved, all twelve: ThunSoul (BN4's 6), GutsSoul (BN4's 2), AquaSoul (BN4's 11), SrchSoul
                             * (BN4's 4), NumSoul (BN4's 8), FireSoul (BN4's 5), MetlSoul (BN4's 9), WoodSoul (BN4's 12), WindSoul
                             * (BN4's 3), ProtSoul (BN4's 7), RollSoul (BN4's 1), JunkSoul (BN4's 10) */
#define SOULS_HELD_BN4 6    /* the bit of the first in the Souls held (soulunite_held), past BN5's six */

/* A BN4 Soul's rows as BN6's engine takes them, read from BN4's ROM */
typedef struct {
	uint8_t form;      /* BN4's Soul, its form (1-12) */
	uint8_t kind;      /* the chip kind it unites by (BN4_SOUL_KINDS: BN4's numbering, soulkinds.h's soulkinds_bn4_of) */
	uint8_t element;   /* MegaMan's element in it (BN4_FORM_ELEMENTS: BN6's numbering) */
	uint8_t power;     /* its charged shot's power attack, BN4's id (BN4_FORM_LOADOUTS +3) */
	uint32_t routine;  /* ... its routine moved with its family's unit (xchips_bn4_moved_at), 0 none */
	uint8_t charge[10];/* ... its charge times by his Charge (BN4_CHARGE_TIMES, BN6_CHARGE_ROW's shape) */
	uint8_t chip;      /* his chip charge's power attack, BN4's id (BN4_FORM_LOADOUTS +2: BN4's empty routines, BN4_POWER_EMPTY and
	                    * BN4_POWER_EMPTY2), NONE none */
	uint8_t chip_charge[10];  /* ... its charge times by his Charge, the A button's hold before a chip is used charged */
	uint8_t plain;     /* his plain shot's power attack, BN4's id (BN4_FORM_LOADOUTS +1), 0 BN4's own for every form: MetlSoul's 0x0B */
	uint32_t plain_routine;   /* ... its routine moved with his unit (tools/xchiprecipe_bn4.py SOUL_PLAINS), 0 none */
	uint8_t plain_charge[10]; /* ... its charge times */
	uint8_t left;      /* his B and left's power attack, BN4's id, which his taking of the form sets (BN4's AI data +0, BN6's
	                    * BN6_AI_POWER_C: ProtSoul's shield 0x6A), NONE none */
	uint32_t left_routine;    /* ... its routine moved with a unit (SOUL_POWERS), 0 none */
	uint8_t left_charge[10];  /* ... its charge times */
} SoulBn4;

/* BN6's k-th BN4 Soul (0 to SOULS_BN4 - 1) read: false where BN4's ROM is
 * not read, its rows are not the shape read, or its charged shot's code is
 * not moved (BN4's chips' units installed first: xchips_code.h) */
bool soulbn4_read(int k, SoulBn4 *out);
/* k of BN6's Soul number `soul`, -1 for none of BN4's */
int soulbn4_k(int soul);
/* BN4's form of BN6's k-th BN4 Soul (ThunSoul's 6, GutsSoul's 2), 0 for none */
int soulbn4_form(int k);
/* (a hook's, MegaMan `obj` entering the proxy form with BN4's k-th Soul)
 * what BN4's routine as the form is taken does (BN4_FORM_ENTERS) where it
 * is moved: GutsSoul's pass of his side's hand, +30 to the bonus of each
 * chip up next of no kind or panel-cracking whose power shows (BN4_GUTS_PASS,
 * its numbers read from BN4's code; BN4's hand's +0x26 on, BN6's
 * BN6_HAND_BONUS: the attack's +6 beside its power at +8 in both) */
void soulbn4_enter(uint32_t obj, int k);
/* whether no hit's status takes BN4's k-th Soul: BN4's player's frame passes over it for his form (BN4_STATUS_GUARD_FORM:
 * WoodSoul's), read from BN4's code */
bool soulbn4_guarded(int k);
/* the collision flags BN4's k-th Soul's taking of the form ORs in where it also spawns his side's wind (BN4_WIND_ENTER,
 * WindSoul's 0x30), read from BN4's code; 0 for a Soul with no wind */
uint32_t soulbn4_wind(int k);
/* ... and the level WindSoul's taking of the form spawns it at (BN4_WIND_LEVEL, its r4), -1 where the code is not that */
int soulbn4_wind_level(void);
/* ... and the arguments it sets his collision up again with first (BN4_WIND_COLL); false where the code is not that */
typedef struct { uint8_t region, region2, arg; } SoulBn4CollSet;
bool soulbn4_wind_coll(SoulBn4CollSet *out);
/* (after xchips_bn4_code_install) BN4's k-th Soul's taking of the form moved with his unit (tools/xchiprecipe_bn4.py
 * SOUL_ENTERS: SrchSoul's, which spawns his object), its copy's address with its Thumb bit, 0 where it is not moved */
uint32_t soulbn4_enter_copy(int k);
/* ... and whether that routine is run at the end of his next frame rather than as he enters the proxy form: JunkSoul's, whose
 * object lasts the fight (BN4_JUNK_ENTER), spawned on the fight's clock as BN4's (soulwind.c, as WindSoul's wind) */
bool soulbn4_late(int k);
/* (a hook's) whether BN4's k-th Soul charges chip `chip` (BN6's id) with A, by BN4's test (BN4_CHIP_CHARGE_TEST: the chip's
 * kind by BN4's numbering, soulkinds_bn4_of, and its flags): AquaSoul an Aqua chip not left alone; false for a Soul with no
 * chip charge */
bool soulbn4_charges(int k, int chip);
/* (after xchips_bn4_code_install) BN4's k-th Soul's own attack for a chip he charged, which BN4's chip use runs in place of the
 * chip's (FireSoul's flame, BN4_FIRE_CHARGED, at BN4_FIRE_CHARGED_CALL), its copy's address with its Thumb bit, 0 none */
uint32_t soulbn4_charged_copy(int k);
/* ... and what BN4 adds to the power of a chip it charged (BN4_CHIP_CHARGE_BONUS): BN4_CHARGED_DOUBLE, its damage doubled */
unsigned soulbn4_charge_bonus(int k);
/* ... and what it adds to an uncharged chip's (BN4_CHIP_BONUS): ThunSoul's BN4_PARALYSING to an Elec chip whose power shows, 0 else */
unsigned soulbn4_chip_bonus(int k, int chip);
/* ... and his heal after a chip of his (RollSoul's, BN4_ROLL_HEAL_FORM's routine): a chip of another kind than `kind` (BN4's
 * numbering) heals his max HP over `share`, the effect `effect` spawned at him and the sound `sound` played, BN4's ids; false
 * for a Soul with no heal or where the code is not that */
typedef struct { uint8_t kind, share, effect, sound; } SoulBn4Heal;
bool soulbn4_heal(int k, SoulBn4Heal *out);
/* BN4's Souls a run holds, bit n - 1 its Soul n by BN4's numbers (its forms,
 * src/director/souls.h souls_held_bn4), as the engine's bits of them in the
 * mask soulunite_held takes (from SOULS_HELD_BN4, by k): the moved ones
 * alone, so a Soul not yet moved is held by the run and lights nothing */
unsigned soulbn4_held(unsigned bn4);

#endif
