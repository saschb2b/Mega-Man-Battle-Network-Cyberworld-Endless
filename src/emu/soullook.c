/* A Soul's look in BN6's engine (soullook.h). */
#include "soullook.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn5.h"
#include "bn6.h"
#include "bytes.h"
#include "claims.h"
#include "emu.h"
#include "rom.h"
#include "soul_room.h"
#include "soulcode.h"
#include "soullife.h"
#include "xnavi.h"
#include "xnavi_sprites.h"

#define BUS 0x08000000u
#define SOULS 6              /* Team Colonel's, BN5's 7-12 */
#define SOUL_FIRST 7
#define FACE_SIZE 0x100      /* a face's tiles (the emotion window's 0x06017680, 0x0801CBCC) */
#define DIGIT_SIZE 0x80      /* a digit's (its 0x06017780) */
#define DIGITS 11            /* 0-10 (0xA - the count, 0x0801CBF6) */
#define PAL_SIZE 0x20
#define PALETTE_SOUL 0       /* BN5's palette of a Soul's sprite (0x0800DD8C), */
#define PALETTE_CHAOS 2      /* ... in Chaos Unison */
#define BN5_LIST_SOULS 0     /* BN5's list of MegaMan's sprites: a Soul's at its number (0x0800DA32's table 0x0800DA50) */
#define SOUL_TOMAHAWK 10
#define TOMAHAWK_SHAKE_STRENGTH 2    /* TomahawkSoul's landing as he unites (BN5's phase 2, 0x08012128: camera_initShakeEffect(2, 0x1E)) */
#define TOMAHAWK_SHAKE_FRAMES   0x1E

static int breaks;
static bool done;
static uint32_t flash_spawner;   /* BN5's arm flash spawner, moved (its Thumb bit set) */

/* ---- the Soul a side shows ---- */

static int megaman_side(void) { return hook_read8(BN6_BATTLE_SIDE) & 1; }
static int side_of(uint32_t obj) { return hook_read8(obj + BN6_T1_ALLIANCE) & 1; }

/* the Soul side stands in or is about to (the one the Custom screen committed, taken as he enters the proxy), 0 none */
static int soul_of(int side) {
	uint8_t next = hook_read8(soul_field(side, BN6_SOUL_NEXT));
	return next ? next : hook_read8(soul_field(side, BN6_SOUL_ID));
}

static bool chaos_of(int side) {
	return hook_read8(soul_field(side, hook_read8(soul_field(side, BN6_SOUL_NEXT)) ? BN6_SOUL_NEXT_CHAOS : BN6_SOUL_CHAOS)) != 0;
}

static bool a_soul(int soul) { return soul >= SOUL_FIRST && soul < SOUL_FIRST + SOULS; }

/* ---- the sprite and its palette ---- */

/* (the sprite selector 0x0800FC9E's read of its table by the form, r1 the form, MegaMan's) the Soul's sprite in list 0 */
static HookAct sprite_read(HookRegs *r, void *user) {
	(void)user;
	int soul = soul_of(megaman_side());
	if (r->r[1] != BN6_FORM_PROXY || !a_soul(soul)) return HOOK_CONTINUE;
	int k = soul - SOUL_FIRST + ((breaks & SOULLOOK_BREAK_SPRITE) ? 1 : 0);
	r->r[2] = (uint32_t)(SOUL_LIST0_FIRST + k % SOULS);
	r->r[12] = BN6_FORM_SPRITE_BACK | 1;
	return HOOK_JUMP;
}

static uint32_t soul_palette(int side) {
	return (breaks & SOULLOOK_BREAK_PALETTE) ? 1 : chaos_of(side) ? PALETTE_CHAOS : PALETTE_SOUL;
}

/* (0x0801002C's read of the palettes by the form, r0 the form, r5 MegaMan) */
static HookAct palette_read(HookRegs *r, void *user) {
	(void)user;
	int side = side_of(r->r[5]);
	if (r->r[0] != BN6_FORM_PROXY || !a_soul(soul_of(side))) return HOOK_CONTINUE;
	r->r[4] = soul_palette(side);
	r->r[12] = BN6_FORM_PALETTE_BACK | 1;
	return HOOK_JUMP;
}

/* (an object drawn in MegaMan's form, 0x080BF516: r0 the form of its side, [r5+0x16]) */
static HookAct palette_read2(HookRegs *r, void *user) {
	(void)user;
	int side = side_of(r->r[5]);
	if (r->r[0] != BN6_FORM_PROXY || !a_soul(soul_of(side))) return HOOK_CONTINUE;
	r->r[0] = soul_palette(side);
	r->r[12] = BN6_FORM_PALETTE2_BACK | 1;
	return HOOK_JUMP;
}

/* ---- the unison: BN5's phase 1, the Soul's image ---- */

/* (the crossing's phases for forms 1-10, r0 the pending form, r5 MegaMan) a Soul's: BN6's with BN5's phase 1 */
static HookAct phases(HookRegs *r, void *user) {
	(void)user;
	int side = side_of(r->r[5]);
	if (r->r[0] != BN6_FORM_PROXY || !hook_read8(soul_field(side, BN6_SOUL_NEXT))) return HOOK_CONTINUE;
	r->r[1] = SOULS_PHASES;
	r->r[12] = BN6_CROSSING_PHASES_BACK | 1;
	return HOOK_JUMP;
}

/* (the crossing's phase 2: the sound BN5 plays in its phase 1, skipped for a Soul) and BN5's phase 2's landing for
 * TomahawkSoul (0x0801211E: the camera shaken, strength 2 for 30 frames, the battle's flag 0x20 set through the unison) */
static HookAct crossing_sound(HookRegs *r, void *user) {
	(void)user;
	int side = side_of(r->r[5]);
	if (!soul_united(side)) return HOOK_CONTINUE;
	if (soul_of(side) == SOUL_TOMAHAWK && !(breaks & SOULLOOK_BREAK_SHAKE)) {
		uint32_t camera = hook_read32(BN6_TOOLKIT + BN6_TOOLKIT_CAMERA);
		hook_write16(camera + BN6_CAMERA_SHAKE_STRENGTH, TOMAHAWK_SHAKE_STRENGTH);
		hook_write16(camera + BN6_CAMERA_SHAKE, TOMAHAWK_SHAKE_FRAMES);
		hook_write16(BN6_BATTLE_FLAGS, (uint16_t)(hook_read16(BN6_BATTLE_FLAGS) | BN6_BATTLE_FLAG_HOLD));
	}
	r->r[12] = BN6_CROSSING_SOUND_NEXT | 1;
	return HOOK_JUMP;
}

/* (the image's set-up, BN6's copy of BN5's object: its sprite by its Soul, [r5+4], from BN6's list 2) */
static HookAct image_sprite(HookRegs *r, void *user) {
	(void)user;
	int k = xnavi_sprite_number(hook_read8(r->r[5] + BN6_T1_PARAM));
	if (k < 0) return HOOK_CONTINUE;
	r->r[2] = (uint32_t)k;
	r->r[12] = BN6_SOUL_IMAGE_SPRITE_NEXT | 1;
	return HOOK_JUMP;
}

/* (its decoration by BN6's Navi of that number, and its taking away: none for a Soul's image, user the call's next) */
static HookAct image_deco(HookRegs *r, void *user) {
	if (!a_soul(hook_read8(r->r[5] + BN6_T1_PARAM))) return HOOK_CONTINUE;
	r->r[12] = (uint32_t)(uintptr_t)user | 1;
	return HOOK_JUMP;
}

/* ---- the face and the turns ---- */

/* a Soul's face and turns in place of the form's (r4 the form; the owner's side through [r5+0x48]), on to `out` */
static HookAct face(HookRegs *r, void *user) {
	uint32_t owner = hook_read32(r->r[5] + BN6_FACE_OWNER);
	int side = owner ? side_of(owner) : megaman_side(), soul = soul_of(side);
	if (r->r[4] != BN6_FORM_PROXY || !a_soul(soul)) return HOOK_CONTINUE;
	r->r[6] = (uint32_t)(SOUL_FACE_FIRST + soul - SOUL_FIRST);
	r->r[3] = hook_read8(soul_field(side, BN6_SOUL_TURNS));
	r->r[12] = (uint32_t)(uintptr_t)user | 1;
	return HOOK_JUMP;
}

/* (whether the emotion window counts under the face: always for a Soul, BN5's) */
static HookAct counter_shown(HookRegs *r, void *user) {
	(void)user;
	if (!soul_united(megaman_side())) return HOOK_CONTINUE;
	r->r[0] = 1;
	return HOOK_RETURN;
}

/* (the count's digit, r4 the face: a Soul's from BN5's digits, drawn in its palette) */
static HookAct digits(HookRegs *r, void *user) {
	(void)user;
	if (r->r[4] < SOUL_FACE_FIRST || r->r[4] >= SOUL_FACE_FIRST + SOULS) return HOOK_CONTINUE;
	r->r[1] = SOULS_DIGITS;
	r->r[12] = BN6_FACE_DIGITS_NEXT | 1;
	return HOOK_JUMP;
}

/* ---- the arm's flash ---- */

/* The flash is BN5's object, the image object (the first kind's type 5), moved: the allocations' amendment has every track
 * run BN5's, its kinds table BN5's (tools/bn5_twins.py's MOVE_TYPES), and BN6's table 0x080BA444 grown by none. The Souls'
 * code spawns it through BN5's spawner, moved (tools/xsoul.py); MegaMan's buster in a Soul through BN6's arm flash routine,
 * whose kind heat_flash (soulproxy.c) makes BN5's and whose call of BN6's spawner goes to BN5's here */

/* (BN6's arm flash routine at its mov r7,r8 before it calls BN6's spawner: r4 the flash's word, its kind the low byte, r8
 * where its object goes, r5 MegaMan) a Soul's kind spawned as BN5's object, on to the routine's end as the call returns */
static HookAct flash_spawn(HookRegs *r, void *user) {
	(void)user;
	uint32_t kind = r->r[4] & 0xFF;
	if ((kind != BN5_FLASH_SOUL && kind != BN5_FLASH_CHAOS) || !soul_united(side_of(r->r[5])) || (breaks & SOULLOOK_BREAK_FLASH))
		return HOOK_CONTINUE;
	r->r[7] = r->r[8];
	r->lr = BN6_ARM_FLASH_SPAWN_BACK | 1;
	r->r[12] = flash_spawner;
	return HOOK_JUMP;
}

/* (BN6's own flash, a BN6 chip's muzzle flash, set up for an owner in a form, BN6_FLASH_HEIGHT: raised 8 pixels for a Cross) a
 * Soul's at its kind's own height, as BN5's object keeps it for MegaMan in a Soul (0x080B9BBE: r4 the kind's record) */
static HookAct flash_height(HookRegs *r, void *user) {
	(void)user;
	if (!soul_united(side_of(r->r[5]))) return HOOK_CONTINUE;
	r->r[0] = hook_read8(r->r[4] + 3);
	r->r[12] = BN6_FLASH_HEIGHT_NEXT | 1;
	return HOOK_JUMP;
}

bool soullook_flash(int side, int *kind, int *frame) {
	int soul = done ? hook_read8(soul_field(side, BN6_SOUL_ID)) : 0;
	if (!a_soul(soul)) return false;
	*kind = hook_read8(soul_field(side, BN6_SOUL_CHAOS)) ? BN5_FLASH_CHAOS : BN5_FLASH_SOUL;
	*frame = soul;
	return true;
}

/* ---- the install ---- */

static uint32_t rom5(uint32_t a) { return get32(XR[XROM_BN5_COLONEL_US].data + (a - BUS)); }
static bool in5(uint32_t a, uint32_t n) { return a >= BUS && a - BUS + n <= ROM_SIZE; }

/* every instruction and word the look stands on, as Gregar and BN5 have them */
static bool expected(void) {
	static const struct { uint32_t at; uint16_t op; } code[] = {
		{ BN6_FORM_SPRITE_READ, 0x5C42 }, { BN6_FORM_PALETTE_READ, 0x5C0C }, { BN6_FORM_PALETTE_READ2, 0x5C08 },
		{ BN6_CROSSING_PHASES, 0x490B }, { BN6_CROSSING_SOUND, 0x208D }, { BN6_SOUL_IMAGE_SPRITE, 0x220B },
		{ BN6_FACE_OF_FORM, 0x5D16 }, { BN6_FACE_OF_FORM2, 0x5D16 }, { BN6_FACE_COUNTER, 0xB510 }, { BN6_FACE_DIGITS, 0x4971 },
		{ BN6_FLASH_HEIGHT, 0x20F8 }, { BN6_ARM_FLASH_SPAWN, 0x4647 }, { BN6_ARM_FLASH_CALL, 0xF7CD }, { BN6_ARM_FLASH_CALL2, 0xFEA3 },
	};
	for (size_t i = 0; i < sizeof code / sizeof *code; ++i)
		if (rom_u16(code[i].at - BUS) != code[i].op) {
			fprintf(stderr, "soullook: %08x is not the instruction expected\n", (unsigned)code[i].at);
			return false;
		}
	bool words = rom_u32(BN6_FACES_REF - BUS) == BN6_FACES && rom_u32(BN6_FACE_PALS_REF - BUS) == BN6_FACE_PALS &&
		rom_u32(BN6_ARM_ROW_OF_REF - BUS) == BN6_ARM_ROW_OF && rom_u32(BN6_ARM_LISTS - BUS) == BN6_ARM_ROWS0 &&
		rom_u32(BN6_CROSSING_PHASES_TABLE - BUS) == (BN6_CROSSING_PHASE0 | 1);
	bool words5 = in5(rom5(BN5_SOUL_FACES_REF), SOULS * FACE_SIZE) && in5(rom5(BN5_FACE_PALS_REF), (BN5_FACE_SOUL + SOULS) * PAL_SIZE) &&
		in5(rom5(BN5_ARM_LISTS), BN5_ARM_ROW_SIZE * (SOUL_FIRST + SOULS)) &&
		in5(rom5(BN5_FACE_DIGITS_REF), DIGITS * DIGIT_SIZE);
	return words && words5;
}

_Static_assert(SOUL_LIST0_FIRST == BN6_SPRITES_THROUGH && SOUL_LIST0_FIRST + SOULS <= SOUL_LIST0_ART_FIRST &&
	SOULS_LIST0 + 4u * SOULS_LIST0_ENTRIES <= SOULS_LIST0_END && SOULS_LIST0_ENTRIES <= SOULS_ARM_OF_END - SOULS_ARM_OF,
	"the Souls' list-0 numbers past what BN6 reads through list 0 (soul_room.h)");

/* BN5's six Souls' sprites into SOULS_SPRITES, BN6's list 0 grown with them: its words copied as far as BN6's code reads
 * through it (BN6_SPRITES_THROUGH: lists 1 and 2's tables after list 0's 14 entries, which a Falzar Navi on the map reads
 * it for), so every number under the Souls' names what it named; then what their code draws (soulcode.c copied it) */
static bool sprites_copy(void) {
	const XRom *x = &XR[XROM_BN5_COLONEL_US];
	/* (list 0 grown by the Souls' code's install, its own sprites listed there: soulcode.c's list0_set) */
	uint32_t list_word = BUS + R.layout->sprite_lists + 4u * BN6_SPRITES_MEGAMAN;
	if (emu_read32(list_word) != SOULS_LIST0) {
		fprintf(stderr, "soullook: BN6's sprite list 0 not grown by the Souls' code\n");
		return false;
	}
	uint8_t table[4 * SOULS];
	uint32_t at = SOULS_SPRITES;
	for (int k = 0; k < SOULS; ++k) {
		uint32_t len;
		uint8_t *d = xnavi_sprite_bytes(x->data, x->layout->sprite_lists, BN5_LIST_SOULS, SOUL_FIRST + k, &len);
		if (!d || at + len > SOULS_SPRITES_END) {
			fprintf(stderr, "soullook: BN5's Soul %d's sprite cannot be read, or does not fit\n", SOUL_FIRST + k);
			free(d);
			return false;
		}
		emu_write(at, d, len);
		free(d);
		put32(table + 4 * k, at);
		at = (at + len + 3) & ~3u;
	}
	/* (the list's bytes stay the Souls' code's, whoever fills an entry: claims.h) */
	claims_begin(SOULCODE_CLAIMS);
	emu_write(SOULS_LIST0 + 4u * SOUL_LIST0_FIRST, table, sizeof table);
	claims_end();
	return true;
}

/* the arm's points of the Souls' sprites: BN6's list-0 row by number grown (its bytes as BN6 reads them, the Souls'
 * numbers rows of their own) and its rows grown with BN5's of the Souls (BN5's row by the Soul's number, its points
 * BN6's first; BN6's rows hold more, the rest MegaMan's own in BN6), the two words pointed at the copies */
static void arm_points(void) {
	const uint8_t *b5 = XR[XROM_BN5_COLONEL_US].data;
	uint8_t of[SOULS_ARM_OF_END - SOULS_ARM_OF], rows[BN6_ARM_ROW_SIZE * (BN6_ARM_ROWS0_COUNT + SOULS)];
	memcpy(of, R.data + (BN6_ARM_ROW_OF - BUS), sizeof of);
	memcpy(rows, R.data + (BN6_ARM_ROWS0 - BUS), BN6_ARM_ROW_SIZE * BN6_ARM_ROWS0_COUNT);
	for (uint32_t k = 0; k < SOULS; ++k) {
		uint8_t *row = rows + BN6_ARM_ROW_SIZE * (BN6_ARM_ROWS0_COUNT + k);
		memcpy(row, R.data + (BN6_ARM_ROWS0 - BUS), BN6_ARM_ROW_SIZE);
		memcpy(row, b5 + (rom5(BN5_ARM_LISTS) - BUS) + BN5_ARM_ROW_SIZE * (SOUL_FIRST + k), BN5_ARM_ROW_SIZE);
		of[SOUL_LIST0_FIRST + k] = (uint8_t)(BN6_ARM_ROWS0_COUNT + k);
	}
	emu_write(SOULS_ARM_OF, of, sizeof of);
	emu_write(SOULS_ARM_ROWS, rows, sizeof rows);
	emu_write32(BN6_ARM_ROW_OF_REF, SOULS_ARM_OF);
	emu_write32(BN6_ARM_LISTS, SOULS_ARM_ROWS);
}

/* BN5's Souls' faces, their palettes and its digits; BN6's faces' table and palettes grown with them */
static void faces_copy(void) {
	const uint8_t *b5 = XR[XROM_BN5_COLONEL_US].data;
	emu_write(SOULS_FACE_TILES, b5 + (rom5(BN5_SOUL_FACES_REF) - BUS), SOULS * FACE_SIZE);
	emu_write(SOULS_DIGITS, b5 + (rom5(BN5_FACE_DIGITS_REF) - BUS), DIGITS * DIGIT_SIZE);
	uint8_t faces[4 * (SOUL_FACE_FIRST + SOULS)], pals[PAL_SIZE * (SOUL_FACE_FIRST + SOULS)];
	for (uint32_t i = 0; i < SOUL_FACE_FIRST; ++i) put32(faces + 4 * i, rom_u32(BN6_FACES - BUS + 4 * i));
	memcpy(pals, R.data + (BN6_FACE_PALS - BUS), PAL_SIZE * SOUL_FACE_FIRST);
	for (uint32_t k = 0; k < SOULS; ++k) {
		put32(faces + 4 * (SOUL_FACE_FIRST + k), SOULS_FACE_TILES + k * FACE_SIZE);
		memcpy(pals + PAL_SIZE * (SOUL_FACE_FIRST + k), b5 + (rom5(BN5_FACE_PALS_REF) - BUS) + PAL_SIZE * (BN5_FACE_SOUL + k), PAL_SIZE);
	}
	emu_write(SOULS_FACES, faces, sizeof faces);
	emu_write(SOULS_FACE_PALS, pals, sizeof pals);
	emu_write32(BN6_FACES_REF, SOULS_FACES);
	emu_write32(BN6_FACE_PALS_REF, SOULS_FACE_PALS);
}

/* a Soul's way into the proxy and out of it: BN5's entries by the Soul (0x0800F030, 0x0800F094), moved, 0 where BN5's
 * is its empty routine */
static void routes(void) {
	uint32_t enter[SOULS_BN5_FORMS] = { 0 }, leave[SOULS_BN5_FORMS] = { 0 };
	for (int n = SOUL_FIRST; n < SOUL_FIRST + SOULS; ++n) {
		uint32_t e = rom5(BN5_SOUL_ENTER + 4u * (uint32_t)n), l = rom5(BN5_SOUL_LEAVE + 4u * (uint32_t)n);
		enter[n] = e == BN5_FORM_NULLSUB ? 0 : soulcode_moved(e);
		leave[n] = l == BN5_FORM_NULLSUB ? 0 : soulcode_moved(l);
	}
	soullife_routes(enter, leave);
}

/* the look's hooks, each where BN6's engine draws MegaMan by his form */
static bool hooks_set(void) {
	return emu_hook(BN6_FORM_SPRITE_READ, sprite_read, NULL) && emu_hook(BN6_FORM_PALETTE_READ, palette_read, NULL) &&
		emu_hook(BN6_FORM_PALETTE_READ2, palette_read2, NULL) && emu_hook(BN6_CROSSING_PHASES, phases, NULL) &&
		emu_hook(BN6_CROSSING_SOUND, crossing_sound, NULL) && emu_hook(BN6_SOUL_IMAGE_SPRITE, image_sprite, NULL) &&
		emu_hook(BN6_SOUL_IMAGE_DECO, image_deco, (void *)(uintptr_t)BN6_SOUL_IMAGE_DECO_NEXT) &&
		emu_hook(BN6_SOUL_IMAGE_UNDECO, image_deco, (void *)(uintptr_t)BN6_SOUL_IMAGE_UNDECO_NEXT) &&
		emu_hook(BN6_FACE_OF_FORM, face, (void *)(uintptr_t)BN6_FACE_OF_FORM_OUT) &&
		emu_hook(BN6_FACE_OF_FORM2, face, (void *)(uintptr_t)BN6_FACE_OF_FORM2_OUT) &&
		emu_hook(BN6_FACE_COUNTER, counter_shown, NULL) && emu_hook(BN6_FACE_DIGITS, digits, NULL) &&
		emu_hook(BN6_FLASH_HEIGHT, flash_height, NULL) && emu_hook(BN6_ARM_FLASH_SPAWN, flash_spawn, NULL);
}

bool soullook_install(int brk) {
	if (done) return true;
	const XRom *x = &XR[XROM_BN5_COLONEL_US];
	uint32_t p1 = soulcode_moved(BN5_UNITE_IMAGE_PHASE);
	flash_spawner = soulcode_moved(BN5_FLASH_SPAWN | 1);
	if (!R.data || !R.layout || !emu_ready() || !x->data || !x->layout || !p1 || !flash_spawner || !expected()) return false;
	for (int n = SOUL_FIRST; n < SOUL_FIRST + SOULS; ++n)
		if (!xnavi_sprite_install(n)) return false;
	claims_begin(SOULLOOK_CLAIMS);
	bool ok = sprites_copy();
	if (ok) {
		faces_copy();
		arm_points();
		const uint32_t phase[4] = { BN6_CROSSING_PHASE0 | 1, p1 | 1, BN6_CROSSING_PHASE2 | 1, BN6_CROSSING_PHASE3 | 1 };
		uint8_t words[sizeof phase];
		for (int i = 0; i < 4; ++i) put32(words + 4 * i, phase[i]);
		emu_write(SOULS_PHASES, words, sizeof words);
		routes();
		ok = hooks_set();
	}
	claims_end();
	done = ok;
	breaks = brk;
	printf("soullook: the Souls' look%s%s\n", ok ? ": their sprites, palettes, image, faces and flashes" : " NOT placed whole",
		brk ? " (soulbreak: parts of it broken, the differential's own test)" : "");
	return ok;
}
