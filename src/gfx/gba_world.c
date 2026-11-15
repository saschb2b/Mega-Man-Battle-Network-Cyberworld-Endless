#include "gba_world.h"

#include <stdbool.h>

#include "gba_layers.h"
#include "gba_tiles.h"

/* A palette entry's 15 bits after the frame's fade, where `fade` says the
 * layer takes it: BLDCNT's mode 2 toward white, 3 toward black, by BLDY
 * sixteenths */
static unsigned faded(const GbaWorld *g, unsigned index, int fade, unsigned evy) {
	unsigned c = gba_rd16(g->pal, index * 2u), out = 0;
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

uint32_t gba_shown(unsigned c) {
	uint32_t out = 0xFF000000u;
	for (unsigned s = 0; s < 3; ++s) {
		unsigned v = c >> (5u * s) & 31u;
		out |= (v << 3 | v >> 2) << (8u * s);
	}
	return out;
}

static uint32_t shown_by(const GbaWorld *g, unsigned c) { return g->lut ? g->lut[c & 0x7FFFu] : gba_shown(c); }

/* A background as the frame shows it: its tiles, how BLDCNT fades or
 * mixes it, and its palette's 15 bits with the fade on them */
typedef struct { GbaBg t; int fade; bool over, under; uint16_t col[256]; } Bg;

static void bg_read(const GbaWorld *g, unsigned b, unsigned evy, Bg *out) {
	unsigned bld = gba_rd16(g->io, 0x50);
	out->t = gba_bg(g->io, b);
	out->fade = (bld >> b & 1u) && (bld >> 6 & 3u) > 1 ? (int)(bld >> 6 & 3u) : 0;
	out->over = (bld >> 6 & 3u) == 1 && (bld >> b & 1u);
	out->under = (bld >> (8u + b) & 1u) != 0;
	for (unsigned n = 0; n < 256; ++n) out->col[n] = (uint16_t)faded(g, n, out->fade, evy);
}

/* A map background's row from the frame's (x0, y): its whole tile map's, or
 * with none (a battle's field) the frame's own pixels from its screen map */
static void map_row(const GbaWorld *g, const Bg *b, int layer, int x0, int y, int w, uint8_t *idx) {
	memset(idx, 0, (size_t)w);
	if (!b->t.on) return;
	if (!g->map[layer]) {
		int from = x0 < 0 ? -x0 : 0, to = 240 - x0 < w ? 240 - x0 : w;
		if (y >= 0 && y < 160 && to > from) gba_screen_row(g->vram, &b->t, x0 + from, y, to - from, idx + from);
		return;
	}
	int py = y + g->oy, px0 = x0 + g->ox;
	if (py < 0 || py >= g->th * 8) return;
	const uint16_t *line = g->map[layer] + (py >> 3) * g->tw;
	int from = px0 < 0 ? -px0 : 0, to = g->tw * 8 - px0 < w ? g->tw * 8 - px0 : w;
	uint8_t row[8];
	for (int i = from; i < to;) {
		int px = px0 + i;
		gba_tile_row(g->vram, &b->t, line[px >> 3], (unsigned)py & 7u, row);
		for (int tx = px & 7; tx < 8 && i < to; ++tx, ++i) idx[i] = row[tx];
	}
}

#define ROW_MOST 1024

void gba_world(const GbaWorld *g, const GbaWindow *win) {
	int x0 = win->x0, y0 = win->y0, w = win->w, h = win->h, stride = win->stride;
	uint32_t *map = win->map, *sky = win->sky;
	unsigned bld = gba_rd16(g->io, 0x50), alpha = gba_rd16(g->io, 0x52), evy = gba_rd16(g->io, 0x54) & 31u;
	unsigned eva = alpha & 31u, evb = alpha >> 8 & 31u;
	if (evy > 16) evy = 16;
	if (eva > 16) eva = 16;
	if (evb > 16) evb = 16;
	Bg bg[4];
	for (unsigned b = 1; b < 4; ++b) bg_read(g, b, evy, &bg[b]);
	int back_fade = (bld >> 5 & 1u) && (bld >> 6 & 3u) > 1 ? (int)(bld >> 6 & 3u) : 0;
	unsigned back = faded(g, 0, back_fade, evy);
	bool back_under = (bld >> 13 & 1u) != 0;
	/* (BG3 in front of the map's backgrounds is the HUD's, not the sky) */
	bool sky3 = !gba_bg3_front(g->io);
	/* (of the two, the one the game shows in front first) */
	int first = bg[2].t.prio < bg[1].t.prio ? 2 : 1, second = 3 - first;
	uint8_t i1[ROW_MOST], i2[ROW_MOST], i3[ROW_MOST];
	if (w > ROW_MOST) w = ROW_MOST;
	for (int j = 0; j < h; ++j) {
		int y = y0 + j;
		if (sky3) gba_screen_row(g->vram, &bg[3].t, x0, y, w, i3);
		else memset(i3, 0, (size_t)w);
		map_row(g, &bg[second], second - 1, x0, y, w, i2);
		map_row(g, &bg[first], first - 1, x0, y, w, i1);
		for (int i = 0; i < w; ++i) {
			int at = j * stride + i;
			unsigned n3 = i3[i], n2 = i2[i], n1 = i1[i];
			unsigned behind = n3 ? bg[3].col[n3] : back;
			sky[at] = shown_by(g, behind);
			if (!n1 && !n2) { map[at] = 0; continue; }
			/* (the lower of the two over what is behind, then the upper over that: each mixed where BLDCNT pairs them) */
			unsigned c = behind;
			bool c_under = n3 ? bg[3].under : back_under;
			if (n2) {
				c = bg[second].over && c_under ? mixed(bg[second].col[n2], c, eva, evb) : bg[second].col[n2];
				c_under = bg[second].under;
			}
			if (n1) c = bg[first].over && c_under ? mixed(bg[first].col[n1], c, eva, evb) : bg[first].col[n1];
			map[at] = shown_by(g, c);
		}
	}
}
