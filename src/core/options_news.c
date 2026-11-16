/* options_news.h. The release's notes as GitHub's page has them, a page at
 * a time: first its cover (the tagline in the bold letters, the summary
 * under it), then an entry a page, its section's name in small capitals
 * and the page's number over BN6's list window, the entry's heading in the
 * bold letters, its paragraphs and points (the PET's gold arrow before
 * each) in the list's thin ones, space between them; Left and Right turn
 * the pages, Up and Down scroll a long one. Under the window the buttons
 * as SAVES has them, STATUS's pills, the action NaviCust's green RUN!,
 * each with its key. An entry's picture stands under its heading at half
 * the game's scale (each pixel of it four of the capture's, mixed), SELECT
 * shows it whole over the box. The notes are laid out once, as the box
 * opens, a page's picture unpacked as the page shows: drawing changes
 * nothing. */
#include "options_news.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio.h"
#include "controls.h"
#include "gfx.h"
#include "options.h"
#include "pet_panel.h"
#include "platform.h"
#include "touch.h"
#include "update.h"
#include "update_pictures.h"
#include "update_text.h"

#define BOX_WAIT 30          /* frames before it takes a press: a menu's A mashed is no Update */
#define HEAD_Y 20            /* the section's name and the page's number */
#define WIN_X 7
#define WIN_Y 30
#define WIN_W (CORE_W - 2 * WIN_X)
#define WIN_H 102
#define VIEW_H (WIN_H - 2 * LOOK_FRAME)
#define TEXT_IN 5            /* the letters in from the field's left */
#define TEXT_W (WIN_W - LOOK_FRAME - LOOK_FRAME_BAR - 2 * TEXT_IN)
#define POINT_IN 9           /* a point's letters after its arrow */
#define GAP 5                /* between paragraphs, under a heading */
#define BUTTON_Y (WIN_Y + WIN_H + 4)
#define BUTTON_W 108
#define BUTTON_GAP 6
#define PARS_MAX 700
#define PAGES_MAX 120
#define TEXT_MAX (48 * 1024)
#define BOLD_LINES 4         /* a heading's or the tagline's, at most */
#define BOLD_LEN 96

/* A paragraph's kind, by its mark in update.json's changes */
enum { P_TAGLINE, P_SUMMARY, P_SECTION, P_HEADING, P_POINT, P_PARA, P_PICTURE };

typedef struct {
	int at, skip;     /* its letters' start, and its mark's to leave off */
	int kind, lines;  /* (P_*), its lines */
	int y, page;      /* its top within its page, and the page (-1: a section's name, over its pages) */
	int h;            /* its height: its lines', or its picture's at half scale */
	int pic, w;       /* a picture's place in the file, and its width at half scale */
} Par;

typedef struct {
	int section;      /* the section it stands in (a paragraph), -1 none */
	int h;            /* its height, the scroll's reach */
} Page;

static struct {
	bool open, first, zoom;
	int t, page, scroll, pars, pages;
	char text[TEXT_MAX];
	Par par[PARS_MAX];
	Page pg[PAGES_MAX];
	/* the shown page's picture, unpacked: whole and at half scale */
	SDL_Texture *full, *half;
	int shown, full_w, full_h;   /* (its place in the file, -1 none) */
} K;

static bool asks(void) { return update_way() == UPDATE_WAY_INSTALL || update_way() == UPDATE_WAY_PAGE; }

/* ---- the notes laid out ---- */

static const char *par_text(const Par *p) { return K.text + p->at + p->skip; }

/* a line's kind by its mark, and the mark's length */
static int kind_of(const char *s, int *skip) {
	static const struct { const char *mark; int kind; } marks[] = {
		{ "= ", P_TAGLINE }, { "> ", P_SUMMARY }, { "## ", P_SECTION }, { "# ", P_HEADING }, { "- ", P_POINT }, { "@", P_PICTURE } };
	for (size_t i = 0; i < sizeof marks / sizeof *marks; ++i)
		if (!strncmp(s, marks[i].mark, strlen(marks[i].mark))) {
			*skip = (int)strlen(marks[i].mark);
			return marks[i].kind;
		}
	*skip = 0;
	return P_PARA;
}

/* `s` in the bold letters, in lines no wider than `w`, broken at spaces:
 * how many (each into `out` where it is given) */
static int bold_wrap(const char *s, int w, char out[][BOLD_LEN]) {
	int n = 0;
	while (*s && n < BOLD_LINES) {
		while (*s == ' ') ++s;
		size_t len = strlen(s), cut = 0;
		for (size_t k = 1; k <= len && k < BOLD_LEN; ++k) {
			if (s[k] != ' ' && s[k]) continue;
			char t[BOLD_LEN];
			snprintf(t, sizeof t, "%.*s", (int)k, s);
			if (text_width(t) > w) break;
			cut = k;
		}
		if (!cut) cut = len < BOLD_LEN - 1 ? len : BOLD_LEN - 1;
		if (out) snprintf(out[n], BOLD_LEN, "%.*s", (int)cut, s);
		++n;
		s += cut;
	}
	return n;
}

static bool bold(int kind) { return kind == P_TAGLINE || kind == P_HEADING || kind == P_SECTION; }

/* a paragraph onto page `page`, wrapped as its kind sets it; a picture
 * ("@N WxH") its room at half scale, where pictures can come */
static void add(int at, int skip, int kind, int page) {
	Par *p = &K.par[K.pars];
	*p = (Par){ at, skip, kind, 0, 0, page, 0, -1, 0 };
	if (kind == P_PICTURE) {
		int w = 0, h = 0;
		if (!update_pictures_here() || sscanf(par_text(p), "%d %dx%d", &p->pic, &w, &h) != 3 || w <= 0 || h <= 0) return;
		p->w = w / 2;
		p->h = h / 2;
		++K.pars;
		return;
	}
	++K.pars;
	p->lines = bold(kind) ? bold_wrap(par_text(p), TEXT_W, NULL) : look_wrapped_lines(par_text(p), kind == P_POINT ? TEXT_W - POINT_IN : TEXT_W);
	p->h = p->lines * LOOK_LINE;
}

/* each page's paragraphs stacked, in order: each one's top, the page's height */
static void stack(void) {
	for (int i = 0; i < K.pages; ++i) K.pg[i].h = GAP;
	for (int i = 0; i < K.pars; ++i) {
		Par *p = &K.par[i];
		if (p->page < 0) continue;
		p->y = K.pg[p->page].h;
		K.pg[p->page].h += p->h + (p->kind == P_POINT ? 2 : GAP) + (p->kind == P_TAGLINE ? 4 : 0);
	}
}

/* The changes cut into paragraphs and onto pages: the cover's (the
 * tagline and the summary), then a page each heading; a section's name
 * stands over its pages, and heads one of its own where its text comes
 * before any heading in it */
static void lay_out(void) {
	const char *changes = update_changes();
	snprintf(K.text, sizeof K.text, "%s", changes[0] ? changes : update_word(UW_BOX_NONE));
	K.pars = K.pages = 0;
	int section = -1, page = -1;
	bool pending = false;
	for (char *s = K.text; s && K.pars < PARS_MAX - 2; ) {
		char *next = strchr(s, '\n');
		if (next) *next++ = 0;
		int skip, kind = kind_of(s, &skip), at = (int)(s - K.text);
		s = next;
		if (!K.text[at + skip]) continue;
		if (kind == P_SECTION) {
			add(at, skip, kind, -1);
			section = K.pars - 1;
			pending = true;
			continue;
		}
		bool cover = kind == P_TAGLINE || kind == P_SUMMARY;
		if ((kind == P_HEADING || page < 0 || (pending && !cover)) && K.pages < PAGES_MAX) {
			page = K.pages++;
			K.pg[page] = (Page){ section, 0 };
			if (kind != P_HEADING && pending) add(K.par[section].at, K.par[section].skip, P_HEADING, page);
			pending = false;
		}
		add(at, skip, kind, page);
	}
	stack();
}

/* ---- the page's picture ---- */

static void picture_drop(void) {
	if (K.full) SDL_DestroyTexture(K.full);
	if (K.half) SDL_DestroyTexture(K.half);
	K.full = K.half = NULL;
	K.shown = -1;
	K.zoom = false;
}

/* the shown page's picture, NULL where it has none */
static const Par *page_picture(void) {
	for (int i = 0; i < K.pars; ++i)
		if (K.par[i].page == K.page && K.par[i].kind == P_PICTURE) return &K.par[i];
	return NULL;
}

static SDL_Texture *texture_of(const uint32_t *px, int w, int h) {
	SDL_Texture *t = SDL_CreateTexture(P.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, w, h);
	if (t) SDL_UpdateTexture(t, NULL, px, w * 4);
	return t;
}

/* `px` (w x h) at half scale, each pixel the mean of the four it stands for */
static uint32_t *halved(const uint32_t *px, int w, int h) {
	int hw = w / 2, hh = h / 2;
	uint32_t *out = hw > 0 && hh > 0 ? malloc((size_t)hw * hh * sizeof *out) : NULL;
	for (int y = 0; out && y < hh; ++y)
		for (int x = 0; x < hw; ++x) {
			const uint32_t *a = px + (2 * y) * w + 2 * x, *b = a + w;
			uint32_t c = 0xFF000000u;
			for (int s = 0; s < 24; s += 8) c |= (((a[0] >> s & 255) + (a[1] >> s & 255) + (b[0] >> s & 255) + (b[1] >> s & 255) + 2) / 4) << s;
			out[y * hw + x] = c;
		}
	return out;
}

/* The shown page's picture unpacked, once it has come */
static void picture_sync(void) {
	const Par *p = page_picture();
	int w, h;
	uint32_t *px;
	if (!p || p->pic == K.shown || !update_picture(p->pic, &w, &h, &px)) return;
	picture_drop();
	uint32_t *half = halved(px, w, h);
	K.full = texture_of(px, w, h);
	K.half = half ? texture_of(half, w / 2, h / 2) : NULL;
	free(px);
	free(half);
	K.shown = p->pic;
	K.full_w = w;
	K.full_h = h;
}

/* ---- the box ---- */

static void open_box(bool first) {
	K.shown = -1;
	K.open = true;
	K.first = first;
	K.t = K.page = K.scroll = 0;
	picture_drop();
	update_pictures_want();
	lay_out();
	/* (the finger that opened it presses nothing more) */
	touch_release();
}

void news_box_offer(void) {
	if (!K.open && !options_shown() && update_box_due()) open_box(true);
}

bool news_box_open(void) {
	if (update_state() != UPDATE_FOUND) return false;
	open_box(false);
	return true;
}

bool news_box_shown(void) { return K.open; }

void news_box_describe(FILE *f) {
	static const char *const states[] = { [UPDATE_IDLE] = "none", [UPDATE_FOUND] = "found", [UPDATE_GETTING] = "getting", [UPDATE_FAILED] = "failed",
		[UPDATE_READY] = "ready" };
	if (!update_here()) return;
	fprintf(f, "update %s\n", states[update_state()]);
	if (update_state() != UPDATE_IDLE) fprintf(f, "newversion %s\n", update_version());
}

/* A button: kept for the version (the title's box shows once), then the
 * download or the download page; from the title, Options on the update's
 * line to show the download go */
static void press(bool act) {
	update_box_done();
	K.open = false;
	picture_drop();
	audio_sfx(act ? SFX_SELECT : SFX_CANCEL);
	if (!act || !asks() || !update_press()) return;
	if (K.first && update_way() == UPDATE_WAY_INSTALL) options_open_update();
}

static int scroll_most(void) {
	int h = K.pages ? K.pg[K.page].h : 0;
	return h > VIEW_H ? h - VIEW_H : 0;
}

static void scroll_by(int by) {
	int most = scroll_most(), s = K.scroll + by;
	s = s < 0 ? 0 : s > most ? most : s;
	if (s != K.scroll) audio_sfx(SFX_CURSOR);
	K.scroll = s;
}

static void turn(int by) {
	int p = K.page + by;
	if (p < 0 || p >= K.pages) return;
	K.page = p;
	K.scroll = 0;
	audio_sfx(SFX_CURSOR);
}

void news_box_update(void) {
	if (!K.open) return;
	picture_sync();
	if (++K.t < BOX_WAIT) return;
	uint32_t pr = P.menu_pressed, rep = P.menu_repeat;
	/* (a picture whole: any of the buttons back to the page) */
	if (K.zoom) {
		if (pr & (BTN_A | BTN_B | BTN_SELECT | BTN_START)) {
			K.zoom = false;
			audio_sfx(SFX_CANCEL);
		}
		return;
	}
	if ((pr & BTN_SELECT) && K.full && page_picture()) {
		K.zoom = true;
		audio_sfx(SFX_SELECT);
		return;
	}
	if (pr & BTN_B) { press(false); return; }
	if (pr & (BTN_A | BTN_START)) { press(asks()); return; }
	if (rep & (BTN_UP | BTN_DOWN)) scroll_by(rep & BTN_UP ? -LOOK_LINE : LOOK_LINE);
	if (rep & (BTN_LEFT | BTN_RIGHT | BTN_L | BTN_R)) turn(rep & (BTN_LEFT | BTN_L) ? -1 : 1);
}

bool news_box_back(void) {
	if (!K.open) return false;
	if (K.zoom) {
		K.zoom = false;
		return true;
	}
	press(false);
	return true;
}

static int buttons(void) { return asks() ? 2 : 1; }

/* the button `b`'s place, from the picture's corner (OK alone at the right) */
static SDL_Rect button_rect(int b) {
	int n = buttons(), x = n == 1 ? CORE_W - WIN_X - BUTTON_W : (CORE_W - n * BUTTON_W - (n - 1) * BUTTON_GAP) / 2;
	return (SDL_Rect){ x + b * (BUTTON_W + BUTTON_GAP), BUTTON_Y, BUTTON_W, LOOK_PILL_H };
}

void news_box_tap(int x, int y) {
	if (!K.open || K.t < BOX_WAIT) return;
	if (K.zoom) {
		K.zoom = false;
		return;
	}
	for (int b = 0; b < buttons(); ++b) {
		SDL_Rect r = button_rect(b);
		if (x >= r.x && x < r.x + r.w && y >= r.y - 2 && y < r.y + r.h + 2) { press(asks() && b == 0); return; }
	}
	/* (the window's left third a page back, its right third on, between them a scroll down) */
	if (y < WIN_Y || y >= WIN_Y + WIN_H) return;
	if (x < CORE_W / 3) turn(-1);
	else if (x >= 2 * CORE_W / 3) turn(1);
	else scroll_by(scroll_most() > K.scroll ? VIEW_H - LOOK_LINE : -K.scroll);
}

/* ---- the picture ---- */

/* a bold paragraph's lines from its top `y` in the page, those in view */
static void bold_draw(const Par *p, SDL_Rect f, int y, bool centre, SDL_Color c) {
	char lines[BOLD_LINES][BOLD_LEN];
	int n = bold_wrap(par_text(p), TEXT_W, lines);
	for (int i = 0; i < n; ++i) {
		int ly = y + i * LOOK_LINE;
		if (ly < 0 || ly + TEXT_H > VIEW_H) continue;
		text_draw(centre ? f.x + f.w / 2 : f.x + TEXT_IN, f.y + ly, lines[i], c, centre ? TEXT_CENTER : TEXT_LEFT);
	}
}

/* a thin paragraph's lines in view, from its top `y` in the page */
static void thin_draw(const Par *p, SDL_Rect f, int y, int in, SDL_Color c) {
	int first = y < 0 ? (-y + LOOK_LINE - 1) / LOOK_LINE : 0, last = p->lines;
	while (last > first && y + last * LOOK_LINE > VIEW_H) --last;
	if (last <= first) return;
	LookLines at = { f.x + TEXT_IN + in, f.y + y + first * LOOK_LINE, TEXT_W - in, first, last - first, c, LOOK_SHADE };
	look_wrapped_from(par_text(p), &at);
}

/* the page's picture at half scale, the part in view; its room, until it comes */
static void picture_draw(const Par *p, SDL_Rect f, int y) {
	int top = y < 0 ? -y : 0, bottom = y + p->h > VIEW_H ? VIEW_H - y : p->h;
	if (bottom <= top) return;
	SDL_Rect dst = { f.x + (f.w - p->w) / 2, f.y + y + top, p->w, bottom - top };
	if (!K.half || K.shown != p->pic) {
		fill_rect(dst.x, dst.y, dst.w, dst.h, rgba(16, 40, 72, 255));
		return;
	}
	SDL_Rect src = { 0, top, p->w, bottom - top };
	SDL_RenderCopy(P.renderer, K.half, &src, &dst);
}

static void page_draw(SDL_Rect f) {
	for (int i = 0; i < K.pars; ++i) {
		const Par *p = &K.par[i];
		if (p->page != K.page) continue;
		int y = p->y - K.scroll;
		if (y + p->h <= 0 || y >= VIEW_H) continue;
		switch (p->kind) {
		case P_PICTURE: picture_draw(p, f, y); break;
		case P_TAGLINE: bold_draw(p, f, y, true, PET_GOLD); break;
		case P_HEADING: case P_SECTION: bold_draw(p, f, y, false, PET_GOLD); break;
		case P_POINT:
			if (y >= 0 && y + LOOK_LINE <= VIEW_H) look_pointer(f.x + TEXT_IN, f.y + y + 1, 3, false, LOOK_GOLD);
			thin_draw(p, f, y, POINT_IN, LOOK_TEXT);
			break;
		default: thin_draw(p, f, y, 0, LOOK_TEXT); break;
		}
	}
}

/* over the window: the section's name, and the page's number between the
 * PET's arrows where there are more */
static void head_draw(int x0, int y0) {
	const Page *pg = &K.pg[K.page];
	char s[64];
	if (pg->section >= 0) snprintf(s, sizeof s, "%s", par_text(&K.par[pg->section]));
	else snprintf(s, sizeof s, "%s v%s", update_word(UW_BOX_NEWS), update_version());
	for (char *c = s; *c; ++c) *c = (char)(*c >= 'a' && *c <= 'z' ? *c - 32 : *c);
	look_caps(x0 + WIN_X + 2, y0 + HEAD_Y + 1, s, PET_GOLD, LOOK_SHADE);
	if (K.pages < 2) return;
	snprintf(s, sizeof s, "%d/%d", K.page + 1, K.pages);
	int w = look_caps_width(s), r = x0 + CORE_W - WIN_X - 8;
	look_caps(r - w, y0 + HEAD_Y + 1, s, LOOK_TEXT, LOOK_SHADE);
	if (K.page > 0) look_pointer(r - w - 8, y0 + HEAD_Y, 4, true, PET_ORANGE);
	if (K.page + 1 < K.pages) look_pointer(r + 3, y0 + HEAD_Y, 4, false, PET_ORANGE);
}

static void buttons_draw(int x0, int y0) {
	static const UpdateWord acts[] = { [UPDATE_WAY_INSTALL] = UW_BTN_UPDATE, [UPDATE_WAY_PAGE] = UW_BTN_DOWNLOAD };
	static const UpdateWord says[] = { [UPDATE_WAY_NONE] = UW_BOX_RELEASES, [UPDATE_WAY_RELEASES] = UW_BOX_RELEASES,
		[UPDATE_WAY_ITCH] = UW_BOX_ITCH, [UPDATE_WAY_STORE] = UW_BOX_STORE };
	UpdateWay way = update_way();
	if (!asks()) look_text(x0 + WIN_X + 2, y0 + BUTTON_Y + 2, update_word(says[way]), LOOK_TEXT, LOOK_SHADE, TEXT_LEFT);
	for (int b = 0; b < buttons(); ++b) {
		SDL_Rect r = button_rect(b);
		r.x += x0;
		r.y += y0;
		bool act = asks() && b == 0;
		/* (STATUS's pills; the action NaviCust's green RUN!, as BN6 marks its one action) */
		if (act && K.t >= BOX_WAIT) look_button(r.x, r.y, r.w - 1, r.h - 1);
		else look_pill(r.x, r.y, r.w);
		char label[48];
		snprintf(label, sizeof label, "%s: %s", controls_word(act || !asks() ? BTN_A : BTN_B),
			update_word(!asks() ? UW_OK : act ? acts[way] : UW_BTN_LATER));
		look_text(r.x + r.w / 2, r.y + 1, label, LOOK_TEXT, LOOK_SHADE, TEXT_CENTER);
	}
}

void news_box_draw(void) {
	if (!K.open || !K.pages) return;
	int x0 = P.core_x, y0 = P.core_y;
	/* (a picture whole, over all, at the game's own scale) */
	if (K.zoom && K.full) {
		fill_rect(x0, y0, CORE_W, CORE_H, BLACK);
		SDL_Rect dst = { x0 + (CORE_W - K.full_w) / 2, y0 + (CORE_H - K.full_h) / 2, K.full_w, K.full_h };
		SDL_RenderCopy(P.renderer, K.full, NULL, &dst);
		return;
	}
	bool long_page = scroll_most() > 0;
	pet_panel_draw(x0, y0, update_word(UW_BOX_TITLE));
	/* (the band's hint: SELECT's picture where the page has one, else Up and Down's scroll) */
	if (K.full && page_picture()) look_band_hint(x0, y0, CORE_W, controls_word(BTN_SELECT), update_word(UW_BOX_PICTURE));
	else if (long_page) look_band_hint(x0, y0, CORE_W, "UP/DOWN", update_word(UW_BOX_SCROLL));
	head_draw(x0, y0);
	SDL_Rect f = look_window(x0 + WIN_X, y0 + WIN_Y, WIN_W, WIN_H, 0, long_page);
	page_draw(f);
	if (long_page) look_scroll((SDL_Rect){ x0 + WIN_X, y0 + WIN_Y, WIN_W, WIN_H }, 0, K.scroll, VIEW_H, K.pg[K.page].h);
	buttons_draw(x0, y0);
	look_fade_in(x0, y0, CORE_W, CORE_H, K.t);
}
