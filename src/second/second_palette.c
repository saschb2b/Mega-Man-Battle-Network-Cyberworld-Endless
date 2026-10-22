/* second_palette.h. As the DS games kept their PET on the touch screen,
 * the second screen keeps every emote and phrase a tap away: the emotes
 * in two rows of six (MegaMan's pose from the player's ROM, drawn as the
 * panel is drawn, its name under it), the phrases in two rows of four,
 * each in a PET panel of Gregar's colours (second_frame.h). */
#include "second_palette.h"

#include <stdio.h>

#include "bn6.h"
#include "emote_strip_draw.h"
#include "emote_strip_text.h"
#include "minifont.h"
#include "netpass_draw.h"
#include "online_text.h"
#include "second_frame.h"
#include "square_emote.h"

#define FACE_BACK rgba(16, 54, 74, 255)
#define INK       rgba(0, 49, 74, 255)
#define FACING 3   /* (the strip's: toward the screen's lower left) */

static const char *item_name(int item) { return item < SQUARE_PHRASE ? online_emotes[item] : online_phrases[item - SQUARE_PHRASE]; }

static int way_of(const PaletteView *v, int item) {
	for (int k = 0; k < 4; ++k) if (v->favourites[k] == item) return k;
	return -1;
}

/* whether every emote's name fits w in the game's letters (else all are
 * in the small capitals: one look a row) */
static bool letters_fit(int w) {
	for (int e = 0; e < ONLINE_EMOTE_COUNT; ++e) if (text_width(online_emotes[e]) > w - 4) return false;
	return true;
}

/* a name centred at (cx, y): the game's letters, or the small capitals */
static void name(int cx, int y, bool letters, const char *s, SDL_Color c) {
	if (letters) text_draw(cx, y, s, c, TEXT_CENTER);
	else minifont_draw_centered(cx, y + 6, s, c, 1);
}

void second_palette_tabs(int w, int h, const PaletteView *v) {
	const char *names[2] = { strip_text_pages[0], strip_text_netpass };
	for (int t = 0; t < 2; ++t) {
		SDL_Rect r = emote_palette_tab(w, h, t);
		bool on = v->tab == t;
		netpass_panel(r.x, r.y, r.w, r.h, on ? PET_SLOT : PET_EXIT, on ? PET_CYAN_HI : PET_SLOT_EDGE);
		text_draw(r.x + r.w / 2, r.y + 1, names[t], on ? PET_WHITE : PET_DIM, TEXT_CENTER);
	}
}

/* a button: its panel, lit where just tapped (dim where the burst was spent), its way of L in its corner */
static SDL_Rect button(int w, int h, int item, const PaletteView *v) {
	SDL_Rect r = emote_palette_item(w, h, item);
	bool lit = v->flash == item;
	SDL_Color fill = lit ? (v->spent ? PET_SLOT_EDGE : PET_GOLD) : FACE_BACK, edge = lit ? PET_WHITE : PET_CYAN;
	netpass_panel(r.x, r.y, r.w, r.h, fill, edge);
	fill_rect(r.x + 2, r.y + 1, r.w - 4, 1, lit ? PET_WHITE : PET_NAVY);
	int way = way_of(v, item);
	if (way >= 0) {
		fill_rect(r.x + 2, r.y + 2, 7, 7, PET_GOLD);
		strip_arrow(r.x + 3, r.y + 3, way, INK);
	}
	return r;
}

static void emote(int w, int h, int e, bool letters, const PaletteView *v) {
	SDL_Rect r = button(w, h, e, v);
	Sprite *s = sprite_get(SPR_NPC, BN6_OW_MEGAMAN);
	int anim = square_emote_anim(e, FACING, 0), frame = anim == BN6_OW_ANIM_COLLAPSE ? 2 : 0;
	SDL_Rect in = { r.x + 1, r.y + 1, r.w - 2, r.h - 14 };
	/* (twice his size where the button has room: a display larger than the 3DS's) */
	int k = in.h >= 70 && in.w >= 48 ? 2 : 1;
	if (s) {
		gfx_clip(&in);
		sprite_draw_into(s, anim, frame, r.x + r.w / 2, in.y + in.h - (frame ? 5 : 1) * k, 0, k);
		gfx_clip(NULL);
	}
	name(r.x + r.w / 2, r.y + r.h - 13, letters, item_name(e), v->flash == e ? INK : PET_WHITE);
}

static void phrase(int w, int h, int i, const PaletteView *v) {
	int item = SQUARE_PHRASE + i;
	SDL_Rect r = button(w, h, item, v);
	name(r.x + r.w / 2 + (way_of(v, item) >= 0 ? 4 : 0), r.y + (r.h - 12) / 2, true, item_name(item), v->flash == item ? INK : PET_WHITE);
}

void second_palette_draw(int w, int h, const PaletteView *v) {
	SDL_Rect b = second_frame_rest(w, h, strip_text_pages[0], "NET SQUARE");
	netpass_ground(b.x, b.y, b.w, b.h);
	second_palette_tabs(w, h, v);
	/* (whom Wave, Cheer and Bow go to, at the tabs' right) */
	const char *aim = strip_text_aim(v->aim);
	minifont_draw(b.x + b.w - 4 - minifont_width(aim, 1), b.y + 6, aim, v->aim[0] ? PET_GOLD : PET_DIM, 1);
	bool letters = letters_fit(emote_palette_item(w, h, 0).w);
	for (int e = 0; e < ONLINE_EMOTE_COUNT; ++e) emote(w, h, e, letters, v);
	for (int i = 0; i < ONLINE_PHRASE_COUNT; ++i) phrase(w, h, i, v);
	minifont_draw(b.x + 4, b.y + b.h - 9, strip_text_tap, PET_CYAN_HI, 1);
	/* (the gold arrow's key) */
	int kx = b.x + b.w - 4 - minifont_width(strip_text_on_l, 1);
	fill_rect(kx - 10, b.y + b.h - 11, 7, 7, PET_GOLD);
	strip_arrow(kx - 9, b.y + b.h - 10, 0, INK);
	minifont_draw(kx, b.y + b.h - 9, strip_text_on_l, PET_DIM, 1);
}
