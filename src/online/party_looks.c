/* Each seat's look on the shared dive's field (party_looks.h; issue #879,
 * docs/online/shared-dive.md, Looks on the field; the palettes and picks in
 * docs/rom/megaman-colours.md, NetBattles). */
#include "party_looks.h"

#include <string.h>

#include <mgba/core/core.h>

#include "bn6.h"
#include "bn6_forms.h"
#include "bn6_look.h"
#include "look_apply.h"
#include "soullife.h"

#define BUS 0x08000000u
/* each seat's plain body and its pale twin in the block's spare palettes:
 * four seats take all eight, so Full Synchro and the emotion window's face
 * stay BN6's on the field */
enum { AS_PLAIN, AS_PALE, AS_KINDS };
_Static_assert(PARTY_MOST * AS_KINDS <= BN6_LOOK_PAL_SPARES, "every seat's palettes in the block's spare ones");

static const uint8_t spare[BN6_LOOK_PAL_SPARES] = BN6_LOOK_PAL_SPARE;
static const uint8_t kinds[AS_KINDS] = { BN6_LOOK_PAL_PLAIN, BN6_LOOK_PAL_PALE };
static const int sprite_of[AS_KINDS] = { LOOK_SPRITE_BATTLE, LOOK_SPRITE_BATTLE_PALE };

static struct {
	bool on;
	bool looked[PARTY_MOST];
} P;

static uint32_t pal_at(int palette) { return BN6_LOOK_BATTLE_PALS + (uint32_t)palette * BN6_LOOK_PAL_SIZE; }

void party_looks_rom(uint8_t *rom, size_t size, const OnlineLoadout *seat, unsigned n) {
	memset(&P, 0, sizeof P);
	look_rom_plain(rom, size);
	if (pal_at(BN6_LOOK_BATTLE_PAL_COUNT) - BUS > size) return;
	for (unsigned k = 0; k < n && k < PARTY_MOST; ++k) {
		if (!seat[k].looked || !look_valid(&seat[k].look)) continue;
		for (int a = 0; a < AS_KINDS; ++a) {
			uint16_t pal[16];
			if (!look_palette_of(&seat[k].look, sprite_of[a], pal)) continue;
			uint8_t *at = rom + pal_at(spare[k * AS_KINDS + (unsigned)a]) - BUS;
			for (int i = 0; i < 16; ++i) at[2 * i] = (uint8_t)pal[i], at[2 * i + 1] = (uint8_t)(pal[i] >> 8);
		}
		P.looked[k] = true;
		P.on = true;
	}
}

/* ---- the hooks (user: the field) ---- */

/* the seat an object is: side 0's slot that holds it, the slot traded in
 * while an enemy's routine runs (party_field.c's logic_in) traded back;
 * -1 none */
static int seat_of(uint32_t obj, const PartyField *f) {
	if (!obj || hook_read8(obj + BN6_T1_ALLIANCE) != 0) return -1;
	for (int k = 0; k < f->standing && k < PARTY_MOST; ++k) {
		if (hook_read32(BN6_BATTLE_PLAYERS + (uint32_t)k * BN6_SIDE_PARTNER) != obj) continue;
		int s = (unsigned)k == f->swapped ? 0 : k == 0 && f->swapped ? (int)f->swapped : k;
		return P.looked[s] ? s : -1;
	}
	return -1;
}

/* MegaMan as BN6 draws him in no form: his own Navi, no Cross, Soul or Beast Out */
static bool unformed(uint32_t obj) {
	uint32_t stats = soul_navi_stats(obj);
	return !hook_read8(stats + BN6_NAVI_SIDE_NAVI) && !hook_read8(stats + BN6_NAVI_FORM);
}

/* (the pick's end, r4 the palette, r5 the object) */
static HookAct pick_end(HookRegs *r, void *user) {
	for (int a = 0; a < AS_KINDS; ++a) {
		if (r->r[4] != kinds[a]) continue;
		int s = seat_of(r->r[5], user);
		if (s >= 0 && unformed(r->r[5])) r->r[4] = spare[(unsigned)s * AS_KINDS + (unsigned)a];
		break;
	}
	return HOOK_CONTINUE;
}

/* (an object drawn as MegaMan by his side's form, r0 the form) */
static HookAct form_read2(HookRegs *r, void *user) {
	int s = r->r[0] ? -1 : seat_of(r->r[5], user);
	if (s < 0 || !unformed(r->r[5])) return HOOK_CONTINUE;
	r->r[0] = spare[(unsigned)s * AS_KINDS + AS_PLAIN];
	r->r[12] = BN6_LOOK_FORM_READ2_SET | 1;
	return HOOK_JUMP;
}

bool party_looks_hooks(HookCore *core, PartyField *f) {
	if (!P.on) return true;
	return hook_on(core, BN6_LOOK_PICK_END, pick_end, f) && hook_on(core, BN6_LOOK_FORM_READ2, form_read2, f);
}

/* ---- the state ---- */

static bool in_obj_palettes(struct mCore *c, uint32_t at) {
	for (unsigned slot = 0; slot < 16; ++slot) {
		bool same = true;
		for (unsigned i = 1; same && i < 16; ++i) same = c->rawRead16(c, 0x05000200u + slot * 0x20u + 2 * i, -1) == c->rawRead16(c, at + 2 * i, -1);
		if (same) return true;
	}
	return false;
}

void party_looks_describe(FILE *out, struct mCore *field, unsigned n) {
	if (!field) return;
	fprintf(out, "partylooks");
	for (unsigned k = 0; k < n && k < PARTY_MOST; ++k) {
		const char *says = "-";
		if (P.looked[k])
			says = in_obj_palettes(field, pal_at(spare[k * AS_KINDS + AS_PLAIN])) || in_obj_palettes(field, pal_at(spare[k * AS_KINDS + AS_PALE])) ? "drawn" : "unseen";
		uint32_t obj = (uint32_t)field->rawRead32(field, BN6_BATTLE_PLAYERS + k * BN6_SIDE_PARTNER, -1);
		fprintf(out, " %u:%s", k, says);
		/* (where his MegaMan stands, column and row: which of the field's MegaMen is whose) */
		if (obj) fprintf(out, "@%u,%u", (unsigned)field->rawRead8(field, obj + BN6_T1_PANEL_X, -1), (unsigned)field->rawRead8(field, obj + BN6_T1_PANEL_Y, -1));
	}
	fputc('\n', out);
}
