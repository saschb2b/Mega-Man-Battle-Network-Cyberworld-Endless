/* Chaos Unison's Thumb (soulchaos_thumb.h). */
#include "soulchaos_thumb.h"

#include <stddef.h>

#include "bytes.h"

/* The project's own Thumb (issue #82, the Souls track's S6; docs/ROM_DATA.md, Chaos Unison), each part beside the BN5 code
 * it is the twin of, its calls through a register to each BN5 routine's BN6 twin (BN6's own where BN6's leave of a form
 * does a step BN5's failure does inline); its literal words the routines' addresses, those soulchaos.c fills 0 here.
 * Written by hand, each instruction's encoding as the CPU reads it. */
static const uint16_t thumb[CHAOS_THUMB_SIZE / 2] = {
	/* power_colonel: */
	0xB500,            /* 000 push {lr}: ColonelSoul's: DrkSonic (BN5's power 0x3A, 0x0800FA90) */
	0x480B,            /* 002 ldr r0, =chip_colonel */
	0xE00D,            /* 004 b power_tail */
	/* power_shadow: */
	0xB500,            /* 006 push {lr}: ShadowSoul's: DarkInvs (0x31, 0x0800FA36) */
	0x480A,            /* 008 ldr r0, =chip_shadow */
	0xE00A,            /* 00A b power_tail */
	/* power_number: */
	0xB500,            /* 00C push {lr}: NumberSoul's: DarkPlus (0x32, 0x0800FA40) */
	0x480A,            /* 00E ldr r0, =chip_number */
	0xE007,            /* 010 b power_tail */
	/* power_tomahawk: */
	0xB500,            /* 012 push {lr}: TomahawkSoul's: DrkLance (0x33, 0x0800FA4A) */
	0x4809,            /* 014 ldr r0, =chip_tomahawk */
	0xE004,            /* 016 b power_tail */
	/* power_knight: */
	0xB500,            /* 018 push {lr}: KnightSoul's: DarkDril (0x38, 0x0800FA7C) */
	0x4809,            /* 01A ldr r0, =chip_knight */
	0xE001,            /* 01C b power_tail */
	/* power_toad: */
	0xB500,            /* 01E push {lr}: ToadSoul's: DarkWide (0x34, 0x0800FA54) */
	0x4808,            /* 020 ldr r0, =chip_toad */
	/* power_tail: */
	0x4B09,            /* 022 ldr r3, =w_writer: the chip writer's second entry (BN5's 0x0800FE70): its family in r0 */
	0xF000, 0xF802,    /* 024 bl call3 */
	0xBD00,            /* 028 pop {pc} */
	/* call1: */
	0x4708,            /* 02A bx r1 */
	/* call3: */
	0x4718,            /* 02C bx r3 */
	/* call4: */
	0x4720,            /* 02E bx r4 */
	/* chip_colonel: */
	0x0000, 0x0000,    /* 030 .word 0 */
	/* chip_shadow: */
	0x0000, 0x0000,    /* 034 .word 0 */
	/* chip_number: */
	0x0000, 0x0000,    /* 038 .word 0 */
	/* chip_tomahawk: */
	0x0000, 0x0000,    /* 03C .word 0 */
	/* chip_knight: */
	0x0000, 0x0000,    /* 040 .word 0 */
	/* chip_toad: */
	0x0000, 0x0000,    /* 044 .word 0 */
	/* w_writer: */
	0x26EB, 0x0801,    /* 048 .word 0x080126EB: the chip writer's second entry, no chip-used log (BN5's 0x0800FE70) */
	/* chaos_attack: */
	0xB5C0,            /* 04C push {r6, r7, lr}: BN5's 0x0800F330: the battle's counter 0 one more, */
	0x7DA8,            /* 04E ldrb r0, [r5, #0x16] */
	0x2100,            /* 050 movs r1, #0 */
	0x2201,            /* 052 movs r2, #1 */
	0x4B1D,            /* 054 ldr r3, =w_count */
	0xF7FF, 0xFFE9,    /* 056 bl call3 */
	0x6DAE,            /* 05A ldr r6, [r5, #0x58]: ... then his AI data's +0x11 power attack run */
	0x27A0,            /* 05C movs r7, #0xA0 */
	0x19BF,            /* 05E adds r7, r7, r6 */
	0x491B,            /* 060 ldr r1, =w_powers */
	0x6809,            /* 062 ldr r1, [r1] */
	0x7C70,            /* 064 ldrb r0, [r6, #0x11] */
	0x0080,            /* 066 lsls r0, r0, #2 */
	0x5809,            /* 068 ldr r1, [r1, r0] */
	0xF7FF, 0xFFDE,    /* 06A bl call1 */
	0xBDC0,            /* 06E pop {r6, r7, pc} */
	/* chaos_hit: */
	0x2010,            /* 070 movs r0, #0x10: BN5's 0x080F043C: +0x48's 0x10 cleared, */
	0x4B18,            /* 072 ldr r3, =w_clear48 */
	0xF7FF, 0xFFDA,    /* 074 bl call3 */
	0x7DA8,            /* 078 ldrb r0, [r5, #0x16]: ... 0x40 where his side is not the battle's, */
	0x4B18,            /* 07A ldr r3, =w_side */
	0xF7FF, 0xFFD6,    /* 07C bl call3 */
	0x4200,            /* 080 tst r0, r0 */
	0xD103,            /* 082 bne hit_attack */
	0x2040,            /* 084 movs r0, #0x40 */
	0x4B16,            /* 086 ldr r3, =w_flag40 */
	0xF7FF, 0xFFD0,    /* 088 bl call3 */
	/* hit_attack: */
	0xF7FF, 0xFFDE,    /* 08C bl chaos_attack: ... his chaos attack, as setAttack5 */
	0x4B14,            /* 090 ldr r3, =w_attack5 */
	0xF7FF, 0xFFCB,    /* 092 bl call3 */
	0x2001,            /* 096 movs r0, #1 */
	0xBDD0,            /* 098 pop {r4, r6, r7, pc}: (the consumer's own return) */
	/* chaos_miss: */
	0x4814,            /* 09A ldr r0, =w_bit20000: BN5's 0x080F0470: +0x48's 0x20000 set, 0x10 cleared, */
	0x4B0E,            /* 09C ldr r3, =w_set48 */
	0xF7FF, 0xFFC5,    /* 09E bl call3 */
	0x2010,            /* 0A2 movs r0, #0x10 */
	0x4B0B,            /* 0A4 ldr r3, =w_clear48 */
	0xF7FF, 0xFFC1,    /* 0A6 bl call3 */
	0x7DA8,            /* 0AA ldrb r0, [r5, #0x16]: ... 0x40 where his side is not the battle's, */
	0x4B0B,            /* 0AC ldr r3, =w_side */
	0xF7FF, 0xFFBD,    /* 0AE bl call3 */
	0x4200,            /* 0B2 tst r0, r0 */
	0xD103,            /* 0B4 bne miss_attack */
	0x2040,            /* 0B6 movs r0, #0x40 */
	0x4B09,            /* 0B8 ldr r3, =w_flag40 */
	0xF7FF, 0xFFB7,    /* 0BA bl call3 */
	/* miss_attack: */
	0x20C7,            /* 0BE movs r0, #0xC7: ... the failure (BN5's family 0x39, BN6's 0xC7) as setAttack6 */
	0x2206,            /* 0C0 movs r2, #6 */
	0x4B09,            /* 0C2 ldr r3, =w_attack */
	0xF7FF, 0xFFB2,    /* 0C4 bl call3 */
	0x2001,            /* 0C8 movs r0, #1 */
	0xBDD0,            /* 0CA pop {r4, r6, r7, pc} */
	/* w_count: */
	0xAB47, 0x0800,    /* 0CC .word 0x0800AB47: a battle counter of a side's, one more (BN5's 0x08009326) */
	/* w_powers: */
	0x17D0, 0x0801,    /* 0D0 .word 0x080117D0: the word naming the power attacks (grown: soulpower.c) */
	/* w_clear48: */
	0x031D, 0x0801,    /* 0D4 .word 0x0801031D: AIData +0x48's bits cleared (BN5's 0x0800E0A0) */
	/* w_set48: */
	0x0313, 0x0801,    /* 0D8 .word 0x08010313: ... set (BN5's 0x0800E096) */
	/* w_side: */
	0xA9ED, 0x0800,    /* 0DC .word 0x0800A9ED: the side against the battle's (BN5's 0x080091EA) */
	/* w_flag40: */
	0xDACD, 0x0801,    /* 0E0 .word 0x0801DACD: (BN5's 0x0801A410) */
	/* w_attack5: */
	0x1695, 0x0801,    /* 0E4 .word 0x08011695: setAttack5 (BN5's 0x0800F214) */
	/* w_attack: */
	0x1699, 0x0801,    /* 0E8 .word 0x08011699: setAttack's common part, the kind in r2 (BN5's setAttack6 0x0800F218) */
	/* w_bit20000: */
	0x0000, 0x0002,    /* 0EC .word 0x00020000 */
	/* fail_family: */
	0xB500,            /* 0F0 push {lr}: BN5's family 0x39 (0x080EE724): its phase's routine */
	0x7838,            /* 0F2 ldrb r0, [r7] */
	0x492D,            /* 0F4 ldr r1, =w_phases */
	0x5809,            /* 0F6 ldr r1, [r1, r0] */
	0xF7FF, 0xFF97,    /* 0F8 bl call1 */
	0xBD00,            /* 0FC pop {pc} */
	/* fail_0: */
	0xB500,            /* 0FE push {lr}: phase 0 (0x080EE744): the battle's flag 4 and his animation 0xC, 5 frames */
	0x7878,            /* 100 ldrb r0, [r7, #1] */
	0x4200,            /* 102 tst r0, r0 */
	0xD10F,            /* 104 bne f0_run */
	0x4B2D,            /* 106 ldr r3, =w_stopped */
	0xF7FF, 0xFF90,    /* 108 bl call3 */
	0xD119,            /* 10C bne f0_done */
	0x2004,            /* 10E movs r0, #4 */
	0x4B2B,            /* 110 ldr r3, =w_flag_set */
	0xF7FF, 0xFF8B,    /* 112 bl call3 */
	0x200C,            /* 116 movs r0, #0xC */
	0x4B2A,            /* 118 ldr r3, =w_anim */
	0xF7FF, 0xFF87,    /* 11A bl call3 */
	0x2005,            /* 11E movs r0, #5 */
	0x8238,            /* 120 strh r0, [r7, #0x10] */
	0x2004,            /* 122 movs r0, #4 */
	0x7078,            /* 124 strb r0, [r7, #1] */
	/* f0_run: */
	0x8A38,            /* 126 ldrh r0, [r7, #0x10] */
	0x3801,            /* 128 subs r0, #1 */
	0x8238,            /* 12A strh r0, [r7, #0x10] */
	0xDA01,            /* 12C bge f0_tick */
	0x2004,            /* 12E movs r0, #4 */
	0x8038,            /* 130 strh r0, [r7] */
	/* f0_tick: */
	0x8A78,            /* 132 ldrh r0, [r7, #0x12] */
	0x3001,            /* 134 adds r0, #1 */
	0x8278,            /* 136 strh r0, [r7, #0x12] */
	0x0880,            /* 138 lsrs r0, r0, #2 */
	0xD302,            /* 13A blo f0_done */
	0x4B22,            /* 13C ldr r3, =w_tick */
	0xF7FF, 0xFF75,    /* 13E bl call3 */
	/* f0_done: */
	0xBD00,            /* 142 pop {pc} */
	/* fail_4: */
	0xB510,            /* 144 push {r4, lr}: phase 4 (0x080EE782): animation 1, sound 0x8E, 0x18 frames; at 8 the Dark Soul */
	0x7878,            /* 146 ldrb r0, [r7, #1] */
	0x4200,            /* 148 tst r0, r0 */
	0xD10B,            /* 14A bne f4_run */
	0x2001,            /* 14C movs r0, #1 */
	0x4B1D,            /* 14E ldr r3, =w_anim */
	0xF7FF, 0xFF6C,    /* 150 bl call3 */
	0x208E,            /* 154 movs r0, #0x8E */
	0x4B1D,            /* 156 ldr r3, =w_sound */
	0xF7FF, 0xFF68,    /* 158 bl call3 */
	0x2018,            /* 15C movs r0, #0x18 */
	0x8238,            /* 15E strh r0, [r7, #0x10] */
	0x2004,            /* 160 movs r0, #4 */
	0x7078,            /* 162 strb r0, [r7, #1] */
	/* f4_run: */
	0x8A38,            /* 164 ldrh r0, [r7, #0x10] */
	0x2808,            /* 166 cmp r0, #8 */
	0xD111,            /* 168 bne f4_count */
	0x7DA8,            /* 16A ldrb r0, [r5, #0x16]: (the one out ended, BN5's 0x0810436C, and the lists: the engine's C) */
	0x4B18,            /* 16C ldr r3, =w_out */
	0xF7FF, 0xFF5D,    /* 16E bl call3 */
	0x4B18,            /* 172 ldr r3, =w_panel: a panel for it (BN5's 0x0801021E) */
	0xF7FF, 0xFF5A,    /* 174 bl call3 */
	0x4200,            /* 178 tst r0, r0 */
	0xD008,            /* 17A beq f4_count */
	0x1C0A,            /* 17C adds r2, r1, #0 */
	0x1C01,            /* 17E adds r1, r0, #0 */
	0x7DAB,            /* 180 ldrb r3, [r5, #0x16] */
	0x2001,            /* 182 movs r0, #1 */
	0x4043,            /* 184 eors r3, r0 */
	0x4814,            /* 186 ldr r0, =w_dark */
	0x4C14,            /* 188 ldr r4, =w_spawn: (BN5's 0x08006AAE: r3 the other side, so through r4) */
	0xF7FF, 0xFF50,    /* 18A bl call4 */
	/* f4_count: */
	0x8A38,            /* 18E ldrh r0, [r7, #0x10] */
	0x3801,            /* 190 subs r0, #1 */
	0x8238,            /* 192 strh r0, [r7, #0x10] */
	0xDA01,            /* 194 bge f4_tick */
	0x2008,            /* 196 movs r0, #8 */
	0x8038,            /* 198 strh r0, [r7] */
	/* f4_tick: */
	0x8A78,            /* 19A ldrh r0, [r7, #0x12] */
	0x3001,            /* 19C adds r0, #1 */
	0x8278,            /* 19E strh r0, [r7, #0x12] */
	0x0880,            /* 1A0 lsrs r0, r0, #2 */
	0xD302,            /* 1A2 blo f4_done */
	0x4B08,            /* 1A4 ldr r3, =w_tick */
	0xF7FF, 0xFF41,    /* 1A6 bl call3 */
	/* f4_done: */
	0xBD10,            /* 1AA pop {r4, pc} */
	/* w_phases: */
	0x0000, 0x0000,    /* 1AC .word 0 */
	/* fail_phases: */
	0x0000, 0x0000,    /* 1B0 .word 0 */
	0x0000, 0x0000,    /* 1B4 .word 0 */
	0x0000, 0x0000,    /* 1B8 .word 0 */
	/* w_stopped: */
	0xA099, 0x0800,    /* 1BC .word 0x0800A099: time stopped (BN5's 0x08008A78) */
	/* w_flag_set: */
	0xA2D9, 0x0800,    /* 1C0 .word 0x0800A2D9: BattleState +0x32's bits set (BN5's 0x08008C88) */
	/* w_anim: */
	0xF2B7, 0x0800,    /* 1C4 .word 0x0800F2B7: his animation (BN5's 0x0800D1D2) */
	/* w_tick: */
	0x2DB1, 0x0800,    /* 1C8 .word 0x08002DB1: (BN5's 0x08002C1C) */
	/* w_sound: */
	0x05CD, 0x0800,    /* 1CC .word 0x080005CD: PlaySoundEffect (BN5's 0x08000598) */
	/* w_out: */
	0x0000, 0x0000,    /* 1D0 .word 0 */
	/* w_panel: */
	0x29F5, 0x0801,    /* 1D4 .word 0x080129F5: a panel of the other side's for it (BN5's 0x0801021E) */
	/* w_dark: */
	0x0000, 0x0000,    /* 1D8 .word 0 */
	/* w_spawn: */
	0x7697, 0x0800,    /* 1DC .word 0x08007697: an enemy spawned: r0 its id, r1 r2 its panel, r3 its side (BN5's 0x08006AAE) */
	/* fail_8: */
	0xB510,            /* 1E0 push {r4, lr}: phase 8 (0x080EE7E4): out of the Soul at once, BN6's leave's steps (0x080156B0-0x08015718) for BN5's */
	0x212C,            /* 1E2 movs r1, #0x2C */
	0x4B2B,            /* 1E4 ldr r3, =w_form */
	0xF7FF, 0xFF21,    /* 1E6 bl call3 */
	0x4B2B,            /* 1EA ldr r3, =w_exit */
	0xF7FF, 0xFF1E,    /* 1EC bl call3 */
	0x2000,            /* 1F0 movs r0, #0 */
	0x2100,            /* 1F2 movs r1, #0 */
	0x4B29,            /* 1F4 ldr r3, =w_base */
	0xF7FF, 0xFF19,    /* 1F6 bl call3 */
	0x2080,            /* 1FA movs r0, #0x80 */
	0x4B28,            /* 1FC ldr r3, =w_load */
	0xF7FF, 0xFF15,    /* 1FE bl call3 */
	0x4B28,            /* 202 ldr r3, =w_init */
	0xF7FF, 0xFF12,    /* 204 bl call3 */
	0x4B27,            /* 208 ldr r3, =w_shadow */
	0xF7FF, 0xFF0F,    /* 20A bl call3 */
	0x2000,            /* 20E movs r0, #0 */
	0x4B26,            /* 210 ldr r3, =w_anim8 */
	0xF7FF, 0xFF0B,    /* 212 bl call3 */
	0x7C28,            /* 216 ldrb r0, [r5, #0x10] */
	0x4B25,            /* 218 ldr r3, =w_anim_set */
	0xF7FF, 0xFF07,    /* 21A bl call3 */
	0x4B21,            /* 21E ldr r3, =w_init */
	0xF7FF, 0xFF04,    /* 220 bl call3 */
	0x4B23,            /* 224 ldr r3, =w_flip */
	0xF7FF, 0xFF01,    /* 226 bl call3 */
	0x4B23,            /* 22A ldr r3, =w_place */
	0xF7FF, 0xFEFE,    /* 22C bl call3 */
	0x4B22,            /* 230 ldr r3, =w_tick8 */
	0xF7FF, 0xFEFB,    /* 232 bl call3 */
	0x4B22,            /* 236 ldr r3, =w_calm */
	0xF7FF, 0xFEF8,    /* 238 bl call3 */
	0x212C,            /* 23C movs r1, #0x2C */
	0x2200,            /* 23E movs r2, #0 */
	0x4B20,            /* 240 ldr r3, =w_form_set */
	0xF7FF, 0xFEF3,    /* 242 bl call3 */
	0x2000,            /* 246 movs r0, #0 */
	0x4B1F,            /* 248 ldr r3, =w_name */
	0xF7FF, 0xFEEF,    /* 24A bl call3 */
	0x4B1F,            /* 24E ldr r3, =w_state */
	0xF7FF, 0xFEEC,    /* 250 bl call3 */
	0x4B1E,            /* 254 ldr r3, =w_unbeast */
	0xF7FF, 0xFEE9,    /* 256 bl call3 */
	0x212C,            /* 25A movs r1, #0x2C */
	0x4B0D,            /* 25C ldr r3, =w_form */
	0xF7FF, 0xFEE5,    /* 25E bl call3 */
	0x2200,            /* 262 movs r2, #0 */
	0x4B1B,            /* 264 ldr r3, =w_enter */
	0xF7FF, 0xFEE1,    /* 266 bl call3 */
	0x2000,            /* 26A movs r0, #0 */
	0x64E8,            /* 26C str r0, [r5, #0x4C] */
	0x6DA9,            /* 26E ldr r1, [r5, #0x58] */
	0x6688,            /* 270 str r0, [r1, #0x68] */
	0x203D,            /* 272 movs r0, #0x3D */
	0x4B18,            /* 274 ldr r3, =w_requests */
	0xF7FF, 0xFED9,    /* 276 bl call3 */
	0x4818,            /* 27A ldr r0, =w_bit20000b */
	0x4B18,            /* 27C ldr r3, =w_clear48b */
	0xF7FF, 0xFED5,    /* 27E bl call3 */
	0x2004,            /* 282 movs r0, #4 */
	0x4B17,            /* 284 ldr r3, =w_flag_clear */
	0xF7FF, 0xFED1,    /* 286 bl call3 */
	0x4B17,            /* 28A ldr r3, =w_end */
	0xF7FF, 0xFECE,    /* 28C bl call3 */
	0xBD10,            /* 290 pop {r4, pc} */
	0x46C0,            /* 292 nop (align) */
	/* w_form: */
	0x3775, 0x0801,    /* 294 .word 0x08013775: his battle NaviStats' byte r1 (BN5's 0x08010DEE) */
	/* w_exit: */
	0x1385, 0x0801,    /* 298 .word 0x08011385: his form left by its overlay (BN6's leave's, 0x080156B6) */
	/* w_base: */
	0xFC9F, 0x0800,    /* 29C .word 0x0800FC9F: (BN5's 0x0800DA32) */
	/* w_load: */
	0x26E5, 0x0800,    /* 2A0 .word 0x080026E5: sprite_load (BN5's 0x08002650) */
	/* w_init: */
	0x26A5, 0x0800,    /* 2A4 .word 0x080026A5: (BN5's 0x08002610) */
	/* w_shadow: */
	0x2E3D, 0x0800,    /* 2A8 .word 0x08002E3D: (BN5's 0x08002CA8) */
	/* w_anim8: */
	0xF2B7, 0x0800,    /* 2AC .word 0x0800F2B7: his animation */
	/* w_anim_set: */
	0x2DA5, 0x0800,    /* 2B0 .word 0x08002DA5: (BN5's 0x08002C10) */
	/* w_flip: */
	0xE457, 0x0800,    /* 2B4 .word 0x0800E457: (BN5's 0x0800C88E) */
	/* w_place: */
	0x2F5D, 0x0800,    /* 2B8 .word 0x08002F5D: (BN5's 0x08002DC0) */
	/* w_tick8: */
	0x2DB1, 0x0800,    /* 2BC .word 0x08002DB1: (BN5's 0x08002C1C) */
	/* w_calm: */
	0x58CD, 0x0801,    /* 2C0 .word 0x080158CD: his mood calm, BN6's leave's (BN5's calm 0x08011A8C) */
	/* w_form_set: */
	0x3755, 0x0801,    /* 2C4 .word 0x08013755: his battle NaviStats' byte r1 set to r2 (BN5's 0x08010DCE) */
	/* w_name: */
	0x5B23, 0x0801,    /* 2C8 .word 0x08015B23: his NameID by form, BN6's leave's */
	/* w_state: */
	0x44C1, 0x0801,    /* 2CC .word 0x080144C1: (BN5's 0x08011B18, but its Colonel's side's bit: soullife.c's) */
	/* w_unbeast: */
	0x43B5, 0x0801,    /* 2D0 .word 0x080143B5: BN6's leave's */
	/* w_enter: */
	0x1269, 0x0801,    /* 2D4 .word 0x08011269: the form's overlay entered (BN5's 0x0800F01C) */
	/* w_requests: */
	0xFFEF, 0x0800,    /* 2D8 .word 0x0800FFEF: AIData +0x44's requests cleared (BN5's 0x0800DD4E) */
	/* w_bit20000b: */
	0x0000, 0x0002,    /* 2DC .word 0x00020000 */
	/* w_clear48b: */
	0x031D, 0x0801,    /* 2E0 .word 0x0801031D: AIData +0x48's bits cleared */
	/* w_flag_clear: */
	0xA2E5, 0x0800,    /* 2E4 .word 0x0800A2E5: BattleState +0x32's bits cleared (BN5's 0x08008C94) */
	/* w_end: */
	0x1715, 0x0801,    /* 2E8 .word 0x08011715: the attack's end (BN5's 0x0800F29E) */
	/* out_stub: */
	0x4770,            /* 2EC bx lr: (hooked: the engine's C, BN5's 0x0810436C and the lists) */
	/* zero: */
	0x2000,            /* 2EE movs r0, #0: (no panel: where no Dark Soul is installed, the panel's call goes here) */
	0x4770,            /* 2F0 bx lr */
};

void soulchaos_thumb(uint8_t out[CHAOS_THUMB_SIZE]) {
	for (size_t i = 0; i < sizeof thumb / sizeof *thumb; ++i) put16(out + 2 * i, thumb[i]);
}
