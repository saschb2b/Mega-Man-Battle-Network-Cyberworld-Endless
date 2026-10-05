/* BN6's own DarkChips in its battles (darkbn6.h). */
#include "darkbn6.h"

#include <stdbool.h>
#include <stdio.h>

#include "bn6.h"
#include "darkchips.h"
#include "data.h"
#include "debug.h"
#include "emu.h"
#include "events.h"
#include "rom.h"

#define BUS 0x08000000u

static unsigned ran, base_ran;   /* the battle's DarkChips, a bit per DarkChipID: their dark power ran; their base chip ran */
static int powered = -1;         /* the records' power as last written: 1 their own, 0 their base chips', -1 not yet */

/* chip `id`'s record (a ROM offset) */
static uint32_t record(int id) { return R.layout->chip_data + (uint32_t)id * BN6_CHIP_RECORD_SIZE; }

int darkbn6_base(int id) {
	int d = id - DARK_BN6_FIRST;
	if (!R.data || !R.layout || d < 0 || d >= DARK_BN6_COUNT) return 0;
	uint32_t at = BN6_DARK_BASES - BUS + (uint32_t)d * BN6_DARK_BASE_STEP;
	/* (push {lr}; movs r0, #chip) */
	return rom_u16(at) == 0xB500 && R.data[at + 3] == 0x20 ? R.data[at + 2] : 0;
}

void darkbn6_does(int id, char *out, size_t n) {
	if (!R.data || !R.layout || dark_bn6_kind(id) < 0) { if (n) out[0] = 0; return; }
	uint32_t rec = record(id);
	int power = rom_u16(rec + BN6_CHIP_ATTACK_POWER), sub = R.data[rec + BN6_CHIP_SUBFAMILY];
	switch (id - DARK_BN6_FIRST) {
	case 0: snprintf(out, n, "a %d slash on the six panels ahead", power); break;
	case 1: snprintf(out, n, "a %d Thunder that paralyzes", power); break;
	/* (a recovery chip's amount by its subfamily: DrkRecov's the ninth) */
	case 2: snprintf(out, n, "%d HP back", sub < 10 ? rom_u16(BN6_RECOVERY_AMOUNTS - BUS + 2u * (uint32_t)sub) : 0); break;
	case 3: snprintf(out, n, "eight safe seconds,with the darkness fighting for us"); break;
	default: snprintf(out, n, "a puff of darkness"); break;
	}
}

/* The four records' power: their own (with a BugFrag), or their base
 * chip's, the ROM's either way */
static void power(bool own) {
	for (int i = 0; i < DARK_BN6_PLAYED; ++i) {
		int id = DARK_BN6_FIRST + i, from = own ? id : darkbn6_base(id);
		if (!from) continue;
		uint16_t v = rom_u16(record(from) + BN6_CHIP_ATTACK_POWER);
		uint8_t b[2] = { (uint8_t)v, (uint8_t)(v >> 8) };
		emu_write(BUS + record(id) + BN6_CHIP_ATTACK_POWER, b, sizeof b);
	}
	powered = own;
	if (emu_debug_on()) fprintf(stderr, "dark: BN6's DarkChips show %s power\n", own ? "their own" : "their base chips'");
}

/* (names compared as the editor's ABC sort reads them, case aside) */
static int name_cmp(const char *a, const char *b) {
	for (;; ++a, ++b) {
		int x = *a >= 'a' && *a <= 'z' ? *a - 32 : *a, y = *b >= 'a' && *b <= 'z' ? *b - 32 : *b;
		if (x != y || !x) return x - y;
	}
}

#define SORTED_CHIPS 314   /* (the chips a folder or the pack holds, 1-313) */

/* DarkChip `id`'s place in the folder editor's ABC order: the place of the
 * chip whose name comes just before its (a tie the editor breaks by id),
 * which the US version left 0 */
static uint16_t alpha_place(int id) {
	ChipInfo me, ci;
	chip_info(id, &me);
	char best[sizeof ci.name] = "";
	uint16_t place = 1;
	for (int c = 1; c < SORTED_CHIPS; ++c) {
		uint16_t a = rom_u16(record(c) + BN6_CHIP_ALPHA_SORT);
		if (!a || dark_bn6_kind(c) >= 0) continue;
		chip_info(c, &ci);
		if (name_cmp(ci.name, me.name) < 0 && (!best[0] || name_cmp(ci.name, best) > 0)) {
			snprintf(best, sizeof best, "%s", ci.name);
			place = a;
		}
	}
	return place;
}

void darkbn6_records(void) {
	if (!R.data || !R.layout || !R.layout->chip_data) return;
	static uint16_t alpha[DARK_BN6_PLAYED];
	for (int i = 0; i < DARK_BN6_PLAYED; ++i) {
		int id = DARK_BN6_FIRST + i, from = darkbn6_base(id);
		if (!from) continue;
		uint32_t rec = record(id);
		emu_write8(BUS + rec + BN6_CHIP_EFFECT_FLAGS, (uint8_t)(R.data[rec + BN6_CHIP_EFFECT_FLAGS] | BN6_CHIP_DARK_CLASS));
		emu_write8(BUS + rec + BN6_CHIP_LIBRARY_FLAGS, (uint8_t)(R.data[rec + BN6_CHIP_LIBRARY_FLAGS] & ~BN6_CHIP_UNLISTED));
		/* (the icon, picture and palette pointers: the base chip's) */
		emu_write(BUS + rec + BN6_CHIP_ICON_PTR, R.data + record(from) + BN6_CHIP_ICON_PTR, BN6_CHIP_RECORD_SIZE - BN6_CHIP_ICON_PTR);
		/* (their places in the editor's sorts, 0 on all five: its swap tells
		 * two chips apart by the ID's place and the code, so it took any two
		 * DarkChips for one chip and let one in for another past the folder's
		 * three; by ID their own id, a place no chip holds, by name among the
		 * names before theirs) */
		if (!alpha[i]) alpha[i] = alpha_place(id);
		uint8_t sort[2][2] = { { (uint8_t)alpha[i], (uint8_t)(alpha[i] >> 8) }, { (uint8_t)id, (uint8_t)(id >> 8) } };
		emu_write(BUS + rec + BN6_CHIP_ALPHA_SORT, sort[0], 2);
		emu_write(BUS + rec + BN6_CHIP_ID_SORT, sort[1], 2);
	}
	powered = -1;
	darkbn6_sync();
}

void darkbn6_sync(void) {
	if (!R.data || !R.layout || !R.layout->chip_data) return;
	int own = emu_read32(BN6_BUGFRAGS) > 0;
	if (own != powered) power(own);
}

void darkbn6_battle_begins(void) {
	ran = base_ran = 0;
	darkbn6_sync();
}

void darkbn6_event(const HookEvent *e) {
	if (e->kind == EV_DARK_RAN) {
		uint32_t d = e->r[0] - BN6_DARK_FIRST_ID;
		if (d < (uint32_t)DARK_BN6_COUNT) ran |= 1u << d;
		uint32_t left = emu_read32(BN6_BATTLE_BUGFRAGS);
		if (emu_debug_on()) fprintf(stderr, "dark: DarkChip %#x's dark power ran, a BugFrag paid, %u left in the battle\n", (unsigned)e->r[0], (unsigned)left);
		/* (the battle's last BugFrag spent: its next Custom screens show
		 * what will land) */
		if (!left && powered) power(false);
	} else if (e->kind == EV_DARK_BASE && e->r[0] < (uint32_t)DARK_BN6_COUNT) {
		base_ran |= 1u << e->r[0];
		if (emu_debug_on()) fprintf(stderr, "dark: DarkChip %u ran as its base chip, no BugFrag\n", (unsigned)e->r[0]);
	}
}

unsigned darkbn6_ran_take(void) {
	unsigned r = ran;
	ran = 0;
	return r;
}

unsigned darkbn6_base_take(void) {
	unsigned r = base_ran;
	base_ran = 0;
	return r;
}

/* ---- the hooks: MegaMan's side's chip uses only ---- */

/* a chip's after-effects as it runs (BN6_DARK_AFTER): a DarkChip's id
 * reaches it only where its dark power ran, a BugFrag paid */
static HookAct after(HookRegs *r, void *user) {
	(void)user;
	if (r->r[0] - BN6_DARK_FIRST_ID < (uint32_t)DARK_BN6_COUNT && hook_read8(r->r[5] + BN6_T1_ALLIANCE) == 0) hook_post(r, EV_DARK_RAN);
	return HOOK_CONTINUE;
}

/* every chip use's DarkChip check (BN6_DARK_CHECK): a DarkChip with no
 * BugFrag in the battle's count becomes its base chip */
static HookAct check(HookRegs *r, void *user) {
	(void)user;
	if (r->r[0] < (uint32_t)DARK_BN6_COUNT && hook_read8(r->r[5] + BN6_T1_ALLIANCE) == 0 && !hook_read32(BN6_BATTLE_BUGFRAGS)) hook_post(r, EV_DARK_BASE);
	return HOOK_CONTINUE;
}

void darkbn6_install(void) {
	static bool done;
	if (done) return;
	done = true;
	emu_hook(BN6_DARK_AFTER, after, NULL);
	emu_hook(BN6_DARK_CHECK, check, NULL);
}
