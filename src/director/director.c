/* The director: the run's structure around the game.
 *
 * A layer is generated (net_gen), built into its area's map (netmap), given
 * its exit pad and Mystery Data (mapslot, npc), and entered through the
 * game's warp. The game then runs everything MegaMan does; the director only
 * watches his position to take him to the next layer, and keeps the next
 * random battle's enemies in step with the depth. */
#include "director.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "foes.h"
#include "bn6.h"
#include "autopilot.h"
#include "boss.h"
#include "cinema.h"
#include "emu.h"
#include "encounter.h"
#include "debug.h"
#include "flags.h"
#include "game.h"
#include "gamecall.h"
#include "gfx.h"
#include "guardians.h"
#include "layer_objs.h"
#include "mapslot.h"
#include "loot.h"
#include "net.h"
#include "netmap.h"
#include "rom.h"
#include "platform.h"
#include "run.h"
#include "runlog.h"
#include "save.h"
#include "save_blob.h"
#include "scripts.h"
#include "talk.h"
#include "text.h"
#include "town.h"

int director_debug_biome = -1;
const char *director_dev_talks;

#define REROLL_FRAMES 300  /* the next battle's enemies are re-rolled this often */

static struct {
	bool active;
	int group, number;
	int frame;
	bool checkpoint;       /* save once MegaMan has arrived */
	bool gameover;         /* the game's GAME OVER is playing */
	int start_x, start_y;
	LayerObjs objs;
	unsigned chosen;       /* choices already acted on (bit per choice) */
	bool challenge;        /* a challenge battle was started */
	bool in_battle;        /* a battle is on */
	int foes;              /* viruses in the battle the game will start next */
	Encounter next;        /* that battle */
	int battles;           /* random battles fought on this layer */
	int astray;            /* frames MegaMan has spent on another map */
	bool warping;          /* the exit pad's warp is under way */
	bool area_card;        /* show the area's title card once MegaMan is in */
	int arrived;           /* frames on the layer's map since the warp ended */
	int act_viruses;       /* viruses deleted when the act began */
	int act_frames;        /* frames spent in the act */
	const char *act_guardian;  /* the guardian beaten on the way out */
	bool town;             /* Lan is in the town; the first layer waits for his jack-in */
	bool town_seen;        /* ... and has got there */
	int town_frames;       /* frames on the town's map */
	bool intro_said;       /* Lan and MegaMan have spoken there */
	char beat[640];        /* what they say on arriving, once the card has gone */
	bool secret_call;      /* Chaud's call after the Secret Area's guardian is due */
	bool act_resumed;      /* the act was continued from a checkpoint: no clear stats */
	bool l_held, r_held, a_held;   /* L, R and A were down last frame */
	int l_kept;                    /* frames an L pressed while busy is kept */
	int walk_to, walk_t;           /* the NPC slot MegaMan walks up to after an A short of it, frames left */
	bool dir_held;         /* a direction is held this frame */
	bool map_shown;        /* SELECT is held on a layer's map: the map shows */
	uint8_t seen[MAP_H][MAP_W];   /* panels MegaMan has come near on this layer */
	bool arrow_pending;    /* the way-on arrow lasts until a little after L's words close */
	int free_x, free_y;    /* MegaMan's last place clear of every NPC */
	int wedged;            /* frames he has pushed, unmoving, against an NPC he stands inside */
	int last_x, last_y;    /* where he stood the frame before */
	bool port_told;        /* MegaMan has said where the town's port is and how to jack in */
	bool layer_told;       /* ... where they are on this layer (as LAYER_TOLD_FLAG) */
	int lost_to;           /* the guardian MegaMan was deleted by, 0 none */
	bool nest_cleared;     /* the Nest's guardian fell; the profile counts it at the checkpoint */
} D;

#define AREA_CARD_AT 45   /* frames on the map after arriving */
#define WALK_UP      45   /* frames the walk up to a navi out of reach may take */

/* An act begins (or a side layer): its title card, as Hades names each
 * region on entering it. */
static void begin_area(bool new_act) {
	D.area_card = true;
	D.arrived = 0;
	if (!new_act) return;
	D.act_viruses = run.viruses_deleted;
	D.act_frames = 0;
	D.act_resumed = false;
}

/* The label the game keeps at the map's bottom right: its own name for the
 * original map ("AquarumComp3") gave way to where the run is. The game's
 * label routine is pointed at an archive of ours whose every name is it. */
#define LABEL_AT    (EMU_FREE + 0x152000)
#define LABEL_SIZE  0x1000

/* The archive holds more than names: the PET prints its HP, zenny and
 * BugFrags by scripts 0xF0-0xF2, and others are placeholders. A copy of the
 * player's own archive is written with only its twelve-character names
 * pointed at the label. */
static int label_archive(const char *label, uint8_t *out, int max) {
	uint32_t src = rom_u32(BN6_MAP_NAMES_PTR - 0x08000000u) - 0x08000000u;
	if (src + 2 > ROM_SIZE) return 0;
	int n = rom_u16(src) / 2, end = 0;
	if (n <= 0 || n > 512) return 0;
	for (int k = 0; k < n; ++k) {
		int o = rom_u16(src + 2 * (uint32_t)k), j = o;
		while (src + (uint32_t)j < ROM_SIZE && R.data[src + (uint32_t)j] != 0xE6) ++j;
		if (j + 1 > end) end = j + 1;
	}
	if (end + 13 > max) return 0;
	memcpy(out, R.data + src, (size_t)end);
	char padded[20];
	snprintf(padded, sizeof padded, "%12.12s", label);
	ta_encode(padded, out + end, 12);
	out[end + 12] = 0xE6;
	for (int k = 0; k < n; ++k) {
		int o = rom_u16(src + 2 * (uint32_t)k), len = 0;
		bool name = true;
		while (R.data[src + (uint32_t)(o + len)] != 0xE6 && len < 16) { name &= R.data[src + (uint32_t)(o + len)] < 0xE7; ++len; }
		if (!name || len != 12) continue;
		out[2 * k] = (uint8_t)end;
		out[2 * k + 1] = (uint8_t)(end >> 8);
	}
	return end + 13;
}

static void map_label(void) {
	static char last[16];
	char name[16];
	if (D.town) snprintf(name, sizeof name, "%s", town_info()->group == 0x00 ? "ACDC Town" : "Central Town");
	else if (run.side_kind == LAYER_UNDERNET) snprintf(name, sizeof name, "Undernet");
	else if (run.side_kind == LAYER_SECRET) snprintf(name, sizeof name, "Secret Area");
	else if (run.biome == BIOME_NEST) snprintf(name, sizeof name, "Cybeast Nest");
	else snprintf(name, sizeof name, "Layer %d", run.depth);
	if (!strcmp(name, last) && emu_read32(BN6_MAP_NAMES_PTR) == LABEL_AT) return;
	snprintf(last, sizeof last, "%s", name);
	/* (right-aligned with spaces: the game pads its own with underscores,
	 * which show) */
	static uint8_t a[LABEL_SIZE];
	int len = label_archive(name, a, sizeof a);
	if (!len) return;
	emu_write(LABEL_AT, a, (size_t)len);
	emu_write32(BN6_MAP_NAMES_PTR, LABEL_AT);
	emu_write32(BN6_PET_MAP_NAMES_PTR, LABEL_AT);   /* (the PET's PLACE too) */
}

/* The net's version: the Nest rebuilds it, one stronger, each time its
 * guardian falls (1 for the first cycle). */
static int net_version(void) { return (run.depth - 1) / CYCLE_LAYERS + 1; }

/* What MegaMan and Lan (and Dad) say on arriving somewhere new: the first
 * layer, a new cycle, the Undernet, the Graveyard, the Nest, the side
 * layers. Empty for the rest. */
static void arrival_words(void) {
	const char *area = guardian_area_in_text(run.biome, LAYER_NORMAL);
	bool first_of_act = run.side_kind == LAYER_NORMAL && layer_in_act(run.depth) == 0;
	D.beat[0] = 0;
	if (run.side_kind == LAYER_UNDERNET)
		snprintf(D.beat, sizeof D.beat, "@M A copy of the Undernet...|@M The viruses in here are no joke, Lan.|@L Stay sharp. The exit pad leads back to the main path.");
	else if (run.side_kind == LAYER_SECRET)
		snprintf(D.beat, sizeof D.beat, "@M The gate opened, Lan... This must be the Secret Area.|@M Something strong is waiting in here. I can feel it.");
	else if (run.depth == 1)
		snprintf(D.beat, sizeof D.beat, "@M Lan, it looks just like %s... But it's all copied data!|@L Dad was right. Let's find the exit pad and head down!", area);
	else if (first_of_act && (run.depth - 1) % CYCLE_LAYERS == 0)
		snprintf(D.beat, sizeof D.beat, "@D Lan! The Endless Net just rebuilt itself, all of it!|@D The same areas, but stronger data. It's Net V%d now!|@M Then we keep going, Lan!", net_version());
	else if (run.biome == BIOME_NEST)
		snprintf(D.beat, sizeof D.beat, "@M Lan... This is it. The Nest. Something down here is copying everything.|@B Grrrr...|@L Hang on, MegaMan! Whatever it is, we'll find it!");
	else if (first_of_act && run.biome == BIOME_UNDERNET)
		snprintf(D.beat, sizeof D.beat, "@M Even the Undernet got copied... Stay sharp, Lan.");
	else if (first_of_act && run.biome == BIOME_GRAVEYARD)
		snprintf(D.beat, sizeof D.beat, "@M So much deleted data... Lan, I think the bottom is close.");
	else if (first_of_act && run.depth > 1)
		/* a new act: where they are now, and whose copy waits at its end */
		snprintf(D.beat, sizeof D.beat, "@M We're through to %s, Lan!|@L %s's copy guards this one. Let's go!",
			area, guardian(run.boss_order[run.biome])->name);
}

/* What MegaMan says when L is pressed: where they are, what is ahead. */
/* Which way the exit pad lies from MegaMan, as the screen shows it (the
 * d-pad's UP moves +X -Y, RIGHT +X +Y: a world step (dx, dy) goes
 * dx + dy across and (dy - dx) / 2 down), and how far. */
static int way_dir;   /* the index of the last way_to: 0 right, then clockwise */

static const char *way_to(int tx, int ty, int *far) {
	static const char *const ways[8] = {
		"to the right", "down and to the right", "straight down", "down and to the left",
		"to the left", "up and to the left", "straight up", "up and to the right",
	};
	int dx = tx - ((int)emu_read32(BN6_PLAYER + 0x1C) >> 16);
	int dy = ty - ((int)emu_read32(BN6_PLAYER + 0x20) >> 16);
	double sx = dx + dy, sy = (dy - dx) / 2.0;
	int panels = (abs(dx) + abs(dy)) / 32;
	*far = panels < 5 ? 0 : panels < 14 ? 1 : 2;
	double a = atan2(sy, sx);   /* (screen y grows downwards) */
	int k = (int)lround(a / (3.14159265358979 / 4));
	way_dir = (k % 8 + 8) % 8;
	return ways[way_dir];
}

/* The way on along the floor, not as the crow flies: the direction to a
 * point a few panels along the shortest walk from MegaMan to (tx, ty),
 * and how far that walk is. NULL when either is off the grid. */
static const char *route_to(int tx, int ty, int *far) {
	static int16_t prev[MAP_H][MAP_W];
	static int16_t qx[MAP_W * MAP_H], qy[MAP_W * MAP_H];
	int px = (int)emu_read32(BN6_PLAYER + 0x1C) >> 16, py = (int)emu_read32(BN6_PLAYER + 0x20) >> 16;
	int sx, sy, ex, ey;
	if (!netmap_panel(px, py, &sx, &sy) || !netmap_panel(tx, ty, &ex, &ey)) return NULL;
	if (sx < 0 || sy < 0 || sx >= MAP_W || sy >= MAP_H || ex < 0 || ey < 0 || ex >= MAP_W || ey >= MAP_H) return NULL;
	if (layer.cell[sy][sx] != C_PATH || layer.cell[ey][ex] != C_PATH) return NULL;
	for (int y = 0; y < MAP_H; ++y) for (int x = 0; x < MAP_W; ++x) prev[y][x] = -1;
	int h = 0, t = 0;
	qx[t] = (int16_t)sx; qy[t++] = (int16_t)sy;
	prev[sy][sx] = (int16_t)(sy * MAP_W + sx);
	while (h < t && prev[ey][ex] < 0) {
		int x = qx[h], y = qy[h++];
		static const int d[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
		for (int k = 0; k < 4; ++k) {
			int nx = x + d[k][0], ny = y + d[k][1];
			if (nx < 0 || ny < 0 || nx >= MAP_W || ny >= MAP_H || prev[ny][nx] >= 0 || layer.cell[ny][nx] != C_PATH) continue;
			prev[ny][nx] = (int16_t)(y * MAP_W + x);
			qx[t] = (int16_t)nx; qy[t++] = (int16_t)ny;
		}
	}
	if (prev[ey][ex] < 0) return NULL;
	/* the walk, backwards from the target; aim at the panel 3 along it */
	int len = 0, cx = ex, cy = ey;
	static int16_t path[MAP_W * MAP_H];
	while (!(cx == sx && cy == sy)) {
		path[len++] = (int16_t)(cy * MAP_W + cx);
		int p = prev[cy][cx];
		cx = p % MAP_W; cy = p / MAP_W;
	}
	*far = len < 5 ? 0 : len < 14 ? 1 : 2;
	/* aim along the walk's first leg while it runs straight (on a walkway
	 * that is one of the screen's diagonals: a flat arrow between two
	 * forking walkways said neither), else at the farthest of the next four
	 * panels he can walk to in a straight line over the floor (three
	 * along, as the crow flies, cut corners over drops) */
	int aim = len ? path[len - 1] : sy * MAP_W + sx;
	int leg = 0;
	if (len) {
		int dx = path[len - 1] % MAP_W - sx, dy = path[len - 1] / MAP_W - sy;
		while (leg < 4 && leg < len && path[len - 1 - leg] % MAP_W == sx + dx * (leg + 1) && path[len - 1 - leg] / MAP_W == sy + dy * (leg + 1)) ++leg;
	}
	if (leg >= 2) aim = path[len - leg];
	else for (int k = 4; k >= 2; --k) {
		if (len < k) continue;
		int ax = path[len - k] % MAP_W, ay = path[len - k] / MAP_W;
		bool clear = true;
		for (int t = 1; t < 16 * k && clear; ++t) {
			double f = t / (16.0 * k);
			int qx = (int)lround(sx + (ax - sx) * f), qy = (int)lround(sy + (ay - sy) * f);
			clear = qx >= 0 && qy >= 0 && qx < MAP_W && qy < MAP_H && layer.cell[qy][qx] == C_PATH;
		}
		if (clear) { aim = path[len - k]; break; }
	}
	int wx, wy, dummy;
	netmap_world(aim % MAP_W, aim / MAP_W, &wx, &wy);
	return way_to(wx, wy, &dummy);
}

/* The way on as the arrow shows it: along the floor to the exit or the
 * guardian (the port in the town); way_dir holds it. */
static void goal_way(void) {
	int far;
	if (D.town) { way_to(town_info()->port_x, town_info()->port_y, &far); return; }
	int gx = D.objs.exit_x, gy = D.objs.exit_y;
	if (D.objs.guardian.navi && !boss_beaten()) { gx = D.objs.guardian.x; gy = D.objs.guardian.y; }
	if (!route_to(gx, gy, &far)) way_to(gx, gy, &far);
}

static const char *status_words(void) {
	static char buf[800];
	int k = 0;
	#define ADD(...) (k += snprintf(buf + k, k < (int)sizeof buf ? sizeof buf - (size_t)k : 0, __VA_ARGS__))
	if (D.town) {
		int far;
		const char *way = way_to(town_info()->port_x, town_info()->port_y, &far);
		const char *statue = town_info()->group == 0x00 ? "squirrel" : "bird";
		/* all of it the first time, then only the way (a box each) */
		if (!D.port_told) ADD("@M The port's %s, by the %s statue!|@M Stand next to the statue and press R to jack me in!", way, statue);
		else ADD("@M The port's %s, Lan!", way);
		D.port_told = true;
		return buf;
	}
	/* where they are and what guards it the first time on a layer, then
	 * only the way on */
	D.layer_told |= flag_get(LAYER_TOLD_FLAG);
	if (!D.layer_told) {
		const char *area = guardian_area_in_text(run.biome, run.side_kind);
		ADD("@M Layer %d, Lan: %s.", run.depth, area);
		if (D.objs.guardian.navi && !boss_beaten()) {
			ADD(" %s waits at its end!|", guardian(D.objs.guardian.navi)->name);
			const char *tip = guardian_tip(D.objs.guardian.navi);
			if (tip) ADD("@M %s|", tip);
		}
		else if (!D.objs.guardian.navi && run.side_kind == LAYER_NORMAL) ADD(" %s guards the end of it.|", guardian(run.boss_order[run.biome])->name);
		else ADD("|");
		if (run.fragments == 1) ADD("@M We're carrying one ScrtData.|");
		else if (run.fragments > 1) ADD("@M We're carrying %d ScrtData.|", run.fragments);
	}
	/* the services here: all of them the first time, then only the heal
	 * while he is hurt (it heals every time; the map's key names the rest,
	 * and a later L is a box or two, not the briefing again) */
	bool shop = false, heal = false, programs = false, told = D.layer_told;
	for (int i = 0; i < layer.nobj; ++i) {
		shop |= layer.obj[i].type == OBJ_SHOP;
		heal |= layer.obj[i].type == OBJ_HEAL;
		programs |= layer.obj[i].type == OBJ_PROGRAMS;
	}
	bool hurt = emu_read16(BN6_NAVI_STATS + 0x40) < emu_read16(BN6_NAVI_STATS + 0x42);
	if (!told) {
		if (shop && heal) ADD("@M I can sense a Net Dealer and a Recovery Mr. Prog on this layer!|");
		else if (shop) ADD("@M I can sense a Net Dealer on this layer!|");
		else if (heal) ADD("@M I can sense a Recovery Mr. Prog on this layer!|");
		if (programs) ADD("@M There's a NaviCust program shop here too.|");
		/* (the map's tip on the run's first layers: every first L repeated it) */
		if (run.depth <= 2) ADD("@M Hold SELECT to see the map of where we've been.|");
		D.layer_told = true;
		flag_set(LAYER_TOLD_FLAG);
	} else if (heal && hurt) {
		/* (and which way: it was on no map yet, and never found) */
		for (int i = 0; i < layer.nobj; ++i) {
			if (layer.obj[i].type != OBJ_HEAL) continue;
			int wx, wy, hf;
			netmap_world((int)layer.obj[i].x, (int)layer.obj[i].y, &wx, &wy);
			/* (where it lies: the way's first leg pointed off from it) */
			const char *hw = way_to(wx, wy, &hf);
			static const char *const near_far[3] = { "close by", "a ways off", "far off" };
			ADD("@M The Recovery Mr. Prog can patch us up. It's %s, %s.|", hw, near_far[hf]);
			break;
		}
	}
	/* the way on, as MegaMan senses it: along the floor where he can (the
	 * arrow's way); where the walk sets off well away from where the goal
	 * lies, where it lies, which holds still as the walk winds */
	int far;
	bool to_guardian = D.objs.guardian.navi && !boss_beaten();
	int gx = D.objs.exit_x, gy = D.objs.exit_y;
	if (to_guardian) { gx = D.objs.guardian.x; gy = D.objs.guardian.y; }
	const char *lies = way_to(gx, gy, &far);
	int lies_dir = way_dir;
	const char *way = route_to(gx, gy, &far);
	if (!way) way = way_to(gx, gy, &far);
	int apart = abs(way_dir - lies_dir);
	if (apart > 4) apart = 8 - apart;
	static const char *const how_far[3] = { "It's close!", "It's a ways off.", "It's a long way yet." };
	if (D.objs.guardian.navi && !boss_done() && boss_beaten()) ADD("@M Let's take its Guardian Data, Lan!");
	else if (apart >= 2 && told) ADD("@M The %s %s. Follow the arrow!", to_guardian ? "guardian waits" : "exit lies", lies);
	else if (apart >= 2)
		ADD("@M The %s %s.|@M %s The way winds, so follow the arrow!", to_guardian ? "guardian waits" : "exit lies", lies, how_far[far]);
	else ADD("@M The way on goes %s. %s", way, how_far[far]);
	#undef ADD
	return buf;
}

static void area_card(void) {
	char act[32];
	int biome = run.biome, act_no = ((run.depth - 1) % CYCLE_LAYERS) / 3 + 1;
	if (run.side_kind == LAYER_UNDERNET) snprintf(act, sizeof act, "Through a dark warp");
	else if (run.side_kind == LAYER_SECRET) snprintf(act, sizeof act, "Beyond the sealed gate");
	else if (biome == BIOME_NEST && net_version() > 1) snprintf(act, sizeof act, "The bottom of Net V%d", net_version());
	else if (biome == BIOME_NEST) snprintf(act, sizeof act, "The bottom of the net");
	else if (net_version() > 1) snprintf(act, sizeof act, "Net V%d - Act %d", net_version(), act_no);
	else snprintf(act, sizeof act, "Act %d", act_no);
	/* the guardian ahead, named from the start, so the folder can be set
	 * for it (as Slay the Spire shows each act's boss) */
	char ahead[48] = "";
	if (run.side_kind == LAYER_NORMAL || (run.side_kind == LAYER_SECRET && layer.boss_layer))
		snprintf(ahead, sizeof ahead, "Guardian: %s", guardian(run.boss_order[biome])->name);
	cinema_card(act, guardian_area_name(biome), guardian_area_motto(biome), ahead[0] ? ahead : NULL, rgba(120, 200, 248, 255), 200);
}

/* Leaving an area past its beaten guardian. */
static void clear_card(void) {
	char who[48], stats[48];
	int secs = D.act_frames / 60;
	snprintf(who, sizeof who, "%s deleted", D.act_guardian ? D.act_guardian : "Guardian");
	snprintf(stats, sizeof stats, "Viruses %d   Time %d:%02d", run.viruses_deleted - D.act_viruses, secs / 60, secs % 60);
	/* (an act continued from a checkpoint has no whole count) */
	cinema_card(guardian_area_name(run.biome), "AREA CLEAR", who, D.act_resumed ? NULL : stats, rgba(248, 208, 88, 255), 220);
}

/* The next battle's enemies, for the game's encounter roll. */
static void set_encounter(const Encounter *e, bool force) {
	D.foes = e->nfoes;
	D.next = *e;
	if (emu_debug_on()) {
		fprintf(stderr, "encounter field %02x:", e->field);
		for (int i = 0; i < e->nfoes; ++i) fprintf(stderr, " %d/%d/%d@%d,%d", e->foes[i].kind, e->foes[i].family, e->foes[i].version, e->foes[i].col, e->foes[i].row);
		fprintf(stderr, "\n");
	}
	if (force) emu_battle_force(e);
	else emu_encounter_set(e);
}

#define CHECKPOINT_AFTER  60     /* frames after a layer is entered */
#define ASTRAY_FRAMES     90     /* on another map this long: warp back */

static int main_mode(void) { return emu_read8(emu_read32(BN6_TOOLKIT)); }
/* walking the net: the game mode on its map sub-mode (not a battle or menu) */
static bool on_map(void) { return main_mode() == BN6_MODE_GAME && emu_read8(BN6_GAMESTATE) == BN6_SUB_MAP; }

static int key_item(int id) { return emu_read8(emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_KEY_ITEMS) + (uint32_t)id); }

static const __typeof__(R.layout->net_area[0]) *area(int biome) {
	return &R.layout->net_area[biome < 0 || biome >= NET_AREAS ? 0 : biome];
}

static int layer_biome(void) {
	if (director_debug_biome >= 0 && director_debug_biome < BIOME_COUNT) return director_debug_biome;
	if (run.side_kind == LAYER_UNDERNET) return BIOME_UNDERNET;
	if (run.side_kind == LAYER_SECRET) return BIOME_SECRET;
	return biome_for_depth(run.depth);
}

/* MegaMan stays in the run: no jacking out, and no PET save to the game's
 * own flash (the run keeps its checkpoints). The NaviCust is there from the
 * start, for the programs a run finds and buys. */
static void lock_run(void) {
	flag_set(BN6_FLAG_NO_JACK);
	flag_set(BN6_FLAG_NO_PET_SAVE);
	flag_set(BN6_FLAG_NAVICUST);
}

/* The next random battle: the run's first two and every one on the first
 * layer after a guardian from the lower half of the act's band
 * (docs/PROGRESSION.md). */
static void roll_encounter(void) {
	bool opening = run.side_kind == LAYER_NORMAL && layer_in_act(run.depth) == 0 &&
		(run.depth == 1 ? D.battles < 2 : true);
	bool first = run.depth == 1 && run.side_kind == LAYER_NORMAL && D.battles == 0;
	Encounter e = make_encounter(run.depth, run.biome, first ? ENC_FIRST : opening ? ENC_EASY : ENC_NORMAL);
	set_encounter(&e, false);
}

static bool build_layer(void) {
	int biome = layer_biome();
	run.biome = biome;
	run.layer_seed = run.seed ^ (uint32_t)(run.depth * 2654435761u) ^ (uint32_t)(run.side_kind * 40503u);
	int rise;
	unsigned stairs = netmap_stair_dirs(biome, &rise);
	layer_generate(run.layer_seed, run.depth, biome, run.side_kind, stairs, rise);
	if (emu_debug_on()) fprintf(stderr, "layer depth %d biome %d layout %d stairs %d rise %d\n", run.depth, biome, layer.layout, layer.nstairs, layer.rise);
	if (!netmap_build_layer(biome, run.layer_seed)) return false;

	const __typeof__(R.layout->net_area[0]) *a = area(biome);
	D.group = a->group;
	D.number = a->number;
	if (!layer_objs_install(D.group, D.number, &D.objs)) return false;
	mapslot_music(D.group, D.number, a->song);
	D.chosen = 0;
	boss_begin_layer(D.objs.archive, &D.objs.guardian);

	D.battles = 0;
	roll_encounter();
	D.start_x = D.objs.start_x;
	D.start_y = D.objs.start_y;
	D.free_x = D.start_x;
	D.free_y = D.start_y;
	D.wedged = 0;
	/* until MegaMan takes it, the exit pad leads back to the layer's start */
	mapslot_exit_to(D.group, D.number, D.start_x, D.start_y, 4);
	D.active = true;
	D.frame = 0;
	D.gameover = false;
	D.act_guardian = D.objs.guardian.navi ? guardian(D.objs.guardian.navi)->name : NULL;
	bool first_of_act = run.side_kind == LAYER_NORMAL && layer_in_act(run.depth) == 0;
	if (first_of_act || run.side_kind != LAYER_NORMAL || biome == BIOME_NEST) begin_area(first_of_act);
	arrival_words();
	memset(D.seen, 0, sizeof D.seen);
	D.layer_told = false;
	flag_clear(LAYER_TOLD_FLAG);
	D.arrow_pending = false;
	cinema_arrow(0, 0);
	D.secret_call = run.side_kind == LAYER_SECRET;
	talk_reset();
	return true;
}

bool director_start_run(void) {
	/* a new run leaves the last one behind: CONTINUE is for runs that
	 * have reached the net (one left so is no deletion to speak of) */
	town_after_abandon = save_exists();
	save_delete();
	/* the first layer, entered through the town's port; the town itself
	 * (its seed apart from the layers') */
	if (!build_layer()) return false;
	if (!town_plan(town_seed(run.seed)) || !town_install(D.group, D.number, D.start_x, D.start_y)) {
		fprintf(stderr, "town: not built; starting in the net\n");
		lock_run();
		emu_warp(D.group, D.number, D.start_x, D.start_y, 4);
		D.checkpoint = true;
		return true;
	}
	/* R jacks in there; the PET's own Save stays off */
	flag_clear(BN6_FLAG_NO_JACK);
	flag_set(BN6_FLAG_NO_PET_SAVE);
	flag_set(BN6_FLAG_NAVICUST);
	const TownInfo *ti = town_info();
	emu_warp(ti->group, ti->number, ti->start_x, ti->start_y, ti->start_face);
	D.town = true;
	D.town_seen = false;
	D.town_frames = 0;
	D.intro_said = false;
	D.port_told = false;
	D.free_x = town_info()->start_x;
	D.free_y = town_info()->start_y;
	return true;
}

bool director_in_town(void) { return D.active && D.town; }

bool director_on_layer(void) { return D.active && !D.town; }

/* The layer's map, while SELECT is held: the panels MegaMan has come near,
 * as the screen shows them (a panel 4 by 2 pixels, isometric), around him;
 * the exit pad, the arena, the services and the ways off it marked. */
/* SELECT held on a layer: the layer as far as MegaMan has seen it, over the
 * dimmed game. Panels stand apart, so a walkway reads as a line and a room
 * as a block; the whole seen floor is fitted in when it fits, else the map
 * follows MegaMan. The goal, until seen, is a mark on the frame the way it
 * lies. */
void director_draw_map(void) {
	if (!D.active || D.town || !D.map_shown || !on_map()) return;
	int x0 = P.core_x, y0 = P.core_y;
	fill_rect(x0, y0, 240, 160, rgba(0, 8, 28, 255));
	int bx = x0 + 6, by = y0 + 18, bw = 228, bh = 122;
	SDL_Color edge = rgba(120, 200, 255, 220);
	fill_rect(bx - 2, by - 2, bw + 4, 1, edge);
	fill_rect(bx - 2, by + bh + 1, bw + 4, 1, edge);
	fill_rect(bx - 2, by - 2, 1, bh + 4, edge);
	fill_rect(bx + bw + 1, by - 2, 1, bh + 4, edge);
	text_drawf(bx, y0 + 3, rgba(170, 220, 255, 255), TEXT_LEFT, "Layer %d", run.depth);
	int px = (int)emu_read32(BN6_PLAYER + 0x1C) >> 16, py = (int)emu_read32(BN6_PLAYER + 0x20) >> 16, mx, my;
	if (!netmap_panel(px, py, &mx, &my)) return;
	/* a grid step goes 4 pixels across and 2 down (x - y across, x + y down) */
	int umin = mx - my, umax = umin, vmin = mx + my, vmax = vmin;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			if (!D.seen[y][x] || layer.cell[y][x] != C_PATH) continue;
			int u = x - y, v = x + y;
			if (u < umin) umin = u;
			if (u > umax) umax = u;
			if (v < vmin) vmin = v;
			if (v > vmax) vmax = v;
		}
	/* larger while the seen floor fits at it */
	int cu = (umin + umax) / 2, cv = (vmin + vmax) / 2, s = 6;
	if ((umax - umin) * 6 + 14 > bw || (vmax - vmin) * 3 + 8 > bh) s = 4;
	if (s == 4 && ((umax - umin) * 4 + 10 > bw || (vmax - vmin) * 2 + 6 > bh)) { cu = mx - my; cv = mx + my; }
	int ox = bx + bw / 2, oy = by + bh / 2;
	#define SX(x, y) (ox + ((x) - (y) - cu) * s)
	#define SY(x, y) (oy + ((x) + (y) - cv) * (s / 2))
	#define INSIDE(sx, sy, m) ((sx) - (m) >= bx && (sy) - (m) >= by && (sx) + (m) < bx + bw && (sy) + (m) < by + bh)
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			if (!D.seen[y][x] || layer.cell[y][x] != C_PATH) continue;
			int sx = SX(x, y), sy = SY(x, y);
			if (!INSIDE(sx, sy, 3)) continue;
			/* a panel: a diamond 7 wide and 3 high (11 and 5 larger), a pixel
			 * apart from the next */
			SDL_Color c = layer.level[y][x] ? rgba(150, 210, 255, 240) : rgba(60, 140, 230, 240);
			if (s == 6) {
				fill_rect(sx - 1, sy - 2, 3, 1, c);
				fill_rect(sx - 3, sy - 1, 7, 1, c);
				fill_rect(sx - 5, sy, 11, 1, c);
				fill_rect(sx - 3, sy + 1, 7, 1, c);
				fill_rect(sx - 1, sy + 2, 3, 1, c);
			} else {
				fill_rect(sx - 1, sy - 1, 3, 1, c);
				fill_rect(sx - 3, sy, 7, 1, c);
				fill_rect(sx - 1, sy + 1, 3, 1, c);
			}
		}
	/* what stands there, once seen; the goal's way while it is not */
	int gx = -1, gy = -1;
	bool goal_boss = false;   /* (the guardian while it stands, else the exit) */
	SDL_Color gc = rgba(255, 230, 60, 255);
	for (int i = 0; i < layer.nobj; ++i) {
		const NetObj *o = &layer.obj[i];
		int x = (int)o->x, y = (int)o->y;
		SDL_Color c;
		switch (o->type) {
		case OBJ_EXIT: case OBJ_RETURN: c = rgba(255, 230, 60, 255); break;
		case OBJ_BOSS: c = rgba(255, 70, 70, 255); break;
		case OBJ_HEAL: c = rgba(90, 255, 120, 255); break;
		case OBJ_SHOP: case OBJ_PROGRAMS: case OBJ_TRADER: case OBJ_BUGTRADER: c = rgba(255, 160, 40, 255); break;
		case OBJ_UNDERNET: case OBJ_SECRET_GATE: case OBJ_CHALLENGE: c = rgba(210, 110, 255, 255); break;
		default: continue;
		}
		if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) continue;
		if (o->type == OBJ_BOSS && !boss_beaten()) { gx = x; gy = y; gc = c; goal_boss = true; }
		else if (o->type == OBJ_EXIT && !goal_boss) { gx = x; gy = y; }
		int sx = SX(x, y), sy = SY(x, y);
		if (!D.seen[y][x]) {
			/* a service MegaMan senses but has not come near: its ring
			 * where it stands, or a pip on the frame's edge its way (L
			 * named the Recovery Mr. Prog, and it was nowhere on the map) */
			if (o->type == OBJ_EXIT || o->type == OBJ_RETURN || o->type == OBJ_BOSS) continue;
			if (!INSIDE(sx, sy, 3)) {
				double dx = sx - SX(mx, my), dy = sy - SY(mx, my), t = 1e9, hx = bw / 2.0 - 3, hy = bh / 2.0 - 3;
				double sx0 = SX(mx, my), sy0 = SY(mx, my);
				if (dx > 0) t = fmin(t, (ox + hx - sx0) / dx);
				if (dx < 0) t = fmin(t, (ox - hx - sx0) / dx);
				if (dy > 0) t = fmin(t, (oy + hy - sy0) / dy);
				if (dy < 0) t = fmin(t, (oy - hy - sy0) / dy);
				if (!(t > 0 && t < 1e8)) continue;
				sx = (int)lround(sx0 + dx * t); sy = (int)lround(sy0 + dy * t);
				fill_rect(sx - 1, sy - 1, 3, 3, c);
				continue;
			}
			fill_rect(sx - 3, sy - 3, 7, 7, c);
			fill_rect(sx - 2, sy - 2, 5, 5, rgba(0, 8, 28, 255));
			continue;
		}
		if (!INSIDE(sx, sy, 3)) continue;
		fill_rect(sx - 3, sy - 3, 7, 7, rgba(0, 8, 28, 255));
		fill_rect(sx - 2, sy - 2, 5, 5, c);
	}
	if (gx >= 0 && !D.seen[gy][gx]) {
		/* where the ray from MegaMan to it leaves the frame */
		double dx = SX(gx, gy) - SX(mx, my), dy = SY(gx, gy) - SY(mx, my);
		double t = 1e9, hx = bw / 2.0 - 5, hy = bh / 2.0 - 5;
		double sx0 = SX(mx, my), sy0 = SY(mx, my);
		if (dx > 0) t = fmin(t, (ox + hx - sx0) / dx);
		if (dx < 0) t = fmin(t, (ox - hx - sx0) / dx);
		if (dy > 0) t = fmin(t, (oy + hy - sy0) / dy);
		if (dy < 0) t = fmin(t, (oy - hy - sy0) / dy);
		if (t > 0 && t < 1e8) {
			int ax = (int)lround(sx0 + dx * t), ay = (int)lround(sy0 + dy * t);
			fill_rect(ax - 1, ay - 3, 3, 1, gc);
			fill_rect(ax - 2, ay - 2, 5, 1, gc);
			fill_rect(ax - 3, ay - 1, 7, 3, gc);
			fill_rect(ax - 2, ay + 2, 5, 1, gc);
			fill_rect(ax - 1, ay + 3, 3, 1, gc);
		}
	}
	/* MegaMan, always there, his border pulsing */
	int ms = SX(mx, my), mt = SY(mx, my);
	if (INSIDE(ms, mt, 3)) {
		fill_rect(ms - 3, mt - 3, 7, 7, (D.frame / 10) % 2 ? rgba(120, 200, 255, 255) : rgba(0, 8, 28, 255));
		fill_rect(ms - 2, mt - 2, 5, 5, rgba(255, 255, 255, 255));
	}
	/* the key, under the map: MegaMan, the exit, and what else this layer
	 * holds (a Server, a dark warp or a gate is "Event") */
	static const struct { const char *what; SDL_Color c; } key[] = {
		{ "You", { 255, 255, 255, 255 } }, { "Exit", { 255, 230, 60, 255 } },
		{ "Heal", { 90, 255, 120, 255 } }, { "Shop", { 255, 160, 40, 255 } }, { "Boss", { 255, 70, 70, 255 } },
		{ "Event", { 210, 110, 255, 255 } },
	};
	bool has[6] = { true, true, false, false, D.objs.guardian.navi != 0, false };
	for (int i = 0; i < layer.nobj; ++i)
		switch (layer.obj[i].type) {
		case OBJ_HEAL: has[2] = true; break;
		case OBJ_SHOP: case OBJ_PROGRAMS: case OBJ_TRADER: case OBJ_BUGTRADER: has[3] = true; break;
		case OBJ_UNDERNET: case OBJ_SECRET_GATE: case OBJ_CHALLENGE: has[5] = true; break;
		default: break;
		}
	/* (the gaps close up until it fits the picture's width) */
	int gap = 10, width;
	do {
		width = 0;
		for (int i = 0; i < 6; ++i) if (has[i]) width += 8 + text_width(key[i].what) + gap;
		width -= gap;
	} while (width > bw - 2 && (gap -= 2) >= 0);
	if (gap < 0) gap = 0;
	int kx = bx + 1, ky = by + bh + 5;
	for (int i = 0; i < 6; ++i) {
		if (!has[i]) continue;
		fill_rect(kx, ky + 3, 5, 5, key[i].c);
		text_draw(kx + 7, ky, key[i].what, rgba(200, 225, 255, 255), TEXT_LEFT);
		kx += 8 + text_width(key[i].what) + gap;
	}
	#undef SX
	#undef SY
	#undef INSIDE
}

/* The layers' make (generation, objects, loot rolls): a run saved by a build
 * that makes them otherwise continues its layer afresh from its start (the
 * saved RAM's flags and Mystery Data would not match this build's). Bump it
 * with any change to what a layer seed makes. */
#define LAYER_MAKE 15
#define LAYER_MAKE_MAGIC 0x434D4B31u   /* "CMK1" */
#define LAYER_SEEN_MAGIC 0x43534E31u   /* "CSN1" */

static void save_checkpoint(void) {
	char path[600];
	save_state_path(path, sizeof path);
	save_run();
	emu_save_state(path);
	int make = LAYER_MAKE;
	save_write_blob("run.make", LAYER_MAKE_MAGIC, &make, sizeof make);
	/* the map's panels seen so far, beside the state they go with */
	save_write_blob("run.seen", LAYER_SEEN_MAGIC, D.seen, sizeof D.seen);
}

bool director_can_suspend(void) {
	return D.active && !D.town && !D.gameover && on_map() && !emu_read8(BN6_CHATBOX) && !talk_busy() && !D.warping &&
		emu_read8(BN6_WARP + 0x10) == 0 && boss_idle() && !D.challenge && !emu_read8(BN6_DIALOGUE_LOCK) &&
		flag_get(BN6_FLAG_PLAYER_CAN_MOVE);
}

bool director_suspend(void) {
	if (!director_can_suspend()) return false;
	save_checkpoint();
	return true;
}

bool director_arrived(void) {
	if (!D.active || !on_map()) return false;
	int group = emu_read8(BN6_GAMESTATE + 4), number = emu_read8(BN6_GAMESTATE + 5);
	return D.town ? group == town_info()->group && number == town_info()->number : group == D.group && number == D.number;
}

static void print_near(int id, int x, int y, void *ctx) {
	int px = (int)emu_read32(BN6_PLAYER + 0x1C) >> 16, py = (int)emu_read32(BN6_PLAYER + 0x20) >> 16;
	if (abs(x - px) < 48 && abs(y - py) < 48) fprintf(ctx, "near %s %d %d\n", id < 0 ? "folk" : "object", x, y);
}

void director_describe(FILE *f) {
	if (!D.active) { fprintf(f, "where none\n"); return; }
	int mode = main_mode(), sub = emu_read8(BN6_GAMESTATE);
	const char *doing = mode == BN6_MODE_GAME_OVER ? "gameover"
		: mode != BN6_MODE_GAME ? "menu"
		: sub == BN6_SUB_MAP ? "map" : sub == BN6_SUB_BATTLE || sub == BN6_SUB_BATTLE_INIT ? "battle" : "other";
	fprintf(f, "where %s\ndoing %s\nchat %s\ntalk %s\n", D.town ? "town" : "layer", doing,
		emu_read8(BN6_CHATBOX) ? "open" : "closed", talk_busy() ? "director" : "none");
	/* (in a battle the HUD's HP is MegaMan's battle object's) */
	int hp = emu_read16(BN6_NAVI_STATS + 0x40), max = emu_read16(BN6_NAVI_STATS + 0x42);
	if (!on_map())
		for (uint32_t i = 0; i < BN6_T1_COUNT; ++i) {
			uint32_t o = BN6_T1_OBJECTS + i * BN6_T1_SIZE;
			if ((emu_read8(o) & 1) && emu_read8(o + 0x16) == 0) { hp = emu_read16(o + 0x24); max = emu_read16(o + 0x26); break; }
		}
	fprintf(f, "hp %d/%d\nzenny %u\n", hp, max, (unsigned)emu_read32(BN6_GAMESTATE + 0x5C));
	/* (for the developer reproducing a playtest: where Lan or MegaMan is) */
	if (getenv("CYBERWORLD_STATE_POS")) {
		fprintf(f, "pos %d %d %d locked %d jt %02x ace0 %d canmove %d f1718 %d f1719 %d cinema %d\n", (int)emu_read32(BN6_PLAYER + 0x1C) >> 16,
			(int)emu_read32(BN6_PLAYER + 0x20) >> 16, (int)emu_read32(BN6_PLAYER + 0x24) >> 16, emu_read8(BN6_PLAYER + 0x17),
			emu_read8(BN6_PLAYER + 9), emu_read8(BN6_DIALOGUE_LOCK), flag_get(BN6_FLAG_PLAYER_CAN_MOVE), flag_get(BN6_FLAG_DIALOGUE_1718),
			flag_get(BN6_FLAG_DIALOGUE_1719), cinema_input_mode());
		if (D.town) { town_objects(print_near, f); fprintf(f, "port %d %d\n", town_info()->port_x, town_info()->port_y); }
		else {
			int ns = 0, nf = 0;
			for (int y = 0; y < MAP_H; ++y) for (int x = 0; x < MAP_W; ++x) { ns += D.seen[y][x]; nf += D.seen[y][x] && layer.cell[y][x] == C_PATH; }
			fprintf(f, "seen %d floor %d\n", ns, nf);
			fprintf(f, "exit %d %d\nscripts shop %d heal %d gift %d programs %d\n", D.objs.exit_x, D.objs.exit_y, D.objs.script_of[OBJ_SHOP],
				D.objs.script_of[OBJ_HEAL], D.objs.script_of[OBJ_GIFT], D.objs.script_of[OBJ_PROGRAMS]);
			/* the floor around him, panels (x across, y down; @ he, # floor) */
			int px = (int)emu_read32(BN6_PLAYER + 0x1C) >> 16, py = (int)emu_read32(BN6_PLAYER + 0x20) >> 16, cx, cy;
			if (netmap_panel(px, py, &cx, &cy)) {
				int wx, wy;
				netmap_world(cx, cy, &wx, &wy);
				fprintf(f, "panel %d %d (centre %d %d)\n", cx, cy, wx, wy);
				for (int y = cy - 4; y <= cy + 4; ++y) {
					fprintf(f, "cells ");
					for (int x = cx - 4; x <= cx + 4; ++x)
						fputc(x == cx && y == cy ? '@' : x >= 0 && y >= 0 && x < MAP_W && y < MAP_H && layer.cell[y][x] == C_PATH ? '#' : '.', f);
					fputc('\n', f);
				}
			}
		}
		/* the game's NPC objects near him: flags, state, radius, lock, text */
		int px = (int)emu_read32(BN6_PLAYER + 0x1C) >> 16, py = (int)emu_read32(BN6_PLAYER + 0x20) >> 16;
		for (int i = 0; i < 16; ++i) {
			uint32_t o = 0x020057B0u + (uint32_t)i * 0xD8;
			int x = (int16_t)emu_read16(o + 0x26), y = (int16_t)emu_read16(o + 0x2A), z = (int16_t)emu_read16(o + 0x2E);
			if (!(emu_read8(o) & 1) || ((abs(x - px) > 64 || abs(y - py) > 64) && !getenv("CYBERWORLD_STATE_ALLNPC"))) continue;
			fprintf(f, "npc %d flags %02x state %02x radius %d zreach %d locked %d text %d at %d %d %d\n", i, emu_read8(o),
				emu_read8(o + 8), emu_read8(o + 0x0C), emu_read8(o + 0x0D), emu_read8(o + 0x17), emu_read8(o + 0x1C), x, y, z);
		}
	}
	if (D.town) return;
	fprintf(f, "layer %d\narea %s\nscrtdata %d\n", run.depth, guardian_area_in_text(run.biome, run.side_kind), run.fragments);
	if (D.objs.guardian.navi)
		fprintf(f, "guardian %s %s\n", guardian(D.objs.guardian.navi)->name,
			boss_done() ? "done" : boss_beaten() ? "beaten" : boss_fighting() ? "fighting" : "waiting");
}

/* Walking into a walkway's mouth off its line stops MegaMan at the corner
 * (a single-axis direction does not slide there). After a few frames of
 * that, where the panel ahead is empty but the one beside him and the one
 * diagonally ahead are floor, his direction turns toward them until he
 * has moved on along his own (a second at most). */
static bool floor_panel(int x, int y) { return x >= 0 && y >= 0 && x < MAP_W && y < MAP_H && layer.cell[y][x] == C_PATH; }

/* the pad's world steps (UP moves +X -Y, RIGHT +X +Y); the grid's +x is
 * world +Y and its +y world -X (netmap_panel). The first four go along the
 * grid, the single keys across its diagonal. */
static const struct { uint32_t keys; int x, y; } pad_dirs[8] = {
	{ KEY_UP | KEY_RIGHT, 1, 0 }, { KEY_DOWN | KEY_RIGHT, 0, 1 }, { KEY_DOWN | KEY_LEFT, -1, 0 }, { KEY_UP | KEY_LEFT, 0, -1 },
	{ KEY_UP, 1, -1 }, { KEY_RIGHT, 1, 1 }, { KEY_DOWN, -1, 1 }, { KEY_LEFT, -1, -1 },
};

/* A world step (x, y) of one axis from grid panel (cx, cy) onto floor, and
 * the pad key for it (the grid's +x is world +Y, its +y world -X). */
static bool open_step(int cx, int cy, int x, int y) { return floor_panel(cx + y, cy - x); }

static int step_key(int x, int y) {
	for (int k = 0; k < 4; ++k) if (pad_dirs[k].x == x && pad_dirs[k].y == y) return k;
	return -1;
}

/* The step leads into a lane or a spur: floor, with nothing either side. */
static bool lane_step(int cx, int cy, int x, int y) {
	int nx = cx + y, ny = cy - x;
	return floor_panel(nx, ny) && !floor_panel(nx - x, ny - y) && !floor_panel(nx + x, ny + y);
}

/* For a single key held with MegaMan stuck at an edge: the one of its two
 * steps along the grid that is open when the other is not (-1: none). */
static int slide_axis(int held, int cx, int cy) {
	int x = pad_dirs[held].x, y = pad_dirs[held].y;
	bool a = open_step(cx, cy, x, 0), b = open_step(cx, cy, 0, y);
	if (a == b) return -1;
	return a ? step_key(x, 0) : step_key(0, y);
}

/* In a lane, one way on besides back to (bx, by): that way for a single
 * key (world step x, y) held against the lane's side, unless the way back
 * suits the key better (-1: not a lane, or back). A platform's corner is
 * no lane's turn: the floor between its two ways is platform (taken for
 * one, a key held into the corner walked him along the edge and back). */
static int corridor_way(int cx, int cy, int x, int y, int bx, int by) {
	static const int w[4][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	int k = -1, n = 0, on = -9, back = -9, ox = 0, oy = 0;
	for (int i = 0; i < 4; ++i) {
		int nx = cx + w[i][1], ny = cy - w[i][0];
		if (!floor_panel(nx, ny)) continue;
		int dot = w[i][0] * x + w[i][1] * y;
		if (nx == bx && ny == by) { back = dot; continue; }
		++n;
		k = step_key(w[i][0], w[i][1]);
		on = dot;
		ox = nx - cx; oy = ny - cy;
	}
	if (n == 1 && back > -9 && ox != cx - bx && oy != cy - by && floor_panel(cx + ox + bx - cx, cy + oy + by - cy)) return -1;
	return n == 1 && on >= back ? k : -1;
}

/* Calls without MegaMan moving before a held key counts as stuck: the game
 * moves him from the third frame a key is held, so three still frames
 * were every start from rest, and the assist stepped him to the panel's
 * middle before letting him go the way he pressed. */
#define STUCK 5

static uint32_t corner_assist(uint32_t keys) {
	static int stuck, lx, ly, assist = -1, assist_for = -1, along, frames;
	static bool turning, centring;                /* (the assist is a lane's turn, or a step to a panel's middle) */
	static int held_for, last_held = -1;          /* frames the same key has been held */
	static int pull_t, pull_for, pull_ex, pull_ey, pull_wx, pull_wy;   /* a pull into a lane: its frames, key, mouth and way */
	static int turn_cx, turn_cy, cur_cx = -99, cur_cy = -99, prev_cx = -99, prev_cy = -99;
	static int walked[12][2], nwalked;            /* the panels he has walked lately, newest last */
	uint32_t pad = keys & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT);
	int px = (int)emu_read32(BN6_PLAYER + 0x1C) >> 16, py = (int)emu_read32(BN6_PLAYER + 0x20) >> 16;
	bool moved = px != lx || py != ly;
	lx = px; ly = py;
	/* the panel he stands on and the one before it */
	int pcx, pcy;
	netmap_panel(px, py, &pcx, &pcy);
	if (pcx != cur_cx || pcy != cur_cy) {
		prev_cx = cur_cx; prev_cy = cur_cy; cur_cx = pcx; cur_cy = pcy;
		if (nwalked == 12) { memmove(walked, walked[1], sizeof walked - sizeof walked[0]); --nwalked; }
		walked[nwalked][0] = prev_cx; walked[nwalked][1] = prev_cy; ++nwalked;
	}
	int held = -1;
	for (int k = 0; k < 8; ++k) if (pad == pad_dirs[k].keys) held = k;
	if (held < 0) last_held = -1;
	if (held < 0 || D.town || emu_read8(BN6_CHATBOX) || talk_busy() || (assist >= 0 && held != assist_for)) {
		stuck = 0; assist = -1; turning = centring = false;
		return keys;
	}
	/* a single key goes across the panels' diagonal. Where one of its two
	 * steps leads into a lane and the diagonal is off the floor (a walkway's
	 * mouth, a lane's turn), MegaMan lines up with the lane, then goes in:
	 * no lining up by hand. Stuck at an edge otherwise, he slides along the
	 * step that is open. (Not for the autopilot, whose single keys follow
	 * its own path.) */
	if (held >= 4) {
		if (autopilot_on()) return keys;
		int cx, cy, x = pad_dirs[held].x, y = pad_dirs[held].y;
		netmap_panel(px, py, &cx, &cy);
		/* (a tap is left alone: the key held a moment first) */
		held_for = held == last_held ? held_for + 1 : 0;
		last_held = held;
		/* a pull under way: on towards the lane it chose until he stands in
		 * it (a pull re-chosen from each panel fought his own key where the
		 * floor opened: he rocked on a panel's border) */
		if (pull_t > 0 && held == pull_for) {
			if (++pull_t > 60 || (cx == pull_ex + pull_wy && cy == pull_ey - pull_wx)) pull_t = 0;
			else {
				int wcx, wcy;
				netmap_world(pull_ex, pull_ey, &wcx, &wcy);
				int off = pull_wx ? py - wcy : px - wcx;
				int k = abs(off) > 4 ? (pull_wx ? step_key(0, off > 0 ? -1 : 1) : step_key(off > 0 ? -1 : 1, 0)) : step_key(pull_wx, pull_wy);
				stuck = 0; assist = -1;
				return (keys & ~pad) | pad_dirs[k].keys;
			}
		}
		pull_t = 0;
		if (held_for >= 3 && !floor_panel(cx + y, cy - x)) {
			/* a lane's mouth his key points into, opening from his panel or
			 * the one beside it across the lane (standing a unit short of
			 * the panel before a walkway, the mouth was his panel's diagonal
			 * and LEFT slid him back to the corner): on to its middle line,
			 * a step sideways if need be, then in; never into a lane he has
			 * lately walked (he came out of it) */
			int best_off = 37;
			for (int along = 0; along < 2; ++along) {
				int wx = along == 0 ? x : 0, wy = along == 0 ? 0 : y;   /* the lane's way, world */
				int gx = wy, gy = -wx, qx = abs(gy), qy = abs(gx);      /* ... on the grid, and across it */
				for (int s = -1; s <= 1; ++s) {
					int ex = cx + qx * s, ey = cy + qy * s;               /* where the lane opens from */
					if (!floor_panel(ex, ey) || !lane_step(ex, ey, wx, wy)) continue;
					bool walked_it = false;
					for (int i = 0; i < nwalked; ++i) walked_it |= walked[i][0] == ex + gx && walked[i][1] == ey + gy;
					if (walked_it) continue;
					int wcx, wcy;
					netmap_world(ex, ey, &wcx, &wcy);
					int off = wx ? py - wcy : px - wcx;                   /* across the lane, world */
					if (abs(off) >= best_off) continue;
					best_off = abs(off);
					pull_ex = ex; pull_ey = ey; pull_wx = wx; pull_wy = wy;
				}
			}
			if (best_off < 37) {
				pull_t = 1;
				pull_for = held;
				int wcx, wcy;
				netmap_world(pull_ex, pull_ey, &wcx, &wcy);
				int off = pull_wx ? py - wcy : px - wcx;
				int k = abs(off) > 4 ? (pull_wx ? step_key(0, off > 0 ? -1 : 1) : step_key(off > 0 ? -1 : 1, 0)) : step_key(pull_wx, pull_wy);
				stuck = 0; assist = -1;
				return (keys & ~pad) | pad_dirs[k].keys;
			}
		}
		if (assist >= 0) {
			frames = moved ? 0 : frames + 1;
			int k = slide_axis(held, cx, cy);
			if (k < 0) k = corridor_way(cx, cy, x, y, prev_cx, prev_cy);
			if (frames > 6 || k < 0) { assist = -1; stuck = 0; return keys; }
			assist = k;
			return (keys & ~pad) | pad_dirs[assist].keys;
		}
		stuck = moved ? 0 : stuck + 1;
		if (stuck < STUCK) return keys;
		/* (at a lane's peak or tip, where neither of the key's steps goes
		 * on: along the lane) */
		int k = slide_axis(held, cx, cy);
		if (k < 0) k = corridor_way(cx, cy, x, y, prev_cx, prev_cy);
		if (k < 0) return keys;
		assist = k;
		assist_for = held;
		frames = 0;
		return (keys & ~pad) | pad_dirs[k].keys;
	}
	int ax = pad_dirs[held].x, ay = pad_dirs[held].y;
	if (assist >= 0 && turning) {
		/* round a lane's turn: to the panel's middle on his own axis, then
		 * along the side; on the next panel on round the lane while it
		 * goes on one way only, and let go where the pad's way opens */
		frames = moved ? 0 : frames + 1;
		if (frames > 8) { assist = -1; turning = false; stuck = 0; return keys; }   /* (something in the way) */
		if (pcx != turn_cx || pcy != turn_cy) {
			int k = -1, open = 0;
			if (!floor_panel(pcx + ay, pcy - ax))
				for (int s = -1; s <= 1; s += 2) {
					int sx = ay ? s : 0, sy = ax ? s : 0, nx = pcx + sy, ny = pcy - sx;
					if (!floor_panel(nx, ny) || (nx == prev_cx && ny == prev_cy)) continue;
					++open;
					k = step_key(sx, sy);
				}
			if (open != 1 || k < 0) { assist = -1; turning = false; stuck = 0; return keys; }
			assist = k;
			turn_cx = pcx; turn_cy = pcy;
		}
		int wcx, wcy;
		netmap_world(pcx, pcy, &wcx, &wcy);
		int off = ax ? px - wcx : py - wcy;
		if (abs(off) > 4) return (keys & ~pad) | pad_dirs[step_key(ax ? (off > 0 ? -1 : 1) : 0, ay ? (off > 0 ? -1 : 1) : 0)].keys;
		return (keys & ~pad) | pad_dirs[assist].keys;
	}
	if (assist >= 0 && centring) {
		/* to the panel's middle across his way, then on */
		int wcx, wcy;
		netmap_world(pcx, pcy, &wcx, &wcy);
		int off = ax ? py - wcy : px - wcx;
		if (abs(off) <= 3 || ++frames > 30) { assist = -1; centring = false; stuck = 0; return keys; }
		return (keys & ~pad) | pad_dirs[step_key(ax ? 0 : (off > 0 ? -1 : 1), ax ? (off > 0 ? -1 : 1) : 0)].keys;
	}
	if (assist >= 0) {
		int a = ax ? px * ax : py * ay;
		if (a > along || ++frames > 60) { assist = -1; stuck = 0; return keys; }
		return (keys & ~pad) | pad_dirs[assist].keys;
	}
	stuck = moved ? 0 : stuck + 1;
	int cx, cy;
	if (stuck < STUCK || !netmap_panel(px, py, &cx, &cy)) return keys;
	int gdx = ay, gdy = -ax;   /* his direction on the grid */
	if (floor_panel(cx + gdx, cy + gdy)) {
		/* the way ahead is floor: off the middle of a lane he scrapes its
		 * side, so a step across to the middle first (a navi in the way
		 * is left to him; the autopilot keeps to its own path) */
		if (autopilot_on()) return keys;
		int wcx, wcy;
		netmap_world(cx, cy, &wcx, &wcy);
		int off = ax ? py - wcy : px - wcx;
		if (abs(off) <= 4) return keys;
		assist = step_key(ax ? 0 : (off > 0 ? -1 : 1), ax ? (off > 0 ? -1 : 1) : 0);
		assist_for = held;
		centring = true;
		frames = 0;
		return (keys & ~pad) | pad_dirs[assist].keys;
	}
	for (int s = -1; s <= 1; s += 2) {
		int sx = ay ? s : 0, sy = ax ? s : 0, gsx = sy, gsy = -sx;
		if (!floor_panel(cx + gsx, cy + gsy) || !floor_panel(cx + gsx + gdx, cy + gsy + gdy)) continue;
		for (int k = 4; k < 8; ++k)
			if (pad_dirs[k].x == ax + sx && pad_dirs[k].y == ay + sy) {
				assist = k;
				assist_for = held;
				along = ax ? px * ax : py * ay;
				frames = 0;
				return (keys & ~pad) | pad_dirs[k].keys;
			}
	}
	/* no step round it: a lane turning, one side going on (not back the way
	 * he came), is followed (with the pad held the old way, on round its
	 * turns; a junction is left to him) */
	int k = -1, open = 0;
	for (int s = -1; s <= 1; s += 2) {
		int sx = ay ? s : 0, sy = ax ? s : 0, nx = cx + sy, ny = cy - sx;
		if (!floor_panel(nx, ny) || (nx == prev_cx && ny == prev_cy)) continue;
		++open;
		k = step_key(sx, sy);
	}
	if (open == 1 && k >= 0 && !autopilot_on()) {
		assist = k;
		assist_for = held;
		turning = true;
		turn_cx = cx; turn_cy = cy;
		frames = 0;
		return (keys & ~pad) | pad_dirs[k].keys;
	}
	return keys;
}

/* A on the map: MegaMan turns to face what he means to talk to, so the
 * game's own check finds it (walking into a navi slides him round it, and a
 * tap of the pad can leave him facing past it): the navi or Mystery Data
 * clearly before him if there is one, out to two and a half panels, else
 * the nearest within 52 units (the probes' reach). One out of reach he
 * walks up to (a step or two short of a navi, the press did nothing). */
static void probe_vectors(int vx[8], int vy[8]) {
	for (int k = 0; k < 8; ++k) {
		vx[k] = (int32_t)emu_read32(BN6_TALK_PROBES + (uint32_t)k * 24) >> 16;
		vy[k] = (int32_t)emu_read32(BN6_TALK_PROBES + (uint32_t)k * 24 + 4) >> 16;
	}
}

/* The NPC slot A means, or -1. */
static int talk_target(void) {
	int px = (int)emu_read32(BN6_PLAYER + 0x1C) >> 16, py = (int)emu_read32(BN6_PLAYER + 0x20) >> 16;
	int face = emu_read8(BN6_PLAYER + 0x10) & 7, vx[8], vy[8];
	probe_vectors(vx, vy);
	double fl = sqrt((double)vx[face] * vx[face] + vy[face] * vy[face]);
	if (fl < 1) return -1;
	int front = -1, near = -1, fd = 1 << 30, nd = 52 * 52 + 1;
	for (int i = 0; i < 16; ++i) {
		uint32_t o = 0x020057B0u + (uint32_t)i * 0xD8;   /* the game's NPC objects (director_describe) */
		if (!(emu_read8(o) & 1) || !emu_read8(o + 0x0C)) continue;
		int dx = (int16_t)emu_read16(o + 0x26) - px, dy = (int16_t)emu_read16(o + 0x2A) - py, d = dx * dx + dy * dy;
		/* before him: within 30 degrees of his facing (one he faces out of
		 * reach is the one he means: no turn to another beside him) */
		if (d <= 80 * 80 && d > 0 && (vx[face] * dx + vy[face] * dy) / (fl * sqrt((double)d)) >= 0.866 && d < fd) { fd = d; front = i; }
		if (d <= 52 * 52 && d < nd) { nd = d; near = i; }
	}
	/* (but one he touches beats one before him out of reach) */
	if (front >= 0 && !(near >= 0 && nd <= 24 * 24 && fd > 52 * 52)) return front;
	return near;
}

/* Where NPC slot i stands from MegaMan, the facing whose probe points at
 * it best, and whether that probe reaches it. */
static bool talk_reach(int i, int *face) {
	int px = (int)emu_read32(BN6_PLAYER + 0x1C) >> 16, py = (int)emu_read32(BN6_PLAYER + 0x20) >> 16, vx[8], vy[8];
	uint32_t o = 0x020057B0u + (uint32_t)i * 0xD8;
	int tx = (int16_t)emu_read16(o + 0x26) - px, ty = (int16_t)emu_read16(o + 0x2A) - py;
	probe_vectors(vx, vy);
	int k = emu_read8(BN6_PLAYER + 0x10) & 7;
	double top = -2;
	for (int f = 0; f < 8; ++f) {
		double l = sqrt((double)vx[f] * vx[f] + vy[f] * vy[f]) * sqrt((double)tx * tx + ty * ty);
		double c = l > 0 ? (vx[f] * tx + vy[f] * ty) / l : -2;
		if (c > top) { top = c; k = f; }
	}
	*face = k;
	/* (the probe's circle and the NPC's meet, with a little to spare) */
	int r = emu_read8(BN6_TALK_PROBES + (uint32_t)k * 24 + 12) + emu_read8(o + 0x0C) - 3;
	int ex = tx - vx[k], ey = ty - vy[k];
	return ex * ex + ey * ey <= r * r;
}

static void talk_turn(int k) {
	emu_write8(BN6_PLAYER + 0x10, (uint8_t)k);
	emu_write8(BN6_PLAYER + 0x14, (uint8_t)k);
}

#define PAD_KEYS (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)
/* the pad for each facing, in the probes' order */
static const uint32_t face_pad[8] = {
	KEY_UP, KEY_UP | KEY_RIGHT, KEY_RIGHT, KEY_DOWN | KEY_RIGHT, KEY_DOWN, KEY_DOWN | KEY_LEFT, KEY_LEFT, KEY_UP | KEY_LEFT,
};

/* Two navis side by side: the game's own check takes the first whose ring
 * its probe touches (every A went to the Recovery Mr. Prog beside the Net
 * Dealer Kai faced), so while the press goes through, the others near have
 * no ring. */
static struct { int t; uint8_t r[16]; } excl;

static void talk_only(int i) {
	int px = (int)emu_read32(BN6_PLAYER + 0x1C) >> 16, py = (int)emu_read32(BN6_PLAYER + 0x20) >> 16;
	for (int j = 0; j < 16; ++j) {
		uint32_t o = 0x020057B0u + (uint32_t)j * 0xD8;
		uint8_t r = emu_read8(o + 0x0C);
		if (j == i || !(emu_read8(o) & 1) || !r || excl.r[j]) continue;
		int dx = (int16_t)emu_read16(o + 0x26) - px, dy = (int16_t)emu_read16(o + 0x2A) - py;
		if (dx * dx + dy * dy > 96 * 96) continue;
		excl.r[j] = r;
		emu_write8(o + 0x0C, 0);
	}
	excl.t = 8;   /* (the game takes a press two frames on) */
}

static void talk_only_update(void) {
	if (!excl.t || --excl.t) return;
	for (int j = 0; j < 16; ++j) {
		if (excl.r[j]) emu_write8(0x020057B0u + (uint32_t)j * 0xD8 + 0x0C, excl.r[j]);
		excl.r[j] = 0;
	}
}

/* The walk up: towards the NPC until its probe reaches, then A for him.
 * The pad or B takes over; a wall ends it with the A all the same. */
static uint32_t talk_walk(uint32_t keys) {
	static int last_x, last_y, still;
	int px = (int)emu_read32(BN6_PLAYER + 0x1C) >> 16, py = (int)emu_read32(BN6_PLAYER + 0x20) >> 16;
	uint32_t o = 0x020057B0u + (uint32_t)D.walk_to * 0xD8;
	if ((keys & (PAD_KEYS | KEY_B)) || !(emu_read8(o) & 1) || !emu_read8(o + 0x0C) || emu_read8(BN6_CHATBOX) || talk_busy()) {
		D.walk_t = 0;
		return keys;
	}
	if (D.walk_t == WALK_UP) still = 0;
	else if (px == last_x && py == last_y) ++still;
	else still = 0;
	last_x = px; last_y = py;
	int k;
	bool reach = talk_reach(D.walk_to, &k);
	if (reach || still >= 6 || --D.walk_t <= 0) {
		D.walk_t = 0;
		talk_turn(k);
		talk_only(D.walk_to);
		return (keys & ~PAD_KEYS) | KEY_A;
	}
	return (keys & ~(PAD_KEYS | KEY_A)) | face_pad[k];
}

/* A shop's list opened by its keeper's last box: the A pressed to close
 * that box twice over bought the first item's "Are you sure?" (Kai, three
 * sessions running). A is held off the list's first frames. */
static uint32_t shop_guard(uint32_t keys) {
	static int chat_recent, guard, last_mode = -1;
	int mode = main_mode();
	chat_recent = emu_read8(BN6_CHATBOX) ? 30 : chat_recent > 0 ? chat_recent - 1 : 0;
	if (last_mode == BN6_MODE_GAME && mode != BN6_MODE_GAME && mode != BN6_MODE_GAME_OVER && chat_recent) guard = 45;
	last_mode = mode;
	if (guard > 0) { --guard; keys &= ~KEY_A; }
	return keys;
}

uint32_t director_keys(uint32_t keys) {
	talk_only_update();
	if (D.active) keys = shop_guard(keys);
	bool l = (keys & KEY_L) != 0, pressed = l && !D.l_held;
	bool r = (keys & KEY_R) != 0, r_pressed = r && !D.r_held;
	bool a = (keys & KEY_A) != 0, a_pressed = a && !D.a_held;
	D.l_held = l;
	D.r_held = r;
	D.a_held = a;
	D.dir_held = (keys & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) != 0;
	D.map_shown = false;
	if (!D.active || !on_map()) return keys;   /* (in battle L opens the Custom screen) */
	keys = corner_assist(keys);
	/* (turned to what A would talk to, the pad left alone for that frame so
	 * the game does not turn him back; not in the town, where A also reads
	 * the doors and signs Lan faces) */
	if (D.walk_t > 0) keys = talk_walk(keys);
	else if (a_pressed && !D.town && !talk_busy() && !emu_read8(BN6_CHATBOX) && !cinema_busy() && !D.warping) {
		int i = talk_target(), k;
		if (i >= 0 && talk_reach(i, &k)) {
			if (k != (emu_read8(BN6_PLAYER + 0x10) & 7)) { talk_turn(k); keys &= ~PAD_KEYS; }
			talk_only(i);
		} else if (i >= 0 && !(keys & PAD_KEYS) && !autopilot_on()) {
			/* (not while he walks: the pad is his) */
			D.walk_to = i;
			D.walk_t = WALK_UP;
			keys = talk_walk(keys & ~KEY_A);
		}
	}
	/* SELECT on a layer: the map, while it is held */
	if (!D.town && (keys & KEY_SELECT)) { D.map_shown = !emu_read8(BN6_CHATBOX); keys &= ~KEY_SELECT; }
	/* R in the town away from the port: MegaMan says where it is (the game
	 * itself does nothing there) */
	if (D.town && r_pressed && !talk_busy() && !emu_read8(BN6_CHATBOX) && !cinema_busy() && emu_read8(BN6_WARP + 0x10) == 0 &&
		!town_on_port((int)emu_read32(BN6_PLAYER + 0x1C) >> 16, (int)emu_read32(BN6_PLAYER + 0x20) >> 16)) {
		static char buf[160];
		int px = (int)emu_read32(BN6_PLAYER + 0x1C) >> 16, py = (int)emu_read32(BN6_PLAYER + 0x20) >> 16;
		int dx = town_info()->port_x - px, dy = town_info()->port_y - py, far;
		const char *way = way_to(town_info()->port_x, town_info()->port_y, &far);
		/* (close by, from any side of it: a step more, and which way) */
		if (dx * dx + dy * dy < 128 * 128)
			snprintf(buf, sizeof buf, "@M Almost, Lan! The statue's %s.|@M Step right up to it and press R.", way);
		else
			snprintf(buf, sizeof buf, "@M There's no port here, Lan.|@M It's by the %s!",
				town_info()->group == 0x00 ? "squirrel statue in the park" : "bird statue on the plaza");
		talk_start(buf, FACE_MEGAMAN);
		D.arrow_pending = true;
		cinema_arrow(way_dir, 600);
		return keys & ~KEY_R;
	}
	/* on the map L is MegaMan's word on where they are: the game's own
	 * has no lines for this story */
	keys &= ~KEY_L;
	/* (not while a warp or the jack-in departs, nor through a guardian's
	 * staging or the battle it has armed; an L pressed as a chat closes is
	 * kept half a second, as the first press after one went unheard) */
	bool can_l = !talk_busy() && !emu_read8(BN6_CHATBOX) && !cinema_busy() && !D.warping && emu_read8(BN6_WARP + 0x10) == 0 &&
		(D.town || (!boss_cinematic() && !boss_fighting()));
	if (pressed && !can_l) D.l_kept = 30;
	else if (!pressed && D.l_kept > 0) { if (can_l) { pressed = true; D.l_kept = 0; } else --D.l_kept; }
	if (pressed && can_l) {
		D.l_kept = 0;
		/* (the arrow shows through the words and a few seconds after) */
		if (talk_start(status_words(), FACE_MEGAMAN) && (D.town || !D.objs.guardian.navi || !boss_beaten() || boss_done())) {
			D.arrow_pending = true;
			cinema_arrow(way_dir, 600);
		}
	}
	return keys;
}

/* MegaMan wedged inside an NPC (a walker came at him, or a wall's push-out
 * on a walkway moved him in): every one of his movement probes meets it, so
 * no direction moves him. After a second of pushing he is put back where
 * he last stood clear of every NPC (the game's NPC objects, bn6f
 * eOverworldNPCObjects: 16 of 0xD8 bytes). */
static void unwedge(void) {
	int px = (int)emu_read32(BN6_PLAYER + 0x1C) >> 16, py = (int)emu_read32(BN6_PLAYER + 0x20) >> 16;
	bool inside = false;
	for (int i = 0; i < 16 && !inside; ++i) {
		uint32_t o = 0x020057B0u + (uint32_t)i * 0xD8;
		int r = emu_read8(o + 0x0C);
		if (!(emu_read8(o) & 1) || !r) continue;
		int dx = (int16_t)emu_read16(o + 0x26) - px, dy = (int16_t)emu_read16(o + 0x2A) - py;
		inside = dx * dx + dy * dy < (r + 2) * (r + 2);
	}
	bool moved = px != D.last_x || py != D.last_y;
	D.last_x = px; D.last_y = py;
	if (!inside) { D.free_x = px; D.free_y = py; D.wedged = 0; return; }
	if (moved || !D.dir_held || emu_read8(BN6_CHATBOX) || talk_busy()) { D.wedged = 0; return; }
	if (++D.wedged < 60 || (D.free_x == px && D.free_y == py)) return;
	D.wedged = 0;
	emu_write32(BN6_PLAYER + 0x1C, (uint32_t)D.free_x << 16);
	emu_write32(BN6_PLAYER + 0x20, (uint32_t)D.free_y << 16);
	emu_write32(BN6_PLAYER + 0x28, (uint32_t)D.free_x << 16);
	emu_write32(BN6_PLAYER + 0x2C, (uint32_t)D.free_y << 16);
}

/* MegaMan pushing a while where the pad goes nowhere (a platform's
 * corner, a lane's end, with no walkway in reach to line him up with):
 * the way-on arrow shows along the floor, as after L's words. */
static void push_arrow(void) {
	static int ax, ay, pushed;
	int px = (int)emu_read32(BN6_PLAYER + 0x1C) >> 16, py = (int)emu_read32(BN6_PLAYER + 0x20) >> 16;
	/* (the edge's push-out jostles him a unit or two) */
	bool moved = abs(px - ax) > 4 || abs(py - ay) > 4;
	if (moved || !D.dir_held || D.walk_t || emu_read8(BN6_CHATBOX) || talk_busy() || cinema_busy() || boss_cinematic()) {
		ax = px; ay = py; pushed = 0;
		return;
	}
	if (++pushed != 40 || cinema_arrow_on()) return;
	goal_way();
	cinema_arrow(way_dir, 150);
}

/* In the town: nothing to watch but the jack-in, whose arrival on the
 * layer's map starts the run as a layer's warp does. */
/* The way-on arrow: on while L's words last, however many boxes, then
 * three seconds more. */
static void arrow_update(void) {
	/* (it turns as MegaMan walks: frozen, it pointed into the gap he had
	 * walked past) */
	static int tick;
	if (cinema_arrow_on() && on_map() && ++tick % 15 == 0) { goal_way(); cinema_arrow_turn(way_dir); }
	if (!D.arrow_pending) return;
	if (talk_busy()) cinema_arrow_extend(60);
	else { D.arrow_pending = false; cinema_arrow_extend(180); }
}

static void town_update(void) {
	map_label();
	arrow_update();
	if (on_map()) unwedge();
	int group = emu_read8(BN6_GAMESTATE + 4), number = emu_read8(BN6_GAMESTATE + 5);
	if (group == town_info()->group && number == town_info()->number) D.town_seen = true;
	talk_update();
	/* Lan and MegaMan's words (Dad's call, the first time), once Lan is
	 * out and the map has settled */
	if (D.town_seen && on_map() && !D.intro_said && emu_read8(BN6_WARP + 0x10) == 0 && ++D.town_frames > 40 &&
		talk_script(town_info()->talk_archive, town_info()->intro)) {
		D.intro_said = true;
		if (!profile.seen_intro) { profile.seen_intro = true; profile_save(); }
	}
	bool arrived = D.town_seen && on_map() && emu_read8(BN6_WARP + 0x10) == 0 &&
		emu_read8(BN6_GAMESTATE + 4) == D.group && emu_read8(BN6_GAMESTATE + 5) == D.number;
	if (!arrived) return;
	D.town = false;
	D.frame = 0;
	D.checkpoint = true;
	lock_run();
	mapslot_music_forget_town();
}

bool director_start_layer(void) {
	D.town = false;
	if (!build_layer()) return false;
	lock_run();
	emu_warp(D.group, D.number, D.start_x, D.start_y, 4);
	D.checkpoint = true;
	return true;
}

/* ---- dev tools (src/dev/devtools.c) ---- */

bool director_on_map(void) { return D.active && on_map(); }

void director_stop(void) { D.active = false; }

bool director_dev_next_layer(void) {
	if (!director_on_map()) return false;
	run.depth++;
	run.side_kind = LAYER_NORMAL;
	return director_start_layer();
}

void director_dev_place(int x, int y, int face) {
	/* (a test's step: MegaMan put down at world (x, y), facing 0-7) */
	if (!director_on_map()) return;
	emu_write32(BN6_PLAYER + 0x1C, (uint32_t)x << 16);
	emu_write32(BN6_PLAYER + 0x20, (uint32_t)y << 16);
	emu_write32(BN6_PLAYER + 0x28, (uint32_t)x << 16);
	emu_write32(BN6_PLAYER + 0x2C, (uint32_t)y << 16);
	if (face >= 0 && face < 8) { emu_write8(BN6_PLAYER + 0x10, (uint8_t)face); emu_write8(BN6_PLAYER + 0x14, (uint8_t)face); }
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

bool director_resume(void) {
	/* the layer's tables live in the ROM copy, which a state does not hold */
	if (!build_layer()) return false;
	char path[600];
	save_state_path(path, sizeof path);
	int make = 0;
	bool same = save_read_blob("run.make", LAYER_MAKE_MAGIC, &make, sizeof make) && make == LAYER_MAKE;
	if (emu_load_state(path)) {
		lock_run();
		/* the shops' data in RAM is the saved one: this layer's again */
		layer_objs_shops(&D.objs);
		if (!same) {
			/* another build's layer: its flags and Mystery Data picks
			 * forgotten, and in from the start */
			/* (a gift taken stays taken: it is the run's, not the layer's) */
			for (int f = MAPSLOT_MD_FLAG; f <= LAYER_TOLD_FLAG; ++f) if (f != LAYER_GIFT_FLAG) flag_clear(f);
			for (int i = 0; i <= LAYER_GIFT_FLAG - MAPSLOT_MD_FLAG; ++i) { uint8_t z[2] = { 0, 0 }; emu_write(BN6_MYSTERY_PICKS + 2 * (uint32_t)i, z, 2); }
			emu_write32(BN6_PLAYER + 0x1C, (uint32_t)D.start_x << 16);
			emu_write32(BN6_PLAYER + 0x20, (uint32_t)D.start_y << 16);
		}
		/* a state saved while the jack-in still held MegaMan (runs from
		 * before the checkpoint waited for him): the game's own release
		 * would never come, so it is done here */
		if (emu_read8(BN6_DIALOGUE_LOCK) || !flag_get(BN6_FLAG_PLAYER_CAN_MOVE)) {
			emu_write8(BN6_DIALOGUE_LOCK, 0);
			flag_set(BN6_FLAG_PLAYER_CAN_MOVE);
			flag_clear(BN6_FLAG_DIALOGUE_1719);
		}
		boss_resume();
		/* the map as far as it was seen (none for another build's layer) */
		if (same && !save_read_blob("run.seen", LAYER_SEEN_MAGIC, D.seen, sizeof D.seen)) memset(D.seen, 0, sizeof D.seen);
		/* choices made before the checkpoint stay made */
		for (int i = 0; i < D.objs.nchoices; ++i)
			if (flag_get(D.objs.choice[i].flag)) D.chosen |= 1u << i;
		/* enter the map again where MegaMan stood: the game reloads its NPCs
		 * and tiles from this build's tables, which a state does not hold */
		int x = (int)emu_read32(BN6_PLAYER + 0x1C) >> 16, y = (int)emu_read32(BN6_PLAYER + 0x20) >> 16;
		/* (a build that lays the layer out otherwise may have no floor there
		 * any more: then its arrival) */
		int cx, cy;
		if (!netmap_panel(x, y, &cx, &cy) || cx < 0 || cy < 0 || cx >= MAP_W || cy >= MAP_H || layer.cell[cy][cx] != C_PATH)
			x = D.start_x, y = D.start_y;
		emu_warp(D.group, D.number, x, y, 4);
		/* where they are, again; the arrival's words were said before */
		begin_area(false);
		D.act_resumed = true;
		D.beat[0] = 0;
		return true;
	}
	/* no state (a run from before the game engine): enter the layer fresh */
	emu_warp(D.group, D.number, D.start_x, D.start_y, 4);
	D.checkpoint = true;
	return true;
}

/* Into the Undernet or the Secret Area: MegaMan jacks out as on a warp pad,
 * and into the side layer built meanwhile. */
static void enter_side_layer(void) {
	if (!build_layer()) return;
	D.warping = true;
	D.checkpoint = true;
	emu_warp_out();
}

/* A Yes in a layer's choice: a challenge battle, or into a side layer. */
static bool act_on_choices(void) {
	if (emu_read8(BN6_CHATBOX)) return false;   /* once the chat box has closed */
	for (int i = 0; i < D.objs.nchoices; ++i) {
		if ((D.chosen & (1u << i)) || !flag_get(D.objs.choice[i].flag)) continue;
		D.chosen |= 1u << i;
		switch (D.objs.choice[i].type) {
		case OBJ_CHALLENGE: {
			Encounter e = make_encounter(run.depth, run.biome, ENC_CHALLENGE);
			set_encounter(&e, true);
			D.challenge = true;
			return true;
		}
		case OBJ_UNDERNET:
		case OBJ_SECRET_GATE:
			run.side_kind = D.objs.choice[i].type == OBJ_UNDERNET ? LAYER_UNDERNET : LAYER_SECRET;
			enter_side_layer();
			return true;
		default:
			break;
		}
	}
	return false;
}

/* MegaMan stepped on the exit pad: the game plays its warp (jack out, fade,
 * jack in) to warp 1. While it jacks out, the next layer is built and warp 1
 * pointed at its start; nothing else happens until MegaMan has arrived. */
static bool follow_exit_warp(void) {
	int pending = emu_read8(BN6_WARP + 0x10);
	if (D.warping) {
		bool arrived = pending == 0 && on_map() &&
			emu_read8(BN6_GAMESTATE + 4) == D.group && emu_read8(BN6_GAMESTATE + 5) == D.number;
		if (arrived) D.warping = false;
		return !arrived;
	}
	if (pending != 1 || emu_read8(BN6_WARP + 0x11) != 1) return false;
	if (boss_beaten()) clear_card();
	/* past the Nest's guardian: the net rebuilds (the next arrival says so;
	 * counted with the next checkpoint, which a CONTINUE cannot undo) */
	if (boss_beaten() && run.biome == BIOME_NEST) D.nest_cleared = true;
	/* a side layer's exit leads one area deeper too */
	run.depth++;
	run.side_kind = LAYER_NORMAL;
	if (!build_layer()) return false;
	D.warping = true;
	D.checkpoint = true;
	return true;
}

/* --talk NAME:FRAME,...: from the layer's frame FRAME, once no chat is
 * open, the chat of its first NAME (npc shop heal programs gift challenge
 * undernet gate; intro defeat reward for its guardian; status for L), for
 * captures. */
static void dev_talks(void) {
	static unsigned done;
	if (D.frame <= 1) done = 0;
	if (!director_dev_talks || talk_busy() || emu_read8(BN6_CHATBOX)) return;
	static const struct { const char *name; int type; } kinds[] = {
		{ "npc", OBJ_NPC }, { "shop", OBJ_SHOP }, { "heal", OBJ_HEAL }, { "programs", OBJ_PROGRAMS },
		{ "gift", OBJ_GIFT }, { "challenge", OBJ_CHALLENGE }, { "undernet", OBJ_UNDERNET }, { "gate", OBJ_SECRET_GATE },
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
		else if (!strcmp(name, "status")) { talk_start(status_words(), FACE_MEGAMAN); return; }
		if (script < 0) { printf("--talk: no %s on this layer\n", name); continue; }
		game_call(BN6_CHAT_RUN_SCRIPT, D.objs.archive, (uint32_t)script);
		return;   /* (one a frame: the chat box opens on the next) */
	}
}

static void end_run(void) {
	/* what the summary tells: where, and by whom */
	const char *area = guardian_area_in_text(run.biome, run.side_kind);
	if (D.lost_to) snprintf(title_cause, sizeof title_cause, "by %s in %s", guardian(D.lost_to)->name, area);
	else snprintf(title_cause, sizeof title_cause, "in %s", area);
	/* (a first run is no record to beat) */
	title_new_best = profile.runs > 0 && run.depth > profile.best_depth;
	runlog_run_end();
	profile_record_run();
	save_delete();
	run.active = false;
	D.active = false;
	title_summary = true;
	scene_set(&scene_title);
}

void director_update(void) {
	if (!D.active) return;
	if (D.town) { town_update(); return; }
	++D.frame;
	++D.act_frames;
	/* MegaMan deleted: the game plays its GAME OVER, then the run ends */
	int mode = main_mode();
	if (mode == BN6_MODE_GAME_OVER && !D.gameover) {
		D.gameover = true;
		D.lost_to = boss_fighting() ? D.objs.guardian.navi : 0;
		boss_lost();
	}
	if (D.gameover) {
		if (mode == BN6_MODE_START_SCREEN) end_run();
		return;
	}
	if (follow_exit_warp()) return;
	map_label();   /* (once MegaMan has arrived: not over the jack-out) */
	arrow_update();
	if (on_map()) {
		/* what MegaMan has come near, for the map */
		int px = (int)emu_read32(BN6_PLAYER + 0x1C) >> 16, py = (int)emu_read32(BN6_PLAYER + 0x20) >> 16, cx, cy;
		if (netmap_panel(px, py, &cx, &cy)) {
			for (int y = cy - 4; y <= cy + 4; ++y)
				for (int x = cx - 4; x <= cx + 4; ++x)
					if (x >= 0 && y >= 0 && x < MAP_W && y < MAP_H) D.seen[y][x] = 1;
			/* a platform he stands on, all of it */
			for (int i = 0; i < layer.nrooms; ++i) {
				const Room *r = &layer.rooms[i];
				if (cx < r->x || cy < r->y || cx >= r->x + r->w || cy >= r->y + r->h) continue;
				for (int y = r->y - 1; y <= r->y + r->h; ++y)
					for (int x = r->x - 1; x <= r->x + r->w; ++x)
						if (x >= 0 && y >= 0 && x < MAP_W && y < MAP_H) D.seen[y][x] = 1;
			}
		}
	}
	if (on_map()) { unwedge(); push_arrow(); }
	cinema_on_map(on_map());
	if (!on_map()) {
		int sub = emu_read8(BN6_GAMESTATE);
		if (sub == BN6_SUB_BATTLE_INIT || sub == BN6_SUB_BATTLE) {
			emu_battle_release();   /* the forced battle has begun */
			if (!D.in_battle) {
				bool guardian = boss_fighting();
				if (!guardian && !D.challenge) ++D.battles;
				runlog_battle_start(guardian ? NULL : &D.next, guardian ? "guardian" : D.challenge ? "challenge" : "battle");
			}
			D.in_battle = true;
		}
		return;
	}
	if (D.in_battle) {
		/* back from a battle: count the deleted viruses (a navi counts below) */
		D.in_battle = false;
		bool won = emu_read8(BN6_BATTLE_RESULT) == 1;
		runlog_battle_end(won);
		if (won && !boss_fighting()) run.viruses_deleted += D.foes;
		if (!D.challenge && !boss_fighting()) roll_encounter();
	}
	/* back from the guardian's battle */
	if (boss_fighting() && !emu_battle_forcing()) boss_battle_over(emu_read8(BN6_BATTLE_RESULT) == 1);
	if (D.challenge && !emu_battle_forcing()) {
		/* back from the challenge (the game gave its reward, the signal
		 * gives its own for a win): random battles again */
		D.challenge = false;
		if (emu_read8(BN6_BATTLE_RESULT) == 1 && D.objs.challenge_reward >= 0)
			game_call(BN6_CHAT_RUN_SCRIPT, D.objs.archive, (uint32_t)D.objs.challenge_reward);
		roll_encounter();
	}
	run.fragments = key_item(SCRIPTS_SECRET_DATA);
	/* a guardian keeps the exit pad shut (the game clears the map's warp
	 * flags when it enters a map) */
	if (!boss_exit_open()) flag_set(BN6_FLAG_WARP_OFF + 1);
	else flag_clear(BN6_FLAG_WARP_OFF + 1);
	boss_update();
	/* (after the last card: the area cleared on the way here) */
	if (D.area_card && ++D.arrived >= AREA_CARD_AT && !cinema_busy()) {
		D.area_card = false;
		area_card();
	}
	/* the arrival's words once the card has gone; Chaud's call once the
	 * Secret Area's guardian is done */
	talk_update();
	if (!D.area_card && !cinema_busy() && !boss_cinematic() && !boss_fighting() && !talk_busy()) {
		if (D.beat[0] && talk_start(D.beat, FACE_MEGAMAN)) {
			if (run.biome == BIOME_NEST) cinema_shake(30, 3);
			D.beat[0] = 0;
		} else if (D.secret_call && boss_done() &&
			talk_start("@C Lan, it's Chaud. ProtoMan hasn't left my PET all day.|@C Whatever you just beat down there was a copy. Watch yourself.|"
				"@M The Nest can even copy ProtoMan...|@L Then we'd better keep our guard up!", FACE_MEGAMAN)) {
			D.secret_call = false;
		}
	}
	dev_talks();
	if (act_on_choices()) return;
	/* (never with a chat box open: a state would keep it, and the talk
	 * slot's text is not in a state) */
	/* (nor while the arrival still holds him: the jack-in and the warp pad
	 * keep him for about 90 frames, and the release is not in the state) */
	if (D.checkpoint && D.frame >= CHECKPOINT_AFTER && !talk_busy() && !emu_read8(BN6_CHATBOX) &&
		!emu_read8(BN6_DIALOGUE_LOCK) && flag_get(BN6_FLAG_PLAYER_CAN_MOVE)) {
		D.checkpoint = false;
		save_checkpoint();
		if (D.nest_cleared) { D.nest_cleared = false; profile.nest_clears++; profile_save(); }
	}
	/* on another map (a story warp the run does not use): back to the layer */
	if (emu_read8(BN6_GAMESTATE + 4) != D.group || emu_read8(BN6_GAMESTATE + 5) != D.number) {
		if (++D.astray > ASTRAY_FRAMES) {
			D.astray = 0;
			emu_warp(D.group, D.number, D.start_x, D.start_y, 4);
		}
		return;
	}
	D.astray = 0;
	/* (not over a battle that is about to start) */
	if (D.frame % REROLL_FRAMES == 0 && !emu_battle_forcing()) roll_encounter();
}
