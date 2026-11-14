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
static void note(SDL_Rect r, const char *text, SDL_Color color) {
	char lines[8][96];
	int rows = (r.h + PIXFONT_LINE - PIXFONT_H) / PIXFONT_LINE, k = 0;
	if (rows > 8) rows = 8;
	char part[700];
	snprintf(part, sizeof part, "%s", text);
	for (char *p = strtok(part, "\n"); p && k < rows; p = strtok(NULL, "\n"))
		k += pixfont_wrap(p, r.w, 1, lines + k, rows - k);
	for (int i = 0; i < k; ++i) words(r.x, r.y + i * PIXFONT_LINE, lines[i], color);
}

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
	return saves_layout(P.w, P.h, SV.view, actions, n);
}

void saves_draw(void) {
	/* (BN6's PET screens' band, as the ROMs screen's: pet_look.h) */
	fill_rect(0, 0, P.w, P.h, LOOK_GREEN);
	fill_rect(0, 0, P.w, 20, LOOK_BAND);
	const char *title = saves_word_title();
	look_title(8, 6, title, LOOK_PAGE);
	int sx = 22 + look_title_width(title);
	look_stripes(sx > P.w / 2 ? sx : P.w / 2, 0, P.w - 4, 20);
	SavesLayout l = layout();
	if (SV.view == SAVES_HOME || SV.view == SAVES_SEND_CODE || SV.view == SAVES_RECEIVE_CODE) {
		look_pill(l.back.x, l.back.y, l.back.w);
		pixfont_draw(l.back.x + l.back.w / 2, l.back.y + 4, saves_word_back(), PET_WHITE, shade(), PIXFONT_CENTER, 1);
	}
	if (SV.view == SAVES_SEND_CODE) send_view(&l);
	else if (SV.view == SAVES_RECEIVE_CODE) receive_view(&l);
	else if (SV.view == SAVES_HOME) {
		panel(l.here, &SV.here, false, SV.unreadable, false);
		MirrorStatus m;
		mirror_status(&m);
		char line[1200];
		saves_status_line(&m, backup_blocked(), pick_saves_capabilities() & SAVES_CAN_FOLDER, line, sizeof line);
		clipped(l.line.x, l.line.y, SV.said[0] ? SV.said : line, l.line.w, SV.said[0] ? PET_GOLD : PET_WHITE);
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

bool saves_pointer(bool (*back)(void)) {
	int x, y;
	if (!platform_tap(&x, &y)) return false;
	SDL_Point p = { x, y };
	SavesLayout l = layout();
	bool has_back = SV.view == SAVES_HOME || SV.view == SAVES_SEND_CODE || SV.view == SAVES_RECEIVE_CODE;
	if (has_back && SDL_PointInRect(&p, &l.back)) { back(); return true; }
	for (int i = 0; i < l.count; ++i) {
		if (!SDL_PointInRect(&p, &l.buttons[i].rect)) continue;
		SV.focus = l.buttons[i].action;
		saves_activate(SV.focus);
		return true;
	}
	return true;
}
