#include "weekly.h"

#include <stdio.h>
#include <string.h>

#include "audio.h"
#include "backup.h"
#include "game.h"
#include "meta.h"
#include "rivals.h"
#include "run.h"
#include "save.h"
#include "save_blob.h"
#include "save_format.h"

static WeeklyRecord W;
static bool W_read;
int title_dive;

const WeeklyRecord *weekly_record(void) {
	if (!W_read) {
		/* (read in the normal slot's name whatever the slot: save_path
		 * keeps "weekly" names as they are) */
		if (!save_read_blob("weekly.sav", WEEKLY_MAGIC, &W, sizeof W)) memset(&W, 0, sizeof W);
		W_read = true;
	}
	return &W;
}

static void record_write(void) {
	if (!save_write_blob("weekly.sav", WEEKLY_MAGIC, &W, sizeof W)) fprintf(stderr, "weekly: weekly.sav not written\n");
}

bool weekly_on(void) { return save_slot_weekly(); }

/* a dive's act at layer `depth`: three layers an act, the Nest the fourth */
static int weekly_act(int depth) {
	int act = depth >= 1 ? (depth - 1) / 3 + 1 : 0;
	int nest = (SHORT_LAYERS - 1) / 3 + 1;
	return act > nest ? nest : act;
}

/* a dive's summary as a hash, never 0 (the protocol's: a format check alone) */
static uint32_t weekly_hash(uint32_t seed, int depth, int act, bool won, unsigned minutes, int bosses, int viruses) {
	uint32_t v[7] = { seed, (uint32_t)depth, (uint32_t)act, won, minutes, (uint32_t)bosses, (uint32_t)viruses };
	uint32_t h = 2166136261u;
	for (size_t i = 0; i < sizeof v / sizeof *v; ++i)
		for (int b = 0; b < 4; ++b) h = (h ^ (uint8_t)(v[i] >> (8 * b))) * 16777619u;
	return h ? h : 1;
}

/* The profile a dive starts from (docs/ONLINE.md, The week's net: its
 * design): the player's settings and what he has been told, so nothing is
 * explained again; none of what the normal runs earned, which would make
 * one dive stronger or another net than the next player's (the Secret
 * Area's master follows the title's marks, run_new). */
static void standard_profile(void) {
	Profile p = profile;
	p.runs = p.best_depth = p.last_depth = p.bosses = p.viruses = p.secret_clears = p.nest_clears = 0;
	p.first_guardian = p.first_area = p.area_before = p.guardian_before = 0;
	p.threat_open = 0;
	p.folders_open = 0;
	p.short_wins = 0;
	p.crosses_open = 0;
	p.marks = 0;
	p.setup_new = 0;
	memset(p.programs_found, 0, sizeof p.programs_found);
	p.spins = p.spin_colour = 0;
	p.spin_run = 0;
	memset(p.codes_entered, 0, sizeof p.codes_entered);
	p.codes_mailed = 0;
	p.duel_won = p.duel_lost = 0;
	p.duel_run = 0;
	p.duel_depth = 0;
	p.duel_beat = 0;
	p.beast &= (uint8_t)~BEAST_OPEN;
	p.last_lost_to = p.last_won = p.last_job = 0;
	profile = p;
}

/* the slot's files, each of them: a dive begun again on the same seed
 * reads none of the last one's records (they are kept by seed) */
static void slot_clear(void) {
	char file[600];
	save_queue_wait();
	for (int i = 0; i < SAVE_PROGRESS_NAMES; ++i) {
		save_path(file, sizeof file, save_progress_names[i]);
		remove(file);
	}
}

bool weekly_dive(uint16_t week, uint32_t seed) {
	if (!week || !seed) return false;
	weekly_record();
	/* (the profile in play: the normal one's, or a dive's begun again) */
	save_slot(true);
	slot_clear();
	standard_profile();
	profile_save();
	rivals_load();
	run_new(seed);
	run_setup(RUN_SHORT, FOLDER_STANDARD, 0, 0, 0);
	meta_run_begun();
	W.week = week;
	W.seed = seed;
	W.running = 1;
	record_write();
	fprintf(stderr, "weekly: a dive of week %u begun (seed %08x)\n", week, seed);
	emu_start_in_town = true;
	scene_set(&scene_emu);
	return true;
}

bool weekly_continue(void) {
	weekly_record();
	if (!W.running || !W.week) return false;
	int music = profile.music_volume, sfx = profile.sfx_volume;
	save_slot(true);
	profile_load();
	/* (the settings are the player's, whichever slot: the latest) */
	profile.music_volume = music;
	profile.sfx_volume = sfx;
	rivals_load();
	if (!load_run()) {
		/* (none saved yet: the dive begins again, on its own week's net) */
		fprintf(stderr, "weekly: no checkpoint in the slot; the dive begins again\n");
		return weekly_dive(W.week, W.seed);
	}
	emu_resume_requested = true;
	scene_set(&scene_emu);
	return true;
}

/* whether end a (depth, won, minutes) ranks above end b: the server's order */
static bool better(int depth, bool won, unsigned minutes, const WeeklyRecord *b) {
	if (depth != b->r_depth) return depth > b->r_depth;
	if (won != (b->r_won != 0)) return won;
	return minutes < b->r_minutes;
}

void weekly_ended(bool won) {
	weekly_record();
	if (!save_slot_weekly()) return;
	int depth = run_reached();
	if (depth < 1) depth = 1;
	if (depth > SHORT_LAYERS) depth = SHORT_LAYERS;
	unsigned minutes = profile.played_run == run.seed ? (profile.played_frames + 3599u) / 3600u : 0;
	if (minutes > 0xFFFF) minutes = 0xFFFF;
	bool keep = !W.pending || W.r_week != W.week || better(depth, won, minutes, &W);
	if (keep) {
		int act = weekly_act(depth);
		W.r_week = W.week;
		W.r_seed = W.seed;
		W.r_depth = (uint16_t)depth;
		W.r_act = (uint8_t)act;
		W.r_won = won && depth == SHORT_LAYERS;
		W.r_minutes = (uint16_t)minutes;
		W.r_hash = weekly_hash(W.seed, depth, act, W.r_won, minutes, run.bosses_beaten, run.viruses_deleted);
		W.pending = 1;
	}
	W.running = 0;
	record_write();
	title_dive = W.r_depth;
	fprintf(stderr, "weekly: the dive of week %u ended on layer %d%s\n", W.week, depth, won ? ", won" : "");
	weekly_leave();
}

void weekly_leave(void) {
	W_read = false;
	if (!save_slot_weekly()) return;
	int music = profile.music_volume, sfx = profile.sfx_volume;
	save_slot(false);
	profile_load();
	if (!profile.music_volume) profile.music_volume = 9;
	if (!profile.sfx_volume) profile.sfx_volume = 9;
	/* (the volumes as the player last set them; the normal profile's file
	 * keeps its bytes until the normal run writes it) */
	profile.music_volume = music;
	profile.sfx_volume = sfx;
	audio_set_volume(music - 1, sfx - 1);
	rivals_load();
}

bool weekly_normal_saved(void) {
	weekly_leave();
	return save_exists();
}

void weekly_posted(void) {
	weekly_record();
	if (!W.pending) return;
	W.pending = 0;
	record_write();
}
