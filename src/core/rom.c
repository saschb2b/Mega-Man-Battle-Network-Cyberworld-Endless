#include "rom.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

Rom R;

static const RomLayout layouts[] = {
	[ROM_BN6_GREGAR_US] = {
		.name = "Mega Man Battle Network 6: Cybeast Gregar (USA)",
		.sha1 = "89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6",
		.sprite_lists = 0x031CC4,
		.chip_data = 0x021DA8,
		.chip_names = { 0x6E88D0, 0x6E92D8 },
		.enemy_ids = 0x0182C4,
		.encounters = 0x020170,
		.title = { 0x7F3040, 0x7F7CFC, 0x7F2E40, 0x7F1EBC, 0x7F216C, 0x7F218C, 0x7F21EC, 0x7F2C20, 0x6A280C, 0x6A344C },
		.net_area = {
			{ 0x90, 0, 0x0018, 0x0040, false, 0x13, 0x90, 0, 3, { { 0x90, 1 } } },     /* Central Area 1; battles of Central 1-3 */
			{ 0x91, 0, 0x0040, 0x0006, false, 0x11, 0x91, 0, 3, { { 0x91, 1 }, { 0x91, 2 } } },     /* Seaside Area 1; Seaside 1-3 */
			{ 0x94, 1, 0x1000, 0x0140, false, 0x0A, 0x94, 0, 3, { { 0x94, 0 } } },     /* Sky Area 2; Sky 1-3 */
			{ 0x92, 0, 0x0010, 0x0001, false, 0x12, 0x92, 0, 2 },     /* Green Area 1; Green 1-2 */
			{ 0x96, 1, 0x1000, 0x0100, false, 0x09, 0x96, 0, 3, { { 0x96, 0 }, { 0x96, 2 } } },     /* Graveyard; Graveyard 1-3 */
			{ 0x95, 0, 0x0001, 0x0C00, true, 0x14, 0x95, 0, 3, { { 0x95, 2 }, { 0x95, 3 } } },      /* Undernet 1; Undernet 1-3 */
			{ 0x95, 1, 0x1000, 0x0C00, true, 0x20, 0x95, 2, 2 },      /* Undernet Zero; Undernet 3-4 */
			{ 0x93, 1, 0x0004, 0x0000, true, 0x21, 0x93, 0, 2, { { 0x93, 0 } } },      /* Underground 2; Underground 1-2 */
			{ 0x8C, 0, 0x0002, 0x0008, false, 0x13, 0x8C, 0, 16 },    /* a comp (orange, green); the comps of group 0x8C */
			{ 0x88, 3, 0x0800, 0x0020, false, 0x13, 0x88, 1, 6 },     /* a homepage (pink, teal); the homepages */
			{ 0x8C, 1, 0x00C0, 0x0400, false, 0x13, 0x8D, 0, 16 },    /* a comp (blue, pink); the comps of group 0x8D */
			{ 0x80, 1, 0x1000, 0x0200, false, 0x10, 0x80, 0, 2, { { 0x80, 0 }, { 0x85, 3 } } },     /* Robot Control Comp 2 (white, violet walkways; its teal pads are flat inside and would fill the platforms) */
			{ 0x81, 2, 0x00C0, 0x0000, false, 0x11, 0x81, 0, 3, { { 0x81, 0 }, { 0x81, 1 }, { 0x85, 0 } }, 16 },     /* Aquarium Comp 3 (water; its mazes are water too, its yellow fish two panels long; its platforms are glass pads) */
			{ 0x82, 2, 0x0003, 0x0000, false, 0x12, 0x82, 0, 3, { { 0x82, 0 }, { 0x82, 1 }, { 0x85, 1 } } },     /* Judge Tree Comp 3 (brick) */
			{ 0x83, 2, 0x0180, 0x1000, true, 0x0A, 0x83, 0, 3, { { 0x83, 0 }, { 0x83, 1 }, { 0x85, 2 } } },     /* Mr. Weather Comp 3 (lavender; its pale conveyor belts the walkways; snow and clouds on the back layer, their drifts too ragged to learn) */
			{ 0x85, 4, 0x8000, 0x8000, false, 0x20, 0x85, 0, 5 },     /* CopyBot Comp, its floors told by shape (TILES_BY_SHAPE): hue cannot part its purple plateaus in stone rims from its pink and white walkways with teal discs; the Pavilion comps' battles */
			{ 0x88, 1, 0x0003, 0x1000, false, 0x13, 0x88, 1, 1 },     /* ACDC HP (yellow, grey) */
			{ 0x88, 5, 0x0002, 0x0008, false, 0x13, 0x88, 5, 1 },     /* Green HP (brown, green) */
			{ 0x88, 6, 0x0100, 0x00C0, false, 0x13, 0x88, 6, 1 },     /* Sky HP (purple, cyan) */
		},
		.song_table = 0x159F48,
		.battle_bgs = 0x082058,
		.battle_bg_anims = 0x0822E0,
	},
};

/* ---- SHA-1 (FIPS 180-1) ---- */
typedef struct { uint32_t h[5]; uint64_t len; uint8_t buf[64]; size_t fill; } Sha1;

static uint32_t rol(uint32_t v, int n) { return (v << n) | (v >> (32 - n)); }

static void sha1_block(Sha1 *s, const uint8_t *p) {
	uint32_t w[80];
	for (int i = 0; i < 16; ++i) w[i] = (uint32_t)p[i * 4] << 24 | (uint32_t)p[i * 4 + 1] << 16 | (uint32_t)p[i * 4 + 2] << 8 | p[i * 4 + 3];
	for (int i = 16; i < 80; ++i) w[i] = rol(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
	uint32_t a = s->h[0], b = s->h[1], c = s->h[2], d = s->h[3], e = s->h[4];
	for (int i = 0; i < 80; ++i) {
		uint32_t f, k;
		if (i < 20) { f = (b & c) | (~b & d); k = 0x5A827999; }
		else if (i < 40) { f = b ^ c ^ d; k = 0x6ED9EBA1; }
		else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
		else { f = b ^ c ^ d; k = 0xCA62C1D6; }
		uint32_t t = rol(a, 5) + f + e + k + w[i];
		e = d; d = c; c = rol(b, 30); b = a; a = t;
	}
	s->h[0] += a; s->h[1] += b; s->h[2] += c; s->h[3] += d; s->h[4] += e;
}

void sha1_hex(const uint8_t *data, size_t len, char out[41]) {
	Sha1 s = { { 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0 }, 0, { 0 }, 0 };
	size_t i = 0;
	for (; i + 64 <= len; i += 64) sha1_block(&s, data + i);
	uint8_t tail[128] = { 0 };
	size_t rem = len - i;
	memcpy(tail, data + i, rem);
	tail[rem] = 0x80;
	size_t tl = rem + 1 + 8 <= 64 ? 64 : 128;
	uint64_t bits = (uint64_t)len * 8;
	for (int k = 0; k < 8; ++k) tail[tl - 1 - k] = (uint8_t)(bits >> (8 * k));
	sha1_block(&s, tail);
	if (tl == 128) sha1_block(&s, tail + 64);
	for (int k = 0; k < 5; ++k) sprintf(out + k * 8, "%08x", s.h[k]);
}

/* ---- LZ77 ---- */
uint8_t *lz77_decompress(const uint8_t *src, size_t avail, size_t *out_len) {
	if (avail < 4 || src[0] != 0x10) return NULL;
	size_t n = src[1] | src[2] << 8 | src[3] << 16;
	uint8_t *out = malloc(n ? n : 1);
	size_t p = 4, o = 0;
	while (o < n) {
		if (p >= avail) goto fail;
		uint8_t flags = src[p++];
		for (int i = 0; i < 8 && o < n; ++i, flags <<= 1) {
			if (flags & 0x80) {
				if (p + 1 >= avail) goto fail;
				unsigned v = src[p] << 8 | src[p + 1];
				p += 2;
				size_t len = (v >> 12) + 3, dist = (v & 0xFFF) + 1;
				if (dist > o) goto fail;
				for (size_t k = 0; k < len && o < n; ++k, ++o) out[o] = out[o - dist];
			} else {
				if (p >= avail) goto fail;
				out[o++] = src[p++];
			}
		}
	}
	if (out_len) *out_len = n;
	return out;
fail:
	free(out);
	return NULL;
}

/* ---- Text ---- */
static const char *glyph(uint8_t c) {
	static const char *punct[] = {
		[0x98] = "-", [0x99] = "x", [0x9A] = "=", [0x9B] = ":", [0x9C] = "%", [0x9D] = "?", [0x9E] = "+",
		[0xA2] = "!", [0xA3] = "&", [0xA4] = ",", [0xA6] = ".", [0xA8] = ";", [0xA9] = "'", [0xAA] = "\"",
		[0xAB] = "~", [0xAC] = "/", [0xAD] = "(", [0xAE] = ")", [0xB1] = ">", [0xB2] = "_",
	};
	static char one[2];
	if (c == 0) return " ";
	if (c >= 0x01 && c <= 0x0A) { one[0] = '0' + c - 1; one[1] = 0; return one; }
	if (c >= 0x0B && c <= 0x24) { one[0] = 'A' + c - 0x0B; one[1] = 0; return one; }
	if (c == 0x25) return "*";
	if (c >= 0x26 && c <= 0x3F) { one[0] = 'a' + c - 0x26; one[1] = 0; return one; }
	/* Version marks ([RV] [BX] [EX] [SP] [FZ]) have their own glyphs; they
	 * travel through strings as control bytes 1-5. */
	if (c >= 0x40 && c <= 0x44) { one[0] = (char)(1 + c - 0x40); one[1] = 0; return one; }
	if (c < sizeof punct / sizeof *punct && punct[c]) return punct[c];
	return "";
}

void rom_text(uint32_t archive, int index, char *out, size_t outlen) {
	uint32_t p = archive + rom_u16(archive + 2 * index);
	size_t o = 0;
	for (int i = 0; i < 64; ++i) {
		uint8_t c = R.data[p + i];
		if (c >= 0xE5) break;
		const char *g = glyph(c);
		for (; *g && o + 1 < outlen; ++g) out[o++] = *g;
	}
	out[o] = 0;
}

/* ---- Discovery ---- */
bool rom_load_file(const char *path, char *msg, size_t msglen) {
	FILE *f = fopen(path, "rb");
	if (!f) { snprintf(msg, msglen, "Cannot open %s", path); return false; }
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);
	if (size != ROM_SIZE) { fclose(f); snprintf(msg, msglen, "%s is not an 8 MB GBA ROM", path); return false; }
	uint8_t *data = malloc(ROM_SIZE);
	size_t got = fread(data, 1, ROM_SIZE, f);
	fclose(f);
	if (got != ROM_SIZE) { free(data); snprintf(msg, msglen, "Short read on %s", path); return false; }
	char hex[41];
	sha1_hex(data, ROM_SIZE, hex);
	for (size_t i = 0; i < sizeof layouts / sizeof *layouts; ++i) {
		if (!strcmp(hex, layouts[i].sha1)) {
			free(R.data);
			R.data = data;
			R.version = (RomVersion)i;
			R.layout = &layouts[i];
			snprintf(R.path, sizeof R.path, "%s", path);
			return true;
		}
	}
	free(data);
	snprintf(msg, msglen, "%s is not a supported ROM (SHA-1 %.12s...)", path, hex);
	return false;
}

bool rom_find(const char *dir, char *msg, size_t msglen) {
	DIR *d = opendir(dir);
	snprintf(msg, msglen, "Put your Mega Man Battle Network 6: Cybeast Gregar (USA) ROM in %s", dir);
	if (!d) return false;
	struct dirent *e;
	bool found = false;
	char last[512] = "";
	while (!found && (e = readdir(d))) {
		size_t n = strlen(e->d_name);
		if (n < 4 || strcasecmp(e->d_name + n - 4, ".gba")) continue;
		char path[1024];
		snprintf(path, sizeof path, "%s/%s", dir, e->d_name);
		found = rom_load_file(path, last, sizeof last);
	}
	closedir(d);
	if (!found && last[0]) snprintf(msg, msglen, "%s", last);
	return found;
}
