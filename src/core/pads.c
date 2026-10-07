#include "pads.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "buttons.h"
#include "platform.h"

/* (padmap.c numbers a pad's inputs as SDL numbers its buttons and axes;
 * SDL 2.0.14, the oldest a handheld's firmware brings, has these 21) */
_Static_assert((int)PAD_BUTTONS == (int)SDL_CONTROLLER_BUTTON_MAX, "padmap.h's buttons are SDL's");
_Static_assert((int)PAD_AXES == (int)SDL_CONTROLLER_AXIS_MAX, "padmap.h's axes are SDL's");
_Static_assert((int)PAD_BACK == (int)SDL_CONTROLLER_BUTTON_BACK && (int)PAD_DPRIGHT == (int)SDL_CONTROLLER_BUTTON_DPAD_RIGHT &&
	(int)PAD_MISC1 == (int)SDL_CONTROLLER_BUTTON_MISC1 && PAD_LEFTTRIGGER == PAD_AXIS((int)SDL_CONTROLLER_AXIS_TRIGGERLEFT, true),
	"padmap.h's names are SDL's numbers");

_Static_assert(PAD_INPUTS_MOST <= 63, "a family's inputs are bits of one 64-bit set");

#define SLOTS 8
#define PUSHED 16000   /* a stick or trigger past this (of 32767) is pushed */
#define BIT(input) ((uint64_t)1 << (input))
#define AXES_MASK ((BIT(PAD_AXES * 2) - 1) << PAD_AXIS_FIRST)
#define RAW_POLLED ((BIT(PAD_RAW_INPUTS) - 1) & ~(BIT(PAD_RAW_HAT) - 1))   /* (a raw joystick's hat and axes) */
#define DIRECTIONS (BTN_UP | BTN_DOWN | BTN_LEFT | BTN_RIGHT)

/* An open controller: its family and style, and the inputs held (its
 * buttons from SDL's events, its sticks and triggers read each frame); or
 * a joystick SDL makes no controller of, read raw (issue #112): where its
 * axes rest (a trigger at an end) and, in a browser, an axis that is its
 * hat (Chrome's). */
static struct {
	SDL_GameController *gc;
	SDL_Joystick *joy;
	SDL_JoystickID id;
	int family, style;
	uint64_t on;
	int naxes, nhats, pov;
	Sint16 rest[PAD_RAW_AXES];
} slot[SLOTS];
static PadMap map;
static bool map_ready;
static uint32_t lut[PAD_FAMILIES][PAD_INPUTS_MOST];   /* (the map: an input's GBA buttons) */
static uint32_t taken, menu_taken;
static PadPress press[8];
static int npress, last = -1;
static char path[600];

/* (Android's log takes SDL_Log, not stdout) */
static void say(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
static void say(const char *fmt, ...) {
	char s[256];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(s, sizeof s, fmt, ap);
	va_end(ap);
#ifdef __ANDROID__
	SDL_Log("%s", s);
#else
	printf("%s\n", s);
#endif
}

static void relut(void) {
	for (int f = 0; f < PAD_FAMILIES; ++f)
		for (int i = 0; i < PAD_INPUTS_MOST; ++i) lut[f][i] = padmap_bits(&map, f, i);
}

static void map_init(void) {
	if (map_ready) return;
	padmap_default(&map);
	relut();
	map_ready = true;
}

/* ---- which pad is which ---- */

/* Nintendo's pads (SDL 2 names their buttons by their labels, a the one
 * marked A: SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS, on by default), and
 * the words on the others' buttons. (Joy-Cons are SDL 2.24's types 11-13,
 * which the older SDL's headers a handheld's build reads do not name.) */
static void kind_of(SDL_GameController *gc, int *family, int *style) {
	int type = (int)SDL_GameControllerGetType(gc);
	*family = PAD_FAMILY_XBOX;
	*style = PAD_STYLE_GENERIC;
	if (type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO || (type >= 11 && type <= 13) || SDL_GameControllerGetVendor(gc) == 0x057e) {
		*family = PAD_FAMILY_NINTENDO;
		*style = PAD_STYLE_NINTENDO;
	} else if (type == SDL_CONTROLLER_TYPE_PS3 || type == SDL_CONTROLLER_TYPE_PS4 || type == SDL_CONTROLLER_TYPE_PS5) {
		*style = PAD_STYLE_PLAYSTATION;
	} else if (type == SDL_CONTROLLER_TYPE_XBOX360 || type == SDL_CONTROLLER_TYPE_XBOXONE) {
		*style = PAD_STYLE_XBOX;
	}
}

#ifdef __ANDROID__
static bool is_joycon(int i, int *product) {
	*product = SDL_JoystickGetDeviceProduct(i);
	return SDL_JoystickGetDeviceVendor(i) == 0x057e && (*product == 0x2006 || *product == 0x2007);
}

/* Android's Joy-Cons, given padmap_joycon's mapping where SDL has none or
 * only the one it makes up for an Android pad (named "*"): a left Joy-Con
 * has no A, B, X or Y, so SDL 2.32 makes it none (SDL_gamecontroller.c,
 * SDL_CreateMappingForAndroidController) and it was never opened, and the
 * Android driver sends an unopened pad's buttons on as keys (Minus, SDL's
 * BACK, as Escape, which asked to quit; L as nothing) and drops its stick;
 * a right one got the left stick and X and Y swapped (issue #37). One the
 * database maps (a Joy-Con read without Linux's hid-nintendo) keeps it.
 * Halves of one pad once the two have been there together, so one that
 * drops out a moment leaves the other's buttons where they were. */
static void joycon_mappings(void) {
	static int halves;
	int n = SDL_NumJoysticks(), product;
	for (int i = 0; i < n; ++i)
		if (is_joycon(i, &product)) halves |= product == 0x2006 ? 1 : 2;
	for (int i = 0; i < n; ++i) {
		if (!is_joycon(i, &product)) continue;
		char *had = SDL_GameControllerMappingForDeviceIndex(i);
		const char *name = had ? strchr(had, ',') : NULL;
		bool ours = !had || (name && (!strncmp(name, ",*,", 3) || !strncmp(name, ",Joy-Con (", 10)));
		SDL_free(had);
		if (!ours) continue;
		char guid[33], m[400];
		SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(i), guid, sizeof guid);
		snprintf(m, sizeof m, "%s,%s", guid, padmap_joycon(product, halves == 3));
		if (SDL_GameControllerAddMapping(m) < 0) say("Joy-Con mapping refused: %s", SDL_GetError());
	}
}
#endif

/* ---- a browser's own view of a pad ---- */

/* SDL 2.32 takes every browser pad as the W3C's standard gamepad (its one
 * mapping there, "Standard Gamepad"); a pad the browser does not know it
 * gives its buttons and axes as they come (mapping ""), where the D-pad is
 * an axis or two: read raw. Chrome gives a hat as one axis, over 1 at
 * rest and -1 to 1 round its eight ways (issue #112). */
#ifdef __EMSCRIPTEN__
EM_JS_DEPS(cw_pads, "$UTF8ToString");
EM_JS(int, cw_pad_standard, (const char *id), {
	const name = UTF8ToString(id);
	for (const p of (navigator.getGamepads ? navigator.getGamepads() : []))
		if (p && p.id === name) return p.mapping === 'standard' ? 1 : 0;
	return 1;
});
EM_JS(int, cw_pad_pov, (const char *id), {
	const name = UTF8ToString(id);
	for (const p of (navigator.getGamepads ? navigator.getGamepads() : [])) {
		if (!p || p.id !== name) continue;
		for (let i = 0; i < p.axes.length; ++i)
			if (p.axes[i] > 1.01) return i;
		return -1;
	}
	return -1;
});
EM_JS(double, cw_pad_axis, (const char *id, int axis), {
	const name = UTF8ToString(id);
	for (const p of (navigator.getGamepads ? navigator.getGamepads() : []))
		if (p && p.id === name) return axis < p.axes.length ? p.axes[axis] : 2;
	return 2;
});
static bool web_raw(int i) {
	const char *name = SDL_JoystickNameForIndex(i);
	return name && !cw_pad_standard(name);
}
static int web_pov(int i) {
	const char *name = SDL_JoystickNameForIndex(i);
	return name ? cw_pad_pov(name) : -1;
}
static double web_axis(int s, int axis) {
	const char *name = SDL_JoystickName(slot[s].joy);
	return name ? cw_pad_axis(name, axis) : 2;
}
#else
static bool web_raw(int i) { (void)i; return false; }
static int web_pov(int i) { (void)i; return -1; }
static double web_axis(int s, int axis) { (void)s; (void)axis; return 2; }
#endif

/* ---- opening and closing ---- */

static bool used(int s) { return slot[s].gc || slot[s].joy; }

static int slot_of(SDL_JoystickID id) {
	for (int s = 0; s < SLOTS; ++s)
		if (used(s) && slot[s].id == id) return s;
	return -1;
}

static void close_slot(int s) {
	if (s < 0) return;
	if (slot[s].gc) SDL_GameControllerClose(slot[s].gc);
	if (slot[s].joy) SDL_JoystickClose(slot[s].joy);
	memset(&slot[s], 0, sizeof slot[s]);
	if (last == s) last = -1;
}

static void open_controller(int s, int i, SDL_JoystickID id) {
	SDL_GameController *gc = SDL_GameControllerOpen(i);
	if (!gc) return;
	slot[s].gc = gc;
	slot[s].id = id;
	kind_of(gc, &slot[s].family, &slot[s].style);
	static const char *const styles[] = { "a pad", "an Xbox pad", "a PlayStation pad", "a Nintendo pad" };
	const char *name = SDL_GameControllerName(gc);
	say("controller: %s, %s", name ? name : "(no name)", styles[slot[s].style]);
}

/* A joystick SDL makes no controller of, read as it is (issue #112), if
 * it has buttons: Android's and iOS's accelerometer, also a joystick to
 * SDL, has none */
static void open_raw(int s, int i, SDL_JoystickID id) {
	static SDL_JoystickID refused[8];
	static int nrefused;
	for (int k = 0; k < nrefused; ++k)
		if (refused[k] == id) return;
	SDL_Joystick *j = SDL_JoystickOpen(i);
	if (!j) return;
	const char *name = SDL_JoystickName(j);
	int buttons = SDL_JoystickNumButtons(j);
	if (buttons < 2) {
		say("joystick: %s, %d buttons: not read", name ? name : "(no name)", buttons);
		if (nrefused < 8) refused[nrefused++] = id;
		SDL_JoystickClose(j);
		return;
	}
	slot[s].joy = j;
	slot[s].id = id;
	slot[s].family = PAD_FAMILY_RAW;
	slot[s].style = PAD_STYLE_GENERIC;
	slot[s].naxes = SDL_JoystickNumAxes(j) < PAD_RAW_AXES ? SDL_JoystickNumAxes(j) : PAD_RAW_AXES;
	slot[s].nhats = SDL_JoystickNumHats(j);
	slot[s].pov = slot[s].nhats > 0 ? -1 : web_pov(i);
	for (int a = 0; a < slot[s].naxes; ++a) {
		Sint16 v = 0;
		slot[s].rest[a] = SDL_JoystickGetAxisInitialState(j, a, &v) ? v : 0;
	}
	say("joystick: %s, read as it is: %d buttons, %d axes, %d hats%s", name ? name : "(no name)", buttons, slot[s].naxes, slot[s].nhats,
		slot[s].pov >= 0 ? ", its hat an axis" : "");
}

static void open_one(int i) {
	SDL_JoystickID id = SDL_JoystickGetDeviceInstanceID(i);
	if (slot_of(id) >= 0) return;
	int s = 0;
	while (s < SLOTS && used(s)) ++s;
	if (s == SLOTS) return;
	if (SDL_IsGameController(i) && !web_raw(i)) open_controller(s, i, id);
	else open_raw(s, i, id);
}

void pads_open(void) {
	map_init();
#ifdef __ANDROID__
	joycon_mappings();
#endif
	for (int i = 0; i < SDL_NumJoysticks(); ++i) open_one(i);
}

void pads_close(void) {
	for (int s = 0; s < SLOTS; ++s)
		if (used(s)) close_slot(s);
}

/* ---- reading ---- */

/* The fixed buttons the controls screen reads: a controller's D-pad, left
 * stick, a or start (A), b or back (B), x (clear: SELECT's bit), shoulders
 * and triggers (L and R: its tabs); a raw joystick's hat, first two axes,
 * the first button or a start, the second or a select, the third or
 * fourth, b4 to b7 (its map's directions too, by mapped_menu: its D-pad
 * may be any axis) */
static uint32_t menu_of(int family, uint64_t on) {
	typedef struct { uint8_t input; uint16_t bit; } Fixed;
	static const Fixed pad[] = {
		{ PAD_DPUP, BTN_UP }, { PAD_DPDOWN, BTN_DOWN }, { PAD_DPLEFT, BTN_LEFT }, { PAD_DPRIGHT, BTN_RIGHT },
		{ PAD_AXIS(1, false), BTN_UP }, { PAD_AXIS(1, true), BTN_DOWN }, { PAD_AXIS(0, false), BTN_LEFT }, { PAD_AXIS(0, true), BTN_RIGHT },
		{ PAD_A, BTN_A }, { PAD_START, BTN_A }, { PAD_B, BTN_B }, { PAD_BACK, BTN_B }, { PAD_X, BTN_SELECT },
		{ PAD_LEFTSHOULDER, BTN_L }, { PAD_LEFTTRIGGER, BTN_L }, { PAD_RIGHTSHOULDER, BTN_R }, { PAD_RIGHTTRIGGER, BTN_R },
	};
	static const Fixed raw[] = {
		{ PAD_RAW_HAT + 0, BTN_UP }, { PAD_RAW_HAT + 2, BTN_DOWN }, { PAD_RAW_HAT + 3, BTN_LEFT }, { PAD_RAW_HAT + 1, BTN_RIGHT },
		{ PAD_RAW_AXIS(1, false), BTN_UP }, { PAD_RAW_AXIS(1, true), BTN_DOWN }, { PAD_RAW_AXIS(0, false), BTN_LEFT }, { PAD_RAW_AXIS(0, true), BTN_RIGHT },
		{ 0, BTN_A }, { 9, BTN_A }, { 11, BTN_A }, { 1, BTN_B }, { 8, BTN_B }, { 10, BTN_B }, { 2, BTN_SELECT }, { 3, BTN_SELECT },
		{ 4, BTN_L }, { 6, BTN_L }, { 5, BTN_R }, { 7, BTN_R },
	};
	const Fixed *fixed = family == PAD_FAMILY_RAW ? raw : pad;
	size_t n = family == PAD_FAMILY_RAW ? sizeof raw / sizeof *raw : sizeof pad / sizeof *pad;
	uint32_t bits = 0;
	for (size_t i = 0; i < n; ++i)
		if (on & BIT(fixed[i].input)) bits |= fixed[i].bit;
	return bits;
}

static uint32_t mapped(int s) {
	uint32_t bits = 0;
	for (int i = 0; i < padmap_inputs(slot[s].family); ++i)
		if (slot[s].on & BIT(i)) bits |= lut[slot[s].family][i];
	return bits;
}

/* a slot's menu buttons from inputs `on` */
static uint32_t mapped_menu(int s, uint64_t on) {
	uint32_t bits = menu_of(slot[s].family, on);
	if (slot[s].family == PAD_FAMILY_RAW)
		for (int i = 0; i < PAD_RAW_INPUTS; ++i)
			if (on & BIT(i)) bits |= lut[PAD_FAMILY_RAW][i] & DIRECTIONS;
	return bits;
}

/* An input went down: held, its GBA buttons taken, and kept for "press a
 * button". */
static void pressed(int s, int input) {
	slot[s].on |= BIT(input);
	taken |= lut[slot[s].family][input];
	menu_taken |= mapped_menu(s, BIT(input));
	last = s;
	if (npress < (int)(sizeof press / sizeof *press)) press[npress++] = (PadPress){ input, slot[s].family, slot[s].style };
}

static bool button_event(const SDL_ControllerButtonEvent *b, bool down) {
	int s = slot_of(b->which);
	if (s < 0 || !slot[s].gc || b->button >= PAD_BUTTONS) return false;
	if (down) pressed(s, b->button);
	else slot[s].on &= ~BIT(b->button);
	return down;
}

/* (a controller's joystick sends these too: a raw joystick's alone) */
static bool raw_button(const SDL_JoyButtonEvent *b, bool down) {
	int s = slot_of(b->which);
	if (s < 0 || !slot[s].joy || b->button >= PAD_RAW_BUTTONS) return false;
	if (down) pressed(s, b->button);
	else slot[s].on &= ~BIT(b->button);
	return down;
}

bool pads_event(const SDL_Event *e) {
	switch (e->type) {
	case SDL_JOYDEVICEADDED:
	case SDL_CONTROLLERDEVICEADDED:
		pads_open();
		return false;
	case SDL_JOYDEVICEREMOVED:
	case SDL_CONTROLLERDEVICEREMOVED:
		close_slot(slot_of(e->type == SDL_JOYDEVICEREMOVED ? e->jdevice.which : e->cdevice.which));
		pads_open();
		return false;
	case SDL_CONTROLLERDEVICEREMAPPED: {
		int s = slot_of(e->cdevice.which);
		if (s >= 0) slot[s].on &= AXES_MASK;
		return false;
	}
	case SDL_CONTROLLERBUTTONDOWN: return button_event(&e->cbutton, true);
	case SDL_CONTROLLERBUTTONUP: return button_event(&e->cbutton, false);
	case SDL_JOYBUTTONDOWN: return raw_button(&e->jbutton, true);
	case SDL_JOYBUTTONUP: return raw_button(&e->jbutton, false);
	default: return false;
	}
}

void pads_begin(void) { npress = 0; }

static void poll_controller(int s) {
	uint64_t axes = 0;
	for (int a = 0; a < PAD_AXES; ++a) {
		int v = SDL_GameControllerGetAxis(slot[s].gc, (SDL_GameControllerAxis)a);
		/* (a trigger goes one way) */
		if (v < -PUSHED && a < 4) axes |= BIT(PAD_AXIS(a, false));
		if (v > PUSHED) axes |= BIT(PAD_AXIS(a, true));
	}
	for (int i = PAD_AXIS_FIRST; i < PAD_INPUTS; ++i)
		if ((axes & BIT(i)) && !(slot[s].on & BIT(i))) pressed(s, i);
	slot[s].on = (slot[s].on & ~AXES_MASK) | axes;
}

/* A raw joystick's hat and axes: an axis pushed away from where it rests
 * (a trigger resting at an end goes one way), the hat SDL's or, in a
 * browser, the axis that is one */
static void poll_raw(int s) {
	uint64_t now = 0;
	for (int a = 0; a < slot[s].naxes; ++a) {
		if (a == slot[s].pov) continue;
		int v = SDL_JoystickGetAxis(slot[s].joy, a), rest = slot[s].rest[a];
		if (v < -PUSHED && rest > -PUSHED) now |= BIT(PAD_RAW_AXIS(a, false));
		if (v > PUSHED && rest < PUSHED) now |= BIT(PAD_RAW_AXIS(a, true));
	}
	int hat = slot[s].nhats > 0 ? SDL_JoystickGetHat(slot[s].joy, 0) : slot[s].pov >= 0 ? padmap_pov_hat(web_axis(s, slot[s].pov)) : 0;
	for (int k = 0; k < 4; ++k)
		if (hat & (1 << k)) now |= BIT(PAD_RAW_HAT + k);
	for (int i = PAD_RAW_HAT; i < PAD_RAW_INPUTS; ++i)
		if ((now & BIT(i)) && !(slot[s].on & BIT(i))) pressed(s, i);
	slot[s].on = (slot[s].on & ~RAW_POLLED) | now;
}

void pads_poll(void) {
	for (int s = 0; s < SLOTS; ++s) {
		if (slot[s].gc) poll_controller(s);
		else if (slot[s].joy) poll_raw(s);
	}
}

uint32_t pads_held(void) {
	uint32_t bits = 0;
	for (int s = 0; s < SLOTS; ++s)
		if (used(s)) bits |= mapped(s);
	return bits;
}

uint32_t pads_taken(void) {
	uint32_t t = taken;
	taken = 0;
	return t;
}

uint32_t pads_menu_held(void) {
	uint32_t bits = 0;
	for (int s = 0; s < SLOTS; ++s)
		if (used(s)) bits |= mapped_menu(s, slot[s].on);
	return bits;
}

uint32_t pads_menu_taken(void) {
	uint32_t t = menu_taken;
	menu_taken = 0;
	return t;
}

/* (a raw joystick's: SELECT and START as its map has them, or the fixed
 * pairs b8 and b9, b10 and b11) */
bool pads_quit_held(void) {
	for (int s = 0; s < SLOTS; ++s) {
		uint64_t on = slot[s].on;
		if (slot[s].gc && (on & BIT(PAD_BACK)) && (on & BIT(PAD_START))) return true;
		if (slot[s].joy && (((mapped(s) & (BTN_SELECT | BTN_START)) == (BTN_SELECT | BTN_START)) ||
			((on & BIT(8)) && (on & BIT(9))) || ((on & BIT(10)) && (on & BIT(11))))) return true;
	}
	return false;
}

bool pads_present(void) {
	for (int s = 0; s < SLOTS; ++s)
		if (used(s)) return true;
	return false;
}

bool pads_any_held(void) {
	for (int s = 0; s < SLOTS; ++s)
		if (used(s) && slot[s].on) return true;
	return false;
}

bool pads_last(int *family, int *style, const char **name) {
	int s = last >= 0 && used(last) ? last : -1;
	for (int k = 0; s < 0 && k < SLOTS; ++k)
		if (used(k)) s = k;
	if (s < 0) return false;
	*family = slot[s].family;
	*style = slot[s].style;
	*name = slot[s].gc ? SDL_GameControllerName(slot[s].gc) : SDL_JoystickName(slot[s].joy);
	if (!*name) *name = "Controller";
	return true;
}

int pads_presses(PadPress *out, int most) {
	int n = npress < most ? npress : most;
	memcpy(out, press, (size_t)n * sizeof *out);
	return n;
}

/* ---- the map ---- */

const PadMap *pads_map(void) {
	map_init();
	return &map;
}

void pads_set_map(const PadMap *m) {
	map = *m;
	map_ready = true;
	relut();
}

void pads_load(const char *p) {
	map_init();
	snprintf(path, sizeof path, "%s", p);
	static char text[8192];
	FILE *f = fopen(p, "r");
	if (!f) {
		pads_save();
		return;
	}
	size_t n = fread(text, 1, sizeof text - 1, f);
	fclose(f);
	text[n] = 0;
	char errors[1024];
	if (padmap_parse(text, &map, errors, sizeof errors)) fprintf(stderr, "%s:\n%s", p, errors);
	relut();
}

bool pads_save(void) {
	if (!path[0]) return false;
	static char text[8192];
	int n = padmap_format(&map, text, sizeof text);
	FILE *f = fopen(path, "w");
	if (!f) return false;
	bool ok = fwrite(text, 1, (size_t)n, f) == (size_t)n;
	ok = fclose(f) == 0 && ok;
	platform_persist();
	return ok;
}

/* ---- tests: a virtual controller ---- */

#if SDL_VERSION_ATLEAST(2, 24, 0)
static SDL_Joystick *vjoy;
static bool vjoy_raw;

/* (raw: a joystick SDL makes no controller of, as an 8BitDo Micro in its
 * D-input mode is on SDL 2.32: sixteen buttons, two axes, a hat) */
bool pads_virtual(const char *kind) {
	static const struct { const char *kind, *name; Uint16 vendor, product; } kinds[] = {
		{ "xbox", "Xbox 360 Controller", 0x045e, 0x028e },
		{ "playstation", "PS4 Controller", 0x054c, 0x09cc },
		{ "nintendo", "Nintendo Switch Pro Controller", 0x057e, 0x2009 },
		{ "generic", "Virtual Controller", 0, 0 },
		{ "raw", "8BitDo Micro gamepad", 0x2dc8, 0x9020 },
	};
	size_t k = 0;
	while (k < sizeof kinds / sizeof *kinds && strcmp(kinds[k].kind, kind)) ++k;
	if (k == sizeof kinds / sizeof *kinds) return false;
	vjoy_raw = !strcmp(kind, "raw");
	SDL_VirtualJoystickDesc d;
	SDL_zero(d);
	d.version = SDL_VIRTUAL_JOYSTICK_DESC_VERSION;
	d.type = vjoy_raw ? SDL_JOYSTICK_TYPE_UNKNOWN : SDL_JOYSTICK_TYPE_GAMECONTROLLER;
	d.naxes = vjoy_raw ? 2 : PAD_AXES;
	d.nbuttons = vjoy_raw ? 16 : PAD_BUTTONS;
	d.nhats = vjoy_raw ? 1 : 0;
	d.vendor_id = kinds[k].vendor;
	d.product_id = kinds[k].product;
	d.name = kinds[k].name;
	/* (headless, no window has the focus SDL wants before it passes on a
	 * pad's press) */
	SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
	int index = SDL_JoystickAttachVirtualEx(&d);
	vjoy = index >= 0 ? SDL_JoystickOpen(index) : NULL;
	pads_virtual_hold(0);
	pads_open();
	return vjoy != NULL;
}

void pads_virtual_hold(uint64_t inputs) {
	if (!vjoy) return;
	if (vjoy_raw) {
		Uint8 hat = 0;
		for (int b = 0; b < 16; ++b) SDL_JoystickSetVirtualButton(vjoy, b, (Uint8)(inputs >> b & 1));
		for (int k = 0; k < 4; ++k)
			if (inputs & BIT(PAD_RAW_HAT + k)) hat |= (Uint8)(1 << k);
		SDL_JoystickSetVirtualHat(vjoy, 0, hat);
		for (int a = 0; a < 2; ++a)
			SDL_JoystickSetVirtualAxis(vjoy, a, (inputs & BIT(PAD_RAW_AXIS(a, true))) ? 32767 : (inputs & BIT(PAD_RAW_AXIS(a, false))) ? -32768 : 0);
		return;
	}
	for (int b = 0; b < PAD_BUTTONS; ++b) SDL_JoystickSetVirtualButton(vjoy, b, (Uint8)(inputs >> b & 1));
	for (int a = 0; a < PAD_AXES; ++a) {
		/* (a trigger rests at the bottom of its axis) */
		Sint16 v = (inputs & BIT(PAD_AXIS(a, true))) ? 32767 : (inputs & BIT(PAD_AXIS(a, false))) ? -32768 : a >= 4 ? -32768 : 0;
		SDL_JoystickSetVirtualAxis(vjoy, a, v);
	}
}
#else
bool pads_virtual(const char *kind) {
	(void)kind;
	return false;
}

void pads_virtual_hold(uint64_t inputs) { (void)inputs; }
#endif
