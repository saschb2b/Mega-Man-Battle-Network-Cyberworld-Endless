/* Chaos Unison in BN6's battle engine (the Souls track's S6, src/emu/soulchaos.c; docs/rom/souls.md, Chaos Unison):
 * the places of Gregar's charge, release, requests and glow it stands on, by BN5's. Split from bn6_forms.h, which
 * includes it. */
#ifndef CW_BN6_CHAOS_H
#define CW_BN6_CHAOS_H

/* Chaos Unison (the Souls track's S6, soulchaos.c): what it stands on of BN6's charge, release, request and glow, by BN5's */
#define BN6_CHAOS_CHARGE      0x08012F62u /* push {r4,r7,lr}: a charge's time by r0 (1 A, 2 B), r5 MegaMan (BN5's 0x08010654, which a chaos B
                                             answers from his AIData +0x11's power by the chaos level) */
#define BN6_CHAOS_DUE         0x08012E8Au /* bl 0x08012EBC: the charge's state each frame (BN5's 0x0801053C), r4 his AI data; BN5's 0x080105F0 */
#define BN6_CHAOS_PHASE       0x08012E8Eu /* ... after it, pop {r4,r6,pc}: the chaos phase's step (BN6 has none) */
#define BN6_CHAOS_RELEASE     0x080131D4u /* bl 0x0800FFE4: the release's request, r0 1 or 2 (a charged B), r4 his AI data (BN5's 0x08010870,
                                             0x8000 or 0x10000 there for a chaos B) */
#define BN6_CHAOS_CONSUME     0x080F180Eu /* bl 0x0800FFFE in the player's requests (0x080F1694, push {r4,r6,r7,lr}) past the charged B's 2, r0 the
                                             requests (BN5's 0x080F0436: 0x8000 and 0x10000 here) */
#define BN6_COUNTER_TEST      0x080EBA84u /* tst r0,r1 in the player's update (0x080EA734), r1 0x8600: BN6's counter requests, its 0x8000 one BN5 has
                                             not (BN5's 0x080EB0D8 tests 0x600), a request BN5's chaos B makes */
#define BN6_COUNTER_TEST_1D   0x080ED7B0u /* ... the same in family 0x1D's (BN5's 0x080ED422), */
#define BN6_COUNTER_TEST_1E   0x080ED90Eu /* ... family 0x1E's (0x080EDDB2), */
#define BN6_COUNTER_TEST_37   0x080EEE32u /* ... family 0x37's (0x080EEAE4) */
#define BN6_CHAOS_END         0x08011744u /* bl 0x0800FFEE in the attack's end (0x08011714), r0 0x1000003F the requests it clears, r6 the
                                             attack's kind (BN5's 0x0800F2D0 clears 0x1803F, its kind 6 his chaos) */
#define BN6_GLOW_SHOW         0x080E21E4u /* beq in the charge glow's update (the fourth kind's 8, r4 its owner): a B charge seen whatever the
                                             side (BN5's 0x080E0ECE: in chaos by the side, 0x080091EA) */
#define BN6_GLOW_SHOW_SIDE    0x080E21E6u /* ... the side's test */
#define BN6_GLOW_PICK         0x080E2272u /* movs r2,#8: the glow's sprite past A's 0x15 (BN5's 0x080E0F94: 0x1C in chaos), */
#define BN6_GLOW_PICK_NEXT    0x080E2274u /* ... on past it */
#define BN6_GLOW_LOAD         0x080E2280u /* ... its bl sprite_load, r1 list 5's offset 0x14, r2 the number */
#define BN6_GLOW_ANIM         0x080E2208u /* cmp r0,#0 past strb r0,[r5,#0x10]: its animation the charge's state (BN5's 0x080E0F0A: the chaos phase
                                             + 2 for a full chaos B), r6 its owner's AI data */
#define BN6_GLOW_SOUND        0x080E22A8u /* beq in its sounds (0x080E229A, r1 its owner): a B charge's whatever the side (BN5's 0x080E0FD4) */
#define BN6_GLOW_SOUND_SIDE   0x080E22AAu /* ... the side's test */
#define BN6_GLOW_SPRITE_CHAOS 0x1C        /* the chaos glow's number its +0x6C keeps (BN5's list 5's) */
#define BN6_CHAOS_FAMILY      0xC7        /* the failure's attack family (BN5's 0x39, 0x080EE724; xattack_grow.h's Souls' range) */

#endif
