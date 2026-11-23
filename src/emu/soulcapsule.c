/* MeddySoul's capsules on BN6's own Custom screen (soulcapsule.h). */
#include "soulcapsule.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bn5_capsules.h"
#include "bn6.h"
#include "bn6_chips.h"
#include "bn6_forms.h"
#include "bn6_panels.h"
#include "bytes.h"
#include "chip_entry.h"
#include "chips.h"
#include "claims.h"
#include "emu.h"
#include "rom.h"
#include "soul_room.h"
#include "soulcode.h"
#include "soullife.h"
#include "soulteam.h"
#include "thumb_reloc.h"
#include "xchips_registry.h"

#define BUS 0x08000000u
#define CLAIMS "MeddySoul's capsules (soulcapsule.c)"
#define ELEMENT_CAPSULE 8          /* the capsules' elements, 8 and 9 (BN5's +0x60 and +0x6C of the elements, 0x08023D22) */
#define STATE_LIT 0
#define STATE_GREY 1
#define STATE_CHOSEN 2
#define PICKS_FIRST 0x47           /* the screen's +0x48 on its picks' slots, read as [r5 + 0x47 + count] (0x080237D2) */
#define SCREEN_PHASE 0x02          /* the screen's: u16 the mode's phase, */
#define SCREEN_TIMER 0x40          /* ... u32 a phase's frames, */
#define SCREEN_FLIGHT 0x44         /* ... u32 where the flying icon goes (the picks x 16 + 0x18) */
#define TABLE_ENTRIES 16           /* each capsule table's (BN5_CAPSULES_HEALTHY, BN5_CAPSULES_HURT) */
#define ICON_BYTES 0x80
#define PAL_BYTES 0x20
#define FLIGHT_TILES 0x060172C0u   /* BN6's UNITE flight's icon tiles (0x0802709E; BN5's 0x06017240), the OBJ its draw makes (0x080298D8) */
#define FLIGHT_PAL 0x030016F0u     /* ... and its palette's place (0x080270B2; BN5's 0x030036F0) */
#define HP_MAX 0x26                /* an object's max HP (the heal's, BN5's 0x0800FFF8) */
#define CAPSULE_DAMAGE_BITS 0x7800u   /* the damage word's bits a capsule gives (white 0x4000, yellow 0x2000, black 0x1000, purple 0x0800) */

/* the room (SOULS_CAPSULES): */
#define CAPSULE_MODE   (SOULS_CAPSULES + 0x00u)   /* mode 0x3C's routine and its phases' table, */
#define CAPSULE_PHASE0 (SOULS_CAPSULES + 0x48u)   /* its first phase's stub (mov pc,lr: the hook's), */
#define CAPSULE_A      (SOULS_CAPSULES + 0x50u)   /* A on a capsule, */
#define CAPSULE_TABLES (SOULS_CAPSULES + 0x80u)   /* BN5's two tables in BN6's entries, 16 u16 each, */
#define CAPSULE_BITS   (SOULS_CAPSULES + 0xC0u)   /* BN5's bits by capsule (BN5_CAPSULE_BITS), */
#define CAPSULE_IDS    (SOULS_CAPSULES + 0xC8u)   /* ... and each capsule's BN6 id, u16, */
#define CAPSULE_SETUP  (SOULS_CAPSULES + 0xD4u)   /* BN5's set-up moved, called with r7 the Soul (6) */
#define SETUP_SIZE 0x14u
#define MODE_SIZE 0x44u
#define A_SIZE 0x30u
_Static_assert(CAPSULE_MODE + MODE_SIZE <= CAPSULE_PHASE0 && CAPSULE_A + A_SIZE <= CAPSULE_TABLES &&
	CAPSULE_IDS + 2u * BN5_CAPSULE_COUNT <= CAPSULE_SETUP && CAPSULE_SETUP + SETUP_SIZE <= SOULS_CAPSULES_END,
	"the capsules' room as its parts take it");

/* BN5's capsules' BN6 ids, by BN5's id - 380: the registry's two (YelCapsl, BlkCapsl), BN6's own three by name */
static const uint16_t ids6[BN5_CAPSULE_COUNT] = { XREG_CAPSULE_FIRST, XREG_CAPSULE_FIRST + 1, 382, 383, 384 };

static bool done;

static uint32_t element(int k) { return BN6_CUSTOM_ELEMENTS + (uint32_t)k * BN6_ELEMENT_SIZE; }
static int screen_side(void) { return hook_read8(BN6_BATTLE_SIDE) & 1; }
static bool meddy_screen(void) {
	int side = screen_side();
	return done && soul_united(side) && hook_read8(soul_field(side, BN6_SOUL_ID)) == SOUL_MEDDY;
}
static unsigned entry_id(uint32_t el) { return chip_entry_id(hook_read16(hook_read32(el + BN6_ELEMENT_DECK))); }

/* ---- the screen ---- */

/* (soulcolonel.c's set-up hook) BN5's set-up moved (0x08023CF4, xsoul's: its steps, its RNG call BN6's twin, its tables
 * CAPSULE_TABLES), run for Soul 6 through CAPSULE_SETUP */
HookAct soulcapsule_setup(HookRegs *r) {
	if (!done) return HOOK_CONTINUE;
	r->r[12] = CAPSULE_SETUP | 1;
	return HOOK_JUMP;
}

HookAct soulcapsule_light(HookRegs *r) {
	uint32_t screen = r->r[5], e8 = element(ELEMENT_CAPSULE), e9 = element(ELEMENT_CAPSULE + 1);
	if (!done) return HOOK_CONTINUE;
	if (hook_read8(e8 + BN6_ELEMENT_TYPE) != BN5_ELEMENT_CAPSULE) return HOOK_RETURN;
	int n = hook_read8(screen + 8);
	bool lit = false;
	if (n) {
		uint32_t el = element(hook_read8(screen + PICKS_FIRST + (uint32_t)n));
		if (hook_read8(el + BN6_ELEMENT_TYPE) == 0 && !hook_read8(el + BN5_ELEMENT_CAPSULE_ON)) {
			uint32_t rec = chips_table_bus() + entry_id(el) * BN6_CHIP_RECORD_SIZE;
			lit = (hook_read8(rec + BN6_CHIP_EFFECT_FLAGS) & BN6_CHIP_SHOWS_POWER) != 0;
		}
	}
	uint8_t to = lit ? STATE_LIT : STATE_GREY;
	if (hook_read8(e8 + BN6_ELEMENT_STATE) != STATE_CHOSEN) hook_write8(e8 + BN6_ELEMENT_STATE, to);
	if (hook_read8(e9 + BN6_ELEMENT_STATE) != STATE_CHOSEN) hook_write8(e9 + BN6_ELEMENT_STATE, to);
	return HOOK_RETURN;
}

/* (BN6_A_CAPSULE: mov pc,lr) A on a capsule: BN5's (0x08024A06), the project's Thumb (CAPSULE_A) */
static HookAct a_capsule(HookRegs *r, void *user) {
	(void)user;
	if (!meddy_screen()) return HOOK_CONTINUE;
	r->r[12] = CAPSULE_A | 1;
	return HOOK_JUMP;
}

/* (BN6_MODE_CAPSULE_STUB: mode 0x3C back to 4) BN5's mode 0x3C, the project's Thumb (CAPSULE_MODE) */
static HookAct mode_capsule(HookRegs *r, void *user) {
	(void)user;
	if (!meddy_screen()) return HOOK_CONTINUE;
	r->r[12] = CAPSULE_MODE | 1;
	return HOOK_JUMP;
}

/* (CAPSULE_PHASE0, mov pc,lr) BN5's first phase (0x0802376C) as BN6's UNITE flight's first (0x0802707C) takes it: the capsule's
 * icon (its record's, BN6's or the registry's) where BN6's flight draws it, the flight's palette, where it flies to */
static HookAct phase0(HookRegs *r, void *user) {
	(void)user;
	uint32_t screen = r->r[5];
	hook_write32(screen + SCREEN_TIMER, 0);
	uint32_t icon = hook_read32(chips_table_bus() + entry_id(element(hook_read8(screen + 7))) * BN6_CHIP_RECORD_SIZE + BN6_CHIP_ICON_PTR);
	for (uint32_t i = 0; i < ICON_BYTES; i += 4) hook_write32(FLIGHT_TILES + i, hook_read32(icon + i));
	hook_write32(screen + SCREEN_FLIGHT, (uint32_t)hook_read8(screen + 8) * 0x10u + 0x18u);
	uint32_t pal = hook_read32(BN6_UNITE_PICTURE_PAL);
	for (uint32_t i = 0; i < PAL_BYTES; i += 4) hook_write32(FLIGHT_PAL + i, hook_read32(pal + i));
	hook_write16(screen + SCREEN_PHASE, 4);
	return HOOK_CONTINUE;
}

/* (BN5's phase 0x14 moved, at 0x080237BA: the flight landed) the capsule chosen and put on the last pick: its element in the
 * pick's +5, its bit in the pick's +4 (BN5's bits by its id, here by its BN6 id), then on to the moved lit rules' call */
static HookAct take(HookRegs *r, void *user) {
	uint32_t screen = r->r[5];
	int cursor = hook_read8(screen + 7);
	uint32_t cap = element(cursor), pick = element(hook_read8(screen + PICKS_FIRST + hook_read8(screen + 8)));
	hook_write8(cap + BN6_ELEMENT_STATE, STATE_CHOSEN);
	unsigned id = entry_id(cap);
	uint8_t bit = 0;
	for (uint32_t k = 0; k < BN5_CAPSULE_COUNT; ++k)
		if (hook_read16(CAPSULE_IDS + 2 * k) == id) bit = hook_read8(CAPSULE_BITS + k);
	hook_write8(pick + BN5_ELEMENT_CAPSULE_ON, (uint8_t)cursor);
	hook_write8(pick + BN5_ELEMENT_FLAGS, hook_read8(pick + BN5_ELEMENT_FLAGS) | bit);
	r->r[12] = (uint32_t)(uintptr_t)user | 1;
	return HOOK_JUMP;
}

/* ---- the fight ---- */

/* (BN6_CHIP_FLAGS_DAMAGE, r0 the damage, r1 the chip's flags) BN5's two bits Gregar dropped (0x08010374-0x08010386): black
 * 0x1000, purple 0x0800; BN6's own two (white, yellow) after */
static HookAct flags(HookRegs *r, void *user) {
	(void)user;
	if (done) r->r[0] |= (r->r[1] & 0x08 ? 0x1000u : 0) | (r->r[1] & 0x20 ? 0x0800u : 0);
	return HOOK_CONTINUE;
}

/* (BN6_CHIP_HEAL: movs r2,#0, no heal of BN6's own) BN5's pink capsule (0x0800FFEE): a tenth of his max HP, rounded up, healed
 * with the chip (its flags at sp + 0x14, r5 him) */
static HookAct heal(HookRegs *r, void *user) {
	(void)user;
	if (!done || !(hook_read8(r->sp + 0x14) & 0x10)) return HOOK_CONTINUE;
	r->r[2] = (hook_read16(r->r[5] + HP_MAX) + 9u) / 10u;
	r->r[12] = BN6_CHIP_HEAL_NEXT | 1;
	return HOOK_JUMP;
}

/* (BN6_DAMAGE_DECODE, r0 a collision data: its damage word into its hit) BN5's (0x080165E4) where MeddySoul's side's attack
 * carries a capsule's bit, which Gregar reads otherwise (0x2000 and 0x1000 as its +0x12's 0xF8 and 0xF7, 0x0800 not at all):
 * the damage's eleven bits, doubled by 0x8000; white 0x4000 the status 0x10 (BN6's and BN5's tables' paralysis, 90 frames)
 * and +0xE 0; yellow 0x2000 0x20 (confusion, 480); black 0x1000 0x30 (blindness, 480); purple 0x0800 its +0x12 0x1118 */
static HookAct decode(HookRegs *r, void *user) {
	(void)user;
	uint32_t coll = r->r[0];
	uint16_t d = hook_read16(coll + BN6_COLL_DAMAGE);
	int side = hook_read8(coll + BN6_COLL_ALLIANCE) & 1;
	if (!done || !(d & CAPSULE_DAMAGE_BITS) || !soul_united(side) || hook_read8(soul_field(side, BN6_SOUL_ID)) != SOUL_MEDDY)
		return HOOK_CONTINUE;
	uint16_t power = d & 0x7FF;
	hook_write16(coll + BN6_COLL_DAMAGE, d & 0x8000 ? (uint16_t)(power + power) : power);
	if (d & 0x4000) {
		hook_write8(coll + BN6_COLL_STATUS_GIVEN, 0x10);
		hook_write8(coll + BN6_COLL_SETUP_ARG, 0);
	} else if (d & 0x2000) {
		hook_write8(coll + BN6_COLL_STATUS_GIVEN, 0x20);
	} else {
		if (d & 0x1000) hook_write8(coll + BN6_COLL_STATUS_GIVEN, 0x30);
		if (d & 0x0800) hook_write16(coll + BN6_COLL_PUSH, 0x1118);
	}
	return HOOK_RETURN;
}

/* ---- the Thumb ---- */

static void put_words(uint8_t *out, const uint16_t *t, int n) {
	for (int i = 0; i < n; ++i) put16(out + 2 * i, t[i]);
}

/* mode 0x3C (CAPSULE_MODE): its phase from the table, then BN6's draws as its modes run them (0x08029C08, 0x0802899C: BN5's
 * 0x08025828 and 0x08024798); the table BN5's (0x08023750): the first phase the stub, then BN6's UNITE flight (0x080270D0-
 * 0x08027134, BN5's 0x0802335C-0x080233C0 the same steps), then BN5's last two moved */
static bool mode_code(uint8_t out[MODE_SIZE], uint32_t p14, uint32_t p18) {
	static const uint16_t t[] = { 0xB500, 0x4906, 0x78A8, 0x5809, 0x46FE, 0x4708, 0x4B04, 0, 0, 0x4B04, 0, 0, 0xBD00, 0x4718 };
	put_words(out, t, (int)(sizeof t / sizeof *t));
	put32(out + 0x1C, CAPSULE_MODE + 0x28);
	put32(out + 0x20, BN6_CUSTOM_DRAW_A | 1);
	put32(out + 0x24, BN6_CUSTOM_DRAW_B | 1);
	const uint32_t phases[7] = { CAPSULE_PHASE0, BN6_UNITE_FLIGHT, BN6_UNITE_FLIGHT_8, BN6_UNITE_FLIGHT_C, BN6_UNITE_FLIGHT_10, p14, p18 };
	for (int i = 0; i < 7; ++i) put32(out + 0x28 + 4 * i, phases[i] | 1);
	return thumb_bl_write(out, 0x0E, CAPSULE_MODE, CAPSULE_MODE + 0x1A) && thumb_bl_write(out, 0x14, CAPSULE_MODE, CAPSULE_MODE + 0x1A);
}

/* A on a capsule (CAPSULE_A, r4 the element, r5 the screen): BN5's (0x08024A06) step for step, lit: sound 0x81, mode 0x3C,
 * r0 0; grey: BN5's sound 0x69 (at SOUL_TEAM_GREY_SLOT), r0 1 */
static bool a_code(uint8_t out[A_SIZE]) {
	static const uint16_t t[] = { 0xB500, 0x79E0, 0x2800, 0xD108, 0x2081, 0x4B07, 0, 0, 0x203C, 0x7068, 0x2000, 0x8068, 0xBD00,
		0x4804, 0x4B02, 0, 0, 0x2001, 0xBD00, 0x4718 };
	put_words(out, t, (int)(sizeof t / sizeof *t));
	put32(out + 0x28, BN6_PLAY_SOUND | 1);
	put32(out + 0x2C, SOUL_TEAM_GREY_SLOT);   /* (BN5's 0x69, which Gregar has another song of, copied there by soulpower.c) */
	return thumb_bl_write(out, 0x0C, CAPSULE_A, CAPSULE_A + 0x26) && thumb_bl_write(out, 0x1E, CAPSULE_A, CAPSULE_A + 0x26);
}

/* BN5's set-up moved called as BN5's screen calls it, r7 the united Soul (CAPSULE_SETUP; BN6's r7 the form kept) */
static bool setup_code(uint8_t out[SETUP_SIZE], uint32_t moved) {
	static const uint16_t t[] = { 0xB580, 0x2706, 0x4B02, 0, 0, 0xBD80, 0x4718, 0x46C0 };
	put_words(out, t, (int)(sizeof t / sizeof *t));
	put32(out + 0x10, moved | 1);
	return thumb_bl_write(out, 0x06, CAPSULE_SETUP, CAPSULE_SETUP + 0x0C);
}

/* BN5's two tables in BN6's entries (code 27, the capsules' BN6 ids), BN5's bits and the ids; false where BN5's ROM names
 * other than its five capsules */
static bool tables(uint8_t out[CAPSULE_SETUP - CAPSULE_TABLES]) {
	const uint8_t *b5 = XR[XROM_BN5_COLONEL_US].data;
	if (!b5) return false;
	for (uint32_t i = 0; i < 2u * TABLE_ENTRIES; ++i) {
		uint16_t e = get16(b5 + (BN5_CAPSULES_HEALTHY - BUS) + 2 * i);
		unsigned id = chip9_id(e);
		if (id < BN5_CAPSULE_FIRST || id >= BN5_CAPSULE_FIRST + BN5_CAPSULE_COUNT) return false;
		put16(out + 2 * i, chip_entry(ids6[id - BN5_CAPSULE_FIRST], chip9_code(e)));
	}
	for (uint32_t k = 0; k < BN5_CAPSULE_COUNT; ++k) {
		out[CAPSULE_BITS - CAPSULE_TABLES + k] = b5[(BN5_CAPSULE_BITS - BUS) + k];
		put16(out + (CAPSULE_IDS - CAPSULE_TABLES) + 2 * k, ids6[k]);
	}
	return true;
}

/* ---- the install ---- */

static bool expected(void) {
	static const struct { uint32_t at; uint16_t op; } code[] = {
		{ BN6_A_CAPSULE, 0x46F7 }, { BN6_MODE_CAPSULE_STUB, 0x2004 }, { BN6_CHIP_FLAGS_DAMAGE, 0x4A6C }, { BN6_CHIP_HEAL, 0x2200 },
		{ BN6_UNITE_FLIGHT, 0xB500 }, { BN6_DAMAGE_DECODE, 0xB5D0 }, { BN6_CUSTOM_DRAW_A, 0xB500 }, { BN6_CUSTOM_DRAW_B, 0xB570 },
	};
	for (size_t i = 0; i < sizeof code / sizeof *code; ++i)
		if (rom_u16(code[i].at - BUS) != code[i].op) {
			fprintf(stderr, "soulcapsule: %08x is not the instruction expected\n", (unsigned)code[i].at);
			return false;
		}
	for (uint32_t a = SOULS_CAPSULES; a < SOULS_CAPSULES_END; a += 2)
		if (emu_read16(a) != 0xFFFFu) return false;
	return true;
}

bool soulcapsule_install(int brk) {
	(void)brk;
	if (done) return true;
	uint32_t p14 = soulcode_moved(BN5_CAPSULE_PHASE5 | 1), p18 = soulcode_moved(BN5_CAPSULE_PHASE6 | 1), landed = soulcode_moved(BN5_CAPSULE_LANDED);
	uint32_t lit = soulcode_moved(BN5_CAPSULE_LIT_CALL), set = soulcode_moved(BN5_SOUL_BUTTON);
	uint8_t mode[MODE_SIZE], a[A_SIZE], tab[CAPSULE_SETUP - CAPSULE_TABLES], su[SETUP_SIZE];
	memset(tab, 0xFF, sizeof tab);
	if (!R.data || !emu_ready() || !p14 || !p18 || !landed || !lit || !set || !expected() || !mode_code(mode, p14, p18) || !a_code(a) ||
		!tables(tab) || !setup_code(su, set))
		return false;
	static const uint8_t mov_pc_lr[2] = { 0xF7, 0x46 };   /* (mov pc,lr: the mode calls its phase by mov lr,pc, no Thumb bit) */
	claims_begin(CLAIMS);
	emu_write(CAPSULE_MODE, mode, sizeof mode);
	emu_write(CAPSULE_PHASE0, mov_pc_lr, sizeof mov_pc_lr);
	emu_write(CAPSULE_A, a, sizeof a);
	emu_write(CAPSULE_TABLES, tab, sizeof tab);
	emu_write(CAPSULE_SETUP, su, sizeof su);
	bool ok = emu_hook(BN6_A_CAPSULE, a_capsule, NULL) && emu_hook(BN6_MODE_CAPSULE_STUB, mode_capsule, NULL) &&
		emu_hook(CAPSULE_PHASE0, phase0, NULL) && emu_hook(landed, take, (void *)(uintptr_t)lit) &&
		emu_hook(BN6_CHIP_FLAGS_DAMAGE, flags, NULL) && emu_hook(BN6_CHIP_HEAL, heal, NULL) && emu_hook(BN6_DAMAGE_DECODE, decode, NULL);
	claims_end();
	done = ok;
	printf("soulcapsule: MeddySoul's capsules%s\n", ok ? " on BN6's own elements 6 and 7, their bits and heal in the fight" : " NOT placed whole");
	return ok;
}
