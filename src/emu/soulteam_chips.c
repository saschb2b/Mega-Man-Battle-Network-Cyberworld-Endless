/* Team ProtoMan's Souls' rules for the chips MegaMan uses in BN6's engine (soulteam_chips.h). */
#include "soulteam_chips.h"

#include <stdint.h>
#include <stdio.h>

#include "bn6.h"
#include "bn6_attacks.h"
#include "bn6_chips.h"
#include "bn6_forms.h"
#include "bn6_navis.h"
#include "bn6_panels.h"
#include "bytes.h"
#include "chips.h"
#include "claims.h"
#include "emu.h"
#include "rom.h"
#include "soul_room.h"
#include "soulchips.h"
#include "soulkinds.h"
#include "soullife.h"
#include "soulteam.h"
#include "thumb_reloc.h"
#include "xpanels.h"

#define BUS 0x08000000u
#define MODIFIED_DOUBLE 0x8000u   /* a chip's damage doubled by its modifier (BN5's 0x0801045C, BN6's 0x08012DE4: the same bit) */
#define LAVA_CODE_SIZE 28    /* the Thumb NapalmSoul's lava jumps to (lava_code), */
#define PROP_CODE_SIZE 16    /* ... GyroSoul's propeller spent (prop_spend_code), */
#define PROP_SET_SIZE 36     /* ... and set (prop_set_code) */
#define AI_PROPELLER 0x0D    /* his AI data's propeller (BN5's +0x0D, 0x080102EE; no code of Gregar's reads or writes BN6's +0x0D) */
#define KIND_WIND 10         /* BN5_CHIP_KIND's Wind */
#define LAVA_BONUS 10        /* NapalmSoul on lava: his next attack chip's power more (BN5's 0x08012644) */
#define HAND_SIDE 0x50       /* a side's hand after OK, BN6_BATTLE_HAND's step (BN6_HAND_ADDR) */
_Static_assert(SOULS_TEAM_THUMB + LAVA_CODE_SIZE + PROP_CODE_SIZE + PROP_SET_SIZE <= SOULS_TEAM_THUMB_END,
	"Team ProtoMan's Thumb past its room (soul_room.h)");
#define PROP_SPEND (SOULS_TEAM_THUMB + LAVA_CODE_SIZE)
#define PROP_SET (PROP_SPEND + PROP_CODE_SIZE)

static int breaks;
static bool done;

static int side_of(uint32_t obj) { return hook_read8(obj + BN6_T1_ALLIANCE) & 1; }
static int soul_of(uint32_t obj) { return done ? soul_id_of(obj) : 0; }
static uint8_t flags_of(int chip) {
	return chip > 0 && chip < CHIPS_IDS ? hook_read8(chips_table_bus() + (uint32_t)chip * BN6_CHIP_RECORD_SIZE + BN6_CHIP_EFFECT_FLAGS) : 0;
}
static uint32_t panel_at(uint32_t x, uint32_t y) { return BN6_FIELD_PANELS + (y * BN6_FIELD_COLUMNS + x) * BN6_PANEL_SIZE; }

/* GyroSoul's propeller as the chip's modifier reads it (BN5's 0x08010264): set, an attack chip's modifier is the
 * propeller's alone, 3 and the damage doubled for a Wind chip or one of no kind, not left alone (0x080102FA), none for any
 * other, the emotion's not looked at; on to the Thumb that spends it (case 3, 0x0800FFC2: +0x0D 0) and plays the sound */
HookAct soulteam_chips_modified(HookRegs *r, int chip) {
	uint32_t ai = hook_read32(r->r[5] + BN6_T1_AI_DATA);
	uint8_t f = flags_of(chip);
	if (!ai || !hook_read8(ai + AI_PROPELLER) || !(f & BN6_CHIP_SHOWS_POWER) || !done) return HOOK_CONTINUE;
	int kind = soulkinds_of(chip);
	uint16_t before = hook_read16(r->r[7] + BN6_ATTACK_DAMAGE);   /* (the damage the modifier was given, kept until the strh) */
	if ((f & BN6_CHIP_LEFT_ALONE) || (kind != KIND_WIND && kind != SOULKIND_NONE)) {
		r->r[0] = before;
		r->r[1] = 0;
		return HOOK_CONTINUE;
	}
	r->r[0] = before | MODIFIED_DOUBLE;
	r->r[12] = PROP_SPEND | 1;
	return HOOK_JUMP;
}

/* (BN6_PLAYER_TIMERS, his timers' first read, r4 his AI data, r5 him) NapalmSoul's lava step, BN5's 0x080125FA before its
 * timers (0x0800E53A): not paused, he on a lava panel, the next chip of his hand an attack chip (its power shown): its power
 * 10 more as Atk+ chips add theirs, and on to the Thumb that makes the panel plain */
static HookAct lava(HookRegs *r, void *user) {
	uint32_t me = r->r[5];
	if (soul_of(me) != SOUL_NAPALM || (breaks & SOULCHIPS_BREAK_LAVA) || hook_read8(hook_read32(BN6_TOOLKIT + BN6_TOOLKIT_PAUSE) + BN6_PAUSED))
		return HOOK_CONTINUE;
	uint32_t x = hook_read8(me + BN6_T1_PANEL_X), y = hook_read8(me + BN6_T1_PANEL_Y);
	if (x >= BN6_FIELD_COLUMNS || hook_read8(panel_at(x, y) + BN6_PANEL_TYPE) != XPANEL_LAVA) return HOOK_CONTINUE;
	uint32_t hand = BN6_BATTLE_HAND + (uint32_t)side_of(me) * HAND_SIDE, at = 2u * hook_read8(hand + BN6_HAND_AT);
	if (!(flags_of(hook_read16(hand + BN6_HAND_CHIPS + at)) & BN6_CHIP_SHOWS_POWER)) return HOOK_CONTINUE;
	hook_write16(hand + BN6_HAND_BONUS + at, (uint16_t)(hook_read16(hand + BN6_HAND_BONUS + at) + LAVA_BONUS));
	r->r[12] = (uint32_t)(uintptr_t)user | 1;
	return HOOK_JUMP;
}

/* (BN6_CHIP_USE_MODIFIED, past the modifier's switch: r7 its attack's variables, r5 MegaMan) GyroSoul's propeller set by a Wind
 * chip not left alone (BN5's 0x080102CA): on to the Thumb that sets it and plays BN5's sound; and his overlay spun as it stands
 * after the chip (0x11 while set, 0 once spent: BN5's 0x080C3876 from his steps, the chip's first frame in its trace) */
static HookAct propeller_set(HookRegs *r, void *user) {
	(void)user;
	uint32_t me = r->r[5], ai = hook_read32(me + BN6_T1_AI_DATA), ov = hook_read32(me + BN6_T1_OVERLAY);
	int chip = hook_read16(r->r[7] + BN6_ATTACK_CHIP);
	if (soul_of(me) != SOUL_GYRO || !ai || (breaks & SOULCHIPS_BREAK_NAPALM)) return HOOK_CONTINUE;
	bool sets = soulkinds_of(chip) == KIND_WIND && !(flags_of(chip) & BN6_CHIP_LEFT_ALONE);
	/* (BN6's twin of BN5's overlay takes its animation's offset from its +7, BN5's from its +0x60: legit.json's masks, 049) */
	if (ov) hook_write8(ov + BN6_OVERLAY_OFFSET, (sets || hook_read8(ai + AI_PROPELLER)) ? BN6_OVERLAY_SPUN : 0);
	if (!sets) return HOOK_CONTINUE;
	r->r[12] = PROP_SET | 1;
	return HOOK_JUMP;
}

/* ---- the install ---- */

/* the Thumb GyroSoul's propeller jumps to when spent: the hooked strh, his +0x0D 0, on to the modifier's sound */
static void prop_spend_code(uint8_t out[PROP_CODE_SIZE]) {
	static const uint16_t code[] = {
		0x8138,           /* strh r0,[r7,#8] */
		0x6DA9,           /* ldr r1,[r5,#0x58]: his AI data */
		0x2000,           /* movs r0,#0 */
		0x7348,           /* strb r0,[r1,#0xd]: the propeller spent */
		0x4B00,           /* ldr r3,[pc,#0]: the way on */
		0x4718,           /* bx r3 */
	};
	for (size_t i = 0; i < sizeof code / sizeof *code; ++i) put16(out + 2 * i, code[i]);
	put32(out + 12, BN6_CHIP_MODIFIED_SOUND | 1);
}

/* ... and when set: his +0x0D 1, the sound's slot played (BN6_PLAY_SOUND), the hooked ldrh, on to the use's next */
static bool prop_set_code(uint32_t at, uint8_t out[PROP_SET_SIZE]) {
	static const uint16_t code[] = {
		0x6DA9,           /* 00 ldr r1,[r5,#0x58] */
		0x2001,           /* 02 movs r0,#1 */
		0x7348,           /* 04 strb r0,[r1,#0xd]: the propeller set */
		0x4804,           /* 06 ldr r0,[pc,#16]: the sound */
		0x4B04,           /* 08 ldr r3,[pc,#16]: PlaySoundEffect */
		0, 0,             /* 0A bl the bx r3 at 0x14 */
		0x8938,           /* 0E ldrh r0,[r7,#8]: the hooked instruction (BN6_CHIP_USE_MODIFIED's) */
		0x4B03,           /* 10 ldr r3,[pc,#12]: the way on */
		0x4718,           /* 12 bx r3 */
		0x4718,           /* 14 bx r3 */
		0x46C0,           /* 16 nop */
	};
	for (size_t i = 0; i < sizeof code / sizeof *code; ++i) put16(out + 2 * i, code[i]);
	put32(out + 24, SOUL_TEAM_PROPELLER_SLOT);
	put32(out + 28, BN6_PLAY_SOUND | 1);
	put32(out + 32, BN6_CHIP_USE_MODIFIED_NEXT | 1);
	return thumb_bl_write(out, 10, at, at + 20);
}

/* the Thumb NapalmSoul's lava jumps to: the panel under him plain (BN6_SET_PANEL_TYPE, r0 column, r1 row, r2 the type), the
 * hooked ldrb, on to the routine's next */
static bool lava_code(uint32_t at, uint8_t out[LAVA_CODE_SIZE]) {
	static const uint16_t code[] = {
		0x7CA8,           /* ldrb r0,[r5,#0x12] */
		0x7CE9,           /* ldrb r1,[r5,#0x13] */
		0x2202,           /* movs r2,#2 (BN6_PANEL_PLAIN) */
		0x4B03,           /* ldr r3,[pc,#12]: the panel's routine */
		0, 0,             /* bl the bx below */
		0x7E60,           /* ldrb r0,[r4,#0x19]: the hooked instruction (BN6_PLAYER_TIMERS's, checked by expected) */
		0x4B02,           /* ldr r3,[pc,#8]: the way on */
		0x4718,           /* bx r3 */
		0x46C0,           /* nop */
	};
	for (size_t i = 0; i < sizeof code / sizeof *code; ++i) put16(out + 2 * i, code[i]);
	put32(out + 20, BN6_SET_PANEL_TYPE | 1);
	put32(out + 24, BN6_PLAYER_TIMERS_NEXT | 1);
	return BN6_PANEL_PLAIN == 2 && thumb_bl_write(out, 8, at, at + 16);
}

static bool expected(void) {
	static const struct { uint32_t at; uint16_t op; } code[] = {
		{ BN6_PLAYER_TIMERS, 0x7E60 }, { BN6_CHIP_USE_MODIFIED, 0x8938 }, { BN6_CHIP_MODIFIED_SOUND, 0x2087 }, { BN6_SET_PANEL_TYPE, 0xB570 },
	};
	for (size_t i = 0; i < sizeof code / sizeof *code; ++i)
		if (rom_u16(code[i].at - BUS) != code[i].op) {
			fprintf(stderr, "soulteam_chips: %08x is not the instruction expected\n", (unsigned)code[i].at);
			return false;
		}
	return true;
}

bool soulteam_chips_install(int brk) {
	if (done) return true;
	if (!R.data || !emu_ready() || !expected()) return false;
	breaks = brk;
	uint8_t lava_stub[LAVA_CODE_SIZE], spend[PROP_CODE_SIZE], set[PROP_SET_SIZE];
	if (!lava_code(SOULS_TEAM_THUMB, lava_stub) || !prop_set_code(PROP_SET, set)) return false;
	prop_spend_code(spend);
	claims_begin(SOULCHIPS_CLAIMS);
	emu_write(SOULS_TEAM_THUMB, lava_stub, sizeof lava_stub);
	emu_write(PROP_SPEND, spend, sizeof spend);
	emu_write(PROP_SET, set, sizeof set);
	bool ok = emu_hook(BN6_PLAYER_TIMERS, lava, (void *)(uintptr_t)SOULS_TEAM_THUMB) && emu_hook(BN6_CHIP_USE_MODIFIED, propeller_set, NULL);
	claims_end();
	done = ok;
	printf("soulteam_chips: Team ProtoMan's Souls' chips%s\n", ok ? ": NapalmSoul's lava, GyroSoul's propeller" : " NOT placed whole");
	return ok;
}
