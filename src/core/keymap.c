/* keymap.h. The keyboard as the engine reads it: each key a set of the
 * GBA's buttons, the defaults below, keys.ini's over them, written back
 * when the controls screen changes them. */
#include "keymap.h"

#include <stdio.h>
#include <string.h>

#include "buttons.h"
#include "platform.h"

/* The keyboard, after Capcom's own PC layout for Battle Network (the Legacy
 * Collection): WASD moves, J and K are A and B, Q and E are L and R, Enter
 * is Start and R is Select. The arrows with X and Z also work, as on most
 * GBA emulators. Keys are positions (scancodes), not letters, so an AZERTY
 * keyboard moves with ZQSD. keys.ini in the data folder changes them. */
static const struct { const char *name; uint32_t bit; const char *keys; } key_defaults[] = {
	{ "UP", BTN_UP, "W, Up" },
	{ "DOWN", BTN_DOWN, "S, Down" },
	{ "LEFT", BTN_LEFT, "A, Left" },
	{ "RIGHT", BTN_RIGHT, "D, Right" },
	{ "A", BTN_A, "J, X" },
	{ "B", BTN_B, "K, Z" },
	{ "L", BTN_L, "Q" },
	{ "R", BTN_R, "E" },
	{ "START", BTN_START, "Return, Keypad Enter" },
	{ "SELECT", BTN_SELECT, "R, Backspace" },
};
static KeyMap in_play;                         /* each button's keys in order, the first its MAIN */
static uint32_t key_map[SDL_NUM_SCANCODES];    /* (per key its buttons, made from in_play) */
static char keys_path[600];

static void rebuild(void) {
	memset(key_map, 0, sizeof key_map);
	for (int g = 0; g < 10; ++g)
		for (int i = 0; i < KEYS_PER && in_play.slot[g][i]; ++i) key_map[in_play.slot[g][i]] |= 1u << g;
}

static int count_of(const KeyMap *k, int g) {
	int n = 0;
	while (n < KEYS_PER && k->slot[g][n]) ++n;
	return n;
}

/* "J, X" -> button g's keys in that order, after those it has; false and
 * a message for an unknown name */
static bool bind_keys(KeyMap *k, int g, const char *list, const char *where) {
	char buf[256];
	snprintf(buf, sizeof buf, "%s", list);
	bool ok = true;
	char *save = NULL;
	for (char *t = strtok_r(buf, ",", &save); t; t = strtok_r(NULL, ",", &save)) {
		while (*t == ' ' || *t == '\t') ++t;
		char *e = t + strlen(t);
		while (e > t && (e[-1] == ' ' || e[-1] == '\t' || e[-1] == '\r' || e[-1] == '\n')) *--e = 0;
		if (!*t) continue;
		SDL_Scancode sc = SDL_GetScancodeFromName(t);
		if (sc == SDL_SCANCODE_UNKNOWN || sc == SDL_SCANCODE_ESCAPE || sc == SDL_SCANCODE_F11) {
			fprintf(stderr, "%s: no key called \"%s\"%s\n", where, t,
				sc == SDL_SCANCODE_UNKNOWN ? "" : " (Escape and F11 are taken)");
			ok = false;
			continue;
		}
		int n = count_of(k, g);
		bool had = false;
		for (int i = 0; i < n; ++i) had |= k->slot[g][i] == sc;
		if (!had && n < KEYS_PER) k->slot[g][n] = (uint16_t)sc;
	}
	return ok;
}

static void defaults_into(KeyMap *k) {
	memset(k, 0, sizeof *k);
	for (size_t i = 0; i < sizeof key_defaults / sizeof *key_defaults; ++i)
		bind_keys(k, (int)i, key_defaults[i].keys, "defaults");
}

void keymap_default(void) {
	defaults_into(&in_play);
	rebuild();
}

void platform_keys_get(KeyMap *k) { *k = in_play; }
void platform_keys_default(KeyMap *k) { defaults_into(k); }

bool platform_key_free(int sc) {
	if (sc <= SDL_SCANCODE_UNKNOWN || sc >= SDL_NUM_SCANCODES || sc == SDL_SCANCODE_ESCAPE || sc == SDL_SCANCODE_F11 || sc == SDL_SCANCODE_AC_BACK) return false;
	/* (a name keys.ini can say back: "Keypad ," would split its list) */
	const char *name = SDL_GetScancodeName((SDL_Scancode)sc);
	return name[0] && !strchr(name, ',');
}

int platform_keys_slot(const KeyMap *k, int gba, int slot) {
	if (!k) k = &in_play;
	return gba >= 0 && gba < 10 && slot >= 0 && slot < KEYS_PER ? k->slot[gba][slot] : 0;
}

void platform_keys_label(const KeyMap *k, int gba, char *out, size_t n) {
	if (!k) k = &in_play;
	out[0] = 0;
	for (int i = 0; gba >= 0 && gba < 10 && i < KEYS_PER && k->slot[gba][i]; ++i) {
		size_t m = strlen(out);
		snprintf(out + m, n - m, "%s%s", m ? ", " : "", SDL_GetScancodeName((SDL_Scancode)k->slot[gba][i]));
	}
}

/* `sc` out of a row of keys (the rest moved up); whether it was there */
static bool take_key(uint16_t *row, int sc) {
	int k = 0;
	bool was = false;
	for (int i = 0; i < KEYS_PER; ++i) {
		if (row[i] == sc) { was = true; continue; }
		if (row[i]) row[k++] = row[i];
	}
	while (k < KEYS_PER) row[k++] = 0;
	return was;
}

int platform_keys_set_slot(KeyMap *k, int gba, int slot, int sc) {
	if (gba < 0 || gba >= 10 || slot < 0 || slot >= KEYS_PER || !platform_key_free(sc)) return KEYS_REFUSED;
	uint16_t *row = k->slot[gba];
	int n = count_of(k, gba);
	for (int i = 0; i < n; ++i)
		if (row[i] == sc) {
			/* (already the button's: to the slot asked, where it has one) */
			if (slot < n && slot != i) { row[i] = row[slot]; row[slot] = (uint16_t)sc; }
			return KEYS_ALONE;
		}
	int old = slot < n ? row[slot] : 0;
	/* a button whose only key it is gives it up for the one replaced, or,
	 * with none replaced, keeps it */
	for (int g = 0; g < 10; ++g)
		if (g != gba && count_of(k, g) == 1 && k->slot[g][0] == sc && !old) return KEYS_REFUSED;
	int from = KEYS_ALONE;
	for (int g = 0; g < 10; ++g) {
		if (g == gba || !take_key(k->slot[g], sc)) continue;
		from = g;
		if (!k->slot[g][0]) k->slot[g][0] = (uint16_t)old;
	}
	row[slot < n ? slot : n < KEYS_PER ? n : KEYS_PER - 1] = (uint16_t)sc;
	return from;
}

bool platform_keys_clear_slot(KeyMap *k, int gba, int slot) {
	if (gba < 0 || gba >= 10 || slot < 0 || slot >= count_of(k, gba) || count_of(k, gba) < 2) return false;
	take_key(k->slot[gba], k->slot[gba][slot]);
	return true;
}

static const char keys_header[] =
	"# Cyberworld Endless: the keyboard. Each line gives a Game Boy Advance\n"
	"# button its keys, separated by commas, the first its main one. Keys are\n"
	"# named as on a US keyboard (A-Z, 0-9, Up, Down, Left, Right, Space, Return,\n"
	"# Backspace, Tab, Left Shift, Right Shift, Left Ctrl, Keypad 8, Keypad\n"
	"# Enter...) and mean that position: on an AZERTY keyboard W is the key\n"
	"# marked Z. Escape (quit) and F11 (fullscreen) are taken. The controls\n"
	"# screen (SELECT on the title screen) sets them too. Delete this file for\n"
	"# the defaults.\n\n";

/* keys.ini as the map in play has them */
static void keys_write(void) {
	if (!keys_path[0]) return;
	FILE *f = fopen(keys_path, "w");
	if (!f) return;
	fputs(keys_header, f);
	for (size_t i = 0; i < sizeof key_defaults / sizeof *key_defaults; ++i) {
		char list[256];
		platform_keys_label(NULL, (int)i, list, sizeof list);
		fprintf(f, "%-6s = %s\n", key_defaults[i].name, list);
	}
	fclose(f);
	platform_persist();
}

void platform_keys_set(const KeyMap *k) {
	in_play = *k;
	rebuild();
	keys_write();
}

void platform_load_keys(const char *path) {
	keymap_default();
	snprintf(keys_path, sizeof keys_path, "%s", path);
	FILE *f = fopen(path, "r");
	if (!f) {
		keys_write();
		return;
	}
	char line[256];
	int n = 0;
	while (fgets(line, sizeof line, f)) {
		++n;
		char *eq = strchr(line, '=');
		char *hash = strchr(line, '#');
		if (hash && (!eq || hash < eq)) continue;
		if (!eq) continue;
		*eq = 0;
		char name[16] = "";
		sscanf(line, " %15s", name);
		size_t i = 0;
		while (i < sizeof key_defaults / sizeof *key_defaults && SDL_strcasecmp(name, key_defaults[i].name)) ++i;
		char where[600];
		snprintf(where, sizeof where, "%s:%d", path, n);
		if (i == sizeof key_defaults / sizeof *key_defaults) { fprintf(stderr, "%s: no button called \"%s\"\n", where, name); continue; }
		/* the file's keys replace the defaults for this button */
		memset(in_play.slot[i], 0, sizeof in_play.slot[i]);
		bind_keys(&in_play, (int)i, eq + 1, where);
	}
	fclose(f);
	rebuild();
}

uint32_t keymap_button(SDL_Scancode sc) {
	return (unsigned)sc < SDL_NUM_SCANCODES ? key_map[sc] : 0;
}
