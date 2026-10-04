/* The Link Navi obstacles on a layer (issue #42; docs/ROM_DATA.md, Link
 * Navi obstacles).
 *
 * BN6 runs them as map objects of handler 3, one 16-byte record per id: a
 * present flag (the object stands while it is set), the walls' flag and
 * the check's. Cleared, the object plays its open animation and sound,
 * switches its walls and its check off and frees itself; entering the map
 * again with it clear, it never stands. Its talk is a check: A at the
 * trigger cells before it runs a script of the map's own text archive.
 * BN6's text opens one only for the right Link Navi in the PET; a run has
 * MegaMan alone, so the talk is the engine's, and the run's Crosses clear
 * the obstacles Gregar's Link Navis clear. */
#include "blockers.h"

#include <stdio.h>
#include <string.h>

#include "bytes.h"
#include "debug.h"
#include "emu.h"
#include "flags.h"
#include "lz.h"
#include "mapslot.h"
#include "net.h"
#include "netmap.h"
#include "pacing.h"
#include "rom.h"
#include "run.h"
#include "layer_objs.h"
#include "scripts.h"
#include "shop.h"
#include "xnavi.h"

#define H3_LITERAL   0x080A66D4u   /* handler 3's records' literal (Gregar) */
#define H3_TABLE     0x080A6144u   /* ... BN6's 86 (ids 0x00-0x55) */
#define H3_ORIGINALS 86
#define BLOCKERS_AT  (EMU_FREE + 0x0700)   /* BN6's records, then the engine's (docs/EMULATION.md) */
#define BLOCK_ID0    0x56          /* the engine's: + variant x 8 + slot */
#define BLOCK_SLOTS  8
#define NET_CHECKS   0x08034728u   /* per internet group from 0x80, per map: the 16 checks' scripts */
#define NET_ARCHIVES 0x08040798u   /* per internet group from 0x80, per map: its LZ77 text archive */

/* The variants: the cube looking each of four ways (issue #45), then the
 * five obstacles, then another game's looks for them (docs/MULTIROM.md):
 * BN5's Security Cube, its wall of dark flames across a walkway along
 * world Y and along X, and its dark hole, a prop that never opens; bytes
 * 6-15 of their records as BN6's own (sprite list, sprite, idle and open
 * animations, a sprite field, palette, priority, a spare, sound 0x74) */
enum { V_CUBE, V_WATER = 4, V_TREE, V_FLAMES, V_CYCLONE, V_CLOUD, V_XCUBE, V_XWALL, V_XHOLE = V_XWALL + 2, VARIANTS };
#define XPROP_SLOT 7   /* the slot of another game's prop: its present flag set while it stands, its walls and check named by no cell */
static const uint8_t templates[V_XCUBE][10] = {
	{ 0x1C, 0x03, 0, 0, 0x38, 0, 2, 0xFF, 0x74, 0 },
	{ 0x1C, 0x03, 1, 1, 0x38, 0, 2, 0xFF, 0x74, 0 },
	{ 0x1C, 0x03, 2, 2, 0x38, 0, 2, 0xFF, 0x74, 0 },
	{ 0x1C, 0x03, 3, 3, 0x38, 0, 2, 0xFF, 0x74, 0 },
	{ 0x0C, 0x25, 1, 2, 0, 0, 3, 0xFF, 0x74, 0 },    /* the cyber geyser (BN6's id 0x36) */
	{ 0x1C, 0x1F, 0, 0, 0, 0, 2, 0xFF, 0x74, 0 },    /* the cybertree (0x38) */
	{ 0x1C, 0x0E, 0, 1, 0, 0, 2, 0xFF, 0x74, 0 },    /* the flames (0x3C) */
	{ 0x1C, 0x4B, 0, 1, 0, 0, 2, 0xFF, 0x74, 0 },    /* the cyclone (0x37) */
	{ 0x1C, 0x20, 0, 0, 0, 0, 2, 0xFF, 0x74, 0 },    /* the cloud (0x3B) */
};
/* Variant v's record bytes 6-15: another game's look with its sprite's
 * number in BN6 (xnavi.c), or BN6's own look where it was not copied
 * (BN5's Security Cube standing, then opening; its wall of dark flames
 * standing across a walkway along world Y or X, then burning out; its
 * dark hole turning, behind the floor's front; their sound BN6's cubes'
 * and obstacles') */
static void template_of(int v, uint8_t out[10]) {
	static const struct { int obj, bn6; uint8_t rec[10]; } xlook[] = {
		{ XOBJ_CUBE, V_CUBE, { 0x1C, 0, 0, 1, 0, 0, 2, 0xFF, 0x74, 0 } },
		{ XOBJ_DARK_WALL, V_FLAMES, { 0x1C, 0, 0, 2, 0, 0, 2, 0xFF, 0x74, 0 } },
		{ XOBJ_DARK_WALL, V_FLAMES, { 0x1C, 0, 1, 3, 0, 0, 2, 0xFF, 0x74, 0 } },
		{ XOBJ_DARK_HOLE, V_FLAMES, { 0x1C, 0, 0, 0, 0, 0, 3, 0xFF, 0x74, 0 } },
	};
	int slot = v < V_XCUBE ? -1 : xnavi_object(xlook[v - V_XCUBE].obj);
	if (slot < 0) { memcpy(out, templates[v < V_XCUBE ? v : xlook[v - V_XCUBE].bn6], 10); return; }
	memcpy(out, xlook[v - V_XCUBE].rec, 10);
	out[1] = (uint8_t)slot;
}

/* A block's variant: the obstacle's, or the cube looking along its
 * walkway (animation 0 for +X, 1 for -Y, 2 for -X, 3 for +Y), or the look
 * another game's area lends it: BN5's Security Cube for a P-Code's cube
 * or a toll's, its wall of dark flames for the pillar of flames. */
static int variant_of(const NetBlock *b) {
	static const int obstacle[BLOCK_KINDS] = { V_WATER, V_TREE, V_FLAMES, V_CYCLONE, V_CLOUD };
	int dir, edge, side;
	netmap_block_edges(b, &dir, &edge, &side);
	if (b->kind == BLOCK_FLAMES && (layer_objs_xlooks & XLOOK_DARK_WALL) && xnavi_object(XOBJ_DARK_WALL) >= 0) return V_XWALL + !(dir & 1);
	if (b->kind < BLOCK_KINDS) return obstacle[b->kind];
	if ((b->kind == BLOCK_PCODE || b->kind == BLOCK_TOLL) && (layer_objs_xlooks & XLOOK_CUBE) && xnavi_object(XOBJ_CUBE) >= 0) return V_XCUBE;
	static const int look[4] = { 0, 3, 2, 1 };
	return V_CUBE + look[dir];
}

void blockers_install(void) {
	/* the records once a core, fixed per slot: a live object reads its
	 * record every frame, and the map being left runs on through the exit's
	 * warp, so a record rewritten for the next layer could open one there */
	static bool done;
	if (!done) {
		done = true;
		uint8_t rec[(H3_ORIGINALS + VARIANTS * BLOCK_SLOTS) * 16];
		for (int i = 0; i < H3_ORIGINALS * 16; ++i) rec[i] = emu_read8(H3_TABLE + (uint32_t)i);
		for (int v = 0; v < VARIANTS; ++v)
			for (int k = 0; k < BLOCK_SLOTS; ++k) {
				uint8_t *r = rec + (H3_ORIGINALS + v * BLOCK_SLOTS + k) * 16;
				put16(r, (uint32_t)(BLOCK_PRESENT_FLAG + k));
				put16(r + 2, (uint32_t)(0x1640 + k));
				put16(r + 4, (uint32_t)(0x16C0 + k));
				template_of(v, r + 6);
			}
		emu_write(BLOCKERS_AT, rec, sizeof rec);
		emu_write32(H3_LITERAL, BLOCKERS_AT);
	}
	for (int k = 0; k < layer.nblocks; ++k) flag_set(BLOCK_PRESENT_FLAG + k);
}

void blocker_sprite(int b, int *category, int *index) {
	uint8_t t[10];
	template_of(variant_of(&layer.block[b]), t);
	*category = t[0] / 4;
	*index = t[1];
}

/* Where BN6 sets each obstacle in a walkway's mouth (measured on its
 * originals and on a comp's walkways, the four ways): (along the walkway
 * from its edge, across from its lower side), for the geyser, the flames,
 * the tree, the cloud and the cyclone */
static void spot(int dir, int kind, int edge, int side, int *x, int *y) {
	static const int8_t along[2][BLOCK_KINDS] = { { 18, 22, 20, 22, 22 }, { -28, -24, -24, -24, -24 } };
	static const int8_t across[BLOCK_KINDS] = { 20, 16, 16, 18, 18 };
	/* (the cube: +X (8, 4), -X (-10, 32), -Y (22, -8), +Y (-8, 8)) */
	static const int8_t cube[4][2] = { { 8, 4 }, { -8, 8 }, { -10, 32 }, { 22, -8 } };
	if (kind >= BLOCK_KINDS) {
		bool along_x = !(dir & 1);
		*x = along_x ? edge + cube[dir][0] : side + cube[dir][0];
		*y = along_x ? side + cube[dir][1] : edge + cube[dir][1];
		return;
	}
	if (dir == 0 || dir == 2) {
		*x = edge + along[dir == 2][kind];
		*y = side + across[kind];
	} else {
		*x = side + (dir == 1 ? 8 : 6);
		*y = edge + (dir == 1 ? 22 : -20);
	}
}

int blockers_xprop(uint8_t *recs, int n, int max, int x, int y) {
	if (n >= max || xnavi_object(XOBJ_DARK_HOLE) < 0) return n;
	flag_set(BLOCK_PRESENT_FLAG + XPROP_SLOT);
	uint8_t *r = recs + n++ * 20;
	memset(r, 0, 20);
	r[0] = 5;
	r[1] = 3;
	put32(r + 4, (uint32_t)(x * 65536));
	put32(r + 8, (uint32_t)(y * 65536));
	put32(r + 16, (uint32_t)(BLOCK_ID0 + V_XHOLE * BLOCK_SLOTS + XPROP_SLOT));
	return n;
}

int blockers_objects(uint8_t *recs, int n, int max) {
	for (int k = 0; k < layer.nblocks && n < max; ++k) {
		const NetBlock *b = &layer.block[k];
		int dir, edge, side, x, y;
		netmap_block_edges(b, &dir, &edge, &side);
		/* (BN5's Security Cube stands in the walkway's middle, as BN6's
		 * obstacles do: its sprite is drawn about its spot as theirs) */
		spot(dir, variant_of(b) == V_XCUBE ? BLOCK_FLAMES : b->kind, edge, side, &x, &y);
		if (emu_debug_on()) fprintf(stderr, "blockers: lock %d, kind %d, variant %d, at world %d,%d facing %d\n", k, b->kind, variant_of(b), x, y, dir);
		uint8_t *r = recs + n++ * 20;
		memset(r, 0, 20);
		r[0] = 5;
		r[1] = 3;
		put32(r + 4, (uint32_t)(x * 65536));
		put32(r + 8, (uint32_t)(y * 65536));
		put32(r + 16, (uint32_t)(BLOCK_ID0 + variant_of(b) * BLOCK_SLOTS + k));
	}
	return n;
}

/* Each kind: what MegaMan calls it, and per Link Navi who clears it (Gregar's
 * pairs) his name, his mugshot, his sound and what he does */
static const char *const block_names[BLOCK_KINDS] = { "A geyser of cyberwater", "A cybertree", "A pillar of flames", "A cyclone", "A cloud" };
/* ... or, in another game's look, what it is there */
static const char *block_name(const NetBlock *b) { int v = variant_of(b); return v == V_XWALL || v == V_XWALL + 1 ? "A wall of dark flames" : block_names[b->kind]; }
static const struct { int navi, mugshot, sound; const char *name, *deed; } helpers[] = {
	{ 1, 0x47, 0xF7, "HeatMan", "burn" },
	{ 2, 0x49, 0xC6, "ElecMan", "short out" },
	{ 3, 0x4B, 0xFA, "SlashMan", "cut" },
	{ 4, 0x50, 0xC7, "EraseMan", "erase" },
	{ 5, 0x4F, 0xE4, "ChargeMan", "ram" },
};

const char *blockers_pcode(void) {
	static char code[8];
	snprintf(code, sizeof code, "%04u", (unsigned)(run.layer_seed >> 7) % 9000u + 1000u);
	return code;
}

static int talk(TextArchive *t, int k) {
	const NetBlock *b = &layer.block[k];
	/* (a security cube's: a P-Code told on the layer, or a toll, 200
	 * zenny in act 1 and 100 more an act: half the Net Dealer's answer) */
	if (b->kind == BLOCK_PCODE) return ta_cube_pcode(t, BLOCK_PRESENT_FLAG + k, LAYER_PCODE_FLAG, blockers_pcode());
	if (b->kind == BLOCK_TOLL) return ta_cube_toll(t, BLOCK_PRESENT_FLAG + k, 200 + 100 * (pacing_act(run.depth) + 7 * pacing_loop(run.depth)));
	/* (the Undernet's doors, issue #47) */
	if (b->kind == BLOCK_SKULL) return ta_cube_skull(t, BLOCK_PRESENT_FLAG + k, ITEM_WWW_ID);
	if (b->kind == BLOCK_NUMBER) return ta_cube_number(t, BLOCK_PRESENT_FLAG + k, LAYER_NUMBER_SEALED_FLAG, layer.braziers, run.layer_seed >> 11);
	unsigned can = block_openers(b->kind), held = layer_crosses(run.depth) & can;
	int i = ta_script(t);
	bool first = true;
	char s[240];
	const char *who[2] = { NULL, NULL };
	for (int h = 0, n = 0; h < 5; ++h) if (can >> helpers[h].navi & 1 && n < 2) who[n++] = helpers[h].name;
	if (!held) {
		/* (the hint for the next run's Cross, said as it is) */
		snprintf(s, sizeof s, "%s blocks the way, Lan.|%s's or %s's Cross data could clear it... but we don't carry either.", block_name(b),
			who[0], who[1]);
		ta_pages(t, s, FACE_MEGAMAN, &first);
		ta_end(t);
		return i;
	}
	int h = 0;
	while (!(held >> helpers[h].navi & 1)) ++h;
	/* (HeatMan takes a fire in, where he burns a tree: "burn it" before a
	 * pillar of flames read as a fire set on fire, session 64) */
	const char *deed = helpers[h].navi == 1 && b->kind == BLOCK_FLAMES ? "swallow" : helpers[h].deed;
	snprintf(s, sizeof s, "%s blocks the way, Lan!|We carry %s's Cross data. Let's ask him to %s it!", block_name(b), helpers[h].name, deed);
	ta_pages(t, s, FACE_MEGAMAN, &first);
	ta_page(t, helpers[h].mugshot, "Leave it to me!", false);
	/* (his sound, a moment, and the present flag cleared: the obstacle
	 * plays its opening while the box is still open) */
	int sound = h == 2 && b->kind == BLOCK_CYCLONE ? 0x164 : helpers[h].sound;
	uint8_t act[] = { 0xFD, 0x00, (uint8_t)sound, (uint8_t)(sound >> 8), 0xEE, 0x00, 30, 0,
		0xEA, 0x01, (uint8_t)(BLOCK_PRESENT_FLAG + k), (uint8_t)((BLOCK_PRESENT_FLAG + k) >> 8) };
	ta_bytes(t, act, sizeof act);
	ta_wait(t);
	ta_end(t);
	return i;
}

void blockers_talks(TextArchive *t, int scripts[2]) {
	for (int k = 0; k < 2; ++k) scripts[k] = k < layer.nblocks ? talk(t, k) : -1;
}

static bool checks_write(int group, int number, const TextArchive *t, const int scripts[2]) {
	uint8_t table[16];
	memset(table, 0xFF, sizeof table);
	for (int k = 0; k < layer.nblocks && k < 2; ++k) table[k] = (uint8_t)scripts[k];
	/* the archive the map decompresses as it is entered (to 0x02033400,
	 * read from +4, after its size) */
	static uint8_t raw[0x1600], packed[0x1600 + 0x1600 / 8 + 16];
	int len = ta_build(t, raw + 4);
	if (len + 4 > 0x15FC) return false;
	raw[0] = 0;
	raw[1] = (uint8_t)(len + 4);
	raw[2] = (uint8_t)((len + 4) >> 8);
	raw[3] = 0;
	size_t n = lz_literal(raw, (size_t)len + 4, packed);
	uint32_t at = mapslot_alloc(packed, (int)n), tab = mapslot_alloc(table, sizeof table);
	uint32_t checks = emu_read32(NET_CHECKS + (uint32_t)(group - 0x80) * 4), archives = emu_read32(emu_read32(NET_ARCHIVES) + (uint32_t)(group - 0x80) * 4);
	if (!at || !tab || checks < 0x08000000u || archives < 0x08000000u) return false;
	emu_write32(checks + (uint32_t)number * 4, tab);
	emu_write32(archives + (uint32_t)number * 4, at);
	return true;
}

/* The map's own checks and text archive back, as the ROM has them: an
 * earlier layer's on this map lie in a half of the layer space that a
 * later layer overwrites, and the game decompresses the archive as it
 * enters the map (garbage there ran the decompression over all of EWRAM:
 * a black screen at the exit pad, session 64). */
static void checks_restore(int group, int number) {
	if (group < 0x80) return;
	uint32_t g = (uint32_t)(group - 0x80) * 4, n = (uint32_t)number * 4;
	uint32_t tables[2] = { emu_read32(NET_CHECKS + g), emu_read32(emu_read32(NET_ARCHIVES) + g) };
	for (int i = 0; i < 2; ++i)
		if (rom_is_ptr(tables[i]) && rom_is_ptr(tables[i] + n + 3))
			emu_write32(tables[i] + n, get32(R.data + (tables[i] + n - 0x08000000u)));
}

void blockers_checks(int group, int number, const TextArchive *t, const int scripts[2]) {
	if (!layer.nblocks) { checks_restore(group, number); return; }
	/* (no talk, no obstacle: one that could never open would lock its
	 * pocket for good) */
	if (group < 0x80 || !checks_write(group, number, t, scripts)) {
		checks_restore(group, number);
		for (int k = 0; k < layer.nblocks; ++k) flag_clear(BLOCK_PRESENT_FLAG + k);
	}
}
