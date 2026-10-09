#include "saves_state.h"

#include <stdio.h>
#include <string.h>

#include "controls.h"
#include "gfx.h"
#include "pick.h"
#include "pixfont.h"
#include "saves_text.h"
#include "second_frame.h"

typedef struct {
	SDL_Rect here, file, buttons[SAVES_ACTIONS], note, back;
	bool columns;
	int count;
} Layout;

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

static Layout layout(void) {
	Layout l = { 0 };
	l.columns = P.w >= 230;
	l.back = (SDL_Rect){ P.w - 48, 2, 44, 16 };
	int panel_h = SV.comparing ? 73 : l.columns ? 48 : 81;
	l.here = (SDL_Rect){ 6, 25, P.w - 12, panel_h };
	l.count = SV.comparing ? 2 : SAVES_ACTIONS;
	if (SV.comparing) {
		if (l.columns) {
			l.here.w = (P.w - 16) / 2;
			l.file = (SDL_Rect){ l.here.x + l.here.w + 4, 25, l.here.w, panel_h };
		} else l.file = (SDL_Rect){ 6, 25 + panel_h + 3, P.w - 12, panel_h };
	}
	int bottom = SV.comparing && !l.columns ? l.file.y + l.file.h : l.here.y + l.here.h;
	int columns = l.columns ? 2 : 1;
	int button_y = SV.comparing ? P.h - (l.columns ? 30 : 48) : bottom + 5;
	int bw = l.columns ? (P.w - 16) / 2 : P.w - 12;
	for (int i = 0; i < l.count; ++i)
		l.buttons[i] = (SDL_Rect){ 6 + (i % columns) * (bw + 4), button_y + (i / columns) * 18, bw, 16 };
	if (SV.comparing) l.note = (SDL_Rect){ 8, bottom + 3, P.w - 16, button_y - bottom - 5 };
	else {
		int actions_h = ((SAVES_ACTIONS + columns - 1) / columns) * 18;
		l.note = (SDL_Rect){ 8, button_y + actions_h, P.w - 16, P.h - button_y - actions_h - 3 };
	}
	return l;
}

static void button(SDL_Rect r, const char *label, bool focus, bool disabled) {
	fill_rect(r.x, r.y, r.w, r.h, focus ? PET_GOLD : PET_SLOT_EDGE);
	fill_rect(r.x + 1, r.y + 1, r.w - 2, r.h - 2, focus ? PET_NAVY : PET_SLOT);
	clipped(r.x + 5, r.y + 4, label, r.w - 10, disabled ? PET_DIM : PET_WHITE);
}

static bool newer(const BackupInfo *a, const BackupInfo *b) {
	if (a->save_id[0] && !strcmp(a->save_id, b->save_id)) return a->revision > b->revision;
	return a->stamp && b->stamp && a->stamp > b->stamp;
}

static void summary(SDL_Rect r, const BackupInfo *info, const char *label, bool is_newer, bool compact) {
	fill_rect(r.x, r.y, r.w, r.h, PET_NAVY);
	char heading[64];
	snprintf(heading, sizeof heading, "%s%s", label, is_newer ? " (newer)" : "");
	clipped(r.x + 4, r.y + 3, heading, r.w - 8, PET_GOLD);
	for (int i = 0; i < 6; ++i) {
		char text[128];
		saves_summary(info, i, text, sizeof text);
		int col = compact && i >= 3 ? 1 : 0;
		int width = compact ? (r.w - 12) / 2 : r.w - 8;
		int x = r.x + 4 + col * (width + 4), y = r.y + 15 + (compact ? i % 3 : i) * (compact ? 11 : 9);
		clipped(x, y, text, width, PET_WHITE);
	}
}

/* Wrap long file paths as well as prose, so every part can be read. */
static void note(SDL_Rect r, const char *s, SDL_Color color) {
	char lines[12][96];
	int most = r.h / PIXFONT_LINE;
	if (most > 12) most = 12;
	if (most <= 0) return;
	int n = pixfont_wrap(s, r.w, 1, lines, most);
	for (int i = 0; i < n; ++i) clipped(r.x, r.y + i * PIXFONT_LINE, lines[i], r.w, color);
}

static void message(void) {
	fill_rect(4, 25, P.w - 8, P.h - 29, PET_NAVY);
	note((SDL_Rect){ 10, 31, P.w - 20, P.h - 65 }, SV.note, PET_WHITE);
	button((SDL_Rect){ 8, P.h - 28, P.w - 16, 18 }, "OK", true, false);
}

void saves_draw(void) {
	fill_rect(0, 0, P.w, P.h, PET_GREEN);
	fill_rect(0, 0, P.w, 20, PET_DARK);
	fill_rect(0, 20, P.w, 1, PET_LINE);
	words(8, 5, "SAVES", PET_WHITE);
	second_stripes(49, 4);
	Layout l = layout();
	button(l.back, "Back", false, SV.busy);
	if (!SV.comparing && SV.note[0]) { message(); return; }
	summary(l.here, &SV.here, "This device", SV.comparing && newer(&SV.here, &SV.file), !SV.comparing && l.columns);
	if (SV.comparing) {
		summary(l.file, &SV.file, "The file", newer(&SV.file, &SV.here), false);
		note(l.note, SV.note, SV.status == BACKUP_OK ? PET_WHITE : PET_GOLD);
		button(l.buttons[SAVES_KEEP], "Keep this device", SV.focus == SAVES_KEEP, false);
		button(l.buttons[SAVES_TAKE], "Import the file", SV.focus == SAVES_TAKE, SV.status != BACKUP_OK);
	} else {
		for (int i = 0; i < SAVES_ACTIONS; ++i) {
			char label[40];
			snprintf(label, sizeof label, "%s%s", saves_label(i), i == SAVES_AUTO ? pick_saves_auto_enabled() ? ": on" : ": off" : "");
			bool disabled = SV.busy || (i == SAVES_UNDO && !SV.undo) ||
				(i == SAVES_AUTO && !(pick_saves_capabilities() & SAVES_CAN_AUTO)) || (i == SAVES_FOLDER && !(pick_saves_capabilities() & SAVES_CAN_FOLDER));
			button(l.buttons[i], label, SV.focus == i, disabled);
		}
		note(l.note, SV.busy ? "Waiting for the file picker..." : saves_hint(SV.focus), PET_WHITE);
	}
}

bool saves_pointer(void) {
	int x, y;
	if (!platform_tap(&x, &y)) return false;
	SDL_Point p = { x, y };
	Layout l = layout();
	if (!SV.comparing && SV.note[0]) { SV.note[0] = 0; return true; }
	if (SDL_PointInRect(&p, &l.back)) { saves_back(); return true; }
	for (int i = 0; i < l.count; ++i) {
		if (!SDL_PointInRect(&p, &l.buttons[i])) continue;
		SV.focus = i;
		if (SV.comparing) saves_answer(i == SAVES_TAKE);
		else saves_activate(i);
		return true;
	}
	return true;
}
