/* The older net's wait (guest_wait.h). All of it drawn at runtime in BN6's
 * look, none of it from a file: BN6's chat box (chatbox.c) with MegaMan's
 * mugshot and the chat font from the player's ROM; a gauge drawn to the
 * measure of BN6's Custom gauge and a box to its HP box's (captured in a
 * battle: their rims, track, fill and pins, where BN6's battle HUD has
 * them, so BN5's own HUD takes their places as its battle opens), the
 * box's number in the battle font from the ROM; and light streaming over
 * the switch's white in the chat box's blues, as a jack-in's streams. The
 * words are ours, in MegaMan's voice. */
#include "guest_wait.h"

#include <stdint.h>
#include <string.h>

#include "guest_wait_lines.h"
#include "chatbox.h"
#include "gfx.h"
#include "guest.h"
#include "minifont.h"
#include "platform.h"
#include "text.h"

/* It opens once the white has held OPEN_AFTER frames with more than
 * OPEN_LEFT_MS of the boot left at its pace (or that pace not known yet),
 * or OPEN_LATEST frames whatever is left: a boot about done shows the
 * switch's white alone, as every BN5 battle's */
#define OPEN_AFTER 12
#define OPEN_LEFT_MS 700
#define OPEN_LATEST 45
#define BOX_FRAMES 3     /* BN6's chat box opens in three frames, from its middle row out (captured) */
#define HUD_FRAMES 6     /* the HUD drops in from the top */
#define CLOSE_FRAMES 8
#define FADE_FRAMES 16   /* the light comes up */
#define TRACK_W 128      /* the gauge's track, 56-183: the whole boot */

/* BN6's battle HUD's colours (captured in a battle, as chatbox.c's are in
 * a chat) */
#define SLATE rgba(0x39, 0x52, 0x6B, 255)
#define INK   rgba(0x29, 0x29, 0x29, 255)
#define EDGE  rgba(0x7B, 0x9C, 0xDE, 255)
#define TRACK rgba(0x42, 0x42, 0x84, 255)
#define FILL  rgba(0xE7, 0xEF, 0xFF, 255)
#define PALE  rgba(0xCE, 0xE7, 0xFF, 255)
#define SHINE rgba(0xF7, 0xFF, 0xFF, 255)

static struct {
	int held;     /* frames the whole white has held while a battle waits */
	int open;     /* frames since it opened, 0 closed */
	int close;    /* frames since it began to close, 0 none */
	int fill;     /* the gauge's fill, in 1/16 of the track's pixels */
	int pc;       /* the boot's percent, as last read */
	int page;     /* MegaMan's words: what happens, that it happens once, almost done */
	int typed;    /* characters of the page typed, one a frame as BN6's chats type them */
	int stream;   /* how far the light has streamed, in 1/16 pixels */
} W;

#define READ_FRAMES 100   /* a page typed, then read */
#define LAST_AT 85        /* the boot's percent the last page waits for */

static int page_len(int page) {
	int n = 0;
	for (int i = 0; i < 3; ++i) n += (int)strlen(guest_wait_lines[page][i]);
	return n;
}

/* the next page, once this one has been typed and read */
static void page_turn(void) {
	if (W.page == 2 || W.typed < page_len(W.page) + READ_FRAMES) return;
	if (W.pc >= LAST_AT) W.page = 2;
	else if (W.page == 0) W.page = 1;
	else return;
	W.typed = 0;
}

void guest_wait_update(bool waiting, bool white) {
	if (W.close) {
		if (++W.close > CLOSE_FRAMES) memset(&W, 0, sizeof W);
		else W.stream += 48;
		return;
	}
	if (!waiting) {
		/* (the boot done: it closes, its gauge whole, as the battle opens
		 * behind it) */
		if (W.open) { W.close = 1; W.fill = TRACK_W * 16; W.pc = 100; }
		else W.held = 0;
		return;
	}
	if (white) ++W.held;
	if (W.open) ++W.open;
	else {
		int left = guest_boot_left_ms();
		if (W.held < OPEN_AFTER || (left > 0 && left <= OPEN_LEFT_MS && W.held < OPEN_LATEST)) return;
		W.open = 1;
	}
	int pc = guest_boot_progress();
	W.pc = pc < 0 ? 100 : pc;
	/* (the fill catches up with the boot, never ahead of it) */
	int to = W.pc * TRACK_W * 16 / 100;
	if (W.fill < to) W.fill += (to - W.fill + 3) / 4;
	W.stream += 16 + W.pc / 4;
	if (W.open > BOX_FRAMES + 1) ++W.typed;
	page_turn();
}

/* ---- drawing ---- */

/* span a-b of a row and its mirror image (the HUD's gauge is even about
 * the picture's middle, x 119.5) */
static void mirrored(int x0, int y, int a, int b, SDL_Color c) {
	fill_rect(x0 + a, y, b - a + 1, 1, c);
	fill_rect(x0 + CORE_W - 1 - b, y, b - a + 1, 1, c);
}

/* Text in the 3x5 font, two-toned as the HUD's labels are: pale at the
 * top and bottom rows, white between */
static void label(int cx, int y, const char *s, const SDL_Rect *clip) {
	minifont_draw_centered(cx, y, s, PALE, 1);
	SDL_Rect rows = { clip->x, y + 1, clip->w, 3 }, mid;
	if (!SDL_IntersectRect(&rows, clip, &mid)) return;
	SDL_RenderSetClipRect(P.renderer, &mid);
	minifont_draw_centered(cx, y, s, SHINE, 1);
	SDL_RenderSetClipRect(P.renderer, clip);
}

/* The gauge where BN6's Custom gauge stands, rows 6-15 of the picture
 * (its label's tab 0-5 over it), `px` of its track filled */
static void gauge(int x0, int y0, int px, const SDL_Rect *clip) {
	for (int k = 0; k < 2; ++k) {   /* (its upper half, rows 6-10, and its lower mirrored, 15-11) */
		int d = k ? -1 : 1, y = y0 + (k ? 15 : 6);
		fill_rect(x0 + 53, y, 134, 1, SLATE);
		y += d;
		mirrored(x0, y, 51, 53, SLATE);
		fill_rect(x0 + 54, y, 132, 1, INK);
		y += d;
		mirrored(x0, y, 49, 51, SLATE);
		mirrored(x0, y, 52, 55, INK);
		y += d;
		mirrored(x0, y, 48, 49, SLATE);
		mirrored(x0, y, 50, 55, INK);
		y += d;
		mirrored(x0, y, 48, 48, SLATE);
		mirrored(x0, y, 49, 49, INK);
		mirrored(x0, y, 50, 54, EDGE);   /* (its pins) */
		mirrored(x0, y, 55, 55, INK);
	}
	int f = px < 0 ? 0 : px > TRACK_W ? TRACK_W : px;
	fill_rect(x0 + 56 + f, y0 + 8, TRACK_W - f, 6, TRACK);
	if (f) {
		fill_rect(x0 + 56, y0 + 8, f, 1, EDGE);
		fill_rect(x0 + 56, y0 + 13, f, 1, EDGE);
		fill_rect(x0 + 56, y0 + 9, f, 4, f == TRACK_W ? SHINE : FILL);
	}
	/* (its tab, where BN6's says CUSTOM) */
	fill_rect(x0 + 104, y0, 32, 6, INK);
	label(x0 + 120, y0 + 1, "OLD NET", clip);
}

/* The box where BN6's HP box stands, the boot's percent in it */
static void percent(int x0, int y0, int pc) {
	fill_rect(x0 + 2, y0, 44, 16, SHINE);
	fill_rect(x0 + 4, y0 + 1, 40, 14, SLATE);
	text_drawf(x0 + 42, y0 + 2, WHITE, TEXT_RIGHT, "%d%%", pc);
}

static uint32_t mix(uint32_t x) {
	x ^= x >> 16;
	x *= 0x7FEB352Du;
	x ^= x >> 15;
	x *= 0x846CA68Bu;
	return x ^ x >> 16;
}

/* The net's light streaming leftwards over the white between the HUD and
 * the box, `in` of 16 come up: the chat box's blues, a bright head on each */
static void light(int x0, int y0, int in) {
	enum { STREAKS = 18, LANES = 11 };
	for (int i = 0; i < STREAKS; ++i) {
		uint32_t h = mix((uint32_t)i * 0x9E3779B9u + 1);
		int len = 14 + (int)(h % 46), speed = 2 + (int)(h >> 8) % 5, span = CORE_W + len + (int)(h >> 16) % 90;
		int x = CORE_W - (int)(((uint32_t)W.stream * (uint32_t)speed / 16 + (h >> 4)) % (uint32_t)span);
		int y = 24 + (i % LANES) * 7 + (int)(h >> 28) % 3;
		int a = (40 + (int)(h >> 20) % 60) * in / 16;
		fill_rect(x0 + x, y0 + y, len, 1 + (int)(h >> 12) % 2, rgba(0x4A, 0xBD, 0xF7, a));
		fill_rect(x0 + x, y0 + y, 3, 1, rgba(0x08, 0x6B, 0xC6, a));
	}
}

/* BN6's chat box with MegaMan in it, `t` frames open (or closing, its
 * height going as it came) */
static void box(int x0, int y0, int t, bool closing) {
	static const int heights[BOX_FRAMES] = { 8, 28, CHATBOX_H };
	int k = closing ? BOX_FRAMES - 1 - t : t - 1;
	if (k < 0) return;
	int h = heights[k > BOX_FRAMES - 1 ? BOX_FRAMES - 1 : k];
	chatbox_frame(x0 + CHATBOX_X, y0 + CHATBOX_Y + (CHATBOX_H - h) / 2, CHATBOX_W, h);
	if (closing || t <= BOX_FRAMES) return;
	Sprite *face = sprite_get(SPR_MUGSHOT, FACE_MEGAMAN);
	if (face) sprite_draw_frame(face, 0, 0, x0 + CHATBOX_FACE_X, y0 + CHATBOX_FACE_Y, false, 0, 0);
	int n = W.typed;
	for (int i = 0; i < 3 && n > 0; ++i) {
		const char *line = guest_wait_lines[W.page][i];
		chatbox_text(x0 + CHATBOX_TEXT_X, y0 + CHATBOX_TEXT_Y + i * CHATBOX_LINE, line, n);
		n -= (int)strlen(line);
	}
}

void guest_wait_draw(int x0, int y0) {
	if (!W.open) return;
	SDL_Rect clip = { x0, y0, CORE_W, CORE_H };
	SDL_RenderSetClipRect(P.renderer, &clip);
	int t = W.close ? W.close : W.open;
	int in = W.close ? (CLOSE_FRAMES - W.close) * 16 / CLOSE_FRAMES : t >= FADE_FRAMES ? 16 : t * 16 / FADE_FRAMES;
	light(x0, y0, in);
	/* (the HUD drops in, and back up as it closes) */
	int drop = W.close ? -16 * (W.close > HUD_FRAMES ? HUD_FRAMES : W.close) / HUD_FRAMES : -16 + 16 * (t > HUD_FRAMES ? HUD_FRAMES : t) / HUD_FRAMES;
	percent(x0, y0 + drop, W.pc);
	gauge(x0, y0 + drop, W.fill / 16, &clip);
	box(x0, y0, t, W.close != 0);
	SDL_RenderSetClipRect(P.renderer, NULL);
}
