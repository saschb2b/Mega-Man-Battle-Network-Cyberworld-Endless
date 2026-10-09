#include "saves_state.h"

#include <stdio.h>
#include <string.h>

#include "gfx.h"
#include "pick.h"
#include "pixfont.h"
#include "saves_layout.h"
#include "saves_text.h"
#include "second_frame.h"

static SDL_Color shade(void) { return rgba(0, 49, 74, 255); }

static void words(int x, int y, const char *s, SDL_Color c) {
	pixfont_draw(x, y, s, c, shade(), PIXFONT_LEFT, 1);
}

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

/* Unlike a fixed array of wrapped lines, this keeps every path character. */
static bool next_line(const char **cursor, int width, char line[96]) {
	const char *start = *cursor;
	while (*start == ' ') ++start;
	if (!*start) return false;
	int n = 0, space = -1;
	while (start[n] && start[n] != '\n' && n < 95) {
		line[n] = start[n];
		line[n + 1] = 0;
		if (pixfont_width(line, 1) > width && n) break;
		if (start[n] == ' ') space = n;
		++n;
	}
	if (start[n] && start[n] != '\n' && space > 0) n = space;
	line[n] = 0;
	*cursor = start + n;
	if (**cursor == '\n' || **cursor == ' ') ++*cursor;
	return true;
}

static int rows(SDL_Rect r) { return (r.h + PIXFONT_LINE - PIXFONT_H) / PIXFONT_LINE; }

static void note(SDL_Rect r, const char *text, SDL_Color color) {
	const char *cursor = text;
	char line[96];
	for (int i = 0; i < rows(r) && next_line(&cursor, r.w, line); ++i)
		words(r.x, r.y + i * PIXFONT_LINE, line, color);
}

static void button(SavesButton b) {
	SDL_Rect r = b.rect;
	bool focus = SV.focus == b.action;
	bool disabled = saves_disabled(b.action);
	fill_rect(r.x, r.y, r.w, r.h, focus ? PET_GOLD : PET_LINE);
	fill_rect(r.x + 1, r.y + 1, r.w - 2, r.h - 2, focus ? PET_NAVY : PET_SLOT);
	char label[64];
	snprintf(label, sizeof label, "%s%s", saves_label(b.action),
		b.action == SAVES_AUTO ? pick_saves_auto_enabled() ? ": on" : ": off" : "");
	clipped(r.x + 5, r.y + (r.h - PIXFONT_H) / 2, label, r.w - 10,
		disabled ? PET_CYAN_HI : PET_WHITE);
}

static void summary(SDL_Rect r, const BackupInfo *info, const char *title, bool compact) {
	fill_rect(r.x, r.y, r.w, r.h, PET_NAVY);
	clipped(r.x + 4, r.y + 3, title, r.w - 8, PET_GOLD);
	static const int home_rows[] = { 1, 2, 0 };
	static const int compare_rows[] = { 1, 2, 0, 6 };
	const int *order = compact ? compare_rows : home_rows;
	bool stacked = compact && P.w < 230;
	int count = compact && !stacked ? 4 : 3;
	for (int i = 0; i < count; ++i) {
		char text[128];
		saves_summary(info, order[i], text, sizeof text);
		clipped(r.x + 4, r.y + 14 + i * (stacked ? PIXFONT_H : PIXFONT_LINE), text, r.w - 8, i ? PET_WHITE : PET_GOLD);
	}
}

static SavesLayout current_layout(void) {
	bool resume = SV.view == SAVES_RESULT ? SV.result_resume : SV.resume;
	return saves_layout(P.w, P.h, SV.view, resume, SV.result_undo);
}

int saves_details_pages(void) {
	char text[4096], line[96];
	saves_details_text(text, sizeof text);
	const char *cursor = text;
	SDL_Rect r = current_layout().content;
	if (r.h <= 0) r = saves_layout(P.w, P.h, SAVES_DETAIL, false, false).content;
	int count = 0, per_page = rows(r);
	while (next_line(&cursor, r.w, line)) ++count;
	return per_page > 0 && count ? (count + per_page - 1) / per_page : 1;
}

static void details(SavesLayout l) {
	char text[4096], line[96], title[64];
	saves_details_text(text, sizeof text);
	saves_page_label(SV.details_page, saves_details_pages(), title, sizeof title);
	clipped(8, 25, title, P.w - 16, PET_GOLD);
	fill_rect(6, l.content.y - 2, P.w - 12, l.content.h + 4, PET_NAVY);
	const char *cursor = text;
	int per_page = rows(l.content), first = SV.details_page * per_page;
	for (int i = 0; next_line(&cursor, l.content.w, line); ++i) {
		if (i < first) continue;
		if (i >= first + per_page) break;
		words(l.content.x, l.content.y + (i - first) * PIXFONT_LINE, line, PET_WHITE);
	}
}

static void status(SDL_Rect r) {
	char text[1000];
	saves_transfer_status(text, sizeof text);
	fill_rect(r.x, r.y, r.w, r.h, PET_NAVY);
	char *second = strchr(text, '\n');
	if (second) *second++ = 0;
	clipped(r.x + 4, r.y + 3, text, r.w - 8, PET_WHITE);
	if (second) clipped(r.x + 4, r.y + 3 + PIXFONT_LINE, second, r.w - 8, PET_WHITE);
}

static void hint(SDL_Rect r) {
	if (r.h <= 0) return;
	char text[300];
	if (SV.feedback[0] && SDL_GetTicks() < SV.feedback_until) snprintf(text, sizeof text, "%s", SV.feedback);
	else saves_action_hint(SV.focus, text, sizeof text);
	note(r, text, PET_WHITE);
}

void saves_draw(void) {
	fill_rect(0, 0, P.w, P.h, PET_GREEN);
	fill_rect(0, 0, P.w, 20, PET_DARK);
	fill_rect(0, 20, P.w, 1, PET_LINE);
	const char *title = saves_view_title();
	words(8, 5, title, PET_WHITE);
	second_stripes(14 + pixfont_width(title, 1), 4);
	SavesLayout l = current_layout();
	fill_rect(l.back.x, l.back.y, l.back.w, l.back.h, PET_SLOT_EDGE);
	fill_rect(l.back.x + 1, l.back.y + 1, l.back.w - 2, l.back.h - 2, PET_SLOT);
	words(l.back.x + 5, l.back.y + 4, "Back", SV.busy ? PET_CYAN_HI : PET_WHITE);
	if (SV.view == SAVES_HOME) {
		summary(l.hero, &SV.here, saves_panel_title(false), false);
		status(l.status);
	} else if (SV.view == SAVES_TRANSFER) status(l.status);
	else if (SV.view == SAVES_COMPARE) {
		summary(l.hero, &SV.here, saves_panel_title(false), true);
		summary(l.file, &SV.file, saves_panel_title(true), true);
		note(l.note, SV.note, SV.status == BACKUP_OK ? PET_WHITE : PET_GOLD);
	} else if (SV.view == SAVES_DETAIL) details(l);
	else {
		fill_rect(6, 24, P.w - 12, l.note.h + 8, PET_NAVY);
		note(l.note, SV.note, PET_WHITE);
	}
	for (int i = 0; i < l.count; ++i) button(l.buttons[i]);
	hint(l.hint);
}

void saves_move_focus(int dx, int dy) {
	SavesLayout l = current_layout();
	int focus = saves_layout_move(&l, SV.focus, dx, dy);
	if (focus != SV.focus) SV.feedback[0] = 0;
	SV.focus = focus;
}

bool saves_pointer(void) {
	int x, y;
	if (!platform_tap(&x, &y)) return false;
	SDL_Point p = { x, y };
	SavesLayout l = current_layout();
	if (SDL_PointInRect(&p, &l.back)) { saves_back(); return true; }
	for (int i = 0; i < l.count; ++i) {
		if (!SDL_PointInRect(&p, &l.buttons[i].rect)) continue;
		SV.focus = l.buttons[i].action;
		saves_activate(SV.focus);
		return true;
	}
	return true;
}
