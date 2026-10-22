/* --dev arena2v2 on a seat's screen (arena_team_view.h). */
#include "arena_team_view.h"

#include <stdio.h>

#include <mgba/core/core.h>

#include "arena_pair.h"
#include "arena_team_custom.h"
#include "bn6.h"
#include "gfx.h"
#include "pixfont.h"
#include "platform.h"
#include "second_frame.h"

/* the GBA's picture as BN6 lays its battle out, measured from the pair's
 * captures: its field's panels 40 x 24 from row 1's top, its HP box at
 * the top left in the fight and right of the Custom screen's window while
 * it is open */
#define PANEL_W      40
#define PANEL_H      24
#define FIELD_TOP    72
#define HP_X         2
#define HP_X_CUSTOM  122
#define HP_W         44
#define HP_H         16
#define TAG_X        20   /* a seat's mark from his panel's middle, left of BN6's HP label under him and a line up */
#define TAG_UP       9
#define SIGN_Y       30   /* the team's sign: under the Square's name plate (arena_view.h), at the picture's right */
#define SIGN_H       11
#define DOT_FRAMES   20   /* a dot of the three lit this long, then the next */
#define HURRY_FRAMES 6    /* the late one's own plate shown this long, then hidden as long: BN6's "Cstmzing..." blinks */

static const SDL_Color shade = { 0, 24, 41, 255 };

unsigned arena_team_view_side(unsigned seat) { return seat < 2 ? seat & 1u : (seat - 2) & 1u; }

static unsigned rd8(struct mCore *c, uint32_t at) { return (unsigned)c->rawRead8(c, at, -1) & 0xFFu; }
static unsigned rd16(struct mCore *c, uint32_t at) { return (unsigned)c->rawRead16(c, at, -1) & 0xFFFFu; }

static void frame_box(int x, int y, int w, int h, SDL_Color c) {
	fill_rect(x - 1, y - 1, w + 2, 1, c);
	fill_rect(x - 1, y + h, w + 2, 1, c);
	fill_rect(x - 1, y, 1, h, c);
	fill_rect(x + w, y, 1, h, c);
}

/* a MegaMan's object: side's player (k 0) or partner (k 1); 0 none */
static uint32_t megaman(struct mCore *c, unsigned side, unsigned k) {
	return (uint32_t)c->rawRead32(c, BN6_BATTLE_PLAYERS + side * BN6_BATTLE_PLAYER_STEP + k * BN6_SIDE_PARTNER, -1);
}

/* the HP box, his own over BN6's (which shows his lead's) */
static void hp_box(unsigned hp, bool open) {
	int x = P.core_x + (open ? HP_X_CUSTOM : HP_X), y = P.core_y;
	char n[8];
	snprintf(n, sizeof n, "%u", hp);
	fill_rect(x, y, HP_W, HP_H, PET_EXIT);
	frame_box(x + 1, y + 1, HP_W - 2, HP_H - 2, PET_CYAN);
	text_draw(x + HP_W - 4, y + 1, n, PET_WHITE, TEXT_RIGHT);
}

/* each MegaMan's seat by his feet, in his side's colour (the viewer's own
 * side's cyan), and the ally BN6 leaves unlabelled his HP under him */
static void marks(struct mCore *c, unsigned view, unsigned own) {
	for (unsigned side = 0; side < 2; ++side)
		for (unsigned k = 0; k < 2; ++k) {
			uint32_t o = megaman(c, side, k);
			if (o < BN6_EWRAM || o >= BN6_EWRAM_END) continue;
			unsigned col = rd8(c, o + BN6_T1_PANEL_X), row = rd8(c, o + BN6_T1_PANEL_Y), hp = rd16(c, o + BN6_T1_HP);
			if (!col || col > 6 || !row || row > 3 || !hp) continue;
			unsigned shown_col = view ? 7 - col : col, seat = k ? 2 + side : side;
			int px = P.core_x + (int)(shown_col - 1) * PANEL_W + PANEL_W / 2, py = P.core_y + FIELD_TOP + (int)(row - 1) * PANEL_H + 16;
			char tag[8];
			snprintf(tag, sizeof tag, "P%u", seat + 1);
			pixfont_draw(px - TAG_X, py - TAG_UP, tag, side == view ? PET_CYAN_HI : PET_GOLD, shade, PIXFONT_LEFT, 1);
			/* (BN6 labels the enemies and keeps the viewer's in the box: the ally beside him has none) */
			if (side != view || seat == own) continue;
			char n[8];
			snprintf(n, sizeof n, "%u", hp);
			pixfont_draw(px + 2, py, n, PET_WHITE, shade, PIXFONT_LEFT, 1);
		}
}

/* The team's sign on the Custom screen, quiet, no words: on a partner's
 * screen his lead's seat and a tick once his lead has pressed OK and
 * waits for him; on a lead's, his partner's seat and three dots, one lit
 * in turn, while his partner picks */
static void sign(struct mCore *c, unsigned side, unsigned seat) {
	static unsigned shown;   /* (the sign's own frames, for its dots: a drawing's count, no game state) */
	bool partner = seat >= 2;
	if (partner ? !arena_team_custom_lead_waits(c, side) : !arena_team_custom_picking(c, side)) return;
	uint32_t o = megaman(c, side, 1);
	if (!partner && (o < BN6_EWRAM || o >= BN6_EWRAM_END || !rd16(c, o + BN6_T1_HP))) return;
	char tag[8];
	snprintf(tag, sizeof tag, "P%u", partner ? side + 1 : side + 3);
	int w = pixfont_width(tag, 1) + 4 + (partner ? pixfont_width(PIXFONT_CHECK, 1) : 3 * 4) + 6;
	int x = P.core_x + ARENA_W - 2 - w, y = P.core_y + SIGN_Y;
	fill_rect(x, y, w, SIGN_H, rgba(0, 49, 74, 220));
	fill_rect(x, y + SIGN_H - 1, w, 1, PET_CYAN);
	pixfont_draw(x + 3, y + 2, tag, PET_CYAN_HI, shade, PIXFONT_LEFT, 1);
	int after = x + 3 + pixfont_width(tag, 1) + 4;
	if (partner) { pixfont_draw(after, y + 2, PIXFONT_CHECK, PET_GOLD, shade, PIXFONT_LEFT, 1); return; }
	unsigned lit = ++shown / DOT_FRAMES % 3;
	for (unsigned k = 0; k < 3; ++k) fill_rect(after + 4 * (int)k, y + 6, 2, 2, k == lit ? PET_WHITE : PET_DIM);
}

/* The late one's sign, his alone, in the turn's last ten seconds: his own seat's plate where the team's sign stands
 * (under it, the Square's name plate above both), blinking fast as BN6's "Cstmzing..." blinks; no number, no word */
static void hurry_sign(unsigned seat) {
	static unsigned shown;   /* (the sign's own frames: a drawing's count, no game state) */
	if (++shown / HURRY_FRAMES % 2) return;
	char tag[8];
	snprintf(tag, sizeof tag, "P%u", seat + 1);
	int w = pixfont_width(tag, 1) + 6, x = P.core_x + ARENA_W - 2 - w, y = P.core_y + SIGN_Y + SIGN_H + 2;
	fill_rect(x, y, w, SIGN_H, rgba(74, 49, 0, 220));
	fill_rect(x, y + SIGN_H - 1, w, 1, PET_GOLD);
	pixfont_draw(x + 3, y + 2, tag, PET_GOLD, shade, PIXFONT_LEFT, 1);
}

void arena_team_view(struct mCore *c, unsigned seat, bool hurry) {
	if (!c || seat > 3) return;
	unsigned side = arena_team_view_side(seat);
	bool open = rd8(c, BN6_BATTLE_PHASE) == BN6_PHASE_CUSTOM, partner = seat >= 2;
	uint32_t own = megaman(c, side, partner);
	if (partner && own >= BN6_EWRAM && own < BN6_EWRAM_END) hp_box(rd16(c, own + BN6_T1_HP), open);
	if (open) sign(c, side, seat);
	else marks(c, side, seat);
	if (open && hurry) hurry_sign(seat);
}
