/* saves_draw.h. Drawn in the engine's own letters (pixfont.h), as the ROMs
 * screen, so that SAVES reads before the game's font is (the launcher) and
 * after. A draw reads SV and the copy's status, and changes nothing. */
#include "saves_draw.h"

#include <stdio.h>
#include <string.h>

#include "backup.h"
#include "gfx.h"
#include "mirror.h"
#include "pick.h"
#include "pet_look.h"
#include "pixfont.h"
#include "platform.h"
#include "saves_actions.h"
#include "saves_cloud.h"
#include "saves_home.h"
#include "saves_layout.h"
#include "saves_state.h"
#include "saves_text.h"
#include "second_frame.h"

static SDL_Color shade(void) { return rgba(0, 49, 74, 255); }

static void words(int x, int y, const char *s, SDL_Color c) { pixfont_draw(x, y, s, c, shade(), PIXFONT_LEFT, 1); }

/* (one line, cut with "..." where it is wider than `width`) */
static void clipped(int x, int y, const char *s, int width, SDL_Color c) {
	char line[160];
	snprintf(line, sizeof line, "%s", s);
	size_t n = strlen(line);
	if (pixfont_width(line, 1) > width) {
		while (n && pixfont_width(line, 1) + pixfont_width("...", 1) > width) line[--n] = 0;
		snprintf(line + n, sizeof line - n, "...");
	}
	words(x, y, line, c);
}

/* (the note's lines, wrapped at `r`'s width, as many as it has room for) */
static void note_in(SDL_Rect r, const char *text, SDL_Color color, SDL_Color under) {
	char lines[8][96];
	int rows = (r.h + PIXFONT_LINE - PIXFONT_H) / PIXFONT_LINE, k = 0;
	if (rows > 8) rows = 8;
	char part[700];
	snprintf(part, sizeof part, "%s", text);
	for (char *p = strtok(part, "\n"); p && k < rows; p = strtok(NULL, "\n"))
		k += pixfont_wrap(p, r.w, 1, lines + k, rows - k);
	for (int i = 0; i < k; ++i) pixfont_draw(r.x, r.y + i * PIXFONT_LINE, lines[i], color, under, PIXFONT_LEFT, 1);
}

static void note(SDL_Rect r, const char *text, SDL_Color color) { note_in(r, text, color, shade()); }

static void button(SavesButton b) {
	SDL_Rect r = b.rect;
	/* (STATUS's pills; the one with the focus NaviCust's green RUN!, as BN6 marks its one action) */
	if (SV.focus == b.action) look_button(r.x, r.y, r.w - 1, r.h - 1);
	else look_pill(r.x, r.y, r.w);
	const char *label = saves_label(b.action);
	int w = pixfont_width(label, 1);
	if (w <= r.w - 10) pixfont_draw(r.x + r.w / 2, r.y + (r.h - PIXFONT_H) / 2, label, PET_WHITE, shade(), PIXFONT_CENTER, 1);
	else clipped(r.x + 5, r.y + (r.h - PIXFONT_H) / 2, label, r.w - 10, PET_WHITE);
}

static void panel(SDL_Rect r, const BackupInfo *info, bool file, bool damaged, bool source) {
	look_box(r.x, r.y, r.w, r.h, LOOK_NAVY);
	clipped(r.x + 4, r.y + 3, saves_panel_title(file), r.w - 8, PET_GOLD);
	char rows[4][64];
	int n = saves_rows(info, damaged, source, rows);
	int room = (r.h - 14) / PIXFONT_LINE + ((r.h - 14) % PIXFONT_LINE >= PIXFONT_H);
	for (int i = 0; i < n && i < room; ++i) clipped(r.x + 4, r.y + 14 + i * PIXFONT_LINE, rows[i], r.w - 8, i ? PET_WHITE : PET_CYAN_HI);
}

/* (a code in letters twice the size, centred in `r`'s width) */
static void big_code(SDL_Rect r, int y, const char *code, SDL_Color c) {
	pixfont_draw(r.x + r.w / 2, y, code, c, shade(), PIXFONT_CENTER, 2);
}

/* SEND's view: what is sent, its code and how long it holds, or why not */
static void send_view(const SavesLayout *l) {
	panel(l->here, &SV.here, false, SV.unreadable, false);
	const TransferState s = SV.transfer;
	char line[200], shown[16];
	bool held = (int32_t)(s.until - SDL_GetTicks()) > 0;
	saves_code_words(&s, false, !held, line, sizeof line);
	if (s.phase == TRANSFER_SENT && held) {
		saves_code_shown(s.code, shown, sizeof shown);
		big_code(l->code, l->code.y + 2, shown, PET_CYAN_HI);
		char left[96];
		saves_code_left(s.until - SDL_GetTicks(), left, sizeof left);
		pixfont_draw(l->code.x + l->code.w / 2, l->code.y + 22, left, PET_WHITE, shade(), PIXFONT_CENTER, 1);
	}
	note(l->line, line, s.phase == TRANSFER_FAILED ? PET_GOLD : PET_WHITE);
}

/* RECEIVE's view: the digits typed so far, and how the asking goes */
static void receive_view(const SavesLayout *l) {
	const TransferState s = SV.transfer;
	SDL_Rect r = l->line;
	note((SDL_Rect){ r.x, r.y, r.w, PIXFONT_LINE * 2 }, saves_code_prompt(), PET_WHITE);
	char typed[16];
	saves_code_typed(SV.code, typed, sizeof typed);
	look_box(r.x, r.y + 2 * PIXFONT_LINE + 2, r.w, 24, LOOK_NAVY);
	big_code(r, r.y + 2 * PIXFONT_LINE + 6, typed, PET_CYAN_HI);
	char line[200];
	saves_code_words(&s, true, false, line, sizeof line);
	int below = r.y + 2 * PIXFONT_LINE + 30;
	note((SDL_Rect){ r.x, below, r.w, r.y + r.h - below }, line, s.phase == TRANSFER_FAILED ? PET_GOLD : PET_WHITE);
}

static SavesLayout layout(void) {
	int actions[SAVES_ACTIONS];
	int n = saves_actions(actions);
	if (SV.view != SAVES_HOME) return saves_layout(P.w, P.h, SV.view, actions, n);
	SavesPage pages[SAVES_PAGES];
	return saves_home_layout(P.w, P.h, saves_pages(pages), actions, n);
}

/* (home's page shown, its place among the pages this system has) */
static int page_index(const SavesPage *pages, int n) {
	for (int i = 0; i < n; ++i) if (pages[i] == SV.page) return i;
	return 0;
}

/* (the information box's white page: its words this far in from the
 * sides and the top, as Options' are, TEXT_IN) */
#define PAGE_IN 6
#define PAGE_TOP 2

/* KeyItem's information box as Options draws it: INFORMATION in its top
 * band, the white page under it, 2 in from its sides and 3 from its foot:
 * the page */
static SDL_Rect info_page(SDL_Rect b) {
	look_info(b.x, b.y, b.w, b.h, NULL);
	look_caps(b.x + 4, b.y + 2, saves_word_information(), LOOK_PAGE, (SDL_Color){ 0, 0, 0, 0 });
	SDL_Rect page = { b.x + 2, b.y + 2 + LOOK_CAPS_H + 2, b.w - 4, b.h - LOOK_CAPS_H - 7 };
	fill_rect(page.x, page.y, page.w, page.h, LOOK_PAGE);
	return page;
}

/* (START:UNDO IMPORT at the band's right end, where an import can be undone) */
static SDL_Rect undo_hint(void) {
	const char *what = saves_word_undo();
	int w = look_hint_width("START", what), x = P.w - 8 - w;
	return (SDL_Rect){ x - 2, 4, w + 4, 12 };
}

/* This device's run and record in a slim strip, named THIS DEVICE (the
 * cloud's page shows it too): the name and the run, then the record and,
 * where it fits, the area */
static void strip(SDL_Rect r) {
	SDL_Rect f = look_box(r.x, r.y, r.w, r.h, LOOK_NAVY);
	char rows[4][64];
	int n = saves_rows(&SV.here, SV.unreadable, false, rows), at = f.y + 1;
	const char *name = saves_word_device();
	int left = f.x + 4, right = f.x + f.w - 4;
	look_caps(left, at + 2, name, PET_GOLD, LOOK_SHADE);
	/* (the run, or the record where there is none, after the name) */
	int room = right - left - look_caps_width(name) - 8;
	clipped(right - (pixfont_width(rows[0], 1) < room ? pixfont_width(rows[0], 1) : room), at, rows[0], room, PET_CYAN_HI);
	if (r.h < 20 || n < 2) return;
	/* (a tall strip's second line: the record, the area at its right where both fit) */
	const char *record = rows[n - 1];
	at += PIXFONT_LINE;
	clipped(left, at, record, right - left, PET_WHITE);
	if (n == 3 && pixfont_width(record, 1) + pixfont_width(rows[1], 1) + 8 <= right - left)
		pixfont_draw(right, at, rows[1], PET_WHITE, shade(), PIXFONT_RIGHT, 1);
}

/* A page's line: its name, its value at the right (a value Left and Right
 * turn between Options' small orange arrows), the cursor's arrow */
static void line(SavesButton b) {
	SDL_Rect r = b.rect;
	int ty = r.y + (r.h - PIXFONT_H) / 2;
	if (SV.focus == b.action && saves_home_asking() < 0) look_cursor(r.x + 3, ty - 1);
	pixfont_draw(r.x + 15, ty, saves_line_label(b.action), PET_WHITE, shade(), PIXFONT_LEFT, 1);
	char value[96];
	if (!saves_line_value(b.action, value, sizeof value)) return;
	int right = r.x + r.w - (b.action == SAVES_AUTO ? 12 : 5);
	pixfont_draw(right, ty, value, b.action == SAVES_AUTO ? PET_GOLD : PET_CYAN_HI, shade(), PIXFONT_RIGHT, 1);
	if (b.action != SAVES_AUTO) return;
	look_pointer(right - pixfont_width(value, 1) - 4, ty, 4, true, LOOK_ORANGE);
	look_pointer(right + 4, ty, 4, false, LOOK_ORANGE);
}

/* (Yes and No after the question's last words where they fit on its
 * line, else on the line under them, in the box's page `r`) */
static SDL_Rect answer_rect(SDL_Rect r, bool yes) {
	char text[200], lines[8][96];
	saves_ask_words(saves_home_asking(), text, sizeof text);
	int k = pixfont_wrap(text, r.w - 2 * PAGE_IN, 1, lines, 8);
	int yes_w = pixfont_width(saves_word_yes(), 1), no_w = pixfont_width(saves_word_no(), 1);
	int both = 2 * (LOOK_CHOICE_W + 3) + yes_w + 12 + no_w;
	int last = k ? pixfont_width(lines[k - 1], 1) : 0, x = r.x + PAGE_IN + last + 10;
	int y = r.y + PAGE_TOP + (k ? k - 1 : 0) * PIXFONT_LINE;
	if (!k || x + both > r.x + r.w - PAGE_IN) {
		x = r.x + PAGE_IN;
		y += k ? PIXFONT_LINE : 0;
	}
	if (!yes) x += LOOK_CHOICE_W + 3 + yes_w + 12;
	return (SDL_Rect){ x, y - 2, (yes ? yes_w : no_w) + LOOK_CHOICE_W + 3, PIXFONT_LINE + 2 };
}

/* The box's question: its words, then Yes and No, BN6's green triangle
 * before the one chosen */
static void question(SDL_Rect page) {
	char text[200];
	saves_ask_words(saves_home_asking(), text, sizeof text);
	note_in((SDL_Rect){ page.x + PAGE_IN, page.y + PAGE_TOP, page.w - 2 * PAGE_IN, page.h - PAGE_TOP - 1 }, text, LOOK_INK, LOOK_INK_SHADE);
	for (int k = 0; k < 2; ++k) {
		SDL_Rect a = answer_rect(page, k == 0);
		pixfont_draw(a.x + LOOK_CHOICE_W + 3, a.y + 2, k == 0 ? saves_word_yes() : saves_word_no(), LOOK_INK, LOOK_INK_SHADE, PIXFONT_LEFT, 1);
		if ((k == 0) == saves_home_yes()) look_choice(a.x, a.y - 1);
	}
}

/* Home: the pages' tabs, this device's strip, the page's lines, and in
 * the information box what the line under the cursor does (or what a
 * picker said), how the cloud or the copy stands */
static void home_view(const SavesLayout *l) {
	SavesPage pages[SAVES_PAGES];
	int n = saves_pages(pages);
	const char *names[SAVES_PAGES];
	for (int i = 0; i < n; ++i) names[i] = saves_page_name(pages[i]);
	if (n > 1) look_tabs(l->tabs.x, l->tabs.y, l->tabs.w, n, page_index(pages, n), names);
	if (SV.undo) {
		SDL_Rect h = undo_hint();
		look_hint(h.x + 2, 8, "START", saves_word_undo());
	}
	strip(l->here);
	look_window(l->list.x, l->list.y, l->list.w, l->list.h, 0, false);
	for (int i = 0; i < l->count; ++i) line(l->buttons[i]);
	SDL_Rect page = info_page(l->info);
	if (saves_home_asking() >= 0) {
		question(page);
		return;
	}
	char text[400];
	if (SV.said[0]) snprintf(text, sizeof text, "%s", SV.said);
	else saves_line_note(SV.focus, text, sizeof text);
	/* (the page's ink, KeyItem's, its light shade on the white, in from its edges) */
	note_in((SDL_Rect){ page.x + PAGE_IN, page.y + PAGE_TOP, page.w - 2 * PAGE_IN, page.h - PAGE_TOP - 1 }, text, LOOK_INK, LOOK_INK_SHADE);
}

void saves_draw(void) {
	/* (BN6's PET screens' band, as the ROMs screen's: pet_look.h) */
	fill_rect(0, 0, P.w, P.h, LOOK_GREEN);
	fill_rect(0, 0, P.w, 20, LOOK_BAND);
	/* (the view's name, a back arrow before it where B goes back: its tap
	 * too, as a phone without a Back key has nothing else) */
	const char *title = saves_view_title();
	bool backs = SV.view == SAVES_HOME || SV.view == SAVES_SEND_CODE || SV.view == SAVES_RECEIVE_CODE;
	if (backs) look_arrow(4, 6, false);
	look_title(backs ? 15 : 8, 6, title, LOOK_PAGE);
	int sx = 29 + look_title_width(title);
	/* (the stripes stop short of a hint at the band's right end) */
	int right = SV.view == SAVES_HOME && SV.undo ? undo_hint().x - 6 : P.w - 4;
	if (right > sx) look_stripes(sx > P.w / 2 ? sx : P.w / 2, 0, right, 20);
	SavesLayout l = layout();
	if (SV.view == SAVES_SEND_CODE) send_view(&l);
	else if (SV.view == SAVES_RECEIVE_CODE) receive_view(&l);
	else if (SV.view == SAVES_HOME) {
		home_view(&l);
		return;
	} else if (SV.view == SAVES_COMPARE) {
		panel(l.here, &SV.here, false, SV.unreadable, true);
		panel(l.file, &SV.file, true, SV.status == BACKUP_DAMAGED, true);
		note(l.line, SV.note, SV.status == BACKUP_OK ? PET_WHITE : PET_GOLD);
	} else {
		fill_rect(6, l.line.y - 4, P.w - 12, l.line.h + 1, PET_NAVY);
		note(l.line, SV.note, PET_WHITE);
	}
	for (int i = 0; i < l.count; ++i) button(l.buttons[i]);
}

void saves_move_focus(int dx, int dy) {
	SavesLayout l = layout();
	SV.focus = saves_layout_move(&l, SV.focus, dx, dy);
}

/* (home's taps: a tab turns to its page, the hint undoes) */
static bool home_tap(SDL_Point p, const SavesLayout *l, void (*turn)(int step)) {
	SavesPage pages[SAVES_PAGES];
	int n = saves_pages(pages), at = page_index(pages, n);
	for (int i = 0; i < l->tab_count; ++i)
		if (SDL_PointInRect(&p, &l->tab[i])) {
			if (i != at) turn(i - at);
			return true;
		}
	if (saves_home_asking() >= 0) {
		SDL_Rect b = l->info, page = { b.x + 2, b.y + 2 + LOOK_CAPS_H + 2, b.w - 4, b.h - LOOK_CAPS_H - 7 };
		for (int k = 0; k < 2; ++k) {
			SDL_Rect a = answer_rect(page, k == 0);
			if (SDL_PointInRect(&p, &a)) { saves_home_answer(k == 0); return true; }
		}
		return true;
	}
	SDL_Rect h = undo_hint();
	if (!SV.undo || !SDL_PointInRect(&p, &h)) return false;
	saves_home_choose(SAVES_UNDO);
	return true;
}

bool saves_pointer(bool (*back)(void), void (*turn)(int step)) {
	int x, y;
	if (!platform_tap(&x, &y)) return false;
	SDL_Point p = { x, y };
	SavesLayout l = layout();
	bool has_back = SV.view == SAVES_HOME || SV.view == SAVES_SEND_CODE || SV.view == SAVES_RECEIVE_CODE;
	if (has_back && SDL_PointInRect(&p, &l.back)) { back(); return true; }
	if (SV.view == SAVES_HOME && home_tap(p, &l, turn)) return true;
	for (int i = 0; i < l.count; ++i) {
		if (!SDL_PointInRect(&p, &l.buttons[i].rect)) continue;
		SV.focus = l.buttons[i].action;
		if (SV.view == SAVES_HOME) saves_home_choose(SV.focus);
		else saves_activate(SV.focus);
		return true;
	}
	return true;
}
