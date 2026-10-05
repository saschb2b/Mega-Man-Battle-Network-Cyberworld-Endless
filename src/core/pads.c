#include "pads.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "buttons.h"
#include "platform.h"

/* (padmap.c numbers a pad's inputs as SDL numbers its buttons and axes;
 * SDL 2.0.14, the oldest a handheld's firmware brings, has these 21) */
_Static_assert((int)PAD_BUTTONS == (int)SDL_CONTROLLER_BUTTON_MAX, "padmap.h's buttons are SDL's");
_Static_assert((int)PAD_AXES == (int)SDL_CONTROLLER_AXIS_MAX, "padmap.h's axes are SDL's");
_Static_assert((int)PAD_BACK == (int)SDL_CONTROLLER_BUTTON_BACK && (int)PAD_DPRIGHT == (int)SDL_CONTROLLER_BUTTON_DPAD_RIGHT &&
	(int)PAD_MISC1 == (int)SDL_CONTROLLER_BUTTON_MISC1 && PAD_LEFTTRIGGER == PAD_AXIS((int)SDL_CONTROLLER_AXIS_TRIGGERLEFT, true),
	"padmap.h's names are SDL's numbers");

#define SLOTS 8
#define PUSHED 16000   /* a stick or trigger past this (of 32767) is pushed */
#define BIT(input) ((uint64_t)1 << (input))
#define AXES_MASK ((BIT(PAD_AXES * 2) - 1) << PAD_AXIS_FIRST)

/* An open controller: its family and style, and the inputs held (its
 * buttons from SDL's events, its sticks and triggers read each frame). */
static struct {
	SDL_GameController *gc;
	SDL_JoystickID id;
	int family, style;
	uint64_t on;
} slot[SLOTS];
static PadMap map;
static bool map_ready;
static uint32_t lut[PAD_FAMILIES][PAD_INPUTS];   /* (the map: an input's GBA buttons) */
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
		for (int i = 0; i < PAD_INPUTS; ++i) lut[f][i] = padmap_bits(&map, f, i);
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

/* ---- opening and closing ---- */

static int slot_of(SDL_JoystickID id) {
	for (int s = 0; s < SLOTS; ++s)
		if (slot[s].gc && slot[s].id == id) return s;
	return -1;
}

static void close_slot(int s) {
	if (s < 0) return;
	SDL_GameControllerClose(slot[s].gc);
	memset(&slot[s], 0, sizeof slot[s]);
	if (last == s) last = -1;
}

static void open_one(int i) {
	SDL_JoystickID id = SDL_JoystickGetDeviceInstanceID(i);
	if (!SDL_IsGameController(i) || slot_of(id) >= 0) return;
	int s = 0;
	while (s < SLOTS && slot[s].gc) ++s;
	if (s == SLOTS) return;
	SDL_GameController *gc = SDL_GameControllerOpen(i);
	if (!gc) return;
	slot[s].gc = gc;
	slot[s].id = id;
	kind_of(gc, &slot[s].family, &slot[s].style);
	static const char *const styles[] = { "a pad", "an Xbox pad", "a PlayStation pad", "a Nintendo pad" };
	const char *name = SDL_GameControllerName(gc);
	say("controller: %s, %s", name ? name : "(no name)", styles[slot[s].style]);
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
		if (slot[s].gc) close_slot(s);
}

/* ---- reading ---- */

/* The fixed buttons the controls screen reads */
static uint32_t menu_of(uint64_t on) {
	static const struct { uint8_t input; uint16_t bit; } fixed[] = {
		{ PAD_DPUP, BTN_UP }, { PAD_DPDOWN, BTN_DOWN }, { PAD_DPLEFT, BTN_LEFT }, { PAD_DPRIGHT, BTN_RIGHT },
		{ PAD_AXIS(1, false), BTN_UP }, { PAD_AXIS(1, true), BTN_DOWN }, { PAD_AXIS(0, false), BTN_LEFT }, { PAD_AXIS(0, true), BTN_RIGHT },
		{ PAD_A, BTN_A }, { PAD_START, BTN_A }, { PAD_B, BTN_B }, { PAD_BACK, BTN_B },
	};
	uint32_t bits = 0;
	for (size_t i = 0; i < sizeof fixed / sizeof *fixed; ++i)
		if (on & BIT(fixed[i].input)) bits |= fixed[i].bit;
	return bits;
}

static uint32_t mapped(int s) {
	uint32_t bits = 0;
	for (int i = 0; i < PAD_INPUTS; ++i)
		if (slot[s].on & BIT(i)) bits |= lut[slot[s].family][i];
	return bits;
}

/* An input went down: held, its GBA buttons taken, and kept for "press a
 * button". */
static void pressed(int s, int input) {
	slot[s].on |= BIT(input);
	taken |= lut[slot[s].family][input];
	menu_taken |= menu_of(BIT(input));
	last = s;
	if (npress < (int)(sizeof press / sizeof *press)) press[npress++] = (PadPress){ input, slot[s].family, slot[s].style };
}

static bool button_event(const SDL_ControllerButtonEvent *b, bool down) {
	int s = slot_of(b->which);
	if (s < 0 || b->button >= PAD_BUTTONS) return false;
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
	default: return false;
	}
}

void pads_begin(void) { npress = 0; }

void pads_poll(void) {
	for (int s = 0; s < SLOTS; ++s) {
		if (!slot[s].gc) continue;
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
}

uint32_t pads_held(void) {
	uint32_t bits = 0;
	for (int s = 0; s < SLOTS; ++s)
		if (slot[s].gc) bits |= mapped(s);
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
		if (slot[s].gc) bits |= menu_of(slot[s].on);
	return bits;
}

uint32_t pads_menu_taken(void) {
	uint32_t t = menu_taken;
	menu_taken = 0;
	return t;
}

bool pads_quit_held(void) {
	for (int s = 0; s < SLOTS; ++s)
		if (slot[s].gc && (slot[s].on & BIT(PAD_BACK)) && (slot[s].on & BIT(PAD_START))) return true;
	return false;
}

bool pads_present(void) {
	for (int s = 0; s < SLOTS; ++s)
		if (slot[s].gc) return true;
	return false;
}

bool pads_last(int *family, int *style, const char **name) {
	int s = last >= 0 && slot[last].gc ? last : -1;
	for (int k = 0; s < 0 && k < SLOTS; ++k)
		if (slot[k].gc) s = k;
	if (s < 0) return false;
	*family = slot[s].family;
	*style = slot[s].style;
	*name = SDL_GameControllerName(slot[s].gc);
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

bool pads_virtual(const char *kind) {
	static const struct { const char *kind, *name; Uint16 vendor, product; } kinds[] = {
		{ "xbox", "Xbox 360 Controller", 0x045e, 0x028e },
		{ "playstation", "PS4 Controller", 0x054c, 0x09cc },
		{ "nintendo", "Nintendo Switch Pro Controller", 0x057e, 0x2009 },
		{ "generic", "Virtual Controller", 0, 0 },
	};
	size_t k = 0;
	while (k < sizeof kinds / sizeof *kinds && strcmp(kinds[k].kind, kind)) ++k;
	if (k == sizeof kinds / sizeof *kinds) return false;
	SDL_VirtualJoystickDesc d;
	SDL_zero(d);
	d.version = SDL_VIRTUAL_JOYSTICK_DESC_VERSION;
	d.type = SDL_JOYSTICK_TYPE_GAMECONTROLLER;
	d.naxes = PAD_AXES;
	d.nbuttons = PAD_BUTTONS;
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
