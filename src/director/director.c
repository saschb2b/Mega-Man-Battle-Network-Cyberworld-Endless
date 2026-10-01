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
#include "chip_pool.h"
#include "bytes.h"
#include "cinema.h"
#include "data.h"
#include "emu.h"
#include "encounter.h"
#include "debug.h"
#include "devtools.h"
#include "flags.h"
#include "game.h"
#include "gamecall.h"
#include "gfx.h"
#include "guardians.h"
#include "layer_objs.h"
#include "layer_make.h"
#include "mapslot.h"
#include "loot.h"
#include "net.h"
#include "navicust.h"
#include "net_route.h"
#include "pacing.h"
#include "netmap.h"
#include "rom.h"
#include "platform.h"
#include "pet.h"
#include "pet_text.h"
#include "powers.h"
#include "rivals.h"
#include "run.h"
#include "runlog.h"
#include "meta.h"
#include "save.h"
#include "save_blob.h"
#include "scripts.h"
#include "shop.h"
#include "talk.h"
#include "trader.h"
#include "text.h"
#include "town.h"
#include "xnavi.h"
#include "xsong.h"

int director_debug_biome = -1;
int director_debug_area = -1;
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
	bool reward_due;       /* ... and won: its prize is told (and given) once a talk can start */
	bool gate_fight;       /* the challenge is a Navi gate's SP (docs/META.md, gates) */
	/* the rival's duel (docs/RIVAL.md): the layer's squad, the duel under
	 * way (MegaMan's HP as last read, a hit taken, the DeleteTime), and
	 * Chaud's words due: his call as the layer begins, his verdict after */
	Encounter duel_enc;
	bool duel, duel_hit, duel_call_due, duel_verdict_due;
	int duel_hp, duel_time;
	int duel_cap;       /* the netbattle's ProtoMan at most this HP (half the act's guardian band's top), 0 none or done */
	bool lost_duel;     /* MegaMan deleted in the rival's duel (the summary says so) */
	char duel_verdict[400];
	bool gate_due;         /* ... and won: his SP chip is given once a talk can start */
	int fragments_seen;    /* ScrtData held last frame: one more, and MegaMan says what it is for */
	bool fragment_due;     /* ... once a talk can start */
	unsigned spins_seen;   /* the Spins in the game's key items last frame (a bit per colour) */
	bool spin_due;         /* the run's Spin picked up: MegaMan says what it does once a talk can start */
	int fragments_told;    /* ScrtData L's briefing (or MegaMan at one) last counted */
	bool in_battle;        /* a battle is on */
	bool record_known;     /* the battle's record (D.rolled) is known */
	bool placed_told;      /* (debug) MegaMan's first panel in it was printed */
	int foes;              /* viruses in the battle the game will start next */
	Encounter next;        /* that battle */
	Encounter rolled[2];   /* the battles in the two records the roll hands out (encounter.c) */
	int battles;           /* random battles fought on this layer */
	int astray;            /* frames MegaMan has spent on another map */
	bool warping;          /* the exit pad's warp is under way */
	bool area_card;        /* show the area's title card once MegaMan is in */
	bool arrival_hold;     /* MegaMan held for the arrival's cards and the words after them */
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
	bool chat_was_open;            /* the game's chat box was open last frame */
	int a_quiet;                   /* frames an A is not passed on after a chat on the map closed */
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
	int layer_act;         /* 1 + the act of the layer built last, 0 none (a side layer) */
	int dealer_act;        /* 1 + the act whose Net Dealer has already spoken, 0 none (kept across a CONTINUE: act_note) */
	int heard_act;         /* 1 + the act whose guardian a bystander has named this session, 0 none */
	bool gem_due;          /* a battle with a Mystery Data on its field is over: MegaMan says what it is (once) */
	int mail_due;          /* a guardian whose battle data Dad has just mailed (the PET's E-Mail), 0 none */
	bool mail_quiet;       /* the session's first mails come without a word (a run's start brings every guardian's) */
	bool pet_refreshed;    /* the layer's PET words, items and mail made (once on the map: a warp's frames go by unseen) */
	bool checkpoint_data;  /* the checkpoint due is the Guardian Data's ... */
	bool checkpoint_here;  /* ... or the PET's Save's, where MegaMan stands */
	const char *saved_at;  /* where the run was last saved, for the quit prompt */
	bool beat_guardian;    /* the arrival's words (beat) name the act's guardian ... */
	bool guardian_named;   /* ... and have been said on this layer */
	int lost_to;           /* the guardian MegaMan was deleted by, 0 none */
	bool nest_cleared;     /* the Nest's guardian fell; the profile counts it at the checkpoint */
	uint8_t bugs[NAVICUST_BUGS];   /* the NaviCust's bug counts MegaMan last spoke of */
	bool bugs_known;       /* ... read on this layer */
	bool pet_seen;         /* the PET's menus were open since the map was last quiet */
	bool off_told;         /* ... and MegaMan has said, on this layer, that a program is off the board */
	bool last_stop_told;   /* ... and named the Net Dealer and the heal before the guardian's arena */
	bool final_told;       /* ... and, the short net's last guardian fallen, said the run is won */
} D;

#define AREA_CARD_AT 45   /* frames on the map after arriving */
#define CARD_SKIP    30   /* frames a card shows before A ends it, where MegaMan is held for it */
#define PORT_STEP    16   /* world units from a jack-in cell that R steps onto it */
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
#define PET_LABEL_AT (EMU_FREE + 0x153000)   /* the PET's PLACE: the area and the layer (docs/EMULATION.md) */

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
	if (D.town) snprintf(name, sizeof name, "%s", town_info()->name ? town_info()->name : "Town");
	else if (run.side_kind == LAYER_UNDERNET) snprintf(name, sizeof name, "Undernet");
	else if (run.side_kind == LAYER_SECRET) snprintf(name, sizeof name, "Secret Area");
	else if (run.biome == BIOME_NEST) snprintf(name, sizeof name, "Cybeast Nest");
	else snprintf(name, sizeof name, "Layer %d", run.depth);
	/* the PET's PLACE says where, beside the layer ("ACDC HP 8"): a
	 * label of its own, the map's entry keeping "Layer 8" */
	char place[16];
	if (D.town || run.side_kind != LAYER_NORMAL || run.biome == BIOME_NEST) snprintf(place, sizeof place, "%s", name);
	else snprintf(place, sizeof place, "%s %d", guardian_area_short(run.biome), run.depth);
	static char last_place[16];
	if (!strcmp(name, last) && !strcmp(place, last_place) && emu_read32(BN6_MAP_NAMES_PTR) == LABEL_AT &&
		emu_read32(BN6_PET_MAP_NAMES_PTR) == PET_LABEL_AT) return;
	snprintf(last, sizeof last, "%s", name);
	snprintf(last_place, sizeof last_place, "%s", place);
	/* (right-aligned with spaces: the game pads its own with underscores,
	 * which show) */
	static uint8_t a[LABEL_SIZE];
	int len = label_archive(name, a, sizeof a);
	if (!len) return;
	emu_write(LABEL_AT, a, (size_t)len);
	emu_write32(BN6_MAP_NAMES_PTR, LABEL_AT);
	int plen = label_archive(place, a, sizeof a);
	if (plen) {
		emu_write(PET_LABEL_AT, a, (size_t)plen);
		emu_write32(BN6_PET_MAP_NAMES_PTR, PET_LABEL_AT);
	} else emu_write32(BN6_PET_MAP_NAMES_PTR, LABEL_AT);
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
	D.beat_guardian = D.guardian_named = false;
	if (run.side_kind == LAYER_UNDERNET)
		snprintf(D.beat, sizeof D.beat, "@M A copy of the Undernet...|@M The viruses in here are no joke, Lan.|@L Stay sharp. The exit pad leads back to the main path.");
	else if (run.side_kind == LAYER_SECRET)
		snprintf(D.beat, sizeof D.beat, "@M The gate opened, Lan... This must be the Secret Area.|@M Something strong is waiting in here. I can feel it.");
	else if (run.depth == 1 && profile.runs >= 2)
		snprintf(D.beat, sizeof D.beat, "@M %s this time, Lan. Let's find the exit pad!", area);
	else if (run.depth == 1)
		snprintf(D.beat, sizeof D.beat, "@M Lan, it looks just like %s... But it's all copied data!|@L Dad was right. Let's find the exit pad and head down!", area);
	else if (first_of_act && (run.depth - 1) % CYCLE_LAYERS == 0)
		snprintf(D.beat, sizeof D.beat, "@D Lan! The Endless Net just rebuilt itself, all of it!|@D The same areas, but stronger data. It's Net V%d now!|@M Then we keep going, Lan!", net_version());
	else if (run.biome == BIOME_NEST && run_short_nest(run.depth) && !run_short_last(run.depth))
		/* (threat 10: said before the first of the two, docs/META.md) */
		snprintf(D.beat, sizeof D.beat, "@M Lan... This is it. The Nest. Something down here is copying everything.|@B Grrrr...|"
			"@M And it's not alone. I can feel a second guardian further down.|@L Then we beat them both, MegaMan!");
	else if (run.biome == BIOME_NEST && run_short_nest(run.depth) && run.depth > SHORT_LAYERS)
		snprintf(D.beat, sizeof D.beat, "@M Below the Nest... The second guardian's here, Lan.|@B Grrrr...|@L The last one, MegaMan. Let's finish this!");
	else if (run.biome == BIOME_NEST)
		snprintf(D.beat, sizeof D.beat, "@M Lan... This is it. The Nest. Something down here is copying everything.|@B Grrrr...|@L Hang on, MegaMan! Whatever it is, we'll find it!");
	else if (first_of_act && run.biome == BIOME_UNDERNET)
		snprintf(D.beat, sizeof D.beat, "@M Even the Undernet got copied... Stay sharp, Lan.");
	else if (first_of_act && run.biome == BIOME_GRAVEYARD)
		snprintf(D.beat, sizeof D.beat, "@M So much deleted data... Lan, I think the bottom is close.");
	else if (first_of_act && run.depth > 1) {
		/* a new act: where they are now, and whose copy waits at its end,
		 * named where they have battled him: else a signal MegaMan does
		 * not know (docs/META.md, what MegaMan knows) */
		int navi = run.boss_order[run.biome];
		if (guardian_known(navi))
			snprintf(D.beat, sizeof D.beat, "@M We're through to %s, Lan!|@L %s's copy guards this one. Let's go!", area, guardian(navi)->name);
		else
			snprintf(D.beat, sizeof D.beat, "@M We're through to %s, Lan! A strong Navi's signal waits at its end, one "
				"we've never faced down here.|@L Then let's find out who. Let's go!", area);
		D.beat_guardian = true;
	}
}

/* What MegaMan says when L is pressed: where they are, what is ahead. */
/* Which way the exit pad lies from MegaMan, as the screen shows it (the
 * d-pad's UP moves +X -Y, RIGHT +X +Y: a world step (dx, dy) goes
 * dx + dy across and (dy - dx) / 2 down), and how far. */
static int way_dir;   /* the index of the last way_to: 0 right, then clockwise */
static bool map_used;   /* the layer's map has been held (SELECT) since the game started */
static const char *const ways[8] = {
	"to the right", "down and to the right", "straight down", "down and to the left",
	"to the left", "up and to the left", "straight up", "up and to the right",
};

static const char *way_to(int tx, int ty, int *far) {
	int px = (int)emu_read32(BN6_PLAYER + 0x1C) >> 16, py = (int)emu_read32(BN6_PLAYER + 0x20) >> 16;
	int dx = tx - px, dy = ty - py;
	int panels = (abs(dx) + abs(dy)) / 32;
	*far = panels < 5 ? 0 : panels < 14 ? 1 : 2;
	/* (on the grid, whose +x is the world's +Y and +y its -X: the pad's
	 * ways, net_route.c) */
	way_dir = route_grid_way(dy / 32.0, -dx / 32.0);
	return ways[way_dir];
}

/* ProtoMan's mark on the layer's map, and its key's */
#define RIVAL_MARK { 255, 96, 176, 255 }

/* ProtoMan, while his duel waits on this layer (not taken, not a netbattle
 * named for later): where he stands in the world. */
static bool duel_waiting(int *wx, int *wy) {
	if (layer_objs_duel_later) return false;
	for (int i = 0; i < D.objs.nchoices; ++i)
		if (D.objs.choice[i].type == OBJ_DUEL && (D.chosen & (1u << i))) return false;
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].type == OBJ_DUEL) { netmap_world((int)layer.obj[i].x, (int)layer.obj[i].y, wx, wy); return true; }
	return false;
}

/* The town's way to the port on foot (town_walk): the first stretch of the
 * walk, not the line to it, which led a playtester into a house front
 * ("straight up") on two runs; how far that walk is. */
static const char *town_way(int *far) {
	int px = (int)emu_read32(BN6_PLAYER + 0x1C) >> 16, py = (int)emu_read32(BN6_PLAYER + 0x20) >> 16;
	int wx, wy, cells;
	if (!town_walk(px, py, 8, &wx, &wy, &cells)) return way_to(town_info()->port_x, town_info()->port_y, far);
	const char *way = way_to(wx, wy, far);
	int panels = cells / 4;   /* (a panel is 32 units, a cell 8) */
	*far = panels < 5 ? 0 : panels < 14 ? 1 : 2;
	return way;
}

/* The way on along the floor, not as the crow flies (net_route.c), and
 * how far that walk is. NULL when either end is off the floor. */
static const char *route_to(int tx, int ty, int *far) {
	int px = (int)emu_read32(BN6_PLAYER + 0x1C) >> 16, py = (int)emu_read32(BN6_PLAYER + 0x20) >> 16;
	double gx, gy;
	int ex, ey, len;
	netmap_grid(px, py, &gx, &gy);
	if (!netmap_panel(tx, ty, &ex, &ey)) return NULL;
	int w = route_way(gx, gy, ex, ey, &len);
	if (w < 0) return NULL;
	*far = len < 5 ? 0 : len < 14 ? 1 : 2;
	way_dir = w;
	return ways[w];
}

/* Where something lies from MegaMan, as L and the map's mark give it,
 * with how far the walk there is (*far) and whether that walk sets off
 * another way (*winds); way_dir left as it was. */
static const char *lie_and_walk(int wx, int wy, int *far, bool *winds) {
	int keep = way_dir, walk_far;
	const char *lies = way_to(wx, wy, far);
	int lies_dir = way_dir;
	const char *walk = route_to(wx, wy, &walk_far);
	int apart = walk ? abs(way_dir - lies_dir) : 0;
	if (apart > 4) apart = 8 - apart;
	if (walk) *far = walk_far;
	*winds = apart >= 2;
	way_dir = keep;
	return lies;
}

/* Where ProtoMan waits, for L's words: the lie of his pink mark, how far
 * the walk to him is, and whether it winds (as the crow flies, "close by"
 * named him across a gap); NULL while no duel waits. */
static const char *rival_where(void) {
	static const char *const dist[3] = { "close by", "a ways off", "far off" };
	static char buf[96];
	int wx, wy, far;
	bool winds;
	if (!duel_waiting(&wx, &wy)) return NULL;
	const char *lies = lie_and_walk(wx, wy, &far, &winds);
	snprintf(buf, sizeof buf, "%s, %s%s", lies, dist[far], winds ? ", though the way there winds" : "");
	return buf;
}

/* MegaMan below three quarters of his HP (at 220 of 240 the heal led L's
 * words before the way on) */
static bool hurt_now(void) { return emu_read16(BN6_NAVI_STATS + 0x40) * 4 < emu_read16(BN6_NAVI_STATS + 0x42) * 3; }

/* Where the layer's Recovery Mr. Prog stands, in the world; false for
 * none. */
static bool heal_spot(int *wx, int *wy) {
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].type == OBJ_HEAL) { netmap_world((int)layer.obj[i].x, (int)layer.obj[i].y, wx, wy); return true; }
	return false;
}

/* The way on as the arrow shows it: along the floor to the exit or the
 * guardian (the port in the town); way_dir holds it. While MegaMan is
 * hurt, to the layer's Recovery Mr. Prog first, as L says (a playtester
 * at 180 HP heard which way it was, and the arrow led to the exit). */
static void goal_way(void) {
	int far;
	if (D.town) { town_way(&far); return; }
	int gx = D.objs.exit_x, gy = D.objs.exit_y;
	if (D.objs.guardian.navi && !boss_beaten()) { gx = D.objs.guardian.x; gy = D.objs.guardian.y; }
	if (hurt_now()) heal_spot(&gx, &gy);
	if (!route_to(gx, gy, &far)) way_to(gx, gy, &far);
}

/* A program MegaMan has that is not on the NaviCust's board (the key
 * items count a program whether placed or not; the board's list holds the
 * placed ones): its name, "" for one outside the draft's pool, NULL for
 * none. A playtester played two acts believing a Guardian Data's UnderSht
 * was running. */
static bool fits_beside_placed(int v);
static int key_item(int id);
static void spins_sync(void);

/* A program left off the board that cannot fit beside those on it: said
 * once a board size (the next grows it), not on every layer; the words, or
 * NULL when said already. */
static int no_room_told = -1;   /* the board size it was said for (a new run or a CONTINUE forgets) */

static const char *no_room_words(const char *name) {
	static char words[200];
	int board = key_item(SCRIPTS_EXP_MEMORY);
	if (no_room_told == board) return NULL;
	no_room_told = board;
	if (board < 2)
		snprintf(words, sizeof words, "@M %s won't fit beside the programs on our board yet, Lan. It'll keep in the PET until the board grows.", name);
	else snprintf(words, sizeof words, "@M %s won't fit beside the programs on our board, Lan. To use it, we'd have to take another off.", name);
	return words;
}

static const char *program_off_board(int *variant) {
	static char name[16];
	uint32_t items = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_KEY_ITEMS);
	*variant = 0;
	for (int v = 4; v < 47 * 4; ++v) {
		int owned = emu_read8(items + BN6_PROGRAM_ITEMS + (uint32_t)v), placed = 0;
		if (!owned) continue;
		for (int e = 0; e < BN6_NAVICUST_PLACED_MAX; ++e) {
			int id = emu_read16(BN6_NAVICUST_PLACED + (uint32_t)e * 8);
			if (!id) break;
			placed += id == v;
		}
		if (owned <= placed) continue;
		*variant = v;
		const char *about = navicust_about(v / 4);
		const char *colon = about ? strchr(about, ':') : NULL;
		snprintf(name, sizeof name, "%.*s", colon ? (int)(colon - about) : 0, colon ? about : "");
		return name;
	}
	return NULL;
}

/* A Navi on the net has named the act's guardian this session: the first
 * bystander on its first layer, or the Net Dealer's word from its second
 * (docs/META.md, what MegaMan knows). */
static bool guardian_heard(void) {
	return D.layer_act && (D.heard_act == D.layer_act || D.dealer_act == D.layer_act ||
		(flag_get(LAYER_DEALER_TOLD_FLAG) && layer_objs_dealer_named));
}

bool director_guardian_heard(void) { return guardian_heard(); }

/* The run is won where MegaMan stands: the short net's last guardian
 * deleted, only the exit ahead (no reminders then: a playtester was told
 * to place a program on the walk to it). */
static bool run_won_here(void) {
	return run.side_kind == LAYER_NORMAL && run.mode == RUN_SHORT && run_short_last(run.depth) && boss_done();
}

/* MegaMan's words on the layer's guardian, on arriving: who he is and how
 * he fights once they have fought him, in any run: before that MegaMan has
 * no battle data on the copy, only a strong signal, and naming him or
 * reciting his moves would spend the first fight's discovery (and how
 * could he know?); what he always knows is the net's own grammar, the
 * yellow panels that light where an attack will land. Appended to `buf`
 * at `k`; the new length. */
static int guardian_words(char *buf, int k, int size) {
	#define ADD(...) (k += snprintf(buf + k, k < size ? (size_t)(size - k) : 0, __VA_ARGS__))
	int navi = D.objs.guardian.navi;
	const char *tip = guardian_tip(navi);
	if (guardian_known(navi)) {
		ADD(" %s waits at its end!|", guardian(navi)->name);
		if (tip) ADD("@M We've got battle data on him from before:|@M %s|", tip);
		else ADD("@M Watch the yellow panels: they light where an attack will land!|");
	} else {
		/* (what a Navi on the net said, as hearsay) */
		if (guardian_heard()) ADD(" %s waits at its end, if the word on the net is right.|@M We've got no battle data on him, Lan.",
			guardian(navi)->name);
		else ADD(" A strong Navi's signal waits at its end, one we've never faced down here.|@M We've got no battle data on it, Lan.");
		ADD(" Watch the yellow panels: they light where an attack will land!|");
	}
	/* (EraseCross on a Navi, which the setup has no room for: a playtester
	 * saw BlastMan's HP drain after a Vulcan, and only patch notes had
	 * said why) */
	if (flag_get(BN6_FLAG_ERASE_CROSS))
		ADD("@M And in EraseCross, a plain chip that hits him while his HP has a 4 in it bugs him: his HP drains away!|");
	#undef ADD
	return k;
}

/* L's word on a heal while MegaMan is hurt, appended to `buf` at `k`; the
 * new length: which way the Recovery Mr. Prog is, the arrow's way while he
 * is hurt (goal_way), else the Net Dealer, who always has a MiniEnrg (it
 * was on no map yet, and never found: a playtester at 90 of 240 ran to the
 * exit past the dealer, then a dozen moves back). */
static int heal_note(char *buf, int k, int size, bool heal) {
	static const char *const near_far[3] = { "close by", "a ways off", "far off" };
	for (int i = 0; i < layer.nobj; ++i) {
		if (layer.obj[i].type != (heal ? OBJ_HEAL : OBJ_SHOP)) continue;
		int wx, wy, hf;
		netmap_world((int)layer.obj[i].x, (int)layer.obj[i].y, &wx, &wy);
		/* (where it lies: the way's first leg pointed off from it) */
		const char *hw = way_to(wx, wy, &hf);
		k += snprintf(buf + k, k < size ? (size_t)(size - k) : 0,
			heal ? "@M The Recovery Mr. Prog can patch us up. It's %s, %s: the arrow leads there first.|"
			     : "@M The Net Dealer has MiniEnrg to patch us up. He's %s, %s.|", hw, near_far[hf]);
		break;
	}
	return k;
}

/* The area's viruses that fight in ways BN6 never explains, where the act
 * or a side layer begins, appended to `buf` at `k`; the new length. Only
 * once they have been battled, in any run: the first meeting is theirs to
 * show. (A playtester's Thunder healed a ScarCrow to full, two DarkMechs
 * took 460 HP before he knew, and a StarFish's bubbles ate three Cannons,
 * a WideSht and a Navi chip's fire while it took 350.) */
static int family_words(char *buf, int k, int size) {
	static const struct { int family; const char *words; } warn[] = {
		{ FAMILY_SCARCROW, "@M ScarCrows here again: they call down lightning to heal, and Elec chips heal them too! Hit them hard, with anything but Elec.|" },
		{ FAMILY_DARKMECH, "@M DarkMechs here again. They warp right beside us to slash, so keep moving and strike as they appear!|" },
		{ FAMILY_STARFISH, "@M StarFish here again: their bubbles soak up our shots and trap us if we touch one. "
			"A chip that drops from above gets past them!|" },
	};
	uint32_t fams = loot_families_here(run.depth, run.biome);
	for (unsigned i = 0; i < sizeof warn / sizeof *warn; ++i)
		if (fams & (1u << warn[i].family) && profile_family_fought(warn[i].family))
			k += snprintf(buf + k, k < size ? (size_t)(size - k) : 0, "%s", warn[i].words);
	return k;
}

/* What MegaMan senses on the layer (`here`, n of them), appended to `buf`
 * at `k`, then the rival, whose call has said why (docs/RIVAL.md), apart:
 * in the list, "ProtoMan, waiting for our duel and an official gate" read
 * as waiting for the gate too (a playtester's). The new length. */
static int sense_words(char *buf, int k, int size, const char *const *here, int n, bool duel) {
	#define ADD(...) (k += snprintf(buf + k, k < size ? (size_t)(size - k) : 0, __VA_ARGS__))
	if (n) {
		ADD("@M I sense");
		for (int i = 0; i < n; ++i) ADD("%s %s", i == 0 ? "" : i == n - 1 ? " and" : ",", here[i]);
		ADD(" here!|");
	}
	if (duel) ADD(n ? "@M And ProtoMan's waiting for our duel!|" : "@M ProtoMan's waiting for our duel here!|");
	#undef ADD
	return k;
}

/* L's word on a program left off the board, appended to `buf` at `k`; the
 * new length. Said on every layer until placed, unless MegaMan has said it
 * on this layer already (a playtester heard it on CONTINUE, then again in
 * L's first words); one that cannot fit, once a board (a playtester's
 * SuprArmr could not share the 4x4 board with Custom1). */
static int off_board_note(char *buf, int k, int size) {
	#define ADD(...) (k += snprintf(buf + k, k < size ? (size_t)(size - k) : 0, __VA_ARGS__))
	int offv;
	const char *off = run_won_here() || D.off_told ? NULL : program_off_board(&offv);
	if (off && !fits_beside_placed(offv)) {
		const char *w = no_room_words(*off ? off : "That program");
		if (w) ADD("%s|", w);
	} else if (off && *off) ADD("@M Lan, %s isn't on our NaviCust's board yet! PET: MegaMan, then NaviCust. %s|", off, navicust_turn_words(offv));
	else if (off) ADD("@M Lan, a program isn't on our NaviCust's board yet! PET: MegaMan, then NaviCust.|");
	#undef ADD
	return k;
}

static const char *status_words(void) {
	static char buf[800];
	int k = 0;
	#define ADD(...) (k += snprintf(buf + k, k < (int)sizeof buf ? sizeof buf - (size_t)k : 0, __VA_ARGS__))
	if (D.town) {
		int far;
		const char *way = town_way(&far);
		/* all of it the first time, then only the way (a box each) */
		if (!D.port_told) ADD("@M The port's %s, by the %s!|@M Stand next to it and press R to jack me in!", way, town_info()->landmark);
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
		if (D.objs.guardian.navi && !boss_beaten()) k = guardian_words(buf, k, (int)sizeof buf);
		/* (not after the act's arrival words, which spoke of him; a
		 * CONTINUE does not say them again, and there he is spoken of) */
		else if (!D.objs.guardian.navi && run.side_kind == LAYER_NORMAL && !D.guardian_named) {
			int navi = run.boss_order[run.biome];
			if (guardian_known(navi)) ADD(" %s guards the end of it.|", guardian(navi)->name);
			else if (guardian_heard()) ADD(" %s guards the end of it, word is.|", guardian(navi)->name);
			else ADD(" A strong Navi's signal waits at its end, one we've never faced down here.|");
		}
		else ADD("|");
		/* (the area's battlefields, on its first layer: a playtester froze
		 * on the Aquarium's ice, 140 to 80 HP, and nothing had said so) */
		if (run.biome == BIOME_AQUARIUM_COMP && run.side_kind == LAYER_NORMAL && layer_in_act(run.depth) == 0)
			ADD("@M The battlefields here are icy. An Aqua hit on ice freezes us, so keep off it when the viruses shoot water!|");
		/* (and a homepage's conveyors and ice: a conveyor carried a
		 * playtester off the row he stepped into, every time, and the ice
		 * froze him twice in the next act) */
		if (run.biome == BIOME_HOMEPAGE && run.side_kind == LAYER_NORMAL && layer_in_act(run.depth) == 0)
			ADD("@M The battlefields here have conveyor and ice panels. The arrows carry us along, and an Aqua hit on ice freezes us. Mind where we stand!|");
		if (run.side_kind != LAYER_NORMAL || layer_in_act(run.depth) == 0) k = family_words(buf, k, (int)sizeof buf);
		/* (what they are for: a playtester carried two and never learned;
		 * once per count, the next heard it on every layer) */
		if (run.fragments != D.fragments_told) {
			if (run.fragments == 1) ADD("@M We're carrying one ScrtData. Three open the golden gate to the Secret Area!|");
			else if (run.fragments > 1) ADD("@M We're carrying %d ScrtData. Three open the golden gate to the Secret Area!|", run.fragments);
			D.fragments_told = run.fragments;
		}
	}
	/* the services here: all of them the first time, then only the heal
	 * while he is hurt (it heals every time; the map's key names the rest,
	 * and a later L is a box or two, not the briefing again) */
	bool shop = false, heal = false, programs = false, trader = false, bugtrader = false, told = D.layer_told;
	bool challenge = false, warp = false, gate = false, navi_gate = false, vault = false, duel = false, official = false;
	for (int i = 0; i < layer.nobj; ++i) {
		duel |= layer.obj[i].type == OBJ_DUEL && !layer_objs_duel_later;
		official |= layer.obj[i].type == OBJ_OFFICIAL;
		shop |= layer.obj[i].type == OBJ_SHOP;
		heal |= layer.obj[i].type == OBJ_HEAL;
		programs |= layer.obj[i].type == OBJ_PROGRAMS;
		trader |= layer.obj[i].type == OBJ_TRADER;
		bugtrader |= layer.obj[i].type == OBJ_BUGTRADER;
		challenge |= layer.obj[i].type == OBJ_CHALLENGE;
		warp |= layer.obj[i].type == OBJ_UNDERNET;
		gate |= layer.obj[i].type == OBJ_SECRET_GATE;
		navi_gate |= layer.obj[i].type == OBJ_NAVI_GATE;
		vault |= layer.obj[i].type == OBJ_VAULT;
	}
	bool hurt = hurt_now();
	if (!told) {
		/* what is here, in one breath (a playtester paged eight boxes on
		 * arriving in act 3): the services (the map marks a trader as a
		 * shop, and L had said nothing of one), then the map's violet mark,
		 * in full until it has been explained (a playtester stood beside one
		 * and never found out what it was), named after that */
		const char *here[8];
		int n = 0;
		if (shop) here[n++] = "a Net Dealer";
		if (heal) here[n++] = "a Recovery Mr. Prog";
		if (programs) here[n++] = "a NaviCust program shop";
		if (trader) here[n++] = "a Chip Trader";
		if (bugtrader) here[n++] = "a BugFrag Trader";
		int marks = (challenge ? MARK_SERVER : 0) | (warp ? MARK_WARP : 0) | (gate ? MARK_GATE : 0) | (navi_gate ? MARK_NAVI_GATE : 0) |
			(vault ? MARK_VAULT : 0);
		int known = marks & profile.marks_taught, fresh = marks & ~profile.marks_taught;
		if (known & MARK_SERVER) here[n++] = "a strong virus signal";
		if (known & MARK_WARP) here[n++] = "a dark warp";
		if (known & MARK_GATE) here[n++] = "the golden gate";
		static char sealed[48];
		if (known & MARK_NAVI_GATE) { snprintf(sealed, sizeof sealed, "a gate with %s's code", guardian(D.objs.gate_navi)->name); here[n++] = sealed; }
		if (known & MARK_VAULT) here[n++] = "a collector's vault";
		if (official && n < 8) here[n++] = "an official gate";
		k = sense_words(buf, k, (int)sizeof buf, here, n, duel);
		if (fresh & MARK_SERVER) ADD("@M A strong virus signal, the violet mark on the map! Its Server offers a hard battle for a good chip.|");
		if (fresh & MARK_WARP) ADD("@M A dark warp into the Undernet, the violet mark on the map! Tougher viruses in there, and richer data.|");
		if (fresh & MARK_GATE) ADD("@M The golden gate to the Secret Area, the violet mark on the map!|");
		if (fresh & MARK_NAVI_GATE)
			ADD("@M A gate sealed with %s's code, the violet mark on the map! His code opens it for good, and his SP waits inside.|",
				guardian(D.objs.gate_navi)->name);
		if (fresh & MARK_VAULT)
			ADD("@M A collector's vault, the violet mark on the map! A big enough Library opens it, and it holds rare chips.|");
		if (fresh) { profile.marks_taught |= (uint8_t)fresh; profile_save(); }
		/* (where the rival waits, and his mark: the map showed him as the
		 * official gate's violet, and a playtester's session ran out at
		 * the gate, alone, looking for him) */
		const char *rw = rival_where();
		if (rw) ADD("@M ProtoMan's %s: the pink mark on the map.|", rw);
		k = off_board_note(buf, k, (int)sizeof buf);
		/* (the map's tip on the run's first layers, until the map has been
		 * held: a playtester who used it heard it again every run) */
		if (run.depth <= 2 && !map_used) ADD("@M Hold SELECT to see the map of where we've been.|");
		D.layer_told = true;
		flag_set(LAYER_TOLD_FLAG);
	}
	if (hurt && (heal || shop)) k = heal_note(buf, k, (int)sizeof buf, heal);
	/* (and after that, where ProtoMan waits, while he does) */
	const char *rival = told ? rival_where() : NULL;
	if (rival) ADD("@M ProtoMan's waiting %s.|", rival);
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
	/* (the arrow leading to the heal: the exit's words without it) */
	int apart = hurt && heal ? 0 : abs(way_dir - lies_dir);
	if (apart > 4) apart = 8 - apart;
	static const char *const how_far[3] = { "It's close!", "It's a ways off.", "It's a long way yet." };
	if (D.objs.guardian.navi && !boss_done() && boss_beaten()) ADD("@M Let's take its Guardian Data, Lan!");
	/* (the words where it lies, the arrow the walk: say they part) */
	else if (apart >= 2 && told) ADD("@M The %s %s, but the way winds. Follow the arrow!", to_guardian ? "guardian waits" : "exit lies", lies);
	else if (apart >= 2)
		ADD("@M The %s %s.|@M %s The way winds, so follow the arrow!", to_guardian ? "guardian waits" : "exit lies", lies, how_far[far]);
	else ADD("@M The way on goes %s. %s", way, how_far[far]);
	#undef ADD
	goal_way();   /* (the arrow's way, which the words that follow start) */
	return buf;
}

static void area_card(void) {
	char act[32];
	int biome = run.biome, act_no = ((run.depth - 1) % CYCLE_LAYERS) / 3 + 1;
	if (run.side_kind == LAYER_UNDERNET) snprintf(act, sizeof act, "Through a dark warp");
	else if (run.side_kind == LAYER_SECRET) snprintf(act, sizeof act, "Beyond the sealed gate");
	else if (biome == BIOME_NEST && run_short_nest(run.depth) && run.depth > SHORT_LAYERS) snprintf(act, sizeof act, "Below the Nest");
	else if (biome == BIOME_NEST && net_version() > 1) snprintf(act, sizeof act, "The bottom of Net V%d", net_version());
	else if (biome == BIOME_NEST) snprintf(act, sizeof act, "The bottom of the net");
	else if (net_version() > 1) snprintf(act, sizeof act, "Net V%d - Act %d", net_version(), act_no);
	else snprintf(act, sizeof act, "Act %d", act_no);
	/* the guardian ahead, from the start, so the folder can be set for it
	 * (as Slay the Spire shows each act's boss), where MegaMan has battled
	 * him: else "???", as nothing yet says who (docs/META.md, what MegaMan
	 * knows; his element alone gave SpoutMan away) */
	char ahead[48] = "";
	if (run.side_kind == LAYER_NORMAL || (run.side_kind == LAYER_SECRET && layer.boss_layer)) {
		int navi = run.boss_order[biome];
		snprintf(ahead, sizeof ahead, "Guardian: %s", guardian_known(navi) ? guardian(navi)->name : "???");
	}
	/* (a CONTINUE by a guardian already deleted: it said he waited) */
	if (D.objs.guardian.navi && boss_beaten())
		snprintf(ahead, sizeof ahead, "%s deleted", guardian(D.objs.guardian.navi)->name);
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
		fprintf(stderr, "encounter field %02x player %02x:", e->field, e->player);
		for (int i = 0; i < e->nfoes; ++i) fprintf(stderr, " %d/%d/%d@%d,%d", e->foes[i].kind, e->foes[i].family, e->foes[i].version, e->foes[i].col, e->foes[i].row);
		fprintf(stderr, "\n");
	}
	if (force) emu_battle_force(e);
	else emu_encounter_set(e);
	D.rolled[emu_encounter_slot()] = *e;
}

#define CHECKPOINT_AFTER  60     /* frames after a layer is entered */
#define ASTRAY_FRAMES     90     /* on another map this long: warp back */

static int main_mode(void) { return emu_read8(emu_read32(BN6_TOOLKIT)); }
/* walking the net: the game mode on its map sub-mode (not a battle or menu) */
static bool on_map(void) { return main_mode() == BN6_MODE_GAME && emu_read8(BN6_GAMESTATE) == BN6_SUB_MAP; }

/* What the drawing reads of the game, taken after each frame as the update
 * runs: on a 3DS the next frame runs on another core while this one is
 * drawn, and a read of the game then waits for it (the bottom screen's map
 * took 14 ms so, a frame lost six times a second; the duel's clock every
 * frame of its battle). */
static struct { int px, py; bool on_map, battle; uint32_t timer; int tent_x, tent_y; } seen;

/* CircusMan's tent: as his object's action turns to it (+0x09, 0x0C), BN6
 * lights the panel MegaMan stands on for a few frames, under his feet,
 * and drops the tent there; a step off it begun within 32 frames clears
 * it, and 36 frames in he is held (measured in god mode, a step at each
 * delay: the fade the briefing named came 6 frames before the hold, and a
 * playtester lost to him five times). The panel is marked over the picture
 * for those 36 frames of the battle's clock (it holds on the Custom
 * screen). */
#define TENT_ACTION 0x0C
#define TENT_FRAMES 36
static void see_tent(void) {
	static bool on;
	static uint32_t start;
	static int tx, ty;
	seen.tent_x = seen.tent_y = 0;
	if (!seen.battle || !boss_fighting() || D.objs.guardian.navi != 14 /* (CircusMan) */) { on = false; return; }
	int act = -1, mx = 0, my = 0, most = 0;
	for (uint32_t i = 0; i < BN6_T1_COUNT; ++i) {
		uint32_t o = BN6_T1_OBJECTS + i * BN6_T1_SIZE;
		if (!(emu_read8(o) & 1)) continue;
		/* (MegaMan's panel, and the side's biggest: CircusMan, not his lion) */
		if (emu_read8(o + 0x16) == 0 && emu_read8(o + 0x12)) { mx = emu_read8(o + 0x12); my = emu_read8(o + 0x13); }
		else if (emu_read8(o + 0x16) == 1 && emu_read16(o + 0x26) > most) { most = emu_read16(o + 0x26); act = emu_read8(o + 0x09); }
	}
	if (act != TENT_ACTION) { on = false; return; }
	if (!on) { on = true; start = seen.timer; tx = mx; ty = my; }
	if (seen.timer - start < TENT_FRAMES) { seen.tent_x = tx; seen.tent_y = ty; }
}

void director_see(void) {
	seen.px = (int)emu_read32(BN6_PLAYER + 0x1C) >> 16;
	seen.py = (int)emu_read32(BN6_PLAYER + 0x20) >> 16;
	seen.on_map = on_map();
	seen.battle = emu_read8(BN6_GAMESTATE) == BN6_SUB_BATTLE;
	seen.timer = emu_read32(BN6_BATTLE_TIMER);
	see_tent();
}

/* The panel BN6 lights under MegaMan's feet, marked over his sprite: a
 * panel of the field (40 by 24, its first row 72 down the picture) in the
 * game's warning yellow, steady. */
void director_draw_tent(void) {
	if (!seen.tent_x) return;
	int x = P.core_x + (seen.tent_x - 1) * 40, y = P.core_y + 72 + (seen.tent_y - 1) * 24;
	SDL_Color c = rgba(255, 232, 0, 255);
	fill_rect(x + 2, y + 2, 36, 20, rgba(255, 232, 0, 96));
	fill_rect(x, y, 40, 2, c);
	fill_rect(x, y + 22, 40, 2, c);
	fill_rect(x, y + 2, 2, 20, c);
	fill_rect(x + 38, y + 2, 2, 20, c);
}

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
	if (dev.gem) loot_add_gem(&e);
	set_encounter(&e, false);
}

/* The NaviCust's board as the game has it: the variants on it, then those
 * MegaMan has that are not, where they fit beside them (a program left
 * off because it cannot fit is left out); how many. */
static int board_programs(uint8_t *out, int max) {
	uint32_t items = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_KEY_ITEMS);
	int n = 0, w, h;
	navicust_board(key_item(SCRIPTS_EXP_MEMORY), &w, &h);
	NaviShape s[10];
	int ns = 0;
	for (int e = 0; e < BN6_NAVICUST_PLACED_MAX && n < max; ++e) {
		int id = emu_read16(BN6_NAVICUST_PLACED + (uint32_t)e * 8);
		if (!id) break;
		if (id >= 47 * 4) continue;
		out[n++] = (uint8_t)id;
		if (ns < 10 && navicust_shape(id, &s[ns])) ++ns;
	}
	if (items < 0x02000000u || items >= 0x02040000u) return n;
	for (int v = 4; v < 47 * 4 && n < max && ns < 10; ++v) {
		int owned = emu_read8(items + BN6_PROGRAM_ITEMS + (uint32_t)v), placed = 0;
		for (int i = 0; i < n; ++i) placed += out[i] == v;
		for (int k = placed; k < owned && n < max && ns < 10; ++k) {
			if (!navicust_shape(v, &s[ns]) || !navicust_pack(s, ns + 1, w, h)) break;
			out[n++] = (uint8_t)v;
			++ns;
		}
	}
	return n;
}

/* Whether variant `v` fits the board beside the programs placed on it. */
static bool fits_beside_placed(int v) {
	int w, h, ns = 0;
	navicust_board(key_item(SCRIPTS_EXP_MEMORY), &w, &h);
	NaviShape s[10];
	for (int e = 0; e < BN6_NAVICUST_PLACED_MAX && ns < 9; ++e) {
		int id = emu_read16(BN6_NAVICUST_PLACED + (uint32_t)e * 8);
		if (!id) break;
		if (id != v && navicust_shape(id, &s[ns])) ++ns;
	}
	if (!navicust_shape(v, &s[ns])) return true;
	return navicust_pack(s, ns + 1, w, h);
}

static void library_from_game(void);

/* The folder's codes, for the layer about to be made (loot_fit_code): read
 * from the game as MegaMan moves on, kept with the run, so a checkpoint
 * rebuilds the layer as it was without the game's memory; and the
 * NaviCust's board, which the guardian's draft fits its programs beside;
 * and the Library, which a vault's lock counts (as the checkpoint after
 * keeps it, which a CONTINUE's rebuild reads). */
static void own_folder_chips(void);

/* The folder as the layer was made: its codes are the run's (run.codes),
 * its copies of each chip beside the checkpoint ("run.folder"), so a
 * CONTINUE makes the same stock before the game's memory is back. */
static uint16_t folder_made[BN6_FOLDER_ENTRIES];
#define FOLDER_MADE_MAGIC 0x43464C44u   /* "CFLD" */

static void note_folder_codes(void) {
	library_from_game();
	own_folder_chips();
	memset(run.programs, 0, sizeof run.programs);
	board_programs(run.programs, (int)sizeof run.programs);
	uint32_t data = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CHIPS);
	if (data < 0x02000000u || data >= 0x02040000u) return;
	for (int i = 0; i < BN6_FOLDER_ENTRIES; ++i) folder_made[i] = emu_read16(data + 2u * (uint32_t)i);
	loot_folder_codes(folder_made, BN6_FOLDER_ENTRIES, run.codes);
	loot_folder_counts(folder_made, BN6_FOLDER_ENTRIES);
}

/* Whether this run's duel on this layer is fought (the profile keeps the
 * last one's layer). */
static bool duel_fought(void) {
	return run.side_kind == LAYER_NORMAL && profile.duel_depth == run.depth && profile.duel_run == run.seed;
}

/* ProtoMan's duel as a CONTINUE finds it: Chaud's call, made before the
 * save, not made again (every CONTINUE on a duel layer had replayed it,
 * after the duel too); and a duel fought after the checkpoint stays
 * fought, ProtoMan gone as he goes when it begins (its choice made), the
 * gate beside him as the verdict left it (issue #20: a CONTINUE met him
 * again, and each duel won anew counted in the record). */
static void resume_duel(void) {
	if (flag_get(LAYER_DUEL_CALLED_FLAG)) D.duel_call_due = false;
	if (!duel_fought()) return;
	for (int i = 0; i < D.objs.nchoices; ++i)
		if (D.objs.choice[i].type == OBJ_DUEL) flag_set(D.objs.choice[i].flag);
	if (profile.duel_beat && layer_objs_official_level && rival_clearance() >= layer_objs_official_level) flag_set(LAYER_CLEARED_FLAG);
	D.duel_call_due = false;
}

/* Whether ProtoMan's duel stands on the layer. */
static bool duel_layer(void) {
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].type == OBJ_DUEL) return true;
	return false;
}

/* The layer's official gate open where Chaud's clearance reaches its level
 * (its script reads LAYER_CLEARED_FLAG, docs/RIVAL.md); the one beside
 * ProtoMan's duel opens to its winner alone, as the duel is won, and stays
 * as a CONTINUE (`resumed`) finds it (a playtester with the clearance found
 * it open before the duel, which then paid nothing but the record). Where
 * his netbattle waits for a later act, there is no duel to win: the
 * clearance's, as elsewhere. */
static void official_sync(bool resumed) {
	if (duel_layer() && !layer_objs_duel_later) {
		if (!resumed) flag_clear(LAYER_CLEARED_FLAG);
		return;
	}
	if (layer_objs_official_level && rival_clearance() >= layer_objs_official_level) flag_set(LAYER_CLEARED_FLAG);
	else flag_clear(LAYER_CLEARED_FLAG);
}

/* The rival's time to beat for a squad of `hp` (docs/RIVAL.md): three
 * seconds and one for every thirty HP, eight percent faster for every two
 * duels ProtoMan has lost (a rung's round), never under six tenths of it. */
static int duel_frames(int hp) {
	/* (loose at first: a playtester's act 1 hand took 27.5 s to ProtoMan's
	 * 11, and a good hand should beat the first rung) */
	double t = (4.0 + hp / 20.0) * 60.0, k = 1.0;
	for (int i = 0; i < profile.duel_won / 2; ++i) k *= 0.92;
	if (k < 0.6) k = 0.6;
	return (int)(t * k);
}

/* A race's squad holds no virus that decides when it can be hit. */
#define FAMILY_QUAKER 6   /* (the families are BN6's sprite categories less 0x0E) */
static bool duel_race_fair(const Encounter *e) {
	for (int i = 0; i < e->nfoes; ++i)
		if (e->foes[i].kind == FOE_VIRUS && e->foes[i].family == FAMILY_QUAKER) return false;
	return true;
}

static int encounter_hp(const Encounter *e) {
	int total = 0;
	for (int i = 0; i < e->nfoes; ++i) {
		const Foe *f = &e->foes[i];
		if (f->kind == FOE_ROCK) continue;
		int id = f->id >= 0 ? f->id : enemy_id(f->kind == FOE_NAVI ? 1 : 0, f->family, f->version), hp = 0, dmg = 0;
		if (id >= 0 && enemy_stats(id, &hp, &dmg)) total += hp;
	}
	return total;
}

/* A layer takes a second to make on a New 3DS, seconds more where a new
 * area's tiles are learned: where the last took long, the next one's
 * making is named over the still picture as MegaMan leaves a layer, or the
 * pause reads as a hang (a player on a 3DS: "felt like it froze"). A PC's
 * 60 ms show nothing. */
#define SLOW_BUILD_MS 100
static double build_ms;

static void building_word(void) {
	if (build_ms < SLOW_BUILD_MS) return;
	static const char *word = "Building the next layer...";
	int w = text_width(word) + 12, y = P.core_y + EMU_H - 38;
	platform_draw_over();
	fill_rect(P.core_x + (EMU_W - w) / 2, y, w, 16, rgba(0, 16, 40, 200));
	text_draw(P.core_x + EMU_W / 2, y + 4, word, WHITE, TEXT_CENTER);
	platform_present_now();
}

/* The net area a layer is drawn in: the biome's own, another game's that
 * dresses it in this run (run_dress), or the test hook's area of another
 * game (docs/MULTIROM.md), laid out as the BN6 area it is like, which then
 * is the layer's biome. */
void director_net_biome_arg(const char *v) {
	if (v[0] == 'x') director_debug_area = NET_AREAS + atoi(v + 1);
	else director_debug_biome = atoi(v);
}

static int layer_area(int *biome) {
	const NetAreaDef *x = director_debug_area >= 0 ? net_area_def(director_debug_area) : NULL;
	if (!x) return run.side_kind == LAYER_NORMAL ? run_dress(*biome) : *biome;
	*biome = x->like;
	return director_debug_area;
}

/* The song an area's layers play: another game's area its own theme,
 * copied into BN6 (docs/MULTIROM.md), where it can be; else `song`, the
 * BN6 area's. Its battles' themes (encounter_song) the same way: that
 * game's virus battle and boss music, else BN6's. */
static int layer_song(int tiles, int song) {
	const NetAreaDef *x = net_area_def(tiles);
	static const int bn6[2] = { ENCOUNTER_SONG_BATTLE, ENCOUNTER_SONG_BOSS };
	for (int b = 0; b < 2; ++b) {
		int theirs = x && x->xrom > 0 ? XR[x->xrom - 1].layout->battle_songs[b] : 0;
		encounter_song[b] = theirs ? xsong_install(x->xrom - 1, theirs, bn6[b]) : bn6[b];
	}
	return x && x->xrom > 0 && x->xsong ? xsong_install(x->xrom - 1, x->xsong, song) : song;
}

/* The map an area's layers take over, and the Navi their bystanders are
 * (layer_objs_bystander): another game's area its own Navi, copied into
 * BN6 (docs/MULTIROM.md), where it can be. */
static void layer_host(int tiles, int *group, int *number) {
	const NetAreaDef *a = net_area_def(tiles);
	if (!a) a = net_area_def(0);
	layer_objs_bystander = a->xrom > 0 && a->xnavi ? xnavi_slot(a->xrom - 1, a->xnavi, LAYER_BYSTANDER) : LAYER_BYSTANDER;
	if (a->xrom) { *group = a->over[0]; *number = a->over[1]; return; }
	*group = a->group;
	*number = a->host ? a->host - 1 : a->number;
}

static bool build_layer(void) {
	int biome = layer_biome(), tiles = layer_area(&biome);
	run.biome = biome;
	run.layer_seed = run.seed ^ (uint32_t)(run.depth * 2654435761u) ^ (uint32_t)(run.side_kind * 40503u);
	LayerKit kit;
	netmap_kit(tiles, &kit);
	layer_generate(run.layer_seed, run.depth, biome, run.side_kind, &kit);
	if (emu_debug_on()) fprintf(stderr, "layer depth %d biome %d layout %d stairs %d rise %d\n", run.depth, biome, layer.layout, layer.nstairs, layer.rise);
	if (!netmap_build_layer(tiles, run.layer_seed)) return false;

	const __typeof__(R.layout->net_area[0]) *a = area(biome);
	layer_host(tiles, &D.group, &D.number);   /* (the map its layers take over, their bystanders) */
	/* (the layer just left had its Net Dealer speak: this act's next say
	 * a line, not the greeting and the pick's reasons again, 5 to 8 boxes
	 * on every layer for a playtester) */
	if (D.layer_act && flag_get(LAYER_DEALER_TOLD_FLAG) && layer_objs_dealer_named) D.dealer_act = D.layer_act;
	D.layer_act = run.side_kind == LAYER_NORMAL ? (run.depth - 1) / 3 + 1 : 0;
	layer_objs_dealer_again = D.layer_act && D.dealer_act == D.layer_act;
	navicust_set_spins(meta_spins());   /* (the draft fits what turns) */
	/* the rival's duel (docs/RIVAL.md): its squad, rolled aside from the
	 * layer's own rolls, and ProtoMan's time for it */
	D.duel_call_due = false;
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].type == OBJ_DUEL) {
			uint32_t saved = rng_state();
			rng_seed(run.layer_seed ^ 0xD0E15EEDu);
			/* (rung 2: no race, ProtoMan himself, from the third act, where
			 * MegaMan can stand his hits: his attacks are his 1800 HP
			 * version's, his HP the act's guardian band at most; before it,
			 * he names the act) */
			layer_objs_duel_rung = profile.duel_won % 3;
			layer_objs_duel_later = layer_objs_duel_rung == 2 && pacing_loop(run.depth) == 0 && pacing_act(run.depth) < 2;
			/* (a race's squad is one of the act's own battles: its time is
			 * the test, not its strength; one above the band deleted a
			 * playtester at 100 of 140 HP on layer 2, his run over) */
			D.duel_enc = layer_objs_duel_rung == 2 ? make_boss(run.depth, run.biome, 11) : make_encounter(run.depth, run.biome, ENC_NORMAL);
			/* (and none a race can't hurry: a Quaker is out of reach in the
			 * air until it lands, so the clock times its hops, not the
			 * player; three of a playtester's four duels were Quakers, "a
			 * Quaker lottery") */
			for (int tries = 0; layer_objs_duel_rung != 2 && tries < 8 && !duel_race_fair(&D.duel_enc); ++tries)
				D.duel_enc = make_encounter(run.depth, run.biome, ENC_NORMAL);
			layer_objs_duel_foes = D.duel_enc.nfoes;
			if (emu_debug_on()) {
				fprintf(stderr, "duel squad (rung %d):", layer_objs_duel_rung);
				for (int k = 0; k < D.duel_enc.nfoes; ++k) fprintf(stderr, " %d/%d/%d", D.duel_enc.foes[k].kind, D.duel_enc.foes[k].family, D.duel_enc.foes[k].version);
				fprintf(stderr, "\n");
			}
			/* (the netbattle's ProtoMan: half the act's guardian band's top
			 * at most, as his attacks stay his 1800 HP version's, ten times
			 * a guardian's damage a second: at the top, 1000 in act 3, a
			 * playtester's MegaMan ran after one hand, docs/RIVAL.md) */
			int lo, hi;
			pacing_guardian_band(pacing_act(run.depth), &lo, &hi);
			D.duel_cap = layer_objs_duel_rung == 2 && pacing_loop(run.depth) == 0 ? hi / 2 : 0;
			rng_seed(saved);
			layer_objs_duel_frames = duel_frames(encounter_hp(&D.duel_enc));
			D.duel_call_due = true;
		}
	if (!layer_objs_install(D.group, D.number, &D.objs)) return false;
	mapslot_music(D.group, D.number, layer_song(tiles, a->song));
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
	D.fragments_seen = key_item(SCRIPTS_SECRET_DATA);
	D.fragment_due = false;
	D.spin_due = false;
	D.duel = false;
	D.duel_verdict_due = false;
	spins_sync();
	D.bugs_known = false;
	D.off_told = false;
	D.last_stop_told = false;
	D.final_told = false;
	D.checkpoint_data = false;
	D.pet_refreshed = false;
	flag_clear(LAYER_TOLD_FLAG);
	flag_clear(LAYER_DEALER_TOLD_FLAG);
	flag_clear(LAYER_VENDOR_TOLD_FLAG);
	flag_clear(LAYER_HEAL_TOLD_FLAG);
	flag_clear(LAYER_VAULT_FLAG);
	flag_clear(LAYER_OFFICIAL_FLAG);
	flag_clear(LAYER_DUEL_CALLED_FLAG);
	official_sync(false);
	D.arrow_pending = false;
	cinema_arrow(0, 0);
	D.secret_call = run.side_kind == LAYER_SECRET;
	talk_reset();
	return true;
}

/* build_layer timed (build_ms), named first where MegaMan leaves a layer */
static bool new_layer(bool leaving) {
	if (leaving) building_word();
	uint64_t t0 = SDL_GetPerformanceCounter();
	bool ok = build_layer();
	build_ms = (double)(SDL_GetPerformanceCounter() - t0) * 1000.0 / (double)SDL_GetPerformanceFrequency();
	return ok;
}

/* The run's starting folder in the game's first folder (docs/META.md): the
 * chosen one's 30 chips over the game's own (Standard keeps those), as
 * BN6's GiveFolder copies a folder in (bn6f sub_8021AB4). */
/* The folder's chips owned, as BN6 marks a chip it gives (bn6f
 * encryption_applyPack, Gregar 0x08006E70): its byte in the table at
 * Toolkit+0x7C is its key (0x020008A0 + chip) XOR 0x17, and a chip whose
 * byte does not match is taken for a cheat's and drawn blank in battle, no
 * name and no effect. A chip only written into the folder had none: a
 * playtester's LongSwrd, PanlGrab and Barrier came up blank in every hand,
 * where the Standard folder's chips, given at NEW GAME, played (session
 * 30). Every fresh layer and CONTINUE mark the folder's chips again, so a
 * run saved before this is mended. */
static void own_folder_chips(void) {
	uint32_t data = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CHIPS), marks = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CHIP_MARKS);
	if (data < 0x02000000u || data >= 0x02040000u || marks < 0x02000000u || marks >= 0x02040000u) return;
	for (int i = 0; i < BN6_FOLDER_ENTRIES; ++i) {
		int id = emu_read16(data + 2u * (uint32_t)i) & 0x1FF;
		if (id > 0) emu_write8(marks + (uint32_t)id, (uint8_t)(emu_read8(BN6_CHIP_KEYS + (uint32_t)id) ^ BN6_CHIP_KEY_XOR));
	}
}

/* A folder entry's count in the pack (bn6f getOffsetToQuantityOfChipCode:
 * the code's place among the chip record's four, else the first) */
static uint32_t pack_count_at(uint32_t pack, int entry) {
	int id = entry & 0x1FF, code = entry >> 9, slot = 0;
	uint32_t rec = R.layout->chip_data + (uint32_t)id * 0x2C;
	for (int k = 0; k < 4; ++k) if (R.data[rec + (uint32_t)k] == code) { slot = k; break; }
	return pack + 12u * (uint32_t)id + (uint32_t)slot;
}

static void set_start_folder(void) {
	const uint16_t *chips = meta_folder_chips(run.folder);
	uint32_t data = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CHIPS), pack = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_PACK);
	if (!chips || data < 0x02000000u || data >= 0x02040000u) return;
	/* the pack as if this folder had been given at NEW GAME, not the
	 * Standard one (GiveFolder counts a folder's chips in the pack): a
	 * playtester's Blade run kept the Standard folder's CrakShot and
	 * Cannons as spares, and the folder's promise with them (session 31) */
	bool counts = pack >= 0x02000000u && pack < 0x02040000u && R.data;
	for (int i = 0; i < BN6_FOLDER_ENTRIES && counts; ++i) {
		uint32_t at = pack_count_at(pack, emu_read16(data + 2u * (uint32_t)i));
		int n = emu_read8(at);
		if (n > 0) emu_write8(at, (uint8_t)(n - 1));
	}
	for (int i = 0; i < BN6_FOLDER_ENTRIES && counts; ++i) {
		uint32_t at = pack_count_at(pack, chips[i]);
		int n = emu_read8(at);
		if (n < 99) emu_write8(at, (uint8_t)(n + 1));
	}
	for (int i = 0; i < BN6_FOLDER_ENTRIES; ++i) {
		uint8_t b[2] = { (uint8_t)chips[i], (uint8_t)(chips[i] >> 8) };
		emu_write(data + 2u * (uint32_t)i, b, 2);
		flag_set(BN6_FLAG_LIBRARY + (chips[i] & 0x1FF));   /* (in the Library, as GiveFolder puts them) */
	}
	own_folder_chips();
}

/* The profile's Library (docs/META.md) in the run's game: the PET's Library
 * shows every chip held in any run, and a Chip Trader's prize, new to the
 * Library first, is new across runs. */
static void library_to_game(void) {
	for (int id = 1; id < 8 * (int)sizeof profile.library; ++id)
		if (meta_library_has(id)) flag_set(BN6_FLAG_LIBRARY + id);
}

/* ... and back: the chips the run's game has put in its Library since. */
static void library_from_game(void) {
	bool added = false;
	/* (a run begun by a build before the Library: its count starts here,
	 * not at 0; a playtester's chips of earlier runs read "+32" at its end) */
	if (run.active && profile.library_run != run.seed) {
		profile.library_start = (uint16_t)meta_library_count(-1);
		profile.library_run = run.seed;
		added = true;
	}
	for (int id = 1; id < 8 * (int)sizeof profile.library; ++id)
		if (chip_pool_class(id) >= 0 && flag_get(BN6_FLAG_LIBRARY + id)) added |= meta_library_add(id);
	if (added) profile_save();
}

/* ... and the programs MegaMan has, into the programs found, which later
 * runs' NaviCust vendors keep (docs/NAVICUST.md, 7). */
static void programs_from_game(void) {
	uint8_t now[10];
	int n = board_programs(now, (int)sizeof now);
	bool added = false;
	for (int i = 0; i < n; ++i) {
		int p = now[i] / 4;
		if (p <= 0 || p >= 64 || shop_program_found(p)) continue;
		profile.programs_found[p / 8] |= (uint8_t)(1u << (p % 8));
		added = true;
	}
	if (added) profile_save();
}

/* What this session has heard, forgotten by a run begun or continued (a
 * last run's act 1 is not this one's). */
static void forget_heard(void) { D.heard_act = D.dealer_act = 0; D.mail_quiet = false; D.mail_due = 0; }

bool director_start_run(void) {
	/* a new run leaves the last one behind: CONTINUE is for runs that
	 * have reached the net (one left so is no deletion to speak of) */
	no_room_told = -1;
	forget_heard();
	if (emu_debug_on()) {
		fprintf(stderr, "run guardians:");
		for (int a = 0; a < 4; ++a) {
			int g = run.boss_order[run.biome_order[a]];
			fprintf(stderr, " %s%s", guardian(g)->name, guardian_known(g) || rival(g)->met ? "" : "(new)");
		}
		fprintf(stderr, " nest %s\n", guardian(run.boss_order[BIOME_NEST])->name);
	}
	set_start_folder();
	library_to_game();
	powers_bring(run.cross);
	note_folder_codes();
	town_after_abandon = save_exists();
	save_delete();
	/* the first layer, entered through the town's port; the town itself
	 * (its seed apart from the layers') */
	if (!new_layer(false)) return false;
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
/* The duel's clock (docs/RIVAL.md): while its battle runs, the time so far
 * against ProtoMan's, as BN6's results screen counts them, in the picture's
 * top right, under the Custom gauge; on the second rung whether MegaMan has
 * been hit. Hidden while the clock holds (BATTLE START!, the Custom screen,
 * the pause): a playtester raced a time he could not see. */
void director_draw_duel(void) {
	if (!D.active || !D.duel || layer_objs_duel_rung == 2 || seen.on_map || !seen.battle) return;
	static uint32_t last;
	static int still;
	uint32_t t = seen.timer;
	still = t == last ? still + 1 : 0;
	last = t;
	if (still > 2 || !t) return;
	int his = layer_objs_duel_frames;
	char mine[16], theirs[16];
	snprintf(mine, sizeof mine, "%u:%02u.%02u", t / 3600, t / 60 % 60, t % 60 * 100 / 60);
	snprintf(theirs, sizeof theirs, "%d:%02d.%02d", his / 3600, his / 60 % 60, his % 60 * 100 / 60);
	char vs[32];
	snprintf(vs, sizeof vs, "ProtoMan %s", theirs);
	int lines = layer_objs_duel_rung == 1 ? 3 : 2, x = P.core_x + 236, y = P.core_y + 19, w = text_width(vs) + 6;
	fill_rect(x - w + 2, y - 2, w, lines * 10 + 3, rgba(0, 16, 40, 170));
	text_drawf(x, y, (int)t < his ? WHITE : rgba(255, 120, 120, 255), TEXT_RIGHT, "%s", mine);
	text_draw(x, y + 10, vs, rgba(170, 200, 255, 255), TEXT_RIGHT);
	if (lines == 3) text_draw(x, y + 20, D.duel_hit ? "Hit!" : "No hits", D.duel_hit ? rgba(255, 120, 120, 255) : rgba(140, 255, 170, 255), TEXT_RIGHT);
}

/* The layer's map (SELECT's over the picture, the second screen's): its
 * frame, and a grid step there, s pixels across and s / 2 down (x - y
 * across, x + y down) */
typedef struct {
	int bx, by, bw, bh;   /* the frame, inside its edge */
	int ox, oy, cu, cv, s;   /* the frame's middle, the grid's point there, the step */
	int mx, my;   /* MegaMan's panel */
} MapView;

static int map_x(const MapView *m, int x, int y) { return m->ox + (x - y - m->cu) * m->s; }
static int map_y(const MapView *m, int x, int y) { return m->oy + (x + y - m->cv) * (m->s / 2); }

static bool map_inside(const MapView *m, int sx, int sy, int pad) {
	return sx - pad >= m->bx && sy - pad >= m->by && sx + pad < m->bx + m->bw && sy + pad < m->by + m->bh;
}

/* where the ray from MegaMan to (sx, sy) leaves the frame, pad pixels in */
static bool map_edge(const MapView *m, int sx, int sy, int pad, int *ex, int *ey) {
	double sx0 = map_x(m, m->mx, m->my), sy0 = map_y(m, m->mx, m->my);
	double dx = sx - sx0, dy = sy - sy0, t = 1e9, hx = m->bw / 2.0 - pad, hy = m->bh / 2.0 - pad;
	if (dx > 0) t = fmin(t, (m->ox + hx - sx0) / dx);
	if (dx < 0) t = fmin(t, (m->ox - hx - sx0) / dx);
	if (dy > 0) t = fmin(t, (m->oy + hy - sy0) / dy);
	if (dy < 0) t = fmin(t, (m->oy - hy - sy0) / dy);
	if (!(t > 0 && t < 1e8)) return false;
	*ex = (int)lround(sx0 + dx * t);
	*ey = (int)lround(sy0 + dy * t);
	return true;
}

/* the key's entries, under the map */
enum { MAP_YOU, MAP_EXIT, MAP_HEAL, MAP_SHOP, MAP_BOSS, MAP_EVENT, MAP_RIVAL, MAP_KEYS };
static const struct { const char *what; SDL_Color c; } map_key[MAP_KEYS] = {
	{ "You", { 255, 255, 255, 255 } }, { "Exit", { 255, 230, 60, 255 } },
	{ "Heal", { 90, 255, 120, 255 } }, { "Shop", { 255, 160, 40, 255 } }, { "Boss", { 255, 70, 70, 255 } },
	{ "Event", { 210, 110, 255, 255 } }, { "ProtoMan", RIVAL_MARK },
};

/* an object's entry in the key, -1 for none: a Server, a dark warp or a
 * gate is "Event"; the rival his own, a white eye in it (he and the
 * official gate both showed violet) */
static int map_kind(int type) {
	switch (type) {
	case OBJ_EXIT: case OBJ_RETURN: return MAP_EXIT;
	case OBJ_BOSS: return MAP_BOSS;
	case OBJ_HEAL: return MAP_HEAL;
	case OBJ_SHOP: case OBJ_PROGRAMS: case OBJ_TRADER: case OBJ_BUGTRADER: return MAP_SHOP;
	case OBJ_UNDERNET: case OBJ_SECRET_GATE: case OBJ_CHALLENGE: case OBJ_NAVI_GATE: case OBJ_VAULT: case OBJ_OFFICIAL:
		return MAP_EVENT;
	case OBJ_DUEL: return MAP_RIVAL;
	default: return -1;
	}
}

/* a mark the map leaves out: a Server's once its battle is taken (a
 * playtester saw it still there after he had won), ProtoMan's once his
 * duel is done or put off (he only talks then) */
static bool map_left_out(int type) {
	if (type != OBJ_CHALLENGE && type != OBJ_DUEL) return false;
	for (int i = 0; i < D.objs.nchoices; ++i)
		if (D.objs.choice[i].type == type && (D.chosen & (1u << i))) return true;
	return type == OBJ_DUEL && layer_objs_duel_later;
}

/* the step as large as lets the floor seen so far fit (8 on the second
 * screen's larger frame, 6 over the picture, 4 at the least), on the
 * seen floor's middle, or on MegaMan when it fits at none */
static void map_view(MapView *m) {
	int umin = m->mx - m->my, umax = umin, vmin = m->mx + m->my, vmax = vmin;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			if (!D.seen[y][x] || layer.cell[y][x] == C_VOID) continue;   /* (a counter's panels are floor too) */
			int u = x - y, v = x + y;
			if (u < umin) umin = u;
			if (u > umax) umax = u;
			if (v < vmin) vmin = v;
			if (v > vmax) vmax = v;
		}
	int s = m->bw >= 300 ? 8 : 6;
	while (s > 4 && ((umax - umin) * s + 2 * s + 2 > m->bw || (vmax - vmin) * (s / 2) + s + 2 > m->bh)) s -= 2;
	m->s = s;
	m->ox = m->bx + m->bw / 2;
	m->oy = m->by + m->bh / 2;
	m->cu = (umin + umax) / 2;
	m->cv = (vmin + vmax) / 2;
	if (s == 4 && ((umax - umin) * 4 + 10 > m->bw || (vmax - vmin) * 2 + 6 > m->bh)) {
		m->cu = m->mx - m->my;
		m->cv = m->mx + m->my;
	}
}

/* the panels seen: each a diamond 2s - 1 wide and s - 1 high (7 by 3, 11
 * by 5, 15 by 7), a pixel apart from the next; their rows drawn a batch
 * per colour, raised floor lighter (thousands of rows, a call each had
 * cost the 3DS's processor) */
static void map_panels(const MapView *m) {
	static const SDL_Color colour[2] = { { 60, 140, 230, 240 }, { 150, 210, 255, 240 } };
	static SDL_Rect rows[2][512];
	int n[2] = { 0, 0 }, half = m->s / 2 - 1;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			if (!D.seen[y][x] || layer.cell[y][x] == C_VOID) continue;   /* (a counter's panels are floor too) */
			int sx = map_x(m, x, y), sy = map_y(m, x, y), k = layer.level[y][x] != 0;
			if (!map_inside(m, sx, sy, 3)) continue;
			for (int r = -half; r <= half; ++r) {
				if (n[k] == 512) { fill_rects(rows[k], n[k], colour[k]); n[k] = 0; }
				rows[k][n[k]++] = (SDL_Rect){ sx - (m->s - 1 - 2 * abs(r)), sy + r, 2 * m->s - 1 - 4 * abs(r), 1 };
			}
		}
	for (int k = 0; k < 2; ++k) fill_rects(rows[k], n[k], colour[k]);
}

/* the way on, over the panels he has come near, up to the first he
 * hasn't: marks he earned (a V in a comp's maze read as a dead end, the
 * arm on to the exit nowhere on the map); in straight runs as far as a
 * straight line over the floor goes (the walk's steps zig-zagged across a
 * platform) */
static void map_way(const MapView *m, int px, int py) {
	int tx = D.objs.exit_x, ty = D.objs.exit_y, ex, ey, len;
	SDL_Color tc = rgba(255, 230, 60, 200);
	if (D.objs.guardian.navi && !boss_beaten()) { tx = D.objs.guardian.x; ty = D.objs.guardian.y; tc = rgba(255, 110, 90, 200); }
	double wx, wy;
	netmap_grid(px, py, &wx, &wy);
	if (!netmap_panel(tx, ty, &ex, &ey) || route_way(wx, wy, ex, ey, &len) < 0) return;
	int cx = m->mx, cy = m->my, k = route_walk_len - 1;
	while (k >= 0) {
		int x = route_walk[k] % MAP_W, y = route_walk[k] / MAP_W, far = k;
		if (!D.seen[y][x]) break;
		for (int j = k - 1; j >= 0 && j >= k - 12; --j) {
			int jx = route_walk[j] % MAP_W, jy = route_walk[j] / MAP_W;
			if (!D.seen[jy][jx]) break;
			if (route_floor_line(cx, cy, jx, jy)) far = j;
		}
		int fx = route_walk[far] % MAP_W, fy = route_walk[far] / MAP_W, steps = abs(fx - cx) + abs(fy - cy);
		int ax = map_x(m, cx, cy), ay = map_y(m, cx, cy), bx = map_x(m, fx, fy), by = map_y(m, fx, fy);
		for (int t = 1; t <= steps; ++t) {
			int lx = ax + (bx - ax) * t / steps, ly = ay + (by - ay) * t / steps;
			if (map_inside(m, lx, ly, 3)) fill_rect(lx - (m->s - 1) / 2, ly, m->s - 1, 1, tc);
		}
		cx = fx; cy = fy;
		k = far - 1;
	}
}

/* one thing standing there, once seen; before, a ring where it stands if
 * MegaMan senses it, or a pip on the frame's edge its way (L named the
 * Recovery Mr. Prog, and it was nowhere on the map): the exit and the
 * guardian neither */
static void map_mark(const MapView *m, int type, int x, int y, SDL_Color c) {
	int sx = map_x(m, x, y), sy = map_y(m, x, y), ex, ey;
	if (!D.seen[y][x]) {
		if (type == OBJ_EXIT || type == OBJ_RETURN || type == OBJ_BOSS) return;
		if (map_inside(m, sx, sy, 3)) {
			fill_rect(sx - 3, sy - 3, 7, 7, c);
			fill_rect(sx - 2, sy - 2, 5, 5, rgba(0, 8, 28, 255));
		} else if (map_edge(m, sx, sy, 3, &ex, &ey))
			fill_rect(ex - 1, ey - 1, 3, 3, c);
		return;
	}
	if (!map_inside(m, sx, sy, 3)) return;
	fill_rect(sx - 3, sy - 3, 7, 7, rgba(0, 8, 28, 255));
	fill_rect(sx - 2, sy - 2, 5, 5, c);
	if (type == OBJ_DUEL) fill_rect(sx, sy, 1, 1, rgba(255, 255, 255, 255));
}

/* what stands there, and the goal: the guardian while it stands, else
 * the exit */
static void map_marks(const MapView *m, int *gx, int *gy, SDL_Color *gc) {
	bool goal_boss = false;
	for (int i = 0; i < layer.nobj; ++i) {
		const NetObj *o = &layer.obj[i];
		int x = (int)o->x, y = (int)o->y, kind = map_kind(o->type);
		if (kind < 0 || x < 0 || y < 0 || x >= MAP_W || y >= MAP_H || map_left_out(o->type)) continue;
		if (o->type == OBJ_BOSS && !boss_beaten()) { *gx = x; *gy = y; *gc = map_key[kind].c; goal_boss = true; }
		else if (o->type == OBJ_EXIT && !goal_boss) { *gx = x; *gy = y; }
		map_mark(m, o->type, x, y, map_key[kind].c);
	}
}

/* the goal while it is unseen: where the ray from MegaMan to it leaves
 * the frame */
static void map_goal(const MapView *m, int gx, int gy, SDL_Color gc) {
	int ax, ay;
	if (gx < 0 || D.seen[gy][gx] || !map_edge(m, map_x(m, gx, gy), map_y(m, gx, gy), 5, &ax, &ay)) return;
	fill_rect(ax - 1, ay - 3, 3, 1, gc);
	fill_rect(ax - 2, ay - 2, 5, 1, gc);
	fill_rect(ax - 3, ay - 1, 7, 3, gc);
	fill_rect(ax - 2, ay + 2, 5, 1, gc);
	fill_rect(ax - 1, ay + 3, 3, 1, gc);
}

/* the key, from (kx, ky): MegaMan, the exit, and what else this layer
 * holds; the gaps close up until it fits the width, then "You" goes,
 * whose mark pulses (ProtoMan's name ran off the picture) */
static void draw_map_key(int kx, int ky, int width) {
	bool has[MAP_KEYS] = { [MAP_YOU] = true, [MAP_EXIT] = true, [MAP_BOSS] = D.objs.guardian.navi != 0 };
	for (int i = 0; i < layer.nobj; ++i) {
		int kind = map_kind(layer.obj[i].type);
		if (kind > MAP_EXIT && kind != MAP_BOSS && !map_left_out(layer.obj[i].type)) has[kind] = true;
	}
	int gap = 10, need;
	for (;;) {
		need = 0;
		for (int i = 0; i < MAP_KEYS; ++i) if (has[i]) need += 8 + text_width(map_key[i].what) + gap;
		need -= gap;
		if (need <= width || (gap <= 0 && !has[MAP_YOU])) break;
		if (gap > 2 || !has[MAP_YOU]) gap -= 2;
		else { has[MAP_YOU] = false; gap = 8; }
	}
	if (gap < 0) gap = 0;
	for (int i = 0; i < MAP_KEYS; ++i) {
		if (!has[i]) continue;
		fill_rect(kx, ky + 3, 5, 5, map_key[i].c);
		if (i == MAP_RIVAL) fill_rect(kx + 2, ky + 5, 1, 1, rgba(255, 255, 255, 255));
		text_draw(kx + 7, ky, map_key[i].what, rgba(200, 225, 255, 255), TEXT_LEFT);
		kx += 8 + text_width(map_key[i].what) + gap;
	}
}

/* The layer's map in w x h from (x0, y0): SELECT's over the picture, and
 * the second screen's (the 3DS's bottom one, issue #9). */
static void draw_map(int x0, int y0, int w, int h) {
	fill_rect(x0, y0, w, h, rgba(0, 8, 28, 255));
	MapView m = { .bx = x0 + 6, .by = y0 + 18, .bw = w - 12, .bh = h - 38 };
	SDL_Color edge = rgba(120, 200, 255, 220);
	fill_rect(m.bx - 2, m.by - 2, m.bw + 4, 1, edge);
	fill_rect(m.bx - 2, m.by + m.bh + 1, m.bw + 4, 1, edge);
	fill_rect(m.bx - 2, m.by - 2, 1, m.bh + 4, edge);
	fill_rect(m.bx + m.bw + 1, m.by - 2, 1, m.bh + 4, edge);
	text_drawf(m.bx, y0 + 3, rgba(170, 220, 255, 255), TEXT_LEFT, "Layer %d", run.depth);
	int px = seen.px, py = seen.py;
	if (!netmap_panel(px, py, &m.mx, &m.my)) return;
	map_view(&m);
	map_panels(&m);
	map_way(&m, px, py);
	int gx = -1, gy = -1;
	SDL_Color gc = map_key[MAP_EXIT].c;
	map_marks(&m, &gx, &gy, &gc);
	map_goal(&m, gx, gy, gc);
	/* MegaMan, always there, his border pulsing */
	int ms = map_x(&m, m.mx, m.my), mt = map_y(&m, m.mx, m.my);
	if (map_inside(&m, ms, mt, 3)) {
		fill_rect(ms - 3, mt - 3, 7, 7, (D.frame / 10) % 2 ? rgba(120, 200, 255, 255) : rgba(0, 8, 28, 255));
		fill_rect(ms - 2, mt - 2, 5, 5, rgba(255, 255, 255, 255));
	}
	draw_map_key(m.bx + 1, m.by + m.bh + 5, m.bw - 2);
}

void director_draw_map(void) {
	if (!D.active || D.town || !D.map_shown || !seen.on_map) return;
	draw_map(P.core_x, P.core_y, 240, 160);
}

bool director_draw_second_screen(int w, int h) {
	/* (on the net, in battle too: the town and the title keep it dark) */
	if (!D.active || D.town) return false;
	draw_map(0, 0, w, h);
	return true;
}

/* (LAYER_MAKE: layer_make.h, beside its hash) */
#define LAYER_MAKE_MAGIC 0x434D4B31u   /* "CMK1" */
#define LAYER_SEEN_MAGIC 0x43534E31u   /* "CSN1" */

/* What the director knows of the act a checkpoint is in, saved beside its
 * state: the viruses deleted before the act began, its frames, and the act
 * whose Net Dealer has spoken. A CONTINUE takes it back, where an act
 * continued from a save had no count on its AREA CLEAR card and its next
 * Net Dealer greeted MegaMan as new (a playtester's, both). */
#define ACT_NOTE_MAGIC 0x41435432u   /* "ACT2" */
typedef struct { uint32_t seed; int32_t act, viruses, frames, dealer, unknown; } ActNote;
static ActNote act_note;
static bool act_note_ok;

static void act_note_save(void) {
	/* (an act already continued without one has no whole count to keep) */
	ActNote an = { run.seed, (run.depth - 1) / 3, D.act_viruses, D.act_frames, D.dealer_act, D.act_resumed };
	save_write_blob("run.act", ACT_NOTE_MAGIC, &an, sizeof an);
}

/* on CONTINUE, before the layer is built: its dealer's greeting reads it */
static void act_note_read(void) {
	act_note_ok = save_read_blob("run.act", ACT_NOTE_MAGIC, &act_note, sizeof act_note) && act_note.seed == run.seed &&
		act_note.act == (run.depth - 1) / 3;
	if (act_note_ok) D.dealer_act = act_note.dealer;
}

/* ... and after it, where building it began the act afresh */
static void act_note_apply(void) {
	D.act_resumed = !act_note_ok || act_note.unknown;
	if (act_note_ok) { D.act_viruses = act_note.viruses; D.act_frames = act_note.frames; }
	if (emu_debug_on()) fprintf(stderr, "act note: %s, act %d, viruses from %d, %d frames, dealer %d\n", act_note_ok ? "taken back" : "none",
		act_note.act, act_note.viruses, act_note.frames, act_note.dealer);
}

static void save_checkpoint(void) {
	char path[600];
	save_state_path(path, sizeof path);
	library_from_game();
	programs_from_game();
	save_run();
	emu_save_state(path);
	int make = LAYER_MAKE;
	save_write_blob("run.make", LAYER_MAKE_MAGIC, &make, sizeof make);
	/* the map's panels seen so far, beside the state they go with */
	save_write_blob("run.seen", LAYER_SEEN_MAGIC, D.seen, sizeof D.seen);
	save_write_blob("run.folder", FOLDER_MADE_MAGIC, folder_made, sizeof folder_made);
	act_note_save();
}

bool director_can_suspend(void) {
	return D.active && !D.town && !D.gameover && on_map() && !emu_read8(BN6_CHATBOX) && !talk_busy() && !D.warping &&
		emu_read8(BN6_WARP + 0x10) == 0 && boss_idle() && !D.challenge && !emu_read8(BN6_DIALOGUE_LOCK) &&
		flag_get(BN6_FLAG_PLAYER_CAN_MOVE);
}

const char *director_saved_where(void) { return D.saved_at ? D.saved_at : "Run saved at the layer's start"; }

void director_save_here(void) {
	/* (the PET's Save: in the town, before the run's first layer, there is
	 * no run to save yet) */
	if (!D.active) return;
	if (D.town) { cinema_note("Saves begin on layer 1", 150); return; }
	D.checkpoint = true;
	D.checkpoint_here = true;
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
	/* (GameState's protected zenny, then its BugFrags) */
	fprintf(f, "hp %d/%d\nzenny %u\nbugfrags %u\n", hp, max, (unsigned)emu_read32(BN6_GAMESTATE + 0x5C), (unsigned)emu_read32(BN6_GAMESTATE + 0x60));
	if (sub == BN6_SUB_BATTLE) fprintf(f, "custom gauge %d%%\n", emu_read16(BN6_CUSTOM_GAUGE) * 100 / 0x4000);
	/* (in a battle, the panel MegaMan stands on, from the left and the top:
	 * a player sees it at a glance, a playtester reading stills misread it
	 * turn after turn, as BN6 draws him half a row above his panel) */
	if (!on_map())
		for (uint32_t i = 0; i < BN6_T1_COUNT; ++i) {
			uint32_t o = BN6_T1_OBJECTS + i * BN6_T1_SIZE;
			if ((emu_read8(o) & 1) && emu_read8(o + 0x16) == 0 && emu_read8(o + 0x12)) {
				fprintf(f, "megaman stands column %d row %d\n", emu_read8(o + 0x12), emu_read8(o + 0x13));
				break;
			}
		}
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
				goal_way();
				fprintf(f, "way %d %s\n", way_dir, ways[way_dir]);
				for (int y = cy - 4; y <= cy + 4; ++y) {
					fprintf(f, "cells ");
					for (int x = cx - 4; x <= cx + 4; ++x)
						fputc(x == cx && y == cy ? '@' : x >= 0 && y >= 0 && x < MAP_W && y < MAP_H && layer.cell[y][x] == C_PATH ? '#' : '.', f);
					fputc('\n', f);
				}
				/* (CYBERWORLD_STATE_POS=map: the whole layer, with its objects
				 * and the arrow's walk: * the walk, + where the arrow aims,
				 * letters the objects, ^ raised floor, , the floor of the
				 * room before a guardian's arena) */
				if (!strcmp(getenv("CYBERWORLD_STATE_POS"), "map")) {
					goal_way();
					static char g[MAP_H][MAP_W + 1];
					int x0 = MAP_W, y0 = MAP_H, x1 = 0, y1 = 0;
					for (int y = 0; y < MAP_H; ++y) {
						for (int x = 0; x < MAP_W; ++x) {
							/* (= a counter's aisle, # the counter) */
							g[y][x] = layer.cell[y][x] == C_VOID ? ' ' : layer.cell[y][x] == C_SOLID ? '=' : layer.cell[y][x] == C_PROPPED ? '#'
								: layer.level[y][x] ? '^' : '.';
							if (layer.cell[y][x] != C_VOID) { x0 = x < x0 ? x : x0; y0 = y < y0 ? y : y0; x1 = x > x1 ? x : x1; y1 = y > y1 ? y : y1; }
						}
						g[y][MAP_W] = 0;
					}
					if (layer.ante >= 0) {
						const Room *r = &layer.rooms[layer.ante];
						for (int y = r->y; y < r->y + r->h; ++y)
							for (int x = r->x; x < r->x + r->w; ++x)
								if (g[y][x] == '.') g[y][x] = ',';
					}
					/* (% a sprite prop, in the void or in its walled hole) */
					for (int i = 0; i < layer.nprops; ++i)
						if (layer.props[i].kind == PROP_SPRITE) {
							int qx = layer.props[i].x, qy = layer.props[i].y;
							g[qy][qx] = '%';
							x0 = qx < x0 ? qx : x0; y0 = qy < y0 ? qy : y0; x1 = qx > x1 ? qx : x1; y1 = qy > y1 ? qy : y1;
						}
					for (int i = 0; i < route_walk_len; ++i) g[route_walk[i] / MAP_W][route_walk[i] % MAP_W] = '*';
					if (route_walk_aim >= 0) g[route_walk_aim / MAP_W][route_walk_aim % MAP_W] = '+';
					static const char mark[] = "IXMSHTTBUGNCPRFDVYO";   /* (D a Navi gate, V a vault, Y the rival, O an official gate) */
					for (int i = 0; i < layer.nobj; ++i) {
						int ox = (int)layer.obj[i].x, oy = (int)layer.obj[i].y;
						if (ox >= 0 && oy >= 0 && ox < MAP_W && oy < MAP_H && layer.obj[i].type < (int)sizeof mark - 1) g[oy][ox] = mark[layer.obj[i].type];
					}
					g[cy][cx] = '@';
					for (int y = y0; y <= y1; ++y) fprintf(f, "map %.*s\n", x1 - x0 + 1, &g[y][x0]);
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
	/* (named as the game shows him: a playtester reads this; a dev's
	 * state, CYBERWORLD_STATE_POS, names him always, as the scripts that
	 * find a guardian's seed read it) */
	if (D.objs.guardian.navi)
		fprintf(f, "guardian %s %s\n",
			guardian_known(D.objs.guardian.navi) || guardian_heard() || boss_cinematic() || boss_fighting() || boss_beaten() || boss_done() ||
					getenv("CYBERWORLD_STATE_POS")
				? guardian(D.objs.guardian.navi)->name : "???",
			boss_done() ? "done" : boss_beaten() ? "beaten" : boss_fighting() ? "fighting" : "waiting");
}

/* A on the map: MegaMan turns to face what he means to talk to, so the
 * game's own check finds it (walking into a navi slides him round it, and a
 * tap of the pad can leave him facing past it): the navi or Mystery Data
 * clearly before him if there is one, out to two and a half panels, else
 * the nearest within 52 units (the probes' reach) on the side he faces,
 * else the nearest behind (a vendor a step behind turned him round from
 * Mystery Data ahead). One out of reach he
 * walks up to (a step or two short of a navi, the press did nothing). */
static void probe_vectors(int vx[8], int vy[8]) {
	for (int k = 0; k < 8; ++k) {
		vx[k] = (int32_t)emu_read32(BN6_TALK_PROBES + (uint32_t)k * 24) >> 16;
		vy[k] = (int32_t)emu_read32(BN6_TALK_PROBES + (uint32_t)k * 24 + 4) >> 16;
	}
}

/* Where the game measures NPC object o from for talking and collision:
 * its place and its centre's shift (bn6f OverworldNPCObject +0x11-0x13: a
 * navi behind a counter is spoken to across it; a floor sprite stands
 * further back than it shows). */
static void npc_centre(uint32_t o, int *x, int *y) {
	*x = (int16_t)emu_read16(o + 0x26) + (int8_t)emu_read8(o + 0x11);
	*y = (int16_t)emu_read16(o + 0x2A) + (int8_t)emu_read8(o + 0x12);
}

/* The NPC slot A means, or -1. */
static int talk_target(void) {
	int px = (int)emu_read32(BN6_PLAYER + 0x1C) >> 16, py = (int)emu_read32(BN6_PLAYER + 0x20) >> 16;
	int face = emu_read8(BN6_PLAYER + 0x10) & 7, vx[8], vy[8];
	probe_vectors(vx, vy);
	double fl = sqrt((double)vx[face] * vx[face] + vy[face] * vy[face]);
	if (fl < 1) return -1;
	int front = -1, ahead = -1, near = -1, fd = 1 << 30, ad = 52 * 52 + 1, nd = 52 * 52 + 1;
	double fc = 0;
	for (int i = 0; i < 16; ++i) {
		uint32_t o = 0x020057B0u + (uint32_t)i * 0xD8;   /* the game's NPC objects (director_describe) */
		if (!(emu_read8(o) & 1) || !emu_read8(o + 0x0C)) continue;
		int cx, cy;
		npc_centre(o, &cx, &cy);
		int dx = cx - px, dy = cy - py, d = dx * dx + dy * dy;
		/* before him: within 30 degrees of his facing, the one most straight
		 * ahead (the nearest in that cone took a bystander a little off his
		 * line over the Mystery Data he faced); one he faces out of reach is
		 * the one he means: no turn to another beside him */
		double c = d > 0 ? (vx[face] * dx + vy[face] * dy) / (fl * sqrt((double)d)) : 1;
		if (d <= 80 * 80 && d > 0 && c >= 0.866 && (c > fc + 0.02 || (c > fc - 0.02 && d < fd))) { fd = d; fc = c; front = i; }
		if (d <= 52 * 52 && c >= 0.2 && d < ad) { ad = d; ahead = i; }
		if (d <= 52 * 52 && d < nd) { nd = d; near = i; }
	}
	/* (but one he touches beats one before him out of reach) */
	if (front >= 0 && !(near >= 0 && nd <= 24 * 24 && fd > 52 * 52)) return front;
	return ahead >= 0 ? ahead : near;
}

/* Where NPC slot i stands from MegaMan, the facing whose probe points at
 * it best, and whether that probe reaches it. */
static bool talk_reach(int i, int *face) {
	int px = (int)emu_read32(BN6_PLAYER + 0x1C) >> 16, py = (int)emu_read32(BN6_PLAYER + 0x20) >> 16, vx[8], vy[8];
	uint32_t o = 0x020057B0u + (uint32_t)i * 0xD8;
	int cx, cy;
	npc_centre(o, &cx, &cy);
	int tx = cx - px, ty = cy - py;
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
		int cx, cy;
		npc_centre(o, &cx, &cy);
		int dx = cx - px, dy = cy - py;
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

/* The guards below hold off an A for a fifth of a second as a screen
 * changes under it: a press that soon cannot be an answer to what just
 * appeared (a reaction takes about that long), so it was pressed at what
 * went before. One pressed after a look always passes (they had held A
 * off for half a second and more, which read as input lag). */
#define A_GUARD 12

/* A shop's list opened by its keeper's last box: the A pressed to close
 * that box twice over bought the first item's "Are you sure?" (Kai, three
 * sessions running). */
static bool shop_first;   /* a keeper's words opened the shop, and no "Are you sure?" has been asked in it */
static uint32_t shop_guard(uint32_t keys) {
	static int chat_recent, guard, last_mode = -1;
	int mode = main_mode();
	chat_recent = emu_read8(BN6_CHATBOX) ? 30 : chat_recent > 0 ? chat_recent - 1 : 0;
	if (last_mode == BN6_MODE_GAME && mode != BN6_MODE_GAME && mode != BN6_MODE_GAME_OVER && chat_recent) guard = A_GUARD, shop_first = true;
	if (mode == BN6_MODE_GAME) shop_first = false;
	last_mode = mode;
	if (guard > 0) { --guard; keys &= ~KEY_A; }
	return keys;
}

/* A choice, BN6's or ours: a playtester's A's, pressed through a Net
 * Dealer's words, landed twice on the shop's "Are you sure? > Yes" (BN6's
 * default) and bought what he had not chosen. B still answers No. And the
 * first "Are you sure?" in a shop its keeper's words opened starts on No:
 * A's paced through the Net Dealer's five to eight boxes, 50 frames apart,
 * still chose the list's first row and bought it (Kai, session 41). A
 * press carried over from the words then cancels, where it had spent the
 * zenny; a buy chosen after that starts on Yes, as BN6's do. */
static uint32_t choice_guard(uint32_t keys) {
	static int age;
	bool choice = emu_read8(BN6_CHATBOX) && emu_read8(BN6_CHATBOX_OPTIONS) >= 2;
	age = choice ? age + 1 : 0;
	if (choice && age == 1 && shop_first && main_mode() != BN6_MODE_GAME) {
		shop_first = false;
		emu_write8(BN6_CHATBOX_CURSOR, 1);
	}
	if (choice && age <= A_GUARD) keys &= ~KEY_A;
	return keys;
}

/* In battle an L or R pressed as the Custom gauge was all but full was
 * lost (the game takes them at a full gauge only; Kai re-pressed in every
 * fight, and at 50 frames still: the gauge's last tenth takes about a
 * second and looks full), and so was one the game let pass while MegaMan
 * fired or flinched: a press is kept two and a half seconds (at a second
 * and a half, one Kai pressed 126 frames before the gauge filled was
 * dropped: the last of it looks full) and given as
 * the gauge fills, one frame let go first so the game sees a press, then
 * again every CUSTOM_RETRY frames until the Custom screen opens (the
 * gauge empties as it does) or CUSTOM_TRIES frames have passed. While
 * the d-pad is held, and CUSTOM_CALM frames after, it waits, a second at
 * most: the step comes first (a kept R opened the Custom screen over the
 * UP that was stepping MegaMan off a lit panel, and the bomb burst as the
 * battle went on; dropping the press at a d-pad press instead lost a
 * playtester's R pressed before a dodge in a guardian's fight, where both
 * are needed). Once a kept press has opened the Custom screen, L and R are
 * held off CUSTOM_HUSH frames: a second R, pressed as the screen was slow
 * to slide in, opened a chip's description on it. */
#define CUSTOM_EARLY 150
#define CUSTOM_TRIES 45
#define CUSTOM_RETRY 20
#define CUSTOM_CALM 12
#define CUSTOM_WAIT 60
#define CUSTOM_HUSH 30
static uint32_t custom_buffer(uint32_t keys, bool l_pressed, bool r_pressed) {
	static int kept, step, calm, wait, hush;
	static uint32_t which;
	if (main_mode() != BN6_MODE_GAME || emu_read8(BN6_GAMESTATE) != BN6_SUB_BATTLE) { kept = step = calm = hush = 0; return keys; }
	if (hush > 0) { --hush; return keys & ~(KEY_L | KEY_R); }
	if (keys & PAD_KEYS) calm = CUSTOM_CALM;
	else if (calm > 0) --calm;
	bool full = emu_read16(BN6_CUSTOM_GAUGE) >= 0x4000;
	if (l_pressed || r_pressed) {
		/* (pressed at a full gauge: the game has this press; again later
		 * only if it let it pass) */
		kept = full ? CUSTOM_TRIES : CUSTOM_EARLY;
		step = full ? 2 : 0;
		wait = CUSTOM_WAIT;
		which = l_pressed ? KEY_L : KEY_R;
		return keys;
	}
	if (kept <= 0) return keys;
	if (!full) {
		/* still filling, or emptied by the Custom screen taking a press */
		if (step) hush = CUSTOM_HUSH;
		kept = step ? 0 : kept - 1;
		return keys;
	}
	if (calm > 0) {
		if (--wait <= 0) kept = step = 0;
		return keys;
	}
	if (step == 0) kept = CUSTOM_TRIES;
	--kept;
	int s = step++ % CUSTOM_RETRY;
	if (s == 0) return keys & ~(KEY_L | KEY_R);
	if (s == 1) return keys | which;
	return keys;
}

uint32_t director_keys(uint32_t keys) {
	talk_only_update();
	if (D.active) keys = choice_guard(shop_guard(keys));
	bool l = (keys & KEY_L) != 0, pressed = l && !D.l_held;
	bool r = (keys & KEY_R) != 0, r_pressed = r && !D.r_held;
	bool a = (keys & KEY_A) != 0, a_pressed = a && !D.a_held;
	D.l_held = l;
	D.r_held = r;
	D.a_held = a;
	D.dir_held = (keys & PAD_KEYS) != 0;
	D.map_shown = false;
	if (D.active && !autopilot_on()) keys = custom_buffer(keys, pressed, r_pressed);
	if (!D.active || !on_map()) return keys;   /* (in battle L opens the Custom screen) */
	/* (A ends the arrival's card early: its words wait on it) */
	if (D.arrival_hold && a_pressed && cinema_card_age() >= CARD_SKIP) cinema_card_yield();
	/* (no A a fifth of a second after a chat closes: a playtester's A
	 * pressed through a chat's last box talked to the gift Prog beside him
	 * again, twice a session) */
	bool chat_open = emu_read8(BN6_CHATBOX) || talk_busy();
	if (D.chat_was_open && !chat_open) D.a_quiet = A_GUARD;
	D.chat_was_open = chat_open;
	if (D.a_quiet > 0) { --D.a_quiet; if (!chat_open) { keys &= ~KEY_A; a_pressed = false; } }
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
	if (!D.town && (keys & KEY_SELECT)) { D.map_shown = !emu_read8(BN6_CHATBOX); map_used |= D.map_shown; keys &= ~KEY_SELECT; }
	/* R on the port: the jack-in, which the arrow does not follow into its
	 * flash and tunnel (a playtester saw it drawn over them) */
	if (D.town && r_pressed && town_on_port((int)emu_read32(BN6_PLAYER + 0x1C) >> 16, (int)emu_read32(BN6_PLAYER + 0x20) >> 16)) {
		cinema_arrow(0, 0);
		D.arrow_pending = false;
	}
	/* R in the town away from the port: MegaMan says where it is (the game
	 * itself does nothing there) */
	if (D.town && r_pressed && !talk_busy() && !emu_read8(BN6_CHATBOX) && !cinema_busy() && emu_read8(BN6_WARP + 0x10) == 0 &&
		!town_on_port((int)emu_read32(BN6_PLAYER + 0x1C) >> 16, (int)emu_read32(BN6_PLAYER + 0x20) >> 16)) {
		static char buf[160];
		int px = (int)emu_read32(BN6_PLAYER + 0x1C) >> 16, py = (int)emu_read32(BN6_PLAYER + 0x20) >> 16;
		int dx = town_info()->port_x - px, dy = town_info()->port_y - py, far, nx = 0, ny = 0;
		int near = town_port_near(px, py, &nx, &ny);
		/* (a step short of a jack-in cell: he takes it, and R jacks in; a
		 * playtester stood at the mermaid fountain's rim a step off its
		 * ring and pressed R five times) */
		if (near >= 0 && near <= PORT_STEP * PORT_STEP) {
			emu_write32(BN6_PLAYER + 0x1C, (uint32_t)nx << 16);
			emu_write32(BN6_PLAYER + 0x20, (uint32_t)ny << 16);
			cinema_arrow(0, 0);
			D.arrow_pending = false;
			return keys;
		}
		const char *way = town_way(&far);
		/* (close by: which way to its nearest cell, as the crow flies, where
		 * the walk to the front's middle wound round the basin and turned
		 * from "up and to the left" to "straight down" a step apart) */
		if (near >= 0 && near < 128 * 128)
			snprintf(buf, sizeof buf, "@M Almost, Lan! Step up to the %s, %s, and press R.", town_info()->landmark, way_to(nx, ny, &far));
		else if (dx * dx + dy * dy < 128 * 128)
			snprintf(buf, sizeof buf, "@M Almost, Lan! The %s is %s.|@M Step right up to it and press R.", town_info()->landmark, way);
		else
			snprintf(buf, sizeof buf, "@M There's no port here, Lan.|@M It's by the %s!", town_info()->landmark_at);
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
	else if (!pressed && D.l_kept > 0) {
		/* (kept through the engine's own cards and words, and given after
		 * the arrival's, which wait for the card too: an L pressed as the
		 * act card faded went to the act's arrival words and was gone) */
		if (can_l && !D.beat[0]) { pressed = true; D.l_kept = 0; }
		else if (!talk_busy() && !cinema_busy() && !D.beat[0]) --D.l_kept;
	}
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
		int cx, cy;
		npc_centre(o, &cx, &cy);
		int dx = cx - px, dy = cy - py;
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

/* The NaviCust's rotations (key items 0x50-0x55, one a colour: white,
 * yellow, pink, red, blue, green; "Lets you rotate white parts with the L
 * and R Button"): BN6 hands them out over its story; a run has those the
 * profile found in the net, one a run (docs/META.md), and no other: a
 * program of another colour lies as its record draws it, and the drafts
 * offer only what fits so (navicust_set_spins). */
static unsigned spins_in_game(void) {
	uint32_t items = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_KEY_ITEMS);
	if (items < 0x02000000u || items >= 0x02040000u) return 0;
	unsigned mask = 0;
	for (uint32_t c = 1; c <= 6; ++c)
		if (emu_read8(items + 0x4F + c)) mask |= 1u << (c - 1);
	return mask;
}

static void grant_spins(void) {
	uint32_t items = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_KEY_ITEMS), check = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_KEY_CHECK);
	if (items < 0x02000000u || items >= 0x02040000u || check < 0x02000000u || check >= 0x02040000u) return;
	/* (the count and the item's check, its seed ^ 0x55, as the game's own
	 * giving writes them: a count without it reads as none) */
	unsigned held = meta_spins();
	for (uint32_t c = 1; c <= 6; ++c) {
		uint32_t id = 0x4F + c;
		uint8_t want = (uint8_t)(emu_read8(BN6_KEY_ITEM_SEEDS + id) ^ 0x55);
		if (held >> (c - 1) & 1) {
			if (!emu_read8(items + id) || emu_read8(check + id) != want) {
				emu_write8(items + id, 1);
				emu_write8(check + id, want);
			}
		} else if (emu_read8(items + id)) emu_write8(items + id, 0);
	}
}

/* The Spins as the profile has them, in the game's key items and the
 * drafts: as a layer is made and after a checkpoint's state (a run saved
 * by an older build held all six) */
static void spins_sync(void) {
	navicust_set_spins(meta_spins());
	grant_spins();
	D.spins_seen = spins_in_game();
}

/* The run's Spin picked up from its Mystery Data (the game gives the key
 * item): the profile keeps it, and MegaMan says what it does. */
static void spin_watch(void) {
	unsigned now = spins_in_game(), fresh = now & ~D.spins_seen;
	D.spins_seen = now;
	int c = D.objs.spin_colour;
	if (c && (fresh >> (c - 1) & 1) && !(meta_spins() >> (c - 1) & 1)) {
		meta_spin_found(c);
		navicust_set_spins(meta_spins());
		D.spin_due = D.objs.spin_found >= 0;
	}
	if (D.spin_due && talk_script(D.objs.archive, D.objs.spin_found)) D.spin_due = false;
}

/* MegaMan's words for a program left off the board, once a layer (NULL:
 * none to say): where to place it, and whether it turns. */
static const char *off_board_words(void) {
	int offv = 0;
	const char *off = D.off_told || run_won_here() ? NULL : program_off_board(&offv);
	static char words[320];
	if (!off || !*off) return NULL;
	if (!fits_beside_placed(offv)) return no_room_words(off);
	/* (and whether it turns: a playtester pressed L and R on his gift's
	 * SuperArmor with no Spin, and nothing said why) */
	snprintf(words, sizeof words, "@M Lan, %s isn't on our NaviCust's board! It does nothing until it's placed: PET, MegaMan, then NaviCust.|@M %s",
		off, navicust_turn_words(offv));
	return words;
}

/* The NaviCust's bugs, named in MegaMan's words when they change: after the
 * player runs the NaviCust in the PET, or an ExpMemry grows the board
 * (docs/NAVICUST.md). A bug the player can read is a price they chose; the
 * game only says that there is one. Read on the layer's first quiet frame
 * without a word, so a layer entered bugged does not repeat it; spoken
 * BUG_CALM frames after the map is back, before a step (issue #13: a
 * second's wait, kept since a talk opened straight out of the PET had
 * drawn its letters as noise, let MegaMan walk first; a talk opened six
 * frames after the PET closed, from its menu and from the NaviCust, drew
 * them whole). */
/* Why the board bugs, from the game's own grid and its programs'
 * records (navicust_bug_cause), or NULL. */
static const char *bug_cause(void) {
	static char names[BN6_NAVICUST_SLOTS][16];
	NaviPart parts[BN6_NAVICUST_SLOTS];
	uint8_t grid[NAVICUST_GRID * NAVICUST_GRID];
	if (!R.data || !R.layout || !R.layout->navicust_programs) return NULL;
	int n = 0;
	for (int i = 0; i < BN6_NAVICUST_SLOTS; ++i) {
		int v = emu_read16(BN6_NAVICUST_PLACED + (uint32_t)i * 8);
		const char *about = v > 0 && v < 47 * 4 ? navicust_about(v / 4) : NULL, *colon = about ? strchr(about, ':') : NULL;
		parts[i] = (NaviPart){ NULL, -1, 0 };
		if (v <= 0 || v >= 47 * 4) continue;
		const uint8_t *rec = R.data + R.layout->navicust_programs + (uint32_t)v * 16;
		snprintf(names[i], sizeof names[0], "%.*s", colon ? (int)(colon - about) : 0, colon ? about : "");
		parts[i] = (NaviPart){ colon ? names[i] : NULL, rec[1], rec[3] };
		n = i + 1;
	}
	for (int i = 0; i < NAVICUST_GRID * NAVICUST_GRID; ++i) grid[i] = emu_read8(BN6_NAVICUST_GRID + (uint32_t)i);
	int w, h;
	navicust_board(key_item(SCRIPTS_EXP_MEMORY), &w, &h);
	return navicust_bug_cause(grid, parts, n, w, h);
}

#define BUG_CALM 10
static void bug_watch(void) {
	static int last, calm;
	calm = D.frame == last + 1 ? calm + 1 : 0;
	last = D.frame;
	if (calm < BUG_CALM) return;
	uint8_t now[NAVICUST_BUGS];
	for (int t = 0; t < NAVICUST_BUGS; ++t) now[t] = emu_read8(BN6_NAVICUST_BUGS + (uint32_t)t);
	if (!D.bugs_known) {
		memcpy(D.bugs, now, sizeof now);
		D.bugs_known = true;
		return;
	}
	/* (back from the PET with a program left off the board: said at once,
	 * where L said it only on the next layer; a playtester ran the NaviCust
	 * without placing his Guardian Data's HP+100) */
	static bool ran;   /* (the PET was open since the last words: a RUN) */
	ran |= D.pet_seen;
	if (D.pet_seen && !talk_busy() && !cinema_busy() && !emu_read8(BN6_CHATBOX)) {
		D.pet_seen = false;
		const char *say = off_board_words();
		if (say && talk_start(say, FACE_MEGAMAN)) { D.off_told = true; return; }
	}
	if (!memcmp(D.bugs, now, sizeof now)) { ran = D.pet_seen; return; }
	if (talk_busy() || cinema_busy() || emu_read8(BN6_CHATBOX)) return;
	bool had = false;
	for (int t = 0; t < NAVICUST_BUGS; ++t) had |= D.bugs[t] != 0;
	const char *words = navicust_bug_words(now, ran, bug_cause());
	if (*words ? talk_start(words, FACE_MEGAMAN) : !had || talk_start("@M Our NaviCust runs clean now, Lan!", FACE_MEGAMAN)) {
		memcpy(D.bugs, now, sizeof now);
		ran = false;
	}
}

/* A BugFrag Trader's trade (issue #12). After Yes, BN6's script holds
 * (ts_wait_hold) for the trader machine on the Undernet's map, which rolls
 * the prize, gives it, takes the ten BugFrags, saves and runs the script
 * that shows it (bn6f sub_809A078); after No it closes the box and holds
 * for the machine's scene to end the chat. A layer's trader stands without
 * the machine: its chat held for good after Yes, and after No MegaMan
 * walked with the chat still open, the PET shut. The director does what
 * the machine does where the chat holds, but takes the ten BugFrags before
 * it gives: the script's "not enough" line holds as its Yes does, and
 * read as a Yes it gave a chip for none, TakeBugfrags taking nothing
 * (issue #21: A mashed through "Try again?" traded on at 0 BugFrags). */
static void bugfrag_trade(void) {
	static bool howl;
	if (!emu_read8(BN6_CHATBOX)) {
		/* (the machine clears its howl as it shows the prize) */
		if (howl) flag_clear(BN6_FLAG_TRADER_HOWL);
		howl = false;
		return;
	}
	uint32_t f = emu_read32(BN6_CHATBOX_FLAGS);
	if (D.objs.trader_kind != TRADER_BUGFRAG || !(f & 0x80) || !(f & 0x08) ||
	    emu_read32(BN6_CHATBOX + 0x30) != BN6_TRADER_TEXT) return;
	/* (after No the box is closed: the chat ends as the Chip Trader's No
	 * ends it, with its script 5, a bare end) */
	if (f & 7) { game_call(BN6_CHAT_RUN_SCRIPT, BN6_TRADER_TEXT, 5); return; }
	uint32_t prize[2], took[2];
	game_call(BN6_TRADER_RESET, 0, 0);
	if (!game_call_ret(BN6_TRADER_PRIZE, 0, 0, 0, prize)) return;
	/* (TakeBugfrags: 0 where it took them, else it took none) */
	if (!game_call_ret(BN6_TAKE_BUGFRAGS, 10, 0, 0, took) || took[0]) {
		game_call(BN6_CHAT_RUN_SCRIPT, BN6_TRADER_TEXT, 5);
		return;
	}
	uint16_t chip = (uint16_t)prize[0], code = (uint16_t)prize[1];
	uint8_t v[4];
	put16(v, chip);
	put16(v + 2, code);
	emu_write(BN6_TRADER_STATE + 4, v, 4);
	game_call_ret(BN6_GIVE_CHIPS, chip, code, 1, NULL);
	/* (the map saved on) */
	put16(v, emu_read16(BN6_GAMESTATE + 4));
	emu_write(BN6_GAMESTATE + 0x0C, v, 2);
	game_call(BN6_SAVE_GAME, 0, 0);
	/* (the script names the prize from the chat box's two words) */
	emu_write32(BN6_CHATBOX + 0x4C, chip);
	emu_write32(BN6_CHATBOX + 0x50, code);
	game_call(BN6_CHAT_RUN_SCRIPT, BN6_TRADER_TEXT, 15);
	memset(v, 0, sizeof v);
	emu_write(BN6_TRADER_STATE + 4, v, 4);
	emu_write(BN6_TRADER_STATE + 0x30, v, 2);
	howl = true;
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

/* As the layer's map closes (SELECT let go): the way-on arrow, as after
 * L's words, for a player who looked for the way there (a playtester
 * looked at the map on a strip where the arrow had faded, then probed
 * directions for four calls). */
static void map_arrow(void) {
	static bool was;
	if (was && !D.map_shown && on_map()) {
		goal_way();
		if (cinema_arrow_on()) cinema_arrow_extend(300);
		else cinema_arrow(way_dir, 300);
	}
	was = D.map_shown;
}

/* In the town: nothing to watch but the jack-in, whose arrival on the
 * layer's map starts the run as a layer's warp does. */
/* The way-on arrow: on while L's words last, however many boxes, then ten
 * seconds more, and on while MegaMan walks, up to half a minute; and after
 * the map. */
static void arrow_update(void) {
	/* (not over a warp's or the jack-in's flash and tunnel) */
	if (D.warping || emu_read8(BN6_WARP + 0x10)) {
		if (cinema_arrow_on()) cinema_arrow(0, 0);
		D.arrow_pending = false;
		return;
	}
	map_arrow();
	/* (it turns as MegaMan walks: frozen, it pointed into the gap he had
	 * walked past) */
	/* (a new way twice running before it turns: at a walkway's mouth the
	 * route's first leg flipped as MegaMan crossed a panel's border, and
	 * the arrow with it; a look every 5 frames, as every 15 a running
	 * MegaMan was two panels past a turn before it turned) */
	/* (and not for a way just past the edge of the one shown: the arrow
	 * wobbled between neighbouring eighths as the walk's aim moved, a third
	 * of its turns swung back within a second, and a playtester holding
	 * the way a picture showed ran past the turns) */
	/* (and at once for the first frames after MegaMan stops: standing, the
	 * way can't flip on a border, and a playtester's pictures 4 frames
	 * after each step showed the way from before it, three steps running
	 * at one walkway's mouth) */
	static int tick, pending = -1, settle;
	settle = D.dir_held ? 4 : settle > 0 ? settle - 1 : 0;
	bool stopping = !D.dir_held && settle > 0;
	if (cinema_arrow_on() && on_map() && (++tick % 5 == 0 || stopping)) {
		goal_way();
		if ((way_dir == pending || stopping) && !route_way_holds(cinema_arrow_dir(), 0.8)) cinema_arrow_turn(way_dir);
		pending = way_dir;
	}
	/* (it faded three seconds after the words, and in the Aquarium Comp's
	 * mazes of short walkways a playtester lost a dozen moves at a time
	 * between one L and the next) */
	if (cinema_arrow_on() && !D.arrow_pending && D.dir_held && on_map() && cinema_arrow_age() < 1800) cinema_arrow_extend(180);
	if (!D.arrow_pending) return;
	/* (three seconds after a briefing of eight boxes, and a playtester who
	 * closed its last had lost it before he set off) */
	if (talk_busy()) cinema_arrow_extend(60);
	else { D.arrow_pending = false; cinema_arrow_extend(600); }
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
	no_room_told = -1;
	forget_heard();
	D.town = false;
	/* (a headless run starting in the net: its folder as the town would
	 * have set it) */
	if (run.depth == 1 && run.side_kind == LAYER_NORMAL) { set_start_folder(); library_to_game(); }
	/* (and the Cross it brought at any depth, as a run has it there) */
	powers_bring(run.cross);
	note_folder_codes();
	if (!new_layer(false)) return false;
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
	no_room_told = -1;
	forget_heard();
	D.saved_at = "Run saved where you continued";
	/* (the folder the layer was made with: none for a run saved before it
	 * was kept, which made its stock without it) */
	if (!save_read_blob("run.folder", FOLDER_MADE_MAGIC, folder_made, sizeof folder_made)) memset(folder_made, 0, sizeof folder_made);
	loot_folder_counts(folder_made, BN6_FOLDER_ENTRIES);
	act_note_read();
	/* the layer's tables live in the ROM copy, which a state does not hold */
	if (!new_layer(false)) return false;
	char path[600];
	save_state_path(path, sizeof path);
	int make = 0;
	bool same = save_read_blob("run.make", LAYER_MAKE_MAGIC, &make, sizeof make) && make == LAYER_MAKE;
	if (emu_load_state(path)) {
		lock_run();
		spins_sync();
		/* (the ScrtData the save holds, which building the layer could not
		 * read yet: MegaMan took the saved one for a new find, "Our second
		 * ScrtData!" with one in hand) */
		D.fragments_seen = key_item(SCRIPTS_SECRET_DATA);
		/* the shops' data in RAM is the saved one: this layer's again,
		 * what was bought before the save still bought (a CONTINUE had
		 * restocked both shops); another build's layer, afresh */
		layer_objs_shops(&D.objs, same);
		own_folder_chips();   /* (a run saved with the folder's chips unmarked) */
		official_sync(true);
		if (!same) {
			/* another build's layer: its flags and Mystery Data picks
			 * forgotten, and in from the start */
			/* (a gift taken stays taken, and a guardian beaten stays
			 * beaten, his Guardian Data shown or taken, his exit open:
			 * they are the run's, not the layer's; a playtester's run
			 * saved beside ElementMan's open exit would have met him
			 * again, his Guardian Data twice) */
			for (int f = MAPSLOT_MD_FLAG; f <= LAYER_HEAL_TOLD_FLAG; ++f)
				if (f != LAYER_GIFT_FLAG && (f < LAYER_BOSS_GONE_FLAG || f > LAYER_EXIT_OPEN_FLAG)) flag_clear(f);
			flag_clear(LAYER_VAULT_FLAG);
			flag_clear(LAYER_OFFICIAL_FLAG);
			flag_clear(LAYER_DUEL_CALLED_FLAG);
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
		resume_duel();
		/* choices made before the checkpoint stay made */
		for (int i = 0; i < D.objs.nchoices; ++i)
			if (flag_get(D.objs.choice[i].flag)) {
				D.chosen |= 1u << i;
				if (D.objs.choice[i].type == OBJ_NPC) D.heard_act = D.layer_act;
			}
		/* enter the map again where MegaMan stood: the game reloads its NPCs
		 * and tiles from this build's tables, which a state does not hold */
		int x = (int)emu_read32(BN6_PLAYER + 0x1C) >> 16, y = (int)emu_read32(BN6_PLAYER + 0x20) >> 16;
		/* (a build that lays the layer out otherwise may have no floor there
		 * any more: then its arrival) */
		int cx, cy;
		if (!netmap_panel(x, y, &cx, &cy) || cx < 0 || cy < 0 || cx >= MAP_W || cy >= MAP_H ||
		    (layer.cell[cy][cx] != C_PATH && layer.cell[cy][cx] != C_PROPPED))
			x = D.start_x, y = D.start_y;
		emu_warp(D.group, D.number, x, y, 4);
		/* where they are, again; the arrival's words were said before */
		begin_area(false);
		act_note_apply();
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
	note_folder_codes();
	if (!new_layer(true)) return;
	D.warping = true;
	D.checkpoint = true;
	emu_warp_out();
}

/* The duel's verdict (docs/RIVAL.md): MegaMan's DeleteTime against
 * ProtoMan's, and on rung 1 no hit taken; the record kept, and Chaud's
 * words queued, with both times as the results screen shows them. */
/* Chaud's first words on a duel's end, with both times as the results
 * screen shows them: the netbattle, a win, a hit taken, a time too slow. */
static int verdict_result(char *out, size_t n, bool won, bool beat, int rung, int mine, int his) {
	char a[16], b[16];
	snprintf(a, sizeof a, "%d:%02d.%02d", mine / 3600, mine / 60 % 60, (mine % 60) * 100 / 60);
	snprintf(b, sizeof b, "%d:%02d.%02d", his / 3600, his / 60 % 60, (his % 60) * 100 / 60);
	if (!won && rung == 2) return snprintf(out, n, "@C Out of the netbattle, Lan? Better than deleted. That's a loss.|@C ProtoMan will be back.|");
	if (!won) return snprintf(out, n, "@C Out of the duel, Lan? That's a loss.|");
	if (rung == 2) return snprintf(out, n, "@C ...Log out, ProtoMan. You beat him, Lan.|");
	if (beat && rung == 1) return snprintf(out, n, "@C %s, and not a scratch. ...Not bad, Lan.|@C ProtoMan, we train harder.|", a);
	if (beat) return snprintf(out, n, "@C %s. ProtoMan's was %s. ...Not bad, Lan.|@C We'll be faster next time.|", a, b);
	if (mine < his) return snprintf(out, n, "@C %s, but MegaMan took a hit. A clean bust or nothing, Lan.|", a);
	return snprintf(out, n, "@C %s. ProtoMan's was %s. Too slow, Lan.|", a, b);
}

static void duel_verdict(bool won) {
	D.duel = false;
	int mine = D.duel_time, his = layer_objs_duel_frames, rung = layer_objs_duel_rung, before = rival_clearance();
	bool beat = won && (rung == 2 || (mine < his && !(rung == 1 && D.duel_hit)));

	if (beat) profile.duel_won++;
	else profile.duel_lost++;
	profile.duel_run = run.seed;
	profile.duel_depth = (uint16_t)run.depth;
	profile.duel_beat = beat;
	profile_save();
	int size = (int)sizeof D.duel_verdict, k = verdict_result(D.duel_verdict, sizeof D.duel_verdict, won, beat, rung, mine, his);
	#define ADD(...) (k += snprintf(D.duel_verdict + k, k < size ? (size_t)(size - k) : 0, __VA_ARGS__))
	/* (what his respect opens: docs/RIVAL.md) */
	int after = rival_clearance();
	if (after > before && after == 1) ADD("@C You've earned my clearance, Lan. The net's official gates will open for you now.|");
	else if (after > before) ADD("@C My full clearance, Lan. Every official gate opens for you now.|");
	ADD("@C That's %d-%d between us.", profile.duel_won, profile.duel_lost);
	/* (and the next rung, the door it leads to: a playtester's second win
	 * read as for the record alone) */
	if (beat && rung == 1 && after < 2) ADD("|@C Next time, no race: ProtoMan faces MegaMan himself. Beat him, and my full clearance is yours.");
	/* (Lan answers a win too: a playtester's first, after five losses, met
	 * silence where every loss had had his "Next time, Chaud!") */
	if (beat) ADD("|@L %s", rung == 2 ? "Good battle, ProtoMan! See you next time, Chaud!"
		: profile.duel_won == 1 && profile.duel_lost ? "We finally beat his time, MegaMan! See you next time, Chaud!"
		: "Yes! See you next time, Chaud!");
	/* (and the gate beside the duel opens at once to its winner: the prize
	 * where it was offered) */
	bool opened = beat && layer_objs_official_level && after >= layer_objs_official_level;
	if (opened) {
		flag_set(LAYER_CLEARED_FLAG);
		ADD("|@M Lan! The official gate on this layer will open for us now!");
	}
	/* (Lan answers a loss, as he took the duel: Chaud had the last word) */
	if (!beat) ADD("|@L Next time, Chaud!");
	#undef ADD
	D.duel_verdict_due = true;
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
		case OBJ_DUEL:
			set_encounter(&D.duel_enc, true);
			D.challenge = true;
			D.duel = true;
			D.duel_hit = false;
			D.duel_hp = -1;
			D.duel_time = 0;
			D.duel_call_due = false;
			flag_set(LAYER_DUEL_CALLED_FLAG);
			return true;
		case OBJ_UNDERNET:
		case OBJ_SECRET_GATE:
			run.side_kind = D.objs.choice[i].type == OBJ_UNDERNET ? LAYER_UNDERNET : LAYER_SECRET;
			enter_side_layer();
			return true;
		case OBJ_NPC:
			/* a bystander named the act's guardian (and no more to do) */
			D.heard_act = D.layer_act;
			break;
		case OBJ_NAVI_GATE: {
			/* his SP, as a challenge: the boss theme, and his chip after */
			Encounter e = make_boss(run.depth, run.biome, D.objs.gate_navi);
			e.foes[0].version = 2;
			set_encounter(&e, true);
			D.challenge = true;
			D.gate_fight = true;
			return true;
		}
		default:
			break;
		}
	}
	return false;
}

static void win_run(void);

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
	/* the short net's Nest fallen: the run is won (on threat 10, its first
	 * guardian's exit leads down to the second's, docs/META.md) */
	if (boss_beaten() && run.biome == BIOME_NEST && run_short_last(run.depth)) { win_run(); return true; }
	if (boss_beaten() && run.biome == BIOME_NEST && run_short_nest(run.depth)) run.boss_order[BIOME_NEST] = (uint8_t)run_nest_second();
	if (boss_beaten()) clear_card();
	/* past the endless net's Nest guardian: the net rebuilds (the next
	 * arrival says so; counted with the next checkpoint, which a CONTINUE
	 * cannot undo) */
	if (boss_beaten() && run.biome == BIOME_NEST && !run_short_nest(run.depth)) D.nest_cleared = true;
	/* the Guardian Data's second way on, where it was taken (docs/META.md,
	 * routes): the next act in that area, under its guardian */
	if (run.side_kind == LAYER_NORMAL && is_boss_depth(run.depth) && flag_get(LAYER_ROUTE_FLAG)) {
		int act = (run.depth % CYCLE_LAYERS) / 3, navi = 0, b = run_route_alt(act, &navi);
		if (b >= 0) {
			run.biome_order[act] = (uint8_t)b;
			run.boss_order[b] = (uint8_t)navi;
		}
		flag_clear(LAYER_ROUTE_FLAG);
	}
	/* ... or its dark way, into the Undernet */
	if (run.side_kind == LAYER_NORMAL && is_boss_depth(run.depth) && flag_get(LAYER_ROUTE_DARK_FLAG)) {
		int act = (run.depth % CYCLE_LAYERS) / 3, alt_navi = 0, navi = 0;
		int b = run_route_alt(act, &alt_navi) >= 0 ? run_route_dark(act, alt_navi, &navi) : -1;
		if (b >= 0) {
			run.biome_order[act] = (uint8_t)b;
			run.boss_order[b] = (uint8_t)navi;
		}
		flag_clear(LAYER_ROUTE_DARK_FLAG);
	}
	/* a side layer's exit leads one area deeper too */
	run.depth++;
	run.side_kind = LAYER_NORMAL;
	note_folder_codes();
	if (!new_layer(true)) return false;
	D.warping = true;
	D.checkpoint = true;
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
		else if (!strcmp(name, "status")) { talk_start(status_words(), FACE_MEGAMAN); return; }
		/* (a BugFrag Trader's trade wants ten: fifty given, no chat) */
		else if (!strcmp(name, "bugfrags")) { game_call(BN6_GIVE_BUGFRAGS, 50, 0); return; }
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

static void end_run(void) {
	/* what the summary tells: where, and by whom */
	const char *area = guardian_area_in_text(run.biome, run.side_kind);
	if (D.lost_to) snprintf(title_cause, sizeof title_cause, "by %s in %s", guardian(D.lost_to)->name, area);
	else if (D.lost_duel) snprintf(title_cause, sizeof title_cause, "in ProtoMan's duel");
	else snprintf(title_cause, sizeof title_cause, "in %s", area);
	/* (the guardian's first battle, lost: its battle data is what the run
	 * leaves for the next briefing) */
	const Rival *rv = D.lost_to ? rival(D.lost_to) : NULL;
	snprintf(title_learned, sizeof title_learned, "%s", rv && rv->megaman_won + rv->navi_won == 1 ? guardian(D.lost_to)->name : "");
	/* (a first run is no record to beat) */
	title_new_best = profile.runs > 0 && run.depth > profile.best_depth;
	title_won = false;
	runlog_run_end();
	library_from_game();
	programs_from_game();
	meta_run_over(false);
	profile_record_run();
	save_delete();
	run.active = false;
	D.active = false;
	title_summary = true;
	scene_set(&scene_title);
}

/* The short net won: its Nest's guardian fell and MegaMan stepped on its
 * exit. The run ends on the title's summary of a win, and what it opened
 * for the next (docs/META.md). */
static void win_run(void) {
	snprintf(title_cause, sizeof title_cause, "on layer %d", run.depth);
	title_learned[0] = 0;
	title_new_best = run.depth > profile.best_depth;
	title_won = true;
	runlog_run_end();
	library_from_game();
	programs_from_game();
	meta_run_over(true);   /* (before the clear counts: it names what the win opened) */
	profile.nest_clears++;
	profile_record_run();
	save_delete();
	run.active = false;
	D.active = false;
	title_summary = true;
	scene_set(&scene_title);
}

/* The last stop before a guardian's arena: the Net Dealer and a heal stand
 * in the room before it or one beside it (net_gen.c), off the arrow's
 * line, and a playtester walked past both to SpoutMan at 120 of 140 HP
 * with 1150 zenny unspent. Stepping into the room before the arena,
 * MegaMan names those not yet used, once, and which way each is. */
/* (where a service lies from MegaMan, as L and the map give it, and how
 * far the walk there is: "right here, to the left", "down and to the
 * right", "a long way back, straight up"; where the walk sets off another
 * way, that it winds: the walk's first step, "up and to the right", named
 * a heal that L and the map put up and to the left) */
static const char *service_where(int wx, int wy, char *buf, size_t n) {
	int far;
	bool winds;
	const char *lies = lie_and_walk(wx, wy, &far, &winds);
	snprintf(buf, n, far == 0 ? "right here, %s%s" : far == 1 ? "%s%s" : "a long way back, %s%s", lies,
		winds ? ", though the way there winds" : "");
	return buf;
}

/* Chaud's call on a duel layer (docs/RIVAL.md): the record, ProtoMan's time
 * and the term, what a win opens, and Lan's answer. */
static const char *duel_call_words(void) {
	int f = layer_objs_duel_frames, sec = f / 60;
	static char call[400];
	/* (the record said: Chaud remembers every duel) */
	char record[64];
	snprintf(record, sizeof record, "@C Lan, it's Chaud. It's %d-%d between us.|", profile.duel_won, profile.duel_lost);
	/* (where, as the net goes, and what for: "the third act" was the
	 * game's word, and a playtester asked what the netbattle would pay;
	 * the rung comes after two wins, whose clearance he holds: "every
	 * official gate" read as if he had none) */
	static const char *const full = "Beat him, and my full clearance is yours: the official vaults open too.";
	if (layer_objs_duel_later)
		snprintf(call, sizeof call, "@C Lan, it's Chaud. No more races: ProtoMan wants a netbattle with MegaMan himself.|"
			"@C He'll be waiting past the next %s. %s", pacing_act(run.depth) == 0 ? "two guardians" : "guardian",
			rival_clearance() < 2 ? full : "Get MegaMan ready.");
	else if (layer_objs_duel_rung == 2)
		snprintf(call, sizeof call, "@C Lan, it's Chaud. ProtoMan's on this layer, and this time it's no race.|"
			"@C He'll face MegaMan himself. %s%s", rival_clearance() < 2 ? full : "He hasn't forgotten the last time.",
			rival_clearance() < 2 && layer_objs_official_level >= 2 ? "|@C There's one on this layer, beside him: three Mega chips." : "");
	else {
		/* (what a win opens for one already cleared: the gate beside
		 * him, whose prize the duel is) */
		const char *stake = !layer_objs_official_level ? ""
			: rival_clearance() < layer_objs_official_level ? "@C The official vault beside him takes my full clearance: three wins, the last against ProtoMan himself.|"
			: layer_objs_official_level >= 2 ? "@C Beat it, and the official vault beside him opens: three Mega chips.|"
			: "@C Beat it, and the official gate beside him opens: an official Chip Order, three chips you've held, one to order.|";
		snprintf(call, sizeof call, "%s@C ProtoMan's on this layer. He busted its viruses in %d:%02d.%02d.|%s@C Think MegaMan can beat that%s?",
			profile.duel_won + profile.duel_lost ? record :
			"@C Lan. It's Chaud. I hear you're diving the Cyberworld.|@C The Nest copies Navis. ProtoMan's the real thing.|",
			sec / 60, sec % 60, (f % 60) * 100 / 60,
			/* (what a win earns, before the first: a playtester risked his
			 * run for pride alone) */
			/* (and what the gate holds: a playtester, five duels lost, took
			 * the gates for scenery, their prize never named) */
			profile.duel_won ? stake : layer_objs_official_level ? "@C Beat it, and I'll clear you for the net's official gates. There's one on this layer: an official Chip Order, three chips you've held, one to order.|"
			: "@C Beat it, and I'll clear you for the net's official gates.|",
			layer_objs_duel_rung == 1 ? ", without a hit" : "");
	}
	/* (Lan answers: a call no one answered read as a message left, the
	 * netbattle's too) */
	size_t n = strlen(call);
	snprintf(call + n, sizeof call - n, "|@L %s", layer_objs_duel_later ? "We'll be ready, Chaud!"
		: profile.duel_won + profile.duel_lost ? "You're on, Chaud!" : "Chaud?! ...You're on!");
	return call;
}

static void last_stop(int cx, int cy) {
	if (D.last_stop_told || layer.ante < 0 || !D.objs.guardian.navi || boss_beaten() || boss_fighting()) return;
	const Room *a = &layer.rooms[layer.ante];
	if (cx < a->x || cy < a->y || cx >= a->x + a->w || cy >= a->y + a->h) return;
	if (emu_read8(BN6_CHATBOX) || talk_busy() || cinema_busy() || D.warping || D.map_shown) return;
	D.last_stop_told = true;
	int hp = emu_read16(BN6_NAVI_STATS + 0x40), max = emu_read16(BN6_NAVI_STATS + 0x42);
	const char *dealer = NULL, *heal = NULL;
	static char dway[80], hway[80];
	for (int i = 0; i < layer.nobj; ++i) {
		const NetObj *o = &layer.obj[i];
		int wx, wy;
		netmap_world((int)o->x, (int)o->y, &wx, &wy);
		if (o->type == OBJ_SHOP && !dealer && !flag_get(LAYER_DEALER_TOLD_FLAG)) dealer = service_where(wx, wy, dway, sizeof dway);
		if (o->type == OBJ_HEAL && !heal && !flag_get(LAYER_HEAL_TOLD_FLAG) && hp < max) heal = service_where(wx, wy, hway, sizeof hway);
	}
	if (!dealer && !heal) return;
	static char buf[300];
	int k = guardian_known(D.objs.guardian.navi) || guardian_heard()
		? snprintf(buf, sizeof buf, "@M %s's arena is just ahead, Lan!|@M ", guardian(D.objs.guardian.navi)->name)
		: snprintf(buf, sizeof buf, "@M The guardian's arena is just ahead, Lan!|@M ");
	if (dealer && heal) snprintf(buf + k, sizeof buf - (size_t)k, "The Net Dealer's %s, and a Recovery Mr. Prog's %s, if we want to get ready first.", dealer, heal);
	else if (dealer) snprintf(buf + k, sizeof buf - (size_t)k, "The Net Dealer's %s, if we want to get ready first.", dealer);
	else snprintf(buf + k, sizeof buf - (size_t)k, "A Recovery Mr. Prog's %s, if we want to heal up first.", heal);
	talk_start(buf, FACE_MEGAMAN);
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
		D.lost_duel = D.duel;
		/* (deleted in a duel: a duel lost, which the verdict, never
		 * reached, would have counted; Chaud's next call knew nothing of
		 * it) */
		if (D.duel) { profile.duel_lost++; profile_save(); }
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
			last_stop(cx, cy);
			/* the short net's last guardian fallen and its exit open: the
			 * run's end said on the map, before the pad (the win went from
			 * the pad straight to the title's summary) */
			if (!D.final_told && run.biome == BIOME_NEST && run_short_last(run.depth) && boss_done() && !emu_read8(BN6_CHATBOX) &&
				!talk_busy() && !cinema_busy() && !D.warping &&
				/* (the arrival's growl answered, Dad's voice, and the endless
				 * net's hook: a playtester's first win ended on two lines) */
				talk_start("@M That was the Nest's last guardian, Lan... The whole net has gone quiet.|"
					"@B Grrrr......|"
					"@M ...Almost. Something deeper down is still awake, Lan. The Nest was only its den.|"
					"@D Lan, MegaMan, it's Dad! I watched it all from the lab. You did it!|"
					"@D Whatever is growling down there, we'll be ready for it. Now jack out and come home, you two.|"
					"@L We did it, MegaMan! The exit's open. Let's jack out!", FACE_MEGAMAN))
				D.final_told = true;
		}
	}
	/* (the PET's first menu is a screen of the game's own mode, its pages
	 * other modes: either, and not a battle) */
	int screen = emu_read8(BN6_GAMESTATE);
	if (main_mode() != BN6_MODE_GAME ? main_mode() != BN6_MODE_GAME_OVER
	    : screen != BN6_SUB_MAP && screen != BN6_SUB_BATTLE && screen != BN6_SUB_BATTLE_INIT) D.pet_seen = true;
	if (on_map()) { unwedge(); push_arrow(); emu_encounter_battle_forget(); bug_watch(); spin_watch(); grant_spins(); bugfrag_trade(); }
	cinema_on_map(on_map());
	if (!on_map()) {
		int sub = emu_read8(BN6_GAMESTATE);
		if (sub == BN6_SUB_BATTLE_INIT || sub == BN6_SUB_BATTLE) {
			emu_battle_release();   /* the forced battle has begun */
			if (!D.in_battle) {
				bool guardian = boss_fighting();
				D.record_known = guardian;
				if (!guardian && !D.challenge) ++D.battles;
				if (guardian) runlog_battle_start(NULL, "guardian");
			}
			D.in_battle = true;
			/* the duel's hits and time: MegaMan's HP lower than at the last
			 * frame, and the results screen's DeleteTime (docs/RIVAL.md) */
			if (D.duel && sub == BN6_SUB_BATTLE) {
				for (uint32_t k = 0; k < BN6_T1_COUNT; ++k) {
					uint32_t o = BN6_T1_OBJECTS + k * BN6_T1_SIZE;
					if (!(emu_read8(o) & 1) || emu_read8(o + 0x16) != 0) continue;
					int hp = emu_read16(o + 0x24);
					if (D.duel_hp >= 0 && hp < D.duel_hp) D.duel_hit = true;
					D.duel_hp = hp;
					break;
				}
				int t = (int)emu_read32(BN6_BATTLE_TIMER);
				if (t > D.duel_time) D.duel_time = t;
				/* the netbattle's ProtoMan, once he stands on the field: his
				 * HP and MaxHP (+0x24, +0x26) the act's guardian band at
				 * most (docs/RIVAL.md, docs/EMULATION.md) */
				for (uint32_t k = 0; D.duel_cap && k < BN6_T1_COUNT; ++k) {
					uint32_t o = BN6_T1_OBJECTS + k * BN6_T1_SIZE;
					if (!(emu_read8(o) & 1) || emu_read8(o + 0x16) != 1) continue;
					if (emu_read16(o + 0x26) > D.duel_cap) {
						uint8_t v[2] = { (uint8_t)D.duel_cap, (uint8_t)(D.duel_cap >> 8) };
						emu_write(o + 0x24, v, 2);
						emu_write(o + 0x26, v, 2);
					}
					D.duel_cap = 0;
				}
			}
			/* its rewards in the folder's codes, half the time (read again
			 * through the battle: its enemies spawn a few frames in, and
			 * the reward is picked as it ends) */
			if (sub == BN6_SUB_BATTLE && D.frame % 16 == 0) emu_encounter_lean_drops();
			/* the battle the game was handed, once its setup names the
			 * record (a re-roll may have come between its roll and now) */
			if (!D.record_known) {
				int s = emu_encounter_battle_slot();
				if (s >= 0) { D.next = D.rolled[s]; D.foes = D.next.nfoes; }
				if (s != -2) {
					D.record_known = true;
					loot_battle_fought(&D.next);
					runlog_battle_start(&D.next, D.challenge ? "challenge" : "battle");
				}
				if (emu_debug_on() && s != -2) fprintf(stderr, "battle from record %d: field %02x player %02x foes %d\n", s, D.next.field, D.next.player, D.foes);
			}
			/* (where the game put MegaMan, once a battle) */
			if (emu_debug_on() && !D.placed_told)
				for (uint32_t i = 0; i < BN6_T1_COUNT; ++i) {
					uint32_t o = BN6_T1_OBJECTS + i * BN6_T1_SIZE;
					if ((emu_read8(o) & 1) && emu_read8(o + 0x16) == 0) {
						fprintf(stderr, "megaman on panel %d %d\n", emu_read8(o + 0x12), emu_read8(o + 0x13));
						D.placed_told = true;
						break;
					}
				}
		}
		return;
	}
	if (D.in_battle) {
		/* back from a battle: count the deleted viruses (a navi counts below) */
		D.in_battle = false;
		D.placed_told = false;
		bool won = emu_read8(BN6_BATTLE_RESULT) == 1;
		runlog_battle_end(won);
		/* the PET's battle data on the viruses just fought */
		if (!boss_fighting()) {
			for (int i = 0; i < D.next.nfoes; ++i)
				if (D.next.foes[i].kind == FOE_VIRUS) profile_family_note(D.next.foes[i].family);
			profile_save();
			/* (and, the first time, what the Mystery Data on its field
			 * was: said after it was met, kept or broken, not before) */
			for (int i = 0; i < D.next.nobj; ++i) D.gem_due |= D.next.obj[i].kind >> 4 == FIELD_GEM && !profile.gem_taught;
		}
		if (emu_debug_on() && won) {
			int r = emu_read16(BN6_BATTLE_REWARD);
			if (r >> 14 == 0 && r != 0xFFFF) {
				ChipInfo ci;
				chip_info(r & 0x1FF, &ci);
				fprintf(stderr, "battle reward %s %c (folder codes %c%c%c)\n", ci.name, (r >> 9 & 0x1F) >= 26 ? '*' : 'A' + (r >> 9 & 0x1F),
					run.codes[0] ? 'A' + run.codes[0] - 1 : '-', run.codes[1] ? 'A' + run.codes[1] - 1 : '-', run.codes[2] ? 'A' + run.codes[2] - 1 : '-');
			} else fprintf(stderr, "battle reward %04x\n", r);
		}
		if (won && !boss_fighting()) run.viruses_deleted += D.foes;
		if (!D.challenge && !boss_fighting()) roll_encounter();
	}
	/* back from the guardian's battle */
	if (boss_fighting() && !emu_battle_forcing()) {
		boss_battle_over(emu_read8(BN6_BATTLE_RESULT) == 1);
		D.pet_refreshed = false;   /* (Dad's mails made again: the Records, the report) */
	}
	if (D.challenge && !emu_battle_forcing()) {
		/* back from the challenge (the game gave its reward, the signal
		 * gives its own for a win): random battles again */
		D.challenge = false;
		bool won = emu_read8(BN6_BATTLE_RESULT) == 1;
		if (D.duel) { duel_verdict(won); D.pet_refreshed = false; }
		else if (D.gate_fight) D.gate_due = won && D.objs.gate_reward >= 0;
		else D.reward_due = won && D.objs.challenge_reward >= 0;
		D.gate_fight = false;
		roll_encounter();
	}
	/* (as a talk, MegaMan held: run straight off, the A paging its first
	 * box talked to the Server he faced, whose own words took the box, and
	 * the prize was never named) */
	if (D.reward_due && talk_script(D.objs.archive, D.objs.challenge_reward)) D.reward_due = false;
	/* the PET's words that count (the codes, the Library), the profile's
	 * key items and Dad's mail, once a layer is under way (after a
	 * CONTINUE's state, which holds what the run had) */
	if (!D.pet_refreshed && on_map()) {
		D.pet_refreshed = true;
		int mailed = pet_text_refresh();
		if (mailed && D.mail_quiet) D.mail_due = mailed;
		D.mail_quiet = true;
	}
	/* (after the arrival's card and words: said over the jack-in, it was
	 * lost under them) */
	if (D.mail_due && !D.reward_due && !D.gem_due && !D.area_card && !D.beat[0] && !cinema_busy() && !talk_busy() &&
		!emu_read8(BN6_CHATBOX) && !boss_cinematic()) {
		char words[160];
		snprintf(words, sizeof words, "@M Mail from Dad, Lan! He sorted out our battle data on %s. It's in the PET's E-Mail.",
			guardian(D.mail_due)->name);
		if (talk_start(words, FACE_MEGAMAN)) D.mail_due = 0;
	}
	/* the rival (docs/RIVAL.md): Chaud's verdict after the duel, his call
	 * once the layer's arrival has been said */
	if (D.duel_verdict_due && on_map() && !cinema_busy() && !talk_busy() && !emu_read8(BN6_CHATBOX) &&
		talk_start(D.duel_verdict, FACE_CHAUD))
		D.duel_verdict_due = false;
	if (D.duel_call_due && !D.reward_due && !D.gem_due && !D.mail_due && !D.area_card && !D.beat[0] && !cinema_busy() && !talk_busy() &&
		!emu_read8(BN6_CHATBOX) && !boss_cinematic() && D.frame > 60) {
		const char *call = duel_call_words();
		if (talk_start(call, FACE_CHAUD)) {
			D.duel_call_due = false;
			flag_set(LAYER_DUEL_CALLED_FLAG);
		}
	}
	if (D.gem_due && !D.reward_due && talk_start("@M Mystery Data on the battlefield, Lan! Any hit breaks it, theirs or ours.|"
		"@M But if it's still there when we win, its data is ours!", FACE_MEGAMAN)) {
		D.gem_due = false;
		profile.gem_taught = 1;
		profile_save();
	}
	if (D.gate_due && talk_script(D.objs.archive, D.objs.gate_reward)) D.gate_due = false;
	run.fragments = key_item(SCRIPTS_SECRET_DATA);
	if (run.fragments > D.fragments_seen && D.objs.fragment_found >= 0) D.fragment_due = true;
	D.fragments_seen = run.fragments;
	if (D.fragment_due && talk_script(D.objs.archive, D.objs.fragment_found)) {
		D.fragment_due = false;
		D.fragments_told = run.fragments;
	}
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
	if (emu_read8(BN6_CHATBOX)) cinema_card_yield();
	/* the arrival's cards and MegaMan's words after them are one beat: he
	 * is held from his arrival until the words begin, as BN6 holds him for
	 * its own scenes, and A ends a card early (issue #13: free under the
	 * card, a player walked to a Mystery Data and opened it, and the words
	 * came after it, out of their moment) */
	bool hold = D.beat[0] && (D.area_card || cinema_busy()) && !talk_busy();
	if (hold && cinema_input_mode() == CINEMA_FREE) cinema_input(CINEMA_HOLD);
	else if (!hold && D.arrival_hold && cinema_input_mode() == CINEMA_HOLD) cinema_input(CINEMA_FREE);
	D.arrival_hold = hold;
	/* the arrival's words once the card has gone; Chaud's call once the
	 * Secret Area's guardian is done */
	talk_update();
	if (!D.area_card && !cinema_busy() && !boss_cinematic() && !boss_fighting() && !talk_busy()) {
		if (D.beat[0] && talk_start(D.beat, FACE_MEGAMAN)) {
			if (run.biome == BIOME_NEST) cinema_shake(30, 3);
			D.guardian_named = D.beat_guardian;
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
	if (boss_take_checkpoint()) D.checkpoint = D.checkpoint_data = true;
	if (D.checkpoint && D.frame >= CHECKPOINT_AFTER && !talk_busy() && !emu_read8(BN6_CHATBOX) &&
		!emu_read8(BN6_DIALOGUE_LOCK) && flag_get(BN6_FLAG_PLAYER_CAN_MOVE)) {
		D.checkpoint = false;
		save_checkpoint();
		/* (said: a playtester who plays in short sessions asked where it
		 * is safe to stop) */
		D.saved_at = D.checkpoint_here ? "Run saved where you saved it" : D.checkpoint_data ? "Run saved at the Guardian Data" :
			"Run saved at the layer's start";
		D.checkpoint_data = D.checkpoint_here = false;
		cinema_note("Run saved", 150);
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
