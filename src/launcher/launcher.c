/* The launcher (game.h's scene_launcher, issue #97): cartridge slots
 * before the game, BN6 Cybeast Gregar's (needed) and one each extra ROM
 * offered (optional: BN5 Team Colonel's and BN4 Red Sun's, rom.h's
 * xrom_offered), each an open spot until
 * its ROM is in, then the cartridge with its label; PLAY the moment BN6
 * is in. A start shows it where it is wanted (no BN6, the first start, an
 * extra ROM gone since the last; --launcher open, Android's ROMs
 * shortcut); the title's R brings it back over the game, to add BN5. The
 * ROMs come from the system's own chooser (pick.h), a file dropped on the
 * window, or the places the game looks; on a phone the folder chosen may
 * hold the saves a reinstall left there (mirror.h), which SAVES compares
 * with this device's (src/saves/). Its layout and
 * picture are launcher_draw.c's, its words launcher_text.c's. */
#include "launcher_state.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio.h"
#include "controls.h"
#include "game.h"
#include "gfx.h"
#include "launcher_roms.h"
#include "mirror.h"
#include "pick.h"
#include "platform.h"
#include "rivals.h"
#include "rom.h"
#include "save.h"
#include "saves.h"

#define LOOK_FRAMES 180   /* a ROM put in by hand looked for every three seconds */

Launcher L;
static char start_rom_dir[600];   /* (the start's: --rom-dir's, else the data folder's rom/) */

bool launcher_here(void) {
#if defined(CW_DESKTOP) || defined(CW_IOS) || defined(__ANDROID__)
	return true;
#else
	return false;
#endif
}

/* ---- notes ---- */

static void say(const char *s, int kind) {
	snprintf(L.note, sizeof L.note, "%s", s);
	L.note_kind = kind;
	L.note_focus = L.focus;
}

/* a sound where the game's are up (over the title: there are none before
 * the game's ROM is read and begun) */
static void sound(Sfx s) {
	if (L.from_title) audio_sfx(s);
}

/* A path as a player reads it: the home folder as ~ (a Windows one with \) */
static void shown_path(const char *path, char *out, size_t n) {
	const char *home = getenv("HOME");
	size_t hl = home ? strlen(home) : 0;
	if (hl > 1 && !strncmp(path, home, hl) && path[hl] == '/') snprintf(out, n, "~%s", path + hl);
	else snprintf(out, n, "%s", path);
#ifdef _WIN32
	for (char *c = out; *c; ++c)
		if (*c == '/') *c = '\\';
#endif
}

static void saves_line(void) {
	char path[700];
	if (L.kinds & PICK_FOLDER) {
		MirrorStatus m;
		mirror_status(&m);
		saves_line_phone(m.place, m.state != MIRROR_OFF, m.state == MIRROR_FAILED, L.saves, sizeof L.saves);
		return;
	}
	snprintf(path, sizeof path, "%s/savedata", g_data_dir);
	char shown[700];
	shown_path(path, shown, sizeof shown);
	saves_line_desktop(shown, L.saves, sizeof L.saves);
}

void launcher_cursor_note(char *out, size_t n, int *kind) {
	*kind = NOTE_INFO;
	if (L.note[0] && L.note_focus == L.focus) {
		snprintf(out, n, "%s", L.note);
		*kind = L.note_kind;
		return;
	}
	char where[700];
	shown_path(L.rom_dir, where, sizeof where);
	unsigned shown = roms_shown_set(false), in = roms_shown_set(true), extras = ~(1u << SLOT_BN6);
	if (L.focus < SLOTS) {
		int slot = L.focus;
		if (roms_have(slot)) note_slot_ready(slot, slot == SLOT_BN6 && strstr(R.path, L.rom_dir) != R.path ? where : NULL, out, n);
		else note_slot_empty(slot, L.kinds, where, shown, out, n);
	} else if (L.focus == FOCUS_ALT) note_alt(L.kinds, shown, out, n);
	else if (!roms_have(SLOT_BN6)) {
		snprintf(out, n, "%s", launcher_word(W_PLAY_LOCKED));
		*kind = NOTE_BAD;
	} else {
		char r[32];
		snprintf(r, sizeof r, "%s", controls_word(BTN_R));
		note_play(in & extras, shown & ~in & extras, L.from_title, r, out, n);
	}
}

/* ---- what comes in ---- */

/* the ROMs looked for again: the places by hand, a phone's folder kept;
 * `said`: what came of it said (a press, not the look every few seconds) */
static void look_again(bool said) {
	char msg[512] = "";
	bool had = roms_have(SLOT_BN6);
	if (L.kinds & PICK_FOLDER) pick_look(msg, sizeof msg);
	roms_reload(L.rom_dir);
	L.looked_at = L.t;
	if (msg[0]) say(msg, roms_have(SLOT_BN6) ? NOTE_INFO : NOTE_BAD);
	else if (said && !had && roms_have(SLOT_BN6)) {
		char where[700], dir[700], s[800];
		snprintf(dir, sizeof dir, "%s", R.path);
		char *slash = strrchr(dir, '/');
		if (slash) *slash = 0;
		shown_path(dir, where, sizeof where);
		note_found(where, s, sizeof s);
		say(s, NOTE_GOOD);
	} else if (said && !had) {
		char where[700], s[800];
		shown_path(L.rom_dir, where, sizeof where);
		note_slot_empty(SLOT_BN6, 0, where, roms_shown_set(false), s, sizeof s);
		say(s, NOTE_BAD);
	}
}

/* A picker closed: a desktop's file taken, a phone's look's copies read */
static bool picked(void) {
	PickResult r;
	if (!pick_done(&r)) return false;
	int asked = L.busy_slot > 0 && L.busy_slot < SLOTS ? L.busy_slot : SLOT_BN6;
	L.busy_slot = -1;
	if (r.status < 0) { say(launcher_word(W_CANCELLED), NOTE_INFO); return false; }
	if (r.path) {
		unsigned beside;
		char s[600];
		int took = roms_take(r.text, L.rom_dir, s, sizeof s, &beside);
		say(s, took >= 0 ? NOTE_GOOD : NOTE_BAD);
		sound(took >= 0 ? SFX_CONFIRM : SFX_ERROR);
		return false;
	}
	unsigned had = roms_shown_set(true);
	roms_reload(L.rom_dir);
	bool came = (roms_shown_set(true) & ~had) != 0;
	say(r.text, came ? NOTE_GOOD : roms_have(asked) ? NOTE_INFO : NOTE_BAD);
	/* (a folder newly chosen: looked at for the saves' copy, which SAVES
	 * compares where it holds saves not answered, saves.h) */
	char was[sizeof L.folder];
	snprintf(was, sizeof was, "%s", L.folder);
	if (!pick_folder(L.folder, sizeof L.folder)) snprintf(L.folder, sizeof L.folder, "%s", was);
	else if (strcmp(was, L.folder)) mirror_new_folder();
	return false;
}

/* a file dropped on the window: a ROM, or a .cwsave's saves */
static bool dropped(void) {
	char path[1024];
	if (pick_busy() || pick_saves_busy()) return false;
	if (!platform_dropped(path, sizeof path)) return false;
	size_t n = strlen(path);
	if (n > 7 && !SDL_strcasecmp(path + n - 7, ".cwsave")) {
		saves_import(path);
		return true;
	}
	unsigned beside;
	char s[600];
	int took = roms_take(path, L.rom_dir, s, sizeof s, &beside);
	say(s, took >= 0 ? NOTE_GOOD : NOTE_BAD);
	sound(took >= 0 ? SFX_CONFIRM : SFX_ERROR);
	return false;
}

/* a cartridge just in: its drop, and the cursor on to PLAY once BN6 is */
static void fills(void) {
	for (int s = 0; s < SLOTS; ++s) {
		if (L.had[s] == roms_have(s)) continue;
		L.had[s] = roms_have(s);
		if (!L.had[s]) continue;
		L.filled_at[s] = L.t;
		/* (the game's font, for its quit prompt and what comes after) */
		if (s == SLOT_BN6) gfx_init();
		if (s == SLOT_BN6 && L.focus != FOCUS_PLAY) {
			L.focus = FOCUS_PLAY;
			L.note_focus = L.note[0] ? FOCUS_PLAY : L.note_focus;
		}
	}
}

/* ---- what the player does ---- */

static void leave_for_game(void) {
	roms_record_write();
	if (L.from_title) {
		sound(SFX_CANCEL);
		scene_set(&scene_title);
		return;
	}
	/* (the game from the ROM chosen: the start's own way on, main.c) */
	if (L.begin) L.begin();
}

static void activate(int what) {
	if (pick_busy()) return;
	if (what == FOCUS_PLAY) {
		if (roms_have(SLOT_BN6)) leave_for_game();
		else {
			say(launcher_word(W_PLAY_LOCKED), NOTE_BAD);
			sound(SFX_ERROR);
		}
		return;
	}
	if (what == FOCUS_ALT) {
		if ((L.kinds & PICK_FILES) && pick_open(PICK_FILES, -1)) L.busy_slot = SLOTS;
		else look_again(true);
		sound(SFX_SELECT);
		return;
	}
	int slot = what;
	if (slot < 0 || slot >= SLOTS || roms_have(slot)) return;
	int kind = L.kinds & PICK_FOLDER ? PICK_FOLDER : L.kinds & PICK_FILE ? PICK_FILE : 0;
	if (kind && pick_open(kind, slot)) L.busy_slot = slot;
	else {
		look_again(false);
		if (!roms_have(slot)) {
			char where[700], s[800];
			shown_path(L.rom_dir, where, sizeof where);
			note_no_picker(where, s, sizeof s);
			say(s, NOTE_INFO);
		}
	}
	sound(SFX_SELECT);
}

/* The cursor's neighbours: the cartridges shown side by side over the
 * buttons, the first over the second button and the rest over PLAY, or
 * (held upright) all in one column */
static int neighbour(int from, int dx, int dy, bool upright, bool alt) {
	int column[FOCUSES], n = 0, at = -1;
	for (int s = 0; s < SLOTS; ++s)
		if (roms_shown(s)) column[n++] = s;
	int carts = n;
	column[n++] = FOCUS_PLAY;
	column[n++] = FOCUS_ALT;
	for (int i = 0; i < n; ++i)
		if (column[i] == from) at = i;
	int to = -1;
	if (at < 0) return -1;
	if (upright) {
		int step = dx + dy;
		if (at + step >= 0 && at + step < n) to = column[at + step];
	} else if (from < SLOTS) {
		if (dx) to = at + dx >= 0 && at + dx < carts ? column[at + dx] : -1;
		else if (dy > 0) to = at == 0 ? FOCUS_ALT : FOCUS_PLAY;
	} else if (from == FOCUS_ALT) to = dx > 0 ? FOCUS_PLAY : dy < 0 ? column[0] : -1;
	else to = dx < 0 ? FOCUS_ALT : dy < 0 ? column[carts - 1] : -1;
	if (to == FOCUS_ALT && !alt) to = upright ? -1 : from < SLOTS ? FOCUS_PLAY : -1;
	return to;
}

static int hit(const LauncherLayout *lay, int x, int y) {
	SDL_Point p = { x, y };
	for (int s = 0; s < SLOTS; ++s) {
		SDL_Rect r = lay->cart[s];
		r.h = lay->caption[s].y + lay->caption[s].h - r.y;
		if (r.w && SDL_PointInRect(&p, &r)) return s;
	}
	if (lay->alt.w && SDL_PointInRect(&p, &lay->alt)) return FOCUS_ALT;
	if (SDL_PointInRect(&p, &lay->play)) return FOCUS_PLAY;
	return -1;
}

static void move(int to) {
	if (to < 0 || to == L.focus) return;
	L.focus = to;
	sound(SFX_CURSOR);
}

/* the mouse over a piece puts the cursor on it; a tap or click presses it */
static void pointer(const LauncherLayout *lay) {
	int x, y, over = -1;
	if (platform_pointer(&x, &y)) over = hit(lay, x, y);
	if (over >= 0 && over != L.pointed) move(over);
	L.pointed = over;
	if (!platform_tap(&x, &y)) return;
	int h = hit(lay, x, y);
	if (h < 0) return;
	move(h);
	activate(h);
}

static void keys(const LauncherLayout *lay) {
	int dx = btn_repeat(BTN_LEFT) ? -1 : btn_repeat(BTN_RIGHT) ? 1 : 0, dy = btn_repeat(BTN_UP) ? -1 : btn_repeat(BTN_DOWN) ? 1 : 0;
	if (dx || dy) move(neighbour(L.focus, dx, dy, lay->upright, lay->alt.w > 0));
	if (btn_pressed(BTN_A)) activate(L.focus);
	else if (btn_pressed(BTN_START) && roms_have(SLOT_BN6) && !pick_busy()) leave_for_game();
	else if (btn_pressed(BTN_B) && L.from_title && !pick_busy()) leave_for_game();
}

static bool back(void) {
	if (!L.from_title || pick_busy()) return false;
	leave_for_game();
	return true;
}

/* ---- the scene ---- */

static void enter(void) {
	platform_own_taps(true);
	platform_on_back(back);
	/* (no second screen: its PET draws in the game's font) */
	platform_second_screen(NULL, NULL);
	L.t = 0;
	L.kinds = pick_kinds();
	L.busy_slot = -1;
	L.pointed = -1;
	for (int s = 0; s < SLOTS; ++s) {
		L.had[s] = roms_have(s);
		L.filled_at[s] = -1;
	}
	if (!L.note[0]) L.focus = roms_have(SLOT_BN6) ? FOCUS_PLAY : SLOT_BN6;
	if (!pick_folder(L.folder, sizeof L.folder)) L.folder[0] = 0;
	saves_line();
}

static void leave(void) {
	platform_own_taps(false);
	platform_on_back(NULL);
}

static void update(void) {
	++L.t;
	LauncherLayout lay;
	launcher_layout(&lay);
	if (picked() || dropped()) return;
	saves_tick();
	if (!roms_have(SLOT_BN6) && !pick_busy() && L.t - L.looked_at >= LOOK_FRAMES) look_again(false);
	fills();
	/* (kept up: a copy refused shows at once) */
	saves_line();
	pointer(&lay);
	keys(&lay);
	if (L.note[0] && L.note_focus != L.focus) L.note[0] = 0;
}

static void draw(void) {
	LauncherLayout lay;
	launcher_layout(&lay);
	launcher_draw_all(&lay);
}

const Scene scene_launcher = { "launcher", enter, update, draw, leave };

/* ---- the ways in ---- */

static void clear(void) {
	memset(&L, 0, sizeof L);
	L.looked_at = -LOOK_FRAMES;
	snprintf(L.rom_dir, sizeof L.rom_dir, "%s", start_rom_dir);
}

bool launcher_start(int mode, const char *rom_dir, void (*begin)(void)) {
	if (rom_dir) snprintf(start_rom_dir, sizeof start_rom_dir, "%s", rom_dir);
	else snprintf(start_rom_dir, sizeof start_rom_dir, "%s/rom", g_data_dir);
	RomRecord now = roms_record_now(), last = roms_record_read();
	bool bn6 = now.have[SLOT_BN6], changed = false;
	int gone = -1;
	for (int s = 0; s < SLOTS; ++s) {
		changed |= roms_shown(s) && last.have[s] != now.have[s];
		if (gone < 0 && s != SLOT_BN6 && last.have[s] && !now.have[s]) gone = s;
	}
	if (mode == LAUNCHER_OFF || !roms_launcher_wanted(mode == LAUNCHER_OPEN, now, last)) {
		/* (an extra ROM come since the last start: the title says so;
		 * written, so that its going is seen) */
		if (mode != LAUNCHER_OFF && changed) roms_record_write();
		return false;
	}
	clear();
	L.begin = begin;
	if (bn6) gfx_init();
	if (bn6 && last.known && gone >= 0) {
		note_gone(gone, L.note, sizeof L.note);
		L.note_kind = NOTE_BAD;
		L.focus = L.note_focus = gone;
	} else if (bn6 && strncmp(R.path, L.rom_dir, strlen(L.rom_dir))) {
		/* (found where front ends and downloads keep theirs) */
		char where[700], dir[700];
		snprintf(dir, sizeof dir, "%s", R.path);
		char *slash = strrchr(dir, '/');
		if (slash) *slash = 0;
		shown_path(dir, where, sizeof where);
		note_found(where, L.note, sizeof L.note);
		L.note_kind = NOTE_GOOD;
		L.focus = L.note_focus = FOCUS_PLAY;
	}
	scene_set(&scene_launcher);
	return true;
}

bool launcher_open(void) {
	if (!launcher_here() || !R.data) return false;
	clear();
	L.from_title = true;
	scene_set(&scene_launcher);
	return true;
}
