/* BN6's attack tables in Gregar (USA), part of bn6.h's addresses (it
 * includes this): the attack families every attack runs as its CurAction
 * (MegaMan's charged shots, his chips, an enemy's attacks), the power
 * attacks MegaMan's loadout names, and their charge times; each the one
 * word or words that name it, as the engine grows them (issue #82: the
 * families grown for BN5's chips and Souls, xattack_grow.c; the power
 * attacks and charge times for BN5's Souls, soulpower.c; docs/ROM_DATA.md,
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
/* The power attacks: a routine each, by MegaMan's loadout's id (his AI data +4-+8, +0x11), which sets an attack's
 * numbers and returns its family (0x0801176C-0x080117C2 read it through one word) */
#define BN6_POWER_ATTACKS       0x080117D4u
#define BN6_POWER_ATTACKS_REF   0x080117D0u   /* the one word naming it (five loads) */
#define BN6_POWER_ATTACK_COUNT  148           /* BN6's own, to 0x08011A24 */
/* The charge times: five halfwords a power attack, by his buster's Charge (NaviStats +3; DeterminePowerAttackChargeTime 0x08012F62) */
#define BN6_CHARGE_TIMES        0x08020404u
#define BN6_CHARGE_TIMES_REF    0x08012FC4u   /* the one word naming it (0x08012FA6) */
#define BN6_CHARGE_ROW          10            /* bytes a power attack's */

#endif
