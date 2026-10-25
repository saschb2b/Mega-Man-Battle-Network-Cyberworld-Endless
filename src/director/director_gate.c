/* director_gate.h. The Net Square's gate in Lan's HP (docs/world/home/square-gate.md, piece
 * 11; docs/online/): BN6's great teleporter on the room's front floor,
 * the way out to other players, and its keeper, a Mr.Prog. Stepped on, the
 * gate asks first, in BN6's own chat boxes: the keeper the first time a
 * server is used (what its server sees, the cursor on No), then MegaMan
 * each time after; a Yes saves the run where MegaMan stands and begins the
 * visit (online_visit.c), the gate's core flickering while the server
 * answers. Talked to, the keeper sets where the gate leads (the official
 * Square, or another server's address) and the player's name there; the
 * two fields are the one screen of its own going online has. What a visit
 * ended in is said at the gate after it. */
#include "director_gate.h"

#include <stdio.h>
#include <string.h>

#include "bn6_fields.h"
#include "cinema.h"
#include "debug.h"
#include "devtools.h"
#include "director_hold.h"
#include "director_home.h"
#include "director_save.h"
#include "director_state.h"
#include "flags.h"
#include "game.h"
#include "gate_words.h"
#include "lanhp.h"
#include "network_prefs.h"
#include "online_field.h"
#include "online_visit.h"
#include "save_queue.h"
#include "script_kit.h"
#include "talk.h"

#define GO_WAIT 120   /* frames a Yes waits for MegaMan to be free to save */

/* what follows the talk under way, once it closes */
enum { AFTER_NONE, AFTER_NAME_FOR_GATE, AFTER_PRIVACY, AFTER_GO, AFTER_OFFICIAL };

static struct {
	NetworkPrefs prefs;
	bool identity;       /* the prefs' local identity read or made */
	bool on;             /* MegaMan on the gate last frame */
	int after;
	bool for_gate;       /* the name field opened on the way through the gate */
	FieldKind field;
	int going;           /* frames a Yes has waited to save; 0 none */
	VisitEnd ended;      /* how the last visit ended, still to be said */
	bool written;        /* the keeper's words as the prefs stand */
} G;

/* Whether the gate leads to the official Square (--dev online's server
 * where one is given, as go() takes it) */
static bool leads_official(void) {
	return dev.online ? !strcmp(dev.online, NETWORK_DEFAULT_URL) : G.prefs.selected == 0;
}

static bool store(void) {
	/* (the local identity made with the first settings written) */
	if (!G.identity) G.identity = network_identity(G.prefs.identity);
	bool ok = G.identity && network_prefs_write(g_data_dir, &G.prefs);
	if (!ok) fprintf(stderr, "gate: network.ini could not be written\n");
	G.written = false;
	return ok;
}

/* the keeper's words as the prefs stand: who he is, where the gate leads,
 * his options; each option's script sets its flag */
static void keeper_words(void) {
	if (G.written || emu_read8(BN6_CHATBOX)) return;
	static TextArchive t;
	static uint8_t out[TEXT_ARCHIVE_MAX];
	ta_begin(&t);
	int talk = ta_script(&t);
	bool first = true;
	ta_pages(&t, gate_keeper_hello(leads_official()), FACE_PROG, &first);
	ta_page(&t, FACE_PROG, gate_keeper_ask(), first);
	ta_mugshot(&t, FACE_PROG);
	ta_clear(&t);
	ta_options(&t, gate_options, GATE_OPTIONS);
	/* (the options' scripts follow: talk is 0, they 1-3, B's 4) */
	int pick[GATE_OPTIONS] = { talk + 1, talk + 2, talk + 3 };
	ta_choose(&t, pick, GATE_OPTIONS, talk + 4);
	static const int flag[GATE_OPTIONS] = { GATE_OFFICIAL_FLAG, GATE_OTHER_FLAG, GATE_NAME_FLAG };
	/* (the official Square's asked about as the talk closes: a stray A on
	 * the first option changed the server with no question, session 74) */
	for (int k = 0; k < GATE_OPTIONS; ++k) {
		ta_script(&t);
		ta_flag_set(&t, flag[k]);
		ta_end(&t);
	}
	ta_say(&t, FACE_PROG, gate_keeper_bye());
	G.written = lanhp_keeper_say(out, ta_build(&t, out));
	if (!G.written && emu_debug_on()) fprintf(stderr, "gate: the keeper's words did not fit\n");
}

void home_gate_visit(void) {
	G.identity = network_prefs_read(g_data_dir, &G.prefs);
	G.after = AFTER_NONE;
	G.going = 0;
	G.written = false;
	/* (stood on at a CONTINUE, a visit's return among them: the gate asks
	 * again only once he steps off it and on) */
	G.on = true;
	flag_clear(GATE_YES_FLAG);
	flag_clear(GATE_OFFICIAL_FLAG);
	flag_clear(GATE_OTHER_FLAG);
	flag_clear(GATE_NAME_FLAG);
	keeper_words();
}

bool home_gate_holds(void) { return online_field_shown(); }

bool home_gate_asking(void) { return home_in_hp() && G.after == AFTER_GO; }

bool home_gate_save(void) {
	save_queue_wait();
	unsigned failures = save_write_failures();
	D.checkpoint_here = true;
	save_checkpoint();
	D.checkpoint_here = false;
	save_queue_wait();
	if (save_write_failures() != failures) return false;
	D.home_save_due = false;
	D.saved_at = "Run saved at the Net Square's gate";
	return true;
}

/* The checkpoint not written: told in the game's note, as "Run saved" is,
 * and MegaMan stays home */
static void stay(void) {
	cinema_note("Run not saved", 150);
	talk_start(gate_save_failed(), FACE_MEGAMAN);
}

/* The run saved where MegaMan stands, and the visit begun */
static void go(void) {
	if (!hold_save_ok()) {
		if (++G.going > GO_WAIT) { G.going = 0; stay(); }
		return;
	}
	G.going = 0;
	if (!home_gate_save()) { stay(); return; }
	cinema_note("Run saved", 150);
	const char *url = dev.online ? dev.online : G.prefs.servers[G.prefs.selected];
	const char *name = dev.online_name ? dev.online_name : G.prefs.name;
	if (!G.identity) store();
	uint8_t id[NETWORK_ID_BYTES];
	if (!G.identity || !network_server_identity(G.prefs.identity, url, id)) { G.ended = VISIT_END_UNREACHABLE; return; }
	if (!online_visit_begin(url, name, id, !strcmp(url, NETWORK_DEFAULT_URL))) G.ended = online_visit_ended();
}

/* The gate stepped on: its question (the keeper's the first time a
 * server is used, a name asked for first), or, for --dev online, the
 * visit at once */
static void step_on(void) {
	if (dev.online) { G.going = 1; return; }
	flag_clear(GATE_YES_FLAG);
	if (!G.prefs.named) {
		if (talk_start(gate_keeper_name_first(), FACE_PROG)) G.after = AFTER_NAME_FOR_GATE;
	} else if (!(G.prefs.consent & 1u << G.prefs.selected)) {
		if (talk_ask(gate_keeper_privacy(), FACE_PROG, gate_keeper_privacy_question(), GATE_YES_FLAG)) G.after = AFTER_PRIVACY;
	} else if (talk_ask("", FACE_MEGAMAN, gate_question(), GATE_YES_FLAG)) G.after = AFTER_GO;
}

/* the talk under way closed: what follows it */
static void talk_closed(void) {
	int after = G.after;
	bool yes = flag_get(GATE_YES_FLAG);
	G.after = AFTER_NONE;
	flag_clear(GATE_YES_FLAG);
	if (after == AFTER_NAME_FOR_GATE) {
		G.for_gate = true;
		G.field = FIELD_NAME;
		online_field_open(FIELD_NAME, "");
	} else if (after == AFTER_PRIVACY && yes) {
		G.prefs.consent |= 1u << G.prefs.selected;
		if (store()) G.going = 1;
	} else if (after == AFTER_GO && yes) G.going = 1;
	else if (after == AFTER_OFFICIAL && yes) {
		G.prefs.selected = 0;
		if (store()) talk_start(gate_keeper_official(), FACE_PROG);
	}
}

/* the keeper's option chosen, his talk closed */
static void keeper_chose(void) {
	char host[64];
	if (flag_get(GATE_OFFICIAL_FLAG)) {
		/* (asked first where it changes the gate, the cursor on No) */
		flag_clear(GATE_YES_FLAG);
		if (leads_official()) talk_start(gate_keeper_official_already(), FACE_PROG);
		else if (talk_ask(gate_keeper_official_ask(), FACE_PROG, gate_keeper_official_question(), GATE_YES_FLAG)) G.after = AFTER_OFFICIAL;
	} else if (flag_get(GATE_OTHER_FLAG)) {
		G.for_gate = false;
		G.field = FIELD_ADDRESS;
		if (G.prefs.count > 1) online_field_host(G.prefs.servers[G.prefs.count - 1], host, sizeof host);
		else host[0] = 0;
		online_field_open(FIELD_ADDRESS, host);
	} else if (flag_get(GATE_NAME_FLAG)) {
		G.for_gate = false;
		G.field = FIELD_NAME;
		online_field_open(FIELD_NAME, G.prefs.named ? G.prefs.name : "");
	}
	flag_clear(GATE_OFFICIAL_FLAG);
	flag_clear(GATE_OTHER_FLAG);
	flag_clear(GATE_NAME_FLAG);
}

/* a field closed with `typed`: the name, or the other server's address
 * (the last custom one, its consent asked again where it changed) */
static void field_done(const char *typed) {
	if (G.field == FIELD_NAME) {
		snprintf(G.prefs.name, sizeof G.prefs.name, "%.*s", NETWORK_NAME - 1, typed);
		G.prefs.named = 1;
		if (!store()) return;
		if (G.for_gate) { G.on = false; step_on(); G.on = true; }
		else talk_start(gate_keeper_named(G.prefs.name), FACE_PROG);
		return;
	}
	unsigned slot = G.prefs.count > 1 ? G.prefs.count - 1 : 1;
	if (slot >= NETWORK_SERVERS) slot = NETWORK_SERVERS - 1;
	if (strcmp(G.prefs.servers[slot], typed)) G.prefs.consent &= ~(1u << slot);
	snprintf(G.prefs.servers[slot], NETWORK_URL, "%s", typed);
	if (G.prefs.count <= slot) G.prefs.count = slot + 1;
	G.prefs.selected = slot;
	/* (never the address itself: what a picture of the talk may show) */
	if (store()) talk_start(gate_keeper_set(), FACE_PROG);
}

void home_gate_frame(void) {
	if (!home_in_hp()) return;
	lanhp_gate_calling(online_visit_stage() == VISIT_CALLING);
	if (online_field_shown()) {
		char typed[NETWORK_URL];
		int r = online_field_update(typed, sizeof typed);
		if (r == FIELD_DONE) field_done(typed);
		return;
	}
	VisitEnd why = online_visit_ended();
	if (why != VISIT_END_NONE) G.ended = why;
	if (online_visit_active()) return;
	keeper_words();
	bool idle = !talk_busy() && !emu_read8(BN6_CHATBOX) && on_map() && emu_read8(BN6_WARP_PENDING) == 0;
	if (!idle) return;
	if (G.ended != VISIT_END_NONE) {
		int face;
		const char *words = gate_end_words(G.ended, &face);
		if (!words || talk_start(words, face)) G.ended = VISIT_END_NONE;
		return;
	}
	/* (a trade the visit made, the run's now: MegaMan's word on it) */
	const char *traded = online_visit_news();
	if (traded && !talk_start(traded, FACE_MEGAMAN)) online_visit_news_again(traded);
	if (G.after != AFTER_NONE) { talk_closed(); return; }
	if (G.going) { go(); return; }
	keeper_chose();
	bool on = lanhp_on_gate(bn6_player_x(), bn6_player_y());
	if (on && !G.on) step_on();
	G.on = on;
}
