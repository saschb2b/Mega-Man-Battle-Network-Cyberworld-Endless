/* Another game's NaviCust program as one of BN6's (xprograms.h). */
#include "xprograms.h"

#include <stdlib.h>
#include <string.h>

#include "bn5.h"
#include "rom.h"

#define BUS      0x08000000u
#define RECORD   16     /* a colour variant's record, in both games */
#define TEXT_END 0xE6
#define TEXT_LINE 0xE9
#define FRAME    5      /* a description's bytes before its lines, and after them */

static uint32_t u16at(const uint8_t *p) { return (uint32_t)(p[0] | p[1] << 8); }
static uint32_t u32at(const uint8_t *p) { return u16at(p) | u16at(p + 2) << 16; }
static void w16(uint8_t *p, uint32_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }

void xprogram_shape7(const uint8_t five[XPROGRAM_SHAPE5], uint8_t seven[XPROGRAM_SHAPE]) {
	memset(seven, 0, XPROGRAM_SHAPE);
	for (int r = 0; r < 5; ++r)
		for (int c = 0; c < 5; ++c) seven[(r + 1) * 7 + c + 1] = five[r * 5 + c] != 0;
}

/* an entry's length with its end mark, 0 where none ends it inside `size` */
static size_t entry_len(const uint8_t *archive, size_t size, uint32_t at) {
	for (size_t i = at; i < size; ++i)
		if (archive[i] == TEXT_END) return i - at + 1;
	return 0;
}

size_t xprogram_name_add(const XArchive *a, int index, const uint8_t *name, size_t n, uint8_t *out, size_t max) {
	const uint8_t *archive = a->data;
	size_t size = a->size;
	int entries = a->entries;
	if (index < 0 || index + 1 >= entries || size < 2u * (size_t)entries || u16at(archive) != 2u * (uint32_t)entries) return 0;
	/* (a filler: its text the next entry's too) */
	if (u16at(archive + 2 * index) != u16at(archive + 2 * index + 2)) return 0;
	size_t end = 0;
	for (int i = 0; i < entries; ++i) {
		uint32_t at = u16at(archive + 2 * i);
		size_t len = at < size ? entry_len(archive, size, at) : 0;
		if (!len) return 0;
		if (at + len > end) end = at + len;
	}
	for (size_t i = 0; i < n; ++i)
		if (name[i] >= 0xE5) return 0;
	if (!n || end + n + 1 > max || end + n + 1 > 0xFFFF) return 0;
	memcpy(out, archive, end);
	memcpy(out + end, name, n);
	out[end + n] = TEXT_END;
	w16(out + 2 * index, (uint32_t)end);
	return end + n + 1;
}

size_t xprogram_descs_grow(const XArchive *a, int framed, const uint8_t *lines, size_t ln, uint8_t *out, size_t max) {
	const uint8_t *raw = a->data;
	size_t n = a->size;
	int entries = a->entries;
	if (n < 4u + 2u * (size_t)entries || entries < 1 || framed < 0 || framed >= entries) return 0;
	const uint8_t *arc = raw + 4;
	size_t an = n - 4;
	if (u32at(raw) != (uint32_t)n << 8 || u16at(arc) != 2u * (uint32_t)entries) return 0;
	uint32_t f = u16at(arc + 2 * framed);
	size_t flen = f < an ? entry_len(arc, an, f) : 0;
	if (flen < 2 * FRAME || arc[f + flen - 1] != TEXT_END) return 0;
	size_t total = n + 2 + FRAME + ln + FRAME;
	if (total > max || total > 0xFFFF) return 0;
	out[0] = 0;
	out[1] = (uint8_t)total;
	out[2] = (uint8_t)(total >> 8);
	out[3] = (uint8_t)(total >> 16);
	uint8_t *o = out + 4;
	/* (every entry two bytes on, for the one offset more; the new entry where the archive ended) */
	for (int i = 0; i < entries; ++i) w16(o + 2 * i, u16at(arc + 2 * i) + 2);
	w16(o + 2 * entries, (uint32_t)an + 2);
	memcpy(o + 2 * entries + 2, arc + 2 * entries, an - 2u * (size_t)entries);
	uint8_t *e = o + an + 2;
	memcpy(e, arc + f, FRAME);
	memcpy(e + FRAME, lines, ln);
	memcpy(e + FRAME + ln, arc + f + flen - FRAME, FRAME);
	return total;
}

/* a description's lines out of BN5's entry `s` (to its end mark, `n` at
 * most) in BN6's codes: the frame's five bytes off each end, the text
 * between through xrom_text_bn6, its line breaks kept; 0 where it is not
 * framed so */
static size_t lines_of(const uint8_t *s, size_t n, uint8_t *out, size_t max) {
	size_t len = entry_len(s, n, 0), k = 0;
	if (len < 2 * FRAME + 1 || s[0] != 0xE8 || s[len - FRAME] != 0xEE) return 0;
	for (size_t i = FRAME; i < len - FRAME;) {
		if (s[i] == TEXT_LINE) { if (k >= max) return 0; out[k++] = s[i++]; continue; }
		if (s[i] >= 0xE5) return 0;
		size_t run = i;
		while (run < len - FRAME && s[run] < 0xE5) ++run;
		k += xrom_text_bn6(XROM_BN5_COLONEL_US, s + i, run - i, out + k, max - k);
		i = run;
	}
	return k;
}

/* its record's four colour variants and its two shapes out of BN5's ROM */
static bool record_read(const uint8_t *rom5, int program5, XProgram *out) {
	const uint8_t *rec = rom5 + (BN5_NCP_RECORDS - BUS) + RECORD * 4u * (uint32_t)program5;
	uint32_t shape = u32at(rec + 8), small = u32at(rec + 12);
	if (shape < BUS || shape - BUS > ROM_SIZE - XPROGRAM_SHAPE5 || small < BUS || small - BUS > ROM_SIZE - XPROGRAM_SHAPE5) return false;
	int colours = 0;
	for (int v = 0; v < XPROGRAM_VARIANTS; ++v) {
		const uint8_t *r = rec + RECORD * v;
		/* (the four of one program: one part, one bug's kind, the same shapes) */
		if (r[1] > 2 || r[3] > 6 || r[1] != rec[1] || r[4] != rec[4] || u32at(r + 8) != shape || u32at(r + 12) != small) return false;
		memcpy(out->head[v], r, 8);
		colours += r[3] != 0;
	}
	xprogram_shape7(rom5 + (shape - BUS), out->shape);
	xprogram_shape7(rom5 + (small - BUS), out->small);
	int cells = 0;
	for (int i = 0; i < XPROGRAM_SHAPE; ++i) {
		cells += out->shape[i];
		if (out->small[i] && !out->shape[i]) return false;   /* (a compressed shape is a part of the full one, as BN6's are) */
	}
	return colours && cells;
}

/* its name and its description's lines, in BN6's codes */
static bool words_read(const uint8_t *rom5, int program5, XProgram *out) {
	const uint8_t *names = rom5 + (BN5_NCP_NAMES - BUS), *name = names + u16at(names + 2 * program5);
	size_t nn = 0;
	while (nn < sizeof out->name && name[nn] != TEXT_END) ++nn;
	if (!nn || nn == sizeof out->name) return false;
	out->name_len = (uint8_t)xrom_text_bn6(XROM_BN5_COLONEL_US, name, nn, out->name, sizeof out->name);
	/* (the descriptions' archive four bytes into its block) */
	size_t dn = 0;
	uint8_t *descs = lz77_decompress(rom5 + (BN5_NCP_DESCS - BUS), ROM_SIZE - (BN5_NCP_DESCS - BUS), &dn);
	if (descs && dn > 4u + 2u * (uint32_t)program5 + 2 && u16at(descs + 4 + 2 * program5) < dn - 4) {
		uint32_t at = 4 + u16at(descs + 4 + 2 * program5);
		out->about_len = (uint8_t)lines_of(descs + at, dn - at, out->about, sizeof out->about);
	}
	free(descs);
	return out->name_len > 0 && out->about_len > 0;
}

bool xprogram_read(const uint8_t *rom5, int program5, XProgram *out) {
	memset(out, 0, sizeof *out);
	return rom5 && program5 > 0 && record_read(rom5, program5, out) && words_read(rom5, program5, out);
}

const XProgram *xprogram_soult(void) {
	static XProgram soult;
	static const uint8_t *read_from;
	static bool ok;
	const uint8_t *rom5 = XR[XROM_BN5_COLONEL_US].data;
	if (!rom5) return NULL;
	if (read_from != rom5) {
		read_from = rom5;
		/* (a plus part, as BN5's SoulT+1 is: BN6's compile counts its bug by the part) */
		ok = xprogram_read(rom5, BN5_NCP_SOULT, &soult) && soult.head[0][1] == 1;
	}
	return ok ? &soult : NULL;
}
