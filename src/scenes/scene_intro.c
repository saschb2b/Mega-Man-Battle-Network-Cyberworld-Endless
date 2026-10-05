/* The start, before the title: the developer's boot screen, then MegaMan
 * asking for word on GitHub. Each start, a few seconds, and any button
 * skips either.
 *
 * The boot screen is ours (intro_logo.c): the name Saschb2b in pixel
 * letters, near-black on white, comes up as "Sasch" and "b2b" on the two
 * tones of a chime from Pixabay (intro_chime.h). No Nintendo logo, no Nintendo sound. Out
 * to white, from which the notice comes up. The notice has the title's net
 * behind MegaMan in the game's chat box (its frame, its font and his face
 * from the ROM; the words ours), and the project's address as a QR code
 * and in letters above him, where a phone's camera takes it from the
 * screen. */
#include <string.h>

#include "backdrop.h"
#include "chatbox.h"
#include "game.h"
#include "gfx.h"
#include "intro_logo.h"
#include "minifont.h"
#include "platform.h"
#include "qr.h"
#include "text.h"

#define REPO "https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless"

/* The notice: in from the boot screen's white, its words typed a
 * character a frame, out to black at NOTE_END, where the title fades in. */
#define NOTE_FADE 10
#define NOTE_END 240

/* The code: 2 screen pixels a module, on a white card with a quiet zone
 * of three modules, at the top left; the address and what to do there
 * beside it */
#define CODE_K 2
#define CODE_QUIET 3
#define CARD_X 10
#define CARD_Y 10
#define SIDE_X 100

/* MegaMan's three lines, typed in the chat box */
static const char *const said[3] = { "Lan, this net is still", "growing! Bugs? Ideas?", "Tell Saschb2b there!" };

static struct {
	int t;
	bool notice;
	SDL_Texture *tex;
} S;

static Backdrop bd;
static uint8_t code[QR_MAX * QR_MAX];
static int code_size;
static uint32_t px[CORE_W * CORE_H];

static void enter(void) {
	SDL_Texture *tex = S.tex;
	memset(&S, 0, sizeof S);
	S.tex = tex;
	if (!code_size) code_size = qr_make(REPO, code);
}

static void update(void) {
	++S.t;
	bool skip = P.pressed != 0;
	if (!S.notice) {
		intro_logo_sound(S.t);
		if (skip || S.t >= INTRO_LOGO_END) {
			S.notice = true;
			S.t = 0;
			/* (the title's first net: the notice goes over into it) */
			backdrop_load(&bd, 0x09);
		}
		return;
	}
	if (skip || S.t >= NOTE_END) scene_set(&scene_title);
}

/* ---- the notice ---- */

static void code_render(void) {
	int side = (code_size + 2 * CODE_QUIET) * CODE_K;
	for (int y = 0; y < side; ++y)
		for (int x = 0; x < side; ++x) {
			int mx = x / CODE_K - CODE_QUIET, my = y / CODE_K - CODE_QUIET;
			bool dark = mx >= 0 && my >= 0 && mx < code_size && my < code_size && code[my * code_size + mx];
			px[(CARD_Y + y) * CORE_W + CARD_X + x] = dark ? 0xFF000000u : 0xFFFFFFFFu;
		}
}

static void notice_render(void) {
	/* the net, darker than behind the title, so the card and the words stand out */
	uint8_t dim[CORE_H];
	for (int y = 0; y < CORE_H; ++y) dim[y] = (uint8_t)(8 + y * 6 / CORE_H);
	backdrop_draw(&bd, S.t, 0, 0, dim, 1, px);
	if (code_size) code_render();
}

/* the address, and the three things to do there (the ideas are
 * MegaMan's to ask for) */
static void side_draw(int x0, int y0) {
	SDL_Color gold = rgba(255, 230, 90, 255), sky = rgba(170, 200, 255, 255);
	int x = x0 + SIDE_X;
	text_draw(x, y0 + 10, "On GitHub", gold, TEXT_LEFT);
	static const char *const address[3] = { "GITHUB.COM/SASCHB2B/", "MEGA-MAN-BATTLE-NETWORK-", "CYBERWORLD-ENDLESS" };
	for (int i = 0; i < 3; ++i) minifont_draw(x, y0 + 26 + i * 7, address[i], sky, 1);
	static const char *const asks[3] = { "Star the project", "Follow Saschb2b", "Report bugs" };
	for (int i = 0; i < 3; ++i) text_draw(x, y0 + 52 + i * 12, asks[i], WHITE, TEXT_LEFT);
}

static void chat_draw(int x0, int y0) {
	chatbox_frame(x0 + CHATBOX_X, y0 + CHATBOX_Y, CHATBOX_W, CHATBOX_H);
	Sprite *face = sprite_get(SPR_MUGSHOT, FACE_MEGAMAN);
	if (face) sprite_draw_frame(face, 0, 0, x0 + CHATBOX_FACE_X, y0 + CHATBOX_FACE_Y, false, 0, 0);
	int typed = S.t - NOTE_FADE;
	for (int i = 0; i < 3 && typed > 0; ++i) {
		chatbox_text(x0 + CHATBOX_TEXT_X, y0 + CHATBOX_TEXT_Y + i * CHATBOX_LINE, said[i], typed);
		typed -= (int)strlen(said[i]);
	}
}

static void draw(void) {
	fill_rect(0, 0, P.w, P.h, BLACK);
	int x0 = P.core_x, y0 = P.core_y;
	if (S.notice) notice_render();
	else intro_logo_render(px, S.t);
	if (!S.tex) {
		S.tex = SDL_CreateTexture(P.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, CORE_W, CORE_H);
		SDL_SetTextureScaleMode(S.tex, SDL_ScaleModeNearest);
	}
	SDL_UpdateTexture(S.tex, NULL, px, CORE_W * 4);
	SDL_Rect dst = { x0, y0, CORE_W, CORE_H };
	SDL_RenderCopy(P.renderer, S.tex, NULL, &dst);
	int fade = 0;
	SDL_Color to = BLACK;
	if (S.notice) {
		side_draw(x0, y0);
		chat_draw(x0, y0);
		if (S.t < NOTE_FADE) { fade = 16 - S.t * 16 / NOTE_FADE; to = WHITE; }
		else if (S.t > NOTE_END - NOTE_FADE) fade = (S.t - (NOTE_END - NOTE_FADE)) * 16 / NOTE_FADE;
	} else if (S.t < INTRO_LOGO_IN) fade = 16 - S.t * 16 / INTRO_LOGO_IN;
	else if (S.t > INTRO_LOGO_END - INTRO_LOGO_OUT) { fade = (S.t - (INTRO_LOGO_END - INTRO_LOGO_OUT)) * 16 / INTRO_LOGO_OUT; to = WHITE; }
	if (fade) { P.fx_fade = fade; P.fx_fade_color = to; }
}

const Scene scene_intro = { "intro", enter, update, draw, NULL };
