/* The dev menu: switches for long test runs (MegaMan cannot die, enemies
 * fall to one hit, no random battles, the game fast-forwarded) and jumps
 * (the next layer, the next guardian, a chosen area), drawn over the game,
 * which holds still while it is open. */
#include "devtools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "analytics.h"
#include "bn5.h"
#include "bn6.h"
#include "darkchips.h"
#include "director.h"
#include "emu.h"
#include "flags.h"
#include "gamecall.h"
#include "gfx.h"
#include "guardians.h"
#include "mapslot.h"
#include "net.h"
#include "platform.h"
#include "rivals.h"
#include "run.h"
#include "save.h"
#include "souls.h"
#include "soulunite.h"
#include "text.h"

DevFlags dev = { .speed = 1, .duels = -1, .arena_lag = -1, .arena_roll = -1, .arena_window = -1 };
char devtools_shot[512];

enum { I_GOD, I_ONEHIT, I_QUIET, I_SPEED, I_WIN, I_HEAL, I_ZENNY, I_NEXT, I_GUARDIAN, I_AREA, I_COUNT };

static struct {
	bool open;
	int cursor;
	int area;           /* the area chosen for the next layers: -1 the run's own */
	char toast[64];
	int toast_t;
} M = { false, 0, -1, "", 0 };

/* A switch that sets a flag, or one that takes a number: true where `t`
 * is one */
static bool dev_switch(const char *t) {
	static const struct { const char *name; bool *on; } flags[] = {
		{ "god", &dev.god }, { "onehit", &dev.onehit }, { "quiet", &dev.quiet }, { "fragile", &dev.fragile }, { "powers", &dev.powers },
		{ "gem", &dev.gem }, { "veteran", &dev.veteran }, { "mapall", &dev.mapall }, { "worried", &dev.worried },
		{ "xcontrol", &dev.xcontrol }, { "nofamilies", &dev.nofamilies }, { "soulproxy", &dev.soulproxy }, { "chipai", &dev.chipai }, { "visitors", &dev.visitors },
		{ "xnavicontrol", &dev.xnavicontrol }, { "nonavis", &dev.nonavis }, { "arena", &dev.arena }, { "arenabusy", &dev.arena_busy },
		{ "rollbench", &dev.rollbench }, { "arenaboth", &dev.arena_both }, { "arenaother", &dev.arena_other }, { "quitend", &dev.quit_end },
	};
	static const struct { const char *name; int *value; } numbers[] = {
		{ "speed=", &dev.speed }, { "duels=", &dev.duels }, { "pack=", &dev.pack }, { "programs=", &dev.programs }, { "clock=", &dev.clock },
		{ "job=", &dev.job }, { "jobstate=", &dev.job_state }, { "zenny=", &dev.zenny },
		{ "side=", &dev.side }, { "bass=", &dev.bass }, { "beast=", &dev.beast }, { "proxybreak=", &dev.proxybreak },
		{ "unitebreak=", &dev.unitebreak }, { "soulbreak=", &dev.soulbreak },
		{ "arenadelay=", &dev.arena_delay }, { "arenalag=", &dev.arena_lag }, { "arenahp=", &dev.arena_hp }, { "arenacarry=", &dev.arena_carry }, { "arenadesync=", &dev.arena_desync },
		{ "arenaroll=", &dev.arena_roll }, { "arena2v2=", &dev.arena_2v2 }, { "teamhp=", &dev.team_hp }, { "arenawindow=", &dev.arena_window },
		{ "arenafreeze=", &dev.arena_freeze }, { "arenaseed=", &dev.arena_seed }, { "arenaagain=", &dev.arena_again },
		{ "arenaseedoff=", &dev.arena_seed_off }, { "teamseat=", &dev.team_seat }, { "teampick=", &dev.team_pick }, { "teambuster=", &dev.team_buster },
		{ "teamslow=", &dev.team_slow }, { "teamform=", &dev.team_form }, { "teamcust=", &dev.team_cust }, { "teamcross=", &dev.team_cross },
		{ "teampa=", &dev.team_pa }, { "teambeast=", &dev.team_beast },
		{ "arenapeer=", &dev.arena_peer }, { "chipaibreak=", &dev.chipai_break }, { "partnerbreak=", &dev.partner_break },
	};
	for (size_t i = 0; i < sizeof flags / sizeof *flags; ++i)
		if (!strcmp(t, flags[i].name)) { *flags[i].on = true; return true; }
	for (size_t i = 0; i < sizeof numbers / sizeof *numbers; ++i)
		if (!strncmp(t, numbers[i].name, strlen(numbers[i].name))) { *numbers[i].value = atoi(t + strlen(numbers[i].name)); return true; }
	return false;
}

void devtools_parse(const char *spec) {
	char buf[256];
	snprintf(buf, sizeof buf, "%s", spec);
	for (char *t = strtok(buf, ","); t; t = strtok(NULL, ",")) {
		/* (statistics=ask|on|off, statsurl=URL: analytics.h) */
		if (dev_switch(t) || analytics_dev(t)) continue;
		if (!strncmp(t, "hp=", 3)) { dev.hp = atoi(t + 3); dev.hp_now = strchr(t, '/') ? atoi(strchr(t, '/') + 1) : 0; }
		else if (!strncmp(t, "pieces=", 7)) layer_pieces_forced = (unsigned)strtoul(t + 7, NULL, 0);
		else if (!strncmp(t, "darkchips=", 10)) dark_dev_mask = (uint16_t)strtoul(t + 10, NULL, 0);
		else if (!strncmp(t, "folder=", 7)) {
			char *end;
			dev.folder = (int)strtol(t + 7, &end, 0);
			dev.folder_n = *end == '/' ? atoi(end + 1) : 30;
		}
		else if (!strncmp(t, "souls=", 6)) souls_dev_mask = (uint8_t)strtoul(t + 6, NULL, 0);
		/* (kept past this call: copies of their own) */
		else if (!strncmp(t, "online=", 7)) { static char url[200]; snprintf(url, sizeof url, "%s", t + 7); dev.online = url; }
		else if (!strncmp(t, "onlinename=", 11)) { static char name[17]; snprintf(name, sizeof name, "%s", t + 11); dev.online_name = name; }
	}
	if (dev.speed < 1) dev.speed = 1;
	if (dev.speed > 8) dev.speed = 8;
}

void devtools_veteran(void) {
	if (dev.duels >= 0) { profile.duel_won = (uint16_t)dev.duels; profile_save(); }
	/* (the super bosses' records made N: their forms and words, docs/
	 * BOSSES.md, Super bosses) */
	for (int k = 0; k < 2; ++k) {
		int navi = k ? SUPER_CYBEAST : SUPER_BASS, wins = k ? dev.beast : dev.bass;
		while (rival(navi)->megaman_won < wins) {
			rival_met(navi);
			rival_result(navi, RIVAL_MEGAMAN_WON);
		}
	}
	if (!dev.veteran) return;
	/* (guardian, MegaMan's wins, the guardian's, how the last went) */
	static const struct { int navi, won, lost, last; } rec[] = {
		{ 1, 2, 1, RIVAL_MEGAMAN_WON }, { 3, 1, 0, RIVAL_MEGAMAN_WON }, { 4, 0, 1, RIVAL_NAVI_WON }, { 6, 2, 0, RIVAL_MEGAMAN_WON },
		{ 12, 3, 0, RIVAL_MEGAMAN_WON }, { 13, 1, 1, RIVAL_MEGAMAN_WON }, { 14, 1, 0, RIVAL_MEGAMAN_WON },
	};
	for (unsigned i = 0; i < sizeof rec / sizeof *rec; ++i) {
		if (rival(rec[i].navi)->met) continue;
		int n = rec[i].won + rec[i].lost;
		for (int k = 0; k < n; ++k) {
			rival_met(rec[i].navi);
			/* (the last result last) */
			bool won = k < n - 1 ? (rec[i].last == RIVAL_MEGAMAN_WON ? k < rec[i].won - 1 : k < rec[i].won) : rec[i].last == RIVAL_MEGAMAN_WON;
			rival_result(rec[i].navi, won ? RIVAL_MEGAMAN_WON : RIVAL_NAVI_WON);
		}
	}
	if (!profile.spins) profile.spins = 0x05;   /* (white and pink) */
	if (!profile.duel_won && dev.duels < 0) { profile.duel_won = 3; profile.duel_lost = 1; }   /* (the rival beaten: every official gate open) */
	if (profile.best_depth < 9) profile.best_depth = 9;
	if (profile.runs < 6) profile.runs = 6;
	profile_save();
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

void devtools_zenny(void) {
	static const uint8_t give[] = { 0xEF, 0x0E, 0x10, 0x27, 0x00, 0x00, 0xFF, 0xFF, 0xFF };   /* ts_check_give_zenny 10000 */
	run_text(give, sizeof give);
}

/* The battle's objects on one side (0 MegaMan, 1 the enemies): HP to `hp`
 * (-1: to its max), only lowered unless `raise` */
static void battle_hp(int side, int hp, bool raise) {
	for (uint32_t i = 0; i < BN6_T1_COUNT; ++i) {
		uint32_t o = BN6_T1_OBJECTS + i * BN6_T1_SIZE;
		if (!(emu_read8(o) & 1) || emu_read8(o + BN6_T1_ALLIANCE) != side) continue;
		int cur = emu_read16(o + BN6_T1_HP), max = emu_read16(o + BN6_T1_MAX_HP);
		int want = hp < 0 ? max : hp;
		if (cur <= 0 || want == cur || (!raise && want > cur) || (raise && want < cur)) continue;
		uint8_t v[2] = { (uint8_t)want, (uint8_t)(want >> 8) };
		emu_write(o + BN6_T1_HP, v, 2);
	}
}

/* (worried: MegaMan's mood as BN5 keeps it in BN6's battles held worried
 * once the battle's opening set it, so BN5's DarkChips come into his hand,
 * for captures; soulmood.h) */
static void keep_worried(void) {
	uint32_t mood = BN6_SOULS + BN6_SOUL_MOOD;
	if (!soulunite_ready() || !emu_read8(BN6_SOULS + BN6_SOUL_OPENED) || emu_read8(BN6_GAMESTATE) != BN6_SUB_BATTLE) return;
	if (emu_read8(mood) > BN5_MOOD_WORRIED / 2) emu_write8(mood, BN5_MOOD_WORRIED / 2);
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
		uint16_t max = emu_read16(BN6_NAVI_MAX_HP);
		uint8_t v[2] = { (uint8_t)max, (uint8_t)(max >> 8) };
		emu_write(BN6_NAVI_HP, v, 2);
		battle_hp(0, -1, true);
		toast("HP full");
		break;
	}
	case I_ZENNY:
		M.open = false;
		devtools_zenny();
		toast("+10000 zenny");
		break;
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
	/* (Trade Central's screens opened for a capture: the game held under them) */
	if (trade_preview_on()) return trade_preview_keys(keys);
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
	if (dev.mapall) director_dev_reveal();
	if (dev.god) {
		uint16_t max = emu_read16(BN6_NAVI_MAX_HP);
		if (max && emu_read16(BN6_NAVI_HP) < max) {
			uint8_t v[2] = { (uint8_t)max, (uint8_t)(max >> 8) };
			emu_write(BN6_NAVI_HP, v, 2);
		}
		battle_hp(0, -1, true);
	}
	/* (hp=N/H: his HP H through the run's first 600 frames, past the
	 * heals of its start, then left to the battles) */
	static int now_frames;
	bool set_now = dev.hp_now > 0 && dev.hp_now <= dev.hp && now_frames++ < 600 && emu_read16(BN6_NAVI_HP) != dev.hp_now;
	if (dev.hp > 0 && dev.hp <= 9999 && (emu_read16(BN6_NAVI_MAX_HP) != dev.hp || set_now)) {
		int now = set_now ? dev.hp_now : dev.hp;
		uint8_t v[4] = { (uint8_t)now, (uint8_t)(now >> 8), (uint8_t)dev.hp, (uint8_t)(dev.hp >> 8) };
		emu_write(BN6_NAVI_HP, v, sizeof v);
	}
	if (dev.onehit) battle_hp(1, 1, false);
	if (dev.powers) {
		flag_set(BN6_FLAG_BEAST_OUT);
		for (int f = BN6_FLAG_HEAT_CROSS; f <= BN6_FLAG_CHARGE_CROSS; ++f) flag_set(f);
	}
	if (dev.fragile) battle_hp(0, 1, false);
	if (dev.worried) keep_worried();
}

static void line(int x, int y, bool sel, const char *label, const char *value) {
	SDL_Color c = sel ? rgba(255, 232, 96, 255) : WHITE;
	text_draw(x, y, sel ? ">" : " ", c, TEXT_LEFT);
	text_draw(x + 8, y, label, c, TEXT_LEFT);
	if (value) text_draw(x + 150, y, value, c, TEXT_RIGHT);
}

void devtools_draw(void) {
	int x0 = P.core_x, y0 = P.core_y;
	trade_preview_draw();
	reconnect_preview_draw();
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
