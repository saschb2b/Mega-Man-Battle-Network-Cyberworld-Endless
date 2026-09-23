/* The director: the run's structure around the game.
 *
 * A layer is generated (net_gen), built into its area's map (netmap), given
 * its exit pad and Mystery Data (mapslot, npc), and entered through the
 * game's warp. The game then runs everything MegaMan does; the director only
 * watches his position to take him to the next layer, and keeps the next
 * random battle's enemies in step with the depth. */
#include "director.h"

#include <stdlib.h>

#include "battle.h"
#include "bn6.h"
#include "boot.h"
#include "emu.h"
#include "encounter.h"
#include "game.h"
#include "loot.h"
#include "mapslot.h"
#include "net.h"
#include "netmap.h"
#include "npc.h"
#include "rom.h"
#include "run.h"

#define EXIT_REACH 10      /* world units from the exit pad's centre */
#define REROLL_FRAMES 300  /* the next battle's enemies are re-rolled this often */

static struct {
	bool active;
	int group, number;
	int exit_x, exit_y;
	int frame;
	int leaving;           /* frames until the next layer is built */
} D;

static const __typeof__(R.layout->net_area[0]) *area(int biome) {
	return &R.layout->net_area[biome < 0 || biome >= 8 ? 0 : biome];
}

/* The game's 8-byte Mystery Data content: kind 1 chip (code, id), 3 zenny,
 * 5 BugFrags (tested in the game; see docs/ROM_DATA.md). */
static void mystery_content(const NetObj *o, uint8_t out[8]) {
	int roll = rng_range(0, 99);
	char code = '*';
	int kind = 3, value = 100;
	if (o->param == 0) {
		if (roll < 50) { kind = 1; value = roll_chip(run.depth, 0, &code); }
		else if (roll < 85) value = (100 + rng_range(0, 8) * 50) * (1 + run.depth / 6);
		else { kind = 5; value = rng_range(3, 8); }
	} else if (o->param == 1) {
		if (roll < 60) { kind = 1; value = roll_chip(run.depth, 1, &code); }
		else value = 800 + run.depth * 60;
	} else {
		kind = 1;
		value = roll_chip(run.depth, 3, &code);
	}
	out[0] = (uint8_t)kind;
	out[1] = 0x20;
	out[2] = 0xFF;
	out[3] = (uint8_t)(kind == 1 ? (code == '*' ? 26 : code - 'A') : 0xFF);
	out[4] = (uint8_t)value;
	out[5] = (uint8_t)(value >> 8);
	out[6] = out[7] = 0;
}

static int layer_biome(void) {
	if (run.side_kind == LAYER_UNDERNET) return BIOME_UNDERNET;
	if (run.side_kind == LAYER_SECRET) return BIOME_SECRET;
	return biome_for_depth(run.depth);
}

bool director_start_layer(void) {
	int biome = layer_biome();
	run.biome = biome;
	run.layer_seed = run.seed ^ (uint32_t)(run.depth * 2654435761u) ^ (uint32_t)(run.side_kind * 40503u);
	layer_generate(run.layer_seed, run.depth, biome, run.side_kind);
	NetLayout lay = { MAP_W, MAP_H, &layer.cell[0][0] };
	if (!netmap_build(biome, &lay)) return false;

	mapslot_reset();
	NpcList npcs = { { 0 }, 0 };
	MysteryData md[16];
	int nmd = 0;
	int start_x = 0, start_y = 0;
	for (int i = 0; i < layer.nobj; ++i) {
		const NetObj *o = &layer.obj[i];
		int wx, wy;
		netmap_world((int)o->x, (int)o->y, &wx, &wy);
		switch (o->type) {
		case OBJ_WARP_IN:
			start_x = wx; start_y = wy;
			break;
		case OBJ_EXIT:
		case OBJ_RETURN:
			D.exit_x = wx; D.exit_y = wy;
			if (npcs.n < 32) npcs.script[npcs.n++] = npc_prop(7, 0x22, wx, wy, 0, 0);
			break;
		case OBJ_MYSTERY:
			if (nmd < 16 && npcs.n < 32) {
				md[nmd].x = wx;
				md[nmd].y = wy;
				mystery_content(o, md[nmd].content);
				npcs.script[npcs.n++] = npc_mystery(nmd);
				++nmd;
			}
			break;
		default:
			break;
		}
	}
	const __typeof__(R.layout->net_area[0]) *a = area(biome);
	D.group = a->group;
	D.number = a->number;
	if (!mapslot_install(D.group, D.number, &npcs, md, nmd)) return false;

	Encounter e = make_encounter(run.depth, biome, false, false);
	emu_encounter_set(&e);
	emu_warp(D.group, D.number, start_x, start_y, 4);
	D.active = true;
	D.leaving = 0;
	D.frame = 0;
	return true;
}

void director_update(void) {
	if (!D.active) return;
	++D.frame;
	if (D.leaving > 0) {
		if (--D.leaving == 0) director_start_layer();
		return;
	}
	/* in the net (not in a battle or menu): the player object is on this map */
	if (emu_read8(BN6_GAMESTATE + 4) != D.group || emu_read8(BN6_GAMESTATE + 5) != D.number) return;
	int x = (int)emu_read32(BN6_PLAYER + 0x1C) >> 16, y = (int)emu_read32(BN6_PLAYER + 0x20) >> 16;
	if (abs(x - D.exit_x) <= EXIT_REACH && abs(y - D.exit_y) <= EXIT_REACH) {
		if (run.side_kind == LAYER_NORMAL) run.depth++;
		else run.side_kind = LAYER_NORMAL;
		D.leaving = 1;
		return;
	}
	if (D.frame % REROLL_FRAMES == 0) {
		Encounter e = make_encounter(run.depth, run.biome, false, false);
		emu_encounter_set(&e);
	}
}
