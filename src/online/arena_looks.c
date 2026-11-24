/* Each player's look in a NetBattle (arena_looks.h; docs/online/arena/looks.md,
 * the addresses in docs/rom/megaman-colours.md, NetBattles). */
#include "arena_looks.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mgba/core/core.h>

#include "bn6.h"
#include "bn6_forms.h"
#include "bn6_look.h"
#include "hook.h"
#include "look_apply.h"
#include "soullife.h"

#define BUS 0x08000000u
#define PLAYERS 2
/* each player's palettes in the block's spare ones: his plain body, its
 * pale twin, Full Synchro (BN6_LOOK_PAL_PLAIN, _PALE, _SYNCHRO), then each
 * player's emotion face */
enum { AS_PLAIN, AS_PALE, AS_SYNCHRO, AS_KINDS };
#define FACE_SPARE(p) (PLAYERS * AS_KINDS + (p))
_Static_assert(PLAYERS * AS_KINDS + PLAYERS <= BN6_LOOK_PAL_SPARES, "both players' palettes in the block's spare ones");

static const uint8_t spare[BN6_LOOK_PAL_SPARES] = BN6_LOOK_PAL_SPARE;
static const uint8_t kinds[AS_KINDS] = { BN6_LOOK_PAL_PLAIN, BN6_LOOK_PAL_PALE, BN6_LOOK_PAL_SYNCHRO };
static const int sprite_of[AS_KINDS] = { LOOK_SPRITE_BATTLE, LOOK_SPRITE_BATTLE_PALE, LOOK_SPRITE_BATTLE_SYNCHRO };

static struct {
	bool on;
	Look look[PLAYERS];
	/* CYBERWORLD_ARENA_LOOKS_PROBE (a measurement, docs/online/arena/looks.md, Measured): 1 every spare palette marked and
	 * no hook, 2 the looks and their hooks; each frame's OBJ palettes compared with the spares, the hooks' calls counted */
	int probe;
	unsigned frames[PLAYERS], drawn[PLAYERS][BN6_LOOK_PAL_SPARES], hits[PLAYERS][3][PLAYERS][AS_KINDS + 1];
} L;

static bool plain(const Look *look) {
	for (int c = 0; c < LOOK_CHANNELS; ++c)
		if (look->palette[c] != LOOK_NONE) return false;
	return true;
}

void arena_looks_set(const Look *side0, const Look *side1) {
	const Look *in[PLAYERS] = { side0, side1 };
	const char *probe = getenv("CYBERWORLD_ARENA_LOOKS_PROBE");
	memset(&L, 0, sizeof L);
	L.probe = probe ? atoi(probe) : 0;
	for (int p = 0; p < PLAYERS; ++p) {
		L.look[p] = in[p] && look_valid(in[p]) ? *in[p] : look_plain();
		L.on |= !plain(&L.look[p]);
	}
	L.on |= L.probe == 1;
}

bool arena_looks_on(void) { return L.on; }

static uint32_t pal_at(int palette) { return BN6_LOOK_BATTLE_PALS + (uint32_t)palette * BN6_LOOK_PAL_SIZE; }

static void put16(uint8_t *rom, uint32_t at, const uint16_t in[16]) {
	for (int i = 0; i < 16; ++i) {
		rom[at - BUS + 2u * (unsigned)i] = (uint8_t)in[i];
		rom[at - BUS + 2u * (unsigned)i + 1] = (uint8_t)(in[i] >> 8);
	}
}

/* (each look's colours from BN6's own bytes, the player's ROM's, never a
 * core's copy a look was written into: look_palette_of) */
void arena_looks_rom(uint8_t *rom, size_t size) {
	if (!L.on || pal_at(BN6_LOOK_BATTLE_PAL_COUNT) - BUS > size) return;
	if (L.probe == 1) {
		uint16_t mark[16];
		for (int k = 0; k < BN6_LOOK_PAL_SPARES; ++k) {
			for (int i = 0; i < 16; ++i) mark[i] = (uint16_t)(0x7C00 | k << 5 | i);
			put16(rom, pal_at(spare[k]), mark);
		}
		return;
	}
	for (int p = 0; p < PLAYERS; ++p) {
		uint16_t out[16];
		for (int k = 0; k < AS_KINDS; ++k)
			if (look_palette_of(&L.look[p], sprite_of[k], out)) put16(rom, pal_at(spare[p * AS_KINDS + k]), out);
		if (look_palette_of(&L.look[p], LOOK_SPRITE_FACE, out)) put16(rom, pal_at(spare[FACE_SPARE(p)]), out);
	}
}

/* ---- the hooks (user: the core's side) ---- */

/* the player an object is: its alliance's, the cable's side on every GBA
 * (measured: the second GBA's emotion window shows alliance 1's face) */
static unsigned player_of(uint32_t obj, void *user) {
	(void)user;
	return hook_read8(obj + BN6_T1_ALLIANCE) & 1u;
}

/* MegaMan as BN6 draws him in no form: his own Navi, no Cross, Soul or Beast Out */
static bool unformed(uint32_t obj) {
	uint32_t stats = soul_navi_stats(obj);
	return !hook_read8(stats + BN6_NAVI_SIDE_NAVI) && !hook_read8(stats + BN6_NAVI_FORM);
}

/* (the pick's end, r4 the palette, r5 the object) */
static HookAct pick_end(HookRegs *r, void *user) {
	for (int k = 0; k < AS_KINDS; ++k)
		if (r->r[4] == kinds[k] && unformed(r->r[5])) {
			if (L.probe) ++L.hits[(uintptr_t)user & 1u][0][hook_read8(r->r[5] + BN6_T1_ALLIANCE) & 1u][k];
			r->r[4] = spare[player_of(r->r[5], user) * AS_KINDS + (unsigned)k];
			return HOOK_CONTINUE;
		}
	if (L.probe) ++L.hits[(uintptr_t)user & 1u][0][hook_read8(r->r[5] + BN6_T1_ALLIANCE) & 1u][AS_KINDS];
	return HOOK_CONTINUE;
}

/* (an object drawn as MegaMan by his side's form, r0 the form: BN6 reads
 * BN6_FORM_PALETTES next, which soullook.c answers for a Soul) */
static HookAct form_read2(HookRegs *r, void *user) {
	if (L.probe) ++L.hits[(uintptr_t)user & 1u][1][hook_read8(r->r[5] + BN6_T1_ALLIANCE) & 1u][r->r[0] ? AS_KINDS : 0];
	if (r->r[0] || !unformed(r->r[5])) return HOOK_CONTINUE;
	r->r[0] = spare[player_of(r->r[5], user) * AS_KINDS + AS_PLAIN];
	r->r[12] = BN6_LOOK_FORM_READ2_SET | 1;
	return HOOK_JUMP;
}

/* (the emotion window's face palette, r0 its address, r4 the face, r5 the
 * window: face 0, his own, the window's owner's look) */
static HookAct face_copy(HookRegs *r, void *user) {
	uint32_t owner = hook_read32(r->r[5] + BN6_FACE_OWNER);
	if (L.probe) ++L.hits[(uintptr_t)user & 1u][2][owner ? hook_read8(owner + BN6_T1_ALLIANCE) & 1u : 0][r->r[4] ? AS_KINDS : 0];
	if (r->r[4] != 0 || !owner) return HOOK_CONTINUE;
	r->r[0] = pal_at(spare[FACE_SPARE(player_of(owner, user))]);
	return HOOK_CONTINUE;
}

bool arena_looks_hooks(struct mCore *core, unsigned side) {
	if (!L.on || L.probe == 1) return true;
	HookCore *c = hook_core(core);
	void *user = (void *)(uintptr_t)(side & 1u);
	bool ok = c && hook_on(c, BN6_LOOK_PICK_END, pick_end, user) && hook_on(c, BN6_LOOK_FORM_READ2, form_read2, user) &&
		hook_on(c, BN6_LOOK_FACE_COPY, face_copy, user);
	if (!ok) printf("arena: the looks' hooks not all taken on side %u's core\n", side);
	return ok;
}

uint64_t arena_looks_hash(uint64_t h) {
	if (!L.on) return h;
	for (int p = 0; p < PLAYERS; ++p)
		for (int c = 0; c < LOOK_CHANNELS; ++c) {
			h = (h ^ L.look[p].palette[c]) * UINT64_C(1099511628211);
			h = (h ^ L.look[p].colour[c]) * UINT64_C(1099511628211);
		}
	return h;
}

bool arena_looks_parse(const char *text, Look *out) {
	if (!text || strspn(text, "0123456789abcdefABCDEF") < 12) return false;
	uint8_t b[2 * LOOK_CHANNELS];
	for (int i = 0; i < 2 * LOOK_CHANNELS; ++i) {
		char two[3] = { text[2 * i], text[2 * i + 1], 0 };
		b[i] = (uint8_t)strtoul(two, NULL, 16);
	}
	memcpy(out->palette, b, LOOK_CHANNELS);
	memcpy(out->colour, b + LOOK_CHANNELS, LOOK_CHANNELS);
	return look_valid(out);
}

void arena_looks_bout(unsigned side, const char *given) {
	Look look[PLAYERS] = { look_plain(), look_plain() };
	const char *theirs = given ? strchr(given, '/') : NULL;
	bool ok = given && arena_looks_parse(given, &look[side & 1u]) && (!theirs || arena_looks_parse(theirs + 1, &look[~side & 1u]));
	if (given && !ok) printf("arena: arenalook=%s is not MINE[/THEIRS], 12 hex digits each: BN6's colours\n", given);
	if (ok) arena_looks_set(&look[0], &look[1]);
	else arena_looks_set(NULL, NULL);
	if (L.on) printf("arena: the looks, side 0's %02X%02X%02X%02X%02X%02X, side 1's %02X%02X%02X%02X%02X%02X\n", L.look[0].palette[0],
		L.look[0].palette[1], L.look[0].palette[2], L.look[0].colour[0], L.look[0].colour[1], L.look[0].colour[2], L.look[1].palette[0],
		L.look[1].palette[1], L.look[1].palette[2], L.look[1].colour[0], L.look[1].colour[1], L.look[1].colour[2]);
}

/* ---- the probe ---- */

void arena_looks_probe_frame(struct mCore *core, unsigned side) {
	if (!L.probe || !L.on || side >= PLAYERS) return;
	++L.frames[side];
	for (int k = 0; k < BN6_LOOK_PAL_SPARES; ++k) {
		uint32_t at = pal_at(spare[k]);
		for (unsigned slot = 0; slot < 16; ++slot) {
			bool same = true;
			for (unsigned i = 1; same && i < 16; ++i)
				same = core->rawRead16(core, 0x05000200u + slot * 0x20u + 2 * i, -1) == core->rawRead16(core, at + 2 * i, -1);
			if (same) { ++L.drawn[side][k]; break; }
		}
	}
}

void arena_looks_probe_report(void) {
	if (!L.probe || !L.on) return;
	for (unsigned side = 0; side < PLAYERS; ++side) {
		printf("arena: looks probe %d: core %u, %u frames; frames each spare palette (0x19 0x1A 0x1E 0x1F 0x23 0x24 0x28 0x29) "
			"was in OBJ palette RAM:", L.probe, side, L.frames[side]);
		for (int k = 0; k < BN6_LOOK_PAL_SPARES; ++k) printf(" %u", L.drawn[side][k]);
		printf("\n");
		static const char *const hook_name[3] = { "the pick's end", "the form's second reader", "the face's copy" };
		for (int h = 0; h < 3; ++h)
			for (unsigned a = 0; a < PLAYERS; ++a)
				printf("arena: looks probe: core %u, %s, alliance %u: plain %u, pale %u, Full Synchro %u, other %u\n", side,
					hook_name[h], a, L.hits[side][h][a][0], L.hits[side][h][a][1], L.hits[side][h][a][2], L.hits[side][h][a][3]);
	}
}
