/* ColonelSoul's own in BN6's engine (soulcolonel.h). */
#include "soulcolonel.h"

#include <stdint.h>
#include <stdio.h>

#include "bn5.h"
#include "bn6.h"
#include "bn6_chips.h"
#include "bn6_forms.h"
#include "bytes.h"
#include "chips.h"
#include "claims.h"
#include "emu.h"
#include "rom.h"
#include "soul_room.h"
#include "soulcode.h"
#include "soulkinds.h"
#include "soullife.h"
#include "thumb_reloc.h"

#define BUS 0x08000000u
#define FREE_HALF 0xFFFFu          /* the free space as emu_init pads it */
#define THUMB_BX_LR 0x4770u
#define SOUL_COLONEL 7
#define POWER_CHIP_SHOT 0x13       /* BN5's power attacks: his chip shot, */
#define POWER_DIVIDE 0x14          /* ... and his ScreenDivide (0x080124BA-0x080124C4) */
#define AI_POWER_B 0x07            /* AI data +7, BPwrAtk: his charged shot's power attack */
#define ELEMENT_BUTTON 8           /* the button's two elements, 8 and 9, their types 8 and 9 (BN5's 0x08023D74) */
#define ELEMENT_LINK_3 0x03        /* an element's +2 and +3, elements the cursor goes on to (0xFF none, 0x08027F84's fix-up): element 8's +3 */
#define ELEMENT_LINK_2 0x02        /* ... 0xB (the emblem, UNITE) and element 9's +2 7 (BN5's 0x08023D84-0x08023D8A) */
#define STATE_LIT 0
#define STATE_GREY 1
#define STATE_CHOSEN 2
#define BUTTON_SLOT 0x04           /* element 8's: the slot of the chip it took, */
#define BUTTON_PICK 0x05           /* ... its place among the picks, */
#define BUTTON_CHIP 0x08           /* ... and the chip (u32) */
#define PICKS_FIRST 0x47           /* the screen's +0x48 on its picks' slots, read as [r5 + 0x47 + count] (0x08024C20) */
#define LIBRARY_STANDARD 0         /* BN6_CHIP_LIBRARY_TYPE: a standard chip */
#define PANEL_AT 0x0800BD15u       /* the word BN5's panel getter leaves in r2 (0x0800AFC4 ldr r2: its routine), which the strike's spawn stores */
#define SITES 4
#define DECK_SPENT 0xFFFFu         /* a deck entry spent at OK (BN5's 0x080250C4, BN6's 0x0802943C) */
#define ART_SIZE 0x180             /* the button's tiles on the chips' screen, one state's */
#define INFO_SIZE 0x540            /* its picture in the chip's window */
#define PAL_SIZE 0x20
#define ROM5_SIZE 0x800000u        /* BN5's ROM's bytes (8 MiB) */

static int breaks;
static bool done;
static uint32_t phase0, phase1, draw, wrapper;   /* BN5's moved: the button's two phases, its draw, the strike's spawn */

static int side_of(uint32_t obj) { return hook_read8(obj + BN6_T1_ALLIANCE) & 1; }
static int soul_on(int side) { return done && soul_united(side) ? hook_read8(soul_field(side, BN6_SOUL_ID)) : 0; }
static int screen_side(void) { return hook_read8(BN6_BATTLE_SIDE) & 1; }
static uint32_t element(int k) { return BN6_CUSTOM_ELEMENTS + (uint32_t)k * BN6_ELEMENT_SIZE; }
static bool colonel_screen(void) { return soul_on(screen_side()) == SOUL_COLONEL; }
static bool button_chosen(void) {
	uint32_t b = element(ELEMENT_BUTTON);
	return colonel_screen() && hook_read8(b + BN6_ELEMENT_TYPE) == ELEMENT_BUTTON && hook_read8(b + BN6_ELEMENT_STATE) == STATE_CHOSEN;
}

/* ---- the button ---- */

/* (DustCross's elements' set-up as a Custom screen opens, its first instruction: r5 the screen) BN5's for ColonelSoul
 * (0x08023D70-0x08023D8A): elements 8 and 9 the button, grey */
static HookAct setup(HookRegs *r, void *user) {
	(void)user;
	(void)r;
	if (!colonel_screen() || hook_read8(BN6_CUSTOM_NAVI)) return HOOK_CONTINUE;
	uint32_t b = element(ELEMENT_BUTTON), b9 = element(ELEMENT_BUTTON + 1);
	hook_write8(b + BN6_ELEMENT_TYPE, ELEMENT_BUTTON);
	hook_write8(b9 + BN6_ELEMENT_TYPE, ELEMENT_BUTTON + 1);
	hook_write8(b + BN6_ELEMENT_STATE, STATE_GREY);
	hook_write8(b + ELEMENT_LINK_3, 0xB);
	hook_write8(b9 + ELEMENT_LINK_2, 7);
	return HOOK_RETURN;
}

/* BN5's rule (0x08024C04): lit where the last chip picked is a standard chip of no kind as BN5 has it, its power shown,
 * not left alone, no DarkChip; then the button keeps the chip, its slot and its place among the picks */
static bool lit(uint32_t screen, uint32_t b) {
	int n = hook_read8(screen + 8);
	if (!n) return false;
	int slot = hook_read8(screen + PICKS_FIRST + (uint32_t)n);
	uint32_t el = element(slot);
	if (hook_read8(el + BN6_ELEMENT_TYPE) != 0) return false;
	int chip = hook_read16(hook_read32(el + BN6_ELEMENT_DECK)) & 0x1FF;
	uint32_t rec = chips_table_bus() + (uint32_t)chip * BN6_CHIP_RECORD_SIZE;
	uint8_t fx = hook_read8(rec + BN6_CHIP_EFFECT_FLAGS);
	if (!(breaks & SOULCOLONEL_BREAK_RULE) &&
		(hook_read8(rec + BN6_CHIP_LIBRARY_TYPE) != LIBRARY_STANDARD || soulkinds_of(chip) != SOULKIND_NONE ||
		 !(fx & BN6_CHIP_SHOWS_POWER) || (fx & BN6_CHIP_LEFT_ALONE) || (fx & BN6_CHIP_DARK_CLASS)))
		return false;
	hook_write32(b + BUTTON_CHIP, (uint32_t)chip);
	hook_write8(b + BUTTON_SLOT, (uint8_t)slot);
	hook_write8(b + BUTTON_PICK, (uint8_t)(n - 1));
	return true;
}

/* (DustCross's lit rule after a pick, its first instruction: r5 the screen) BN5's ColonelSoul's */
static HookAct light(HookRegs *r, void *user) {
	(void)user;
	if (!colonel_screen()) return HOOK_CONTINUE;
	uint32_t b = element(ELEMENT_BUTTON);
	if (hook_read8(b + BN6_ELEMENT_TYPE) != ELEMENT_BUTTON || hook_read8(b + BN6_ELEMENT_STATE) == STATE_CHOSEN) return HOOK_RETURN;
	hook_write8(b + BN6_ELEMENT_STATE, lit(r->r[5], b) ? STATE_LIT : STATE_GREY);
	return HOOK_RETURN;
}

/* (mode 0x38's phases' table read: A on the button) BN5's two phases (0x080236BC, 0x080236E8), moved: the last pick
 * dropped into the button, its icon blinking, back to the chips */
static HookAct phases(HookRegs *r, void *user) {
	if (!colonel_screen()) return HOOK_CONTINUE;
	r->r[1] = SOULS_COLONEL;
	r->r[12] = (uint32_t)(uintptr_t)user | 1;
	return HOOK_JUMP;
}

/* (OK's end, where BN5's goes on: 0x08024EC4-0x08024ED6) the chip the button took kept for the turn, none without
 * (BN5's Custom record's +6, 0xFFFF as each screen opens, 0x080123CC, copied into the side's pending record at OK; the
 * block's 0: a byte written as it stands for every other OK) */
static HookAct ok(HookRegs *r, void *user) {
	(void)r;
	(void)user;
	uint16_t chip = button_chosen() ? (uint16_t)hook_read32(element(ELEMENT_BUTTON) + BUTTON_CHIP) : 0;
	uint32_t at = soul_field(screen_side(), BN6_SOUL_NEXT_CHIP);
	if (done && hook_read16(at) != chip) hook_write16(at, chip);
	return HOOK_CONTINUE;
}

/* (the turn's form check, its first instruction: r6 and r7 the sides' pending records, +8 each side's player) BN5's
 * first step (0x08011DD4 -> 0x080124A6): ColonelSoul's chip his, and his charged shot the chip shot with one, his
 * ScreenDivide without (his AI data +0x32 as BN5's, the Souls' block's BN6_SOUL_CHIP: 0 none) */
static HookAct turn(HookRegs *r, void *user) {
	(void)user;
	for (int k = 0; k < 2; ++k) {
		uint32_t obj = hook_read32((k ? r->r[7] : r->r[6]) + 8);
		if (obj < 0x02000000u || obj >= 0x02040000u) continue;
		int side = side_of(obj);
		if (soul_on(side) != SOUL_COLONEL) continue;
		uint16_t chip = (uint16_t)hook_read16(soul_field(side, BN6_SOUL_NEXT_CHIP));
		bool none = !chip;
		hook_write16(soul_field(side, BN6_SOUL_CHIP), none ? 0 : chip);
		int power = soulcode_power(none || (breaks & SOULCOLONEL_BREAK_TURN) ? POWER_DIVIDE : POWER_CHIP_SHOT);
		uint32_t ai = hook_read32(obj + BN6_T1_AI_DATA);
		if (power >= 0 && ai) hook_write8(ai + AI_POWER_B, (uint8_t)power);
	}
	return HOOK_CONTINUE;
}

/* (mode 4's last draw, and mode 0x24's end) the loaded chip drawn on the button (BN5's 0x080254F8, moved), as BN5's
 * modes draw it: through the Thumb that calls it and goes on as BN6 would */
static HookAct drawn(HookRegs *r, void *user) {
	if (!button_chosen()) return HOOK_CONTINUE;
	r->r[12] = (uint32_t)(uintptr_t)user | 1;
	return HOOK_JUMP;
}

/* the button's art as BN5's draws it, each where Gregar's loads DustCross's (BN6_DUST_ART and the window's three) */
typedef struct { uint32_t at, next, ours; } Art;
static const Art arts[] = {
	{ BN6_DUST_ART, BN6_DUST_ART_NEXT, SOULS_COLONEL_ART },
	{ BN6_DUST_INFO_ART, BN6_DUST_INFO_ART_NEXT, SOULS_COLONEL_INFO },
	{ BN6_DUST_INFO_PAL, BN6_DUST_INFO_PAL_NEXT, SOULS_COLONEL_INFO_PAL },
	{ BN6_DUST_NAME_PAL, BN6_DUST_NAME_PAL_NEXT, SOULS_COLONEL_NAME_PAL },
};

/* (each ldr r0 of DustCross's art) ColonelSoul's, BN5's copied: the button's tiles by its state (BN5's lit, grey, and its lit
 * again where BN6 has a chosen look, 0x0802415E), its picture and palettes in the chip's window (0x080245A4, 0x0802460A) */
static HookAct art(HookRegs *r, void *user) {
	const Art *a = user;
	if (!colonel_screen()) return HOOK_CONTINUE;
	r->r[0] = a->ours;
	r->r[12] = a->next | 1;
	return HOOK_JUMP;
}

/* (B on the chips, its first instruction: r5 the screen) BN5's first step (0x08024CEA-0x08024D28): the button chosen as the
 * last thing done (the picks as many as when it took its chip, its +5) gives its chip back to the picks, through the Thumb */
static HookAct undo(HookRegs *r, void *user) {
	(void)user;
	if (!button_chosen() || hook_read8(r->r[5] + 8) != hook_read8(element(ELEMENT_BUTTON) + BUTTON_PICK) || (breaks & SOULCOLONEL_BREAK_UNDO))
		return HOOK_CONTINUE;
	r->r[12] = SOUL_COLONEL_UNDO | 1;
	return HOOK_JUMP;
}

/* (OK's picks spent, its end) BN5's last step (0x080250CC-0x080250E6): the button's chip's deck entry spent as well */
static HookAct spent(HookRegs *r, void *user) {
	(void)r;
	(void)user;
	if (!button_chosen() || (breaks & SOULCOLONEL_BREAK_SPENT)) return HOOK_CONTINUE;
	uint32_t entry = hook_read32(element(hook_read8(element(ELEMENT_BUTTON) + BUTTON_SLOT)) + BN6_ELEMENT_DECK);
	if (entry >= BN6_EWRAM && entry < BN6_EWRAM_END) hook_write16(entry, DECK_SPENT);
	return HOOK_CONTINUE;
}

/* ---- the strike ---- */

/* (BN5's Colonel's side's flag set, 0x080CAD06, the stub: r5 the object) the side's in the Souls' block */
static HookAct strike_set(HookRegs *r, void *user) {
	(void)user;
	hook_write8(soul_field(hook_read8(r->r[5] + BN6_T1_ALLIANCE) & 1, BN6_SOUL_STRIKE), 1);
	return HOOK_RETURN;
}

static uint32_t panel_flags(int x, int y) {
	if ((unsigned)(x - 1) >= 6 || (unsigned)(y - 1) >= 3) return 0;
	return hook_read32(BN6_FIELD_PANELS + (uint32_t)(y * BN6_FIELD_COLUMNS + x) * BN6_PANEL_SIZE + BN6_PANEL_FLAGS);
}

/* (BN5's test of an obstacle at r0, r1, 0x080CABEA, the stub) where the panel's other side has the strike: an object of
 * the panel's side within two panels before the obstacle (0x080CAC42) its strike 1, else one beyond it (0x080CAC98) its
 * strike 2; r1 the striking side, r2 as BN5's searches leave it (its spawn keeps its low byte) */
static HookAct strike_test(HookRegs *r, void *user) {
	(void)user;
	int x = (int)r->r[0], y = (int)r->r[1];
	r->r[0] = 0;
	uint32_t f = panel_flags(x, y);
	if (!(f & BN6_PANEL_FIELD) || (breaks & SOULCOLONEL_BREAK_STRIKE)) return HOOK_RETURN;
	int alliance = hook_read8(BN6_FIELD_PANELS + (uint32_t)(y * BN6_FIELD_COLUMNS + x) * BN6_PANEL_SIZE + BN6_PANEL_ALLIANCE);
	int side = (alliance & 1) ^ 1;
	if (!hook_read8(soul_field(side, BN6_SOUL_STRIKE))) return HOOK_RETURN;
	uint32_t mask = side ? BN6_PANEL_ALLIES : BN6_PANEL_ENEMIES;
	int dir = side ? -1 : 1;
	r->r[1] = (uint32_t)side;
	for (int k = 0, at = x - dir; k < 2; ++k, at -= dir) {
		uint32_t g = panel_flags(at, y);
		if (!g) break;
		if (g & mask) {
			r->r[0] = 1;
			r->r[2] = PANEL_AT;
			return HOOK_RETURN;
		}
	}
	for (int at = x + dir; (unsigned)(at - 1) < 6; at += dir)
		if ((panel_flags(at, y) & mask) == mask) {
			r->r[0] = 2;
			r->r[2] = mask;
			return HOOK_RETURN;
		}
	return HOOK_RETURN;
}

/* (an obstacle's states, at their collision flags' read where BN5's block stands) on to the Thumb of BN5's block where a
 * side has the strike, r3 the way back */
static HookAct obstacle(HookRegs *r, void *user) {
	if (!done || !(hook_read8(soul_field(0, BN6_SOUL_STRIKE)) | hook_read8(soul_field(1, BN6_SOUL_STRIKE)))) return HOOK_CONTINUE;
	r->r[3] = ((uint32_t)(uintptr_t)user + 4) | 1;
	r->r[12] = SOUL_COLONEL_STRIKE | 1;
	return HOOK_JUMP;
}

/* ---- the Thumb ---- */

static void put_words(uint8_t *out, const uint16_t *t, int n) {
	for (int i = 0; i < n; ++i) put16(out + 2 * i, t[i]);
}

/* the mode 4 draw (SOUL_COLONEL_DRAW): BN5's draw, BN6's last draw, on past it */
static void draw_code(uint8_t out[0x20]) {
	static const uint16_t t[] = { 0x4B04, 0, 0, 0x4B04, 0, 0, 0x4B03, 0x4718, 0x4718, 0x46C0 };
	put_words(out, t, (int)(sizeof t / sizeof *t));
	thumb_bl_write(out, 0x02, SOUL_COLONEL_DRAW, SOUL_COLONEL_DRAW + 0x10);
	thumb_bl_write(out, 0x08, SOUL_COLONEL_DRAW, SOUL_COLONEL_DRAW + 0x10);
	put32(out + 0x14, draw | 1);
	put32(out + 0x18, BN6_MODE4_LAST | 1);
	put32(out + 0x1C, BN6_MODE4_BACK | 1);
}

/* the mode 0x24 end (SOUL_COLONEL_DRAW2): BN5's draw, then its pop {pc} */
static void draw2_code(uint8_t out[0x10]) {
	static const uint16_t t[] = { 0x4B02, 0, 0, 0xBD00, 0x4718, 0x46C0 };
	put_words(out, t, (int)(sizeof t / sizeof *t));
	thumb_bl_write(out, 0x02, SOUL_COLONEL_DRAW2, SOUL_COLONEL_DRAW2 + 0x08);
	put32(out + 0x0C, draw | 1);
}

/* BN5's block (SOUL_COLONEL_STRIKE; 0x08018002-0x0801802E): r3 the way back, then the collision flags' read it stood
 * before */
static void strike_code(uint8_t out[0x58]) {
	static const uint16_t t[] = {
		0xB408, 0x4B11, 0, 0, 0xD117,     /* push {r3}; ldr r3,=paused; bl bx r3; bne the end */
		0x7A68, 0x2800, 0xD014,           /* ldrb r0,[r5,#9]; cmp r0,#0; beq the end */
		0x7CA8, 0x7CE9, 0x7DAA,           /* ldrb r0,[r5,#0x12]; ldrb r1,[r5,#0x13]; ldrb r2,[r5,#0x16] */
		0x4B0D, 0, 0, 0x4200, 0xD00C,     /* ldr r3,=the test; bl bx r3; tst r0,r0; beq the end */
		0x1E44, 0x1C0B, 0x7CA8, 0x7CE9,   /* subs r4,r0,#1; adds r3,r1,#0; ldrb r0,[r5,#0x12]; ldrb r1,[r5,#0x13] */
		0x2600, 0xB408, 0x4B08, 0x469C,   /* movs r6,#0; push {r3}; ldr r3,=the spawn; mov r12,r3 */
		0xBC08, 0, 0,                     /* pop {r3}; bl bx r12 */
		0x2000, 0x6268,                   /* movs r0,#0; str r0,[r5,#0x24] */
		0x4B06, 0, 0, 0xBC08, 0x4718,     /* the end: ldr r3,=the flags' read; bl bx r3; pop {r3}; bx r3 */
		0x4718, 0x4760,                   /* bx r3; bx r12 */
	};
	put_words(out, t, (int)(sizeof t / sizeof *t));
	thumb_bl_write(out, 0x04, SOUL_COLONEL_STRIKE, SOUL_COLONEL_STRIKE + 0x44);
	thumb_bl_write(out, 0x18, SOUL_COLONEL_STRIKE, SOUL_COLONEL_STRIKE + 0x44);
	thumb_bl_write(out, 0x32, SOUL_COLONEL_STRIKE, SOUL_COLONEL_STRIKE + 0x46);
	thumb_bl_write(out, 0x3C, SOUL_COLONEL_STRIKE, SOUL_COLONEL_STRIKE + 0x44);
	put32(out + 0x48, BN6_BATTLE_PAUSED | 1);
	put32(out + 0x4C, SOUL_STUB_STRIKE_CHECK | 1);
	put32(out + 0x50, wrapper | 1);
	put32(out + 0x54, BN6_OBJ_FLAGS | 1);
}

/* B giving the button's chip back (SOUL_COLONEL_UNDO): BN5's steps (0x08024CEA-0x08024D28), its calls BN6's twins, then
 * BN6's B's end as BN5's goes on to its own (0x08024D96: the button lit, the lit rules, the window, sound 0x83) */
static void undo_code(uint8_t out[0x40]) {
	static const uint16_t t[] = {
		0xB510, 0x4C0B, 0x7A28,           /* push {r4,lr}; ldr r4,=element 8; ldrb r0,[r5,#8] */
		0x2148, 0x1809, 0x7922, 0x546A,   /* movs r1,#0x48; adds r1,r1,r0; ldrb r2,[r4,#4]; strb r2,[r5,r1]: its slot a pick again */
		0x1C41, 0x7229,                   /* adds r1,r0,#1; strb r1,[r5,#8] */
		0x1C01, 0x68A0, 0x4B07, 0, 0,     /* adds r1,r0,#0; ldr r0,[r4,#8]; ldr r3,=its icon; bl bx r3 */
		0x7A28, 0x3801, 0x2101, 0x2200,   /* ldrb r0,[r5,#8]; subs r0,#1; movs r1,#1; movs r2,#0 */
		0x4B04, 0, 0,                     /* ldr r3,=its slot drawn; bl bx r3 */
		0x4B04, 0x4718, 0x4718,           /* ldr r3,=B's end; bx r3; bx r3 */
	};
	put_words(out, t, (int)(sizeof t / sizeof *t));
	thumb_bl_write(out, 0x18, SOUL_COLONEL_UNDO, SOUL_COLONEL_UNDO + 0x2E);
	thumb_bl_write(out, 0x26, SOUL_COLONEL_UNDO, SOUL_COLONEL_UNDO + 0x2E);
	put32(out + 0x30, element(ELEMENT_BUTTON));
	put32(out + 0x34, BN6_PICK_ICON | 1);
	put32(out + 0x38, BN6_PICK_SLOT | 1);
	put32(out + 0x3C, BN6_EMBLEM_B_TAIL | 1);
}

/* BN5's art for the button into SOULS_COLONEL_ART (soul_room.h), read through the words its routines load it by; false where one
 * names no place in BN5's ROM */
static bool art_copy(void) {
	const uint8_t *b5 = XR[XROM_BN5_COLONEL_US].data;
	if (!b5) return false;
	uint32_t w[4];
	static const uint32_t words[4] = { BN5_COLONEL_ART, BN5_COLONEL_INFO_ART, BN5_COLONEL_INFO_PAL, BN5_COLONEL_NAME_PAL };
	static const uint32_t sizes[4] = { 2 * ART_SIZE, INFO_SIZE, PAL_SIZE, PAL_SIZE };
	for (int i = 0; i < 4; ++i) {
		w[i] = get32(b5 + (words[i] - BUS));
		if (w[i] < BUS || w[i] - BUS + sizes[i] > ROM5_SIZE) return false;
	}
	emu_write(SOULS_COLONEL_ART, b5 + (w[0] - BUS), ART_SIZE);                     /* lit, */
	emu_write(SOULS_COLONEL_ART + ART_SIZE, b5 + (w[0] - BUS) + ART_SIZE, ART_SIZE); /* grey, */
	emu_write(SOULS_COLONEL_ART + 2 * ART_SIZE, b5 + (w[0] - BUS), ART_SIZE);      /* chosen: BN5's lit */
	emu_write(SOULS_COLONEL_INFO, b5 + (w[1] - BUS), INFO_SIZE);
	emu_write(SOULS_COLONEL_INFO_PAL, b5 + (w[2] - BUS), PAL_SIZE);
	emu_write(SOULS_COLONEL_NAME_PAL, b5 + (w[3] - BUS), PAL_SIZE);
	return true;
}

/* ---- the install ---- */

static const uint32_t sites[SITES] = { BN6_OBSTACLE_FLAGS_A, BN6_OBSTACLE_FLAGS_B, BN6_OBSTACLE_FLAGS_C, BN6_OBSTACLE_FLAGS_D };

static bool expected(void) {
	static const struct { uint32_t at; uint16_t op; } code[] = {
		{ BN6_DUST_ELEMENTS, 0xB550 }, { BN6_DUST_LIT, 0xB5D0 }, { BN6_DUST_PHASES, 0x4904 }, { BN6_OK_END, 0xBDD0 },
		{ BN6_TURN_FORMS, 0xB510 }, { BN6_MODE4_LAST_DRAW, 0xF003 }, { BN6_MODE24_END, 0xBD00 }, { BN6_DUST_ART, 0x4807 },
		{ BN6_DUST_INFO_ART, 0x4806 }, { BN6_DUST_INFO_PAL, 0x4804 }, { BN6_DUST_NAME_PAL, 0x480E }, { BN6_CUSTOM_B, 0xB510 },
		{ BN6_EMBLEM_B_TAIL, 0x2000 }, { BN6_PICK_ICON, 0xB506 }, { BN6_PICK_SLOT, 0xB570 }, { BN6_OK_SPENT, 0xBDD0 },
	};
	for (size_t i = 0; i < sizeof code / sizeof *code; ++i)
		if (rom_u16(code[i].at - BUS) != code[i].op) {
			fprintf(stderr, "soulcolonel: %08x is not the instruction expected\n", (unsigned)code[i].at);
			return false;
		}
	for (int i = 0; i < SITES; ++i) {
		uint32_t t;
		if (!thumb_bl_decode(sites[i], rom_u16(sites[i] - BUS), rom_u16(sites[i] + 2 - BUS), &t) || t != BN6_OBJ_FLAGS) {
			fprintf(stderr, "soulcolonel: %08x is not the obstacle's flags' read expected\n", (unsigned)sites[i]);
			return false;
		}
	}
	for (uint32_t a = SOULS_COLONEL; a < SOULS_COLONEL_END; a += 2)
		if (emu_read16(a) != FREE_HALF) return false;
	for (uint32_t a = SOULS_COLONEL_ART; a < SOULS_COLONEL_ART_END; a += 2)
		if (emu_read16(a) != FREE_HALF) return false;
	return emu_read16(SOUL_STUB_STRIKE) == FREE_HALF && emu_read16(SOUL_STUB_STRIKE_CHECK) == FREE_HALF;
}

static bool hooks(void) {
	bool ok_ = emu_hook(SOUL_STUB_STRIKE, strike_set, NULL) && emu_hook(SOUL_STUB_STRIKE_CHECK, strike_test, NULL) &&
		emu_hook(BN6_DUST_ELEMENTS, setup, NULL) && emu_hook(BN6_DUST_LIT, light, NULL) &&
		emu_hook(BN6_DUST_PHASES, phases, (void *)(uintptr_t)BN6_DUST_PHASES_NEXT) && emu_hook(BN6_OK_END, ok, NULL) &&
		emu_hook(BN6_TURN_FORMS, turn, NULL) && emu_hook(BN6_MODE4_LAST_DRAW, drawn, (void *)(uintptr_t)SOUL_COLONEL_DRAW) &&
		emu_hook(BN6_MODE24_END, drawn, (void *)(uintptr_t)SOUL_COLONEL_DRAW2);
	for (int i = 0; ok_ && i < SITES; ++i) ok_ = emu_hook(sites[i], obstacle, (void *)(uintptr_t)sites[i]);
	for (size_t i = 0; ok_ && i < sizeof arts / sizeof *arts; ++i) ok_ = emu_hook(arts[i].at, art, (void *)(uintptr_t)&arts[i]);
	return ok_ && emu_hook(BN6_CUSTOM_B, undo, NULL) && emu_hook(BN6_OK_SPENT, spent, NULL);
}

bool soulcolonel_install(int brk) {
	if (done) return true;
	phase0 = soulcode_moved(BN5_COLONEL_PHASE0 | 1);
	phase1 = soulcode_moved(BN5_COLONEL_PHASE1 | 1);
	draw = soulcode_moved(BN5_COLONEL_DRAW | 1);
	wrapper = soulcode_moved(BN5_COLONEL_STRIKE_SPAWN | 1);
	if (!R.data || !emu_ready() || !phase0 || !phase1 || !draw || !wrapper || !expected()) return false;
	breaks = brk;
	uint8_t table[8], d4[0x20], d24[0x10], st[0x58], un[0x40];
	put32(table, phase0 | 1);
	put32(table + 4, phase1 | 1);
	draw_code(d4);
	draw2_code(d24);
	strike_code(st);
	undo_code(un);
	const uint8_t bx[2] = { THUMB_BX_LR & 0xFF, THUMB_BX_LR >> 8 };
	claims_begin(SOULCOLONEL_CLAIMS);
	emu_write(SOULS_COLONEL, table, sizeof table);
	emu_write(SOUL_COLONEL_DRAW, d4, sizeof d4);
	emu_write(SOUL_COLONEL_DRAW2, d24, sizeof d24);
	emu_write(SOUL_COLONEL_STRIKE, st, sizeof st);
	emu_write(SOUL_COLONEL_UNDO, un, sizeof un);
	bool drawn_ = art_copy();
	emu_write(SOUL_STUB_STRIKE, bx, sizeof bx);
	emu_write(SOUL_STUB_STRIKE_CHECK, bx, sizeof bx);
	bool ok_ = drawn_ && hooks();
	claims_end();
	done = ok_;
	printf("soulcolonel: ColonelSoul's%s%s\n", ok_ ? " button, its art, chip shot and obstacle strike" : " button and strike NOT placed whole",
		brk ? " (soulbreak: parts of it broken, the differential's own test)" : "");
	return ok_;
}
