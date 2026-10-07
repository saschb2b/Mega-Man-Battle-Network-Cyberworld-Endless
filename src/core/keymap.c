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
static uint32_t key_map[SDL_NUM_SCANCODES];
static char keys_path[600];

/* "J, X" -> the button on each key; false and a message for an unknown name */
static bool bind_keys(uint32_t *map, uint32_t bit, const char *list, const char *where) {
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
		map[sc] |= bit;
	}
	return ok;
}

static void defaults_into(uint32_t *map) {
	memset(map, 0, sizeof key_map);
	for (size_t i = 0; i < sizeof key_defaults / sizeof *key_defaults; ++i)
		bind_keys(map, key_defaults[i].bit, key_defaults[i].keys, "defaults");
}

void keymap_default(void) { defaults_into(key_map); }

void platform_keys_get(KeyMap *k) { memcpy(k->bits, key_map, sizeof key_map); }
void platform_keys_default(KeyMap *k) { defaults_into(k->bits); }

bool platform_key_free(int sc) {
	if (sc <= SDL_SCANCODE_UNKNOWN || sc >= SDL_NUM_SCANCODES || sc == SDL_SCANCODE_ESCAPE || sc == SDL_SCANCODE_F11 || sc == SDL_SCANCODE_AC_BACK) return false;
	/* (a name keys.ini can say back: "Keypad ," would split its list) */
	const char *name = SDL_GetScancodeName((SDL_Scancode)sc);
	return name[0] && !strchr(name, ',');
}

void platform_keys_label(const KeyMap *k, int gba, char *out, size_t n) {
	const uint32_t *map = k ? k->bits : key_map;
	out[0] = 0;
	for (int sc = 0; sc < SDL_NUM_SCANCODES && gba >= 0 && gba < 10; ++sc) {
		const char *name = SDL_GetScancodeName((SDL_Scancode)sc);
		if (!(map[sc] >> gba & 1) || !name[0]) continue;
		size_t m = strlen(out);
		snprintf(out + m, n - m, "%s%s", m ? ", " : "", name);
	}
}

void platform_keys_bind(KeyMap *k, int gba, int sc) {
	if (gba < 0 || gba >= 10 || !platform_key_free(sc)) return;
	static bool had[SDL_NUM_SCANCODES];   /* (the keys `gba` had) */
	uint32_t bit = 1u << gba, others = k->bits[sc] & ~bit;
	for (int s = 0; s < SDL_NUM_SCANCODES; ++s) {
		had[s] = k->bits[s] & bit;
		k->bits[s] &= ~bit;
	}
	k->bits[sc] = bit;
	/* (a button the key was taken from, left with none, takes them: the two swap) */
	for (int g = 0; g < 10; ++g) {
		bool any = false;
		if (!(others >> g & 1)) continue;
		for (int s = 0; s < SDL_NUM_SCANCODES && !any; ++s) any = k->bits[s] >> g & 1;
		for (int s = 0; s < SDL_NUM_SCANCODES && !any; ++s)
			if (had[s] && s != sc) k->bits[s] |= 1u << g;
	}
}

void platform_keys_add(KeyMap *k, int gba, int sc) {
	if (gba < 0 || gba >= 10 || !platform_key_free(sc)) return;
	k->bits[sc] = 1u << gba;
}

static const char keys_header[] =
	"# Cyberworld Endless: the keyboard. Each line gives a Game Boy Advance\n"
	"# button its keys, separated by commas. Keys are named as on a US\n"
	"# keyboard (A-Z, 0-9, Up, Down, Left, Right, Space, Return, Backspace, Tab,\n"
	"# Left Shift, Right Shift, Left Ctrl, Keypad 8, Keypad Enter...) and mean\n"
	"# that position: on an AZERTY keyboard W is the key marked Z. Escape (quit)\n"
	"# and F11 (fullscreen) are taken. The controls screen (SELECT on the title\n"
	"# screen) sets A, B, L, R, START and SELECT too. Delete this file for the\n"
	"# defaults.\n\n";

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
	memcpy(key_map, k->bits, sizeof key_map);
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
		for (int sc = 0; sc < SDL_NUM_SCANCODES; ++sc) key_map[sc] &= ~key_defaults[i].bit;
		bind_keys(key_map, key_defaults[i].bit, eq + 1, where);
	}
	fclose(f);
}

uint32_t keymap_button(SDL_Scancode sc) {
	return (unsigned)sc < SDL_NUM_SCANCODES ? key_map[sc] : 0;
}
