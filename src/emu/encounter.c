/* The game decides when a random battle happens (its own step counter); the
 * roll that picks which battle returns a BattleSettings record the engine
 * writes into the free ROM space. Layout per the bn6f disassembly's
 * battle_settings_struct: field, unk, music, type, background, number,
 * side modifier, unk, options (u32), then a pointer to the entity list of
 * 4-byte entries (kind, panel y<<4|x, id) ending with 0xF0. */
#include "encounter.h"

#include "data.h"
#include "emu.h"
#include "run.h"

#define ROLL        0x080ABD30u        /* the encounter roll (returns BattleSettings*) */
#define SETTINGS    (EMU_FREE + 0x200)
#define ENTITIES    (EMU_FREE + 0x220)

static void put32(uint8_t *p, uint32_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24); }

void emu_encounters_install(void) {
	/* ldr r0,[pc,#4]; tst r0,r0; bx lr; nop; .word SETTINGS */
	uint8_t stub[12] = { 0x01, 0x48, 0x00, 0x42, 0x70, 0x47, 0xC0, 0x46 };
	put32(stub + 8, SETTINGS);
	emu_write(ROLL, stub, sizeof stub);
}

void emu_encounter_set(const Encounter *e) {
	uint8_t list[4 * 6 + 1], *p = list;
	*p++ = 0x00; *p++ = 0x22; *p++ = 0; *p++ = 0;          /* MegaMan, column 2 row 2 */
	for (int i = 0; i < e->nfoes && i < 4; ++i) {
		const Foe *f = &e->foes[i];
		int id = enemy_id(f->kind == FOE_NAVI ? 1 : 0, f->family, f->version);
		if (id < 0) id = enemy_id(f->kind == FOE_NAVI ? 1 : 0, f->family, 0);
		if (id < 0) continue;
		*p++ = 0x11;
		*p++ = (uint8_t)((f->row + 1) << 4 | (f->col + 1));
		*p++ = (uint8_t)id;
		*p++ = (uint8_t)(id >> 8);
	}
	*p++ = 0xF0;
	emu_write(ENTITIES, list, (size_t)(p - list));
	/* the values of a Central Area random battle, with the area's background
	 * and the virus or boss theme */
	uint8_t s[16] = { 0x00, 0x36, (uint8_t)(e->boss ? 0x16 : 0x15), 0x00, (uint8_t)biome_bg(e->biome), 0x00, 0x38, 0x00 };
	put32(s + 8, 0x000049E2);
	put32(s + 12, ENTITIES);
	emu_write(SETTINGS, s, sizeof s);
}
