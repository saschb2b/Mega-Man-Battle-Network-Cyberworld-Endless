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
#include "run.h"
#include "runlog.h"
#include "save.h"
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
	bool l_held, r_held;   /* L and R were down last frame */
	int lost_to;           /* the guardian MegaMan was deleted by, 0 none */
	bool nest_cleared;     /* the Nest's guardian fell; the profile counts it at the checkpoint */
} D;

#define AREA_CARD_AT 45   /* frames on the map after arriving */

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
#define LABEL_NAMES 244

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
	uint8_t a[LABEL_NAMES * 2 + 13];
	for (int i = 0; i < LABEL_NAMES; ++i) { a[2 * i] = (uint8_t)(LABEL_NAMES * 2); a[2 * i + 1] = (uint8_t)(LABEL_NAMES * 2 >> 8); }
	/* twelve characters, right-aligned with spaces (the game pads its own
	 * with underscores, which show) */
	char padded[20];
	snprintf(padded, sizeof padded, "%12.12s", name);
	ta_encode(padded, a + LABEL_NAMES * 2, 12);
	a[LABEL_NAMES * 2 + 12] = 0xE6;
	emu_write(LABEL_AT, a, sizeof a);
	emu_write32(BN6_MAP_NAMES_PTR, LABEL_AT);
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
}

/* What MegaMan says when L is pressed: where they are, what is ahead. */
/* Which way the exit pad lies from MegaMan, as the screen shows it (the
 * d-pad's UP moves +X -Y, RIGHT +X +Y: a world step (dx, dy) goes
 * dx + dy across and (dy - dx) / 2 down), and how far. */
static const char *exit_way(int *far) {
	static const char *const ways[8] = {
		"to the right", "down and to the right", "straight down", "down and to the left",
		"to the left", "up and to the left", "straight up", "up and to the right",
	};
	int dx = D.objs.exit_x - ((int)emu_read32(BN6_PLAYER + 0x1C) >> 16);
	int dy = D.objs.exit_y - ((int)emu_read32(BN6_PLAYER + 0x20) >> 16);
	double sx = dx + dy, sy = (dy - dx) / 2.0;
	int panels = (abs(dx) + abs(dy)) / 32;
	*far = panels < 5 ? 0 : panels < 14 ? 1 : 2;
	double a = atan2(sy, sx);   /* (screen y grows downwards) */
	int k = (int)lround(a / (3.14159265358979 / 4));
	return ways[(k % 8 + 8) % 8];
}

static const char *status_words(void) {
	static char buf[400];
	int k = 0;
	#define ADD(...) (k += snprintf(buf + k, k < (int)sizeof buf ? sizeof buf - (size_t)k : 0, __VA_ARGS__))
	if (D.town) {
		ADD("@M The port's by the %s, Lan.|@M Stand next to the statue and press R to jack me in!",
			town_info()->group == 0x00 ? "squirrel statue in the park" : "bird statue on the plaza");
		return buf;
	}
	const char *area = guardian_area_in_text(run.biome, run.side_kind);
	ADD("@M We're on layer %d, Lan. This is %s.", run.depth, area);
	if (D.objs.guardian.navi && !boss_beaten()) ADD("|@M %s is waiting at the end of this layer!", guardian(D.objs.guardian.navi)->name);
	else if (D.objs.guardian.navi && !boss_done()) ADD("|@M Let's take its Guardian Data, Lan!");
	else if (D.objs.guardian.navi) ADD("|@M The exit pad's open. Let's head down!");
	else if (run.side_kind == LAYER_NORMAL) ADD("|@M %s guards the end of this area.", guardian(run.boss_order[run.biome])->name);
	if (run.fragments == 1) ADD("|@M We're carrying one ScrtData.");
	else if (run.fragments > 1) ADD("|@M We're carrying %d ScrtData.", run.fragments);
	/* the way on, as MegaMan senses it */
	int far;
	const char *way = exit_way(&far);
	static const char *const how_far[3] = { "It's close!", "It's a little ways off.", "It's still a long way." };
	if (!D.objs.guardian.navi || boss_done())
		ADD("|@M I can sense the exit pad, Lan. It's %s.|@M %s", way, how_far[far]);
	else if (!boss_beaten())
		ADD("|@M Its arena is %s, Lan.|@M %s", way, how_far[far]);
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
 * own flash (the run keeps its checkpoints). */
static void lock_run(void) {
	flag_set(BN6_FLAG_NO_JACK);
	flag_set(BN6_FLAG_NO_PET_SAVE);
}

/* The next random battle: the run's first two and the first after each
 * guardian from the lower half of the act's band (docs/PROGRESSION.md). */
static void roll_encounter(void) {
	bool opening = run.side_kind == LAYER_NORMAL && layer_in_act(run.depth) == 0 &&
		(run.depth == 1 ? D.battles < 2 : D.battles < 1);
	Encounter e = make_encounter(run.depth, run.biome, opening ? ENC_EASY : ENC_NORMAL);
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
	/* until MegaMan takes it, the exit pad leads back to the layer's start */
	mapslot_exit_to(D.group, D.number, D.start_x, D.start_y, 4);
	D.active = true;
	D.frame = 0;
	D.gameover = false;
	D.act_guardian = D.objs.guardian.navi ? guardian(D.objs.guardian.navi)->name : NULL;
	bool first_of_act = run.side_kind == LAYER_NORMAL && layer_in_act(run.depth) == 0;
	if (first_of_act || run.side_kind != LAYER_NORMAL || biome == BIOME_NEST) begin_area(first_of_act);
	arrival_words();
	D.secret_call = run.side_kind == LAYER_SECRET;
	talk_reset();
	return true;
}

bool director_start_run(void) {
	/* a new run leaves the last one behind: CONTINUE is for runs that
	 * have reached the net */
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
	const TownInfo *ti = town_info();
	emu_warp(ti->group, ti->number, ti->start_x, ti->start_y, ti->start_face);
	D.town = true;
	D.town_seen = false;
	D.town_frames = 0;
	D.intro_said = false;
	return true;
}

bool director_in_town(void) { return D.active && D.town; }

bool director_on_layer(void) { return D.active && !D.town; }

bool director_arrived(void) {
	if (!D.active || !on_map()) return false;
	int group = emu_read8(BN6_GAMESTATE + 4), number = emu_read8(BN6_GAMESTATE + 5);
	return D.town ? group == town_info()->group && number == town_info()->number : group == D.group && number == D.number;
}

void director_describe(FILE *f) {
	if (!D.active) { fprintf(f, "where none\n"); return; }
	int mode = main_mode(), sub = emu_read8(BN6_GAMESTATE);
	const char *doing = mode == BN6_MODE_GAME_OVER ? "gameover"
		: mode != BN6_MODE_GAME ? "menu"
		: sub == BN6_SUB_MAP ? "map" : sub == BN6_SUB_BATTLE || sub == BN6_SUB_BATTLE_INIT ? "battle" : "other";
	fprintf(f, "where %s\ndoing %s\nchat %s\ntalk %s\n", D.town ? "town" : "layer", doing,
		emu_read8(BN6_CHATBOX) ? "open" : "closed", talk_busy() ? "director" : "none");
	fprintf(f, "hp %d/%d\nzenny %u\n", emu_read16(BN6_NAVI_STATS + 0x40), emu_read16(BN6_NAVI_STATS + 0x42),
		(unsigned)emu_read32(BN6_GAMESTATE + 0x5C));
	if (D.town) return;
	fprintf(f, "layer %d\narea %s\nscrtdata %d\n", run.depth, guardian_area_in_text(run.biome, run.side_kind), run.fragments);
	if (D.objs.guardian.navi)
		fprintf(f, "guardian %s %s\n", guardian(D.objs.guardian.navi)->name,
			boss_done() ? "done" : boss_beaten() ? "beaten" : boss_fighting() ? "fighting" : "waiting");
}

uint32_t director_keys(uint32_t keys) {
	bool l = (keys & KEY_L) != 0, pressed = l && !D.l_held;
	bool r = (keys & KEY_R) != 0, r_pressed = r && !D.r_held;
	D.l_held = l;
	D.r_held = r;
	if (!D.active || !on_map()) return keys;   /* (in battle L opens the Custom screen) */
	/* R in the town away from the port: MegaMan says where it is (the game
	 * itself does nothing there) */
	if (D.town && r_pressed && !talk_busy() && !emu_read8(BN6_CHATBOX) && !cinema_busy() && emu_read8(BN6_WARP + 0x10) == 0 &&
		!town_on_port((int)emu_read32(BN6_PLAYER + 0x1C) >> 16, (int)emu_read32(BN6_PLAYER + 0x20) >> 16)) {
		static char buf[160];
		snprintf(buf, sizeof buf, "@M There's no port here, Lan.|@M It's by the %s!",
			town_info()->group == 0x00 ? "squirrel statue in the park" : "bird statue on the plaza");
		talk_start(buf, FACE_MEGAMAN);
		return keys & ~KEY_R;
	}
	/* on the map L is MegaMan's word on where they are: the game's own
	 * has no lines for this story */
	keys &= ~KEY_L;
	/* (not while a warp or the jack-in departs, nor through a guardian's
	 * staging or the battle it has armed) */
	if (pressed && !talk_busy() && !emu_read8(BN6_CHATBOX) && !cinema_busy() && !D.warping && emu_read8(BN6_WARP + 0x10) == 0 &&
		(D.town || (!boss_cinematic() && !boss_fighting())))
		talk_start(status_words(), FACE_MEGAMAN);
	return keys;
}

/* In the town: nothing to watch but the jack-in, whose arrival on the
 * layer's map starts the run as a layer's warp does. */
static void town_update(void) {
	map_label();
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
	if (emu_load_state(path)) {
		lock_run();
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
	if (D.checkpoint && D.frame >= CHECKPOINT_AFTER && !talk_busy() && !emu_read8(BN6_CHATBOX)) {
		D.checkpoint = false;
		char path[600];
		save_state_path(path, sizeof path);
		save_run();
		emu_save_state(path);
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
