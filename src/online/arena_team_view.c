/* --dev arena2v2 on a seat's screen (arena_team_view.h). */
#include "arena_team_view.h"

#include <stdio.h>

#include <mgba/core/core.h>

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

void arena_team_view(struct mCore *c, unsigned seat) {
	if (!c || seat > 3) return;
	unsigned side = arena_team_view_side(seat);
	bool open = rd8(c, BN6_BATTLE_PHASE) == BN6_PHASE_CUSTOM, partner = seat >= 2;
	uint32_t own = megaman(c, side, partner);
	if (partner && own >= BN6_EWRAM && own < BN6_EWRAM_END) hp_box(rd16(c, own + BN6_T1_HP), open);
	if (!open) marks(c, side, seat);
}
