/* Dev steps on a layer: --talk's chats at given frames, the battle step,
 * gifts, MegaMan placed or moved on, and where the test autopilot heads. */
#include "director_dev.h"

#include <stdio.h>
#include <string.h>

#include "boss.h"
#include "briefing_words.h"
#include "cinema.h"
#include "debug.h"
#include "devtools.h"
#include "director.h"
#include "director_folder.h"
#include "director_guest.h"
#include "director_layer.h"
#include "director_state.h"
#include "emu.h"
#include "encounter.h"
#include "gamecall.h"
#include "netmap.h"
#include "rumor_lines.h"
#include "save.h"
#include "talk.h"
#include "trader.h"

const char *director_dev_talks;

bool director_dev_next_layer(void) {
	if (!director_on_map()) return false;
	/* (from a trip back, the run's own next: docs/HOME.md) */
	if (run.home_depth) run.depth = run.home_depth - 1;
	run.home_depth = 0;
	run.depth++;
	run.side_kind = LAYER_NORMAL;
	return director_start_layer();
}

void director_dev_reveal(void) {
	if (D.active && !D.town) memset(D.seen, 1, sizeof D.seen);
}

void director_dev_place(int x, int y, int face) {
	/* (a test's step: MegaMan put down at world (x, y), facing 0-7) */
	if (!director_on_map()) return;
	emu_write32(BN6_PLAYER_X, (uint32_t)x << 16);
	emu_write32(BN6_PLAYER_Y, (uint32_t)y << 16);
	emu_write32(BN6_PLAYER_NEXT_X, (uint32_t)x << 16);
	emu_write32(BN6_PLAYER_NEXT_Y, (uint32_t)y << 16);
	if (face >= 0 && face < 8) { emu_write8(BN6_PLAYER_FACING, (uint8_t)face); emu_write8(BN6_PLAYER_ANIM, (uint8_t)face); }
}

bool director_dev_warp_cell(int x, int y) {
	if (!director_on_map()) return false;
	int wx, wy;
	netmap_world(x, y, &wx, &wy);
	emu_warp(D.group, D.number, wx, wy, 4);
	return true;
}

bool director_dev_guardian(void) {
	if (!director_on_map()) return false;
	/* the next guardian's layer, arriving in the room before its arena */
	if (run.home_depth) run.depth = run.home_depth - 1;
	run.home_depth = 0;
	do run.depth++; while (!is_boss_depth(run.depth));
	run.side_kind = LAYER_NORMAL;
	if (!director_start_layer()) return false;
	if (layer.ante >= 0) {
		int wx, wy;
		netmap_world(layer.rooms[layer.ante].ax, layer.rooms[layer.ante].ay, &wx, &wy);
		emu_warp(D.group, D.number, wx, wy, 4);
	}
	return true;
}

bool director_goal_panel(int *x, int *y, bool *talk) {
	if (!D.active) return false;
	/* the guardian's arena and its Guardian Data, then the exit pad */
	if (boss_goal(x, y, talk)) return true;
	*talk = false;
	for (int i = 0; i < layer.nobj; ++i) {
		int t = layer.obj[i].type;
		if (t == OBJ_EXIT || t == OBJ_RETURN) {
			*x = (int)layer.obj[i].x;
			*y = (int)layer.obj[i].y;
			return true;
		}
	}
	return false;
}

/* A dev talk's gift, no chat: fifty BugFrags (a BugFrag Trader's trade
 * wants ten) or one (a DarkChip's last, BN6's own), an Unlocker, three
 * RushFood, a WWW-ID (the set pieces' keys), 10000 zenny (a cube's toll), a
 * RegUP3 (+3 MB of Reg memory). */

static bool dev_gift(const char *name) {
	if (!strcmp(name, "bugfrags")) game_call(BN6_GIVE_BUGFRAGS, 50, 0);
	else if (!strcmp(name, "bugfrag")) game_call(BN6_GIVE_BUGFRAGS, 1, 0);
	else if (!strcmp(name, "zenny")) devtools_zenny();
	else if (!strcmp(name, "keys")) game_call(BN6_GIVE_ITEM | 1u, SUB_UNLOCKER, 1);
	else if (!strcmp(name, "rushfood")) game_call(BN6_GIVE_ITEM | 1u, ITEM_RUSH_FOOD, 3);
	else if (!strcmp(name, "wwwid")) game_call(BN6_GIVE_ITEM | 1u, ITEM_WWW_ID, 1);
	else if (!strcmp(name, "regup")) game_call(BN6_GIVE_ITEM | 1u, SCRIPTS_REG_UP1 + 2, 1);
	/* (a battle in the older net's own engine at once, on a layer whose
	 * battles are its: docs/MULTIROM.md, Guest battles) */
	else if (!strcmp(name, "guest") && encounter_guest) guest_begin();
	else return false;
	return true;
}

/* A dev talk said in the engine's own words: L's (status), the layer's
 * rumor (rumor, rumor_lines.c); false for another. */
static bool dev_say(const char *name) {
	if (!strcmp(name, "status")) talk_start(status_words(), FACE_MEGAMAN);
	else if (!strcmp(name, "rumor")) { if (rumors_line()) talk_start(rumors_line(), FACE_NAVI); }
	else return false;
	return true;
}

/* --talk NAME:FRAME,...: from the layer's frame FRAME, once no chat is
 * open, the chat of its first NAME (npc shop heal programs gift challenge
 * undernet gate; intro defeat reward for its guardian; status for L;
 * fragment, what MegaMan says as its ScrtData is picked up), for captures. */
static void dev_talks(void) {
	static unsigned done;
	static uint32_t layer_of;
	/* (each layer's own: the talks of a later layer too) */
	if (D.frame <= 1 || run.layer_seed != layer_of) { done = 0; layer_of = run.layer_seed; }
	if (!director_dev_talks || talk_busy() || emu_read8(BN6_CHATBOX)) return;
	static const struct { const char *name; int type; } kinds[] = {
		{ "npc", OBJ_NPC }, { "shop", OBJ_SHOP }, { "heal", OBJ_HEAL }, { "programs", OBJ_PROGRAMS },
		{ "gift", OBJ_GIFT }, { "challenge", OBJ_CHALLENGE }, { "duel", OBJ_DUEL }, { "official", OBJ_OFFICIAL }, { "undernet", OBJ_UNDERNET }, { "gate", OBJ_SECRET_GATE },
		{ "navigate", OBJ_NAVI_GATE }, { "vault", OBJ_VAULT }, { "trader", OBJ_TRADER }, { "bugtrader", OBJ_BUGTRADER },
	};
	char buf[256];
	snprintf(buf, sizeof buf, "%s", director_dev_talks);
	int n = 0;
	for (char *t = strtok(buf, ","); t; t = strtok(NULL, ","), ++n) {
		char name[32];
		int frame = 0;
		if (sscanf(t, "%31[^:]:%d", name, &frame) != 2 || D.frame < frame || (done & (1u << n))) continue;
		done |= 1u << n;
		int script = -1;
		for (unsigned i = 0; i < sizeof kinds / sizeof *kinds; ++i)
			if (!strcmp(name, kinds[i].name)) script = D.objs.script_of[kinds[i].type];
		if (!strcmp(name, "intro")) script = D.objs.guardian.intro;
		else if (!strcmp(name, "defeat")) script = D.objs.guardian.defeat;
		else if (!strcmp(name, "reward")) script = D.objs.guardian.reward;
		else if (!strcmp(name, "fragment")) script = D.objs.fragment_found;
		else if (!strcmp(name, "spin")) script = D.objs.spin_found;
		else if (!strcmp(name, "dark")) script = D.objs.dark_flame;
		else if (dev_say(name) || dev_gift(name)) return;
		/* (a trader talks from the game's own trader archive) */
		if (!strcmp(name, "trader") || !strcmp(name, "bugtrader")) {
			int k = D.objs.trader_kind;
			if (k < 0 || (k == TRADER_BUGFRAG) != !strcmp(name, "bugtrader")) { printf("--talk: no %s on this layer\n", name); continue; }
			game_call(BN6_CHAT_RUN_SCRIPT, BN6_TRADER_TEXT, (uint32_t)k);
			return;
		}
		if (script < 0) { printf("--talk: no %s on this layer\n", name); continue; }
		game_call(BN6_CHAT_RUN_SCRIPT, D.objs.archive, (uint32_t)script);
		return;   /* (one a frame: the chat box opens on the next) */
	}
}

/* The battle step (a capture's "0:battle", play.py's battle): the layer's
 * next random battle at once, at the first moment MegaMan stands free on
 * its map (no chat, talk, scene or warp, no guardian's staging, challenge
 * or forced battle under way): on a layer whose battles are an older
 * net's, its battle on the guest core (docs/MULTIROM.md, Guest battles),
 * else BN6's own, forced past its roll as the guardians' are. */
void director_dev_battle(void) {
	if (D.active && !D.town) D.battle_due = true;
	else printf("battle: only on a layer\n");
}

static void dev_battle(void) {
	if (!D.battle_due || !on_map() || guest_active() || emu_read8(BN6_CHATBOX) || talk_busy() || cinema_busy() || D.warping ||
		emu_read8(BN6_WARP_PENDING) || !boss_idle() || D.challenge || emu_battle_forcing())
		return;
	D.battle_due = false;
	if (emu_debug_on()) fprintf(stderr, "battle: the step's, %s\n", encounter_guest ? "the guest's" : "BN6's");
	if (encounter_guest) { guest_begin(); return; }
	Encounter e = D.next;
	set_encounter(&e, true);
}

/* the dev talks and the battle step, a frame on the layer */
void dev_steps(void) {
	dev_talks();
	dev_battle();
}
