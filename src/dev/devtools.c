/* The dev menu: switches for long test runs (MegaMan cannot die, enemies
 * fall to one hit, no random battles, the game fast-forwarded) and jumps
 * (the next layer, the next guardian, a chosen area), drawn over the game,
 * which holds still while it is open. */
#include "devtools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6.h"
#include "director.h"
#include "emu.h"
#include "flags.h"
#include "gamecall.h"
#include "gfx.h"
#include "guardians.h"
#include "mapslot.h"
#include "net.h"
#include "platform.h"
#include "run.h"
#include "text.h"

DevFlags dev = { false, false, false, 1 };
char devtools_shot[512];

enum { I_GOD, I_ONEHIT, I_QUIET, I_SPEED, I_WIN, I_HEAL, I_ZENNY, I_NEXT, I_GUARDIAN, I_AREA, I_COUNT };

static struct {
	bool open;
	int cursor;
	int area;           /* the area chosen for the next layers: -1 the run's own */
	char toast[64];
	int toast_t;
} M = { false, 0, -1, "", 0 };

void devtools_parse(const char *spec) {
	char buf[256];
	snprintf(buf, sizeof buf, "%s", spec);
	for (char *t = strtok(buf, ","); t; t = strtok(NULL, ",")) {
		if (!strcmp(t, "god")) dev.god = true;
		else if (!strcmp(t, "onehit")) dev.onehit = true;
		else if (!strcmp(t, "quiet")) dev.quiet = true;
		else if (!strncmp(t, "speed=", 6)) dev.speed = atoi(t + 6);
	}
	if (dev.speed < 1) dev.speed = 1;
	if (dev.speed > 8) dev.speed = 8;
}

bool devtools_open(void) { return M.open; }

static void toast(const char *s) {
	snprintf(M.toast, sizeof M.toast, "%s", s);
	M.toast_t = 120;
}

/* A text script run through the game (zenny): written into the layer's
 * space and run at once. */
static void run_text(const uint8_t *b, int n) {
	static TextArchive t;
	ta_begin(&t);
	ta_script(&t);
	ta_bytes(&t, b, n);
	ta_end(&t);
	uint32_t at = ta_commit(&t);
	if (at) game_call(BN6_CHAT_RUN_SCRIPT, at, 0);
}

/* The battle's objects on one side (0 MegaMan, 1 the enemies): HP to `hp`
 * (-1: to its max), only lowered unless `raise`. */
static void battle_hp(int side, int hp, bool raise) {
	for (uint32_t i = 0; i < BN6_T1_COUNT; ++i) {
		uint32_t o = BN6_T1_OBJECTS + i * BN6_T1_SIZE;
		if (!(emu_read8(o) & 1) || emu_read8(o + 0x16) != side) continue;
		int cur = emu_read16(o + 0x24), max = emu_read16(o + 0x26);
		int want = hp < 0 ? max : hp;
		if (cur <= 0 || want == cur || (!raise && want > cur) || (raise && want < cur)) continue;
		uint8_t v[2] = { (uint8_t)want, (uint8_t)(want >> 8) };
		emu_write(o + 0x24, v, 2);
	}
}

static void act(int item, int dir) {
	switch (item) {
	case I_GOD: dev.god = !dev.god; break;
	case I_ONEHIT: dev.onehit = !dev.onehit; break;
	case I_QUIET:
		dev.quiet = !dev.quiet;
		if (!dev.quiet) flag_clear(BN6_FLAG_NO_ENCOUNTERS);
		break;
	case I_SPEED:
		dev.speed = dir < 0 ? (dev.speed > 1 ? dev.speed / 2 : 8) : (dev.speed < 8 ? dev.speed * 2 : 1);
		break;
	case I_WIN:
		M.open = false;
		battle_hp(1, 0, false);
		toast("Enemies deleted");
		break;
	case I_HEAL: {
		M.open = false;
		uint16_t max = emu_read16(BN6_NAVI_STATS + 0x42);
		uint8_t v[2] = { (uint8_t)max, (uint8_t)(max >> 8) };
		emu_write(BN6_NAVI_STATS + 0x40, v, 2);
		battle_hp(0, -1, true);
		toast("HP full");
		break;
	}
	case I_ZENNY: {
		M.open = false;
		static const uint8_t give[] = { 0xEF, 0x0E, 0x10, 0x27, 0x00, 0x00, 0xFF, 0xFF, 0xFF };   /* ts_check_give_zenny 10000 */
		run_text(give, sizeof give);
		toast("+10000 zenny");
		break;
	}
	case I_NEXT:
		M.open = false;
		toast(director_dev_next_layer() ? "Next layer" : "Only on the net");
		break;
	case I_GUARDIAN:
		M.open = false;
		toast(director_dev_guardian() ? "To the guardian" : "Only on the net");
		break;
	case I_AREA:
		M.area += dir < 0 ? -1 : 1;
		if (M.area < -1) M.area = BIOME_COUNT - 1;
		if (M.area >= BIOME_COUNT) M.area = -1;
		director_debug_biome = M.area;
		break;
	}
}

uint32_t devtools_keys(uint32_t keys) {
	if (M.toast_t > 0) --M.toast_t;
	if (!M.open) {
		/* hold SELECT, press R */
		if (btn_held(BTN_SELECT) && btn_pressed(BTN_R)) { M.open = true; return 0; }
		return keys;
	}
	if (btn_pressed(BTN_B) || (btn_held(BTN_SELECT) && btn_pressed(BTN_R))) M.open = false;
	else if (btn_repeat(BTN_UP)) M.cursor = (M.cursor + I_COUNT - 1) % I_COUNT;
	else if (btn_repeat(BTN_DOWN)) M.cursor = (M.cursor + 1) % I_COUNT;
	else if (btn_pressed(BTN_LEFT)) act(M.cursor, -1);
	else if (btn_pressed(BTN_RIGHT) || btn_pressed(BTN_A)) act(M.cursor, 1);
	return 0;
}

void devtools_update(void) {
	if (dev.quiet) flag_set(BN6_FLAG_NO_ENCOUNTERS);
	if (dev.god) {
		uint16_t max = emu_read16(BN6_NAVI_STATS + 0x42);
		if (max && emu_read16(BN6_NAVI_STATS + 0x40) < max) {
			uint8_t v[2] = { (uint8_t)max, (uint8_t)(max >> 8) };
			emu_write(BN6_NAVI_STATS + 0x40, v, 2);
		}
		battle_hp(0, -1, true);
	}
	if (dev.onehit) battle_hp(1, 1, false);
}

static void line(int x, int y, bool sel, const char *label, const char *value) {
	SDL_Color c = sel ? rgba(255, 232, 96, 255) : WHITE;
	text_draw(x, y, sel ? ">" : " ", c, TEXT_LEFT);
	text_draw(x + 8, y, label, c, TEXT_LEFT);
	if (value) text_draw(x + 150, y, value, c, TEXT_RIGHT);
}

void devtools_draw(void) {
	int x0 = P.core_x, y0 = P.core_y;
	if (M.toast_t > 0) {
		fill_rect(x0 + 4, y0 + CORE_H - 20, text_width(M.toast) + 8, 14, rgba(0, 0, 0, 180));
		text_draw(x0 + 8, y0 + CORE_H - 19, M.toast, WHITE, TEXT_LEFT);
	}
	if (!M.open) return;
	fill_rect(x0 + 40, y0 + 4, 164, 152, rgba(8, 12, 32, 225));
	text_draw(x0 + 122, y0 + 7, "DEV", rgba(120, 200, 248, 255), TEXT_CENTER);
	char speed[8], area[40];
	snprintf(speed, sizeof speed, "%dx", dev.speed);
	snprintf(area, sizeof area, "%s", M.area < 0 ? "the run's" : guardian_area_name(M.area));
	const char *on = "on", *off = "off";
	struct { const char *label, *value; } items[I_COUNT] = {
		{ "Can't die", dev.god ? on : off }, { "One-hit enemies", dev.onehit ? on : off },
		{ "Random battles", dev.quiet ? off : on }, { "Speed", speed }, { "Win this battle", NULL },
		{ "Heal", NULL }, { "+10000 zenny", NULL }, { "Next layer", NULL }, { "Next guardian", NULL },
		{ "Area", area },
	};
	for (int i = 0; i < I_COUNT; ++i) line(x0 + 46, y0 + 20 + i * 12, i == M.cursor, items[i].label, items[i].value);
	char info[64];
	snprintf(info, sizeof info, "Depth %d  seed %u", run.depth, run.seed);
	text_draw(x0 + 122, y0 + 142, info, rgba(160, 160, 190, 255), TEXT_CENTER);
}
