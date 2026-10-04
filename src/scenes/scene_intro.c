/* The start, before the title: the developer's boot screen, then MegaMan
 * asking for word on GitHub. Each start, a few seconds, and any button
 * skips either.
 *
 * The boot screen is a Game Boy's start of our own making: saschb2b's
 * name, in letters drawn for it here, drops down a green screen to its
 * middle and a chime of our own rings (audio_chime). No Nintendo logo, no
 * Nintendo sound. The notice has the title's net behind MegaMan in the
 * game's chat box (its frame, its font and his face from the ROM; the
 * words ours), and the project's address as a QR code and in letters
 * above him, where a phone's camera takes it from the screen. */
#include <string.h>

#include "audio.h"
#include "backdrop.h"
#include "chatbox.h"
#include "game.h"
#include "gfx.h"
#include "minifont.h"
#include "platform.h"
#include "qr.h"
#include "text.h"

#define REPO "https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless"

/* The boot screen: the name drops for DROP frames, rings as it lands and
 * stands until BOOT_END, its last BOOT_FADE frames fading to black. */
#define DROP 48
#define BOOT_END 108
#define BOOT_FADE 10
/* The notice: in from black, its words typed a character a frame, out to
 * black at NOTE_END, where the title fades in. */
#define NOTE_FADE 10
#define NOTE_END 240

/* The name: 5 x 8 letters with a column between, 3 screen pixels to one
 * of theirs, in the middle of the screen once it lands. */
#define NAME "saschb2b"
#define LETTER_W 5
#define LETTER_H 8
#define NAME_K 3
#define NAME_W ((int)(sizeof NAME - 1) * (LETTER_W + 1) * NAME_K - NAME_K)
#define NAME_H (LETTER_H * NAME_K)
#define NAME_Y ((CORE_H - NAME_H) / 2)

/* A Game Boy's green: the screen and the darkest shade */
#define LCD 0xFF9BBC0Fu
#define INK 0xFF0F380Fu

/* The code: 2 screen pixels a module, on a white card with a quiet zone
 * of three modules, at the top left; the address and what to do there
 * beside it */
#define CODE_K 2
#define CODE_QUIET 3
#define CARD_X 10
#define CARD_Y 10
#define SIDE_X 100

static const struct { char ch; const char *rows[LETTER_H]; } letters[] = {
	{ 's', { ".....", ".....", ".....", ".####", "##...", ".###.", "...##", "####." } },
	{ 'a', { ".....", ".....", ".....", ".###.", "...##", ".####", "##.##", ".####" } },
	{ 'c', { ".....", ".....", ".....", ".####", "##...", "##...", "##...", ".####" } },
	{ 'h', { "##...", "##...", "##...", "####.", "##.##", "##.##", "##.##", "##.##" } },
	{ 'b', { "##...", "##...", "##...", "####.", "##.##", "##.##", "##.##", "####." } },
	{ '2', { ".###.", "##.##", "...##", "...##", "..##.", ".##..", "##...", "#####" } },
};

/* MegaMan's three lines, typed in the chat box */
static const char *const said[3] = { "Lan, this net is still", "growing! Bugs? Ideas?", "Tell saschb2b there!" };

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
		if (S.t == DROP) audio_chime();
		if (skip || S.t >= BOOT_END) {
			S.notice = true;
			S.t = 0;
			/* (the title's first net: the notice goes over into it) */
			backdrop_load(&bd, 0x09);
		}
		return;
	}
	if (skip || S.t >= NOTE_END) scene_set(&scene_title);
}

/* ---- the boot screen ---- */

static void letter_stamp(char ch, int x0, int y0) {
	for (size_t i = 0; i < sizeof letters / sizeof *letters; ++i) {
		if (letters[i].ch != ch) continue;
		for (int r = 0; r < LETTER_H; ++r)
			for (int c = 0; c < LETTER_W; ++c) {
				if (letters[i].rows[r][c] != '#') continue;
				for (int dy = 0; dy < NAME_K; ++dy) {
					int y = y0 + r * NAME_K + dy;
					if (y < 0 || y >= CORE_H) continue;
					for (int dx = 0; dx < NAME_K; ++dx) px[y * CORE_W + x0 + c * NAME_K + dx] = INK;
				}
			}
	}
}

static void boot_render(void) {
	for (int i = 0; i < CORE_W * CORE_H; ++i) px[i] = LCD;
	/* (from just above the screen to its middle, steadily, as a Game Boy's
	 * own name comes down) */
	int t = S.t < DROP ? S.t : DROP;
	int y = -NAME_H + (NAME_Y + NAME_H) * t / DROP, x = (CORE_W - NAME_W) / 2;
	for (int i = 0; NAME[i]; ++i) letter_stamp(NAME[i], x + i * (LETTER_W + 1) * NAME_K, y);
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
	static const char *const asks[3] = { "Star the project", "Follow saschb2b", "Report bugs" };
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
	else boot_render();
	if (!S.tex) {
		S.tex = SDL_CreateTexture(P.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, CORE_W, CORE_H);
		SDL_SetTextureScaleMode(S.tex, SDL_ScaleModeNearest);
	}
	SDL_UpdateTexture(S.tex, NULL, px, CORE_W * 4);
	SDL_Rect dst = { x0, y0, CORE_W, CORE_H };
	SDL_RenderCopy(P.renderer, S.tex, NULL, &dst);
	int fade = 0;
	if (S.notice) {
		side_draw(x0, y0);
		chat_draw(x0, y0);
		if (S.t < NOTE_FADE) fade = 16 - S.t * 16 / NOTE_FADE;
		else if (S.t > NOTE_END - NOTE_FADE) fade = (S.t - (NOTE_END - NOTE_FADE)) * 16 / NOTE_FADE;
	} else if (S.t > BOOT_END - BOOT_FADE) fade = (S.t - (BOOT_END - BOOT_FADE)) * 16 / BOOT_FADE;
	if (fade) { P.fx_fade = fade; P.fx_fade_color = BLACK; }
}

const Scene scene_intro = { "intro", enter, update, draw, NULL };
