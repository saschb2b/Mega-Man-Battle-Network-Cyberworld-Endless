/* The Arena on the screen (arena_view.h). */
#include "arena_view.h"

#include <stdio.h>
#include <string.h>

#include <SDL.h>

#include "arena_pair.h"
#include "arena_text.h"
#include "gfx.h"
#include "pixfont.h"
#include "platform.h"
#include "protocol.h"
#include "second_frame.h"

#define HEADLESS_SLICE 64   /* the way in's frames a tick without a window (a capture's frames stay the same on every machine) */
#define SLICE          8
#define SLICE_MS       14   /* the way in's time a tick in a window */
#define NOTE_LINES     4
#define BADGE_W        12
#define BADGE_H        11
#define PLATE_Y        17   /* the plate's top: under BN6's label of the other MegaMan */
#define PLATE_H        11
#define PLATE_MOST     62   /* the name's widest: clear of MegaMan's emotion window */

static SDL_Texture *tex;

void arena_view_connect(Arena *a) {
	if (P.headless) { arena_connect(a, HEADLESS_SLICE); return; }
	uint64_t start = SDL_GetPerformanceCounter(), budget = SDL_GetPerformanceFrequency() * SLICE_MS / 1000;
	while (arena_connect(a, SLICE) == ARENA_CONNECTING && SDL_GetPerformanceCounter() - start < budget) {}
}

void arena_view_free(void) {
	if (tex) SDL_DestroyTexture(tex);
	tex = NULL;
}

void arena_view_core(const uint32_t *v) {
#ifdef __3DS__
	const Uint32 format = SDL_PIXELFORMAT_ARGB8888;
#else
	const Uint32 format = SDL_PIXELFORMAT_ABGR8888;
#endif
	if (!tex) {
		tex = SDL_CreateTexture(P.renderer, format, SDL_TEXTUREACCESS_STREAMING, ARENA_W, ARENA_H);
		if (!tex) return;
		SDL_SetTextureScaleMode(tex, SDL_ScaleModeNearest);
		SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_NONE);
	}
	/* (mGBA keeps layer flags in the top byte, which a renderer reads as alpha) */
	static uint32_t px[ARENA_W * ARENA_H];
	for (int i = 0; i < ARENA_W * ARENA_H; ++i) px[i] = v[i] | 0xFF000000u;
	SDL_UpdateTexture(tex, NULL, px, ARENA_W * 4);
	SDL_Rect dst = { P.core_x, P.core_y, ARENA_W, ARENA_H };
	SDL_RenderCopy(P.renderer, tex, NULL, &dst);
}

/* `text` broken at its spaces into lines as wide as the picture: how many */
static int wrap(const char *text, char lines[NOTE_LINES][192]) {
	int n = 0;
	lines[0][0] = 0;
	for (const char *p = text; *p && n < NOTE_LINES;) {
		const char *end = p;
		while (*end && *end != ' ') ++end;
		char word[96];
		snprintf(word, sizeof word, "%.*s", (int)(end - p), p);
		char joined[192];
		snprintf(joined, sizeof joined, "%s%s%s", lines[n], *lines[n] ? " " : "", word);
		if (*lines[n] && text_width(joined) > ARENA_W - 8) {
			if (++n == NOTE_LINES) break;
			snprintf(lines[n], sizeof lines[n], "%s", word);
		} else snprintf(lines[n], sizeof lines[n], "%s", joined);
		p = *end ? end + 1 : end;
	}
	return n < NOTE_LINES ? n + 1 : n;
}

void arena_view_note(const char *line, const char *second) {
	char lines[NOTE_LINES][192];
	int n = second ? wrap(second, lines) : 0, h = 16 + 12 * n;
	int y = P.core_y + ARENA_H - h;
	fill_rect(P.core_x, y, ARENA_W, h, rgba(0, 0, 0, 200));
	text_draw(P.w / 2, y + 3, line, WHITE, TEXT_CENTER);
	for (int i = 0; i < n; ++i) text_draw(P.w / 2, y + 15 + 12 * i, lines[i], rgba(170, 200, 255, 255), TEXT_CENTER);
}

void arena_view_clear(void) { fill_rect(0, 0, P.w, P.h, rgba(0, 0, 0, 255)); }

/* the badge from (x, y): three bars rising, as many lit as the link is steady */
static void badge(int x, int y, ArenaLinkState link, uint32_t t) {
	int lit = link == ARENA_LINK_STEADY ? 3 : link == ARENA_LINK_ROLLING ? 2 : 1;
	SDL_Color on = link == ARENA_LINK_STEADY ? PET_LINE : link == ARENA_LINK_ROLLING ? PET_GOLD : rgba(255, 82, 66, 255);
	if (link == ARENA_LINK_STALLED && t / 15 % 2) on = PET_DIM;
	fill_rect(x, y, BADGE_W, BADGE_H, rgba(0, 49, 74, 200));
	for (int k = 0; k < 3; ++k) {
		int h = 3 + 2 * k;
		fill_rect(x + 2 + k * 3, y + BADGE_H - 2 - h, 2, h, k < lit ? on : PET_SLOT);
	}
}

void arena_view_over(const char *name, bool custom, ArenaLinkState link, uint32_t t) {
	int right = P.core_x + ARENA_W - 2;
	if (!custom) { badge(right - BADGE_W, P.core_y + 2, link, t); return; }
	/* (below BN6's own label of the other MegaMan, at the Custom screen's top right, clear of
	 * MegaMan's emotion window to its left: a long name cut short) */
	char shown[ONLINE_NAME_BYTES + 2];
	snprintf(shown, sizeof shown, "%s", name);
	for (size_t n = strlen(shown); n > 1 && pixfont_width(shown, 1) > PLATE_MOST; --n) snprintf(shown + n - 2, 3, ".");
	int y = P.core_y + PLATE_Y, w = pixfont_width(shown, 1) + 8, x = right - BADGE_W - 2 - w;
	fill_rect(x, y, w, PLATE_H, rgba(0, 49, 74, 220));
	fill_rect(x, y + PLATE_H - 1, w, 1, PET_CYAN);
	pixfont_draw(x + 4, y + 2, shown, PET_WHITE, rgba(0, 49, 74, 255), PIXFONT_LEFT, 1);
	badge(right - BADGE_W, y, link, t);
}

void arena_view_stall(const char *name, uint32_t left, uint32_t total, bool way_out) {
	int h = way_out ? 30 : 22, y = P.core_y + ARENA_H - h;
	fill_rect(P.core_x, y, ARENA_W, h, rgba(0, 0, 0, 200));
	text_draw(P.w / 2, y + 2, arena_waiting_for(name), WHITE, TEXT_CENTER);
	int bw = 120, bx = P.w / 2 - bw / 2;
	fill_rect(bx, y + 16, bw, 3, PET_SLOT);
	if (total) fill_rect(bx, y + 16, (int)((uint64_t)bw * left / total), 3, PET_CYAN_HI);
	if (way_out) pixfont_draw(P.w / 2, y + 21, arena_leave_key, PET_GOLD, rgba(0, 49, 74, 255), PIXFONT_CENTER, 1);
}
