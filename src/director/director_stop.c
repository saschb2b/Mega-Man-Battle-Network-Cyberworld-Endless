/* The stop on the way into an act (docs/world/bn5-town.md): on the act's
 * pink pad in Lan's HP, where the older Net dresses its area, MegaMan asks
 * whether to go in from Oran Isle, the cursor on No (No: the pad's warp, as
 * ever). Yes: the island installed in Seaside Town's map (town.c's Oran
 * style), BN6's own jack-out to it, Lan set down on the pier, the run saved
 * there; its jack-in point starts the act's first layer, built at the exit
 * as the pad's. Once a run at most (run.stop), in half the runs
 * (run_stop.c). */
#include "director_stop.h"

#include <stdio.h>
#include <string.h>

#include "analytics.h"
#include "bn5_world.h"
#include "bn6.h"
#include "debug.h"
#include "devtools.h"
#include "director_home.h"
#include "director_state.h"
#include "emu.h"
#include "flags.h"
#include "gamecall.h"
#include "jobs.h"
#include "lanhp.h"
#include "net.h"
#include "oran.h"
#include "rom.h"
#include "run.h"
#include "run_stop.h"
#include "runlog.h"
#include "talk.h"
#include "town.h"
#include "town_lines.h"

#define PAD_ENTRY 2   /* the pink pad's warp entry (lanhp.c's portal 0) */
#define STOP_OPEN 0   /* offered to players (by the coin), not the dev switch alone */

static struct {
	bool said, saved, job_seen;
	int frames, decisions;
} S;

/* the act's way on (the pink pad's) runs through an area BN5 dresses */
static bool older_way(void) {
	int n = 1;
	bool sealed = false;
	const RunWay *w = home_ways(&n, &sealed);
	const NetAreaDef *a = net_area_def(run_dress(w[0].biome));
	return a && a->xrom == 1 + XROM_BN5_COLONEL_US;
}

bool stop_due(void) {
	if (!D.home || D.home_back || (run.stop & STOP_SPENT) || run.side_kind != LAYER_NORMAL || !XR[XROM_BN5_COLONEL_US].data) return false;
	if (dev.stop) return true;
	/* (players are offered it once the trip lands on the pier: it lands in the
	 * act's first layer yet, docs/world/bn5-town.md, As built) */
	if (!STOP_OPEN) return false;
	int acts = run.mode == RUN_SHORT ? 3 : 6, act = (run.depth - 1) % CYCLE_LAYERS / 3;
	return run_extra(EXTRA_OLDER) && run_stop_coin(run.seed) && run_stop_fits(act, acts, older_way());
}

bool stop_ask(void) {
	flag_clear(HOME_LINK_FLAG);
	return talk_ask(oran_portal_words(), FACE_MEGAMAN, oran_portal_question(), HOME_LINK_FLAG);
}

bool stop_here(void) { return (run.stop & STOP_HERE) != 0; }

/* the island in Seaside Town's map, its jack-in to the act's first layer */
static bool install(void) {
	int n = 1;
	bool sealed = false;
	const RunWay *w = home_ways(&n, &sealed);
	oran_prepare(w[0].biome, w[0].navi, run.depth, run.seed);
	return town_plan_style(town_seed(run.seed), TOWN_ORAN) && town_install(D.group, D.number, D.start_x, D.start_y);
}

/* BN6's own jack-out (internet to real world) to the pier: the saved
 * real-world place is where its map-enter code sets Lan down */
static void jack_out_to_pier(void) {
	const TownInfo *ti = town_info();
	emu_write32(BN6_SAVED_X, (uint32_t)ti->start_x << 16);
	emu_write32(BN6_SAVED_Y, (uint32_t)ti->start_y << 16);
	emu_write32(BN6_SAVED_Z, (uint32_t)BN5_ORAN_DOCK_Z << 16);
	emu_write32(BN6_SAVED_FACING, (uint32_t)ti->start_face);
	emu_write32(BN6_SAVED_MAP, (uint32_t)ti->group | (uint32_t)ti->number << 8);
	flag_set(BN6_FLAG_NAVI_IN_PET);
	flag_clear(BN6_FLAG_NO_JACK);
	/* (the pad's own entry led to the act's layer: for the trip, the pier; hp_ready sets it back at the next visit) */
	lanhp_portal(0, ti->group, ti->number, ti->start_x, ti->start_y, ti->start_face);
	emu_warp_jack_out(PAD_ENTRY);
}

void stop_answer(bool yes) {
	run.stop |= STOP_SPENT;
	memset(&S, 0, sizeof S);
	S.decisions = 1;
	if (emu_debug_on()) fprintf(stderr, "stop: Oran Isle %s\n", yes ? "taken" : "passed by");
	if (!yes || !install()) {
		emu_warp_link(PAD_ENTRY);
		return;
	}
	run.stop |= STOP_HERE;
	flag_clear(ORAN_STALL_FLAG);
	flag_clear(ORAN_JOB_FLAG);
	D.intro_said = true;
	D.errands_told = true;
	jack_out_to_pier();
}

bool stop_rebuild(void) {
	if (!stop_here()) return true;
	S.said = S.saved = true;
	return install();
}

/* the island's job taken: a request of the act, as the lab's (jobs.c),
 * won battles without a scratch, paid after the act */
static void job_taken(void) {
	Job offers[JOB_ASKERS];
	jobs_offers(run.seed, run.depth, offers);
	run.job = offers[JOB_LAB];
	run.job.kind = JOB_CLEAN;
	run.job.need = ORAN_JOB_BATTLES;
	run.job.got = 0;
	run.job.state = JOB_TAKEN;
	runlog_choice(CHOICE_JOB);
}

void stop_frame(void) {
	if (!stop_here()) return;
	const TownInfo *ti = town_info();
	bool there = emu_read8(BN6_MAP_GROUP) == ti->group && emu_read8(BN6_MAP_NUMBER) == ti->number;
	if (there) ++S.frames;
	if (there && !S.said && S.frames > 40 && !emu_read8(BN6_WARP_PENDING) && talk_start(oran_arrival_words(), FACE_MEGAMAN)) S.said = true;
	if (there && S.said && !S.saved && !talk_busy() && !emu_read8(BN6_CHATBOX)) {
		S.saved = true;
		D.home_save_due = true;
	}
	if (flag_get(ORAN_JOB_FLAG) && !S.job_seen) {
		S.job_seen = true;
		++S.decisions;
		if (run.job.kind == JOB_NONE || run.job.state != JOB_TAKEN) job_taken();
	}
	static bool bought;
	if (flag_get(ORAN_STALL_FLAG) != bought) { bought = !bought; S.decisions += bought; }
	/* (the act's first layer reached by the island's jack-in) */
	if (!there && S.frames && emu_read8(BN6_MAP_GROUP) == D.group && emu_read8(BN6_MAP_NUMBER) == D.number) {
		run.stop &= (uint8_t)~STOP_HERE;
		if (emu_debug_on()) fprintf(stderr, "stop: %d frames (%.1f s) on Oran Isle, %d decisions\n", S.frames, S.frames / 60.0, S.decisions);
	}
}
