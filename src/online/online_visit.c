#include "online_visit.h"

#include <stdio.h>
#include <string.h>

#include "cinema.h"
#include "director.h"
#include "director_gate.h"
#include "emu.h"
#include "game.h"
#include "loot.h"
#include "navicust.h"
#include "online_field.h"
#include "online_scene.h"
#include "online_snapshot.h"
#include "online_trade.h"
#include "online_transport.h"
#include "platform.h"
#include "run.h"
#include "save.h"
#include "souls.h"
#include "square_arena.h"

#define CALL_TIMEOUT_MS 20000   /* a server that never answers */

static struct {
	VisitStage stage;
	VisitEnd ended;
	uint32_t called_at;
	/* (what the run held at the gate, put back as MegaMan comes home) */
	Run run;
	Profile profile;
	uint32_t rng;
	LootMemory battle;
	OnlineLoadout loadout;
	bool carried;
	const char *news;     /* MegaMan's word on a trade settled, till said */
} V;

/* A trade the server committed, given to the run on the checkpoint just
 * loaded, then saved there: the file kept till then goes first, so a crash
 * between costs this side the trade, never makes a chip twice */
static void settle(void) {
	const char *words = online_trade_settle();
	if (!words) return;
	online_trade_settled();
	if (!home_gate_save()) fprintf(stderr, "online: the checkpoint after the trade was not written\n");
	V.news = words;
}

void online_visit_resumed(void) { settle(); }

const char *online_visit_news(void) {
	const char *n = V.news;
	V.news = NULL;
	return n;
}

void online_visit_news_again(const char *words) { V.news = words; }

VisitStage online_visit_stage(void) { return V.stage; }

bool online_visit_begin(const char *url, const char *name, const uint8_t identity[ONLINE_IDENTITY_BYTES]) {
	if (V.stage != VISIT_HOME) return false;
	V.run = run;
	V.profile = profile;
	V.rng = rng_state();
	loot_memory(&V.battle);
	V.carried = false;
	V.ended = VISIT_END_NONE;
	if (!online_transport_start(url, name, identity)) {
		online_transport_end();
		V.ended = VISIT_END_UNREACHABLE;
		return false;
	}
	V.stage = VISIT_CALLING;
	V.called_at = SDL_GetTicks();
	fprintf(stderr, "online: calling %s as %s\n", url, name);
	return true;
}

/* What a server's refusal (its code, shared/online/protocol.json) means at
 * the gate; `away`: it came in the Square */
static VisitEnd refusal(const OnlineSnapshot *s, bool away) {
	switch (s->error_code) {
	/* (15: BN5's identity not the one the server knows, which this game
	 * checked as it read the ROM: the server's build is another) */
	case 2: case 3: case 15: return VISIT_END_VERSION;
	case 4: return VISIT_END_ROM;
	case 5: return VISIT_END_NAME;
	case 7: return VISIT_END_FULL;
	case 10: case 11: return VISIT_END_BARRED;
	case 14: return VISIT_END_REMOVED;
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
	V.stage = VISIT_HOME;
	V.ended = why;
	put_back();
	fprintf(stderr, "online: visit ended (%d)\n", (int)why);
	if (!away) return;
	emu_sync();
	cinema_reset();
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
	V.carried = online_snapshot_read(&V.loadout, souls_held());
	director_stop();
	cinema_reset();
	V.stage = VISIT_AWAY;
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
	if (s.error[0]) come_home(refusal(&s, false));
	else if (s.connected) go_away(&s);
	else if (SDL_GetTicks() - V.called_at > CALL_TIMEOUT_MS) come_home(VISIT_END_UNREACHABLE);
}

uint32_t online_visit_keys(uint32_t keys) {
	if (V.stage == VISIT_AWAY) return online_scene_keys(keys);
	if (V.stage != VISIT_CALLING) return keys;
	if (btn_pressed(BTN_B)) come_home(VISIT_END_CANCELLED);
	return 0;
}

bool online_visit_held(void) { return V.stage == VISIT_AWAY && square_arena_on(); }

void online_visit_held_frame(uint32_t keys) { square_arena_frame(keys); }

void online_visit_draw(void) {
	if (online_visit_held()) square_arena_draw();
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

void online_visit_shutdown(void) {
	if (V.stage == VISIT_HOME) return;
	emu_sync();
	online_scene_end();
	online_transport_end();
	put_back();
	V.stage = VISIT_HOME;
}

const OnlineLoadout *online_visit_loadout(void) { return V.stage == VISIT_AWAY && V.carried ? &V.loadout : NULL; }

static uint16_t bounded_depth(int depth) { return (uint16_t)(depth < 0 ? 0 : depth > UINT16_MAX ? UINT16_MAX : depth); }

void online_visit_profile(OnlineProfile *out) {
	memset(out, 0, sizeof *out);
	if (!V.carried) return;
	const OnlineLoadout *navi = &V.loadout;
	out->hp = navi->hp; out->max_hp = navi->max_hp;
	/* NaviStats stores each level less one; the public card shows 1-5. */
	out->attack = navi->attack < 5 ? (uint8_t)(navi->attack + 1u) : 5;
	out->speed = navi->speed < 5 ? (uint8_t)(navi->speed + 1u) : 5;
	out->charge = navi->charge < 5 ? (uint8_t)(navi->charge + 1u) : 5;
	out->souls = navi->souls; out->crosses = navi->crosses;
	out->depth = bounded_depth(V.run.depth);
	unsigned act = out->depth ? ((out->depth - 1u) % CYCLE_LAYERS) / 3u + 1u : 0;
	/* The short net's second Nest at threat ten remains its fourth act. */
	if (V.run.mode == RUN_SHORT && out->depth >= SHORT_LAYERS) act = (SHORT_LAYERS - 1u) / 3u + 1u;
	out->act = (uint8_t)act;
	out->best_depth = bounded_depth(V.profile.best_depth);
	out->marks = V.profile.marks;
	for (unsigned i = 0; i < ONLINE_BOARD_CELLS; ++i) {
		unsigned slot = navi->grid[i];
		if (!slot || slot > BN6_NAVICUST_SLOTS) continue;
		const uint8_t *record = navi->board + (slot - 1u) * 8u;
		unsigned variant = record[0] | (unsigned)record[1] << 8;
		NaviShape shape;
		if (variant && variant < 47 * 4 && navicust_shape((int)variant, &shape)) out->board[i] = shape.color;
	}
}
