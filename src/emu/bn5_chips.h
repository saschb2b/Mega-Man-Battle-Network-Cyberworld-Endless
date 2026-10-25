/* Battle Network 5: Team Colonel (USA): what BN6's engine stands on of
 * BN5's chips' and Souls' code where its own C and Thumb answer a part of
 * it (issue #82's final engine round; docs/rom/chips-code.md, DarkPlus's flash
 * and Chaos Unison), beside bn5.h's addresses: each range the port follows
 * checked by its CRC32 as the core starts. */
#ifndef CW_BN5_CHIPS_H
#define CW_BN5_CHIPS_H

/* DarkPlus's green flash (BN5's AIData +0x3C, a u16 of frames: xchips_flash.c) */
#define BN5_FLASH_STORE       0x0800E1D4u /* the misc family's subfamily 3: strh r0,[r1,#0x3c] after movs r0,#0x18 (the power stored in the hand's next chip) */
#define BN5_FLASH_STORE_GATE  0x0800E228u /* ... the same in its Chip Gate branch (a link battle's, 0x080090E8) */
#define BN5_FLASH_STRH        0x8788      /* strh r0,[r1,#0x3c] */
#define BN5_FLASH_FRAMES_OP   0x2018      /* movs r0,#0x18: 24 frames */
#define BN5_FLASH_COUNT       0x0800E532u /* the player's count-down of AIData +0x3C-+0x43, a frame each but while time stops (0x08008A78), */
#define BN5_FLASH_COUNT_END   0x0800E556u /* ... its first part, to */
#define BN5_FLASH_COUNT_CRC   0x3C0B200Fu
#define BN5_FLASH_TINT        0x080136B0u /* the tint (in the colours' chain 0x08017D9C, before 0x08013738): sprite_setColorShader of the table's */
#define BN5_FLASH_TINT_END    0x080136D8u /* ... byte by the timer & 7, shifted up 5; to the table's end, */
#define BN5_FLASH_TINT_CRC    0x8F9A13FBu
#define BN5_FLASH_TINTS       0x080136D0u /* ... the table: 8 bytes */

/* DarkInvs (darkinvs.c): BN5's chip AI for the player, which its object puts MegaMan under, its time, its end and what its
 * object clears */
#define BN5_CHIP_AI_PLAYER 0x0802B4B0u /* its chip AI for the player (its own side's lists; the Dark Soul's beside it), run from BN5's idle states (0x080F04DC, 0x080F0660)
                                       * while his AI data's BN5_AI_UNDER flag stands (BN5's DarkInvs sets it: BN5_CHIP_AI_ENTER from
                                       * 0x080E2402), else its state cleared (0x0802C03E, 0x40 bytes from +0xF0) */
#define BN5_AI_UNDER      0x200       /* ... the flag: AIData +0x48's bit (BN5's 0x0800E096 sets it, as BN6's 0x08010312 its bits there) */
#define BN5_AI_UNDER_TIME 0x36        /* ... its time, AIData +0x36 (u16: 600 from DarkInvs, counted down by 0x0800DBF0) */
#define BN5_AI_UNDER_END  0x80000     /* ... at its time's 0 (0x0800DBD8, the player's frame's last: 0x08017976) his request (AIData +0x44), which
                                       * BN5's family 0x49 answers (0x080EF270: the flag and the invisibility cleared; started by
                                       * 0x080EB0EA, 0x080ED43C) */
#define BN5_STATUS_FLAGS  0xE800      /* what DarkInvs's 0x08017060 clears of his collision's flags (+0x3C), */
#define BN5_STATUS_FLAGS2 0x8068      /* ... and of +0x40, with the four status times after (BN5 +0x1E-+0x25) */
#define BN5_CHIP_AI_STATE 0xF0        /* ... its state from AIData +0xF0 (+0: 1 under way), 0x40 bytes the entry clears */
#define BN5_ARM_CHAOS_READ 0x080EBBFEu /* ldrb r1,[r7,#0x12] in its buster arm (0x080EBBA6, MegaMan in a form): his Chaos Unison (AIData +0x12),
                                       * the flash BN5_FLASH_CHAOS where set (0x080EBC04) */

/* Chaos Unison (soulchaos.c), BN5's code it ports: */
#define BN5_CHAOS_PHASE       0x080105F0u /* the chaos charge's phase each frame (its cycle AIData +0x1C, the phase +0x1F), to */
#define BN5_CHAOS_PHASE_END   0x08010654u
#define BN5_CHAOS_PHASE_CRC   0x1E435758u
#define BN5_CHAOS_PHASES      0x08010648u /* ... its table, a row of 4 bytes a level (0, 1, 2 and up: 0x080106B4): the cycle's frames, then the
                                             frames to which the phase is 2, 1, 0, past them 1 */
#define BN5_CHAOS_CHARGE      0x08010654u /* a charge's time: a chaos B's by AIData +0x11's power, the level's column (4 at most), to */
#define BN5_CHAOS_CHARGE_END  0x0801069Eu
#define BN5_CHAOS_CHARGE_CRC  0xA14447D5u
#define BN5_CHAOS_RELEASE     0x08010850u /* the release's request: 0x8000 a chaos B in phase 2, 0x10000 another, to */
#define BN5_CHAOS_RELEASE_END 0x08010874u
#define BN5_CHAOS_RELEASE_CRC 0x9ECDB5A9u
#define BN5_CHAOS_CONSUME     0x080F0436u /* the player's requests' 0x8000 (the chaos attack) and 0x10000 (the failure, family 0x39), to */
#define BN5_CHAOS_CONSUME_END 0x080F0496u
#define BN5_CHAOS_CONSUME_CRC 0x4A25F2E1u
#define BN5_CHAOS_ATTACK      0x0800F330u /* the chaos attack: the battle's counter 0, then the power attack AIData +0x11 names, to */
#define BN5_CHAOS_ATTACK_END  0x0800F350u
#define BN5_CHAOS_ATTACK_CRC  0xA0F428F8u
#define BN5_CHAOS_FAMILY      0x080EE724u /* the failure's family 0x39: its phases 0, 4 and 8, the Dark Soul at 8 frames, out of the Soul, to */
#define BN5_CHAOS_FAMILY_END  0x080EE864u
#define BN5_CHAOS_FAMILY_CRC  0x4D8BBCACu
#define BN5_CHAOS_POWERS      0x0800FA36u /* the chaos power attacks 0x31-0x3A (movs r0,#chip; bl 0x0800FE70), to */
#define BN5_CHAOS_POWERS_END  0x0800FA9Au
#define BN5_CHAOS_POWERS_CRC  0x01F90648u
#define BN5_CHAOS_GLOW        0x080E0E44u /* the charge glow's update (the fourth kind's 8): its sprite, animation, sounds in chaos, to */
#define BN5_CHAOS_GLOW_END    0x080E100Cu
#define BN5_CHAOS_GLOW_CRC    0xE1BD6104u
#define BN5_CHAOS_OUT         0x0810436Cu /* the Dark Soul out ended (its AIData +0x88 0) on the other side's four, to */
#define BN5_CHAOS_OUT_END     0x081043A6u
#define BN5_CHAOS_OUT_CRC     0xD7501CDFu
#define BN5_CHAOS_LEVEL_TOP   4           /* the chaos level's most (0x080F0446) */
#define BN5_DARK_SOUL_TIMER   0x88        /* the Dark Soul's AIData +0x88: 0 ends it (BN5_CHAOS_OUT) */

#endif
