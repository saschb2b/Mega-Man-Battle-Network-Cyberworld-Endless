#include "ui.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "audio.h"
#include "data.h"
#include "platform.h"
#include "rom.h"

#define MSG_MAX 8

typedef struct {
	char text[256];
	int mugshot;
} Msg;

static Msg queue[MSG_MAX];
static int q_head, q_len;
static int reveal; /* characters shown of the current message */

static enum { MODE_NONE, MODE_MENU, MODE_CARDS, MODE_FOLDER, MODE_PET } mode;
static PetInfo pet;

#define FOLDER_ROWS 7
static uint16_t f_ids[64];
static char f_codes[64];
static bool f_marked[64];
static int f_n;
static char m_title[48];
static UiItem m_items[UI_MAX_ITEMS];
static int m_n, m_cursor, m_scroll, m_timer;
static bool m_cancel;
static UiMenuDone m_done;

static Anim mug;

void ui_clear(void) {
	q_head = q_len = 0;
	mode = MODE_NONE;
}

#define MSG_LINES 3
#define TEXT_X_PLAIN 8    /* text starts here without a mugshot */
#define TEXT_X_MUG 51     /* ...and here beside one, as in the original */
#define TEXT_RIGHT 226

static int text_area_width(int mugshot) { return TEXT_RIGHT - (mugshot >= 0 ? TEXT_X_MUG : TEXT_X_PLAIN); }

/* Length of the next line: as many whole words as fit in `w` pixels. */
static int line_len(const char *p, int w) {
	char line[200];
	int fit = -1;
	for (int i = 0;; ++i) {
		if (p[i] == ' ' || !p[i]) {
			int n = i < (int)sizeof line - 1 ? i : (int)sizeof line - 1;
			memcpy(line, p, (size_t)n);
			line[n] = 0;
			if (fit >= 0 && chat_width(line) > w) return fit;
			fit = i;
			if (!p[i]) return fit;
		}
	}
}

/* Where the text must break so that at most MSG_LINES fit in `w` pixels. */
static size_t page_end(const char *text, int w) {
	const char *p = text;
	for (int l = 0; l < MSG_LINES && *p; ++l) {
		p += line_len(p, w);
		if (l < MSG_LINES - 1) while (*p == ' ') ++p;
	}
	return (size_t)(p - text);
}

static void enqueue(const char *text, int mugshot);

void ui_message(const char *text, int mugshot) {
	/* Long messages continue on further pages instead of overflowing the box. */
	int w = text_area_width(mugshot);
	while (*text) {
		size_t end = page_end(text, w);
		char page[256];
		snprintf(page, sizeof page, "%.*s", (int)end, text);
		enqueue(page, mugshot);
		text += end;
		while (*text == ' ') ++text;
	}
}

static void enqueue(const char *text, int mugshot) {
	if (q_len >= MSG_MAX) return;
	Msg *m = &queue[(q_head + q_len) % MSG_MAX];
	snprintf(m->text, sizeof m->text, "%s", text);
	m->mugshot = mugshot;
	if (q_len == 0) {
		reveal = 0;
		if (mugshot >= 0) {
			Sprite *s = sprite_get(SPR_MUGSHOT, mugshot);
			if (s) anim_play(&mug, s, 0);
			else mug.spr = NULL;
		}
	}
	++q_len;
}

void ui_messagef(int mugshot, const char *fmt, ...) {
	char buf[256];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof buf, fmt, ap);
	va_end(ap);
	ui_message(buf, mugshot);
}

static void open_list(int kind, const char *title, const UiItem *items, int n, bool cancellable, UiMenuDone done) {
	mode = kind;
	snprintf(m_title, sizeof m_title, "%s", title ? title : "");
	if (n > UI_MAX_ITEMS) n = UI_MAX_ITEMS;
	memcpy(m_items, items, sizeof(UiItem) * (size_t)n);
	m_n = n;
	m_cursor = 0;
	while (m_cursor < m_n - 1 && m_items[m_cursor].disabled) ++m_cursor;
	m_scroll = 0;
	m_cancel = cancellable;
	m_done = done;
	m_timer = 0;
}

void ui_menu(const char *title, const UiItem *items, int n, bool cancellable, UiMenuDone done) {
	open_list(MODE_MENU, title, items, n, cancellable, done);
}

void ui_cards(const char *title, const UiItem *items, int n, bool cancellable, UiMenuDone done) {
	open_list(MODE_CARDS, title, items, n, cancellable, done);
}

void ui_folder(const char *title, const uint16_t *ids, const char *codes, int n, const bool *marked, bool cancellable, UiMenuDone done) {
	mode = MODE_FOLDER;
	snprintf(m_title, sizeof m_title, "%s", title ? title : "");
	if (n > 64) n = 64;
	f_n = n;
	memcpy(f_ids, ids, sizeof(uint16_t) * (size_t)n);
	memcpy(f_codes, codes, (size_t)n);
	for (int i = 0; i < n; ++i) f_marked[i] = marked ? marked[i] : false;
	m_n = n;
	m_cursor = 0;
	m_scroll = 0;
	m_cancel = cancellable;
	m_done = done;
	m_timer = 0;
}

void ui_pet(const PetInfo *info, int start, UiMenuDone done) {
	mode = MODE_PET;
	pet = *info;
	m_n = 8;
	m_cursor = start;
	m_cancel = true;
	m_done = done;
	m_timer = 0;
}

bool ui_active(void) { return q_len > 0 || mode != MODE_NONE; }
bool ui_fullscreen(void) { return mode == MODE_PET || mode == MODE_FOLDER; }

void ui_update(void) {
	if (q_len > 0) {
		Msg *m = &queue[q_head];
		int len = (int)strlen(m->text);
		if (mug.spr) { anim_update(&mug); if (mug.done) anim_play(&mug, mug.spr, 0); }
		if (reveal < len) {
			reveal += 2;
			if (btn_pressed(BTN_A) || btn_pressed(BTN_B)) reveal = len;
			return;
		}
		if (btn_pressed(BTN_A) || btn_pressed(BTN_B)) {
			q_head = (q_head + 1) % MSG_MAX;
			--q_len;
			reveal = 0;
			audio_sfx(SFX_CURSOR);
			if (q_len > 0 && queue[q_head].mugshot >= 0) {
				Sprite *s = sprite_get(SPR_MUGSHOT, queue[q_head].mugshot);
				if (s) anim_play(&mug, s, 0);
				else mug.spr = NULL;
			} else mug.spr = NULL;
		}
		return;
	}
	if (mode == MODE_NONE) return;
	++m_timer;
	int prev = m_cursor;
	uint32_t next = mode == MODE_CARDS ? BTN_RIGHT : BTN_DOWN, back = mode == MODE_CARDS ? BTN_LEFT : BTN_UP;
	int rows = mode == MODE_FOLDER ? FOLDER_ROWS : 7;
	if (mode == MODE_PET) {
		if (btn_repeat(BTN_DOWN)) m_cursor = (m_cursor + 1) % 8;
		if (btn_repeat(BTN_UP)) m_cursor = (m_cursor + 7) % 8;
		if (btn_pressed(BTN_START)) { mode = MODE_NONE; audio_sfx(SFX_CANCEL); if (m_done) m_done(-1); return; }
	} else if (mode == MODE_FOLDER) {
		if (btn_repeat(next) && m_cursor + 1 < m_n) ++m_cursor;
		if (btn_repeat(back) && m_cursor > 0) --m_cursor;
		if (btn_repeat(BTN_R)) m_cursor = m_cursor + rows < m_n ? m_cursor + rows : m_n - 1;
		if (btn_repeat(BTN_L)) m_cursor = m_cursor >= rows ? m_cursor - rows : 0;
	} else if (mode == MODE_MENU && m_n <= 3 && m_items[0].chip < 0 && !m_items[0].detail[0]) {
		if (btn_repeat(BTN_RIGHT) || btn_repeat(BTN_DOWN)) m_cursor = (m_cursor + 1) % m_n;
		if (btn_repeat(BTN_LEFT) || btn_repeat(BTN_UP)) m_cursor = (m_cursor + m_n - 1) % m_n;
	} else {
		if (btn_repeat(next)) do { m_cursor = (m_cursor + 1) % m_n; } while (m_items[m_cursor].disabled && m_cursor != prev);
		if (btn_repeat(back)) do { m_cursor = (m_cursor + m_n - 1) % m_n; } while (m_items[m_cursor].disabled && m_cursor != prev);
	}
	if (m_cursor != prev) audio_sfx(SFX_CURSOR);
	if (m_cursor < m_scroll) m_scroll = m_cursor;
	if (m_cursor >= m_scroll + rows) m_scroll = m_cursor - rows + 1;
	if (m_timer < 6) return;
	bool disabled = mode == MODE_FOLDER ? f_marked[m_cursor] : mode == MODE_PET ? false : m_items[m_cursor].disabled;
	if (btn_pressed(BTN_A) && !disabled && (mode != MODE_FOLDER || m_done)) {
		mode = MODE_NONE;
		audio_sfx(SFX_SELECT);
		if (m_done) m_done(m_cursor);
	} else if (btn_pressed(BTN_B) && m_cancel) {
		mode = MODE_NONE;
		audio_sfx(SFX_CANCEL);
		if (m_done) m_done(-1);
	}
}

void ui_box(int x, int y, int w, int h) {
	fill_rect(x, y, w, h, rgba(236, 244, 255, 245));
	fill_rect(x + 2, y + 2, w - 4, h - 4, rgba(252, 252, 255, 250));
	draw_rect(x, y, w, h, rgba(48, 120, 200, 255));
	draw_rect(x + 1, y + 1, w - 2, h - 2, rgba(140, 200, 255, 255));
}

/* Word-wrapped dialogue in the chat font; the first `limit` characters show. */
static void draw_wrapped(int x, int y, int w, const char *text, int limit) {
	const char *p = text;
	int shown = 0;
	for (int l = 0; l < MSG_LINES && *p && shown < limit; ++l) {
		int n = line_len(p, w);
		char line[200];
		memcpy(line, p, (size_t)n);
		line[n] = 0;
		chat_draw(x, y + l * 16, line, limit - shown < n ? limit - shown : -1);
		shown += n;
		p += n;
		while (*p == ' ') { ++p; ++shown; }
	}
}

/* Short choices (Yes/No and the like) sit in the text window, as the game
 * asks its questions, with the green arrow as cursor. */
static void draw_choice(void) {
	int x = P.core_x, y = P.core_y + 96;
	hud_window(HUD_TEXT, x, y);
	chat_draw(x + TEXT_X_PLAIN, y + 10, m_title, -1);
	for (int i = 0; i < m_n; ++i) {
		int ix = x + 24 + i * 72, iy = y + 36;
		chat_draw(ix, iy, m_items[i].label, -1);
		if (i == m_cursor) rom_tiles(R.layout->ui.next_arrow, R.layout->ui.chat_pal, ix - 16, iy - 4, 2, 2, 0);
	}
}

static void draw_menu(void) {
	if (m_n <= 3 && m_items[0].chip < 0 && !m_items[0].detail[0]) { draw_choice(); return; }
	int w = 180, rows = m_n < 7 ? m_n : 7;
	int h = 22 + rows * 16;
	int x = (P.w - w) / 2, y = (P.h - h) / 2;
	ui_box(x, y, w, h);
	text_draw(x + w / 2, y + 3, m_title, rgba(32, 72, 150, 255), TEXT_CENTER);
	for (int i = 0; i < rows; ++i) {
		int idx = m_scroll + i;
		if (idx >= m_n) break;
		UiItem *it = &m_items[idx];
		int iy = y + 19 + i * 16;
		if (idx == m_cursor) fill_rect(x + 4, iy - 1, w - 8, 15, rgba(120, 190, 255, 255));
		SDL_Color c = it->disabled ? rgba(150, 150, 160, 255) : rgba(24, 32, 64, 255);
		int tx = x + 8;
		if (it->chip >= 0) {
			SDL_Texture *ic = chip_icon(it->chip);
			if (ic) { SDL_Rect d = { x + 6, iy - 1, 14, 14 }; SDL_SetTextureColorMod(ic, 255, 255, 255); SDL_RenderCopy(P.renderer, ic, NULL, &d); }
			tx = x + 24;
		}
		text_draw(tx, iy - 2, it->label, c, TEXT_LEFT);
		if (it->code) text_drawf(x + w - 60, iy - 2, rgba(200, 120, 20, 255), TEXT_LEFT, "%c", it->code);
		if (it->detail[0]) text_draw(x + w - 8, iy - 2, it->detail, c, TEXT_RIGHT);
	}
	if (m_scroll > 0) text_draw(x + w - 10, y + 10, "^", rgba(32, 72, 150, 255), TEXT_CENTER);
	if (m_scroll + rows < m_n) text_draw(x + w - 10, y + h - 12, "v", rgba(32, 72, 150, 255), TEXT_CENTER);
}

static void draw_cards(void) {
	int cw = 64, gap = 6;
	int total = m_n * cw + (m_n - 1) * gap;
	int x0 = (P.w - total) / 2, y0 = P.core_y + 26;
	fill_rect(0, 0, P.w, P.h, rgba(0, 8, 32, 150));
	text_draw(P.w / 2, y0 - 20, m_title, WHITE, TEXT_CENTER);
	for (int i = 0; i < m_n; ++i) {
		UiItem *it = &m_items[i];
		int x = x0 + i * (cw + gap), y = y0 + (i == m_cursor ? -4 : 0);
		draw_frame(x, y, cw, 100, i == m_cursor ? rgba(40, 60, 110, 255) : rgba(24, 32, 64, 255),
			i == m_cursor ? rgba(255, 220, 90, 255) : rgba(140, 170, 220, 255));
		if (it->chip >= 0) {
			SDL_Texture *art = chip_image(it->chip);
			if (art) { SDL_Rect d = { x + 4, y + 16, 56, 48 }; SDL_RenderCopy(P.renderer, art, NULL, &d); }
		}
		text_draw(x + cw / 2, y + 1, it->label, WHITE, TEXT_CENTER);
		if (it->code) text_drawf(x + 5, y + 64, rgba(255, 230, 90, 255), TEXT_LEFT, "%c", it->code);
		if (it->detail[0]) text_draw(x + cw - 4, y + 64, it->detail, WHITE, TEXT_RIGHT);
	}
	/* The highlighted chip's own description underneath. */
	UiItem *cur = &m_items[m_cursor];
	if (cur->chip >= 0) {
		char desc[96];
		chip_desc(cur->chip, desc, sizeof desc);
		for (char *c = desc; *c; ++c) if (*c == '\n') *c = ' ';
		text_draw(P.w / 2, y0 + 106, desc, rgba(200, 220, 255, 255), TEXT_CENTER);
	}
	if (m_cancel) text_draw(P.w / 2, y0 + 122, "B: Skip", rgba(150, 160, 200, 255), TEXT_CENTER);
}

/* 8x16 glyph pair: bottom tile 0x20 after the top */
static void glyph16(uint32_t t, uint32_t pal, int x, int y) {
	rom_tile(t, pal, x, y, 0);
	rom_tile(t + 0x20, pal, x, y + 8, 0);
}

static void draw_folder(void) {
	#define UIL (R.layout->ui)
	int x0 = P.core_x, y0 = P.core_y;
	fill_rect(0, 0, P.w, P.h, rgba(8, 72, 40, 255));
	hud_window(HUD_FOLDER, x0, y0);
	text_draw(x0 + 96, y0 + 14, m_title, WHITE, TEXT_LEFT);
	text_drawf(x0 + 236, y0 + 1, WHITE, TEXT_RIGHT, "%d/30", f_n);
	/* Chip card: the game's own card sprite, then art, code, element, power
	 * and the description in the chat font's fixed cells */
	Sprite *card = sprite_get(SPR_GUI, (int)UIL.card_sprite);
	if (card) {
		SDL_Rect bb = sprite_frame_bounds(card, 0, 0, 0);
		sprite_draw_frame(card, 0, 0, x0 + 8 - bb.x, y0 + 16 - bb.y, false, 0, 0);
	}
	if (m_n > 0) {
		int id = f_ids[m_cursor];
		ChipInfo ci;
		chip_info(id, &ci);
		SDL_Texture *art = chip_image(id);
		if (art) { SDL_Rect d = { x0 + 20, y0 + 21, 56, 48 }; SDL_RenderCopy(P.renderer, art, NULL, &d); }
		char code = f_codes[m_cursor];
		int ci_code = code == '*' ? 26 : code - 'A';
		glyph16(UIL.code_letters + (uint32_t)ci_code * 0x40, UIL.window_pal, x0 + 16, y0 + 69);
		rom_tiles(UIL.list_elem_icons + (uint32_t)ci.chip_element * 0x80, UIL.list_elem_pal, x0 + 32, y0 + 69, 2, 2, 0);
		if (ci.power > 0 && chip_def(id)->kind != CK_RECOVER) {
			char buf[8];
			snprintf(buf, sizeof buf, "%d", ci.power);
			int n = (int)strlen(buf);
			for (int i = 0; i < n && i < 4; ++i) glyph16(UIL.power_digits + (uint32_t)(buf[n - 1 - i] - '0') * 0x40, UIL.window_pal, x0 + 72 - i * 8, y0 + 69);
		}
		char desc[96];
		chip_desc(id, desc, sizeof desc);
		int line = 0;
		for (char *p = desc, *q; line < 3 && p; p = q ? q + 1 : NULL, ++line) {
			q = strchr(p, '\n');
			if (q) *q = 0;
			chat_draw_cells(x0 + 12, y0 + 85 + line * 16, p);
		}
	}
	/* The list */
	for (int r = 0; r < FOLDER_ROWS; ++r) {
		int i = m_scroll + r;
		if (i >= m_n) break;
		int y = y0 + 32 + r * 16;
		ChipInfo ci;
		chip_info(f_ids[i], &ci);
		uint32_t icon = rom_u32(R.layout->chip_data + (uint32_t)f_ids[i] * 0x2C + 0x20);
		if (rom_is_ptr(icon)) rom_tiles(rom_off(icon), UIL.list_icon_pal, x0 + 104, y, 2, 2, 0);
		text_draw(x0 + 120, y, ci.name, f_marked[i] ? rgba(140, 150, 170, 255) : WHITE, TEXT_LEFT);
		rom_tiles(UIL.list_elem_icons + (uint32_t)ci.chip_element * 0x80, UIL.list_elem_pal, x0 + 184, y, 2, 2, 0);
		char code = f_codes[i];
		glyph16(0x6B5A2C + (uint32_t)(code == '*' ? 0x25 : 0x0B + code - 'A') * 64, UIL.list_code_pal, x0 + 200, y);
	}
	if (m_n > 0 && ((P.frame / 8) & 1 || m_timer < 8))
		rom_tiles(UIL.list_arrow, UIL.list_arrow_pal, x0 + 88, y0 + 32 + (m_cursor - m_scroll) * 16, 2, 2, 0);
	#undef UIL
}

static void draw_pet(void) {
	int x0 = P.core_x, y0 = P.core_y;
	hud_pet_menu(x0, y0, m_cursor);
	const uint32_t white = 0x6C7A40; /* the PET's white text palette */
	char buf[40];
	snprintf(buf, sizeof buf, "%d/ %d", pet.hp, pet.max_hp);
	chat_draw_pal(x0 + 216 - chat_width(buf), y0 + 37, buf, white);
	snprintf(buf, sizeof buf, "%dz", pet.zenny);
	chat_draw_pal(x0 + 216 - chat_width(buf), y0 + 61, buf, white);
	snprintf(buf, sizeof buf, "%d", pet.bugfrags);
	chat_draw_pal(x0 + 216 - chat_width(buf), y0 + 85, buf, white);
	chat_draw_pal(x0 + 136, y0 + 121, pet.place, white);
}

void ui_draw(void) {
	if (mode == MODE_PET) draw_pet();
	if (mode == MODE_FOLDER) draw_folder();
	if (mode == MODE_MENU) draw_menu();
	else if (mode == MODE_CARDS) draw_cards();
	if (q_len > 0) {
		Msg *m = &queue[q_head];
		int x = P.core_x, y = P.core_y + 96;
		hud_window(HUD_TEXT, x, y);
		int tx = x + TEXT_X_PLAIN;
		if (m->mugshot >= 0 && mug.spr) {
			/* mugshot top-left at (5, 104), as the original places it */
			SDL_Rect bb = sprite_frame_bounds(mug.spr, mug.anim, mug.frame, 0);
			anim_draw(&mug, x + 5 - bb.x, y + 8 - bb.y, false, 0, 0);
			tx = x + TEXT_X_MUG;
		}
		draw_wrapped(tx, y + 10, text_area_width(m->mugshot), m->text, reveal);
		if (reveal >= (int)strlen(m->text))
			rom_tiles(R.layout->ui.next_arrow, R.layout->ui.chat_pal, x + 226, y + 45 + ((P.frame / 8) & 1), 2, 2, 0);
	}

}
