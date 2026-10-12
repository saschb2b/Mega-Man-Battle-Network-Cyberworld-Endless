/* The chips' data as the core's ROM copy holds it (chips.h): one image of
 * the place past EMU_FREE the chips' tables take, made from the player's
 * ROMs, which the run's code reads and the core's copy is given. */
#include "chips.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn5.h"
#include "bytes.h"
#include "rom.h"
#include "text.h"
#include "xchips_record.h"
#include "xchips_text.h"

#define BUS 0x08000000u
#define ENTRY_MAX 160      /* a name's or a description's bytes at most (BN6's longest description is 80) */
#define TEXT_END 0xE6      /* a text archive's entry ends with it */
#define BLANK_RECORD 292   /* one of BN6's blank records (291-300): its art the placeholders', its bytes the records past BN6's */
#define ICON_BYTES 0x80
#define PICTURE_BYTES 0x540
#define PALETTE_BYTES 0x20

static struct {
	const uint8_t *rom6, *rom5;   /* the ROMs it was made from */
	bool made, installed;
	uint8_t *img;                 /* the place's bytes from CHIPS_FREE */
	uint32_t len;
	uint32_t art[XREG_IDS][3];    /* each own id's icon, picture and palette (bus addresses) */
	uint8_t sits[XREG_IDS / 8];
} M;

/* ---- what goes where ---- */

/* An archive's entry `i` in `rom` (ROM offsets): its bytes to its end (its
 * TEXT_END too), ENTRY_MAX at most; its length, 0 where none is there */
static uint32_t entry_at(const uint8_t *rom, uint32_t archive, uint32_t i, const uint8_t **out) {
	if (!rom || archive + 2 * i + 2 > ROM_SIZE) return 0;
	uint32_t p = archive + get16(rom + archive + 2 * i), n = 0;
	while (p + n < ROM_SIZE && n < ENTRY_MAX && rom[p + n] != TEXT_END) ++n;
	*out = rom + p;
	return p + n < ROM_SIZE && n < ENTRY_MAX ? n + 1 : 0;
}

/* the BN5 records' art planned: each own row's three places, BN6's blank
 * where BN5's record has none (a picture and palette at one word), a
 * picture or palette two of BN5's chips share copied once; the art's
 * bytes, 0 without BN5's ROM */
static uint32_t art_plan(const uint32_t blank[3]) {
	uint32_t at = CHIPS_ART;
	static uint32_t from[XREG_IDS * 2], to[XREG_IDS * 2];
	int nshared = 0;
	for (int i = 0; i < xreg_count; ++i) {
		const XChipReg *r = &xreg_rows[i];
		if (!xreg_own(r)) continue;
		uint32_t *a = M.art[r->bn6];
		memcpy(a, blank, sizeof M.art[0]);
		if (!M.rom5) continue;
		const uint8_t *rec = M.rom5 + BN5_CHIP_RECORDS + XREC_SIZE * r->bn5;
		uint32_t icon = get32(rec + BN6_CHIP_ICON_PTR), pic = get32(rec + BN6_CHIP_IMAGE_PTR), pal = get32(rec + BN6_CHIP_PALETTE_PTR);
		if (rom_is_ptr(icon) && rom_off(icon) + ICON_BYTES <= ROM_SIZE) { a[0] = at; at += ICON_BYTES; }
		if (pic == pal || !rom_is_ptr(pic) || !rom_is_ptr(pal) || rom_off(pic) + PICTURE_BYTES > ROM_SIZE || rom_off(pal) + PALETTE_BYTES > ROM_SIZE)
			continue;
		const uint32_t src[2] = { pic, pal }, size[2] = { PICTURE_BYTES, PALETTE_BYTES };
		for (int k = 0; k < 2; ++k) {
			int s = 0;
			while (s < nshared && from[s] != src[k]) ++s;
			if (s == nshared) { from[nshared] = src[k]; to[nshared++] = at; at += size[k]; }
			a[1 + k] = to[s];
		}
	}
	return at - CHIPS_ART;
}

/* ... and copied into the image, the icons in BN6's icons' palette */
static void art_copy(void) {
	if (!M.rom5) return;
	uint8_t map[16];
	xrec_icon_colours(M.rom5 + BN5_CHIP_ICON_PAL, R.data + R.layout->chip_icon_pal, map);
	for (int i = 0; i < xreg_count; ++i) {
		const XChipReg *r = &xreg_rows[i];
		if (!xreg_own(r)) continue;
		const uint8_t *rec = M.rom5 + BN5_CHIP_RECORDS + XREC_SIZE * r->bn5;
		static const uint32_t size[3] = { ICON_BYTES, PICTURE_BYTES, PALETTE_BYTES };
		for (int k = 0; k < 3; ++k) {
			uint32_t to = M.art[r->bn6][k];
			if (to < CHIPS_ART || to - CHIPS_FREE + size[k] > M.len) continue;
			memcpy(M.img + (to - CHIPS_FREE), M.rom5 + rom_off(get32(rec + BN6_CHIP_ICON_PTR + 4 * k)), size[k]);
			if (!k) xrec_icon_recolour(M.img + (to - CHIPS_FREE), map);
		}
	}
}

/* ---- the names and descriptions ---- */

/* the entries of one archive being made: their bytes, back to back */
typedef struct { uint8_t bytes[256 * ENTRY_MAX]; uint32_t at[257]; int n; } Entries;

static void entry_add(Entries *e, const uint8_t *b, uint32_t n) {
	uint32_t from = e->at[e->n];
	if (n > ENTRY_MAX) n = ENTRY_MAX;
	if (n) memcpy(e->bytes + from, b, n);
	/* (ended, whatever came) */
	if (!n || e->bytes[from + n - 1] != TEXT_END) {
		if (n == ENTRY_MAX) --n;
		e->bytes[from + n++] = TEXT_END;
	}
	e->at[++e->n] = from + n;
}

/* the archive of `e` written at `off` of the image, `room` bytes at most:
 * a u16 offset per entry, then the entries; false where it does not fit */
static bool archive_write(const Entries *e, uint32_t off, uint32_t room) {
	uint32_t head = 2u * (uint32_t)e->n, len = head + e->at[e->n];
	if (len > room || off + len > M.len) return false;
	for (int i = 0; i < e->n; ++i) put16(M.img + off + 2u * (uint32_t)i, head + e->at[i]);
	memcpy(M.img + off + head, e->bytes, e->at[e->n]);
	return true;
}

/* an own chip's name (`desc` false) or description in BN6's text codes */
static void own_text(const XChipReg *r, bool desc, Entries *e) {
	uint8_t b[ENTRY_MAX];
	uint32_t n = 0;
	if (M.rom5) {
		uint32_t arch = desc ? (r->bn5 < 256 ? BN5_CHIP_DESCS_LOW : BN5_CHIP_DESCS_HIGH) : (r->bn5 < 256 ? BN5_CHIP_NAMES_LOW : BN5_CHIP_NAMES_HIGH);
		const uint8_t *s;
		uint32_t len = entry_at(M.rom5, arch, r->bn5 & 0xFFu, &s);
		n = (uint32_t)(desc ? xrom_script_bn6(XROM_BN5_COLONEL_US, s, len, b, sizeof b) : xrom_text_bn6(XROM_BN5_COLONEL_US, s, len, b, sizeof b));
	} else if (!desc) {
		char name[16];
		xchips_text_name(r->bn5, name, sizeof name);
		n = (uint32_t)ta_encode(name, b, (int)sizeof b);
	} else {
		/* (a box as BN6's: E8 06 01 01, F1 00 00, three lines, E7 01) */
		static const uint8_t open[7] = { 0xE8, 0x06, 0x01, 0x01, 0xF1, 0x00, 0x00 }, close[2] = { 0xE7, 0x01 };
		memcpy(b, open, sizeof open);
		n = sizeof open;
		for (int l = 0; l < 3; ++l) {
			if (l) b[n++] = 0xE9;
			n += (uint32_t)ta_encode(xchips_text_desc[l], b + n, (int)(sizeof b - n - 4));
		}
		memcpy(b + n, close, sizeof close);
		n += sizeof close;
	}
	entry_add(e, b, n);
}

/* names (`desc` false) or descriptions for ids 256 * half on: an own id's
 * its own, BN6's id's BN6's, an id past BN6's archives' none */
static bool texts_make(bool desc, int half) {
	static Entries e;
	memset(e.at, 0, sizeof e.at);
	e.n = 0;
	uint32_t arch = desc ? R.layout->chip_descs[half] : R.layout->chip_names[half];
	uint32_t count = get16(R.data + arch) / 2u;
	for (int i = 0; i < 256; ++i) {
		const XChipReg *r = xreg_at(256 * half + i);
		const uint8_t *s = NULL;
		uint32_t len = 0;
		if (r) { own_text(r, desc, &e); continue; }
		if ((uint32_t)i < count) len = entry_at(R.data, arch, (uint32_t)i, &s);
		entry_add(&e, s, len);
	}
	uint32_t at = desc ? (half ? CHIPS_DESCS1 : CHIPS_DESCS0) : (half ? CHIPS_NAMES1 : CHIPS_NAMES0);
	return archive_write(&e, at - CHIPS_FREE, desc ? CHIPS_DESCS_ROOM : CHIPS_NAMES_ROOM);
}

/* entry `id` of the made names (`desc` false) or descriptions */
static const uint8_t *text_of(int id, bool desc, uint32_t *n) {
	uint32_t at = (desc ? (id < 256 ? CHIPS_DESCS0 : CHIPS_DESCS1) : (id < 256 ? CHIPS_NAMES0 : CHIPS_NAMES1)) - CHIPS_FREE;
	uint32_t room = desc ? CHIPS_DESCS_ROOM : CHIPS_NAMES_ROOM, p = at + get16(M.img + at + 2u * (uint32_t)(id & 0xFF));
	*n = p < at + room ? at + room - p : 0;
	if (*n > ENTRY_MAX) *n = ENTRY_MAX;
	return M.img + p;
}

/* ---- the records ---- */

/* the folder editor's ABC places of the own records: after BN6's chips of
 * the names before theirs (xrec_abc_place) */
static void abc_make(void) {
	static char names[CHIPS_BN6_RECORDS][24];
	static const char *refs[CHIPS_BN6_RECORDS];
	static uint16_t places[CHIPS_BN6_RECORDS];
	int n = 0;
	for (int id = 1; id < 314; ++id) {
		if (xreg_at(id)) continue;
		chips_name(id, names[n], sizeof names[n]);
		refs[n] = names[n];
		places[n++] = get16(M.img + XREC_SIZE * (uint32_t)id + BN6_CHIP_ALPHA_SORT);
	}
	for (int i = 0; i < xreg_count; ++i) {
		const XChipReg *r = &xreg_rows[i];
		if (!xreg_own(r)) continue;
		char me[24];
		chips_name(r->bn6, me, sizeof me);
		put16(M.img + XREC_SIZE * r->bn6 + BN6_CHIP_ALPHA_SORT, xrec_abc_place(me, refs, places, n));
	}
}

static void records_make(const uint32_t blank[3]) {
	const uint8_t *table = R.data + R.layout->chip_data;
	for (int id = 0; id < CHIPS_IDS; ++id) {
		uint8_t *out = M.img + XREC_SIZE * (uint32_t)id;
		const XChipReg *r = xreg_at(id);
		if (!r) {
			memcpy(out, table + XREC_SIZE * (uint32_t)(id < CHIPS_BN6_RECORDS ? id : BLANK_RECORD), XREC_SIZE);
			/* (BN6's own chip of a name BN5's plays, its code moved: BN5's in what it does) */
			const XChipMoved *mv = M.rom5 && id < CHIPS_BN6_RECORDS ? xchips_moved_of(id) : NULL;
			const XChipReg *named = mv ? xreg_named(id) : NULL;
			if (named) xrec_same_moved(M.rom5 + BN5_CHIP_RECORDS + XREC_SIZE * named->bn5, mv, out);
			continue;
		}
		if (!M.rom5) { xrec_placeholder(r, 0, blank, out); continue; }
		XRecordFrom in = { M.rom5 + BN5_CHIP_RECORDS + XREC_SIZE * r->bn5, r->twin ? table + XREC_SIZE * r->twin : NULL, r, 0, { 0 },
			r->how == XREG_MOVED || r->how == XREG_PA || r->how == XREG_DARK ? xchips_moved_of(id) : NULL };
		memcpy(in.art, M.art[id], sizeof in.art);
		xrec_from_bn5(&in, out);
	}
}

static void sits_make(void) {
	memset(M.sits, 0, sizeof M.sits);
	for (int i = 0; i < xreg_count; ++i) {
		const XChipReg *r = &xreg_rows[i];
		bool plays = (r->how == XREG_RECORD || (r->how == XREG_MOVED && xchips_moved_of(r->bn6))) && M.rom5;
		if (xreg_own(r) && !plays) M.sits[r->bn6 >> 3] |= (uint8_t)(1u << (r->bn6 & 7));
	}
}

/* ---- made at need ---- */

static bool make(void) {
	const uint8_t *table = R.data + R.layout->chip_data + XREC_SIZE * BLANK_RECORD;
	const uint32_t blank[3] = { get32(table + BN6_CHIP_ICON_PTR), get32(table + BN6_CHIP_IMAGE_PTR), get32(table + BN6_CHIP_PALETTE_PTR) };
	uint32_t art = art_plan(blank);
	free(M.img);
	M.len = CHIPS_ART - CHIPS_FREE + art;
	M.img = calloc(M.len, 1);
	if (!M.img) return false;
	records_make(blank);
	bool fit = true;
	for (int k = 0; k < 4; ++k) fit &= texts_make(k >= 2, k & 1);
	if (!fit) fprintf(stderr, "chips: a text archive past its room\n");
	abc_make();
	art_copy();
	sits_make();
	return fit;
}

static bool ready(void) {
	const uint8_t *rom5 = XR[XROM_BN5_COLONEL_US].data;
	if (!R.data || !R.layout || !R.layout->chip_data) return false;
	if (M.made && M.rom6 == R.data && M.rom5 == rom5) return M.img != NULL;
	M.rom6 = R.data;
	M.rom5 = rom5;
	M.made = true;
	if (!make()) { free(M.img); M.img = NULL; }
	return M.img != NULL;
}

/* ---- read ---- */

const uint8_t *chips_record(int id) { return id >= 0 && id < CHIPS_IDS && ready() ? M.img + XREC_SIZE * (uint32_t)id : NULL; }

void chips_name(int id, char *out, size_t n) {
	uint32_t len;
	if (id < 0 || id >= CHIPS_IDS || !ready()) { if (n) out[0] = 0; return; }
	const uint8_t *s = text_of(id, false, &len);
	rom_text_bytes(s, len, out, n);
}

void chips_desc(int id, char *out, size_t n) {
	uint32_t len;
	if (id < 0 || id >= CHIPS_IDS || !ready()) { if (n) out[0] = 0; return; }
	const uint8_t *s = text_of(id, true, &len);
	rom_desc_bytes(s, len, out, n);
}

const uint8_t *chips_bytes(uint32_t bus, uint32_t n) {
	if (bus >= BUS && bus - BUS + n <= ROM_SIZE && n <= ROM_SIZE) return R.data ? R.data + (bus - BUS) : NULL;
	if (bus >= CHIPS_FREE && ready() && bus - CHIPS_FREE + n <= M.len && n <= M.len) return M.img + (bus - CHIPS_FREE);
	return NULL;
}

const uint8_t *chips_offset_bytes(uint32_t off, uint32_t n) { return off < 0x01000000u ? chips_bytes(BUS + off, n) : NULL; }

bool chips_bn5(int id) { return xreg_at(id) != NULL; }

bool chips_sits_out(int id) { return id > 0 && id < CHIPS_IDS && ready() && (M.sits[id >> 3] & (1u << (id & 7))); }

const uint8_t *chips_sit_bits(void) { return ready() ? M.sits : NULL; }

bool chips_from_bn5(void) { return ready() && M.rom5; }

const uint8_t *chips_image(uint32_t *len) {
	*len = ready() ? M.len : 0;
	return *len ? M.img : NULL;
}

uint32_t chips_table_bus(void) { return M.installed ? CHIPS_RECORDS : BUS + (R.layout ? R.layout->chip_data : 0); }

void chips_installed(bool on) { M.installed = on; }

void chips_forget(void) { M.made = false; }
