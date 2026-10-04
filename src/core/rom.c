#include "rom.h"

#include "net.h"   /* LOOK_* */

#include <dirent.h>
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

/* Entry `index` of the text archive at offset `archive` of ROM `data`
 * (ROM_SIZE bytes), in ASCII: BN5 spells its text as BN6 does. */
static void text_of(const uint8_t *data, uint32_t archive, int index, char *out, size_t outlen) {
	size_t o = 0;
	uint32_t at = archive + 2u * (uint32_t)index;
	if (at + 2 <= ROM_SIZE) {
		uint32_t p = archive + (uint32_t)(data[at] | data[at + 1] << 8);
		for (uint32_t i = 0; i < 64 && p + i < ROM_SIZE; ++i) {
			uint8_t c = data[p + i];
			if (c >= 0xE5) break;
			for (const char *g = glyph(c); *g && o + 1 < outlen; ++g) out[o++] = *g;
		}
	}
	out[o] = 0;
}

void rom_text(uint32_t archive, int index, char *out, size_t outlen) { text_of(R.data, archive, index, out, outlen); }

void xrom_text(int xrom, uint32_t archive, int index, char *out, size_t outlen) {
	if (xrom < 0 || xrom >= XROM_COUNT || !XR[xrom].data) { if (outlen) out[0] = 0; return; }
	text_of(XR[xrom].data, archive, index, out, outlen);
}

void rom_desc(uint32_t archive, int index, char *out, size_t outlen) {
	uint32_t p = archive + rom_u16(archive + 2 * index);
	size_t o = 0;
	for (int i = 0; i < 96 && o + 1 < outlen;) {
		uint8_t c = R.data[p + i];
		if (c == 0xE8) { i += 4; continue; }   /* ts_msg_open_quick_ext */
		if (c == 0xF1) { i += 3; continue; }   /* ts_text_speed */
		if (c == 0xE9) { out[o++] = ' '; ++i; continue; }   /* a new line */
		if (c >= 0xE5) break;                  /* ts_key_wait, ts_end */
		for (const char *g = glyph(c); *g && o + 1 < outlen; ++g) out[o++] = *g;
		++i;
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
#if !defined(__3DS__) && !defined(__EMSCRIPTEN__)
			/* (other games' ROMs beside it lend a run their areas,
			 * docs/MULTIROM.md: a 3DS has no memory for another, and a
			 * browser holds only the file chosen) */
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

/* ---- Extra ROMs (docs/MULTIROM.md) ---- */

/* Battle Network 5's net areas (docs/MULTIROM.md): their maps' descriptors
 * lie one after another from group 0x90's (0x90:0-1 ACDC Area 1-2, then the
 * next groups' as 0x90:2 on), named as BN5's own map labels name them.
 * Each takes over a map whose group keeps the descriptor's palette: the
 * Undernet's, the Graveyard's and the Underground's set their own over it.
 * Their battles are BN5's own maps' by its game's numbers (0x91 Oran Area,
 * 0x92 SciLab, 0x93 End Area, 0x94 the Undernet and Nebula Area, 0x86 the
 * rest of each), an act's layers through the area's maps in BN5's order
 * (docs/ROM_DATA.md, BN5 guest battles). */
static const NetAreaDef bn5_areas[] = {
	{ 0x90, 0, 0x10040, 0x0030, false, 0x13, 0, 0, 0, { { 0x90, 1 } }, .xrom = 1 + XROM_BN5_COLONEL_US, .over = { 0x90, 0 }, .like = 0,
		.name = "ACDC Area", .short_name = "ACDC Area", .motto = "The net of Lan's old hometown", .xsong = 0x13, .xnavi = 60, .xbg = 8,
		.xbattles = { { 0x90, 0 }, { 0x90, 1 }, { 0x86, 0 } } },   /* ACDC Area 1: cyan platforms framed by rims, green walkways and their pale joins; ACDC Area 2 in the same tiles and colours */
	{ 0x90, 4, 0x9020, 0x8050, false, 0x13, 0, 0, 0, { { 0x90, 5 }, { 0x90, 6 } }, .xrom = 1 + XROM_BN5_COLONEL_US, .over = { 0x94, 1 }, .like = 2,
		.name = "SciLab Area", .short_name = "SciLab", .motto = "The net Dad's lab once ran", .xsong = 0x13, .xnavi = 60, .xbg = 10, .held = true,
		.xbattles = { { 0x92, 0 }, { 0x92, 1 }, { 0x86, 2 } } },   /* SciLab 1, 2 and 4: its circuit paths' panels turn green and cyan by turns, so by hue half of them were platform floor and its rooms came out in pale blotches: told by shape (TILES_BY_SHAPE), among its own hues (pale green and grey platforms, green and cyan paths). Held out of runs: its maps hold no platform bigger than a 3x3 pad, and Sky Area's rooms drawn in it meet their pale middles with no frame */
	{ 0x90, 7, 0x1002, 0, false, 0x13, 0, 0, 0, { { 0x90, 8 }, { 0x90, 9 } }, .xrom = 1 + XROM_BN5_COLONEL_US, .over = { 0x91, 1 }, .like = 1,
		.name = "End Area", .short_name = "End Area", .motto = "Highways to the old net's end", .xsong = 0x13, .xnavi = 60, .xbg = 15,
		.xbattles = { { 0x93, 0 }, { 0x86, 3 }, { 0x93, 1 } } },
	{ 0x90, 13, 0x8980, 0x8440, false, 0x13, 0, 0, 0, { { 0x90, 14 } }, .xrom = 1 + XROM_BN5_COLONEL_US, .over = { 0x92, 0 }, .like = 4,
		.name = "Nebula Area", .short_name = "Nebula", .motto = "Where Nebula once ruled", .xsong = 0x14, .xnavi = 60, .xbg = 25,
		.xbattles = { { 0x86, 6 }, { 0x94, 3 }, { 0x86, 7 } } },   /* Nebula Area 2 and 4: small purple platforms with an emblem, on long cobbled paths whose stones turn purple and teal by turns: told by shape among its hues (purple, teal, magenta), not its pale arrows */
};

static const XRomLayout xlayouts[XROM_COUNT] = {
	/* (tables found by their structure beside BN6's own, docs/ROM_DATA.md) */
	[XROM_BN5_COLONEL_US] = { "Mega Man Battle Network 5: Team Colonel (USA)", "5f472f78d8de2df01d5039e045c043cb40969a39", "BRKE",
		0x0331B4u, 0x0331CCu, 0x033FACu, 0x033FC4u, 6, 21, bn5_areas, (int)(sizeof bn5_areas / sizeof *bn5_areas), 0x155BF4u,
		0x03272Cu, { 0x15, 0x16 }, 0x08C5E8u, 0x08C9DCu, 0x08C39Cu, 0x031414u, 0x031468u, "BN5" },
};

const NetAreaDef *net_area_def(int area) {
	if (area < 0) return NULL;
	if (area < NET_AREAS) return R.layout ? &R.layout->net_area[area] : NULL;
	int k = area - NET_AREAS;
	for (int i = 0; i < XROM_COUNT && k < XAREAS_MAX; ++i) {
		if (k < xlayouts[i].nareas) return XR[i].data ? &xlayouts[i].areas[k] : NULL;
		k -= xlayouts[i].nareas;
	}
	return NULL;
}

XRom XR[XROM_COUNT];

static void xrom_load(const char *path) {
	FILE *f = fopen(path, "rb");
	if (!f) return;
	char code[5] = "";
	int id = -1;
	if (fseek(f, 0, SEEK_END) == 0 && ftell(f) == ROM_SIZE && fseek(f, 0xAC, SEEK_SET) == 0 && fread(code, 1, 4, f) == 4)
		for (int i = 0; i < XROM_COUNT; ++i)
			if (!memcmp(code, xlayouts[i].code, 4) && !XR[i].data) id = i;
	uint8_t *data = id < 0 ? NULL : malloc(ROM_SIZE);
	bool ok = data && fseek(f, 0, SEEK_SET) == 0 && fread(data, 1, ROM_SIZE, f) == ROM_SIZE;
	fclose(f);
	char hex[41] = "";
	if (ok) sha1_hex(data, ROM_SIZE, hex);
	if (!ok || strcmp(hex, xlayouts[id].sha1)) { free(data); return; }
	XR[id] = (XRom){ data, &xlayouts[id], "" };
	snprintf(XR[id].path, sizeof XR[id].path, "%s", path);
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
