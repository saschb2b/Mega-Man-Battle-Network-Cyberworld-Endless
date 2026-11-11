/* The chips' data as the core's ROM copy holds it (chips.h): one image of
 * the place past EMU_FREE the chips' tables take, made from the player's
 * ROMs, which the run's code reads and the core's copy is given. */
#include "chips.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn4.h"
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
	int alias, alias_from;        /* chips_alias's */
	const uint8_t *rom6, *rom5, *rom4;   /* the ROMs it was made from */
	bool made, installed;
	uint8_t *img;                 /* the place's bytes from CHIPS_FREE */
	uint32_t len;
	uint32_t art[XREG_IDS][3];    /* each own id's icon, picture and palette (bus addresses) */
	uint8_t sits[XREG_IDS / 8];
} M;

/* ---- what goes where ---- */

/* An archive's entry `i` in `rom` (ROM offsets): its bytes to its end (its
 * TEXT_END too), ENTRY_MAX at most; its length, 0 where none is there */
static uint32_t entry_to(const uint8_t *rom, uint32_t archive, uint32_t i, uint8_t end, const uint8_t **out) {
	if (!rom || archive + 2 * i + 2 > ROM_SIZE) return 0;
	uint32_t p = archive + get16(rom + archive + 2 * i), n = 0;
	while (p + n < ROM_SIZE && n < ENTRY_MAX && rom[p + n] != end) ++n;
	*out = rom + p;
	return p + n < ROM_SIZE && n < ENTRY_MAX ? n + 1 : 0;
}

static uint32_t entry_at(const uint8_t *rom, uint32_t archive, uint32_t i, const uint8_t **out) { return entry_to(rom, archive, i, TEXT_END, out); }

/* A row whose art is BN5's: an own row's, and (after them all, so that
 * theirs keep their places) BN6's own chip of a name whose record BN5's
 * code plays where its ROM is read (SAME_MOVED: DarkPlus, which BN6's US
 * version left a blank picture and an icon reading its id) */
static bool art_row(const XChipReg *r, int pass) {
	return pass ? r->how == XREG_SAME && M.rom5 && xchips_moved_of(r->bn6) != NULL : xreg_own(r);
}

/* the BN5 records' art planned: each such row's three places, BN6's blank
 * where BN5's record has none (a picture and palette at one word), a
 * picture or palette two of BN5's chips share copied once; the art's
 * bytes, 0 without BN5's ROM */
static const uint8_t *bn4_record(int bn4) { return M.rom4 + (BN4_CHIP_RECORDS - BUS) + XREC_SIZE * (uint32_t)bn4; }

/* the shared pictures and palettes planned so far: where each is from (a game's address, BN4's told from BN5's of the
 * same number by bit 31) and its place */
static struct { uint32_t from[XREG_IDS * 4], to[XREG_IDS * 4]; int n; } shared;

/* one record's art planned from `at` on: its icon, then its picture and palette where it has the pair, one two chips
 * share copied once; `a` its three places, BN6's blank's where the record has none; the next free place */
static uint32_t art_take(uint32_t a[3], const uint8_t *rec, uint32_t game, uint32_t at) {
	uint32_t icon = get32(rec + BN6_CHIP_ICON_PTR), pic = get32(rec + BN6_CHIP_IMAGE_PTR), pal = get32(rec + BN6_CHIP_PALETTE_PTR);
	if (rom_is_ptr(icon) && rom_off(icon) + ICON_BYTES <= ROM_SIZE) { a[0] = at; at += ICON_BYTES; }
	if (pic == pal || !rom_is_ptr(pic) || !rom_is_ptr(pal) || rom_off(pic) + PICTURE_BYTES > ROM_SIZE || rom_off(pal) + PALETTE_BYTES > ROM_SIZE)
		return at;
	const uint32_t src[2] = { pic | game, pal | game }, size[2] = { PICTURE_BYTES, PALETTE_BYTES };
	for (int k = 0; k < 2; ++k) {
		int s = 0;
		while (s < shared.n && shared.from[s] != src[k]) ++s;
		if (s == shared.n) { shared.from[shared.n] = src[k]; shared.to[shared.n++] = at; at += size[k]; }
		a[1 + k] = shared.to[s];
	}
	return at;
}

static uint32_t art_plan(const uint32_t blank[3]) {
	uint32_t at = CHIPS_ART;
	shared.n = 0;
	memset(M.art, 0, sizeof M.art);
	for (int n = 0; n < 2 * xreg_count; ++n) {
		const XChipReg *r = &xreg_rows[n % xreg_count];
		if (!art_row(r, n / xreg_count)) continue;
		memcpy(M.art[r->bn6], blank, sizeof M.art[0]);
		if (M.rom5) at = art_take(M.art[r->bn6], M.rom5 + BN5_CHIP_RECORDS + XREC_SIZE * r->bn5, 0, at);
	}
	/* (BN4's rows after them, so that BN5's keep their places) */
	for (int n = 0; n < xreg4_count; ++n) {
		const XChipReg4 *r = &xreg4_rows[n];
		memcpy(M.art[r->bn6], blank, sizeof M.art[0]);
		if (M.rom4) at = art_take(M.art[r->bn6], bn4_record(r->bn4), 0x80000000u, at);
	}
	return at - CHIPS_ART;
}

/* ... and copied into the image, the icons in BN6's icons' palette */
static void art_copy(void) {
	static const uint32_t size[3] = { ICON_BYTES, PICTURE_BYTES, PALETTE_BYTES };
	uint8_t map[16];
	if (M.rom4) xrec_icon_colours(M.rom4 + (BN4_CHIP_ICON_PAL - BUS), R.data + R.layout->chip_icon_pal, map);
	for (int n = 0; M.rom4 && n < xreg4_count; ++n) {
		const uint8_t *rec = bn4_record(xreg4_rows[n].bn4);
		for (int k = 0; k < 3; ++k) {
			uint32_t to = M.art[xreg4_rows[n].bn6][k];
			if (to < CHIPS_ART || to - CHIPS_FREE + size[k] > M.len) continue;
			memcpy(M.img + (to - CHIPS_FREE), M.rom4 + rom_off(get32(rec + BN6_CHIP_ICON_PTR + 4 * k)), size[k]);
			if (!k) xrec_icon_recolour(M.img + (to - CHIPS_FREE), map);
		}
	}
	if (!M.rom5) return;
	xrec_icon_colours(M.rom5 + BN5_CHIP_ICON_PAL, R.data + R.layout->chip_icon_pal, map);
	for (int n = 0; n < 2 * xreg_count; ++n) {
		const XChipReg *r = &xreg_rows[n % xreg_count];
		if (!art_row(r, n / xreg_count)) continue;
		const uint8_t *rec = M.rom5 + BN5_CHIP_RECORDS + XREC_SIZE * r->bn5;
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

/* a box as BN6's descriptions are (E8 06 01 01, F1 00 00, its lines by E9, E7 01) of `lines` lines, each `len` bytes
 * of BN6's codes; its bytes */
static uint32_t box_make(uint8_t *b, uint32_t room, const uint8_t *const *lines, const uint32_t *len, int n) {
	static const uint8_t open[7] = { 0xE8, 0x06, 0x01, 0x01, 0xF1, 0x00, 0x00 }, close[2] = { 0xE7, 0x01 };
	uint32_t k = sizeof open;
	memcpy(b, open, sizeof open);
	for (int l = 0; l < n; ++l) {
		if (l) b[k++] = 0xE9;
		uint32_t take = len[l] < room - k - 4 ? len[l] : room - k - 4;
		memcpy(b + k, lines[l], take);
		k += take;
	}
	memcpy(b + k, close, sizeof close);
	return k + (uint32_t)sizeof close;
}

/* a chip of BN4's: its name (`desc` false) or description in BN6's text codes, the description's lines (BN4's box is
 * E7 02, F0 00 00, lines by BN4_TEXT_LINE, E6 01) set in BN6's box; without its ROM the placeholder's */
static void own_text4(const XChipReg4 *r, bool desc, Entries *e) {
	uint8_t b[ENTRY_MAX], line[3][24];
	const uint8_t *lines[3] = { line[0], line[1], line[2] }, *s = NULL;
	uint32_t n = 0, len[3] = { 0 }, got = 0;
	int nlines = 0;
	if (M.rom4 && (!desc || r->bn4 <= BN4_CHIP_DESC_LAST))
		got = entry_to(M.rom4, (desc ? (r->bn4 < 256 ? BN4_CHIP_DESCS : BN4_CHIP_DESCS_HI) : (r->bn4 < 256 ? BN4_CHIP_NAMES : BN4_CHIP_NAMES_HI)) - BUS,
			r->bn4 & 0xFFu, BN4_TEXT_END, &s);
	if (M.rom4 && !desc) {
		n = (uint32_t)xrom_text_bn6(XROM_BN4_RED_SUN_US, s, got, b, sizeof b);
	} else if (!desc) {
		char name[16];
		xchips_text_name4(r->bn4, name, sizeof name);
		n = (uint32_t)ta_encode(name, b, (int)sizeof b);
	} else if (M.rom4) {
		/* (past its box's five bytes, to the command that closes it) */
		for (uint32_t i = 5, from = 5; got > 5 && i <= got && nlines < 3; ++i)
			if (i == got || s[i] >= 0xE5) {
				len[nlines] = (uint32_t)xrom_text_bn6(XROM_BN4_RED_SUN_US, s + from, i - from, line[nlines], sizeof line[0]);
				++nlines;
				if (i == got || s[i] != BN4_TEXT_LINE) break;
				from = i + 1;
			}
		n = box_make(b, sizeof b, lines, len, nlines);
	} else {
		for (int l = 0; l < 3; ++l) len[l] = (uint32_t)ta_encode(xchips_text_desc4[l], line[l], (int)sizeof line[0]);
		n = box_make(b, sizeof b, lines, len, 3);
	}
	entry_add(e, b, n);
}

/* the names' (`desc` false) or descriptions' archive of ids 256 * half on */
static uint32_t archive_of(bool desc, int half) {
	static const uint32_t names[4] = { CHIPS_NAMES0, CHIPS_NAMES1, CHIPS_NAMES2, CHIPS_NAMES3 };
	static const uint32_t descs[4] = { CHIPS_DESCS0, CHIPS_DESCS1, CHIPS_DESCS2, CHIPS_DESCS3 };
	return desc ? descs[half & 3] : names[half & 3];
}

/* names (`desc` false) or descriptions for ids 256 * half on: an own id's
 * its own, BN6's id's BN6's, an id past BN6's archives' none */
static bool texts_make(bool desc, int half) {
	static Entries e;
	memset(e.at, 0, sizeof e.at);
	e.n = 0;
	/* (BN6's own archives hold ids 0-511; past them, the registry's alone) */
	uint32_t arch = half > 1 ? 0 : desc ? R.layout->chip_descs[half] : R.layout->chip_names[half];
	uint32_t count = arch ? get16(R.data + arch) / 2u : 0;
	for (int i = 0; i < 256; ++i) {
		const XChipReg *r = xreg_at(256 * half + i);
		const uint8_t *s = NULL;
		uint32_t len = 0;
		if (r) { own_text(r, desc, &e); continue; }
		if (M.alias && 256 * half + i == M.alias) {
			uint32_t from = desc ? R.layout->chip_descs[M.alias_from >> 8] : R.layout->chip_names[M.alias_from >> 8];
			entry_add(&e, s, entry_at(R.data, from, (uint32_t)M.alias_from & 0xFFu, &s));
			continue;
		}
		const XChipReg4 *r4 = xreg4_at(256 * half + i);
		if (r4) { own_text4(r4, desc, &e); continue; }
		if ((uint32_t)i < count) len = entry_at(R.data, arch, (uint32_t)i, &s);
		entry_add(&e, s, len);
	}
	return archive_write(&e, archive_of(desc, half) - CHIPS_FREE, desc ? CHIPS_DESCS_ROOM : CHIPS_NAMES_ROOM);
}

/* entry `id` of the made names (`desc` false) or descriptions */
static const uint8_t *text_of(int id, bool desc, uint32_t *n) {
	uint32_t at = archive_of(desc, id >> 8) - CHIPS_FREE;
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
	for (int i = 0; i < xreg4_count; ++i) {
		char me[24];
		if (M.alias == xreg4_rows[i].bn6) continue;
		chips_name(xreg4_rows[i].bn6, me, sizeof me);
		put16(M.img + XREC_SIZE * xreg4_rows[i].bn6 + BN6_CHIP_ALPHA_SORT, xrec_abc_place(me, refs, places, n));
	}
}

static void records_make(const uint32_t blank[3]) {
	const uint8_t *table = R.data + R.layout->chip_data;
	for (int id = 0; id < CHIPS_IDS; ++id) {
		uint8_t *out = M.img + XREC_SIZE * (uint32_t)id;
		const XChipReg *r = xreg_at(id);
		if (!r && M.alias && id == M.alias) { memcpy(out, table + XREC_SIZE * (uint32_t)M.alias_from, XREC_SIZE); continue; }
		const XChipReg4 *r4 = r ? NULL : xreg4_at(id);
		if (r4 && M.rom4) {
			XRecordFrom4 in = { bn4_record(r4->bn4), r4->how == XREG_RECORD && r4->twin ? table + XREC_SIZE * r4->twin : NULL, r4, 0, { 0 } };
			memcpy(in.art, M.art[id], sizeof in.art);
			xrec_from_bn4(&in, out);
			continue;
		}
		if (r4) { xrec_placeholder4(r4, 0, blank, out); continue; }
		if (!r) {
			memcpy(out, table + XREC_SIZE * (uint32_t)(id < CHIPS_BN6_RECORDS ? id : BLANK_RECORD), XREC_SIZE);
			/* (BN6's own chip of a name BN5's plays, its code moved: BN5's in what it does) */
			const XChipMoved *mv = M.rom5 && id < CHIPS_BN6_RECORDS ? xchips_moved_of(id) : NULL;
			const XChipReg *named = mv ? xreg_named(id) : NULL;
			if (named) xrec_same_moved(M.rom5 + BN5_CHIP_RECORDS + XREC_SIZE * named->bn5, mv, out);
			/* (and BN5's art, art_row's) */
			for (int k = 0; named && M.art[id][0] && k < 3; ++k) put32(out + BN6_CHIP_ICON_PTR + 4 * k, M.art[id][k]);
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
	/* (BN4's rows: one plays where its record runs on BN6's own code, XREG_RECORD, or its code is moved, XREG_MOVED
	 * (src/emu/xchips_bn4.h's units, installed where BN4's ROM is read), and BN4's ROM is read; the rest sit out, their code
	 * not in BN6's engine; --dev alias's id, a copy of BN6's chip, plays) */
	for (int i = 0; i < xreg4_count; ++i) {
		bool plays = ((xreg4_rows[i].how == XREG_RECORD || xreg4_rows[i].how == XREG_MOVED) && M.rom4) || M.alias == xreg4_rows[i].bn6;
		if (!plays) M.sits[xreg4_rows[i].bn6 >> 3] |= (uint8_t)(1u << (xreg4_rows[i].bn6 & 7));
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
	for (int k = 0; k < 8; ++k) fit &= texts_make(k >= 4, k & 3);
	if (!fit) fprintf(stderr, "chips: a text archive past its room\n");
	/* (the archives' words, the names' and the descriptions', which ten-bit
	 * ids' name and description routines take an id's from: xchips_wide.c) */
	for (int k = 0; k < 8; ++k) put32(M.img + (CHIPS_TABLES - CHIPS_FREE) + 4u * (uint32_t)k, archive_of(k >= 4, k & 3));
	/* (and the folder editor's empty entry's archive, the third word of its two readers' tables, which load it from here
	 * once their literals name these words) */
	put32(M.img + (CHIPS_TABLES - CHIPS_FREE) + 0x20u, get32(R.data + (BN6_CHIP_NAMES_REF_PACK + 4u + 8u - 0x08000000u)));
	abc_make();
	art_copy();
	sits_make();
	return fit;
}

static bool ready(void) {
	const uint8_t *rom5 = XR[XROM_BN5_COLONEL_US].data, *rom4 = XR[XROM_BN4_RED_SUN_US].data;
	if (!R.data || !R.layout || !R.layout->chip_data) return false;
	if (M.made && M.rom6 == R.data && M.rom5 == rom5 && M.rom4 == rom4) return M.img != NULL;
	M.rom6 = R.data;
	M.rom5 = rom5;
	M.rom4 = rom4;
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

void chips_desc_lines(int id, char *out, size_t n) {
	uint32_t len;
	if (id < 0 || id >= CHIPS_IDS || !ready()) { if (n) out[0] = 0; return; }
	const uint8_t *s = text_of(id, true, &len);
	rom_desc_lines(s, len, out, n, '\n');
}

const uint8_t *chips_bytes(uint32_t bus, uint32_t n) {
	if (bus >= BUS && bus - BUS + n <= ROM_SIZE && n <= ROM_SIZE) return R.data ? R.data + (bus - BUS) : NULL;
	if (bus >= CHIPS_FREE && ready() && bus - CHIPS_FREE + n <= M.len && n <= M.len) return M.img + (bus - CHIPS_FREE);
	return NULL;
}

const uint8_t *chips_offset_bytes(uint32_t off, uint32_t n) { return off < 0x01000000u ? chips_bytes(BUS + off, n) : NULL; }

bool chips_bn5(int id) { return xreg_at(id) != NULL; }

bool chips_bn4(int id) { return xreg4_at(id) != NULL; }

bool chips_sits_out(int id) { return id > 0 && id < CHIPS_IDS && ready() && (M.sits[id >> 3] & (1u << (id & 7))); }

const uint8_t *chips_sit_bits(void) { return ready() ? M.sits : NULL; }

bool chips_from_bn5(void) { return ready() && M.rom5; }

bool chips_from_bn4(void) { return ready() && M.rom4; }

const uint8_t *chips_image(uint32_t *len) {
	*len = ready() ? M.len : 0;
	return *len ? M.img : NULL;
}

uint32_t chips_table_bus(void) { return M.installed ? CHIPS_RECORDS : BUS + (R.layout ? R.layout->chip_data : 0); }

void chips_installed(bool on) { M.installed = on; }

void chips_forget(void) { M.made = false; }

void chips_alias(int id, int from) {
	bool ok = id >= CHIPS_BN6_RECORDS && id < CHIPS_IDS && !xreg_at(id) && from > 0 && from < CHIPS_BN6_RECORDS;
	M.alias = ok ? id : 0;
	M.alias_from = ok ? from : 0;
	M.made = false;
}
