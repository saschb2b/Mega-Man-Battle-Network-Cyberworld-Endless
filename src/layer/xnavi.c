/* Another game's Navis: a sprite is a 4-byte header, then a table of
 * animations, each a run of 20-byte frames (the last's +18 bit 7 set)
 * whose offsets from the table name its tiles and palettes (a length
 * first), its sub-animation and its object table, whose offsets name the
 * object lists (5 bytes an object, 0xFF 0xFF ends one). Offsets only: a
 * sprite copied whole reads the same anywhere, as a mugshot does, which is
 * a sprite of sprite list 8. */
#include "xnavi.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bytes.h"
#include "debug.h"
#include "emu.h"
#include "rom.h"

#define XNAVI_AT   (EMU_FREE + 0x260000)   /* (docs/EMULATION.md) */
#define XNAVI_END  (EMU_FREE + 0x2A0000)
#define BUS        0x08000000u
#define LIST_NAVIS 6                       /* sprite lists: the Navis and Mr. Progs */
#define LIST_FACES 8                       /* the mugshots */
/* (Gregar's list-6 numbers for Falzar's Navis: a placeholder sprite and
 * the black mugshot, docs/ROM_DATA.md; no guardian takes them) */
static const uint8_t free_slots[] = { 72, 74, 76, 77, 78 };
#define EMPTY_SPRITE 48                    /* one of them, to compare with */
#define EMPTY_FACE   0x07

uint32_t xnavi_sprite_len(const uint8_t *hdr, uint32_t n) {
	if (n < 8) return 0;
	const uint8_t *b = hdr + 4;
	uint32_t m = n - 4, anims = get32(b) / 4, end = 4 * anims;
	if (!anims || anims > 256 || end > m) return 0;
	for (uint32_t a = 0; a < anims; ++a) {
		uint32_t f = get32(b + 4 * a);
		for (int k = 0; k < 256; ++k, f += 20) {
			if (f + 20 > m) return 0;
			if (f + 20 > end) end = f + 20;
			for (int part = 0; part < 2; ++part) {   /* tiles, palettes: a length, then it */
				uint32_t at = get32(b + f + 4 * (uint32_t)part);
				if (at + 4 > m || get32(b + at) > m - at - 4) return 0;
				if (at + 4 + get32(b + at) > end) end = at + 4 + get32(b + at);
			}
			/* the object table: its first offset says how many lists */
			uint32_t tab = get32(b + f + 12);
			if (tab + 4 > m || get32(b + tab) > m - tab) return 0;
			for (uint32_t j = 0; j < get32(b + tab) / 4; ++j) {
				uint32_t o = tab + get32(b + tab + 4 * j);
				while (o + 2 <= m && !(b[o] == 0xFF && b[o + 1] == 0xFF)) o += 5;
				if (o + 2 > m) return 0;
				if (o + 2 > end) end = o + 2;
			}
			if (get32(b + f + 8) + 8 > end) end = get32(b + f + 8) + 8;   /* (the sub-animation: its offset, then its first entry) */
			if (b[f + 18] & 0x80) break;
		}
	}
	return end + 4;
}

static struct {
	bool ready;
	uint32_t next;
	int n;
	struct { int rom, navi, slot; uint32_t sprite; } navi[sizeof free_slots];
} X;

static uint32_t list_entry(uint32_t lists, int list, int i) { return get32(R.data + lists + (uint32_t)list * 4) - BUS + (uint32_t)i * 4; }

/* A sprite of the other ROM's list `list`, copied: its address, 0 where it
 * cannot be (a compressed one, which a Navi's never is, among them). */
static uint32_t copy_sprite(const uint8_t *xr, uint32_t lists, int list, int i) {
	uint32_t at = get32(xr + lists + (uint32_t)list * 4) - BUS + (uint32_t)i * 4;
	if (at + 4 > ROM_SIZE) return 0;
	uint32_t p = get32(xr + at);
	if (p < BUS || p - BUS >= ROM_SIZE) return 0;
	uint32_t len = xnavi_sprite_len(xr + (p - BUS), ROM_SIZE - (p - BUS));
	if (!len || X.next + len > XNAVI_END) return 0;
	uint32_t to = X.next;
	emu_write(to, xr + (p - BUS), len);
	X.next = (to + len + 3) & ~3u;
	return to;
}

/* Navi `navi` of ROM `xrom` copied in and listed at the next free slot */
static void copy_in(int xrom, int navi) {
	const uint8_t *xr = XR[xrom].data;
	uint32_t lists = XR[xrom].layout->sprite_lists, own = R.layout->sprite_lists;
	uint32_t empty = get32(R.data + list_entry(own, LIST_NAVIS, EMPTY_SPRITE)), black = get32(R.data + list_entry(own, LIST_FACES, EMPTY_FACE));
	for (unsigned k = 0; k < sizeof free_slots; ++k) {
		int s = free_slots[k];
		bool taken = false;
		for (int i = 0; i < X.n; ++i) taken |= X.navi[i].slot == s;
		/* (not taken, and still a placeholder in the player's ROM) */
		if (taken || get32(R.data + list_entry(own, LIST_NAVIS, s)) != empty || get32(R.data + list_entry(own, LIST_FACES, s)) != black) continue;
		uint32_t sprite = copy_sprite(xr, lists, LIST_NAVIS, navi), face = sprite ? copy_sprite(xr, lists, LIST_FACES, navi) : 0;
		if (!face) return;
		emu_write32(BUS + list_entry(own, LIST_NAVIS, s), sprite);
		emu_write32(BUS + list_entry(own, LIST_FACES, s), face);
		X.navi[X.n++] = (__typeof__(X.navi[0])){ xrom, navi, s, sprite };
		if (emu_debug_on()) fprintf(stderr, "xnavi: navi %d of %s at %d, %u bytes so far\n", navi, XR[xrom].layout->name, s, X.next - XNAVI_AT);
		return;
	}
}

/* ---- Another game's map objects: sprites of its list 7 ---- */

#define LIST_OBJECTS 7                     /* sprite lists: the overworld's objects */
#define OBJECT_PLACEHOLDER 0x084DC040u     /* (Gregar's list-7 numbers that point at it are free: no layer lists them, docs/ROM_DATA.md) */

/* Each: its game, its list-7 number there (bit 31 of the pointer: LZ77,
 * copied decompressed), the free Gregar number it is listed at, and
 * whether its palette is turned purple */
static const struct { int xrom; uint8_t index, slot; bool purple; } objects[XOBJ_COUNT] = {
	[XOBJ_DARK_FLAME] = { XROM_BN5_COLONEL_US, 0x68, 0xA0, true },   /* Nebula Area 6's flames of darkness (compressed) */
	[XOBJ_CUBE] = { XROM_BN5_COLONEL_US, 0x01, 0x92, false },        /* ACDC Area's Security Cubes */
	[XOBJ_DARK_WALL] = { XROM_BN5_COLONEL_US, 0x5D, 0x8A, false },   /* Nebula Area 6's and End Area 5's walls of dark flames (compressed) */
	[XOBJ_DARK_HOLE] = { XROM_BN5_COLONEL_US, 0x69, 0x87, false },   /* the dark holes of Nebula Areas 1, 3, 4 and 6 and Undernet 2 */
};

static struct { bool ready; uint32_t sprite[XOBJ_COUNT]; } O;

/* a palette colour (BGR555) turned purple: blue lends red, green fades
 * but in the whites (the flame was blue, the owner's "dark purple ghost
 * flame") */
static uint16_t purple(uint16_t c) {
	int r = c & 31, g = c >> 5 & 31, b = c >> 10 & 31, m = r < g ? (r < b ? r : b) : (g < b ? g : b);
	int r2 = (b * 7 + r * 3) / 10;
	r = r2 > r ? r2 : r;
	g = m + (g - m) * 35 / 100;
	return (uint16_t)(r | g << 5 | b << 10 | (c & 0x8000));
}

/* every palette its frames name, turned purple in place, each once (frames
 * share them; `b` the sprite past its 4-byte header, `m` its bytes) */
static void tint(uint8_t *b, uint32_t m) {
	uint32_t anims = get32(b) / 4, done[32];
	int ndone = 0;
	for (uint32_t a = 0; a < anims && 4 * a + 4 <= m; ++a)
		for (uint32_t f = get32(b + 4 * a), k = 0; k < 256 && f + 20 <= m; ++k, f += 20) {
			uint32_t at = get32(b + f + 4), len = at + 4 <= m ? get32(b + at) : 0;
			bool again = false;
			for (int j = 0; j < ndone; ++j) again |= done[j] == at;
			if (!again && ndone < 32) done[ndone++] = at;
			for (uint32_t i = 0; !again && at + 4 + len <= m && i + 2 <= len && i < 512; i += 2) {
				uint32_t o = at + 4 + i;
				uint16_t c = (uint16_t)(b[o] | b[o + 1] << 8);
				c = purple(c);
				b[o] = (uint8_t)c; b[o + 1] = (uint8_t)(c >> 8);
			}
			if (b[f + 18] & 0x80) break;
		}
}

/* Object k's sprite as it is copied (its 4-byte header first, *len bytes:
 * decompressed, a size word before it, where its pointer's bit 31 says
 * LZ77; the walk only to check it), malloc'd; NULL where it cannot be. */
static uint8_t *object_bytes(int k, uint32_t *len) {
	const uint8_t *xr = XR[objects[k].xrom].data;
	uint32_t at = get32(xr + XR[objects[k].xrom].layout->sprite_lists + LIST_OBJECTS * 4) - BUS + objects[k].index * 4u;
	uint32_t p = at + 4 <= ROM_SIZE ? get32(xr + at) : 0, off = (p & 0x7FFFFFFFu) - BUS;
	*len = 0;
	if ((p & 0x7FFFFFFFu) < BUS || off >= ROM_SIZE) return NULL;
	if (!(p & 0x80000000u)) {
		uint32_t n = xnavi_sprite_len(xr + off, ROM_SIZE - off);
		uint8_t *d = n ? malloc(n) : NULL;
		if (d) { memcpy(d, xr + off, n); *len = n; }
		return d;
	}
	size_t n = 0;
	uint8_t *d = lz77_decompress(xr + off, ROM_SIZE - off, &n);
	if (!d || n <= 8 || !xnavi_sprite_len(d + 4, (uint32_t)n - 4)) { free(d); return NULL; }
	memmove(d, d + 4, n - 4);
	*len = (uint32_t)n - 4;
	return d;
}

/* Object k copied in after what is there and listed at its number (one
 * that still points at the placeholder in the player's ROM) */
static void object_in(int k, uint32_t own) {
	int s = objects[k].slot;
	uint32_t len;
	if (get32(R.data + list_entry(own, LIST_OBJECTS, s)) != OBJECT_PLACEHOLDER) return;
	uint8_t *d = object_bytes(k, &len);
	if (d && X.next + len <= XNAVI_END) {
		if (objects[k].purple) tint(d + 4, len - 4);
		O.sprite[k] = X.next;
		emu_write(O.sprite[k], d, len);
		X.next = (O.sprite[k] + len + 3) & ~3u;
		emu_write32(BUS + list_entry(own, LIST_OBJECTS, s), O.sprite[k]);
		if (emu_debug_on())
			fprintf(stderr, "xnavi: list 7 %#x of %s at list 7 %d, %u bytes\n", objects[k].index, XR[objects[k].xrom].layout->name, s, len);
	}
	free(d);
}

int xnavi_object(int which) {
	if (which < 0 || which >= XOBJ_COUNT || !R.data || !R.layout || !R.layout->sprite_lists) return -1;
	uint32_t own = R.layout->sprite_lists;
	/* (the core's ROM copy made anew: copied again) */
	for (int k = 0; k < XOBJ_COUNT; ++k)
		if (O.sprite[k] && emu_read32(BUS + list_entry(own, LIST_OBJECTS, objects[k].slot)) != O.sprite[k]) { memset(&O, 0, sizeof O); break; }
	if (!O.ready) {
		O.ready = true;
		xnavi_slot(XROM_BN5_COLONEL_US, 0, 0);   /* (the Navis first: their place stays the same in every session) */
		for (int k = 0; k < XOBJ_COUNT; ++k)
			if (XR[objects[k].xrom].data && XR[objects[k].xrom].layout->sprite_lists) object_in(k, own);
	}
	return O.sprite[which] ? objects[which].slot : -1;
}

static int slot_of(int xrom, int navi) {
	for (int i = 0; i < X.n; ++i)
		if (X.navi[i].rom == xrom && X.navi[i].navi == navi) return X.navi[i].slot;
	return -1;
}

int xnavi_slot(int xrom, int navi, int fallback) {
	if (!R.data || !R.layout || !R.layout->sprite_lists) return fallback;
	/* (the core's ROM copy made anew: the Navis went with the old) */
	if (X.n && emu_read32(BUS + list_entry(R.layout->sprite_lists, LIST_NAVIS, X.navi[0].slot)) != X.navi[0].sprite) memset(&X, 0, sizeof X);
	if (!X.ready) {
		X.ready = true;
		X.next = XNAVI_AT;
		for (int k = 0; k < XAREAS_MAX; ++k) {
			const NetAreaDef *x = net_area_def(NET_AREAS + k);
			int xi = x ? x->xrom - 1 : -1;
			if (xi >= 0 && xi < XROM_COUNT && x->xnavi && XR[xi].data && XR[xi].layout->sprite_lists && slot_of(xi, x->xnavi) < 0) copy_in(xi, x->xnavi);
		}
	}
	int slot = slot_of(xrom, navi);
	return slot < 0 ? fallback : slot;
}
