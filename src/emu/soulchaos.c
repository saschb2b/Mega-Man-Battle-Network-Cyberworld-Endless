/* Chaos Unison in BN6's engine (soulchaos.h). */
#include "soulchaos.h"

#include <stdio.h>
#include <string.h>

#include "bn5.h"
#include "bn5_chips.h"
#include "bn6.h"
#include "bn6_attacks.h"
#include "bn6_forms.h"
#include "bn6_navis.h"
#include "bytes.h"
#include "darkinvs.h"
#include "claims.h"
#include "emu.h"
#include "rom.h"
#include "soul_room.h"
#include "soulchaos_thumb.h"
#include "soulcode.h"
#include "soullife.h"
#include "xattack_grow.h"
#include "xchips_registry.h"
#include "xnavi_install.h"
#include "xvirus_recipe.h"

#define BUS 0x08000000u
#define SOUL_FIRST 7
#define SOULS 6
#define CHARGE_B 2               /* a charge's button (r0, BN6_AI_CHARGE_BUTTON): B */
#define CHARGE_FULL 2            /* BN6_AI_CHARGE: full */
#define REQUEST_B 2u             /* the release's request of a charged B */
#define REQUEST_CHAOS 0x8000u    /* ... of a chaos B in phase 2 (BN5's 0x08010866) */
#define REQUEST_MISS 0x10000u    /* ... of one in another phase (0x0801086E) */
#define PHASE_HIT 2              /* the phase a release hits in */
#define LEVEL_ROWS 3             /* the phases' rows by the level (0x080106B4: 2 at most) */
#define LEVEL_CHARGES 5          /* a chaos power's charge times by the level (BN5_CHAOS_LEVEL_TOP + 1) */
#define NONE 0xFFu
#define FOLDER_BYTES 0x3Cu       /* a folder's bytes (eToolkit +0x48's folders) */
#define LIST_CHIPS 0x2A          /* a chip list's chips (xnavi_dark.h's XNAVI_DARK_CHIPS) */
#define PLAYERS_ROW 0x10u        /* BattleState +0x80's objects, a side's row (BN5's 0x08104376) */
#define PLAYERS_OF_SIDE 4

/* the chaos power attacks by Soul (BN5's numbers 7-12: ColonelSoul's on), BN5's id, its DarkChip (BN5's), its Thumb */
static const struct { uint8_t power, chip; uint32_t thumb, literal; } powers[SOULS] = {
	{ 0x3A, 198, CHAOS_POWER_COLONEL, CHAOS_CHIP_COLONEL },     /* DrkSonic */
	{ 0x31, 189, CHAOS_POWER_SHADOW, CHAOS_CHIP_SHADOW },       /* DarkInvs */
	{ 0x32, 190, CHAOS_POWER_NUMBER, CHAOS_CHIP_NUMBER },       /* DarkPlus */
	{ 0x33, 191, CHAOS_POWER_TOMAHAWK, CHAOS_CHIP_TOMAHAWK },   /* DrkLance */
	{ 0x38, 196, CHAOS_POWER_KNIGHT, CHAOS_CHIP_KNIGHT },       /* DarkDril */
	{ 0x34, 192, CHAOS_POWER_TOAD, CHAOS_CHIP_TOAD },           /* DarkWide */
};

static int breaks;
static bool done;
static uint8_t phases[LEVEL_ROWS][4];   /* BN5_CHAOS_PHASES' rows */
static int glow_number = -1;            /* list 0's number of BN5's dark charge glow (the Souls' copy), -1 none */
static int dark_id = -1;                /* BN6's id of the Dark Soul (xnavi_bn6_id) */
static uint32_t dues[HOOK_CORES];        /* the object whose charge's state just stepped (BN6_CHAOS_DUE), one a core (hook_slot) */

static int side_of(uint32_t obj) { return hook_read8(obj + BN6_T1_ALLIANCE) & 1; }
static bool chaos(int side) { return soul_united(side) && hook_read8(soul_field(side, BN6_SOUL_CHAOS)); }
static int level(int side) {
	int v = hook_read8(soul_field(side, BN6_SOUL_LEVEL));
	return v > BN5_CHAOS_LEVEL_TOP ? BN5_CHAOS_LEVEL_TOP : v;
}

uint32_t soulchaos_power(int bn5_id) {
	for (int i = 0; done && i < SOULS; ++i)
		if (powers[i].power == bn5_id) return (SOULS_CHAOS + powers[i].thumb) | 1u;
	return 0;
}

/* ---- the charge ---- */

/* (BN6_CHAOS_CHARGE, r0 the button, r5 MegaMan) a chaos B's time: his chaos power's row of the charge times by the level
 * (BN5's 0x08010676-0x0801069A), 0xFF where he has none */
static HookAct charge(HookRegs *r, void *user) {
	(void)user;
	int side = side_of(r->r[5]);
	if (r->r[0] != CHARGE_B || !chaos(side)) return HOOK_CONTINUE;
	uint32_t ai = hook_read32(r->r[5] + BN6_T1_AI_DATA);
	uint8_t row = hook_read8(ai + BN6_AI_POWER_CHAOS);
	r->r[0] = row == NONE ? NONE : hook_read16(hook_read32(BN6_CHARGE_TIMES_REF) + BN6_CHARGE_ROW * row + 2u * (uint32_t)level(side));
	return HOOK_RETURN;
}

/* (BN6_CHAOS_DUE, r5 MegaMan) his charge's state is stepped this frame */
static HookAct charge_due(HookRegs *r, void *user) {
	(void)user;
	dues[hook_slot()] = r->r[5];
	return HOOK_CONTINUE;
}

/* (BN6_CHAOS_PHASE, r4 his AI data, r5 him) after it, BN5's phase (0x080105F0): a full chaos B steps the cycle of its row by
 * the level, its phase 2, 1, 0 or 1 by the row's bounds; another charge starts it over */
static HookAct phase(HookRegs *r, void *user) {
	(void)user;
	if (dues[hook_slot()] != r->r[5]) return HOOK_CONTINUE;
	dues[hook_slot()] = 0;
	int side = side_of(r->r[5]);
	if (!chaos(side)) return HOOK_CONTINUE;
	uint32_t ai = r->r[4], cycle = soul_field(side, BN6_SOUL_CYCLE);
	if (hook_read8(ai + BN6_AI_CHARGE_BUTTON) != CHARGE_B || hook_read8(ai + BN6_AI_CHARGE) != CHARGE_FULL) {
		hook_write8(cycle, 0);
		return HOOK_CONTINUE;
	}
	const uint8_t *row = phases[level(side) < LEVEL_ROWS ? level(side) : LEVEL_ROWS - 1];
	int c = hook_read8(cycle) + 1, length = row[0] + ((breaks & SOULCHAOS_BREAK_PHASE) ? 1 : 0);
	if (c >= length) c = 0;
	hook_write8(cycle, (uint8_t)c);
	hook_write8(soul_field(side, BN6_SOUL_PHASE), (uint8_t)(c < row[1] ? 2 : c < row[2] ? 1 : c < row[3] ? 0 : 1));
	return HOOK_CONTINUE;
}

/* ---- the release and its request ---- */

/* (BN6_CHAOS_RELEASE, r0 the request, r4 his AI data, r5 him) a chaos B's: 0x8000 in phase 2, 0x10000 else (BN5's
 * 0x0801085E-0x0801086E), the Souls' own while it lasts */
static HookAct release(HookRegs *r, void *user) {
	(void)user;
	int side = side_of(r->r[5]);
	if (r->r[0] != REQUEST_B || !chaos(side)) return HOOK_CONTINUE;
	r->r[0] = hook_read8(soul_field(side, BN6_SOUL_PHASE)) == PHASE_HIT ? REQUEST_CHAOS : REQUEST_MISS;
	hook_write8(soul_field(side, BN6_SOUL_REQUEST), 1);
	return HOOK_CONTINUE;
}

/* (BN6_CHAOS_CONSUME, r0 his requests, r5 him) BN5's 0x080F0436-0x080F0494 where the request is the Souls': the chaos attack
 * (the level one up, 4 at most) or the failure, each the Thumb's end of the requests' routine */
static HookAct consume(HookRegs *r, void *user) {
	(void)user;
	int side = side_of(r->r[5]);
	if (!hook_read8(soul_field(side, BN6_SOUL_REQUEST))) return HOOK_CONTINUE;
	if (r->r[0] & REQUEST_CHAOS) {
		uint32_t at = soul_field(side, BN6_SOUL_LEVEL);
		int v = hook_read8(at) + 1;
		hook_write8(at, (uint8_t)(v > BN5_CHAOS_LEVEL_TOP ? BN5_CHAOS_LEVEL_TOP : v));
		r->r[12] = (SOULS_CHAOS + CHAOS_CHAOS_HIT) | 1u;
		return HOOK_JUMP;
	}
	if (r->r[0] & REQUEST_MISS) {
		r->r[12] = (SOULS_CHAOS + CHAOS_CHAOS_MISS) | 1u;
		return HOOK_JUMP;
	}
	return HOOK_CONTINUE;
}

/* (BN6_CHAOS_END, r0 the requests it clears, r5 him, r6 the attack's kind) BN5's end clears the release's requests too
 * (0x1803F, 0x0800F2D0) and a failure's kind (6) his chaos (0x0800F2CE) */
static HookAct attack_end(HookRegs *r, void *user) {
	(void)user;
	int side = side_of(r->r[5]);
	uint32_t req = soul_field(side, BN6_SOUL_REQUEST);
	if (!hook_read8(req)) return HOOK_CONTINUE;
	hook_write8(req, 0);
	r->r[0] |= REQUEST_CHAOS | REQUEST_MISS;
	if (r->r[6] == 6) hook_write8(soul_field(side, BN6_SOUL_CHAOS), 0);
	return HOOK_CONTINUE;
}

/* (BN6_COUNTER_TEST and the families' twins, r1 0x8600, r5 him) BN5's 0x600 while the Souls' request stands: BN6's own counter
 * request 0x8000 (to its family 0x4B, 0x0801056A) is BN5's chaos request there */
static HookAct counter_test(HookRegs *r, void *user) {
	(void)user;
	if (hook_read8(soul_field(side_of(r->r[5]), BN6_SOUL_REQUEST))) r->r[1] &= ~REQUEST_CHAOS;
	return HOOK_CONTINUE;
}

/* ---- the glow (the fourth kind's 8: r4 or its +0x4C its owner, r6 his AI data) ---- */

static uint32_t owner(uint32_t glow) { return hook_read32(glow + BN6_T1_PARENT); }

/* (BN6_GLOW_SHOW / BN6_GLOW_SOUND, a B charge's branch past the side's test) in chaos the side's test as BN5's */
static HookAct glow_side(HookRegs *r, void *user) {
	uint32_t who = user == (void *)BN6_GLOW_SHOW_SIDE ? r->r[4] : r->r[1];
	if (!chaos(side_of(who))) return HOOK_CONTINUE;
	r->r[12] = (uint32_t)(uintptr_t)user | 1u;
	return HOOK_JUMP;
}

/* (BN6_GLOW_PICK, r5 the glow) BN5's dark charge glow in chaos, past A's (BN5's 0x080E0F96-0x080E0F9C) */
static HookAct glow_pick(HookRegs *r, void *user) {
	(void)user;
	if (glow_number < 0 || !chaos(side_of(owner(r->r[5])))) return HOOK_CONTINUE;
	r->r[2] = BN6_GLOW_SPRITE_CHAOS;
	r->r[12] = BN6_GLOW_PICK_NEXT | 1u;
	return HOOK_JUMP;
}

/* (BN6_GLOW_LOAD, r1 list 5's offset, r2 the number) the dark glow's number as the Souls' copy in list 0 */
static HookAct glow_load(HookRegs *r, void *user) {
	(void)user;
	if (glow_number < 0 || r->r[2] != BN6_GLOW_SPRITE_CHAOS || !chaos(side_of(owner(r->r[5])))) return HOOK_CONTINUE;
	r->r[1] = 0;
	r->r[2] = (uint32_t)glow_number;
	return HOOK_CONTINUE;
}

/* (BN6_GLOW_ANIM, r0 its animation the charge's state, r5 the glow, r6 his AI data) a full chaos B's the phase + 2 (BN5's
 * 0x080E0EF8-0x080E0F0E) */
static HookAct glow_anim(HookRegs *r, void *user) {
	(void)user;
	uint32_t ai = r->r[6];
	int side = side_of(owner(r->r[5]));
	if (!chaos(side) || hook_read8(ai + BN6_AI_CHARGE) != CHARGE_FULL || hook_read8(ai + BN6_AI_CHARGE_BUTTON) != CHARGE_B)
		return HOOK_CONTINUE;
	r->r[0] = hook_read8(soul_field(side, BN6_SOUL_PHASE)) + 2u;
	hook_write8(r->r[5] + BN6_T1_ANIM, (uint8_t)r->r[0]);
	return HOOK_CONTINUE;
}

/* ---- the failure's Dark Soul ---- */

/* the chip lists of `side` where the battle gave it none: the folder's chips BN5's have (the registry's rows: the chip AI
 * reads BN5's bytes of their records, xnavi_dark.c), the most of a chip first, the folder's order between equals */
void soulchaos_lists_fill(int side) {
	uint32_t list = BN6_DARK_LISTS + (uint32_t)side * BN5_DARK_LIST_SIZE;
	if (hook_read32(list + BN5_DARK_LIST_COUNT)) return;
	uint32_t folder = hook_read32(BN6_TOOLKIT + BN6_TOOLKIT_CHIPS) + FOLDER_BYTES * hook_read8(BN6_NAVI_FOLDER);
	uint16_t ids[BN6_FOLDER_ENTRIES];
	uint8_t count[BN6_FOLDER_ENTRIES];
	int n = 0;
	for (uint32_t i = 0; i < BN6_FOLDER_ENTRIES; ++i) {
		uint16_t id = hook_read16(folder + 2 * i) & 0x1FFu;
		if (!id || (!xreg_at(id) && !xreg_named(id))) continue;
		int k = 0;
		while (k < n && ids[k] != id) ++k;
		if (k == n) { ids[n] = id; count[n++] = 0; }
		++count[k];
	}
	for (int a = 1; a < n; ++a)   /* (stable: the folder's order between equals) */
		for (int b = a; b > 0 && count[b] > count[b - 1]; --b) {
			uint16_t t = ids[b]; ids[b] = ids[b - 1]; ids[b - 1] = t;
			uint8_t c = count[b]; count[b] = count[b - 1]; count[b - 1] = c;
		}
	for (int k = 0; k < LIST_CHIPS; ++k) hook_write16(list + 2u * (uint32_t)k, k < n ? ids[k] : 0xFFFF);
	hook_write32(list + BN5_DARK_LIST_COUNT, (uint32_t)(n < LIST_CHIPS ? n : LIST_CHIPS));
}

/* (CHAOS_OUT_STUB, r0 MegaMan's side) BN5's 0x0810436C: a Dark Soul already out on the other side ended (its AIData +0x88 0),
 * then the lists the new one plays: his side's (the Dark Soul's chip AI reads the other side's to its own, 0x0802BE58) */
static HookAct out(HookRegs *r, void *user) {
	(void)user;
	int side = (int)(r->r[0] & 1), other = side ^ 1;
	uint32_t row = BN6_BATTLE_SIDE_NAVIS + PLAYERS_ROW * (uint32_t)other;
	for (uint32_t k = 0; k < PLAYERS_OF_SIDE; ++k) {
		uint32_t obj = hook_read32(row + 4 * k);
		if (obj && dark_id >= 0 && hook_read16(obj + BN6_T1_NAME_ID) == (uint16_t)dark_id)
			hook_write16(hook_read32(obj + BN6_T1_AI_DATA) + BN5_DARK_SOUL_TIMER, 0);
	}
	soulchaos_lists_fill(side);
	return HOOK_CONTINUE;
}

/* ---- the install ---- */

/* the Thumb as SOULS_CHAOS holds it: the chips by the registry, the failure's phases and the stub at their places, the Dark
 * Soul's id; false where a chip is not registered */
static bool thumb_make(uint8_t out_bytes[CHAOS_THUMB_SIZE]) {
	soulchaos_thumb(out_bytes);
	for (int i = 0; i < SOULS; ++i) {
		const XChipReg *row = xreg_of_bn5(powers[i].chip);
		if (!row) return false;
		put32(out_bytes + powers[i].literal, row->bn6);
	}
	static const uint32_t phase_at[3] = { CHAOS_FAIL_0, CHAOS_FAIL_4, CHAOS_FAIL_8 };
	put32(out_bytes + CHAOS_W_PHASES, SOULS_CHAOS + CHAOS_FAIL_PHASES);
	for (int k = 0; k < 3; ++k) put32(out_bytes + CHAOS_FAIL_PHASES + 4u * (uint32_t)k, (SOULS_CHAOS + phase_at[k]) | 1u);
	put32(out_bytes + CHAOS_W_OUT, (SOULS_CHAOS + CHAOS_OUT_STUB) | 1u);
	put32(out_bytes + CHAOS_W_DARK, (uint32_t)dark_id);
	if (dark_id < 0) put32(out_bytes + CHAOS_W_PANEL, (SOULS_CHAOS + CHAOS_ZERO) | 1u);   /* (no Dark Soul: no panel, none spawned) */
	return true;
}

/* every byte it stands on: BN5's code it ports (each range's CRC32), BN6's at each hook, its room free */
static bool expected(const uint8_t *b5) {
	static const struct { uint32_t lo, hi, crc; } ports[] = {
		{ BN5_CHAOS_PHASE, BN5_CHAOS_PHASE_END, BN5_CHAOS_PHASE_CRC }, { BN5_CHAOS_CHARGE, BN5_CHAOS_CHARGE_END, BN5_CHAOS_CHARGE_CRC },
		{ BN5_CHAOS_RELEASE, BN5_CHAOS_RELEASE_END, BN5_CHAOS_RELEASE_CRC },
		{ BN5_CHAOS_CONSUME, BN5_CHAOS_CONSUME_END, BN5_CHAOS_CONSUME_CRC },
		{ BN5_CHAOS_ATTACK, BN5_CHAOS_ATTACK_END, BN5_CHAOS_ATTACK_CRC }, { BN5_CHAOS_FAMILY, BN5_CHAOS_FAMILY_END, BN5_CHAOS_FAMILY_CRC },
		{ BN5_CHAOS_POWERS, BN5_CHAOS_POWERS_END, BN5_CHAOS_POWERS_CRC }, { BN5_CHAOS_GLOW, BN5_CHAOS_GLOW_END, BN5_CHAOS_GLOW_CRC },
		{ BN5_CHAOS_OUT, BN5_CHAOS_OUT_END, BN5_CHAOS_OUT_CRC },
	};
	for (size_t i = 0; i < sizeof ports / sizeof *ports; ++i)
		if (xvirus_crc32(b5 + (ports[i].lo - BUS), ports[i].hi - ports[i].lo) != ports[i].crc) {
			fprintf(stderr, "soulchaos: BN5's %08x is not the code ported\n", (unsigned)ports[i].lo);
			return false;
		}
	/* (a bl's two halves one word, its first the low halfword) */
	static const struct { uint32_t at, op; } code[] = {
		{ BN6_CHAOS_CHARGE, 0xB590 }, { BN6_CHAOS_DUE, 0xF817F000u }, { BN6_CHAOS_PHASE, 0xBD50 }, { BN6_CHAOS_RELEASE, 0xFF06F7FCu },
		{ BN6_CHAOS_CONSUME, 0xFBF6F71Eu }, { BN6_CHAOS_END, 0xFC53F7FEu }, { BN6_GLOW_SHOW, 0xD008 }, { BN6_GLOW_PICK, 0x2208 },
		{ BN6_GLOW_LOAD, 0xFA30F720u }, { BN6_GLOW_ANIM, 0x2800 }, { BN6_GLOW_SOUND, 0xD004 },
		{ BN6_COUNTER_TEST, 0x4208 }, { BN6_COUNTER_TEST_1D, 0x4208 }, { BN6_COUNTER_TEST_1E, 0x4208 }, { BN6_COUNTER_TEST_37, 0x4208 },
	};
	for (size_t i = 0; i < sizeof code / sizeof *code; ++i)
		if ((code[i].op > 0xFFFFu ? rom_u32(code[i].at - BUS) : rom_u16(code[i].at - BUS)) != code[i].op) {
			fprintf(stderr, "soulchaos: %08x is not the instruction expected\n", (unsigned)code[i].at);
			return false;
		}
	for (uint32_t a = SOULS_CHAOS; a < SOULS_CHAOS_END; a += 4)
		if (emu_read32(a) != 0xFFFFFFFFu) {
			fprintf(stderr, "soulchaos: its room is not free\n");
			return false;
		}
	return emu_read32(BN6_CHARGE_TIMES_REF) != 0;
}

/* list 0's number of BN5's dark charge glow, where the Souls' code copied it (soulcode_bn5.c's sprites), else -1 */
static int glow_copy(void) {
	const SoulCode *c = &soulcode_bn5;
	for (int i = 0; i < c->nsprites; ++i)
		if (c->sprites[i].list == 5 && c->sprites[i].index == BN6_GLOW_SPRITE_CHAOS) return c->sprites[i].to_index;
	return -1;
}

static bool hooks(void) {
	return emu_hook(BN6_CHAOS_CHARGE, charge, NULL) && emu_hook(BN6_CHAOS_DUE, charge_due, NULL) &&
		emu_hook(BN6_CHAOS_PHASE, phase, NULL) && emu_hook(BN6_CHAOS_RELEASE, release, NULL) &&
		emu_hook(BN6_CHAOS_CONSUME, consume, NULL) && emu_hook(BN6_CHAOS_END, attack_end, NULL) &&
		emu_hook(BN6_GLOW_SHOW, glow_side, (void *)(uintptr_t)BN6_GLOW_SHOW_SIDE) &&
		emu_hook(BN6_GLOW_SOUND, glow_side, (void *)(uintptr_t)BN6_GLOW_SOUND_SIDE) && emu_hook(BN6_GLOW_PICK, glow_pick, NULL) &&
		emu_hook(BN6_GLOW_LOAD, glow_load, NULL) && emu_hook(BN6_GLOW_ANIM, glow_anim, NULL) &&
		emu_hook(SOULS_CHAOS + CHAOS_OUT_STUB, out, NULL) && emu_hook(BN6_COUNTER_TEST, counter_test, NULL) &&
		emu_hook(BN6_COUNTER_TEST_1D, counter_test, NULL) && emu_hook(BN6_COUNTER_TEST_1E, counter_test, NULL) &&
		emu_hook(BN6_COUNTER_TEST_37, counter_test, NULL);
}

bool soulchaos_install(int brk) {
	if (done) return true;
	const uint8_t *b5 = XR[XROM_BN5_COLONEL_US].data;
	if (!R.data || !emu_ready() || !b5 || !expected(b5)) return false;
	breaks = brk;
	memcpy(phases, b5 + (BN5_CHAOS_PHASES - BUS), sizeof phases);
	glow_number = glow_copy();
	dark_id = xnavi_bn6_id(BN5_DARK_SOUL_ID);
	static uint8_t code[CHAOS_THUMB_SIZE];
	if (!thumb_make(code)) {
		fprintf(stderr, "soulchaos: a chaos power's DarkChip is not registered\n");
		return false;
	}
	claims_begin(SOULCHAOS_CLAIMS);
	emu_write(SOULS_CHAOS, code, sizeof code);
	bool ok = xattack_set(BN6_CHAOS_FAMILY, (SOULS_CHAOS + CHAOS_FAIL_FAMILY) | 1u) && hooks();
	claims_end();
	done = ok;
	/* (BN5's chip AI for the player, which ShadowSoul's chaos attack, BN5's DarkInvs, puts him under: darkinvs.h) */
	if (ok) darkinvs_install();
	printf("soulchaos: Chaos Unison%s%s%s\n", ok ? ": its charge, phases, glow, release, attacks and failure" : " NOT placed whole",
		dark_id < 0 ? " (no Dark Soul installed: a failure spawns none)" : "", glow_number < 0 ? " (no dark glow copied)" : "");
	return ok;
}
