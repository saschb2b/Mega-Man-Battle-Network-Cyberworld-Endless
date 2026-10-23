/* A liberation's board drawn over the game (liberation.h): BN6 has no
 * board, so the drawing is ours (AGENTS.md). The dark panels are the
 * layer's own floor recoloured where it shows, pixel by pixel in the
 * frame (no Capcom picture kept: BN5's own board tiles, read from its ROM,
 * are a later stage's), the sprites and chat boxes left as they are: a
 * pixel in one of the sprites' colours, or under an open chat box, is not
 * the floor's. Where the floor lies comes from the map layer's scroll,
 * which follows MegaMan (BG1's: x + y and (y - x) / 2 of the world, as
 * docs/ROM_DATA.md's row for BN6's camera says). With BN5's ROM the board
 * is BN5's own look (liberation_look.c), and the panel MegaMan faces is
 * outlined by BN5's own sprite, read from its ROM. The corner's phase, par
 * and Order Points, the banner and the command menu in BN6's PET look
 * (pet_look.h). Reads only: draws change nothing. */
#include "liberation.h"

#include <stdio.h>
#include <string.h>

#include "bn5_liberation.h"
#include "bn6_fields.h"
#include "director.h"
#include "director_state.h"
#include "emu.h"
#include "gfx.h"
#include "guardians.h"
#include "liberation_battle.h"
#include "liberation_pieces.h"
#include "liberation_state.h"
#include "liberation_words.h"
#include "netmap.h"
#include "pet_look.h"
#include "platform.h"
#include "rom.h"

/* The GBA's own registers and memory BN6's map is drawn by: the map's
 * first layer's scroll (BG1, its walkway floor: x + y + 256 and
 * (y - x) / 2 - 256 of the world at the picture's middle, mod 512, read
 * on captures as MegaMan was moved) and the sprites' palettes */
#define GBA_BG1HOFS     0x04000014u
#define GBA_BG1VOFS     0x04000016u
#define GBA_OBJ_PALETTE 0x05000200u
#define BOX_TOP 96        /* a chat box's first row (scene_emu.c's TINT_BOX_TOP) */
#define CAM_X  120        /* where MegaMan's place shows, the scroll's own (calibrated on a capture) */
#define CAM_Y  80
#define OUTLINE_DOWN 6    /* the outline's origin below the panel's centre: on its look's diamond (8 down, the OBJs' 2 above it) */
#define POP_DOWN 6        /* an item's icon's origin below its panel's centre, its shadow on the look's diamond */

static struct {
	bool on;               /* the board shows */
	int cam_u, cam_v;      /* the camera: x + y and (y - x) / 2 of the world at the picture's middle */
	bool box;              /* a chat box open */
	bool look;             /* the board in BN5's own look (liberation_look.c) */
	int blink;             /* frames seen, for the faced panel's blink */
	uint8_t sprite[4096];  /* the sprites' colours, a bit each of BN6's 15-bit colours */
	int fx, fy;            /* the panel MegaMan faces, -1 none */
	Anim outline;          /* ... outlined by BN5's own sprite, its ripple from the menu's opening */
	int out_x, out_y;      /* ... the panel it ripples on, -1 none */
	int out_t;             /* ... frames since */
	Anim pop[LIB_POPS];    /* the items' icons freed (LB.pop) */
	bool pop_on[LIB_POPS];
} S;

/* (x mod 512, near `near`) */
static int unwrap(int v, int near) {
	int d = ((v - near) % 512 + 512 + 256) % 512 - 256;
	return near + d;
}

static Sprite *bn5_sprite(int index) {
	const XRom *x = &XR[XROM_BN5_COLONEL_US];
	return x->data ? sprite_get_rom(x->data, x->layout->sprite_lists, BN5_LIB_SPRITE_LIST, index) : NULL;
}

/* BN5's outline of the faced panel (sprite list 7's, BN5_LIB_OUTLINE): its
 * ripple, while the command window is open over a panel to liberate */
static void see_outline(void) {
	int u = S.fx >= 0 ? lib_unit_at(S.fx, S.fy) : 0;
	Sprite *spr = S.look && S.fx >= 0 && LB.step == LIB_STEP_MENU && u < 0 && lib_liberable(&LB.b, S.fx, S.fy) ? bn5_sprite(BN5_LIB_OUTLINE) : NULL;
	if (!spr) { S.out_x = S.out_y = -1; return; }
	if (S.fx != S.out_x || S.fy != S.out_y) { anim_play(&S.outline, spr, BN5_LIB_OUTLINE_RIPPLE); S.out_t = 0; }
	else if (++S.out_t > BN5_LIB_OUTLINE_DELAY) anim_update(&S.outline);
	S.out_x = S.fx;
	S.out_y = S.fy;
}

/* The items' icons over their freed panels: each kind's own sprite (its
 * index BN5_LIB_ITEM_KINDS' +5) */
static void see_pops(void) {
	const XRom *x = &XR[XROM_BN5_COLONEL_US];
	for (int i = 0; i < LIB_POPS; ++i) {
		bool on = S.look && x->data && i < LB.npops;
		if (on) {
			Sprite *spr = bn5_sprite(x->data[BN5_LIB_ITEM_KINDS - 0x08000000u + 8u * (uint32_t)LB.pop[i].kind + BN5_LIB_ICON_OF]);
			if (!spr) on = false;
			else if (!S.pop_on[i] || S.pop[i].spr != spr) anim_play(&S.pop[i], spr, 0);
			else anim_update(&S.pop[i]);
		}
		S.pop_on[i] = on;
	}
}

void lib_see(void) {
	S.on = lib_on() && on_map() && director_arrived();
	if (!S.on) return;
	int px = bn6_player_x(), py = bn6_player_y();
	lib_camera_place(&px, &py);   /* (the camera's place while the board pans it) */
	S.cam_u = unwrap((emu_read16(GBA_BG1HOFS) & 0x1FF) - 256, px + py);
	S.cam_v = unwrap((emu_read16(GBA_BG1VOFS) & 0x1FF) + 256, (py - px) / 2);
	S.box = emu_read8(BN6_CHATBOX) != 0;
	S.look = lib_look_on();
	++S.blink;
	memset(S.sprite, 0, sizeof S.sprite);
	for (uint32_t i = 0; i < 256; ++i) {
		int c = emu_read16(GBA_OBJ_PALETTE + 2 * i) & 0x7FFF;
		S.sprite[c >> 3] |= (uint8_t)(1u << (c & 7));
	}
	S.fx = S.fy = -1;
	if (LB.step == LIB_STEP_MENU || (LB.step == LIB_STEP_MOVE && lib_faced_takes())) lib_faced(&S.fx, &S.fy);
	see_outline();
	see_pops();
}

/* A dark panel's colour: the floor's, sunk into Nebula's purple */
static uint32_t recolour(uint32_t c, int kind, bool faced) {
	int r = c & 0xFF, g = c >> 8 & 0xFF, b = c >> 16 & 0xFF;
	switch (kind) {
	case LIB_DARK: case LIB_ITEM: r = r * 3 / 10 + 40; g = g / 5 + 6; b = b * 4 / 10 + 70; break;
	case LIB_RING: case LIB_BOSS: r = r * 3 / 10 + 92; g = g / 6 + 4; b = b * 3 / 10 + 60; break;
	case LIB_HOLE: r = r / 6 + 18; g = g / 10; b = b / 5 + 34; break;
	case LIB_BONUS: r = r / 2 + 120; g = g / 2 + 100; b = b / 4; break;
	case LIB_TRAP: r = r * 3 / 10 + 120; g = g / 5 + 20; b = b * 3 / 10 + 40; break;
	case LIB_BAR: r = r / 3 + 110; g = g / 3 + 110; b = b / 3 + 130; break;
	default: break;
	}
	if (faced) { r = (r + 255) / 2; g = (g + 230) / 2; b = (b + 120) / 2; }
	return (c & 0xFF000000u) | (uint32_t)(b > 255 ? 255 : b) << 16 | (uint32_t)(g > 255 ? 255 : g) << 8 | (uint32_t)(r > 255 ? 255 : r);
}

static bool sprite_colour(uint32_t c) {
	int k = (int)((c & 0xFF) >> 3 | (c >> 11 & 0x1F) << 5 | (c >> 19 & 0x1F) << 10);
	return S.sprite[k >> 3] >> (k & 7) & 1;
}

void director_tint_board(uint32_t *px) {
	/* (at home, the dark way's portal marked) */
	lib_way_tint(px);
	if (!S.on || S.look) return;
	for (int y = 0; y < (S.box ? BOX_TOP : EMU_H); ++y)
		for (int x = 0; x < EMU_W; ++x) {
			uint32_t c = px[y * EMU_W + x];
			/* (the backdrop's layer, and the sprites) */
			if ((c >> 24) == 0xF8 || sprite_colour(c)) continue;
			int u = x - CAM_X + S.cam_u, v = 2 * (y - CAM_Y + S.cam_v), gx, gy;
			if (!netmap_panel((u - v) / 2, (u + v) / 2, &gx, &gy) || gx < 0 || gy < 0 || gx >= MAP_W || gy >= MAP_H) continue;
			int k = LB.b.kind[gy][gx];
			bool faced = gx == S.fx && gy == S.fy;
			/* (BN5's own look, where it lies on the board: only the faced panel's light) */
			if (k == LIB_VOID || (!faced && (k == LIB_CLEAN || S.look))) continue;
			px[y * EMU_W + x] = recolour(c, k, faced);
		}
}

/* The corner: the phase against the par, and the Order Points as pips */
static void draw_corner(int x, int y) {
	char s[24];
	SDL_Rect f = look_box(x, y, 104, 25, LOOK_NAVY);
	snprintf(s, sizeof s, "%s %d", lib_label_phase, LB.phase);
	look_caps(f.x + 3, f.y + 3, s, LB.phase > LB.b.par ? LOOK_ORANGE : LOOK_TEXT, LOOK_SHADE);
	snprintf(s, sizeof s, "%s %d", lib_label_par, LB.b.par);
	look_caps(f.x + f.w - 3 - look_caps_width(s), f.y + 3, s, LOOK_GOLD, LOOK_SHADE);
	look_caps(f.x + 3, f.y + 12, lib_label_op, LOOK_TEXT, LOOK_SHADE);
	for (int i = 0; i < LIB_OP_MAX; ++i)
		fill_rect(f.x + 32 + i * 6, f.y + 12, 4, 5, i < LB.op ? LOOK_GOLD : LOOK_NAVY_LO);
}

static void draw_banner(void) {
	int w = 150, x = P.core_x + (EMU_W - w) / 2, y = P.core_y + 52;
	int t = LB.banner_t, slide = t < 8 ? (8 - t) * 20 : 0;
	look_band(x - slide, y, w, LB.enemy_banner ? lib_label_enemy_phase : lib_label_player_phase);
	char s[32];
	snprintf(s, sizeof s, "%s %d   %s %d", lib_label_phase, LB.phase, lib_label_par, LB.b.par);
	SDL_Rect f = look_box(x + 20 + slide, y + LOOK_BAND_H + 2, w - 40, 11, LOOK_NAVY);
	look_caps(f.x + (f.w - look_caps_width(s)) / 2, f.y + 2, s, LOOK_TEXT, LOOK_SHADE);
}

/* What MegaMan faces, for the menu's top line */
static void faced_label(char *s, size_t n) {
	int fx, fy;
	lib_faced(&fx, &fy);
	int u = lib_unit_at(fx, fy);
	if (u == 0) snprintf(s, n, "%s", guardian(LB.darkloid)->name);
	else if (u > 0) snprintf(s, n, "%s", lib_unit_name(LB.b.unit[u].kind));
	else if (fx >= 0 && fy >= 0 && fx < MAP_W && fy < MAP_H) snprintf(s, n, "%s", lib_faced_label(LB.b.kind[fy][fx]));
	else s[0] = 0;
}

static void draw_menu(void) {
	int w = 96, h = 18 + LIB_CMDS * LOOK_LINE, x = P.core_x + EMU_W - w - 6, y = P.core_y + 34;
	look_band(x, y, w, lib_label_menu);
	SDL_Rect f = look_window(x, y + LOOK_BAND_H, w, h, 0, false);
	char s[40];
	faced_label(s, sizeof s);
	look_caps(f.x + 3, f.y + 2, s, LOOK_GOLD, LOOK_SHADE);
	for (int c = 0; c < LIB_CMDS; ++c) {
		int ly = f.y + 10 + c * LOOK_LINE;
		look_text(f.x + LOOK_CURSOR_W + 4, ly, lib_command_label(c), LB.can[c] ? LOOK_TEXT : LOOK_DIM, LOOK_SHADE, TEXT_LEFT);
		if (c == LB.menu) look_cursor(f.x + 1, ly + 1);
	}
}

/* The panel MegaMan faces, outlined as BN5's board outlines it */
static void draw_faced(void) {
	if (S.out_x < 0 || S.out_t < BN5_LIB_OUTLINE_DELAY) return;
	int wx, wy;
	netmap_world(S.out_x, S.out_y, &wx, &wy);
	anim_draw(&S.outline, P.core_x + wx + wy - S.cam_u + CAM_X, P.core_y + (wy - wx) / 2 - S.cam_v + CAM_Y + OUTLINE_DOWN, false, 0, 0);
}

/* The items freed, popped over their panels as BN5's: up 8, then down 2 */
static void draw_pops(void) {
	for (int i = 0; i < LIB_POPS; ++i) {
		if (!S.pop_on[i]) continue;
		int wx, wy, t = LB.pop[i].t, lift = t < BN5_LIB_ICON_POP ? 0 : t < BN5_LIB_ICON_POP + 5 ? 8 : 6;
		netmap_world(LB.pop[i].x, LB.pop[i].y, &wx, &wy);
		anim_draw(&S.pop[i], P.core_x + wx + wy - S.cam_u + CAM_X, P.core_y + (wy - wx) / 2 - S.cam_v + CAM_Y + POP_DOWN - lift, false, 0, 0);
	}
}

/* The command window as BN5's, its lines BN5's commands */
static void draw_window(void) {
	const char *lines[LIB_CMDS];
	for (int c = 0; c < LIB_CMDS; ++c) lines[c] = lib_command_label(c);
	lib_piece_window(lines, LB.can, LIB_CMDS, LB.menu);
}

void director_draw_board(void) {
	if (!S.on) return;
	/* (BN5's own pieces where its ROM is there, else the PET's look) */
	bool bn5 = S.look && lib_pieces_on();
	draw_faced();
	draw_pops();
	if (bn5) lib_piece_gauge(LB.op);
	else draw_corner(P.core_x + EMU_W - 108, P.core_y + 4);
	/* (L held, as BN5's map view: the phase and its target) */
	if (bn5 && D.l_held && LB.step == LIB_STEP_MOVE) lib_piece_phases(LB.phase, LB.b.par, S.blink);
	if (LB.step == LIB_STEP_BANNER) {
		if (bn5 && lib_piece_banner_frames()) lib_piece_phase_banner(LB.enemy_banner, LB.phase, LB.banner_t);
		else draw_banner();
	}
	if (bn5 && LB.cheer && LB.cheer_t >= LIB_CHEER_WAIT) lib_piece_cheer(LB.cheer, LB.cheer_t - LIB_CHEER_WAIT);
	if (LB.step == LIB_STEP_MENU) {
		if (bn5) draw_window();
		else draw_menu();
		/* (what the battle stakes, above BN5's window or under the menu) */
		lib_battle_draw_stakes(EMU_W - 6, bn5 ? 82 : 34 + LOOK_BAND_H + 18 + LIB_CMDS * LOOK_LINE + 2 * LOOK_FRAME + 2);
	}
}
