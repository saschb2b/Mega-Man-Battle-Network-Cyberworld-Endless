/* MegaMan's look onto his palettes in the core's ROM copy (look_apply.h;
 * docs/game/looks.md): each art's palette written from look_palette_of,
 * which computes from BN6's own bytes, so LOOK_NONE is BN6's bytes again. */
#include "look_apply.h"

#include <stdio.h>

#include "bn6.h"
#include "emu.h"
#include "rom.h"

/* ---- the core's copy ---- */

static Look applied;
static bool applied_on;

/* Whether the core's OBJ work palette holds sprite `which`'s palette for the
 * look applied: a slot given this frame, or a fixed one */
static bool seen(int which) {
	uint16_t want[16];
	if (!look_palette_of(&applied, which, want)) return false;
	for (int slot = 0; slot < 16; ++slot) {
		bool same = true;
		for (int i = 1; i < 16 && same; ++i) same = emu_read16(BN6_OBJ_PAL_WORK + 32u * (unsigned)slot + 2u * (unsigned)i) == want[i];
		if (same) return true;
	}
	return false;
}

static void describe_seen(FILE *f) {
	static const char *const names[LOOK_SPRITE_COUNT] = { "battle", "field", "mugshot", "face", "pale", "synchro" };
	fprintf(f, "lookseen");
	int n = 0;
	for (int w = 0; applied_on && w < LOOK_SPRITE_COUNT; ++w)
		if (seen(w)) fprintf(f, " %s", names[w]), ++n;
	fprintf(f, "%s\n", n ? "" : " none");
}

void look_apply(const Look *look) {
	Look plain = look_plain();
	if (!look || !look_valid(look)) look = &plain;
	if (!emu_ready()) return;
	/* (no claim: a recolour of BN6's own bytes and of soullook.c's copy of
	 * the faces, rewritten whenever the look changes, not an install) */
	for (int w = 0; w < LOOK_SPRITE_COUNT; ++w) {
		uint16_t pal[16];
		/* (face 0's: soullook.c's copy in the core, the word repointed) */
		uint32_t at = w == LOOK_SPRITE_FACE ? emu_read32(BN6_FACE_PALS_REF) : look_palette_at(w);
		if (!at || !look_palette_of(look, w, pal)) continue;
		uint8_t bytes[32];
		for (int i = 0; i < 16; ++i) bytes[2 * i] = (uint8_t)pal[i], bytes[2 * i + 1] = (uint8_t)(pal[i] >> 8);
		emu_write(at, bytes, sizeof bytes);
	}
	applied = *look;
	applied_on = R.data != NULL;
	looks_describe_with(describe_seen);
}

static void apply_worn(void) {
	Look w = looks_worn();
	look_apply(&w);
}

void look_apply_worn(void) {
	looks_on_change(apply_worn);
	apply_worn();
}
