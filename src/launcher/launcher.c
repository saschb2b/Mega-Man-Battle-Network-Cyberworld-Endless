/* The launcher (game.h's scene_launcher, issue #97): two cartridge slots
 * before the game, BN6 Cybeast Gregar's (needed) and BN5 Team Colonel's
 * (optional), each an open spot until its ROM is in, then the cartridge
 * with its label; PLAY the moment BN6 is in. A start shows it where it is
 * wanted (no BN6, the first start, BN5 gone since the last; --launcher
 * open, Android's ROMs shortcut); the title's R brings it back over the
 * game, to add BN5. The ROMs come from the system's own chooser (pick.h),
 * a file dropped on the window, or the places the game looks; on a phone
 * the folder chosen may hold the saves a reinstall left there (mirror.h),
 * and the launcher offers them back. Its layout and picture are
 * launcher_draw.c's, its words launcher_text.c's. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio.h"
#include "controls.h"
#include "game.h"
#include "gfx.h"
#include "launcher_roms.h"
#include "launcher_state.h"
#include "mirror.h"
#include "pick.h"
#include "platform.h"
#include "rivals.h"
#include "rom.h"
#include "save.h"

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
		saves_line_phone(L.folder, mirror_last() < 0, L.saves, sizeof L.saves);
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
	switch (L.focus) {
	case FOCUS_BN6:
	case FOCUS_BN5: {
		int slot = L.focus == FOCUS_BN6 ? SLOT_BN6 : SLOT_BN5;
		if (roms_have(slot)) note_slot_ready(slot, slot == SLOT_BN6 && strstr(R.path, L.rom_dir) != R.path ? where : NULL, out, n);
		else note_slot_empty(slot, L.kinds, where, out, n);
		break;
	}
	case FOCUS_ALT: note_alt(L.kinds, out, n); break;
	default:
		if (!roms_have(SLOT_BN6)) { snprintf(out, n, "%s", launcher_word(W_PLAY_LOCKED)); *kind = NOTE_BAD; break; }
		char r[32];
		snprintf(r, sizeof r, "%s", controls_word(BTN_R));
		note_play(roms_have(SLOT_BN5), L.from_title, r, out, n);
		break;
	}
}

/* ---- the saves a folder held ---- */

static void ask_about(const char *path) {
	size_t n = 0;
	uint8_t *b = backup_read_file(path, &n);
	BackupInfo f;
	bool ok = b && backup_info(b, n, &f);
	free(b);
	if (!ok) { say(launcher_word(W_SAVES_DAMAGED), NOTE_BAD); return; }
	BackupInfo here;
	bool have = backup_local_info(g_data_dir, &here);
	if (have && here.hash == f.hash) return;   /* (these saves, the same) */
	L.asking = true;
	L.found = f;
	L.here = here;
	L.here_saves = have;
	/* (the cursor on the saves with more runs in them, this device's at a tie) */
	L.answer = have && here.runs >= f.runs ? ANSWER_NO : ANSWER_YES;
	snprintf(L.found_path, sizeof L.found_path, "%s", path);
	const char *base = strrchr(path, '/');
	ask_saves(L.folder[0] ? L.folder : base ? base + 1 : path, &f, have ? &here : NULL, L.ask_text, sizeof L.ask_text);
	/* (no copy over them while the question is open) */
	mirror_hold(true);
	sound(SFX_REVEAL);
}

/* (the folder's saves copied in to be asked about: gone once answered) */
static void forget_copy(void) {
	if (L.found_copy) remove(L.found_path);
	L.found_copy = false;
}

static void answer(bool yes) {
	L.asking = false;
	mirror_hold(false);
	if (!yes) {
		forget_copy();
		say(launcher_word(L.here_saves ? W_SAVES_KEPT : W_FRESH), NOTE_INFO);
		sound(SFX_CANCEL);
		return;
	}
	size_t n = 0;
	uint8_t *b = backup_read_file(L.found_path, &n);
	bool ok = b && backup_unpack(g_data_dir, b, n);
	free(b);
	forget_copy();
	if (!ok) { say(launcher_word(W_NO_SAVES_BACK), NOTE_BAD); return; }
	/* (over the title the game's profile is read again; from the start it
	 * is read as the game begins) */
	if (L.from_title) {
		save_init();
		rivals_load();
	}
	char s[120];
	note_restored(&L.found, s, sizeof s);
	say(s, NOTE_GOOD);
	sound(SFX_GOT);
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
		note_slot_empty(SLOT_BN6, 0, where, s, sizeof s);
		say(s, NOTE_BAD);
	}
}

/* A picker closed: a desktop's file taken, a phone's look's copies read */
static void picked(void) {
	PickResult r;
	if (!pick_done(&r)) return;
	int asked = L.busy_slot == SLOT_BN5 ? SLOT_BN5 : SLOT_BN6;
	L.busy_slot = -1;
	if (r.status < 0) { say(launcher_word(W_CANCELLED), NOTE_INFO); return; }
	if (r.path) {
		bool beside;
		char s[600];
		int took = roms_take(r.text, L.rom_dir, s, sizeof s, &beside);
		say(s, took >= 0 ? NOTE_GOOD : NOTE_BAD);
		sound(took >= 0 ? SFX_CONFIRM : SFX_ERROR);
		return;
	}
	bool had[SLOTS] = { roms_have(SLOT_BN6), roms_have(SLOT_BN5) };
	roms_reload(L.rom_dir);
	bool came = (!had[SLOT_BN6] && roms_have(SLOT_BN6)) || (!had[SLOT_BN5] && roms_have(SLOT_BN5));
	say(r.text, came ? NOTE_GOOD : roms_have(asked) ? NOTE_INFO : NOTE_BAD);
	/* (a folder newly chosen: the saves copied there at once, or, where it
	 * held saves of its own, once the question over them is answered) */
	char was[sizeof L.folder];
	snprintf(was, sizeof was, "%s", L.folder);
	if (!pick_folder(L.folder, sizeof L.folder)) snprintf(L.folder, sizeof L.folder, "%s", was);
	else if (strcmp(was, L.folder) || mirror_last() < 0) mirror_new_folder();
	if (r.saves) {
		char path[620];
		snprintf(path, sizeof path, "%s/found.cwsave", g_data_dir);
		ask_about(path);
		/* (no question: the same saves as here, or damaged) */
		if (!L.asking) remove(path);
		else L.found_copy = true;
	}
}

/* a file dropped on the window: a ROM, or a .cwsave's saves */
static void dropped(void) {
	char path[1024];
	if (!platform_dropped(path, sizeof path)) return;
	size_t n = strlen(path);
	if (n > 7 && !strcmp(path + n - 7, ".cwsave")) {
		if (!L.asking) ask_about(path);
		return;
	}
	bool beside;
	char s[600];
	int took = roms_take(path, L.rom_dir, s, sizeof s, &beside);
	say(s, took >= 0 ? NOTE_GOOD : NOTE_BAD);
	sound(took >= 0 ? SFX_CONFIRM : SFX_ERROR);
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
	int slot = what == FOCUS_BN6 ? SLOT_BN6 : SLOT_BN5;
	if (roms_have(slot)) return;
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

/* The cursor's neighbours: the cartridges side by side over the buttons,
 * or (held upright) all in one column */
static int neighbour(int from, int dx, int dy, bool upright, bool alt) {
	static const int across[FOCUSES][4] = {
		/* left, right, up, down */
		[FOCUS_BN6] = { -1, FOCUS_BN5, -1, FOCUS_ALT },
		[FOCUS_BN5] = { FOCUS_BN6, -1, -1, FOCUS_PLAY },
		[FOCUS_ALT] = { -1, FOCUS_PLAY, FOCUS_BN6, -1 },
		[FOCUS_PLAY] = { FOCUS_ALT, -1, FOCUS_BN5, -1 },
	};
	static const int column[FOCUSES] = { FOCUS_BN6, FOCUS_BN5, FOCUS_PLAY, FOCUS_ALT };
	int to = -1;
	if (upright) {
		int at = 0;
		while (at < FOCUSES && column[at] != from) ++at;
		int step = dx + dy;
		if (at + step >= 0 && at + step < FOCUSES) to = column[at + step];
	} else to = across[from][dx < 0 ? 0 : dx > 0 ? 1 : dy < 0 ? 2 : 3];
	if (to == FOCUS_ALT && !alt) to = upright ? -1 : from == FOCUS_BN6 ? FOCUS_PLAY : -1;
	return to;
}

static int hit(const LauncherLayout *lay, int x, int y) {
	SDL_Point p = { x, y };
	for (int s = 0; s < SLOTS; ++s) {
		SDL_Rect r = lay->cart[s];
		r.h = lay->caption[s].y + lay->caption[s].h - r.y;
		if (SDL_PointInRect(&p, &r)) return s == SLOT_BN6 ? FOCUS_BN6 : FOCUS_BN5;
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

/* the question: left and right choose, A answers, B keeps what is here */
static void ask_update(const LauncherLayout *lay) {
	int x, y;
	SDL_Point p;
	if (platform_tap(&x, &y)) {
		p = (SDL_Point){ x, y };
		if (SDL_PointInRect(&p, &lay->ask_yes)) { answer(true); return; }
		if (SDL_PointInRect(&p, &lay->ask_no)) { answer(false); return; }
	}
	if (platform_pointer(&x, &y)) {
		p = (SDL_Point){ x, y };
		if (SDL_PointInRect(&p, &lay->ask_yes)) L.answer = ANSWER_YES;
		if (SDL_PointInRect(&p, &lay->ask_no)) L.answer = ANSWER_NO;
	}
	if (btn_pressed(BTN_LEFT) || btn_pressed(BTN_RIGHT) || btn_pressed(BTN_UP) || btn_pressed(BTN_DOWN)) {
		L.answer ^= 1;
		sound(SFX_CURSOR);
	}
	if (btn_pressed(BTN_A) || btn_pressed(BTN_START)) answer(L.answer == ANSWER_YES);
	else if (btn_pressed(BTN_B)) answer(false);
}

/* Escape or Back: the question's No; over the title, back to it; at the
 * start the quit prompt's (false) */
static bool back(void) {
	if (L.asking) { answer(false); return true; }
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
	if (!L.note[0]) L.focus = roms_have(SLOT_BN6) ? FOCUS_PLAY : FOCUS_BN6;
	if (!pick_folder(L.folder, sizeof L.folder)) L.folder[0] = 0;
	saves_line();
}

static void leave(void) {
	platform_own_taps(false);
	platform_on_back(NULL);
	mirror_hold(false);
}

static void update(void) {
	++L.t;
	LauncherLayout lay;
	launcher_layout(&lay);
	picked();
	dropped();
	if (!roms_have(SLOT_BN6) && !pick_busy() && L.t - L.looked_at >= LOOK_FRAMES) look_again(false);
	fills();
	/* (kept up: a copy refused shows at once) */
	saves_line();
	if (L.asking) { ask_update(&lay); return; }
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
	bool bn6 = roms_have(SLOT_BN6), bn5 = roms_have(SLOT_BN5);
	RomRecord last = roms_record_read();
	if (mode == LAUNCHER_OFF || !roms_launcher_wanted(mode == LAUNCHER_OPEN, bn6, bn5, last)) {
		/* (BN5 come since the last start: the title says so; written, so
		 * that its going is seen) */
		if (mode != LAUNCHER_OFF && (last.bn6 != bn6 || last.bn5 != bn5)) roms_record_write();
		return false;
	}
	clear();
	L.begin = begin;
	if (bn6) gfx_init();
	if (bn6 && last.known && last.bn5 && !bn5) {
		note_five_gone(L.note, sizeof L.note);
		L.note_kind = NOTE_BAD;
		L.focus = L.note_focus = FOCUS_BN5;
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
