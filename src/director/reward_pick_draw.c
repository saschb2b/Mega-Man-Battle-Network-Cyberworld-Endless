/* The reward pick's picture (reward_pick.h), laid out as BN6's Library,
 * whose card a hardcore player reads at a glance: the row under the cursor
 * on the Library's card at the left (a chip's picture, its code as the
 * folder editor's card sets it, its element, power and description; a
 * program's shape in its colour where a chip's picture stands; HPMemory's
 * and the BugFrags' sums), the choices in the window at the right, a row
 * each, the skip's row under them, and the information box under the
 * window, KeyItem's, saying what the row adds. The screen's name on the
 * band. Drawing changes nothing. */
#include "reward_pick.h"

#include <stdio.h>
#include <string.h>

#include "bn6_chips.h"
#include "chips.h"
#include "data.h"
#include "director.h"
#include "gfx.h"
#include "library_draw.h"
#include "library_pages.h"
#include "meta.h"
#include "navicust.h"
#include "pet_look.h"
#include "platform.h"
#include "reward_pick_state.h"
#include "reward_pick_text.h"
#include "second_art.h"
#include "second_card.h"

/* where it draws, from the picture's corner: the Library's card, the
 * window of four rows (three choices and the skip) beside it, the
 * information box under the window to the body's foot */
#define CARD_X 7
#define CARD_Y 17
#define WIN_X 92
#define WIN_Y 17
#define WIN_W (CORE_W - WIN_X - 7)
/* The window fitted to its rows, the information box under it to the
 * body's foot: a row with a chip's icon the Library's 16 tall, the rest
 * the chat font's 13, as the Liberation pick's (a draft's four, its
 * programs and the BugFrags). Every pick then leaves the box four lines
 * whole; a longer text scrolls (info()), and --pick-fit lists them. */
#define ROW_CHIP_H 16
#define ROW_TEXT_H 13
#define WIN_H(rows, row_h) (2 * LOOK_FRAME + 4 + (rows) * (row_h))
#define INFO_Y(win_h) (WIN_Y + (win_h) + 2)
#define INFO_H(win_h) (CORE_H - LOOK_RAIL - 1 - INFO_Y(win_h))
static int row_h(void);
#define CODE_INK rgba(255, 231, 0, 255)   /* a chip's code, yellow as BN6 sets it */

/* ---- the card ---- */

/* a program's colour variant (program * 4 + v) of colour `color` */
static int variant_of(int program, int color) {
	for (int v = 0; v < 4; ++v) {
		int kind, c;
		if (navicust_part(program * 4 + v, &kind, &c) && c == color) return program * 4 + v;
	}
	return program * 4;
}

/* a sum in the card's head, in heavy capitals, its label over it */
static void head_sum(int x, int y, const char *label, const char *sum) {
	look_caps(x + 40 - look_caps_width(label) / 2, y + 22, label, LOOK_PAGE, rgba(0, 0, 0, 0));
	look_title(x + 40 - look_title_width(sum) / 2, y + 32, sum, LOOK_GOLD);
}

/* BN6's shops' ITEMS box, measured from the chip shop's and the
 * NaviCust shop's (docs/engine/ui.md, Ours): a navy edge round a white
 * frame, ITEMS on a slate strip, a cell for each code the chip comes in
 * (an orange letter) or each colour the program does (its square), the
 * cells' two light cyans by turns, and under each in a navy cell how many
 * MegaMan holds of it. It stands where the Library's card sets the words,
 * which go to the information box. */
#define ITEMS_EDGE  rgba(41, 99, 140, 255)
#define ITEMS_FRAME rgba(247, 255, 247, 255)
#define ITEMS_BAND  rgba(90, 148, 173, 255)
#define ITEMS_CELL  rgba(189, 231, 239, 255)
#define ITEMS_CELL2 rgba(214, 247, 255, 255)
#define ITEMS_CODE  rgba(239, 148, 0, 255)
#define ITEMS_W 70
#define ITEMS_H 38
static void items_box(int x, int y, int k, bool colours) {
	fill_rect(x, y, ITEMS_W, ITEMS_H, ITEMS_EDGE);
	fill_rect(x + 1, y + 1, ITEMS_W - 2, ITEMS_H - 2, ITEMS_FRAME);
	fill_rect(x + 2, y + 2, ITEMS_W - 4, 9, ITEMS_BAND);
	const char *label = reward_pick_items();
	look_caps(x + (ITEMS_W - look_caps_width(label)) / 2, y + 4, label, ITEMS_FRAME, rgba(0, 0, 0, 0));
	int cw = (ITEMS_W - 4) / PICK_VARIANTS;
	for (int i = 0; i < PICK_VARIANTS; ++i) {
		int cx = x + 2 + i * cw, mid = cx + cw / 2;
		fill_rect(cx, y + 12, i == PICK_VARIANTS - 1 ? ITEMS_W - 4 - i * cw : cw, ITEMS_H - 14, i % 2 ? ITEMS_CELL2 : ITEMS_CELL);
		if (i >= RP.nvar[k]) continue;
		int mark = RP.var[k][i];
		if (colours) second_color_swatch(mark, mid - 4, y + 15, 7);
		else {
			char code[2] = { mark >= 26 ? '*' : (char)('A' + mark), 0 };
			look_title(mid - look_title_width(code) / 2, y + 14, code, ITEMS_CODE);
		}
		char held[4];
		snprintf(held, sizeof held, "%d", RP.var_held[k][i]);
		int hw = look_caps_width(held) + 4;
		fill_rect(mid - hw / 2, y + 25, hw, 9, ITEMS_EDGE);
		look_caps(mid - hw / 2 + 2, y + 27, held, ITEMS_FRAME, rgba(0, 0, 0, 0));
	}
}

static void chip_card(int x, int y, const PickChoice *c) {
	const uint8_t *rec = chips_record(c->id);
	int cls = rec && rec[BN6_CHIP_LIBRARY_TYPE] < LIB_CLASSES ? rec[BN6_CHIP_LIBRARY_TYPE] : LIB_STD;
	library_card_face(x, y, c->id, cls);
	/* (its code at the head's foot left of the element, as the folder
	 * editor's card sets it, where the Library's has none; in heavy
	 * capitals, an O apart from the power's zeros: row()) */
	char code[2] = { c->code >= 26 ? '*' : (char)('A' + c->code), 0 };
	look_title(x + 12, y + 54, code, CODE_INK);
	items_box(x + 5, y + 78, RP.cursor, false);
}

static void program_card(int x, int y, const PickChoice *c) {
	library_card_blank(x, y, LIB_STD);
	NaviShape s;
	if (navicust_shape(variant_of(c->id, c->code), &s)) second_program_shape(&s, x + 7 + (66 - SECOND_SHAPE_W) / 2, y + 2 + (66 - SECOND_SHAPE_W) / 2);
	items_box(x + 5, y + 78, RP.cursor, true);
}

static void card(int x, int y) {
	if (RP.cursor >= RP.o.n) {
		/* (the skip's row: its BugFrags) */
		char sum[12];
		snprintf(sum, sizeof sum, "%d", RP.o.frags);
		library_card_blank(x, y, LIB_STD);
		head_sum(x, y, "BUGFRAGS", sum);
		return;
	}
	const PickChoice *c = &RP.o.c[RP.cursor];
	if (c->what == PICK_CHIP) chip_card(x, y, c);
	else if (c->what == PICK_PROGRAM) program_card(x, y, c);
	else {
		char sum[12], lines[32];
		snprintf(sum, sizeof sum, "+%d", c->id * 20);
		library_card_blank(x, y, LIB_STD);
		head_sum(x, y, "MAX HP", sum);
		snprintf(lines, sizeof lines, "%s\nx%d", reward_pick_hp_name(), c->id);
		library_card_lines(x, y, lines);
	}
}

/* ---- the list ---- */

/* A row: its icon (a chip's) or its colour (a program's), its name, and
 * a chip's code in yellow in a column of its own, as BN6's folder lists
 * set it: nothing else stands there, so a code is never read as a count
 * (an O read as a zero beside HPMemory's x2, the owner's catch). The code
 * in heavy capitals, here and on the card, whose zero is slashed: the
 * list's and the chat font's O is a zero's oval */
#define NAME_X 26
#define CODE_X (NAME_X + 8 * 9)
static void row(SDL_Rect f, int r) {
	int y = f.y + 2 + r * row_h();
	char name[24] = "";
	if (r >= RP.o.n) snprintf(name, sizeof name, "%s", reward_pick_skip_name());
	else {
		const PickChoice *c = &RP.o.c[r];
		if (c->what == PICK_CHIP) {
			second_chip_icon(c->id, f.x + 8, y - 1);
			chips_name(c->id, name, sizeof name);
			char code[2] = { c->code >= 26 ? '*' : (char)('A' + c->code), 0 };
			look_title(f.x + CODE_X + 1, y + 4, code, LOOK_SHADE);
			look_title(f.x + CODE_X, y + 3, code, CODE_INK);
			/* (new to the Library: E-Mail's NEW, as a new mail) */
			if (!meta_library_has(c->id)) look_new(f.x + CODE_X + 11, y + 3, reward_pick_new());
		} else if (c->what == PICK_PROGRAM) {
			second_color_swatch(c->code, f.x + 11, y + 3, 7);
			program_name(c->id, name, sizeof name);
		} else snprintf(name, sizeof name, "%s", reward_pick_hp_name());
	}
	text_draw_cells(f.x + NAME_X, y + 1, name, LOOK_TEXT, LOOK_SHADE);
	if (r == RP.cursor) look_cursor(f.x - 3, y + 2);
}

/* ---- the information box ---- */

/* the box's words: in from its sides, as look_info's room and the page's
 * margins leave them; the lines a box `info_h` tall holds whole */
#define INFO_TEXT_W (WIN_W - 4 - 9)
static int room_of(int info_h) { return (info_h - 2 - LOOK_CAPS_H - 1 - 2 - 1 - LOOK_TEXT_H) / LOOK_LINE + 1; }

/* the shown pick's rows' height and its window's */
static int row_h(void) {
	for (int k = 0; k < RP.o.n; ++k)
		if (RP.o.c[k].what == PICK_CHIP) return ROW_CHIP_H;
	return ROW_TEXT_H;
}
static int win_h(void) { return WIN_H(reward_pick_rows(), row_h()); }

void reward_pick_info_text(int what, int id, char *s, size_t n) {
	if (!n) return;
	s[0] = 0;
	if (what == PICK_CHIP) {
		/* (BN6's own description, as its shops show it on R; its three
		 * lines run on) */
		chips_desc_lines(id, s, n);
		for (char *ch = s; *ch; ++ch) if (*ch == '\n') *ch = ' ';
	} else if (what == PICK_PROGRAM) {
		/* (MegaMan's words of what it does, its name left off) */
		const char *about = navicust_about(id), *colon = about ? strchr(about, ':') : NULL;
		snprintf(s, n, "%s", colon ? colon + 1 + (colon[1] == ' ') : about ? about : "");
		if (s[0] >= 'a' && s[0] <= 'z') s[0] = (char)(s[0] - 'a' + 'A');
	}
}

int reward_pick_info_lines(const char *s) { return look_wrapped_lines(s, INFO_TEXT_W); }

/* (the least a pick leaves: a draft's four rows of text, three of chips) */
int reward_pick_info_room(void) {
	int draft = room_of(INFO_H(WIN_H(PICK_MOST + 1, ROW_TEXT_H))), chips = room_of(INFO_H(WIN_H(PICK_MOST, ROW_CHIP_H)));
	return draft < chips ? draft : chips;
}

/* A text longer than the box, scrolled as BN6's chat box scrolls: the
 * first lines held, then a line up every second, the last held, and round
 * from the top again; from the cursor's last move */
#define SCROLL_HOLD 90
#define SCROLL_STEP 60
#define SCROLL_END 120
static int scrolled(int lines, int room) {
	int extra = lines - room;
	if (extra <= 0) return 0;
	int round = SCROLL_HOLD + extra * SCROLL_STEP + SCROLL_END, t = (RP.t - RP.moved) % round;
	int k = t < SCROLL_HOLD ? 0 : 1 + (t - SCROLL_HOLD) / SCROLL_STEP;
	return k < extra ? k : extra;
}

static void info(int x, int y, int h) {
	SDL_Rect r = look_info(x, y, WIN_W, h, reward_pick_label());
	fill_rect(r.x + 1, r.y + 1, r.w - 2, r.h - 1, LOOK_PAGE);
	char s[200] = "";
	if (RP.cursor >= RP.o.n) reward_pick_skip_info(s, sizeof s, RP.o.frags);
	else {
		const PickChoice *c = &RP.o.c[RP.cursor];
		if (c->what == PICK_CHIP) reward_pick_info_text(PICK_CHIP, c->id, s, sizeof s);
		else if (c->what == PICK_PROGRAM) {
			/* (its part on the box's tag, as KeyItem's box carries KEYITEM) */
			reward_pick_info_text(PICK_PROGRAM, c->id, s, sizeof s);
			int part = NAVI_PART, color;
			navicust_part(variant_of(c->id, c->code), &part, &color);
			const char *tag = reward_pick_part(part);
			int tw = look_caps_width(tag) + 8;
			look_tag(x + WIN_W - tw - 3, y + 1, tw, 7, tag, LOOK_BAND);
		} else reward_pick_hp_info(s, sizeof s, c->id, RP.max_hp);
	}
	int room = room_of(h), lines = reward_pick_info_lines(s);
	LookLines at = { r.x + 5, r.y + 1, INFO_TEXT_W, scrolled(lines, room), room, LOOK_INK, LOOK_INK_SHADE };
	look_wrapped_from(s, &at);
}

void director_draw_pick(void) {
	if (!RP.up) return;
	int x0 = P.core_x, y0 = P.core_y;
	look_screen(x0, y0, CORE_W, CORE_H, reward_pick_title(RP.kind));
	card(x0 + CARD_X, y0 + CARD_Y);
	int wh = win_h();
	SDL_Rect f = look_window(x0 + WIN_X, y0 + WIN_Y, WIN_W, wh, 0, false);
	for (int r = 0; r < reward_pick_rows(); ++r) row(f, r);
	info(x0 + WIN_X, y0 + INFO_Y(wh), INFO_H(wh));
	look_fade_in(x0, y0, CORE_W, CORE_H, RP.t);
}
