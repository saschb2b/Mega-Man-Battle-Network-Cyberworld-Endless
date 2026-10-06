/* The error a ROM that cannot be used shows (game.h's error_show): where
 * there is no launcher to choose another (a PortMaster handheld, the 3DS,
 * a --rom-dir given), or a ROM that could not be decoded. Where the
 * launcher is (src/launcher/), a missing ROM is its open cartridge slot. */
#include <stdio.h>
#include <string.h>

#include "game.h"
#include "gfx.h"
#include "minifont.h"
#include "platform.h"
#include "rom.h"

static char error_msg[512];

void error_show(const char *msg) {
	snprintf(error_msg, sizeof error_msg, "%s", msg);
	scene_set(&scene_error);
}

/* Lines of `text` at most `cols` (under 64) characters wide, broken at
 * newlines, spaces, and after a slash, dot or hyphen (a long path breaks at
 * its folders); returns how many. */
static int wrap_lines(const char *text, int cols, char out[][64], int max) {
	int n = 0;
	const char *p = text;
	while (*p && n < max) {
		while (*p == ' ') ++p;
		if (!*p) break;
		int len = 0, cut = 0;
		while (p[len] && p[len] != '\n' && len < cols) ++len;
		if (!p[len] || p[len] == '\n') cut = len;
		else {
			for (int i = len; i > 0 && !cut; --i) if (p[i] == ' ' || strchr("/.-", p[i - 1])) cut = i;
			if (!cut) cut = len;
		}
		snprintf(out[n++], 64, "%.*s", cut, p);
		p += cut;
		if (*p == '\n') ++p;
	}
	return n;
}

/* The message, in the ROM's font when there is one, else in the engine's
 * own (a handheld without its ROM showed a row of bars). */
static void error_draw(void) {
	SDL_SetRenderDrawColor(P.renderer, 8, 16, 48, 255);
	SDL_RenderClear(P.renderer);
	if (R.data) {
		text_draw(P.w / 2, 40, "Cyberworld Endless", WHITE, TEXT_CENTER);
		text_draw(P.w / 2, 70, error_msg, WHITE, TEXT_CENTER);
		return;
	}
	/* (in the picture's place: a phone held upright has the canvas below
	 * it for its controls) */
	char lines[12][64];
	int n = wrap_lines(error_msg, 54, lines, 12), x = P.core_x + CORE_W / 2;
	minifont_draw_centered(x, P.core_y + 24, "CYBERWORLD ENDLESS", rgba(120, 200, 248, 255), 2);
	for (int i = 0; i < n; ++i) minifont_draw_centered(x, P.core_y + 56 + i * 8, lines[i], WHITE, 1);
	minifont_draw_centered(x, P.core_y + CORE_H - 20, "START OR B: QUIT", rgba(160, 170, 200, 255), 1);
}

static void error_update(void) {
	if (btn_pressed(BTN_START) || btn_pressed(BTN_B)) P.quit = true;
}

const Scene scene_error = { "error", NULL, error_update, error_draw, NULL };
