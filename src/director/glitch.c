/* The run's Glitch layer, the director's part (glitch.h; docs/game/glitch-layer.md).
 *
 * The stage is the share of the layer's Mystery Data taken (BN6's own
 * flags, which a CONTINUE restores): its data lie at the ends of its
 * detours, so the layer comes apart the further he explores it. Each glitch is decided in the update, on numbers
 * of its own (the generator's and the game's left alone), and drawn over
 * the frame read-only: rows of the floor torn sideways, blocks of it
 * slipped by a tile, its colours turned. On the layer holding the warp the
 * same glitches stay round the warp. Sprites, the chat box and BN6's own
 * picture underneath are never changed by a draw. */
#include "glitch.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6.h"
#include "bn6_fields.h"
#include "bn6_forms.h"
#include "devtools.h"
#include "director.h"
#include "director_layer.h"
#include "director_state.h"
#include "emu.h"
#include "flags.h"
#include "glitch_words.h"
#include "mapslot.h"
#include "net.h"
#include "netmap.h"
#include "run.h"
#include "talk.h"

#define CAM_X 120          /* where the camera's point shows */
#define CAM_Y 80
#define BOX_TOP 96         /* a chat box's rows, left whole */
#define ZONE 30            /* the warp's glitch: this many rows above and below it */
#define GBA_OBJ_PALETTE 0x05000200u   /* the sprites' 256 colours */

/* The share of the layer's Mystery Data taken (in hundredths) each stage
 * begins at, the first data taken stage 1's */
static const int stage_at[GLITCH_STAGES] = { 0, 1, 30, 55, 80 };
/* Frames between glitches at each stage, from and to */
static const int gap_lo[GLITCH_STAGES] = { 300, 180, 100, 60, 30 }, gap_hi[GLITCH_STAGES] = { 600, 360, 220, 140, 80 };

enum { FX_NONE, FX_TEAR, FX_SLIP, FX_TURN, FX_FLIP };

#define FLIP_EDGE 6        /* a flip's frames in and out under the mosaic */
#define LOOK_MOSAIC 12     /* the warp's new look under BN6's mosaic as it comes, clearing over the glitch */
#define LOOK_SET 2         /* frames its loop stays pointed at the new look's snippet */

static struct {
	bool on;            /* a Glitch layer */
	bool warp;          /* a layer holding its warp, or a Glitch layer its copy */
	int wx, wy;         /* ... its world place */
	int floor;          /* the layer's floor panels */
	int stage, said;    /* its stage, and the stages said */
	int share;          /* the share of its Mystery Data taken, in hundredths */
	uint32_t rng;       /* its own numbers */
	int next;           /* D.frame of the next glitch */
	int fx, fx_left;    /* the glitch under way */
	int fx_len;         /* ... and its length */
	uint32_t fx_seed;
	/* (for the draw: where the warp shows, a chat box open, on the map, the
	 * sprites' colours, a bit each of BGR555) */
	int sx, sy, mx, my;
	int look;           /* the warp's colours, turned one of three ways */
	int look_k;         /* its look now (glitch_look_sprite) */
	int look_set, blur, blur_len;   /* frames of its new look's snippet, and of its mosaic clearing */
	int box_w, box_up, box_down;    /* the box its glitches are drawn in, round its foot */
	int jitter;         /* frames left of a small jitter between its glitches */
	uint32_t jitter_seed;
	unsigned tick;      /* the update's frames, for a glitch's alternating frames */
	bool box, shown;
	uint8_t sprite[32768 / 8];
	bool settled;       /* the copy deleted: the glitch gone */
} G;

static uint32_t next_rng(void) {
	G.rng ^= G.rng << 13;
	G.rng ^= G.rng >> 17;
	G.rng ^= G.rng << 5;
	return G.rng;
}

static int roll(int lo, int hi) { return lo + (int)(next_rng() % (uint32_t)(hi - lo + 1)); }

void glitch_layer_begin(void) {
	memset(&G, 0, sizeof G);
	G.on = run.side_kind == LAYER_GLITCH;
	G.rng = run.layer_seed ^ 0x9E3779B9u;
	if (!G.rng) G.rng = 1;
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].type == OBJ_GLITCH || layer.obj[i].type == OBJ_COPY) {
			G.warp = true;
			netmap_world((int)layer.obj[i].x, (int)layer.obj[i].y, &G.wx, &G.wy);
			/* (a pad flat on the floor, a Navi standing) */
			bool pad = layer.obj[i].type == OBJ_GLITCH;
			G.box_w = pad ? 36 : 18;
			G.box_up = pad ? 30 : 52;
			G.box_down = pad ? 22 : 10;
		}
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) G.floor += layer.cell[y][x] == C_PATH;
	G.next = roll(gap_lo[0], gap_hi[0]);
	G.look = 1;
	if (G.on) glitch_looks_make(D.layer_tiles);
	else glitch_looks_free();
}

/* The stage from the layer's Mystery Data taken, BN6's own flags (so a
 * CONTINUE keeps it): each data taken deepens the glitch; never lower
 * than it was */
static void measure(void) {
	if (dev.glitch_stage > 0) { G.stage = dev.glitch_stage - 1 < GLITCH_STAGES ? dev.glitch_stage - 1 : GLITCH_STAGES - 1; return; }
	int taken = 0;
	for (int k = 0; k < D.objs.nmd; ++k) taken += flag_get(MAPSLOT_MD_FLAG + k);
	int share = D.objs.nmd ? (taken * 100 + D.objs.nmd - 1) / D.objs.nmd : 0, s = 0;
	G.share = share;
	while (s + 1 < GLITCH_STAGES && share >= stage_at[s + 1]) ++s;
	if (s > G.stage) G.stage = s;
}

/* The warp's next look: a pad of another kind the map loaded, under BN6's
 * mosaic (its talker's look snippet, npc_talker_looks) */
static void warp_look_next(int frames) {
	const NpcLooks *w = &D.objs.glitch_warp;
	if (G.on || !w->jump_at || !w->look || !w->blur) return;
	int other[GLITCH_LOOKS], n = 0;
	for (int k = 0; k < GLITCH_LOOKS; ++k)
		if (k != G.look_k && (D.objs.glitch_looks >> k & 1)) other[n++] = k;
	if (!n) return;
	G.look_k = other[roll(0, n - 1)];
	uint8_t sprite = glitch_look_sprite[G.look_k], m[2] = { LOOK_MOSAIC, LOOK_MOSAIC };
	emu_write(w->look + 1, &sprite, 1);
	emu_write(w->look + 6, m, 2);
	G.look_set = LOOK_SET;
	G.blur = G.blur_len = frames + LOOK_SET;
}

/* Each frame: the warp's loop pointed at its new look's snippet, then at
 * its mosaic clearing, then back at itself */
static void warp_look_step(void) {
	const NpcLooks *w = &D.objs.glitch_warp;
	if (!w->jump_at) return;
	uint32_t target = w->loop;
	if (G.look_set > 0) {
		--G.look_set;
		target = w->look;
	} else if (G.blur > 0) {
		--G.blur;
		int size = G.blur > 2 ? LOOK_MOSAIC * (G.blur - 2) / G.blur_len : 0;
		uint8_t m[2] = { (uint8_t)size, (uint8_t)size };
		emu_write(w->blur + 1, m, 2);
		target = w->blur;
	}
	if (emu_read32(w->jump_at) != target) emu_write32(w->jump_at, target);
}

/* Between the warp's glitches, now and then a frame or two of a few rows
 * slipped a pixel or two: it never quite holds still */
static void warp_jitter(void) {
	if (G.on || G.fx != FX_NONE) return;
	if (G.jitter > 0) { --G.jitter; return; }
	if (roll(0, 59)) return;
	G.jitter = roll(1, 2);
	G.jitter_seed = next_rng();
}

/* The next glitch: torn rows first, blocks slipped from stage 1, colours
 * turned from stage 2; longer the further the layer has come apart */
static void schedule(void) {
	if (G.fx_left > 0) { --G.fx_left; return; }
	/* (a flip over: the layer's own back; the warp's new colours) */
	if (G.fx == FX_FLIP) glitch_look_flip(-1);
	if (G.fx != FX_NONE) G.look = 1 + (G.look + roll(0, 1)) % 3;
	G.fx = FX_NONE;
	if (D.frame < G.next) return;
	int s = G.on ? G.stage : 0, kinds = s >= 2 ? 3 + (glitch_looks() > 0) : s >= 1 ? 2 : 1;
	G.fx = FX_TEAR + roll(0, kinds - 1);
	G.fx_left = G.fx == FX_FLIP ? roll(10 + 20 * (s - 2), 30 + 50 * (s - 2)) : roll(2, 4 + 2 * s);
	G.fx_len = G.fx_left;
	G.fx_seed = next_rng();
	if (G.fx == FX_FLIP) glitch_look_flip(roll(0, glitch_looks() - 1));
	warp_look_next(G.fx_left);
	/* (the warp's: every three to seven seconds) */
	G.next = D.frame + G.fx_left + (G.on ? roll(gap_lo[s], gap_hi[s]) : roll(180, 420));
}

/* (the warp on the screen, from BN6's camera as it is drawn) */
static void warp_on_screen(void) {
	uint32_t cam = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CAMERA);
	int cx = (int32_t)emu_read32(cam + BN6_CAMERA_RENDER_X) >> 16, cy = (int32_t)emu_read32(cam + BN6_CAMERA_RENDER_Y) >> 16;
	int cz = (int32_t)emu_read32(cam + BN6_CAMERA_RENDER_Z) >> 16;
	int cam_u = cx + cy, cam_v = (cy - cx) / 2 - cz;
	G.sx = G.wx + G.wy - cam_u + CAM_X;
	G.sy = (G.wy - G.wx) / 2 - cam_v + CAM_Y;
	int px = bn6_player_x(), py = bn6_player_y(), pz = bn6_player_z();
	G.mx = px + py - cam_u + CAM_X;
	G.my = (py - px) / 2 - pz - cam_v + CAM_Y;
}

/* The stage's words, once each from stage 2, when nothing else is said */
static void stage_words(void) {
	if (G.said >= G.stage || D.beat[0] || talk_busy() || emu_read8(BN6_CHATBOX) || D.area_card) return;
	const char *w = glitch_stage_words(G.said + 1);
	if (!w || talk_start(w, FACE_MEGAMAN)) ++G.said;
}

/* (the run log's stdout: the stage a Glitch layer was left at, for the
 * stages' tuning, docs/game/glitch-layer.md) */
static void left_at(void) {
	static uint32_t told;
	if (!G.on || told == run.layer_seed || D.objs.exit_x == 0) return;
	int px = bn6_player_x(), py = bn6_player_y();
	if (abs(px - D.objs.exit_x) + abs(py - D.objs.exit_y) > 48) return;
	told = run.layer_seed;
	printf("glitch: at the exit at stage %d, %d%% of its data taken%s\n", G.stage, G.share, G.settled ? ", the copy deleted" : "");
}

void glitch_update(void) {
	G.shown = false;
	if (on_map()) left_at();
	if ((!G.on && !G.warp) || !on_map()) return;
	if (G.on && !G.settled) measure();
	if (G.on && !G.settled) stage_words();
	glitch_look_update();
	warp_look_step();
	if (G.settled) {
		if (glitch_look_shown() >= 0) glitch_look_flip(-1);
		G.fx = FX_NONE;
		return;
	}
	schedule();
	warp_jitter();
	++G.tick;
	G.box = emu_read8(BN6_CHATBOX) != 0;
	if (G.warp) warp_on_screen();
	if (G.fx != FX_NONE) {
		memset(G.sprite, 0, sizeof G.sprite);
		for (uint32_t i = 0; i < 256; ++i) {
			int c = emu_read16(GBA_OBJ_PALETTE + 2 * i) & 0x7FFF;
			G.sprite[c >> 3] |= (uint8_t)(1u << (c & 7));
		}
	}
	G.shown = true;
}

void glitch_battle(int *depth, bool *visitors) {
	if (!G.on || G.settled) return;
	if (G.stage >= 2) *visitors = true;
	if (G.stage >= 3) *depth += 3;
}

void glitch_settle(void) {
	G.settled = true;
	G.fx = FX_NONE;
}

/* ---- the frame ---- */

/* (a pixel of the map's layers: not the backdrop, which mGBA's top byte
 * names 0xF8, and no sprite's colour, as a liberation's board tells them) */
static bool floor_px(uint32_t c) {
	if (c >> 24 == 0xF8) return false;
	int k = (int)((c & 0xFF) >> 3 | (c >> 11 & 0x1F) << 5 | (c >> 19 & 0x1F) << 10);
	return !(G.sprite[k >> 3] >> (k & 7) & 1);
}

static uint32_t hash(uint32_t a, uint32_t b) {
	uint32_t h = (a ^ 0x85EBCA6Bu) * 0xC2B2AE35u ^ b * 0x27D4EB2Fu;
	return h ^ h >> 15;
}

/* Rows torn sideways in bands, a band's rows all by one offset */
static void tear(uint32_t *px, int y0, int y1) {
	static uint32_t row[EMU_W];
	for (int y = y0; y < y1; ++y) {
		uint32_t h = hash(G.fx_seed, (uint32_t)y / 6);
		if (h % 3) continue;
		int dx = (int)(h >> 8 & 31) - 16;
		memcpy(row, px + y * EMU_W, sizeof row);
		for (int x = 0; x < EMU_W; ++x) {
			uint32_t from = row[((x - dx) % EMU_W + EMU_W) % EMU_W];
			if (floor_px(px[y * EMU_W + x]) && floor_px(from)) px[y * EMU_W + x] = from;
		}
	}
}

/* Blocks of 8 by 8 slipped by a tile, as a tile map read a tile off */
static void slip(uint32_t *px, int y0, int y1) {
	for (int by = y0 / 8; by * 8 < y1; ++by)
		for (int bx = 0; bx < EMU_W / 8; ++bx) {
			uint32_t h = hash(G.fx_seed, (uint32_t)(by * 64 + bx));
			if (h % 7) continue;
			int sx = (h >> 8 & 1) ? 8 : -8;
			for (int y = by * 8; y < by * 8 + 8 && y < y1; ++y)
				for (int x = bx * 8; x < bx * 8 + 8; ++x) {
					int fx = x + sx;
					if (fx < 0 || fx >= EMU_W) continue;
					uint32_t from = px[y * EMU_W + fx];
					if (floor_px(px[y * EMU_W + x]) && floor_px(from)) px[y * EMU_W + x] = from;
				}
		}
}

/* Colours turned round in bands, red to green to blue */
static void turn(uint32_t *px, int y0, int y1) {
	for (int y = y0; y < y1; ++y) {
		if (hash(G.fx_seed, (uint32_t)y / 10) % 2) continue;
		for (int x = 0; x < EMU_W; ++x) {
			uint32_t c = px[y * EMU_W + x];
			if (!floor_px(c)) continue;
			px[y * EMU_W + x] = (c & 0xFF000000u) | (c & 0xFF) << 8 | (c >> 8 & 0xFF) << 16 | (c >> 16 & 0xFF);
		}
	}
}

/* A flip's edges under BN6's mosaic, as the title changes its backdrops:
 * blocks of `n` by `n`, each its first pixel's colour */
static void mosaic(uint32_t *px, int y0, int y1, int n) {
	for (int y = y0; y < y1; ++y)
		for (int x = 0; x < EMU_W; ++x) {
			int fy = y - (y - y0) % n, fx = x - x % n;
			uint32_t from = px[fy * EMU_W + fx];
			if (floor_px(px[y * EMU_W + x]) && floor_px(from)) px[y * EMU_W + x] = from;
		}
}

/* The warp's own look, and the copy's (its sprite and what stands round
 * it, MegaMan's box left alone): its colours turned one of three ways,
 * never the Undernet's purple nor the guardian's own, the way changed with
 * each glitch; in a glitch, its rows torn sideways, sprite and all */

/* (MegaMan's box, and BN6's HP window in the corner, left alone) */
#define HP_W 48
#define HP_H 18

static bool megaman_px(int x, int y) { return (abs(x - G.mx) <= 12 && y >= G.my - 34 && y <= G.my + 4) || (x < HP_W && y < HP_H); }

static uint32_t turned(uint32_t c, int way) {
	uint32_t r = c & 0xFF, g = c >> 8 & 0xFF, b = c >> 16 & 0xFF, a = c & 0xFF000000u;
	if (way == 1) return a | g | b << 8 | r << 16;
	if (way == 2) return a | b | r << 8 | g << 16;
	return a | (255 - r) | (255 - g) << 8 | (255 - b) << 16;
}

/* (a row of the box: its pixels shifted `dx` round within it, its sprite's
 * colours turned, MegaMan's box left) */
static void warp_row(uint32_t *px, int y, int x0, int x1, int dx) {
	static uint32_t row[EMU_W];
	memcpy(row, px + y * EMU_W, sizeof row);
	for (int x = x0; x < x1; ++x) {
		if (megaman_px(x, y)) continue;
		int fx = x - dx < x0 ? x1 - (x0 - (x - dx)) : x - dx >= x1 ? x0 + (x - dx - x1) : x - dx;
		uint32_t c = row[fx < 0 ? 0 : fx >= EMU_W ? EMU_W - 1 : fx];
		if (megaman_px(fx, y)) continue;
		px[y * EMU_W + x] = c >> 24 != 0xF8 && !floor_px(c) ? turned(c, G.look) : c;
	}
}

/* (a row of the box split by colour: its red from the left, its blue from
 * the right, as a signal's channels drift apart) */
static void warp_split(uint32_t *px, int y, int x0, int x1, int d) {
	static uint32_t row[EMU_W];
	memcpy(row, px + y * EMU_W, sizeof row);
	for (int x = x0; x < x1; ++x) {
		if (megaman_px(x, y)) continue;
		uint32_t l = row[x - d < x0 ? x0 : x - d], r = row[x + d >= x1 ? x1 - 1 : x + d], c = row[x];
		px[y * EMU_W + x] = (c & 0xFF00FF00u) | (l & 0xFFu) | (r & 0xFF0000u);
	}
}

static void warp_frame(uint32_t *px) {
	int x0 = G.sx - G.box_w < 0 ? 0 : G.sx - G.box_w, x1 = G.sx + G.box_w > EMU_W ? EMU_W : G.sx + G.box_w;
	int y0 = G.sy - G.box_up < 0 ? 0 : G.sy - G.box_up, y1 = G.sy + G.box_down, bottom = G.box ? BOX_TOP : EMU_H;
	if (y1 > bottom) y1 = bottom;
	bool glitch = G.fx == FX_TEAR || G.fx == FX_SLIP, split = glitch && (G.tick & 1);
	for (int y = y0; y < y1; ++y) {
		uint32_t h = hash(glitch ? G.fx_seed : G.jitter_seed, (uint32_t)y / 4);
		int dx = glitch ? (int)(h >> 8 & 15) - 8 : G.jitter && h % 5 == 0 ? (int)(h >> 8 & 3) - 2 : 0;
		if (split) warp_split(px, y, x0, x1, 2 + (int)(h >> 12 & 1));
		warp_row(px, y, x0, x1, split ? 0 : dx);
	}
}

/* The frame's glitches over the picture (`px`, the core's frame with
 * mGBA's layer byte on top): read-only, the update decided them */
void director_glitch_frame(uint32_t *px) {
	if (!G.shown) return;
	if (G.warp && G.sx > -G.box_w && G.sx < EMU_W + G.box_w && G.sy > -G.box_down && G.sy < EMU_H + G.box_up) warp_frame(px);
	if (!G.on || G.fx == FX_NONE) return;
	if (G.fx == FX_FLIP) {
		int in = G.fx_len - G.fx_left, edge = in < G.fx_left ? in : G.fx_left;
		if (edge < FLIP_EDGE) mosaic(px, 0, G.box ? BOX_TOP : EMU_H, 2 * (FLIP_EDGE - edge));
		return;
	}
	int y1 = G.box ? BOX_TOP : EMU_H;
	if (G.fx == FX_TEAR) tear(px, 0, y1);
	else if (G.fx == FX_SLIP) slip(px, 0, y1);
	else turn(px, 0, y1);
}

