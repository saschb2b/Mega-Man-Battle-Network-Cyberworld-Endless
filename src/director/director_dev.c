/* Dev steps on a layer: --talk's chats at given frames, the battle step,
 * gifts, MegaMan placed or moved on, and where the test autopilot heads. */
#include "director_dev.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "boss.h"
#include "briefing_words.h"
#include "cinema.h"
#include "data.h"
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

bool director_dev_signature(void) {
	if (!director_on_map() || layer.sig_room < 0) return false;
	const Room *m = &layer.rooms[layer.sig_room];
	int bx = m->ax, by = m->ay, best = 1 << 30;
	for (int y = m->y; y < m->y + m->h; ++y)
		for (int x = m->x; x < m->x + m->w; ++x) {
			bool taken = layer.cell[y][x] != C_PATH || layer.level[y][x];
			for (int i = 0; i < layer.nobj && !taken; ++i) taken = abs((int)layer.obj[i].x - x) + abs((int)layer.obj[i].y - y) <= 1;
			int d = abs(x - m->ax) + abs(y - m->ay);
			if (!taken && d < best) { best = d; bx = x; by = y; }
		}
	int wx, wy;
	netmap_world(bx, by, &wx, &wy);
	director_dev_place(wx, wy, -1);
	return true;
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
	/* (the three a golden gate wants: docs/BOSSES.md, Super bosses) */
	else if (!strcmp(name, "scrtdata")) game_call(BN6_GIVE_ITEM | 1u, SCRIPTS_SECRET_DATA, 3);
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

/* The foes step's battle, due at the first free moment as the battle
 * step's (director.h) */
static Encounter dev_foes;
static bool foes_due;

/* a foe of the step's: its kind, family and version as its id's triple
 * gives them (a family moved into the engine's as it was installed) */
static bool foe_of(unsigned id, int col, int row, Foe *f) {
	const uint8_t *t = id ? enemy_triple((int)id) : NULL;
	if (!t || col < 1 || col > 6 || row < 1 || row > 3) return false;
	*f = (Foe){ t[1] ? FOE_NAVI : FOE_VIRUS, t[2], t[0], col - 1, row - 1, (int)id };
	return true;
}

void director_dev_due(const Encounter *e) {
	dev_foes = *e;
	foes_due = true;
}

bool director_dev_foes(const char *spec) {
	Encounter e = D.next;
	e.nfoes = e.nobj = 0;
	e.field = e.player = e.song = 0;
	e.boss = e.held = e.map_backdrop = e.gem_own = false;
	e.xrecord = 0;
	e.visitor = 0;
	char buf[128];
	snprintf(buf, sizeof buf, "%s", spec);
	for (char *t = strtok(buf, " /,"); t; t = strtok(NULL, " /,")) {
		unsigned id, field;
		int c, r;
		if (sscanf(t, "mm@%1d%1d", &c, &r) == 2 && c >= 1 && c <= 3 && r >= 1 && r <= 3) e.player = r << 4 | c;
		else if (sscanf(t, "field@%x", &field) == 1 && field < BN6_FIELD_COUNT) e.field = (int)field;
		else if (sscanf(t, "%x@%1d%1d", &id, &c, &r) == 3 && e.nfoes < MAX_FOES && foe_of(id, c, r, &e.foes[e.nfoes])) ++e.nfoes;
		else printf("foes: %s is no enemy at a panel (ID@CR, hex, column 1-6, row 1-3), nor mm@CR or field@HH\n", t);
	}
	if (!e.nfoes) return false;
	director_dev_due(&e);
	return true;
}

/* a status the step gives, as a hit gives it: the bit the hit leaves in its
 * collision data for the engine to act on, and the timer it runs for */
typedef struct { const char *name; uint32_t struck; int timer; } DevStatus;
static const DevStatus dev_statuses[] = {
	{ "paralysis", BN6_STRUCK_PARALYSIS, BN6_COLL_PARALYSIS }, { "freeze", BN6_STRUCK_FREEZE, BN6_COLL_FREEZE },
	{ "bubble", BN6_STRUCK_BUBBLE, BN6_COLL_BUBBLE }, { "drag", BN6_STRUCK_DRAG, -1 }, { "slide", BN6_STRUCK_SLIDE, -1 },
	{ "confuse", BN6_STRUCK_CONFUSE, BN6_COLL_CONFUSE }, { "blind", BN6_STRUCK_BLIND, BN6_COLL_BLIND },
};

int director_dev_status(const char *spec) {
	char kind[16] = "";
	int frames = 60;
	unsigned id = 0;
	const DevStatus *s = NULL;
	if (sscanf(spec, "%15s %d %x", kind, &frames, &id) >= 1)
		for (size_t i = 0; i < sizeof dev_statuses / sizeof *dev_statuses; ++i)
			if (!strcmp(kind, dev_statuses[i].name)) s = &dev_statuses[i];
	if (!s || frames < 1 || frames > 0xFFFF || !D.in_battle) {
		printf("status: %s is none (paralysis, freeze, bubble, drag, slide, confuse or blind, then frames and an enemy id), or no battle is on\n", spec);
		return 0;
	}
	int n = 0;
	for (uint32_t k = 0; k < BN6_T1_COUNT; ++k) {
		uint32_t o = BN6_T1_OBJECTS + k * BN6_T1_SIZE, coll = emu_read32(o + BN6_T1_COLLISION);
		if (!(emu_read8(o + BN6_T1_IN_PLAY) & 1) || !emu_read8(o + BN6_T1_ALLIANCE) || (id && emu_read16(o + BN6_T1_NAME_ID) != id) ||
			coll < BN6_EWRAM || coll >= BN6_EWRAM_END)
			continue;
		emu_write32(coll + BN6_COLL_STRUCK, emu_read32(coll + BN6_COLL_STRUCK) | s->struck);
		uint8_t t[2] = { (uint8_t)frames, (uint8_t)(frames >> 8) };
		if (s->timer >= 0) emu_write(coll + (uint32_t)s->timer, t, sizeof t);
		++n;
	}
	printf("status: %s on %d enemies at battle frame %u\n", spec, n, (unsigned)emu_read32(BN6_BATTLE_TIMER));
	return n;
}

int director_dev_chip(const char *spec) {
	int id = 0, n = 1;
	char code[4] = "";
	int got = sscanf(spec, "%i %3s %d", &id, code, &n);
	int c = code[0] == '*' ? CHIP_CODE_STAR : code[0] >= 'A' && code[0] <= 'Z' ? code[0] - 'A' : -1;
	uint32_t data = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CHIPS);
	if (got < 2 || id <= 0 || id >= PACK_CHIPS || c < 0 || n < 1 || n > BN6_FOLDER_ENTRIES || data < BN6_EWRAM || data >= BN6_EWRAM_END) {
		printf("chip: %s is none (an id 1-511, a code A-Z or *, then how many in the folder), or the folder is not there\n", spec);
		return 0;
	}
	uint16_t e = (uint16_t)(id | c << 9);
	uint8_t b[2] = { (uint8_t)e, (uint8_t)(e >> 8) };
	for (uint32_t i = 0; i < (uint32_t)n; ++i) emu_write(data + 2 * i, b, sizeof b);
	own_folder_chips();
	/* (the pack's counts hold the folder's copies too, outside the folder
	 * editor, which takes them out as it opens: n for the folder, one more
	 * the pack's own; the chips written over stay counted, as spares) */
	uint32_t out[2] = { 0, 0 };
	bool given = game_call_ret(BN6_GIVE_CHIPS, (uint32_t)id, (uint32_t)c, (uint32_t)n + 1, out);
	ChipInfo ci;
	chip_info(id, &ci);
	uint32_t at = pack_entry(id);
	printf("chip: %s (%d) %c in the folder's first %d, one more in the pack%s (its counts %d %d %d %d)\n", ci.name, id,
		c == CHIP_CODE_STAR ? '*' : 'A' + c, n, given ? "" : ": not given", at ? emu_read8(at) : -1, at ? emu_read8(at + 1) : -1,
		at ? emu_read8(at + 2) : -1, at ? emu_read8(at + 3) : -1);
	return n;
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
	if ((!D.battle_due && !foes_due) || !on_map() || guest_active() || emu_read8(BN6_CHATBOX) || talk_busy() || cinema_busy() || D.warping ||
		emu_read8(BN6_WARP_PENDING) || !boss_idle() || D.challenge || emu_battle_forcing())
		return;
	if (foes_due) {
		foes_due = false;
		if (emu_debug_on()) fprintf(stderr, "foes: the step's battle, %d foes%s\n", dev_foes.nfoes, dev_foes.xrecord ? " (an older net's record)" : "");
		set_encounter(&dev_foes, true);
		return;
	}
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
