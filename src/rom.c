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
		.enemy_names = 0x6EE400,
		.navi_names = 0x7F1D8C,
		.enemy_ids = 0x0182C4,
		.enemy_stats = 0x00F260,
		.panel_tiles = 0x6DBB24,
		.panel_pals = { 0x6DE87C, 0x6DE8FC, 0x6DE65C, 0x6DE45C, 0x6DE95C, 0x6DE8DC, 0x6DE71C, 0x6DE51C },
		.battle_bg_table = 0x082058,
		.battle_bg_anims = 0x0822E0,
		.chip_icon_pal = 0x6E3880,
		.net_maps = 0x0329C4,
		.net_area = {
			{ 0x90, 0, 62, 1, 0x0018, 0x072B34, 0x072A90, 1 },  /* Central Area 1 */
			{ 0x91, 0, 1, 6, 0x0040, 0x0766D0, 0x076628, 1 },   /* Seaside Area 1 */
			{ 0x94, 1, 38, 7, 0x1000, 0x07BBE4, 0x07BB10, 1 },  /* Sky Area 2 */
			{ 0x92, 0, 1, 1, 0x0010, 0x078FD4, 0x078F34, 1 },   /* Green Area 1 */
			{ 0x96, 1, 26, 5, 0x1000, 0x07FF0C, 0x07FE68, 2 },  /* Graveyard */
			{ 0x95, 0, 52, 5, 0x0800, 0x07E11C, 0x07E020, 0 },  /* Undernet 1 */
			{ 0x95, 2, 26, 5, 0x0800, 0x07E11C, 0x07E050, 0 },  /* Undernet 3 */
			{ 0x93, 1, 58, 8, 0x0200, 0x07A5B8, 0x07A538, 0 },  /* Underground 2 */
		},
		.song_table = 0x159F48,
		.chip_desc = { 0x6E983C, 0 },
		.ui = {
			.hp_digits = 0x6DF5BC, .hp_pal = 0x6DFBFC,
			.gauge = 0x6E2820, .gauge_pal = 0x6DFBFC,
			.window_pal = 0x6E3800,
			.code_letters = 0x6E0E1C, .grid_letters = 0x6E38A0,
			.power_digits = 0x6E20A0,
			.elem_icons = 0x6E151C,
			.icon_pal = 0x72AF30, .icon_dim_pal = 0x72AEF0, .icon_obj_pal = 0x72AF10,
			.icon_elem_colors = 0x6E1A9C,
			.enemy_hp_digits = 0x6DEA3C, .enemy_hp_pal = 0x6A3C8C,
			.emotion = 0x72B750, .emotion_face = 0x72AF50, .emotion_pal = 0x72D050,
			.cursor = 0x6E3540, .cursor_pal = 0x6E3680, .emblem = 0x6F3770,
			.result_digits = 0x7307B0, .result_pal = 0x730750,
			.chat_font = 0x6A3CAC, .chat_pal = 0x6A3C8C, .next_arrow = 0x6A278C,
			.card_sprite = 7,
			.list_elem_icons = 0x6E1B20, .list_elem_pal = 0x6E2360, .list_code_pal = 0x6C7AA0, .list_icon_pal = 0x72AED0,
			.list_arrow = 0x82000084, .list_arrow_pal = 0x6C7CB8,
			.hp_hurt_pal = 0x6DFC3C, .panel_warn_pal = 0x6DE5BC, .cursor_wide = 0x6E3560,
			.ok_art = 0x7204F0, .ok_art_pal = 0x723730, .press_a = 0x730AF0,
			.box_corner = 0x6BCB4C, .box_edge = 0x6BCAAC, .box_side = 0x6BCB6C, .box_side_top = 0x6BCBAC,
			.box_fill = 0x02A6FC, .box_pal = 0x6BCBCC, .box_arrow = 0x6A270C,
			.delete_sprite = 155, .zenny_art = 0x730D90, .zenny_pal = 0x7312D0, .name_tab = 0x6E4020,
			.charge_sprite = 162, .charge_full_pal = 0x3AB1B0, .pause_text = 0x6E40A0,
		},
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

void rom_script_text(uint32_t archive, int index, char *out, size_t outlen) {
	uint32_t p = archive + rom_u16(archive + 2 * index);
	size_t o = 0;
	for (int i = 0; i < 160 && o + 1 < outlen; ++i) {
		uint8_t c = R.data[p];
		if (c == 0xE6) break;
		if (c == 0xE9) { out[o++] = '\n'; ++p; continue; }
		if (c >= 0xE7) {
			/* script commands and their argument bytes */
			static const uint8_t args[] = { [0xE7 - 0xE7] = 1, [0xE8 - 0xE7] = 3, [0xEA - 0xE7] = 3, [0xEB - 0xE7] = 0,
				[0xEC - 0xE7] = 2, [0xED - 0xE7] = 2, [0xEE - 0xE7] = 3, [0xF0 - 0xE7] = 2, [0xF1 - 0xE7] = 2 };
			p += 1 + (c - 0xE7 < (int)sizeof args ? args[c - 0xE7] : 0);
			continue;
		}
		const char *g = glyph(c);
		for (; *g && o + 1 < outlen; ++g) out[o++] = *g;
		++p;
	}
	/* trim leading/trailing breaks */
	while (o && out[o - 1] == '\n') --o;
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
