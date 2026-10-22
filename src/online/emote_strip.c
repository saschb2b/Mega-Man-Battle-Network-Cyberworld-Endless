/* emote_strip.h. */
#include "emote_strip.h"

#include <stdio.h>
#include <string.h>

#include "audio.h"
#include "bn6.h"
#include "debug.h"
#include "emote_palette.h"
#include "emote_strip_draw.h"
#include "gfx.h"
#include "platform.h"
#include "square_emote.h"

#define FACING 3        /* the preview's MegaMan faces the player (BN6's facing toward the screen's lower left) */
#define REST 36         /* frames he stands between two plays of the pose */
#define KEPT_SHOWN 50   /* frames a favourite just kept is lit */

static struct {
	bool shown;
	bool letting_go;        /* closed, its last key still held: kept from BN6 (a new press there) */
	bool select_down;       /* SELECT held in the strip ... */
	bool select_used;       /* ... and a favourite kept with it (its release does not close the strip) */
	int page, cursor[2];
	int kept_t;
	Anim pose;
	StripView view;
} T;

void emote_strip_reset(void) {
	emote_palette_reset();
	platform_picture_taps(false);
	memset(&T, 0, sizeof T);
	T.cursor[1] = SQUARE_PHRASE;
	T.view.kept = -1;
}

bool emote_strip_shown(void) { return T.shown; }

void emote_strip_open(void) {
	T.shown = true;
	/* (the opening SELECT still down: a direction with it keeps, its release leaves the strip open) */
	T.select_down = btn_held(BTN_SELECT);
	T.select_used = true;
	T.view.t = 0;
	T.view.kept = -1;
	T.kept_t = 0;
	memset(&T.pose, 0, sizeof T.pose);
	platform_picture_taps(true);
	audio_sfx(SFX_CUSTOM_OPEN);
	if (emu_debug_on()) fprintf(stderr, "online: the emote strip opened on item %d\n", T.cursor[T.page]);
}

void emote_strip_close(void) {
	if (!T.shown) return;
	T.shown = false;
	T.letting_go = true;
	platform_picture_taps(false);
}

/* the item under the cursor sent, the strip closed: the Square's answer seen whole */
static void send(int item, const OnlineSnapshot *s) {
	bool ok = square_emote_send_item(item, square_emote_faced(s), SDL_GetTicks());
	if (emu_debug_on()) fprintf(stderr, "online: the strip sent item %d%s\n", item, ok ? "" : " (the burst spent)");
	if (!ok) audio_sfx(SFX_ERROR);
	else if (item >= SQUARE_PHRASE) audio_sfx(SFX_CHIP_OK);
	emote_strip_close();
}

/* the cursor moved by (dx, dy) on its page's grid, round at the ends */
static void move(int dx, int dy) {
	int page = T.page, first = page ? SQUARE_PHRASE : 0, i = T.cursor[page] - first;
	int cols = STRIP_COLS(page), rows = STRIP_ROWS(page);
	int col = (i % cols + dx + cols) % cols, row = (i / cols + dy + rows) % rows;
	T.cursor[page] = first + row * cols + col;
	T.view.t = 0;
	audio_sfx(SFX_CHIP_CURSOR);
}

static void turn(int page) {
	if (page == T.page) return;
	T.page = page;
	T.view.t = 0;
	audio_sfx(SFX_CHIP_CURSOR);
}

/* SELECT held: a direction keeps the item under the cursor on L and it */
static void keep(int way) {
	square_emote_set_favourite(way, T.cursor[T.page]);
	T.select_used = true;
	T.view.kept = way;
	T.kept_t = KEPT_SHOWN;
	audio_sfx(SFX_CHIP_SELECT);
}

static const uint32_t ways[4] = { BTN_UP, BTN_RIGHT, BTN_DOWN, BTN_LEFT };

static void keys(const OnlineSnapshot *s) {
	if (btn_pressed(BTN_SELECT)) { T.select_down = true; T.select_used = false; }
	if (T.select_down) {
		for (int k = 0; k < 4; ++k) if (btn_pressed(ways[k])) keep(k);
		if (btn_held(BTN_SELECT)) return;
		T.select_down = false;
		if (!T.select_used) { audio_sfx(SFX_CANCEL); emote_strip_close(); }
		return;
	}
	static const int dx[4] = { 0, 1, 0, -1 }, dy[4] = { -1, 0, 1, 0 };
	for (int k = 0; k < 4; ++k) if (btn_repeat(ways[k])) move(dx[k], dy[k]);
	if (btn_pressed(BTN_L)) turn(0);
	if (btn_pressed(BTN_R)) turn(1);
	if (btn_pressed(BTN_A)) send(T.cursor[T.page], s);
	else if (btn_pressed(BTN_B)) { audio_sfx(SFX_CANCEL); emote_strip_close(); }
}

static bool inside(SDL_Rect r, int x, int y) { return x >= r.x && y >= r.y && x < r.x + r.w && y < r.y + r.h; }

/* a tap on the picture: an icon sends, a tab turns, a favourite sends, the window closes */
static void taps(const OnlineSnapshot *s) {
	int x, y;
	if (!T.shown || !platform_tap(&x, &y)) return;
	x -= P.core_x;
	y -= P.core_y;
	if (inside(strip_preview(), x, y)) { audio_sfx(SFX_CANCEL); emote_strip_close(); return; }
	for (int p = 0; p < 2; ++p) if (inside(strip_tab(p), x, y)) { turn(p); return; }
	int first = T.page ? SQUARE_PHRASE : 0, n = STRIP_COLS(T.page) * STRIP_ROWS(T.page);
	for (int i = 0; i < n; ++i)
		if (inside(strip_cell(T.page, i), x, y)) { T.cursor[T.page] = first + i; send(first + i, s); return; }
	for (int k = 0; k < 4; ++k) if (inside(strip_favourite(k), x, y)) { send(square_emote_favourite(k), s); return; }
}

/* the view the draw reads: the preview's pose played on (the emote's
 * animation as square_emote.c plays it, then MegaMan standing), whom it
 * goes to, the favourites */
static void view(const OnlineSnapshot *s) {
	StripView *v = &T.view;
	v->page = T.page;
	v->cursor = T.cursor[T.page];
	++v->t;
	if (T.kept_t && !--T.kept_t) v->kept = -1;
	for (int k = 0; k < 4; ++k) v->favourites[k] = square_emote_favourite(k);
	int emote = v->cursor < SQUARE_PHRASE ? v->cursor : -1, frames = square_emote_frames(emote);
	int into = frames ? v->t % (frames + REST) : frames;
	int anim = emote >= 0 && into < frames ? square_emote_anim(emote, FACING, into) : FACING;
	Sprite *spr = sprite_get(SPR_NPC, BN6_OW_MEGAMAN);
	if (spr && (!T.pose.spr || T.pose.anim != anim)) anim_play(&T.pose, spr, anim);
	else if (spr) anim_update(&T.pose);
	v->anim = T.pose.anim;
	v->frame = T.pose.frame;
	uint32_t aim = square_emote_aim(emote, square_emote_faced(s), SDL_GetTicks());
	v->aims = emote >= 0 && (ONLINE_EMOTE_AIMED >> emote & 1);
	v->aim[0] = 0;
	for (size_t i = 0; aim && i < s->peer_count; ++i)
		if (s->peers[i].id == aim) snprintf(v->aim, sizeof v->aim, "%s", s->peers[i].name);
}

bool emote_strip_update(const OnlineSnapshot *s) {
	emote_palette_frame(s);
	if (!T.shown) {
		if (T.letting_go && btn_held(BTN_A | BTN_B | BTN_SELECT | BTN_L | BTN_R)) return true;
		T.letting_go = false;
		return false;
	}
	keys(s);
	taps(s);
	if (T.shown) view(s);
	return true;
}

void emote_strip_draw(void) {
	if (T.shown) strip_draw(P.core_x, P.core_y, &T.view);
}
