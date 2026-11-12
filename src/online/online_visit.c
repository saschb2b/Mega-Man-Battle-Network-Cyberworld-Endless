#include "online_visit.h"

#include <stdio.h>
#include <string.h>

#include "analytics_online.h"
#include "cinema.h"
#include "director.h"
#include "director_gate.h"
#include "emu.h"
#include "game.h"
#include "loot.h"
#include "netpass.h"
#include "online_field.h"
#include "online_scene.h"
#include "online_trade.h"
#include "online_transport.h"
#include "platform.h"
#include "save.h"
#include "souls.h"
#include "square_pet.h"
#include "square_arena.h"
#include "square_watch.h"
#include "square_week.h"

#define CALL_TIMEOUT_MS 20000   /* a server that never answers */

static struct {
	VisitStage stage;
	VisitEnd ended;
	uint32_t called_at, arrived_at;
	bool official;        /* the gate's official server (the statistics' flag) */
	/* (what the run held at the gate, put back as MegaMan comes home) */
	Run run;
	Profile profile;
	uint32_t rng;
	LootMemory battle;
	OnlineLoadout loadout;
	bool carried;
	unsigned souls;       /* the Souls held at the gate, a run's record (the carried MegaMan read again) */
	const char *news;     /* MegaMan's word on a trade settled, till said */
	char name[ONLINE_NAME_BYTES + 1];          /* the name this game goes by in the Square */
	char note[ONLINE_MAINTENANCE_BYTES + 1];   /* the server's line with the end (maintenance) */
} V;

/* A trade the server committed, given to the run on the checkpoint just
 * loaded, then saved there: the file kept till then goes first, so a crash
 * between costs this side the trade, never makes a chip twice */
static void settle(void) {
	/* (first what BN6's PET changed in the Square, square_pet.h, written onto the checkpoint: a folder made there
	 * is the run's; a trade the Square made in its own game comes with it, and is not made again) */
	bool traded = false, pet = square_pet_home(&traded);
	const char *words;
	bool trade = online_trade_settle(traded, &words);
	if (trade) online_trade_settled();
	if (!pet && !trade) return;
	if (!home_gate_save()) fprintf(stderr, "online: the checkpoint after the visit's changes was not written\n");
	if (words) V.news = words;
}

void online_visit_resumed(void) { settle(); }

const char *online_visit_news(void) {
	const char *n = V.news;
	V.news = NULL;
	return n;
}

void online_visit_news_again(const char *words) { V.news = words; }

VisitStage online_visit_stage(void) { return V.stage; }

/* The statistics' visit over (analytics_online.h): how, the NetPass (the
 * full set's, every stamp: a full pass of the Square's and Trade's on a
 * system of one extra ROM counts with the Square's, as the server's daily
 * totals count it), the time in the Square where MegaMan got there (below
 * 0 none) */
static void counted(VisitEnd why, bool away) {
	analytics_visit((int)why, netpass_stamps() == STAMP_COUNT, away ? (int)((SDL_GetTicks() - V.arrived_at) / 1000) : -1, V.official);
}

bool online_visit_begin(const char *url, const char *name, const uint8_t identity[ONLINE_IDENTITY_BYTES], bool official) {
	if (V.stage != VISIT_HOME) return false;
	V.official = official;
	V.run = run;
	V.profile = profile;
	V.rng = rng_state();
	loot_memory(&V.battle);
	V.carried = false;
	V.ended = VISIT_END_NONE;
	V.note[0] = 0;
	snprintf(V.name, sizeof V.name, "%s", name);
	if (!online_transport_start(url, name, identity)) {
		online_transport_end();
		V.ended = VISIT_END_UNREACHABLE;
		counted(V.ended, false);
		return false;
	}
	V.stage = VISIT_CALLING;
	V.called_at = SDL_GetTicks();
	fprintf(stderr, "online: calling %s as %s\n", url, name);
	return true;
}

/* What a server's refusal (its code, shared/online/protocol.json) means at
 * the gate; `away`: it came in the Square */
VisitEnd online_visit_refusal(const OnlineSnapshot *s, bool away) {
	switch (s->error_code) {
	/* (2 and 3: the game older than the server; 15: BN5's identity not the
	 * one the server knows, which this game checked as it read the ROM: the
	 * server's build is another) */
	case 2: case 3: case 15: return VISIT_END_VERSION;
	case 4: return VISIT_END_ROM;
	case 5: return VISIT_END_NAME;
	case 7: return VISIT_END_FULL;
	case 10: case 11: return VISIT_END_BARRED;
	case 14: return VISIT_END_REMOVED;
	case ONLINE_ERROR_SERVER_OLDER: return VISIT_END_OLD_SERVER;
	case ONLINE_ERROR_MAINTENANCE:
		SDL_strlcpy(V.note, s->error, sizeof V.note);
		return VISIT_END_MAINTENANCE;
	default: return away ? VISIT_END_LOST : VISIT_END_UNREACHABLE;
	}
}

static void put_back(void) {
	run = V.run;
	profile = V.profile;
	rng_restore(V.rng);
	loot_memory_set(&V.battle);
}

/* The connection closed, and MegaMan back at the gate's checkpoint (from
 * the Square: loaded again, the layer and home rebuilt as CONTINUE does) */
static void come_home(VisitEnd why) {
	bool away = V.stage == VISIT_AWAY;
	online_scene_end();
	online_transport_end();
	counted(why, away);
	V.stage = VISIT_HOME;
	V.ended = why;
	put_back();
	fprintf(stderr, "online: visit ended (%d)\n", (int)why);
	if (!away) return;
	emu_sync();
	cinema_reset();
	/* (the week's net: the gate's checkpoint stays the normal run's, and
	 * the dive begins, or goes on, in its own slot; weekly.h) */
	if (why == VISIT_END_DIVE) {
		/* (what BN6's PET changed in the Square waits in its file for the run's next CONTINUE: square_pet.h) */
		director_stop();
		V.ended = VISIT_END_NONE;
		if (square_week_dive()) return;
		fprintf(stderr, "online: the dive did not begin; back to the gate\n");
		V.ended = VISIT_END_BROKEN;
	}
	if (!director_resume()) {
		/* (the gate's checkpoint is still on disk: CONTINUE tries it again) */
		fprintf(stderr, "online: the gate's checkpoint did not load; to the title\n");
		director_stop();
		title_menu = true;
		scene_set(&scene_title);
		return;
	}
	/* (put back after the resume too: it reads the run's side records;
	 * and no CONTINUE's note: the gate says what happened) */
	put_back();
	cinema_reset();
	settle();
}

/* Into the Square: the run's director stopped, the MegaMan carried read,
 * the Square built into the running game and MegaMan warped there */
static void go_away(const OnlineSnapshot *s) {
	emu_sync();
	V.souls = souls_held();
	V.carried = online_snapshot_read(&V.loadout, V.souls);
	director_stop();
	cinema_reset();
	V.stage = VISIT_AWAY;
	V.arrived_at = SDL_GetTicks();
	OnlineProfile card;
	online_visit_profile(&card);
	if (!online_scene_start(s->seed, s->player_id, &card)) come_home(VISIT_END_BROKEN);
}

void online_visit_update(void) {
	if (V.stage == VISIT_HOME) return;
	if (V.stage == VISIT_AWAY) {
		VisitEnd why = online_scene_update();
		if (why != VISIT_END_NONE) come_home(why);
		return;
	}
	OnlineSnapshot s;
	online_transport_snapshot(&s);
	if (s.error[0]) come_home(online_visit_refusal(&s, false));
	else if (s.connected) go_away(&s);
	else if (SDL_GetTicks() - V.called_at > CALL_TIMEOUT_MS) come_home(VISIT_END_UNREACHABLE);
}

bool online_visit_waiting(void) { return V.stage == VISIT_AWAY && online_scene_waiting(); }

void online_visit_wait_frame(void) {
	VisitEnd why = online_scene_wait();
	if (why != VISIT_END_NONE) come_home(why);
}

uint32_t online_visit_keys(uint32_t keys) {
	if (V.stage == VISIT_AWAY) return online_scene_keys(keys);
	if (V.stage != VISIT_CALLING) return keys;
	if (btn_pressed(BTN_B)) come_home(VISIT_END_CANCELLED);
	return 0;
}

bool online_visit_held(void) { return V.stage == VISIT_AWAY && (square_arena_on() || square_watch_on()); }

/* (a battle of this game's, or one watched from the stands) */
void online_visit_held_frame(uint32_t keys) {
	if (square_arena_on()) square_arena_frame(keys);
	else square_watch_frame(keys);
	/* (a Team NetBattle's next bout, or the others' bout watched) */
	square_arena_held_frame();
}

void online_visit_draw(void) {
	if (online_visit_held() && square_arena_on()) square_arena_draw();
	else if (online_visit_held()) square_watch_draw();
	else if (V.stage == VISIT_AWAY) online_scene_draw();
	/* (the gate's fields, over the game in Lan's HP) */
	else online_field_draw();
}

void online_visit_leave(VisitEnd why) {
	if (V.stage != VISIT_HOME) come_home(why);
}

VisitEnd online_visit_ended(void) {
	VisitEnd why = V.ended;
	V.ended = VISIT_END_NONE;
	return why;
}

const char *online_visit_end_note(void) { return V.note; }

void online_visit_shutdown(void) {
	if (V.stage == VISIT_HOME) return;
	emu_sync();
	online_scene_end();
	online_transport_end();
	counted(VISIT_END_NONE, V.stage == VISIT_AWAY);
	put_back();
	V.stage = VISIT_HOME;
}

const char *online_visit_name(void) { return V.name; }

void online_visit_carried_again(void) {
	if (V.stage != VISIT_AWAY) return;
	V.carried = online_snapshot_read(&V.loadout, V.souls);
	OnlineProfile card;
	online_visit_profile(&card);
	online_scene_card(&card);
}

const OnlineLoadout *online_visit_loadout(void) { return V.stage == VISIT_AWAY && V.carried ? &V.loadout : NULL; }

void online_visit_profile(OnlineProfile *out) {
	memset(out, 0, sizeof *out);
	if (V.carried) online_snapshot_card(&V.loadout, V.run.depth, V.run.mode == RUN_SHORT, V.profile.best_depth, V.profile.marks, out);
}

bool online_visit_square(void) { return V.stage == VISIT_AWAY && !square_pet_open(); }
