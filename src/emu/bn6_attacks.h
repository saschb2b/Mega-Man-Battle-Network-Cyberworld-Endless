/* BN6's attack tables in Gregar (USA), part of bn6.h's addresses (it
 * includes this): the attack families every attack runs as its CurAction
 * (MegaMan's charged shots, his chips, an enemy's attacks), the power
 * attacks MegaMan's loadout names, and their charge times; each the one
 * word or words that name it, as the engine grows them (issue #82: the
 * families grown for BN5's chips and Souls, xattack_grow.c; the power
 * attacks and charge times for BN5's Souls, soulpower.c; docs/rom/souls.md,
 * BN6's attack tables grown). */
#ifndef CW_BN6_ATTACKS_H
#define CW_BN6_ATTACKS_H

/* The attack families: a routine each, by its CurAction - 0x10 (RunAIAttack 0x0801B9E6 and 0x080EC276 read it; 0x10-0x5E) */
#define BN6_ATTACK_FAMILIES     0x080EBFA0u
#define BN6_ATTACK_FAMILIES_REF 0x0801BB10u   /* RunAIAttack's literal naming it */
#define BN6_ATTACK_FAMILIES_REF2 0x080EC384u  /* ... 0x080EC276's */
#define BN6_ATTACK_FAMILY_FIRST 0x10          /* the CurAction of its first entry (a CurAction under it is a state's) */
#define BN6_ATTACK_FAMILY_COUNT 79            /* BN6's own, CurActions 0x10-0x5E */
#define BN6_EXIT_ATTACK         0x08011714u   /* object_exitAttackState: an attack's end (every family's last phase calls it) */
#define BN6_FAMILY_SWORD        0x13          /* the swords' family (the chips' and the Souls' charged shot's: BN5's the same number) */
#define BN6_COUNTER_START       0x080105F2u   /* a counter's attack begun (r0 the hit's flags, r1-r3 its numbers): setAttack0 with family
                                                 0x47 or 0x48, its first frame left to the next (BN5's twin 0x0800E338 runs it at once) */
#define BN6_CHIP_WRITE_NOLOG  0x080126EAu   /* the chip writer's second entry (BN5's 0x0800FE70): r0 the chip, its family back in r0, no
                                               chip-used log (push {r4,r6,r7,lr}; movs r4,#0): a Chaos Unison's power attack's DarkChip */

/* An attack's variables (AttackVars, AI data +0xA0) as a chip's use writes them (0x080127C0, BN5's twin 0x0800FF40) */
#define BN6_ATTACK_CHARGED      0x04          /* u8 the chip charged (the use's first argument) */
#define BN6_ATTACK_DAMAGE       0x08          /* u16 its damage */
#define BN6_ATTACK_CHIP         0x14          /* u16 the chip used */
#define BN6_ATTACK_SPAWNER      0x03          /* u8 the spawner a spawning family runs (family 0x1C, 0x080ED6EC: its table by this; BN5's 0x1A) */
#define BN6_ATTACK_SPAWN_ARG    0x0C          /* u32 ... handed to the spawner in r4 */
/* The power attacks: a routine each, by MegaMan's loadout's id (his AI data +4-+8, +0x11), which sets an attack's
 * numbers and returns its family (0x0801176C-0x080117C2 read it through one word) */
#define BN6_POWER_ATTACKS       0x080117D4u
#define BN6_POWER_ATTACKS_REF   0x080117D0u   /* the one word naming it (five loads) */
#define BN6_POWER_ATTACK_COUNT  148           /* BN6's own, to 0x08011A24 */
/* The charge times: five halfwords a power attack, by his buster's Charge (NaviStats +3; DeterminePowerAttackChargeTime 0x08012F62) */
#define BN6_CHARGE_TIMES        0x08020404u
#define BN6_CHARGE_TIMES_REF    0x08012FC4u   /* the one word naming it (0x08012FA6) */
#define BN6_CHARGE_ROW          10            /* bytes a power attack's */

#define BN6_CHIP_ROUTE    0x080129A6u /* r0 a chip, r7 MegaMan's attack's variables: BN4_CHIP_ROUTE's twin, from BN6's record (its
                                       * +2 with the element's bit of 0x080129E4's table by the record's +6), its family returned */

#define BN6_ATTACK_END        0x08011714u /* an attack's end (BN4's 0x0800C9FC its twin): its variables' +0x1C, the kind its state was set
                                             with (0x08011680-0x08011694 by r2), 2 setting his AI data's +0x19 and 3 (a B and left's)
                                             his BN6_AI_LEFT_HOLD from the variables' +5, */
#define BN6_ATTACK_END_LEFT   0x0801173Cu /* ... its strb r0,[r1,#0x15]: BN4's end sets no lock, its power attack having set it as it began */
#define BN6_ATTACK_END_LEFT_PAST 0x0801173Eu /* ... and past it */

/* An attack's lag and MegaMan's step out of it (his buster's state 4, 0x080EEAE2-0x080EEB4E, the lag ended early by a step
 * the pad asks: RollSoul's charged shot's, BN4's 0x080EC5F6, steps out through these, xchips_bn4.h XC4_STEP_OUT) */
#define BN6_SHOT_LAG           0x0800FAF6u /* r0, r1 a panel, r2 a row: the frames, BN6_SHOT_LAGS' row by the free panels before it (to 5) */
#define BN6_SHOT_LAGS          0x080209CCu /* ... six bytes a row (row 0 BN4_SHOT_LAGS' row 0) */
#define BN6_STEP_ASK           0x0800FA54u /* r5 MegaMan: the step his pad asks (his AI +0x22): 1 up, 2 down, 3 left, 4 right, turned by a table under his status 0x8000, 0 none */
#define BN6_STEP_ASK_PANEL     0x0800F964u /* ... r0 that step: the panel it goes to in r0, r1, 0 where he cannot stand there */
#define BN6_STEP_ASK_STYLE    0x08010332u /* ... his step's style by his NaviStats +0x29 and +0x2B (4 his plain step) */
#define BN6_STEP_ASK_KIND      0x080103A8u /* ... 3 where his NaviStats +0x31 is set, else 0 */
#define BN6_STEP_ASK_START     0x080116AEu /* ... the step's state begun (r0 the step, r1 its style, r2 its kind) */

/* MegaMan's chip use past its reload's call (0x080F18F4-0x080F191A, BN4's 0x080EECDA-0x080EED00): movs r3,#0xa0; adds r3,r3,r6
 * (his attack's variables), where BN4's RollSoul heals (BN4_ROLL_HEAL_FORM's routine, soulroll.c), r4 the chip's record */
#define BN6_CHIP_USE_HEAL      0x080F191Eu
#define BN6_CHIP_USE_HEAL_PAST 0x080F1922u /* ... past the two */

#endif
