/* emote_palette.h. */
#include "emote_palette.h"

#include <stdio.h>
#include <string.h>

#include "bn6_fields.h"
#include "debug.h"
#include "emu.h"
#include "platform.h"
#include "second_frame.h"
#include "square_emote.h"
#include "talk.h"

#define FLASH 24       /* frames a tapped button stays lit */
#define TAB_H 16
#define HINT_H 14
#define GAP 2
#define PHRASE_H 18

static struct {
	int tab, flash, flash_t;
	bool spent;
	char aim[ONLINE_NAME_BYTES + 1];
} Q;

void emote_palette_reset(void) {
	memset(&Q, 0, sizeof Q);
	Q.flash = -1;
	/* (a tap from before the Square goes nowhere) */
	int x, y, w, h;
	(void)platform_second_tap(&x, &y, &w, &h);
}

/* the body's rows: the tabs, two rows of six emotes, four of two phrases
 * (their words in the game's letters, whole) */
static void rows(int w, int h, SDL_Rect *body, int *emote_h, int *phrase_h) {
	*body = second_frame_rest_body(w, h);
	int room = body->h - TAB_H - HINT_H - 6 * GAP;
	*phrase_h = room / 10 > PHRASE_H ? room / 10 : PHRASE_H;
	*emote_h = (room - 4 * *phrase_h) / 2;
}

SDL_Rect emote_palette_tab(int w, int h, int tab) {
	SDL_Rect b;
	int eh, ph;
	rows(w, h, &b, &eh, &ph);
	return (SDL_Rect){ b.x + 2 + tab * 72, b.y + 2, 70, TAB_H - 2 };
}

SDL_Rect emote_palette_item(int w, int h, int item) {
	SDL_Rect b;
	int eh, ph;
	rows(w, h, &b, &eh, &ph);
	int x = b.x + 2, y = b.y + TAB_H + GAP, inner = b.w - 4;
	if (item < SQUARE_PHRASE) {
		int cw = (inner - 5 * GAP) / 6;
		return (SDL_Rect){ x + item % 6 * (cw + GAP), y + item / 6 * (eh + GAP), cw, eh };
	}
	int i = item - SQUARE_PHRASE, cw = (inner - GAP) / 2;
	return (SDL_Rect){ x + i % 2 * (cw + GAP), y + 2 * (eh + GAP) + i / 2 * (ph + GAP), cw, ph };
}

static bool inside(SDL_Rect r, int x, int y) { return x >= r.x && y >= r.y && x < r.x + r.w && y < r.y + r.h; }

void emote_palette_frame(const OnlineSnapshot *s) {
	if (Q.flash_t && !--Q.flash_t) { Q.flash = -1; platform_second_screen_soon(); }
	/* (whom an aimed emote goes to, named on the palette) */
	uint32_t faced = square_emote_faced(s), aim = square_emote_aim(0, faced, SDL_GetTicks());
	Q.aim[0] = 0;
	for (size_t i = 0; aim && i < s->peer_count; ++i)
		if (s->peers[i].id == aim) snprintf(Q.aim, sizeof Q.aim, "%s", s->peers[i].name);
	int x, y, w, h;
	if (!platform_second_tap(&x, &y, &w, &h)) return;
	for (int t = 0; t < 2; ++t)
		if (inside(emote_palette_tab(w, h, t), x, y) && Q.tab != t) { Q.tab = t; platform_second_screen_soon(); return; }
	/* (a chat box open: BN6 has MegaMan, the tap goes nowhere) */
	if (Q.tab || talk_busy() || emu_read8(BN6_CHATBOX)) return;
	for (int item = 0; item < SQUARE_ITEMS; ++item) {
		if (!inside(emote_palette_item(w, h, item), x, y)) continue;
		Q.spent = !square_emote_send_item(item, faced, SDL_GetTicks());
		Q.flash = item;
		Q.flash_t = FLASH;
		platform_second_screen_soon();
		if (emu_debug_on()) fprintf(stderr, "online: the palette sent item %d%s\n", item, Q.spent ? " (the burst spent)" : "");
		return;
	}
}

void emote_palette_view(PaletteView *v) {
	memset(v, 0, sizeof *v);
	v->tab = Q.tab;
	v->flash = Q.flash;
	v->spent = Q.spent;
	for (int k = 0; k < 4; ++k) v->favourites[k] = square_emote_favourite(k);
	snprintf(v->aim, sizeof v->aim, "%s", Q.aim);
}
