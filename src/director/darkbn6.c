/* BN6's own DarkChips in its battles (darkbn6.h). */
#include "darkbn6.h"

#include <stdbool.h>
#include <stdio.h>

#include "bn6.h"
#include "bytes.h"
#include "chips.h"
#include "darkchips.h"
#include "data.h"
#include "debug.h"
#include "emu.h"
#include "events.h"
#include "rom.h"

#define BUS 0x08000000u
#define CHIP_IDS 512   /* (BN6's chip ids: nine bits, the registry's XREG_IDS) */

static unsigned ran, base_ran;   /* the battle's DarkChips: those that ran (dark_ran_add's bits); BN6's whose base chip ran, by DarkChipID */
static int powered = -1;         /* the records' power as last written: 1 their own, 0 their base chips', -1 not yet */

/* chip `id`'s record as the core's copy holds it (src/core/chips.h), and
 * where the game reads it, which a write goes to */
static const uint8_t *record(int id) { return chips_record(id); }
static uint32_t record_bus(int id) { return chips_table_bus() + (uint32_t)id * BN6_CHIP_RECORD_SIZE; }

int darkbn6_base(int id) {
	int d = id - DARK_BN6_FIRST;
	if (!R.data || !R.layout || d < 0 || d >= DARK_BN6_COUNT) return 0;
	uint32_t at = BN6_DARK_BASES - BUS + (uint32_t)d * BN6_DARK_BASE_STEP;
	/* (push {lr}; movs r0, #chip) */
	return rom_u16(at) == 0xB500 && R.data[at + 3] == 0x20 ? R.data[at + 2] : 0;
}

void darkbn6_does(int id, char *out, size_t n) {
	const uint8_t *rec = record(id);
	if (!rec || dark_bn6_kind(id) < 0) { if (n) out[0] = 0; return; }
	int power = get16(rec + BN6_CHIP_ATTACK_POWER), sub = rec[BN6_CHIP_SUBFAMILY];
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
		if (!from || !record(from)) continue;
		emu_write(record_bus(id) + BN6_CHIP_ATTACK_POWER, record(from) + BN6_CHIP_ATTACK_POWER, 2);
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
		uint16_t a = get16(record(c) + BN6_CHIP_ALPHA_SORT);
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
	if (!record(0)) return;
	static uint16_t alpha[DARK_BN6_PLAYED];
	for (int i = 0; i < DARK_BN6_PLAYED; ++i) {
		int id = DARK_BN6_FIRST + i, from = darkbn6_base(id);
		if (!from) continue;
		const uint8_t *rec = record(id);
		uint32_t bus = record_bus(id);
		emu_write8(bus + BN6_CHIP_EFFECT_FLAGS, (uint8_t)(rec[BN6_CHIP_EFFECT_FLAGS] | BN6_CHIP_DARK_CLASS));
		emu_write8(bus + BN6_CHIP_LIBRARY_FLAGS, (uint8_t)(rec[BN6_CHIP_LIBRARY_FLAGS] & ~BN6_CHIP_UNLISTED));
		/* (the icon, picture and palette pointers: the base chip's) */
		emu_write(bus + BN6_CHIP_ICON_PTR, record(from) + BN6_CHIP_ICON_PTR, BN6_CHIP_RECORD_SIZE - BN6_CHIP_ICON_PTR);
		/* (their places in the editor's sorts, 0 on all five: its swap tells
		 * two chips apart by the ID's place and the code, so it took any two
		 * DarkChips for one chip and let one in for another past the folder's
		 * three; by ID their own id, a place no chip holds, by name among the
		 * names before theirs) */
		if (!alpha[i]) alpha[i] = alpha_place(id);
		uint8_t sort[2][2] = { { (uint8_t)alpha[i], (uint8_t)(alpha[i] >> 8) }, { (uint8_t)id, (uint8_t)(id >> 8) } };
		emu_write(bus + BN6_CHIP_ALPHA_SORT, sort[0], 2);
		emu_write(bus + BN6_CHIP_ID_SORT, sort[1], 2);
	}
	powered = -1;
	darkbn6_sync();
}

void darkbn6_sync(void) {
	if (!record(0)) return;
	int own = emu_read32(BN6_BUGFRAGS) > 0;
	if (own != powered) power(own);
}

void darkbn6_battle_begins(void) {
	ran = base_ran = 0;
	darkbn6_sync();
}

void darkbn6_event(const HookEvent *e) {
	if (e->kind == EV_DARK_RAN) {
		/* (by the rule its record names: BN6's DarkChipID, a BugFrag paid; none, BN5's) */
		int id = (int)(e->r[0] % CHIP_IDS);
		const uint8_t *rec = record(id);
		unsigned bit = dark_ran_add(0, id, rec ? rec[BN6_CHIP_DARK_ID] : 0xFF);
		ran |= bit;
		if (bit == DARK_RAN_BN5 && emu_debug_on()) fprintf(stderr, "dark: DarkChip %d ran by BN5's rule, no BugFrag\n", id);
		if (!(bit & DARK_RAN_BN6)) return;
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

/* every DarkChip of the registry, either net's (dark_chip_is), a bit an id: what the hook below tests, made once */
static uint32_t dark_ids[CHIP_IDS / 32];

/* a chip's after-effects as it runs (BN6_DARK_AFTER, from every chip use, 0x080128EA: r0 the chip that ran): a DarkChip of
 * either net's, BN5's own seven and BN6's five (a BN6 one's id reaches it only where its dark power ran, a BugFrag paid; its
 * base chip's where it fell back). A Chaos Unison's DarkChip attack runs through the chip writer alone, never here. */
static HookAct after(HookRegs *r, void *user) {
	(void)user;
	uint32_t id = r->r[0];
	if (id < CHIP_IDS && (dark_ids[id / 32] >> (id % 32) & 1) && hook_read8(r->r[5] + BN6_T1_ALLIANCE) == 0) hook_post(r, EV_DARK_RAN);
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
	for (int id = 1; id < CHIP_IDS; ++id)
		if (dark_chip_is(id)) dark_ids[id / 32] |= 1u << (id % 32);
	emu_hook(BN6_DARK_AFTER, after, NULL);
	emu_hook(BN6_DARK_CHECK, check, NULL);
}
