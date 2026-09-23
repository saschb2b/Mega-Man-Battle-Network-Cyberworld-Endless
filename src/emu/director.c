/* The director: the run's structure around the game.
 *
 * A layer is generated (net_gen), built into its area's map (netmap), given
 * its exit pad and Mystery Data (mapslot, npc), and entered through the
 * game's warp. The game then runs everything MegaMan does; the director only
 * watches his position to take him to the next layer, and keeps the next
 * random battle's enemies in step with the depth. */
#include "director.h"

#include <stdio.h>
#include <stdlib.h>

#include "foes.h"
#include "bn6.h"
#include "boot.h"
#include "emu.h"
#include "encounter.h"
#include "game.h"
#include "layer_objs.h"
#include "loot.h"
#include "net.h"
#include "netmap.h"
#include "rom.h"
#include "run.h"
#include "save.h"
#include "scripts.h"

int director_debug_biome = -1;

#define EXIT_REACH 10      /* world units from the exit pad's centre */
#define REROLL_FRAMES 300  /* the next battle's enemies are re-rolled this often */

static struct {
	bool active;
	int group, number;
	int exit_x, exit_y;
	int frame;
	int leaving;           /* frames until the next layer is built */
	bool checkpoint;       /* save once MegaMan has arrived */
	bool gameover;         /* the game's GAME OVER is playing */
	bool boss_pending;     /* the boss battle was started from the exit */
	int start_x, start_y;
	LayerObjs objs;
	unsigned chosen;       /* choices already acted on (bit per choice) */
	bool challenge;        /* a challenge battle was started */
	bool in_battle;        /* a battle is on */
	int foes;              /* viruses in the battle the game will start next */
} D;

/* The next battle's enemies, for the game's encounter roll. */
static void set_encounter(const Encounter *e, bool force) {
	D.foes = e->nfoes;
	if (force) emu_battle_force(e);
	else emu_encounter_set(e);
}

#define CHECKPOINT_AFTER  60     /* frames after a layer is entered */

static int main_mode(void) { return emu_read8(emu_read32(BN6_TOOLKIT)); }
/* walking the net: the game mode on its map sub-mode (not a battle or menu) */
static bool on_map(void) { return main_mode() == BN6_MODE_GAME && emu_read8(BN6_GAMESTATE) == BN6_SUB_MAP; }

static bool flag_set(int flag) { return emu_read8(BN6_EVENT_FLAGS + (uint32_t)flag / 8u) & (0x80u >> (flag & 7)); }
static int key_item(int id) { return emu_read8(emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_KEY_ITEMS) + (uint32_t)id); }

static const __typeof__(R.layout->net_area[0]) *area(int biome) {
	return &R.layout->net_area[biome < 0 || biome >= 8 ? 0 : biome];
}

static int layer_biome(void) {
	if (director_debug_biome >= 0 && director_debug_biome < BIOME_COUNT) return director_debug_biome;
	if (run.side_kind == LAYER_UNDERNET) return BIOME_UNDERNET;
	if (run.side_kind == LAYER_SECRET) return BIOME_SECRET;
	return biome_for_depth(run.depth);
}

static bool build_layer(void) {
	int biome = layer_biome();
	run.biome = biome;
	run.layer_seed = run.seed ^ (uint32_t)(run.depth * 2654435761u) ^ (uint32_t)(run.side_kind * 40503u);
	layer_generate(run.layer_seed, run.depth, biome, run.side_kind);
	NetLayout lay = { MAP_W, MAP_H, &layer.cell[0][0] };
	if (!netmap_build(biome, &lay)) return false;

	const __typeof__(R.layout->net_area[0]) *a = area(biome);
	D.group = a->group;
	D.number = a->number;
	if (!layer_objs_install(D.group, D.number, &D.objs)) return false;
	D.exit_x = D.objs.exit_x;
	D.exit_y = D.objs.exit_y;
	D.chosen = 0;

	Encounter e = make_encounter(run.depth, biome, false, false);
	set_encounter(&e, false);
	D.start_x = D.objs.start_x;
	D.start_y = D.objs.start_y;
	D.active = true;
	D.leaving = 0;
	D.frame = 0;
	D.gameover = false;
	return true;
}

bool director_start_layer(void) {
	if (!build_layer()) return false;
	emu_warp(D.group, D.number, D.start_x, D.start_y, 4);
	D.checkpoint = true;
	return true;
}

bool director_exit_panel(int *x, int *y) {
	if (!D.active) return false;
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].type == OBJ_EXIT || layer.obj[i].type == OBJ_RETURN) { *x = (int)layer.obj[i].x; *y = (int)layer.obj[i].y; return true; }
	return false;
}

bool director_resume(void) {
	/* the layer's tables live in the ROM copy, which a state does not hold */
	if (!build_layer()) return false;
	char path[600];
	save_state_path(path, sizeof path);
	if (emu_load_state(path)) {
		/* choices made before the checkpoint stay made */
		for (int i = 0; i < D.objs.nchoices; ++i)
			if (flag_set(D.objs.choice[i].flag)) D.chosen |= 1u << i;
		/* enter the map again where MegaMan stood: the game reloads its NPCs
		 * and tiles from this build's tables, which a state does not hold */
		int x = (int)emu_read32(BN6_PLAYER + 0x1C) >> 16, y = (int)emu_read32(BN6_PLAYER + 0x20) >> 16;
		emu_warp(D.group, D.number, x, y, 4);
		return true;
	}
	/* no state (a run from before the game engine): enter the layer fresh */
	emu_warp(D.group, D.number, D.start_x, D.start_y, 4);
	D.checkpoint = true;
	return true;
}

/* A Yes in a layer's choice: a challenge battle, or into a side layer. */
static bool act_on_choices(void) {
	if (emu_read8(BN6_CHATBOX)) return false;   /* once the chat box has closed */
	for (int i = 0; i < D.objs.nchoices; ++i) {
		if ((D.chosen & (1u << i)) || !flag_set(D.objs.choice[i].flag)) continue;
		D.chosen |= 1u << i;
		switch (D.objs.choice[i].type) {
		case OBJ_CHALLENGE: {
			Encounter e = make_encounter(run.depth + 3, run.biome, true, true);
			set_encounter(&e, true);
			D.challenge = true;
			return true;
		}
		case OBJ_UNDERNET:
			run.side_kind = LAYER_UNDERNET;
			D.leaving = 1;
			return true;
		case OBJ_SECRET_GATE:
			run.side_kind = LAYER_SECRET;
			D.leaving = 1;
			return true;
		default:
			break;
		}
	}
	return false;
}

static void end_run(void) {
	profile_record_run();
	save_delete();
	run.active = false;
	D.active = false;
	title_summary = true;
	scene_set(&scene_title);
}

void director_update(void) {
	if (!D.active) return;
	++D.frame;
	/* MegaMan deleted: the game plays its GAME OVER, then the run ends */
	int mode = main_mode();
	if (mode == BN6_MODE_GAME_OVER) D.gameover = true;
	if (D.gameover) {
		if (mode == BN6_MODE_START_SCREEN) end_run();
		return;
	}
	if (!on_map()) {
		int sub = emu_read8(BN6_GAMESTATE);
		if (sub == BN6_SUB_BATTLE_INIT || sub == BN6_SUB_BATTLE) {
			emu_battle_release();   /* the forced battle has begun */
			D.in_battle = true;
		}
		return;
	}
	if (D.in_battle) {
		/* back from a battle: count the deleted viruses (a navi counts below) */
		D.in_battle = false;
		if (emu_read8(BN6_BATTLE_RESULT) == 1 && !D.boss_pending) run.viruses_deleted += D.foes;
	}
	if (D.boss_pending && !emu_battle_forcing()) {
		/* back from the boss battle */
		D.boss_pending = false;
		if (emu_read8(BN6_BATTLE_RESULT) == 1) {
			layer.boss_beaten = true;
			run.bosses_beaten++;
			if (run.side_kind == LAYER_SECRET) run.secret_cleared = true;
		}
	}
	if (D.challenge && !emu_battle_forcing()) {
		/* back from the challenge (the game gave its reward): random battles again */
		D.challenge = false;
		Encounter e = make_encounter(run.depth, run.biome, false, false);
		set_encounter(&e, false);
	}
	run.fragments = key_item(SCRIPTS_SECRET_DATA);
	if (act_on_choices()) return;
	if (D.checkpoint && D.frame >= CHECKPOINT_AFTER) {
		D.checkpoint = false;
		char path[600];
		save_state_path(path, sizeof path);
		save_run();
		emu_save_state(path);
	}
	if (D.leaving > 0) {
		if (--D.leaving == 0) director_start_layer();
		return;
	}
	/* in the net (not in a battle or menu): the player object is on this map */
	if (emu_read8(BN6_GAMESTATE + 4) != D.group || emu_read8(BN6_GAMESTATE + 5) != D.number) return;
	int x = (int)emu_read32(BN6_PLAYER + 0x1C) >> 16, y = (int)emu_read32(BN6_PLAYER + 0x20) >> 16;
	if (abs(x - D.exit_x) <= EXIT_REACH && abs(y - D.exit_y) <= EXIT_REACH) {
		/* a boss layer's navi guards the exit: the game's own navi battle */
		if (layer.boss_layer && !layer.boss_beaten) {
			if (!D.boss_pending) {
				Encounter e = make_boss(run.depth, run.biome, layer.boss_navi);
				set_encounter(&e, true);
				D.boss_pending = true;
			}
			return;
		}
		/* a side layer's exit leads one area deeper too */
		run.depth++;
		run.side_kind = LAYER_NORMAL;
		D.leaving = 1;
		return;
	}
	if (D.frame % REROLL_FRAMES == 0) {
		Encounter e = make_encounter(run.depth, run.biome, false, false);
		set_encounter(&e, false);
	}
}
