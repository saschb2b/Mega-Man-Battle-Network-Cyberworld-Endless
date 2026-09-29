/* The game decides when a random battle happens (its own step counter); the
 * roll that picks which battle returns a BattleSettings record the engine
 * writes into the free ROM space. Layout per the bn6f disassembly's
 * battle_settings_struct: field, unk, music, type, background, number,
 * side modifier, unk, options (u32), then a pointer to the entity list of
 * 4-byte entries (kind, panel y<<4|x, id) ending with 0xF0. */
#include "encounter.h"

#include <stdbool.h>
#include <string.h>

#include "bn6.h"
#include "bytes.h"
#include "data.h"
#include "emu.h"
#include "loot.h"
#include "rom.h"
#include "run.h"

#define ROLL        0x080ABD30u        /* the encounter roll (returns BattleSettings*) */
/* Two records in turn, each BattleSettings and 0x20 on its entity list: the
 * next battle is written to the one the roll does not hand out, and the
 * roll turned to it. A battle the game has rolled but not yet set up reads
 * the record it was handed (rewritten in place, a re-roll in that gap gave
 * the battle another's MegaMan panel and field). */
#define RECORDS     (EMU_FREE + 0x200)
#define RECORD_SIZE 0x80
static int slot;
static uint32_t settings_of(int s) { return RECORDS + (uint32_t)s * RECORD_SIZE; }

/* The roll keeps its own chance: its first 12 bytes (position independent)
 * move into a trampoline that jumps back into it, and a wrapper swaps a
 * non-NULL result for the engine's record. */
#define WRAPPER (EMU_FREE + 0x180)
#define TRAMP   (WRAPPER + 20)

void emu_encounters_install(void) {
	static bool done;
	if (done) return;
	done = true;
	uint8_t orig[12];
	for (int i = 0; i < 12; ++i) orig[i] = emu_read8(ROLL + (uint32_t)i);
	/* wrapper: push {lr}; bl tramp; cmp r0,#0; beq 1f; ldr r0,=SETTINGS; 1: pop {pc} */
	uint8_t w[20] = { 0x00, 0xB5, 0x00, 0xF0, 0x07, 0xF8, 0x00, 0x28, 0x00, 0xD0, 0x01, 0x48, 0x00, 0xBD, 0xC0, 0x46 };
	put32(w + 16, settings_of(slot));
	/* trampoline: the roll's first 12 bytes, then ldr r3,[pc]; bx r3 back into it */
	uint8_t t[20];
	memcpy(t, orig, 12);
	t[12] = 0x00; t[13] = 0x4B; t[14] = 0x18; t[15] = 0x47;
	put32(t + 16, ROLL + 12 + 1);
	/* the roll itself: ldr r3,[pc,#4]; bx r3; nop; nop; .word wrapper */
	uint8_t hook[12] = { 0x01, 0x4B, 0x18, 0x47, 0xC0, 0x46, 0xC0, 0x46 };
	put32(hook + 8, WRAPPER + 1);
	emu_write(WRAPPER, w, sizeof w);
	emu_write(TRAMP, t, sizeof t);
	emu_write(ROLL, hook, sizeof hook);
}

/* A battle right now: the roll returns the record unconditionally and the
 * overworld's encounter check branches straight to it (0x05A98: b to the
 * roll), until emu_battle_release once the battle has begun. */
#define CHECK 0x08005A98u
static uint8_t saved_roll[12], saved_check[2];
static bool forcing;

void emu_battle_force(const Encounter *e) {
	emu_encounter_set(e);
	if (forcing) return;
	for (int i = 0; i < 12; ++i) saved_roll[i] = emu_read8(ROLL + (uint32_t)i);
	for (int i = 0; i < 2; ++i) saved_check[i] = emu_read8(CHECK + (uint32_t)i);
	/* ldr r0,[pc,#4]; tst r0,r0; bx lr; nop; .word the record */
	uint8_t stub[12] = { 0x01, 0x48, 0x00, 0x42, 0x70, 0x47, 0xC0, 0x46 };
	put32(stub + 8, settings_of(slot));
	emu_write(ROLL, stub, sizeof stub);
	static const uint8_t branch[2] = { 0x21, 0xE0 };
	emu_write(CHECK, branch, 2);
	forcing = true;
}

void emu_battle_release(void) {
	if (!forcing) return;
	emu_write(ROLL, saved_roll, sizeof saved_roll);
	emu_write(CHECK, saved_check, sizeof saved_check);
	forcing = false;
}

bool emu_battle_forcing(void) { return forcing; }

void emu_encounter_set(const Encounter *e) {
	uint8_t list[4 * (MAX_FOES + 1 + MAX_FIELD_OBJS) + 1], *p = list;
	int next = slot ^ 1;
	/* MegaMan where the battle's field has him: column 2 row 2, or beside
	 * it where that panel is a hole (he started in one) */
	*p++ = 0x00; *p++ = (uint8_t)(e->player ? e->player : 0x22); *p++ = 0; *p++ = 0;
	for (int i = 0; i < e->nfoes && i < MAX_FOES; ++i) {
		const Foe *f = &e->foes[i];
		int id = f->id;
		if (id < 0) id = enemy_id(f->kind == FOE_NAVI ? 1 : 0, f->family, f->version);
		if (id < 0) id = enemy_id(f->kind == FOE_NAVI ? 1 : 0, f->family, 0);
		if (id < 0) continue;
		*p++ = 0x11;
		*p++ = (uint8_t)((f->row + 1) << 4 | (f->col + 1));
		*p++ = (uint8_t)id;
		*p++ = (uint8_t)(id >> 8);
	}
	/* the field's objects as the area's battle has them (rocks, cubes,
	 * statues); a Mystery Data takes this record's reward row (its byte
	 * the row, the tier, over 0x0F: the spawn keeps it where that byte
	 * beats RNG % 15, so always), its rewards rolled into the row in the
	 * core's copy: one of them is the battle's second reward where it is
	 * still there at the end (bn6f sub_80DA9FE, sub_80AA8E0) */
	bool gem = false;
	for (int i = 0; i < e->nobj && i < MAX_FIELD_OBJS; ++i) {
		int arg = e->obj[i].arg;
		if (e->obj[i].kind >> 4 == FIELD_GEM) {
			if (gem) continue;
			arg = next << 4 | 0x0F;
			gem = true;
		}
		*p++ = (uint8_t)e->obj[i].kind;
		*p++ = (uint8_t)e->obj[i].panel;
		*p++ = (uint8_t)arg;
		*p++ = (uint8_t)(arg >> 8);
	}
	if (gem && R.layout->battle_gem_rewards) {
		uint16_t r[4];
		loot_gem_rewards(run.depth, r);
		uint8_t row[16];
		for (int k = 0; k < 8; ++k) { row[2 * k] = (uint8_t)r[k / 2]; row[2 * k + 1] = (uint8_t)(r[k / 2] >> 8); }
		emu_write(0x08000000u + R.layout->battle_gem_rewards + (uint32_t)next * 16, row, sizeof row);
	}
	*p++ = 0xF0;
	slot = next;
	uint32_t settings = settings_of(slot);
	emu_write(settings + 0x20, list, (size_t)(p - list));
	/* the values of a Central Area random battle, with the formation's
	 * battlefield, the area's background and the virus or boss theme */
	uint8_t s[16] = { (uint8_t)e->field, 0x36, (uint8_t)(e->boss ? 0x16 : 0x15), 0x00, (uint8_t)biome_bg(e->biome), 0x00, 0x38, 0x00 };
	/* (bit 0x20 of the options lets MegaMan run, bn6f 0x08026EC8: the
	 * story's bosses clear it, the random battles and BN6's roaming SP
	 * Navis set it; docs/ROM_DATA.md) */
	put32(s + 8, e->held ? 0x000049C2 : 0x000049E2);
	put32(s + 12, settings + 0x20);
	emu_write(settings, s, sizeof s);
	uint8_t at[4];
	put32(at, settings);
	emu_write(WRAPPER + 16, at, 4);
	if (forcing) emu_write(ROLL + 8, at, 4);
}

int emu_encounter_slot(void) { return slot; }

/* eToolkit's battle state, where the setup keeps its BattleSettings pointer
 * (bn6f), a few frames into it */
static uint32_t battle_settings_at(void) {
	uint32_t state = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_BATTLE);
	return state >= 0x02000000u && state < 0x02040000u ? state + 0x3C : 0;
}

int emu_encounter_battle_slot(void) {
	uint32_t p = battle_settings_at(), at = p ? emu_read32(p) : 0;
	return !at ? -2 : at == settings_of(0) ? 0 : at == settings_of(1) ? 1 : -1;
}

void emu_encounter_battle_forget(void) {
	uint32_t p = battle_settings_at();
	if (p && emu_read32(p)) emu_write32(p, 0);
}

bool emu_encounter_lean_drops(void) {
	/* (BN6 picks the reward as the battle ends, bn6f sub_80AA910: one of
	 * the enemies spawned, then one of its row's 20 entries by the busting
	 * level, a coin and MegaMan's HP; a chip entry has bits 14-15 clear,
	 * the id in 0-8 and the code in 9-13. The coin picks between the two
	 * entries of a pair: the second of each, rewritten from the ROM's own
	 * in the core's copy, comes in one of the folder's codes where the
	 * chip does, so a chip reward leans half the time, as Mystery Data's;
	 * a Navi's chip, in either entry, in its * off the folder's codes) */
	uint32_t state = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_BATTLE);
	if (state < 0x02000000u || state >= 0x02040000u) return false;
	int n = emu_read8(state + BN6_BATTLE_ENEMY_COUNT);
	if (n <= 0) return false;
	for (int i = 0; i < n && i < 8; ++i) {
		int id = emu_read16(state + BN6_BATTLE_ENEMY_IDS + 2u * (uint32_t)i);
		if (id <= 0 || id >= 0x200) continue;
		uint32_t row = BN6_DROP_ROWS - 0x08000000u + (uint32_t)id * 0x28;
		for (uint32_t k = 0; k < 20; ++k) {
			uint16_t v = rom_u16(row + 2 * k);
			if (v == 0xFFFF || v >> 14) continue;
			int chip = v & 0x1FF, code = v >> 9 & 0x1F;
			char c = code >= 26 ? '*' : (char)('A' + code);
			if (k & 1) c = loot_fit_code(chip, c, true);
			/* (a Navi's chip in its * where the folder holds not its
			 * letter, as his Guardian Data gives it, from both entries of
			 * a pair: a Blade folder's guardian dropped ChrgeMan C, and
			 * with the second entry alone, SpoutMan A) */
			bool held = false;
			for (int h = 0; h < 3 && run.codes[h]; ++h) held |= c == 'A' + run.codes[h] - 1;
			if (!held && c != '*' && chip_family(chip) == CHIP_FAMILY_NAVI) {
				ChipInfo ci;
				chip_info(chip, &ci);
				if (memchr(ci.codes, '*', (size_t)ci.ncodes)) c = '*';
			}
			uint16_t w = (uint16_t)((v & ~(0x1F << 9)) | (c == '*' ? 26 : c - 'A') << 9);
			uint8_t b[2] = { (uint8_t)w, (uint8_t)(w >> 8) };
			emu_write(0x08000000u + row + 2 * k, b, 2);
		}
	}
	return true;
}
