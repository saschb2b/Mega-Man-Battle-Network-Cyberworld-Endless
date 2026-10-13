/* The launcher's layout and picture (launcher_state.h). As the second
 * screen's PET (second_frame.c): a dark green band with the screen's name
 * and its three light stripes, the place on a slot at its right, the body
 * framed in green and cyan; in it the two cartridges side by side (one
 * over the other on a screen held upright), a line under each, the note,
 * the saves' lines, the buttons and the keys' line. What does not fit a
 * small canvas is left out in turn: the saves' second line and its first,
 * the keys' line, the note's third and second lines; the large cartridges
 * come where there is room for them. */
#include "launcher_state.h"

#include <stdio.h>
#include <string.h>

#include "controls.h"
#include "game.h"
#include "launcher_art.h"
#include "launcher_roms.h"
#include "pick.h"
#include "pixfont.h"
#include "platform.h"
#include "rom.h"
#include "second_frame.h"
#include "startup.h"

#define HEAD_H 20
#define FRAME 4         /* the body's green and cyan round it */
#define PAD 4
#define GAP 3
#define CAPTION_H 9
#define BUTTON_H 16
#define LINE PIXFONT_LINE

static SDL_Color none(void) { return (SDL_Color){ 0, 0, 0, 0 }; }
static SDL_Color ink_shade(void) { return rgba(0, 49, 74, 255); }

/* whether the keys' line says anything: a keyboard or a controller in hand
 * (a phone's fingers need none) */
static bool hint_wanted(void) { return DESKTOP || platform_pad_present() || P.keyboard_last; }

/* ---- the layout ---- */

/* The saves' line as lines `w` wide: the first as it is (a desktop's path,
 * cut at its start where it is long: its end says where), the rest of a
 * phone's wrapped to two lines at most; how many */
static int saves_split(int w, char lines[3][96]) {
	const char *nl = strchr(L.saves, '\n');
	int len = nl ? (int)(nl - L.saves) : (int)strlen(L.saves), n = 1;
	if (len > 95) snprintf(lines[0], 96, "...%.92s", L.saves + len - 92);
	else snprintf(lines[0], 96, "%.*s", len, L.saves);
	if (nl) n += pixfont_wrap(nl + 1, w, 1, lines + 1, 2);
	char *first = lines[0];
	while (pixfont_width(first, 1) > w && strlen(first) > 8) {
		memmove(first + 3, first + 7, strlen(first + 7) + 1);
		memcpy(first, "...", 3);
	}
	return n;
}

/* the height one column of pieces takes, the cartridges `large` or not,
 * with `lines` of the note, `saves` of the saves' and the keys' line or not */
static int column_h(bool upright, bool large, int lines, int saves, bool hint) {
	int cart = large ? CART_LARGE_H : CART_H, h = 0;
	h += upright ? 2 * (cart + 2 + CAPTION_H) + GAP : cart + 2 + CAPTION_H;
	h += GAP + lines * LINE;
	if (saves) h += GAP + saves * LINE;
	h += GAP + (upright ? 2 * BUTTON_H + 2 + GAP + 4 : BUTTON_H);
	if (hint) h += GAP + LINE - 2;
	return h;
}

/* What a layout holds: the second button (a phone's: its files chosen
 * instead of a folder), the keys' line on a line of its own or beside
 * PLAY (the desktops'), the cartridges' size, the saves' lines and the
 * note's */
typedef struct {
	bool alt, hint, beside, large;
	int saves, lines;
} Fit;

/* ... what fits the body's inside (iw x ih): the large cartridges where all
 * does; else in turn the saves' lines, the keys' line, the note's third
 * and second lines are left out */
static Fit fit(bool upright, int iw, int ih) {
	char lines[3][96];
	Fit f = { .alt = (pick_kinds() & PICK_FILES) != 0, .hint = hint_wanted(), .saves = saves_split(iw, lines), .lines = 3 };
	f.beside = f.hint && !f.alt && !upright;
	bool wide_enough = upright ? iw >= CART_LARGE_W : iw >= 2 * CART_LARGE_W + 3 * GAP;
	f.large = wide_enough && column_h(upright, true, 3, f.saves, f.hint && !f.beside) <= ih;
	while (column_h(upright, f.large, f.lines, f.saves, f.hint && !f.beside) > ih) {
		if (f.saves) --f.saves;
		else if (f.hint && !f.beside) f.hint = false;
		else if (f.lines > 1) --f.lines;
		else break;
	}
	return f;
}

/* the cartridges and their lines from y: one over the other held upright,
 * else side by side; the y under them */
static int place_carts(LauncherLayout *lay, int ix, int iw, int y) {
	int cw = lay->large ? CART_LARGE_W : CART_W, ch = lay->large ? CART_LARGE_H : CART_H, gap = (iw - 2 * cw) / 3;
	for (int s = 0; s < SLOTS; ++s) {
		if (lay->upright) {
			lay->cart[s] = (SDL_Rect){ ix + (iw - cw) / 2, y, cw, ch };
			lay->caption[s] = (SDL_Rect){ ix, y + ch + 2, iw, CAPTION_H };
			y += ch + 2 + CAPTION_H + (s ? 0 : GAP);
		} else {
			lay->cart[s] = (SDL_Rect){ ix + gap + s * (cw + gap), y, cw, ch };
			lay->caption[s] = (SDL_Rect){ lay->cart[s].x - gap / 2, y + ch + 2, cw + gap, CAPTION_H };
		}
	}
	return lay->upright ? y : y + ch + 2 + CAPTION_H;
}

/* PLAY and the second button from y: PLAY full width held upright, the
 * second under it; else side by side, the keys' line beside PLAY where
 * there is no second; the y under them */
static int place_buttons(LauncherLayout *lay, const Fit *f, int ix, int iw, int y) {
	if (lay->upright) {
		lay->play = (SDL_Rect){ ix, y, iw, BUTTON_H + 4 };
		y += BUTTON_H + 4 + GAP + 2;
		if (f->alt) lay->alt = (SDL_Rect){ ix + iw / 6, y, iw - iw / 3, BUTTON_H };
		return y + BUTTON_H;
	}
	int bw = iw / 2 - GAP;
	lay->play = (SDL_Rect){ f->alt || f->beside ? ix + iw - bw : ix + (iw - bw) / 2, y, bw, BUTTON_H };
	if (f->alt) lay->alt = (SDL_Rect){ ix, y, bw, BUTTON_H };
	if (f->beside) lay->hint = (SDL_Rect){ ix, y + (BUTTON_H - PIXFONT_CAP) / 2, iw - bw - GAP, LINE };
	return y + BUTTON_H;
}

void launcher_layout(LauncherLayout *lay) {
	memset(lay, 0, sizeof *lay);
	int st, sl, sb, sr;
	platform_safe_edges(&st, &sl, &sb, &sr);
	int x0 = sl, y0 = st, w = P.w - sl - sr, h = P.h - st - sb;
	lay->upright = h > w;
	lay->head = (SDL_Rect){ x0, y0, w, HEAD_H };
	lay->body = (SDL_Rect){ x0 + FRAME, y0 + HEAD_H + FRAME, w - 2 * FRAME, h - HEAD_H - 2 * FRAME };
	int iw = lay->body.w - 2 * PAD, ih = lay->body.h - 2 * PAD, ix = lay->body.x + PAD, iy = lay->body.y + PAD;
	Fit f = fit(lay->upright, iw, ih);
	lay->large = f.large;
	lay->note_lines = f.lines;
	/* (the spare height shared out: a third over the cartridges, the rest round the buttons) */
	int spare = ih - column_h(lay->upright, f.large, f.lines, f.saves, f.hint && !f.beside);
	int y = place_carts(lay, ix, iw, iy + spare / 3);
	lay->note = (SDL_Rect){ ix, y + GAP, iw, f.lines * LINE };
	y += GAP + f.lines * LINE;
	if (f.saves) {
		lay->saves = (SDL_Rect){ ix, y + GAP, iw, f.saves * LINE };
		y += GAP + f.saves * LINE;
	}
	y = place_buttons(lay, &f, ix, iw, y + GAP + (spare - spare / 3) / 2);
	if (f.hint && !f.beside) lay->hint = (SDL_Rect){ ix, y + GAP, iw, LINE };
}

/* ---- the picture ---- */

/* The band and the frame: second_frame.c's, in the engine's own font */
static void frame(const LauncherLayout *lay) {
	SDL_Rect hd = lay->head, b = lay->body;
	fill_rect(0, 0, P.w, P.h, PET_GREEN);
	fill_rect(0, 0, P.w, hd.y + hd.h, PET_DARK);
	fill_rect(0, hd.y + hd.h, P.w, 1, PET_LINE);
	const char *title = launcher_word(W_TITLE);
	int tx = hd.x + 8, ty = hd.y + (HEAD_H - PIXFONT_CAP) / 2;
	pixfont_draw(tx, ty, title, PET_WHITE, ink_shade(), PIXFONT_LEFT, 1);
	/* (BN6's three light stripes after a screen's name, slanting up) */
	int sx = tx + pixfont_width(title, 1) + 8;
	for (int k = 0; k < 3; ++k)
		for (int y = 0; y < 10; ++y) fill_rect(sx + k * 5 + (9 - y) / 2, hd.y + 5 + y, 2, 1, PET_LINE);
	/* the place: the game's name on a slot at the right */
	const char *place = launcher_word(W_PLACE);
	if (sx + 20 + pixfont_width(place, 1) + 16 > hd.x + hd.w) place = launcher_word(W_PLACE_SHORT);
	int slot_w = pixfont_width(place, 1) + 12, slot_x = hd.x + hd.w - 4 - slot_w;
	second_slot(slot_x, hd.y + 2, slot_w, HEAD_H - 4);
	pixfont_draw(slot_x + 6, ty, place, PET_CYAN_HI, ink_shade(), PIXFONT_LEFT, 1);
	/* the body: its cyan edge, lit along its top, round the PET's navy */
	SDL_Rect edge[4] = { { b.x - 2, b.y - 2, b.w + 4, 2 }, { b.x - 2, b.y + b.h, b.w + 4, 2 }, { b.x - 2, b.y, 2, b.h }, { b.x + b.w, b.y, 2, b.h } };
	fill_rects(edge, 4, PET_CYAN);
	fill_rect(b.x - 1, b.y - 1, b.w + 2, 1, PET_CYAN_HI);
	fill_rect(b.x, b.y, b.w, b.h, PET_NAVY);
}

/* the drop of a cartridge just in: from above, slowing; and its label's flash */
static int drop(int slot, int *flash) {
	int k = L.filled_at[slot] < 0 ? 99 : L.t - L.filled_at[slot];
	*flash = k < 12 ? 200 - k * 16 : 0;
	return k < 10 ? -(10 - k) * (10 - k) / 5 : 0;
}

static void cartridge_or_spot(const LauncherLayout *lay, int slot) {
	SDL_Rect r = lay->cart[slot];
	bool lit = L.focus == (slot == SLOT_BN6 ? FOCUS_BN6 : FOCUS_BN5);
	if (roms_have(slot)) {
		int flash, dy = drop(slot, &flash);
		SDL_Rect at = { r.x, r.y + dy, r.w, r.h };
		if (lit) art_cursor(r, L.t);
		art_cartridge(slot, at, lay->large, flash);
		/* READY on its foot, at the right */
		const char *ready = launcher_word(W_READY);
		int cw = art_chip_width(ready, true);
		art_chip(at.x + at.w - cw - 3, at.y + at.h - CHIP_H - 1, ready, rgba(16, 140, 82, 255), PET_WHITE, true);
		return;
	}
	bool busy = L.busy_slot == slot || (L.busy_slot == SLOTS && slot == SLOT_BN6);
	art_open_spot(r, L.t / 6, lit);
	int cx = r.x + r.w / 2;
	art_plus(cx, r.y + r.h / 2 - 12, lit ? PET_GOLD : PET_CYAN_HI);
	pixfont_draw(cx, r.y + r.h / 2 - 3, slot_short(slot), lit ? PET_WHITE : PET_CYAN_HI, ink_shade(), PIXFONT_CENTER, 2);
	/* its word: what it needs, or (the cursor on it) what A does */
	LauncherWord w = slot == SLOT_BN6 ? W_NEEDED : W_OPTIONAL;
	if (busy) w = W_WAITING;
	else if (lit) w = L.kinds & PICK_FOLDER ? W_CHOOSE_FOLDER : L.kinds & PICK_FILE ? W_CHOOSE_FILE : W_LOOK_AGAIN;
	SDL_Color c = busy || lit ? PET_GOLD : slot == SLOT_BN6 ? rgba(255, 170, 40, 255) : PET_DIM;
	char word[32];
	snprintf(word, sizeof word, "%s%s", launcher_word(w), busy ? &"..."[2 - (L.t / 12) % 3] : "");
	pixfont_draw(cx, r.y + r.h - 13, word, c, ink_shade(), PIXFONT_CENTER, 1);
}

static void captions(const LauncherLayout *lay) {
	for (int s = 0; s < SLOTS; ++s) {
		SDL_Rect c = lay->caption[s];
		bool have = roms_have(s);
		pixfont_draw(c.x + c.w / 2, c.y + 1, slot_caption(s), have ? PET_WHITE : PET_DIM, ink_shade(), PIXFONT_CENTER, 1);
	}
}

static void note(const LauncherLayout *lay) {
	char text[1100], lines[4][96];
	int kind;
	launcher_cursor_note(text, sizeof text, &kind);
	SDL_Color c = kind == NOTE_GOOD ? rgba(140, 255, 150, 255) : kind == NOTE_BAD ? rgba(255, 170, 40, 255) : rgba(214, 244, 255, 255);
	int most = lay->note_lines < 3 ? lay->note_lines : 3, n = pixfont_wrap(text, lay->note.w, 1, lines, most + 1);
	/* (more than there is room for: the last line shown ends in "...") */
	if (n > most) {
		n = most;
		char *last = lines[n - 1];
		size_t k = strlen(last);
		while (k > 0 && pixfont_width(last, 1) + pixfont_width("...", 1) > lay->note.w) last[--k] = 0;
		snprintf(last + k, 96 - k, "...");
	}
	for (int i = 0; i < n; ++i) pixfont_draw(lay->note.x, lay->note.y + i * LINE, lines[i], c, ink_shade(), PIXFONT_LEFT, 1);
}

static void buttons(const LauncherLayout *lay) {
	bool bn6 = roms_have(SLOT_BN6);
	if (lay->alt.w) {
		LauncherWord w = L.kinds & PICK_FILES ? W_FILES_INSTEAD : W_LOOK_AGAIN;
		art_button(lay->alt, launcher_word(w), false, L.focus == FOCUS_ALT, false, false);
	}
	art_button(lay->play, launcher_word(L.from_title ? W_DONE : W_PLAY), true, L.focus == FOCUS_PLAY, !bn6, !L.from_title);
}

static void lines_under(const LauncherLayout *lay) {
	char lines[3][96];
	int n = lay->saves.h ? saves_split(lay->saves.w, lines) : 0;
	for (int i = 0; i < n && (i + 1) * LINE <= lay->saves.h; ++i)
		pixfont_draw(lay->saves.x, lay->saves.y + i * LINE, lines[i], PET_DIM, ink_shade(), PIXFONT_LEFT, 1);
	if (!lay->hint.h) return;
	char a[32], start[32], b[32], hint[120];
	snprintf(a, sizeof a, "%s", controls_word(BTN_A));
	snprintf(start, sizeof start, "%s", controls_word(BTN_START));
	snprintf(b, sizeof b, "%s", controls_word(BTN_B));
	hint_words(a, start, b, L.from_title, false, hint, sizeof hint);
	if (pixfont_width(hint, 1) > lay->hint.w) hint_words(a, start, b, L.from_title, true, hint, sizeof hint);
	pixfont_draw(lay->hint.x + lay->hint.w / 2, lay->hint.y, hint, PET_DIM, ink_shade(), PIXFONT_CENTER, 1);
}

void launcher_draw_all(const LauncherLayout *lay) {
	frame(lay);
	for (int s = 0; s < SLOTS; ++s) cartridge_or_spot(lay, s);
	captions(lay);
	note(lay);
	buttons(lay);
	lines_under(lay);
	/* (the quit prompt, where there is no game font to say it in yet) */
	if (P.quit_prompt > 0 && !R.data) {
		const char *q = quit_words(P.quit_pad);
		int qw = pixfont_width(q, 1) + 16;
		fill_rect((P.w - qw) / 2, P.h / 2 - 10, qw, 20, rgba(0, 0, 0, 220));
		pixfont_draw(P.w / 2, P.h / 2 - 3, q, PET_WHITE, none(), PIXFONT_CENTER, 1);
	}
}
