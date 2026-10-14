#include "online_visit.h"

#include <stdio.h>
#include <string.h>

#include "boot.h"
#include "cinema.h"
#include "director.h"
#include "emu.h"
#include "game.h"
#include "game_hooks.h"
#include "loot.h"
#include "navicust.h"
#include "online_scene.h"
#include "online_transport.h"
#include "online_visit_text.h"
#include "run.h"
#include "save.h"
#include "save_queue.h"
#include "souls.h"

static struct {
	bool active, returning, checkpoint, ready, preserve, released;
	Run before, saved;
	LootMemory battle_before, battle_saved;
	Profile profile;
	uint32_t rng;
	OnlineLoadout loadout;
} visit;

static bool visit_error(char *error, size_t cap, const char *message) {
	if (error && cap) snprintf(error, cap, "%s", message);
	return false;
}

static bool begin_error(char *error, size_t cap, const char *message) {
	run = visit.before;
	profile = visit.profile;
	rng_restore(visit.rng);
	loot_memory_set(&visit.battle_before);
	return visit_error(error, cap, message);
}

bool online_visit_begin(const char *url, const char *name, const uint8_t identity[16], char *error, size_t cap) {
	if (visit.active || visit.returning) return visit_error(error, cap, online_visit_open);
	emu_sync();
	save_queue_wait();
	visit.before = run;
	visit.profile = profile;
	visit.rng = rng_state();
	loot_memory(&visit.battle_before);
	visit.checkpoint = load_run();
	visit.saved = run;
	loot_memory(&visit.battle_saved);
	if (visit.checkpoint) {
		char path[600];
		save_state_path(path, sizeof path);
		FILE *state = fopen(path, "rb");
		if (!state) return begin_error(error, cap, online_visit_save_first);
		fclose(state);
	} else memset(&run, 0, sizeof run);
	if (!online_transport_start(url, name, identity)) {
		online_transport_end();
		return begin_error(error, cap, online_visit_connect_failed);
	}
	visit.active = true;
	visit.ready = false;
	visit.preserve = visit.released = false;
	director_stop();
	emu_resume_requested = emu_start_in_town = emu_start_at_home = false;
	scene_set(&scene_emu);
	return true;
}

bool online_visit_active(void) { return visit.active; }
bool online_visit_preserves_checkpoint(void) { return visit.active || visit.returning || visit.preserve; }
bool online_visit_transitioning(void) { return visit.returning; }

uint32_t online_visit_run_keys(uint32_t keys) {
	if (!visit.preserve) return keys;
	if (!keys) visit.released = true;
	if (!visit.released) return 0;
	if (keys && director_arrived()) visit.preserve = false;
	return keys;
}

static uint16_t bounded_depth(int depth) { return (uint16_t)(depth < 0 ? 0 : depth > UINT16_MAX ? UINT16_MAX : depth); }

void online_visit_profile(OnlineProfile *out) {
	memset(out, 0, sizeof *out);
	if (!visit.ready) return;
	const OnlineLoadout *navi = &visit.loadout;
	out->hp = navi->hp; out->max_hp = navi->max_hp;
	/* NaviStats stores each level less one; the public card shows 1-5. */
	out->attack = navi->attack < 5 ? (uint8_t)(navi->attack + 1u) : 5;
	out->speed = navi->speed < 5 ? (uint8_t)(navi->speed + 1u) : 5;
	out->charge = navi->charge < 5 ? (uint8_t)(navi->charge + 1u) : 5;
	out->souls = navi->souls; out->crosses = navi->crosses;
	out->depth = visit.checkpoint ? bounded_depth(visit.saved.depth) : 0;
	unsigned act = out->depth ? ((out->depth - 1u) % CYCLE_LAYERS) / 3u + 1u : 0;
	/* The short net's second Nest at threat ten remains its fourth act. */
	if (visit.saved.mode == RUN_SHORT && out->depth >= SHORT_LAYERS) act = (SHORT_LAYERS - 1u) / 3u + 1u;
	out->act = (uint8_t)act;
	out->best_depth = bounded_depth(visit.profile.best_depth);
	out->marks = visit.profile.marks;
	for (unsigned i = 0; i < ONLINE_BOARD_CELLS; ++i) {
		unsigned slot = navi->grid[i];
		if (!slot || slot > BN6_NAVICUST_SLOTS) continue;
		const uint8_t *record = navi->board + (slot - 1u) * 8u;
		unsigned variant = record[0] | (unsigned)record[1] << 8;
		NaviShape shape;
		if (variant && variant < 47 * 4 && navicust_shape((int)variant, &shape)) out->board[i] = shape.color;
	}
}

bool online_visit_prepare(char *error, size_t cap) {
	bool ok;
	if (!visit.active) return visit_error(error, cap, online_visit_missing);
	if (visit.checkpoint) {
		char path[600];
		save_state_path(path, sizeof path);
		/* Reject an unreadable state: normal CONTINUE can instead restart
		 * its layer, but a visit must borrow an existing MegaMan exactly. */
		ok = emu_load_state(path);
		if (ok) { game_hooks_after_boot(); ok = director_resume(); }
	} else {
		director_before_boot();
		ok = emu_boot();
		game_hooks_after_boot();
	}
	director_stop();
	cinema_reset();
	if (!ok || !online_snapshot_read(&visit.loadout, visit.checkpoint ? souls_held() : 0))
		return visit_error(error, cap, online_visit_checkpoint_failed);
	visit.ready = true;
	fprintf(stderr, "online: carried checkpoint=%s depth=%d hp=%u max_hp=%u souls=%u\n",
		visit.checkpoint ? "yes" : "no", visit.checkpoint ? visit.saved.depth : 0,
		visit.loadout.hp, visit.loadout.max_hp, visit.loadout.souls);
	online_scene_start();
	return true;
}

static void restore_metadata(void) {
	run = visit.checkpoint ? visit.saved : visit.before;
	profile = visit.profile;
	rng_restore(visit.rng);
	loot_memory_set(visit.checkpoint ? &visit.battle_saved : &visit.battle_before);
}

void online_visit_return(void) {
	if (!visit.active || visit.returning) return;
	emu_sync();
	director_stop();
	online_scene_end();
	restore_metadata();
	visit.active = false;
	visit.returning = true;
	visit.ready = false;
	cinema_reset();
	emu_resume_requested = visit.checkpoint;
	title_menu = true;
	scene_set(visit.checkpoint ? &scene_emu : &scene_title);
}

void online_visit_returned(bool restored) {
	if (!visit.returning) return;
	visit.returning = false;
	OnlineLoadout after;
	bool same = restored && online_snapshot_read(&after, souls_held()) && !memcmp(&after, &visit.loadout, sizeof after);
	restore_metadata();
	visit.preserve = same;
	visit.released = false;
	if (!same) {
		director_stop();
		title_menu = true;
		scene_set(&scene_title);
	}
	fprintf(stderr, "online: checkpoint return=%s carried=%s\n", restored ? "restored" : "failed; save unchanged", same ? "identical" : "changed");
}

void online_visit_left(void) {
	if (visit.returning && !visit.checkpoint) {
		visit.returning = false;
		fprintf(stderr, "online: no checkpoint; returned to title\n");
	} else if (visit.preserve && !visit.returning) {
		/* A deliberate reset to title ends the borrowed checkpoint's
		 * guard before a later NEW GAME can begin. */
		restore_metadata();
		visit.preserve = false;
	}
}

void online_visit_shutdown(void) {
	if (!online_visit_preserves_checkpoint()) return;
	emu_sync();
	director_stop();
	online_scene_end();
	restore_metadata();
	visit.active = visit.returning = visit.ready = visit.preserve = false;
}
