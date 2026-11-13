#include "rom.h"

#include "net.h"   /* LOOK_* */
#include "bn4.h"   /* BN4_GROUP_*, BN4_MUGSHOT_FACE */
#include "bn5.h"   /* BN5_NAVI_* */

#include <dirent.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

Rom R;

/* (an area's looks, bit per LOOK_*) */
#define L(look) (1u << LOOK_##look)
static const RomLayout layouts[] = {
	[ROM_BN6_GREGAR_US] = {
		.name = "Mega Man Battle Network 6: Cybeast Gregar (USA)",
		.sha1 = "89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6",
		.code = "BR5E",
		.sprite_lists = 0x031CC4,
		.chip_data = 0x021DA8,
		.chip_names = { 0x6E88D0, 0x6E92D8 },
		.chip_descs = { 0x6E983C, 0x6EC050 },
		.enemy_ids = 0x0182C4,
		.enemy_stats = 0x00F260,
		.enemy_traits = 0x00F230,
		.encounters = 0x020170,
		.navicust_programs = 0x13B22C,
		.navicust_codes = 0x13D302,
		.program_advances = 0x02BA60,
		.battle_gem_rewards = 0x0211A0,
		.chip_icon_pal = 0x72AED0,
		.element_icons = 0x6E1B20,
		.element_icon_pal = 0x6E2360,
		.enemy_names = { 0x6EE400, 0x6EED00 },
		.navicust_names = 0x73C98C,
		.item_names = 0x73B938,
		.title = { 0x7F3040, 0x7F7CFC, 0x7F2E40, 0x7F1EBC, 0x7F216C, 0x7F218C, 0x7F21EC, 0x7F2C20, 0x6A280C, 0x6A344C },
		.net_area = {
			{ 0x90, 0, 0x100018, 0x0040, false, 0x13, 0x90, 0, 3, { { 0x90, 1 } }, .counter = { 0x94, 2, -300, -144 }, .looks = L(TREE) | L(SIGN), .pad_hues = 0x0040 },     /* Central Area 1 (its framed pads, their blue recess, cut whole); battles of Central 1-3; the Net Dealer's capsule is Sky Area 3's, in the same tiles as Central Area 2's (bank 3), in Central's colours */
			{ 0x91, 0, 0x2040, 0x0006, false, 0x11, 0x91, 0, 3, { { 0x91, 1 }, { 0x91, 2 } }, .counter = { 0x94, 2, -300, -144 }, .looks = L(BBS) | L(SIGN), .pad_hues = 0x0006, .host = 2, .skip_styles = 0x8C00 },     /* Seaside Area 1 (its framed pads, their yellow recess, cut whole); Seaside 1-3 (and their colours: TILES_MORE_COLOURS, Seaside 2 and 3's yellow panels are its second floor's only fields); its layers in Seaside Area 2's slot (Area 1 draws sprites behind its second layer) */
			{ 0x94, 1, 0x505000, 0x0040, false, 0x0A, 0x94, 0, 3, { { 0x94, 0 } }, .counter = { 0x94, 2, -300, -144 }, .looks = L(TREE) | L(SIGN), .pad_hues = 0x0F00, .arrow_maps = { { 0x94, 2 } } },     /* Sky Area 2 (its framed pads, their lavender recess, cut whole; its walkways the cyan glass catwalks alone: the lavender ones' walls and pieces turned up at every bend of the cyan ones; its fields the panels of their hue in blocks, the catwalks those a panel wide, TILES_WALK_NARROW; its arena a field, TILES_ARENA_FLOOR); Sky 1-3 (not the look of its pads, round pods: TILES_NO_PAD_LOOK); the Net Dealer's capsule of Sky Area 3 */
			{ 0x92, 0, 0x100010, 0x0001, false, 0x12, 0x92, 0, 2, { { 0 } }, 0, NET_APART_PLATFORMS, .counter = { 0x92, 1, -28, -468 }, .looks = L(TREE) | L(GIANT_TREE) | L(SIGN), .arrow_maps = { { 0x92, 1 } } },     /* Green Area 1; Green 1-2 (its planks reach the raised grass by stairs, never flush); the Net Dealer's NetCafe desk of Green Area 2 */
			{ 0x96, 1, 0x11000, 0x0100, false, 0x09, 0x96, 0, 3, { { 0x96, 0 }, { 0x96, 2 } }, .looks = L(MONUMENT) | L(GRAVE), .emblem = 0x56C0, .rebank = { 2, 1 } },     /* Graveyard; Graveyard 1-3 (its cyan crosses; its pale platforms' tiles drawn in its dark slabs' colours); its slabs' edges are rims with clips at the panels' joins (TILES_RIMMED) */
			{ 0x95, 0, 0x811000, 0x0C01, true, 0x14, 0x95, 0, 3, { { 0x95, 2 }, { 0x95, 3 } }, .looks = L(STATUE) | L(BRAZIER), .emblem = 0x7D3F, .skip_styles = 0x8038, .joint_hues = 0x0006 },      /* Undernet 1; Undernet 1-3 (mauve stone plateaus in a red lip with spikes, TILES_RIMMED; its magenta crosses; its red striped bridges turn and end in joints with a yellow gem, redder: walkway floor too) */
			{ 0x95, 1, 0x811000, 0x0C01, true, 0x20, 0x95, 2, 2, .looks = L(STATUE) | L(BRAZIER), .emblem = 0x7D3F, .joint_hues = 0x0006 },      /* Undernet Zero; Undernet 3-4 (the same plateaus, crosses and bridges) */
			{ 0x93, 1, 0x211000, 0x0004, true, 0x21, 0x93, 0, 2, { { 0x93, 0 } } },      /* Underground 2; Underground 1-2 (grey stone platforms in a magenta lip with spikes, TILES_RIMMED; its walkways the magenta links with a yellow gem; no scenery: its one piece, a grey cube, is debris under the altar, TILES_NO_SCENERY) */
			{ 0x8C, 0, 0x80002, 0x0008, false, 0x13, 0x8C, 0, 16 },    /* a comp (orange, green; its green crosses its orange fields in stripes, TILES_CROSSING); the comps of group 0x8C */
			{ 0x88, 3, 0x80800, 0x0020, false, 0x13, 0x88, 1, 6 },     /* a homepage (pink, teal); the homepages */
			{ 0x8C, 1, 0x800C0, 0x0400, false, 0x13, 0x8D, 0, 16 },    /* a comp (blue, pink); the comps of group 0x8D */
			{ 0x80, 1, 0x171000, 0x0200, false, 0x10, 0x80, 0, 2, { { 0x80, 0 }, { 0x85, 3 } } },     /* Robot Control Comp 2 (white, violet walkways; its teal pads are flat inside and would fill the platforms). Its white platforms are framed by two bands half a panel deep (TILES_RIMMED); the walls inside them ring the grey cubes that stand on them, no holes (TILES_INNER_WALLS); its one small platform is the striped conveyor before the robot's door, so its pads take the look of Comp 1's and the Pavilion's raised white platforms (TILES_MORE_PADS); its guardian's arena in its platforms' floor, its circuits a panel wide (TILES_ARENA_FLOOR) */
			{ 0x81, 2, 0x00C0, 0x0000, false, 0x11, 0x81, 0, 3, { { 0x81, 0 }, { 0x81, 1 }, { 0x85, 0 } }, 16, NET_APART_PADS },     /* Aquarium Comp 3 (water; its mazes are water too, its yellow fish two panels long; its platforms are glass pads, raised on legs and reached by stairs) */
			{ 0x82, 2, 0x4003, 0x0000, false, 0x12, 0x82, 0, 3, { { 0x82, 0 }, { 0x82, 1 }, { 0x85, 1 } }, 0, NET_APART_NONE, 0x0001 },     /* Judge Tree Comp 3 (brick; the tiles of its flat red courts are left out, and its pads' look, round stumps: TILES_NO_PAD_LOOK) */
			{ 0x83, 2, 0x110180, 0x1000, true, 0x0A, 0x83, 0, 3, { { 0x83, 0 }, { 0x83, 1 }, { 0x85, 2 } }, .host = 1, .skip_styles = 0x4000 },     /* Mr. Weather Comp 3 (lavender; its pale conveyor belts the walkways; snow and clouds on the back layer, their drifts too ragged to learn; its guardian's arena in its platforms' floor, its lamps a panel wide, TILES_ARENA_FLOOR; its fields' edges are rims, TILES_RIMMED; its fan belts and the clouds on its fields lend no tiles, SKIP_PALE); its layers in Comp 1's slot (Comp 3 draws sprites behind its second layer) */
			{ 0x85, 4, 0x8000, 0x8000, false, 0x20, 0x85, 0, 5, { { 0 } }, 0, NET_APART_PLATFORMS },     /* CopyBot Comp, its floors told by shape (TILES_BY_SHAPE): hue cannot part its purple plateaus in stone rims from its pink and white walkways with teal discs; its plateaus stand on pods, reached by ladders; the Pavilion comps' battles */
			{ 0x88, 1, 0x80003, 0x1000, false, 0x13, 0x88, 1, 1 },     /* ACDC HP (yellow, grey) */
			{ 0x88, 5, 0x80002, 0x0008, false, 0x13, 0x88, 5, 1 },     /* Green HP (brown, green) */
			{ 0x88, 6, 0x80100, 0x00C0, false, 0x13, 0x88, 6, 1 },     /* Sky HP (purple, cyan) */
		},
		.song_table = 0x159F48,
		.battle_bgs = 0x082058,
		.battle_bg_anims = 0x0822E0,
		.battle_bg_scroll = 0x081EF4,
		.battle_bg_refs = { 0x082054, 0x081EEC, 0x081EF0 },
		.map_enters = 0x03093C,
		.map_anims = 0x030998,
	},
};

#undef L

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
	for (int k = 0; k < 5; ++k) snprintf(out + k * 8, 9, "%08x", s.h[k]);
}

/* ---- LZ77 ---- */
uint8_t *lz77_decompress(const uint8_t *src, size_t avail, size_t *out_len) {
	if (avail < 4 || src[0] != 0x10) return NULL;
	size_t n = src[1] | src[2] << 8 | src[3] << 16;
	uint8_t *out = malloc(n ? n : 1);
	if (!out) return NULL;   /* (the 3DS runs short: the analyzer found it written unchecked, issue #19) */
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

void rom_text_bytes(const uint8_t *s, size_t n, char *out, size_t outlen) {
	size_t o = 0;
	for (size_t i = 0; i < n && s[i] < 0xE5; ++i)
		for (const char *g = glyph(s[i]); *g && o + 1 < outlen; ++g) out[o++] = *g;
	if (outlen) out[o] = 0;
}

/* Entry `index` of the text archive at offset `archive` of ROM `data`
 * (ROM_SIZE bytes): where its bytes begin, `*n` of them to the ROM's end
 * (64 at most) */
static const uint8_t *entry_of(const uint8_t *data, uint32_t archive, int index, size_t *n) {
	uint32_t at = archive + 2u * (uint32_t)index, p = ROM_SIZE;
	if (at + 2 <= ROM_SIZE) p = archive + (uint32_t)(data[at] | data[at + 1] << 8);
	*n = p < ROM_SIZE ? ROM_SIZE - p : 0;
	if (*n > 64) *n = 64;
	return data + (p < ROM_SIZE ? p : 0);
}

void rom_text(uint32_t archive, int index, char *out, size_t outlen) {
	size_t n;
	const uint8_t *s = entry_of(R.data, archive, index, &n);
	rom_text_bytes(s, n, out, outlen);
}

/* A code of BN5 Team Colonel's text as BN6's code of the same glyph
 * (docs/rom/bn5-world.md, BN5's text: both fonts compared glyph for glyph):
 * the same to 0x3F (space, digits, letters, *), then five on, as BN6 put
 * its version marks at 0x40-0x44 before the rest, three on past BN5's own
 * marks; XTEXT_DS for its DS mark, -1 for a glyph BN6's font lacks (its
 * alpha, beta, omega and three more) */
#define XTEXT_DS (-2)
static int bn5_code(uint8_t c) {
	if (c < 0x40) return c;
	if (c <= 0x92 || (c >= 0x94 && c <= 0x9A)) return c + 5;
	if (c >= 0xA0 && c <= 0xAD && c != 0xA5) return c + 3;
	switch (c) {
	case 0x93: case 0x9C: return 0x98;   /* a dash (BN6's names spell theirs 0x98; its 0xA1 is the same glyph) */
	case 0x9D: return 0xA2;              /* ! */
	case 0x9E: return 0x43;              /* the SP mark */
	case 0x9F: return XTEXT_DS;          /* the DS mark: none in BN6 */
	case 0x9B: case 0xA5: case 0xB1: case 0xB4: return 0xA0;   /* the empty box */
	case 0xAE: case 0xAF: case 0xB0: case 0xB5: case 0xB6: case 0xB7: return -1;
	default: return c;                   /* (0xB2, 0xB3, 0xB8 on: the same) */
	}
}

/* ... and of BN4 Red Sun's: the same to 0x3F; its punctuation, 0x40 to
 * 0x57, BN6's 0x58 on (BN6 put its version marks and more before them:
 * BN4's dash 0x40 is BN6's 0x98, its plus 0x46 the 0x9E, its dot 0x4E the
 * 0xA6). Measured over its chips' names and the 64 descriptions it shares
 * word for word with BN6's or BN5's chip of the name: ten codes there
 * (0x40, 0x42, 0x43, 0x46, 0x4A, 0x4B, 0x4C, 0x4E, 0x51, 0x54), each 0x58
 * below BN6's every time; the rest of the run assumed (docs/multirom/
 * bn4-chips.md, The census). Past it, as they are */
static int bn4_code(uint8_t c) { return c >= 0x40 && c <= 0x57 ? c + 0x58 : c; }

size_t xrom_text_bn6(int xrom, const uint8_t *src, size_t n, uint8_t *dst, size_t max) {
	size_t k = 0;
	for (size_t i = 0; i < n && src[i] < 0xE5; ++i) {
		int c = xrom == XROM_BN5_COLONEL_US ? bn5_code(src[i]) : xrom == XROM_BN4_RED_SUN_US ? bn4_code(src[i]) : src[i];
		/* (the DS mark spelt as BN6 can: D and S) */
		const uint8_t ds[2] = { 0x0B + 'D' - 'A', 0x0B + 'S' - 'A' }, one[1] = { (uint8_t)c };
		const uint8_t *put = c == XTEXT_DS ? ds : one;
		size_t len = c == XTEXT_DS ? 2 : c >= 0 ? 1 : 0;
		for (size_t j = 0; j < len && k < max; ++j) dst[k++] = put[j];
	}
	return k;
}

void xrom_text(int xrom, uint32_t archive, int index, char *out, size_t outlen) {
	if (xrom < 0 || xrom >= XROM_COUNT || !XR[xrom].data) { if (outlen) out[0] = 0; return; }
	size_t n;
	const uint8_t *s = entry_of(XR[xrom].data, archive, index, &n);
	uint8_t bn6[96];
	rom_text_bytes(bn6, xrom_text_bn6(xrom, s, n, bn6, sizeof bn6), out, outlen);
}

/* the arguments a script command of a description takes (a byte past it):
 * E8 three, F1 two, E7 one; E9 and E6 none; -1 one it does not know */
static int command_args(uint8_t c) {
	switch (c) {
	case 0xE8: return 3;
	case 0xF1: return 2;
	case 0xE7: return 1;
	case 0xE9: case 0xE6: return 0;
	default: return -1;
	}
}

size_t xrom_script_bn6(int xrom, const uint8_t *src, size_t n, uint8_t *dst, size_t max) {
	size_t k = 0;
	for (size_t i = 0; i < n && k < max;) {
		if (src[i] < 0xE5) {
			size_t run = i;
			while (run < n && src[run] < 0xE5) ++run;
			k += xrom_text_bn6(xrom, src + i, run - i, dst + k, max - k);
			i = run;
			continue;
		}
		int args = command_args(src[i]);
		if (args < 0) { ++i; continue; }
		for (int j = 0; j <= args && i < n && k < max; ++j) dst[k++] = src[i++];
		if (dst[k - 1] == 0xE6 && !args) break;
	}
	return k;
}

void rom_desc_bytes(const uint8_t *s, size_t n, char *out, size_t outlen) { rom_desc_lines(s, n, out, outlen, ' '); }

void rom_desc_lines(const uint8_t *s, size_t n, char *out, size_t outlen, char line) {
	size_t o = 0;
	for (size_t i = 0; i < n && o + 1 < outlen;) {
		uint8_t c = s[i];
		if (c == 0xE8) { i += 4; continue; }   /* ts_msg_open_quick_ext */
		if (c == 0xF1) { i += 3; continue; }   /* ts_text_speed */
		if (c == 0xE9) { out[o++] = line; ++i; continue; }   /* a new line */
		if (c >= 0xE5) break;                  /* ts_key_wait, ts_end */
		for (const char *g = glyph(c); *g && o + 1 < outlen; ++g) out[o++] = *g;
		++i;
	}
	if (outlen) out[o] = 0;
}

/* ---- Discovery ---- */
bool rom_load_file(const char *path, char *msg, size_t msglen) {
	FILE *f = fopen(path, "rb");
	if (!f) { snprintf(msg, msglen, "Cannot open %s", path); return false; }
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);
	if (size != ROM_SIZE) { fclose(f); snprintf(msg, msglen, "%s is not an 8 MB GBA ROM", path); return false; }
	/* (the header's game code first: a folder of other games' ROMs, a 3DS's
	 * or an emulator's, is passed over without reading each whole) */
	char code[5] = "";
	bool known = false;
	if (fseek(f, 0xAC, SEEK_SET) == 0 && fread(code, 1, 4, f) == 4)
		for (size_t i = 0; i < sizeof layouts / sizeof *layouts; ++i) known |= !memcmp(code, layouts[i].code, 4);
	if (!known) {
		fclose(f);
		/* (Battle Network 6's other versions named: a 3DS player's ROM was
		 * the European Gregar, and the message said only where to put one) */
		static const struct { const char *code, *name; } others[] = {
			{ "BR5P", "Cybeast Gregar (Europe)" }, { "BR6E", "Cybeast Falzar (USA)" }, { "BR6P", "Cybeast Falzar (Europe)" },
			{ "BR5J", "Rockman EXE 6 Gregar (Japan)" }, { "BR6J", "Rockman EXE 6 Falzar (Japan)" },
		};
		const char *name = NULL;
		for (size_t i = 0; i < sizeof others / sizeof *others; ++i) if (!memcmp(code, others[i].code, 4)) name = others[i].name;
		const char *file = strrchr(path, '/');
		file = file ? file + 1 : path;
		if (name) snprintf(msg, msglen, "%s is %s: only Cybeast Gregar (USA) works so far", file, name);
		else snprintf(msg, msglen, "%s is not a supported ROM (game code %.4s)", path, code);
		return false;
	}
	fseek(f, 0, SEEK_SET);
	uint8_t *data = malloc(ROM_SIZE);
	if (!data) { fclose(f); snprintf(msg, msglen, "Not enough memory to read %s", path); return false; }
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
#if !defined(__3DS__)
			/* (other games' ROMs beside it lend a run their areas,
			 * docs/multirom/: the 3DS's own start reads them where
			 * its heap has room, start_3ds.c; a browser holds the files
			 * chosen on its page, BN5's beside BN6's in its storage) */
			xrom_find_beside();
#endif
			return true;
		}
	}
	free(data);
	snprintf(msg, msglen, "%s is not a supported ROM (SHA-1 %.12s...)", path, hex);
	return false;
}

bool rom_find_close;

/* ---- Extra ROMs (docs/multirom/) ---- */

/* Battle Network 5's net areas (docs/multirom/): their maps' descriptors
 * lie one after another from group 0x90's (0x90:0-1 ACDC Area 1-2, then the
 * next groups' as 0x90:2 on), named as BN5's own map labels name them.
 * Each takes over a map whose group keeps the descriptor's palette: the
 * Undernet's, the Graveyard's and the Underground's set their own over it.
 * Their battles are BN5's own maps' by its game's numbers (0x91 Oran Area,
 * 0x92 SciLab, 0x93 End Area, 0x94 the Undernet and Nebula Area, 0x86 the
 * rest of each), an act's layers through the area's maps in BN5's order
 * (docs/rom/bn5-battles.md, BN5 guest battles), fought in BN6's engine (issue
 * #82). Their guardians may be BN5's own Navis where its net maps' records
 * have them roam (xguard: docs/game/bosses/bn5-navis.md); Nebula Area's and
 * the Undernet's acts draw from both nets' Navis by band (run.h). */
static const NetAreaDef bn5_areas[] = {
	{ 0x90, 0, 0x10040, 0x0030, false, 0x13, 0, 0, 0, { { 0x90, 1 } }, .xrom = 1 + XROM_BN5_COLONEL_US, .over = { 0x90, 0 }, .like = 0,
		.name = "ACDC Area", .short_name = "ACDC Area", .motto = "The net of Lan's old hometown", .xsong = 0x13, .xnavi = 60, .xnavi2 = 62, .xbg = 8, .xlooks = XLOOK_CUBE,
		.xbattles = { { 0x90, 0 }, { 0x90, 1 }, { 0x86, 0 } }, .xguard = { BN5_NAVI_KNIGHTMAN } },   /* ACDC Area 1: cyan platforms framed by rims, green walkways and their pale joins; ACDC Area 2 in the same tiles and colours */
	{ 0x90, 5, 0x8000, 0x8000, false, 0x13, 0, 0, 0, { { 0x90, 4 }, { 0x90, 6 } }, .pad_rooms = 64, .apart = NET_APART_PLATFORMS, .skip_styles = 0x2006,
		.xrom = 1 + XROM_BN5_COLONEL_US, .over = { 0x94, 1 }, .like = 2,
		.name = "SciLab Area", .short_name = "SciLab", .motto = "The net Dad's lab once ran", .xsong = 0x13, .xnavi = 60, .xnavi2 = 62, .xbg = 10, .xlooks = XLOOK_CUBE,
		.xbattles = { { 0x92, 0 }, { 0x92, 1 }, { 0x86, 2 } }, .xguard = { BN5_NAVI_TOMAHAWKMAN } },   /* SciLab 2, 1 and 4: green circuits in cyan lips and pale octagon hubs, ears at their corners, told by shape (TILES_BY_SHAPE: its paths' panels turn green and cyan by turns); its maps hold no platform bigger than a 3x3 hub, so every room up to 64 panels and the arena are drawn as its hubs (pad_rooms), their rims and ears framing them; its orange exit paths left out off the hubs (SKIP_OFF_PADS | hue buckets 1-2) */
	{ 0x90, 7, 0x1001002, 0, false, 0x13, 0, 0, 0, { { 0x90, 8 }, { 0x90, 9 } }, .xrom = 1 + XROM_BN5_COLONEL_US, .over = { 0x91, 1 }, .like = 1,
		.name = "End Area", .short_name = "End Area", .motto = "Highways to the old net's end", .xsong = 0x13, .xnavi = 60, .xnavi2 = 62, .xbg = 15, .xlooks = XLOOK_CUBE,
		.xbattles = { { 0x93, 0 }, { 0x86, 3 }, { 0x93, 1 } }, .xguard = { BN5_NAVI_NUMBERMAN, BN5_NAVI_TOADMAN } },   /* End Area 1-3: one-wide yellow roads in grey, rings round posts and grey pads; no floor two wide but the pads, so every room is drawn as one and the way kept one wide (TILES_ONE_WIDE, issue #164) */
	{ 0x90, 13, 0x8980, 0x8440, false, 0x13, 0, 0, 0, { { 0x90, 14 } }, .xrom = 1 + XROM_BN5_COLONEL_US, .over = { 0x92, 0 }, .like = 4,
		.name = "Nebula Area", .short_name = "Nebula", .motto = "Where Nebula once ruled", .xsong = 0x14, .xnavi = 60, .xbg = 25, .xlooks = XLOOK_DARK_WALL | XLOOK_DARK_HOLE,
		.xbattles = { { 0x86, 6 }, { 0x94, 3 }, { 0x86, 7 } } },   /* Nebula Area 2 and 4: small purple platforms with an emblem, on long cobbled paths whose stones turn purple and teal by turns: told by shape among its hues (purple, teal, magenta), not its pale arrows */
	{ 0x90, 3, 0x20008000, 0x8000, false, 0x13, 0, 0, 0, { { 0x90, 2 } }, .pad_rooms = 64, .apart = NET_APART_PLATFORMS, .skip_styles = 0x21C1, .recolour = 0x524231,
		.xrom = 1 + XROM_BN5_COLONEL_US, .over = { 0x92, 1 }, .like = 3,
		.name = "Oran Area", .short_name = "Oran Area", .motto = "The net of Oran Isle's mines", .xsong = 0x13, .xnavi = 60, .xbg = 9,
		.xbattles = { { 0x91, 0 }, { 0x91, 1 }, { 0x86, 1 } }, .xguard = { BN5_NAVI_SHADOWMAN } },   /* Oran Area 2 and 1: olive circuits in grey lips and orange octagon hubs, ears at their corners, told by shape; Oran Area 1's raised cream platforms and green walkways in its hubs' and circuits' shades (banks 3 and 4 as 1 and 2, its mint ones as 2: recolour); every room up to 64 panels and the arena drawn as its hubs (pad_rooms); off the hubs, its brown planks and its blue and purple arrow panels left out (SKIP_OFF_PADS | hue buckets 0, 6-8): one walkway look; its maps' panels laid whole (TILES_WHOLE): its circuits ran in pieces of other paths tile by tile */
	{ 0x90, 11, 0x20008000, 0x8000, false, 0x14, 0, 0, 0, { { 0x90, 12 } }, .apart = NET_APART_PLATFORMS, .looks = 1u << LOOK_STATUE | 1u << LOOK_BRAZIER, .emblem = 0x7D37,
		.recolour = 0x21, .xrom = 1 + XROM_BN5_COLONEL_US, .over = { 0x92, 0 }, .like = 5,
		.name = "Undernet", .short_name = "Undernet", .motto = "The old depths, still burning", .xsong = 0x14, .xnavi = 60, .xbg = 24,
		.xbattles = { { 0x94, 0 }, { 0x94, 1 }, { 0x86, 5 } }, .xguard = { BN5_NAVI_COLONEL } },   /* Undernet 2 and 3: mauve stone plates with a radial seam and lava at their corners, on cobbled paths, told by shape; their raised courts' pale stone and purple slime in the plates' shades (bank 2 as bank 1: recolour); its plates' purple cross its emblem; BN6's statue and braziers, which its number doors count (docs/world/level-design/); its maps' panels laid whole (TILES_WHOLE): its plates' corners with their lava whole */
	/* (the Liberation Missions' nine maps, group 0x86, mission n's 0x86:n-1: a
	 * dark layer's board drawn in its mission's framed panels, backdrop and
	 * colours, BN5's own panel looks over them, docs/game/liberation/building/mission-look.md, Each
	 * mission's own look; never a run's dressing, its battles BN6's) */
#define LIB_MAP(n) { 0x86, n, 0xC000, 0x8000, false, 0x13, 0, 0, 0, { { 0 } }, .xrom = 1 + XROM_BN5_COLONEL_US, .over = { 0x90, 0 }, .like = 0, \
		.name = "Dark area", .short_name = "Dark area", .motto = "", .xsong = 0x13, .xnavi = 60, .held = true }
	LIB_MAP(0), LIB_MAP(1), LIB_MAP(2), LIB_MAP(3), LIB_MAP(4), LIB_MAP(5), LIB_MAP(6), LIB_MAP(7), LIB_MAP(8),
#undef LIB_MAP
};

/* Battle Network 4 Red Sun's five large net areas (docs/rom/bn4.md: its
 * groups 0x90-0x94, each a group of its own, not run on as BN5's are),
 * each dressing the BN6 area it is like in runs by the seed (run_dress;
 * admitted by the owner's call of 5 November 2026, docs/multirom/bn4.md):
 * --net-biome xN draws one, BN4's areas numbered after BN5's (x15 ACDC
 * Area to x19 the Undernet).
 * Their battles are BN6's (no xbattles: BN4's records are its own,
 * docs/rom/bn4.md, Battle records); their guardians, in half the runs, the
 * Navis its net's records set in each (docs/game/bosses/bn4-navis.md), else
 * BN6's; their bystanders BN6's; their battle backgrounds the map's own (BN4_MAP_BACKDROPS),
 * their themes BN4's net and Undernet songs (BN4_SONG_NET, BN4_SONG_UNDERNET). */
static const NetAreaDef bn4_areas[] = {
	{ BN4_GROUP_ACDC, 2, 0x2008000, 0x8000, false, 0x13, 0, 0, 0, { { BN4_GROUP_ACDC, 0 }, { BN4_GROUP_ACDC, 1 } }, .pad_rooms = 64, .apart = NET_APART_PLATFORMS,
		.skip_styles = 0x90C5, .xrom = 1 + XROM_BN4_RED_SUN_US, .over = { 0x90, 0 }, .like = 0, .xguard = { BN4_V1_GUTSMAN, BN4_V1_AQUAMAN },
		.name = "ACDC Area", .short_name = "ACDC Area", .motto = "The net of the old ACDC Town", .xsong = 0x12, .xbg = 4 },   /* ACDC Area 3, 1 and 2: white walkways in green rims, rounded at their turns, and framed squares of four by four, told by shape; its squares its pads (TILES_SQUARE_PADS), so every room up to 64 panels and the arena drawn as one, their frames framing them; its cyan roads and its hubs, ringed in azure and yellow round red, lend no tiles (SKIP_ANY_PIXEL | SKIP_QUARTER | hue buckets 0, 2, 6 and 7: a quarter of a panel's top in them, where its squares' teal frame lines are a few pixels of the roads' hue): a second walkway look, laid tile by tile among the white */
	{ BN4_GROUP_TOWN, 2, 0xE300A000, 0x8000, false, 0x13, 0, 0, 0, { { BN4_GROUP_TOWN, 0 } }, .skip_styles = 0x008C,
		.xrom = 1 + XROM_BN4_RED_SUN_US, .over = { 0x91, 1 }, .like = 1, .xguard = { BN4_V1_FIREMAN, BN4_V1_ROLL },
		.name = "Town Area", .short_name = "Town Area", .motto = "The net between the towns", .xsong = 0x12, .xbg = 5 },   /* Town Area 3 and 1: pale cyan walkways a panel wide in blue rims, told by shape, and Town Area 3's blue-framed squares, its pads (TILES_SQUARE_PADS); its maps lay no floor wider than a walkway but those squares (Town Area 1 a lattice of walkways round holes, whose hole rims a field took), so every room is drawn as one and the way kept one wide (TILES_ONE_WIDE); its yellow roads lend no tiles (hue buckets 2, 3 and 7 in their middles), and Town Area 3's lower blue paths, which pass under its roads inside their walls' rings, are no floor of its level (TILES_OUTER_FLOOR); Town Area 3 lends its squares alone (TILES_PADS_ALONE: its other floor is roads, hubs and those paths, whose pieces the walkways took at their ends and turns), so the walkways are Town Area 1's, in colours Town Area 3's squares never show (TILES_MORE_COLOURS); its maps' panels laid whole (TILES_WHOLE): a square on its blue slab, both its marks */
	{ BN4_GROUP_PARK, 0, 0x3802C001, 0x1000, false, 0x13, 0, 0, 0, { { 0 } }, .apart = NET_APART_PLATFORMS, .skip_styles = 0x8006,
		.xrom = 1 + XROM_BN4_RED_SUN_US, .over = { 0x92, 1 }, .like = 3, .xguard = { BN4_V1_WOODMAN },
		.name = "Park Area", .short_name = "Park Area", .motto = "The net under the park", .xsong = 0x12, .xbg = 6 },   /* Park Area 1: pink walkways in blue rims, peach framed platforms, told by shape, the pale pink walkway floor however wide (TILES_WALK_HUE: grey, the peach hue bucket 0); its orange and yellow paths lend its walkways no tiles (SKIP_ANY_PIXEL | hue buckets 1-2; plain floor in the map's data, docs/rom/bn4.md), and are found and learned apart as its roads, which a layer's way to its exit is drawn in (docs/world/level-design/roads.md); its other maps' looks, mixed in, doubled the seams; its one platform touches no walkway (a ramp climbs to it), so its platforms are drawn apart, whole, a walkway ending at their rim (the owner's call, 13 November 2026); that platform read as one where its overpass crosses over it and the walls ring its far half (TILES_UNDER, TILES_INNER_WALLS), and its panels laid whole (TILES_WHOLE): its frame, its corners' marks and the wall on its upper edges; its small rooms platforms as the large (TILES_NO_PAD_LOOK: its one pad is its Navi's round hub, whose look a square room took in pieces) */
	{ BN4_GROUP_COUNTRIES, 2, 0x2800C006, 0x0008, false, 0x13, 0, 0, 0, { { 0 } }, .apart = NET_APART_PLATFORMS,
		.skip_styles = 0x8040, .xrom = 1 + XROM_BN4_RED_SUN_US, .over = { 0x94, 1 }, .like = 2, .xguard = { BN4_V1_THUNDERMAN, BN4_V1_WINDMAN },
		.name = "Net countries", .short_name = "Countries", .motto = "The nets of far countries", .xsong = 0x12, .xbg = 0x14 },   /* the countries' third map: green floor in red and yellow rims, cream framed platforms drawn apart, told by shape, the green walkway floor however wide (TILES_WALK_HUE: hue bucket 3, the cream and its frames 1 and 2), where its middles stood in the platforms' as green blotches; its cyan arrows lend no tiles (SKIP_ANY_PIXEL | hue bucket 6); the four countries are four looks, so one is learned (which country is which map is not traced); its maps' panels laid whole (TILES_WHOLE), its small rooms platforms as the large, not pads (TILES_NO_PAD_LOOK): every room its framed cream platform */
	{ BN4_GROUP_UNDERNET, 4, 0x20008000, 0x8000, false, 0x14, 0, 0, 0, { { BN4_GROUP_UNDERNET, 2 } }, .apart = NET_APART_PLATFORMS,
		.joint_hues = 0x1000, .emblem = 0x75C0, .xrom = 1 + XROM_BN4_RED_SUN_US, .over = { 0x92, 0 }, .like = 5, .xguard = { BN4_V1_SEARCHMAN, BN4_V1_JUNKMAN },
		.name = "Undernet", .short_name = "Undernet", .motto = "Older depths, darker still", .xsong = 0x13, .xbg = 8 },   /* the Undernet 5 and 3: orange cushions a panel each on paths a panel wide, grey square plates framed in steel with orange corners, drawn apart, told by shape; its steel joints, grey among the cushions, in a look of their own at the paths' ends, turns and crossings (JOINT_GREY, 0x1000, src/map/tiles.h); its plates plain, as the Undernet 5 lays them, the Undernet 3's blue cross its emblem (its middle's blue, 0x75C0, shown nowhere else); Black Earth (0x94:6-7) not drawn; its maps' panels laid whole (TILES_WHOLE): its cushions' paths without other paths' scraps at their joints */
};

static const XRomLayout xlayouts[XROM_COUNT] = {
	/* (tables found by their structure beside BN6's own, docs/rom/) */
	[XROM_BN5_COLONEL_US] = { "Mega Man Battle Network 5: Team Colonel (USA)", "5f472f78d8de2df01d5039e045c043cb40969a39", "BRKE",
		0x0331B4u, 0x0331CCu, 0x033FACu, 0x033FC4u, 6, 21, bn5_areas, (int)(sizeof bn5_areas / sizeof *bn5_areas), 0x155BF4u,
		0x03272Cu, { 0x15, 0x16 }, 0x08C5E8u, 0x08C9DCu, 0x08C39Cu, 0x031414u, 0x031468u, "BN5", 0x45, 0x1C, .file = "bn5c.gba",
		.win_songs = { BN5_SONG_VICTORY, BN5_SONG_FANFARE } },
	/* (its tables as docs/rom/bn4.md found them; its battle backgrounds and
	 * map routines named in bn4.h, as their records are 9 words to BN5's 7) */
	[XROM_BN4_RED_SUN_US] = { .name = "Mega Man Battle Network 4: Red Sun (USA)", .sha1 = "a97e96a7da03abd70f7953328e511b9fa29179f1", .code = "B4WE",
		.map_table_rw = 0x028304u, .map_table = 0x02831Cu, .coord_table_rw = 0x028F04u, .coord_table = 0x028F1Cu,
		.rw_groups = 6, .net_groups = 21, .song_table = 0x155FF8u, .sprite_lists = 0x02793Cu,
		/* (its battle themes, copied as a run's music starts, as a game's that
		 * lends areas: src/audio/xsong.c; its prelude BN6's, BN4 has none of
		 * its own named) */
		.battle_songs = { BN4_SONG_BATTLE, BN4_SONG_BOSS },
		.areas = bn4_areas, .nareas = (int)(sizeof bn4_areas / sizeof *bn4_areas),
		.battle_bgs = 0x08563Cu, .battle_bg_anims = 0x085A50u, .battle_bg_scroll = 0x085488u, .bg_words = 9,
		.map_enters = 0x026634u, .map_anims = 0x026688u, .tag = "BN4", .face = BN4_MUGSHOT_FACE, .file = "bn4rs.gba",
		.kit_keys_played = true },
};

XRom XR[XROM_COUNT];
int xroms_most = XROMS_MOST;

void xroms_dev(const char *spec) {
	const char *at = strstr(spec, "xroms=");
	if (at && (at == spec || at[-1] == ',')) xroms_most = atoi(at + 6);
}
bool xrom_research;
static int reviewed = -1;   /* (the area a dev tool reviews, XAREA_ALL every one; -1 none) */

/* The game whose area `area` is (an XRomId), its index among that game's in `*k`; -1 for BN6's or none */
static int xrom_of_area(int area, int *k) {
	*k = area - NET_AREAS;
	for (int i = 0; i < XROM_COUNT && area >= NET_AREAS && *k < XAREAS_MAX; ++i) {
		if (*k < xlayouts[i].nareas) return i;
		*k -= xlayouts[i].nareas;
	}
	return -1;
}

const NetAreaDef *net_area_def(int area) {
	if (area < 0) return NULL;
	if (area < NET_AREAS) return R.layout ? &R.layout->net_area[area] : NULL;
	int k, i = xrom_of_area(area, &k);
	if (i < 0 || !XR[i].data) return NULL;
	/* (a game read for research: its areas only as a dev tool reviews them) */
	if (xlayouts[i].research && reviewed != XAREA_ALL && reviewed != area) return NULL;
	return &xlayouts[i].areas[k];
}

void xarea_review(int area) {
	int k, i = xrom_of_area(area, &k);
	reviewed = area;
	if (area == XAREA_ALL || (i >= 0 && xlayouts[i].research)) xrom_research = true;
}

bool xrom_offered(int xrom) { return xrom >= 0 && xrom < XROM_COUNT && (!xlayouts[xrom].research || xrom_research); }

const XRomLayout *xrom_layout(int xrom) { return xrom >= 0 && xrom < XROM_COUNT ? &xlayouts[xrom] : NULL; }

#ifdef __EMSCRIPTEN__
/* The browser's page (web/play/app.js) asks before it takes a file: 1
 * where the extra ROM of header game code `code` is offered */
int cw_rom_offered(const char *code);
EMSCRIPTEN_KEEPALIVE int cw_rom_offered(const char *code) { return code && strlen(code) == 4 && xrom_offered(xrom_of_code(code)); }
/* ... and with ?dev=showbn4, the research ROMs offered for review, as --dev showbn4 */
void cw_rom_research(void);
EMSCRIPTEN_KEEPALIVE void cw_rom_research(void) { xrom_research = true; }
#endif

bool xrom_load(const char *path) {
	FILE *f = fopen(path, "rb");
	if (!f) return false;
	char code[5] = "";
	int id = -1;
	if (fseek(f, 0, SEEK_END) == 0 && ftell(f) == ROM_SIZE && fseek(f, 0xAC, SEEK_SET) == 0 && fread(code, 1, 4, f) == 4)
		for (int i = 0; i < XROM_COUNT; ++i)
			if (!memcmp(code, xlayouts[i].code, 4) && !XR[i].data && xrom_offered(i)) id = i;
	uint8_t *data = id < 0 ? NULL : malloc(ROM_SIZE);
	bool ok = data && fseek(f, 0, SEEK_SET) == 0 && fread(data, 1, ROM_SIZE, f) == ROM_SIZE;
	fclose(f);
	char hex[41] = "";
	if (ok) sha1_hex(data, ROM_SIZE, hex);
	if (!ok || strcmp(hex, xlayouts[id].sha1)) { free(data); return false; }
	XR[id] = (XRom){ data, &xlayouts[id], "" };
	snprintf(XR[id].path, sizeof XR[id].path, "%s", path);
	return true;
}

int xrom_of_code(const char *code) {
	for (int i = 0; i < XROM_COUNT; ++i)
		if (!memcmp(code, xlayouts[i].code, 4)) return i;
	return -1;
}

int xrom_find(const char *dir) {
	DIR *d = opendir(dir);
	if (d) {
		struct dirent *e;
		while ((e = readdir(d))) {
			size_t n = strlen(e->d_name);
			if (n < 4 || strcasecmp(e->d_name + n - 4, ".gba")) continue;
			char path[1024];
			snprintf(path, sizeof path, "%s/%s", dir, e->d_name);
			xrom_load(path);
		}
		closedir(d);
	}
	int have = 0;
	for (int i = 0; i < XROM_COUNT; ++i) have += XR[i].data != NULL;
	return have;
}

int xrom_find_beside(void) {
	char dir[1024];
	snprintf(dir, sizeof dir, "%s", R.path);
	char *slash = strrchr(dir, '/'), *back = strrchr(dir, '\\');
	if (back > slash) slash = back;
	if (slash) *slash = 0;
	else snprintf(dir, sizeof dir, ".");
	return xrom_find(dir);
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
	rom_find_close = !found && last[0];
	return found;
}
