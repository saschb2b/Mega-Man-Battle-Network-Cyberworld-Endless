#include "gba_world.h"

#include <stdbool.h>

static unsigned rd16(const uint8_t *p, unsigned at) { return (unsigned)p[at] | (unsigned)p[at + 1] << 8; }

/* A palette entry's 15 bits after the frame's fade, where `fade` says the
 * layer takes it: BLDCNT's mode 2 toward white, 3 toward black, by BLDY
 * sixteenths */
static unsigned faded(const GbaWorld *g, unsigned index, int fade, unsigned evy) {
	unsigned c = rd16(g->pal, index * 2u), out = 0;
	for (unsigned s = 0; s < 15; s += 5) {
		unsigned v = c >> s & 31u;
		if (fade == 2) v += (31u - v) * evy / 16u;
		else if (fade == 3) v -= v * evy / 16u;
		out |= v << s;
	}
	return out;
}

/* `over` on `under` as BLDALPHA mixes them (sixteenths of each, a channel
 * no more than 31) */
static unsigned mixed(unsigned over, unsigned under, unsigned eva, unsigned evb) {
	unsigned out = 0;
	for (unsigned s = 0; s < 15; s += 5) {
		unsigned v = ((over >> s & 31u) * eva + (under >> s & 31u) * evb) / 16u;
		out |= (v > 31u ? 31u : v) << s;
	}
	return out;
}

/* 15 bits as mGBA shows them (5 bits a channel, its top bits again below) */
static uint32_t shown(unsigned c) {
	uint32_t out = 0xFF000000u;
	for (unsigned s = 0; s < 3; ++s) {
		unsigned v = c >> (5u * s) & 31u;
		out |= (v << 3 | v >> 2) << (8u * s);
	}
	return out;
}

typedef struct { unsigned prio, chars, screen, size, hofs, vofs; bool on, wide; int fade; bool over, under; } Bg;

static Bg bg_read(const GbaWorld *g, unsigned b) {
	unsigned cnt = rd16(g->io, 0x08u + 2u * b), bld = rd16(g->io, 0x50);
	return (Bg){ cnt & 3u, (cnt >> 2 & 3u) * 0x4000u, (cnt >> 8 & 31u) * 0x800u, cnt >> 14, rd16(g->io, 0x10u + 4u * b) & 0x1FFu,
		rd16(g->io, 0x12u + 4u * b) & 0x1FFu, (rd16(g->io, 0) >> (8u + b) & 1u) != 0, (cnt & 0x80u) != 0, (bld >> b & 1u) && (bld >> 6 & 3u) > 1 ? (int)(bld >> 6 & 3u) : 0, (bld >> 6 & 3u) == 1 && (bld >> b & 1u), (bld >> (8u + b) & 1u) != 0 };
}

/* The palette index of screen entry `e`'s tile at (tx, ty) in it, 0 clear */
static unsigned tile_pixel(const GbaWorld *g, const Bg *b, unsigned e, unsigned tx, unsigned ty) {
	if (e & 0x400u) tx = 7u - tx;
	if (e & 0x800u) ty = 7u - ty;
	if (b->wide) return g->vram[(b->chars + (e & 0x3FFu) * 64u + ty * 8u + tx) & 0xFFFFu];
	unsigned byte = g->vram[(b->chars + (e & 0x3FFu) * 32u + ty * 4u + (tx >> 1)) & 0xFFFFu];
	unsigned n = (tx & 1u) ? byte >> 4 : byte & 15u;
	return n ? (e >> 12) * 16u + n : 0u;
}

/* A map background's pixel at the frame's (x, y), from the whole tile map */
static unsigned map_pixel(const GbaWorld *g, const Bg *b, int layer, int x, int y) {
	int px = x + g->ox, py = y + g->oy;
	if (!b->on || !g->map[layer] || px < 0 || py < 0 || px >= g->tw * 8 || py >= g->th * 8) return 0;
	return tile_pixel(g, b, g->map[layer][(py >> 3) * g->tw + (px >> 3)], (unsigned)px & 7u, (unsigned)py & 7u);
}

/* ... and one of a background's own screen map, wrapped */
static unsigned screen_pixel(const GbaWorld *g, const Bg *b, int x, int y) {
	if (!b->on) return 0;
	unsigned px = ((unsigned)x + b->hofs) & ((b->size & 1u) ? 511u : 255u), py = ((unsigned)y + b->vofs) & ((b->size & 2u) ? 511u : 255u);
	unsigned block = (px >> 8) + (py >> 8) * ((b->size & 1u) ? 2u : 1u);
	return tile_pixel(g, b, rd16(g->vram, (b->screen + block * 0x800u + ((py >> 3 & 31u) * 32u + (px >> 3 & 31u)) * 2u) & 0xFFFEu), px & 7u, py & 7u);
}

void gba_world(const GbaWorld *g, int x0, int y0, int w, int h, uint32_t *map, uint32_t *sky) {
	Bg bg[4];
	for (unsigned b = 0; b < 4; ++b) bg[b] = bg_read(g, b);
	unsigned bld = rd16(g->io, 0x50), alpha = rd16(g->io, 0x52), evy = rd16(g->io, 0x54) & 31u;
	unsigned eva = alpha & 31u, evb = alpha >> 8 & 31u;
	if (evy > 16) evy = 16;
	if (eva > 16) eva = 16;
	if (evb > 16) evb = 16;
	int back_fade = (bld >> 5 & 1u) && (bld >> 6 & 3u) > 1 ? (int)(bld >> 6 & 3u) : 0;
	bool back_under = (bld >> 13 & 1u) != 0;
	/* (of the two, the one the game shows in front first) */
	int first = bg[2].prio < bg[1].prio ? 2 : 1, second = 3 - first;
	for (int j = 0; j < h; ++j)
		for (int i = 0; i < w; ++i) {
			int x = x0 + i, y = y0 + j, at = j * w + i;
			unsigned n3 = screen_pixel(g, &bg[3], x, y);
			unsigned behind = n3 ? faded(g, n3, bg[3].fade, evy) : faded(g, 0, back_fade, evy);
			bool behind_under = n3 ? bg[3].under : back_under;
			sky[at] = shown(behind);
			map[at] = 0;
			unsigned n2 = map_pixel(g, &bg[second], second - 1, x, y), n1 = map_pixel(g, &bg[first], first - 1, x, y);
			if (!n1 && !n2) continue;
			/* (the lower of the two over what is behind, then the upper over that: each mixed where BLDCNT pairs them) */
			unsigned c = behind;
			bool c_under = behind_under;
			if (n2) {
				unsigned top = faded(g, n2, bg[second].fade, evy);
				c = bg[second].over && c_under ? mixed(top, c, eva, evb) : top;
				c_under = bg[second].under;
			}
			if (n1) {
				unsigned top = faded(g, n1, bg[first].fade, evy);
				c = bg[first].over && c_under ? mixed(top, c, eva, evb) : top;
			}
			map[at] = shown(c);
		}
}
