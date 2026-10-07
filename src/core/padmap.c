#include "padmap.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

/* SDL's names (SDL_gamecontroller.c's map_StringForControllerButton and
 * ...Axis, the same since 2.0.14), an axis's halves as its mappings write
 * them; a trigger goes one way */
static const char *const input_names[PAD_INPUTS] = {
	"a", "b", "x", "y", "back", "guide", "start", "leftstick", "rightstick", "leftshoulder", "rightshoulder",
	"dpup", "dpdown", "dpleft", "dpright", "misc1", "paddle1", "paddle2", "paddle3", "paddle4", "touchpad",
	"-leftx", "+leftx", "-lefty", "+lefty", "-rightx", "+rightx", "-righty", "+righty",
	NULL, "lefttrigger", NULL, "righttrigger",
};
static const char *const gba_names[PAD_GBA] = { "UP", "DOWN", "LEFT", "RIGHT", "A", "B", "L", "R", "START", "SELECT" };
static const char *const family_names[PAD_FAMILIES] = { "pad", "nintendo", "joystick" };

/* (ASCII, either case: the files are written by hand too) */
static bool same_word(const char *a, const char *b) {
	for (; *a && *b; ++a, ++b) {
		char x = *a >= 'A' && *a <= 'Z' ? (char)(*a - 'A' + 'a') : *a, y = *b >= 'A' && *b <= 'Z' ? (char)(*b - 'A' + 'a') : *b;
		if (x != y) return false;
	}
	return *a == *b;
}

const char *padmap_name(int input) { return input >= 0 && input < PAD_INPUTS ? input_names[input] : NULL; }

int padmap_input(const char *name) {
	if (name[0] == '+' && (same_word(name + 1, "lefttrigger") || same_word(name + 1, "righttrigger"))) ++name;
	for (int i = 0; i < PAD_INPUTS; ++i)
		if (input_names[i] && same_word(name, input_names[i])) return i;
	return PAD_NONE;
}

/* ---- a raw joystick's inputs (issue #112) ---- */

static const int hat_bits[4] = { 1, 2, 4, 8 };   /* (SDL's hat values: up, right, down, left) */
static const char *const hat_words[4] = { "up", "right", "down", "left" };

/* its names in pad.ini ("b0", "h0.1", "-a1") and its words on the screen
 * ("Btn 1", "Hat up", "Axis 2-"), made once */
static const char *raw_text(int input, bool words) {
	static char names[PAD_RAW_INPUTS][16], labels[PAD_RAW_INPUTS][20];
	static bool made;
	if (input < 0 || input >= PAD_RAW_INPUTS) return NULL;
	if (!made) {
		made = true;
		for (int i = 0; i < PAD_RAW_INPUTS; ++i) {
			if (i < PAD_RAW_HAT) {
				snprintf(names[i], sizeof names[i], "b%d", i);
				snprintf(labels[i], sizeof labels[i], "Btn %d", i + 1);
			} else if (i < PAD_RAW_AXIS_FIRST) {
				snprintf(names[i], sizeof names[i], "h0.%d", hat_bits[i - PAD_RAW_HAT]);
				snprintf(labels[i], sizeof labels[i], "Hat %s", hat_words[i - PAD_RAW_HAT]);
			} else {
				int a = (i - PAD_RAW_AXIS_FIRST) / 2;
				bool plus = (i - PAD_RAW_AXIS_FIRST) & 1;
				snprintf(names[i], sizeof names[i], "%ca%d", plus ? '+' : '-', a);
				snprintf(labels[i], sizeof labels[i], "Axis %d%c", a + 1, plus ? '+' : '-');
			}
		}
	}
	return words ? labels[input] : names[input];
}

/* "b12", "h0.4", "+a3" (either case) as a raw input, else PAD_NONE */
static int raw_input(const char *name) {
	int n = 0;
	char c = name[0] >= 'A' && name[0] <= 'Z' ? (char)(name[0] - 'A' + 'a') : name[0];
	if (c == 'b' && sscanf(name + 1, "%d", &n) == 1 && n >= 0 && n < PAD_RAW_BUTTONS) return n;
	if (c == 'h' && sscanf(name + 1, "0.%d", &n) == 1)
		for (int k = 0; k < 4; ++k)
			if (hat_bits[k] == n) return PAD_RAW_HAT + k;
	char a = name[1] >= 'A' && name[1] <= 'Z' ? (char)(name[1] - 'A' + 'a') : name[1];
	if ((c == '-' || c == '+') && a == 'a' && sscanf(name + 2, "%d", &n) == 1 && n >= 0 && n < PAD_RAW_AXES) return PAD_RAW_AXIS(n, c == '+');
	return PAD_NONE;
}

int padmap_inputs(int family) { return family == PAD_FAMILY_RAW ? PAD_RAW_INPUTS : PAD_INPUTS; }

int padmap_pov_hat(double v) {
	static const uint8_t ways[8] = { 1, 1 | 2, 2, 2 | 4, 4, 4 | 8, 8, 8 | 1 };
	if (v > 1.01 || v < -1.01) return 0;
	return ways[(int)floor((v + 1.0) * 3.5 + 0.5) & 7];
}

const char *padmap_name_of(int family, int input) { return family == PAD_FAMILY_RAW ? raw_text(input, false) : padmap_name(input); }

int padmap_input_of(int family, const char *name) {
	if (family != PAD_FAMILY_RAW) return padmap_input(name);
	int in = raw_input(name);
	/* (and checked whole: "b1x" names nothing) */
	return in != PAD_NONE && same_word(name, raw_text(in, false)) ? in : PAD_NONE;
}

const char *padmap_gba_name(int gba) { return gba >= 0 && gba < PAD_GBA ? gba_names[gba] : NULL; }

int padmap_gba(const char *name) {
	for (int g = 0; g < PAD_GBA; ++g)
		if (same_word(name, gba_names[g])) return g;
	return -1;
}

/* The words on a pad's buttons per style: generic (a handheld's own: L1,
 * Select), Xbox, PlayStation, Nintendo */
static const char *const words[PAD_INPUTS][4] = {
	[PAD_A] = { "A", "A", "Cross", "A" },
	[PAD_B] = { "B", "B", "Circle", "B" },
	[PAD_X] = { "X", "X", "Square", "X" },
	[PAD_Y] = { "Y", "Y", "Triangle", "Y" },
	[PAD_BACK] = { "Select", "Back", "Share", "Minus" },
	[PAD_GUIDE] = { "Home", "Guide", "PS", "Home" },
	[PAD_START] = { "Start", "Start", "Options", "Plus" },
	[PAD_LEFTSTICK] = { "L3", "LS", "L3", "LS" },
	[PAD_RIGHTSTICK] = { "R3", "RS", "R3", "RS" },
	[PAD_LEFTSHOULDER] = { "L1", "LB", "L1", "L" },
	[PAD_RIGHTSHOULDER] = { "R1", "RB", "R1", "R" },
	[PAD_DPUP] = { "D-pad up", "D-pad up", "D-pad up", "D-pad up" },
	[PAD_DPDOWN] = { "D-pad down", "D-pad down", "D-pad down", "D-pad down" },
	[PAD_DPLEFT] = { "D-pad left", "D-pad left", "D-pad left", "D-pad left" },
	[PAD_DPRIGHT] = { "D-pad right", "D-pad right", "D-pad right", "D-pad right" },
	[PAD_MISC1] = { "Misc", "Share", "Mic", "Capture" },
	[16] = { "P1", "P1", "P1", "P1" },
	[17] = { "P2", "P2", "P2", "P2" },
	[18] = { "P3", "P3", "P3", "P3" },
	[19] = { "P4", "P4", "P4", "P4" },
	[20] = { "Touchpad", "Touchpad", "Touchpad", "Touchpad" },
	[PAD_AXIS(0, false)] = { "L stick left", "L stick left", "L stick left", "L stick left" },
	[PAD_AXIS(0, true)] = { "L stick right", "L stick right", "L stick right", "L stick right" },
	[PAD_AXIS(1, false)] = { "L stick up", "L stick up", "L stick up", "L stick up" },
	[PAD_AXIS(1, true)] = { "L stick down", "L stick down", "L stick down", "L stick down" },
	[PAD_AXIS(2, false)] = { "R stick left", "R stick left", "R stick left", "R stick left" },
	[PAD_AXIS(2, true)] = { "R stick right", "R stick right", "R stick right", "R stick right" },
	[PAD_AXIS(3, false)] = { "R stick up", "R stick up", "R stick up", "R stick up" },
	[PAD_AXIS(3, true)] = { "R stick down", "R stick down", "R stick down", "R stick down" },
	[PAD_LEFTTRIGGER] = { "L2", "LT", "L2", "ZL" },
	[PAD_RIGHTTRIGGER] = { "R2", "RT", "R2", "ZR" },
};

const char *padmap_label(int style, int input) {
	if (style < 0 || style > PAD_STYLE_NINTENDO || input < 0 || input >= PAD_INPUTS || !words[input][style]) return "?";
	return words[input][style];
}

const char *padmap_label_of(int family, int style, int input) {
	if (family != PAD_FAMILY_RAW) return padmap_label(style, input);
	const char *s = raw_text(input, true);
	return s ? s : "?";
}

/* ---- the map ---- */

static void set_row(uint8_t *row, int a, int b) {
	memset(row, PAD_NONE, PAD_PER);
	row[0] = (uint8_t)a;
	if (b != PAD_NONE) row[1] = (uint8_t)b;
}

void padmap_default_family(PadMap *m, int family) {
	uint8_t (*r)[PAD_PER] = m->in[family];
	if (family == PAD_FAMILY_RAW) {
		set_row(r[0], PAD_RAW_HAT + 0, PAD_RAW_AXIS(1, false));
		set_row(r[1], PAD_RAW_HAT + 2, PAD_RAW_AXIS(1, true));
		set_row(r[2], PAD_RAW_HAT + 3, PAD_RAW_AXIS(0, false));
		set_row(r[3], PAD_RAW_HAT + 1, PAD_RAW_AXIS(0, true));
		set_row(r[4], 0, PAD_NONE);
		set_row(r[5], 1, PAD_NONE);
		set_row(r[6], 4, 6);
		set_row(r[7], 5, 7);
		set_row(r[8], 9, 11);
		set_row(r[9], 8, 10);
		return;
	}
	set_row(r[0], PAD_DPUP, PAD_AXIS(1, false));
	set_row(r[1], PAD_DPDOWN, PAD_AXIS(1, true));
	set_row(r[2], PAD_DPLEFT, PAD_AXIS(0, false));
	set_row(r[3], PAD_DPRIGHT, PAD_AXIS(0, true));
	set_row(r[4], PAD_A, PAD_NONE);
	set_row(r[5], PAD_B, PAD_NONE);
	set_row(r[6], PAD_LEFTSHOULDER, PAD_LEFTTRIGGER);
	set_row(r[7], PAD_RIGHTSHOULDER, PAD_RIGHTTRIGGER);
	set_row(r[8], PAD_START, PAD_NONE);
	set_row(r[9], PAD_BACK, PAD_NONE);
}

void padmap_default(PadMap *m) {
	for (int f = 0; f < PAD_FAMILIES; ++f) padmap_default_family(m, f);
}

bool padmap_same(const PadMap *a, const PadMap *b) { return !memcmp(a, b, sizeof *a); }

uint32_t padmap_bits(const PadMap *m, int family, int input) {
	uint32_t bits = 0;
	if (family < 0 || family >= PAD_FAMILIES) return 0;
	for (int g = 0; g < PAD_GBA; ++g)
		for (int i = 0; i < PAD_PER && m->in[family][g][i] != PAD_NONE; ++i)
			if (m->in[family][g][i] == input) bits |= 1u << g;
	return bits;
}

/* An input taken out of a row (the rest moved up); whether it was there. */
static bool take_out(uint8_t *row, int input) {
	int k = 0;
	bool was = false;
	for (int i = 0; i < PAD_PER; ++i) {
		if (row[i] == input) { was = true; continue; }
		if (row[i] != PAD_NONE) row[k++] = row[i];
	}
	while (k < PAD_PER) row[k++] = PAD_NONE;
	return was;
}

void padmap_bind(PadMap *m, int family, int gba, int input) {
	if (family < 0 || family >= PAD_FAMILIES || gba < 0 || gba >= PAD_GBA || !padmap_name_of(family, input)) return;
	uint8_t old[PAD_PER];
	memcpy(old, m->in[family][gba], PAD_PER);
	set_row(m->in[family][gba], input, PAD_NONE);
	for (int g = 0; g < PAD_GBA; ++g) {
		uint8_t *row = m->in[family][g];
		if (g == gba || !take_out(row, input) || row[0] != PAD_NONE) continue;
		/* (left with nothing: what `gba` had, so the two swapped) */
		int k = 0;
		for (int i = 0; i < PAD_PER; ++i)
			if (old[i] != PAD_NONE && old[i] != input) row[k++] = old[i];
	}
}

int padmap_count(const PadMap *m, int family, int gba) {
	int n = 0;
	while (n < PAD_PER && m->in[family][gba][n] != PAD_NONE) ++n;
	return n;
}

int padmap_set_slot(PadMap *m, int family, int gba, int slot, int input) {
	if (family < 0 || family >= PAD_FAMILIES || gba < 0 || gba >= PAD_GBA || slot < 0 || slot >= PAD_PER || !padmap_name_of(family, input))
		return PAD_SLOT_REFUSED;
	uint8_t *row = m->in[family][gba];
	int n = padmap_count(m, family, gba);
	for (int i = 0; i < n; ++i)
		if (row[i] == input) {
			/* (already the button's: to the slot asked, where it has one) */
			if (slot < n && slot != i) { row[i] = row[slot]; row[slot] = (uint8_t)input; }
			return PAD_SLOT_ALONE;
		}
	int old = slot < n ? row[slot] : PAD_NONE;
	/* a button whose only input it is gives it up for the one replaced,
	 * or, with none replaced, keeps it */
	for (int g = 0; g < PAD_GBA; ++g)
		if (g != gba && padmap_count(m, family, g) == 1 && m->in[family][g][0] == input && old == PAD_NONE) return PAD_SLOT_REFUSED;
	int from = PAD_SLOT_ALONE;
	for (int g = 0; g < PAD_GBA; ++g) {
		if (g == gba || !take_out(m->in[family][g], input)) continue;
		from = g;
		if (m->in[family][g][0] == PAD_NONE) m->in[family][g][0] = (uint8_t)old;
	}
	row[slot < n ? slot : n < PAD_PER ? n : PAD_PER - 1] = (uint8_t)input;
	return from;
}

bool padmap_clear_slot(PadMap *m, int family, int gba, int slot) {
	if (family < 0 || family >= PAD_FAMILIES || gba < 0 || gba >= PAD_GBA) return false;
	int n = padmap_count(m, family, gba);
	if (slot < 0 || slot >= n || n < 2) return false;
	take_out(m->in[family][gba], m->in[family][gba][slot]);
	return true;
}

/* ---- the presets: where A and B go ---- */

/* (a raw joystick's first two buttons: b1 is PAD_B's number too) */
static const struct { uint8_t a, b; } presets[PAD_FAMILIES][3] = {
	[PAD_FAMILY_XBOX] = { { PAD_A, PAD_B }, { PAD_A, PAD_X }, { PAD_B, PAD_A } },
	[PAD_FAMILY_NINTENDO] = { { PAD_A, PAD_B }, { PAD_B, PAD_A } },
	[PAD_FAMILY_RAW] = { { 0, 1 }, { 1, 0 } },
};
static const int preset_count[PAD_FAMILIES] = { 3, 2, 2 };

int padmap_presets(int family) { return family >= 0 && family < PAD_FAMILIES ? preset_count[family] : 0; }

const char *padmap_preset_name(int family, int style, int preset) {
	static char s[32];
	if (preset < 0 || preset >= padmap_presets(family)) return "Custom";
	if (preset == 0) return family == PAD_FAMILY_RAW ? "As it came" : "As labeled";
	if (presets[family][preset].a == PAD_B) return "A and B swapped";
	snprintf(s, sizeof s, "B on %s", padmap_label_of(family, style, presets[family][preset].b));
	return s;
}

const char *padmap_preset_about(int family, int style, int preset) {
	static char s[96];
	if (preset < 0 || preset >= padmap_presets(family)) return "A and B set by hand";
	const char *a = padmap_label_of(family, style, presets[family][preset].a), *b = padmap_label_of(family, style, presets[family][preset].b);
	/* (one line under the rows: the preset's own name says which buttons,
	 * and a PlayStation pad's labels are long) */
	if (family == PAD_FAMILY_XBOX && preset == 1) snprintf(s, sizeof s, "B left of A, as on a GBA");
	else if (family == PAD_FAMILY_XBOX && preset == 2) snprintf(s, sizeof s, "A and B as on a Nintendo pad");
	else if (preset == 1) snprintf(s, sizeof s, "For pads that swap A and B");
	else snprintf(s, sizeof s, "A on %s, B on %s", a, b);
	return s;
}

void padmap_preset_apply(PadMap *m, int family, int preset) {
	if (preset < 0 || preset >= padmap_presets(family)) return;
	padmap_bind(m, family, 4, presets[family][preset].a);
	padmap_bind(m, family, 5, presets[family][preset].b);
}

int padmap_preset_of(const PadMap *m, int family) {
	for (int p = 0; p < padmap_presets(family); ++p) {
		const uint8_t *a = m->in[family][4], *b = m->in[family][5];
		if (a[0] == presets[family][p].a && a[1] == PAD_NONE && b[0] == presets[family][p].b && b[1] == PAD_NONE) return p;
	}
	return -1;
}

/* ---- pad.ini ---- */

static char *trim(char *s) {
	while (*s == ' ' || *s == '\t') ++s;
	char *e = s + strlen(s);
	while (e > s && (e[-1] == ' ' || e[-1] == '\t' || e[-1] == '\r' || e[-1] == '\n')) *--e = 0;
	return s;
}

static void say(char *errors, int size, int line, const char *what, const char *name) {
	if (!errors || size <= 0) return;
	size_t n = strlen(errors);
	if (n + 1 < (size_t)size) snprintf(errors + n, (size_t)size - n, "line %d: %s \"%s\"\n", line, what, name);
}

/* "a, -lefty" into a row; false where a name is no input */
static bool parse_inputs(char *list, uint8_t *row, int family, char *errors, int size, int line) {
	bool ok = true;
	int k = 0;
	memset(row, PAD_NONE, PAD_PER);
	for (char *t = list, *comma; t; t = comma ? comma + 1 : NULL) {
		comma = strchr(t, ',');
		if (comma) *comma = 0;
		char *w = trim(t);
		if (!*w) continue;
		int in = padmap_input_of(family, w);
		if (in == PAD_NONE) { say(errors, size, line, "no controller button called", w); ok = false; }
		else if (k < PAD_PER) row[k++] = (uint8_t)in;
	}
	return ok;
}

/* One line: a section, a GBA button's inputs, a comment; false where it
 * says what this does not know. */
static bool parse_line(char *s, PadMap *m, int *family, char *errors, int size, int line) {
	s = trim(s);
	if (!*s || *s == '#' || *s == ';') return true;
	if (*s == '[') {
		char *end = strchr(s, ']');
		if (end) *end = 0;
		for (int f = 0; f < PAD_FAMILIES; ++f)
			if (same_word(trim(s + 1), family_names[f])) { *family = f; return true; }
		say(errors, size, line, "no controllers called", trim(s + 1));
		*family = -1;
		return false;
	}
	char *eq = strchr(s, '=');
	if (!eq) { say(errors, size, line, "no = in", s); return false; }
	*eq = 0;
	int gba = padmap_gba(trim(s));
	if (gba < 0) { say(errors, size, line, "no GBA button called", trim(s)); return false; }
	/* (the file's inputs replace the defaults for this button) */
	return *family < 0 || parse_inputs(eq + 1, m->in[*family][gba], *family, errors, size, line);
}

int padmap_parse(const char *text, PadMap *m, char *errors, int size) {
	padmap_default(m);
	if (errors && size > 0) errors[0] = 0;
	int bad = 0, family = PAD_FAMILY_XBOX, line = 0;
	while (text && *text) {
		const char *nl = strchr(text, '\n');
		size_t n = nl ? (size_t)(nl - text) : strlen(text);
		char buf[256];
		snprintf(buf, sizeof buf, "%.*s", (int)(n < sizeof buf ? n : sizeof buf - 1), text);
		if (!parse_line(buf, m, &family, errors, size, ++line)) ++bad;
		text = nl ? nl + 1 : NULL;
	}
	return bad;
}

int padmap_format(const PadMap *m, char *out, int size) {
	int n = snprintf(out, (size_t)size,
		"# Cyberworld Endless: the controllers' buttons, as the controls screen sets\n"
		"# them (SELECT on the title screen, or CONTROLLER in the touch controls'\n"
		"# menu). Each line gives a GBA button its buttons, as SDL names them: a b x y\n"
		"# back guide start leftstick rightstick leftshoulder rightshoulder dpup\n"
		"# dpdown dpleft dpright misc1 paddle1 paddle2 paddle3 paddle4 touchpad, the\n"
		"# triggers lefttrigger righttrigger, and the sticks pushed one way, -leftx\n"
		"# +leftx -lefty +lefty -rightx +rightx -righty +righty. [pad] is for Xbox,\n"
		"# PlayStation and other controllers, whose a is the bottom button, [nintendo]\n"
		"# for Nintendo's, whose a is the button marked A. [joystick] is for a pad SDL\n"
		"# does not know, read as it is: its buttons b0 b1 b2..., its hat h0.1 (up)\n"
		"# h0.2 (right) h0.4 (down) h0.8 (left), and its axes pushed one way, -a0\n"
		"# +a0 -a1 +a1... Delete this file for the defaults.\n");
	for (int f = 0; f < PAD_FAMILIES && n >= 0 && n < size; ++f) {
		n += snprintf(out + n, (size_t)(size - n), "\n[%s]\n", family_names[f]);
		for (int g = 0; g < PAD_GBA && n >= 0 && n < size; ++g) {
			n += snprintf(out + n, (size_t)(size - n), "%-6s =", gba_names[g]);
			for (int i = 0; i < PAD_PER && m->in[f][g][i] != PAD_NONE && n >= 0 && n < size; ++i)
				if (padmap_name_of(f, m->in[f][g][i])) n += snprintf(out + n, (size_t)(size - n), "%s %s", i ? "," : "", padmap_name_of(f, m->in[f][g][i]));
			if (n >= 0 && n < size) n += snprintf(out + n, (size_t)(size - n), "\n");
		}
	}
	return n < 0 ? 0 : n < size ? n : size - 1;
}

/* ---- Android's Joy-Cons ---- */

/* SDL 2.32's Android driver numbers a pad's buttons by the SDL button its
 * key code stands for (android/SDL_sysjoystick.c, keycode_to_SDL: A 0, B 1,
 * X 2, Y 3, BACK and SELECT 4, MODE 5, START 6, the sticks' presses 7 and 8,
 * L1 9, R1 10, the D-pad 11-14, L2 15, R2 16, Z 18) and its axes as Android
 * sorts them, a stick's X and Y first. Linux's hid-nintendo gives a left
 * Joy-Con BTN_SELECT, BTN_TL, BTN_TL2, BTN_THUMBL, BTN_Z and BTN_DPAD_*,
 * and a right one BTN_SOUTH (B), BTN_EAST (A), BTN_NORTH (X), BTN_WEST (Y),
 * BTN_START, BTN_MODE, BTN_TR, BTN_TR2 and BTN_THUMBR; Android's generic key
 * layout gives those the gamepad's key codes by position (BUTTON_A is the
 * bottom one) and the D-pad's none (GameActivity sends them on as the
 * D-pad's). These are SDL 3's mappings for them (libsdl-org/SDL#15508)
 * with SDL 2's labels on a Nintendo pad, a the button marked A, held
 * sideways a small pad's: the shoulders the one alone has L and R
 * (Android reports neither SL nor SR) and its capture misc1. */
const char *padmap_joycon(int product, bool pair) {
	if (product == 0x2006)
		return pair ? "Joy-Con (L),back:b4,dpdown:b12,dpleft:b13,dpright:b14,dpup:b11,leftshoulder:b9,leftstick:b7,lefttrigger:b15,leftx:a0,lefty:a1,misc1:b18,"
			: "Joy-Con (L),a:b12,b:b13,x:b14,y:b11,leftshoulder:b9,lefttrigger:b15,leftstick:b7,leftx:a1,lefty:a0~,start:b4,misc1:b18,";
	if (product == 0x2007)
		return pair ? "Joy-Con (R),a:b1,b:b0,x:b2,y:b3,guide:b5,rightshoulder:b10,rightstick:b8,righttrigger:b16,rightx:a0,righty:a1,start:b6,"
			: "Joy-Con (R),a:b2,b:b1,x:b3,y:b0,guide:b5,rightshoulder:b10,righttrigger:b16,leftstick:b8,leftx:a1~,lefty:a0,start:b6,";
	return NULL;
}
