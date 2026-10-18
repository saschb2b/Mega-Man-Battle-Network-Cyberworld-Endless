/* The Arena on the screen (arena_view.h). */
#include "arena_view.h"

#include <stdio.h>

#include <SDL.h>

#include "arena_pair.h"
#include "arena_text.h"
#include "gfx.h"
#include "platform.h"

#define HEADLESS_SLICE 64   /* the way in's frames a tick without a window (a capture's frames stay the same on every machine) */
#define SLICE          8
#define SLICE_MS       14   /* the way in's time a tick in a window */
#define NOTE_LINES     4

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

void arena_view_connecting(int progress) {
	int cx = P.w / 2, y = P.core_y + 52;
	text_draw(cx, y, arena_title, rgba(170, 200, 255, 255), TEXT_CENTER);
	text_draw(cx, y + 16, arena_connecting, WHITE, TEXT_CENTER);
	int w = 160, done = w * progress / 100;
	fill_rect(cx - w / 2, y + 36, w, 6, rgba(40, 60, 90, 255));
	fill_rect(cx - w / 2, y + 36, done, 6, rgba(120, 200, 255, 255));
}

void arena_view_clear(void) { fill_rect(0, 0, P.w, P.h, rgba(0, 0, 0, 255)); }

void arena_view_result(unsigned result, bool to_square) {
	arena_view_note(result == 1 ? arena_won : result == 2 ? arena_lost : arena_ended, to_square ? arena_back_square : arena_back);
}

void arena_view_stopped(const char *why) { arena_view_note(arena_stopped, why); }

void arena_view_waiting(void) { arena_view_note(arena_waiting, NULL); }
