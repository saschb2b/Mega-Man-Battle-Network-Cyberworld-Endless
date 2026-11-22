/* party_play.h */
#include "party_play.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "arena_input.h"
#include "arena_link_net.h"
#include "arena_view.h"
#include "arena_wire.h"
#include "bn6.h"
#include "boss.h"
#include "director.h"
#include "emu.h"
#include "encounter.h"
#include "free_space.h"
#include "online_mail.h"
#include "online_visit.h"
#include "online_transport.h"
#include "party_marks.h"
#include "party_turn.h"
#include "rom.h"
#include "souls.h"
#include "stands_wire.h"
#include "xnavi_bn4_sounds.h"

#define BOOT_SLICE   240    /* the boot's frames a tick, behind the staging's last picture */
#define HASH_EVERY   60     /* frames between the hashes compared */
#define CATCH_UP     2      /* frames a tick at most */
#define RESULTS_MOST 1800   /* the results' frames at most before the run takes over */
#define RECORD_FOE   (0x20 + 4)   /* a record's first enemy: its list's second entry, its id in bytes 2-3 (encounter.c) */
#define AHEAD_MOST   128    /* my keys' frames past the field's at most (the Arena's, half ARENA_RING) */
#define BURST_MOST   8      /* my keys owed by the clock at most: past them it slides (the relay holds 150, refilled 120 a second) */
#define WINDOW       8      /* frames run on a guess of the others' keys at most (partyroll=W another; the Arena's on a pair) */
#define RING         ARENA_NET_PEER

enum { PP_NONE, PP_BOOT, PP_LOADOUTS, PP_READY, PP_FIGHT, PP_RESULTS };

static struct {
	bool on;
	PartyDev dev;                 /* --dev partycore's (party_play_dev) */
	int stage;
	PartyBattle b;
	unsigned seat;
	bool have[PARTY_MOST];        /* each seat's MegaMan come */
	uint8_t *run_rom;             /* the run's ROM as the fight began */
	PartyCore *core, *twin;       /* the field, and partytwin's */
	PartyCoreSnap *snap;          /* partyroll's */
	/* over the server */
	bool net, started;
	uint32_t battle;
	unsigned delay;
	ArenaNetLink link;
	unsigned gone;                /* seats gone (bit k), whose keys are released from then */
	bool apart;                   /* the hashes stopped: a seat gone, each game on its own */
	unsigned left;                /* ... and whose MegaMen have left the field */
	uint32_t tick;                /* my keys' frames so far (the next one sent) */
	uint16_t real[PARTY_MOST][RING];   /* each seat's keys by frame as they came (mine as sent), */
	uint32_t known[PARTY_MOST];   /* ... below this frame */
	uint16_t used[RING][PARTY_MOST];   /* the keys each frame ran with, a guess among them */
	PartyCoreSnap *back;          /* the field before its first frame run on a guess, */
	uint32_t back_at;             /* ... that frame, */
	bool guessing;                /* ... while a guess stands unproved */
	unsigned window, rollbacks, guessed;
	uint32_t hash_at;             /* the last 60th frame run, its hash, and whether it went */
	uint64_t hash;
	bool hash_sent;
	unsigned agreed, results, waited;
	uint32_t over_at;
	uint32_t started_ms, said_ms;   /* the fight's first frame by the wall clock; its last status line */
	bool alone;                   /* the link broken: every other seat gone */
	uint64_t mine_ready;
	bool crew;                    /* a crew's random battle (party_play_crew), not a guardian's */
	/* this game's own link through a drop (issue #180): down since (SDL ticks, 0 up), the resumes seen, my session, the leave said */
	uint32_t down_since, resumes, me;
	bool leave_said;
} PP;

/* the seat the field is drawn for: mine, or the dev's partyview */
static unsigned viewer(void) { return !PP.net && PP.dev.view > 0 && PP.dev.view < PARTY_MOST ? (unsigned)PP.dev.view : PP.seat; }

/* how the last fight's results went on this game, for the state's words: "shown" (BN6's) or "skipped" (MegaMan fallen) */
static char last_over[48];

/* A crew's battle's end, kept for party_play_crew_end */
static struct { bool due; CrewOver o; } crew_end;

static bool out(void *context, const uint8_t *data, size_t size) {
	(void)context;
	return online_transport_send(data, size);
}

/* seat 0's report and the battle's end (party.py: a door's won by the party is the feed's guardian down) */
static void send_over(bool won) {
	uint8_t m[ONLINE_ARENA_STATUS_BYTES > ONLINE_ARENA_BYTES ? ONLINE_ARENA_STATUS_BYTES : ONLINE_ARENA_BYTES];
	ArenaReport r = { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, (uint8_t)(won ? 1 : 2) };
	if (!online_transport_send(m, stands_wire_status(m, PP.battle, &r))) fprintf(stderr, "party: the report not sent\n");
	if (!online_transport_send(m, arena_wire_action(m, ARENA_ACT_OVER, PP.battle, 0, 0))) fprintf(stderr, "party: the end not sent\n");
}

static void stop(void) {
	party_turn_reset();
	party_core_close(PP.core);
	party_core_close(PP.twin);
	party_core_snap_free(PP.snap);
	party_core_snap_free(PP.back);
	free(PP.run_rom);
	bool on = PP.on;
	PartyDev d = PP.dev;
	memset(&PP, 0, sizeof PP);
	PP.on = on;
	PP.dev = d;
}

/* ---- the fight taken over ---- */

/* the record the run's core was given, as at FREE_RECORDS, and its BN4
 * Navi's sounds' pool pointed at him in the run's ROM (xnavi_bn4_sounds.h) */
static void take_record(const Encounter *e) {
	/* (a crew's: its caller has made it the run's next battle already, director_crew_record) */
	if (!PP.crew) emu_encounter_set(e);
	uint32_t at = FREE_RECORDS + (uint32_t)emu_encounter_slot() * PARTY_RECORD_BYTES;
	emu_read_bytes(at, PP.b.record, PARTY_RECORD_BYTES);
	xnavi_bn4_sounds_for((uint32_t)PP.b.record[RECORD_FOE + 2] | (uint32_t)PP.b.record[RECORD_FOE + 3] << 8);
}

/* my MegaMan as the others read him (the pack stays home: arena_wire.h),
 * kept so here too, so that every game builds the same field; and sent */
static bool carry_mine(void) {
	uint8_t bytes[ONLINE_ARENA_LOADOUT_MAX], m[ONLINE_MAX_MESSAGE];
	char why[96];
	bool carried;
	size_t n = arena_loadout_pack(&PP.b.seat[PP.seat], bytes, sizeof bytes);
	if (!n || !arena_loadout_unpack(bytes, n, &PP.b.seat[PP.seat], &carried, why, sizeof why)) return false;
	size_t size = PP.net ? arena_wire_loadout(m, sizeof m, PP.battle, bytes, n) : 0;
	if (PP.net && (!size || !online_transport_send(m, size))) fprintf(stderr, "party: my MegaMan could not be sent\n");
	return true;
}

static bool fight(const Encounter *e, int own_hp, int fought_hp) {
	bool dev_party = PP.dev.n >= 1;
	if (!PP.on || (!dev_party && !PP.started)) return false;
	PP.net = PP.started;
	PP.b.n = PP.net ? PP.b.n : (unsigned)(PP.dev.n > PARTY_MOST ? PARTY_MOST : PP.dev.n);
	PP.b.foe_from = own_hp;
	PP.b.foe_to = fought_hp;
	take_record(e);
	OnlineLoadout *mine = &PP.b.seat[PP.seat];
	bool read = online_snapshot_read(mine, souls_held());
	/* (the dive's pull-back spent goes with my MegaMan, so every game of a crew's field knows it: crew_pull.h) */
	mine->pulled = PP.crew && director_crew_pull_spent();
	if (!read || !carry_mine() || !(PP.run_rom = malloc(EMU_ROM_SIZE)) ||
		!emu_rom_clean(PP.run_rom, EMU_ROM_SIZE)) {
		fprintf(stderr, "party: the fight could not be taken over; the run's core fights it\n");
		free(PP.run_rom);
		PP.run_rom = NULL;
		return false;
	}
	PP.have[PP.seat] = true;
	/* (the dev's stood-in partners: my MegaMan again, their keys a script) */
	for (unsigned k = 0; !PP.net && k < PP.b.n; ++k) {
		PP.b.seat[k] = *mine;
		PP.have[k] = true;
		if (k && PP.dev.hp > 0 && PP.dev.hp < mine->max_hp) PP.b.seat[k].hp = (uint16_t)PP.dev.hp;
	}
	PP.stage = PP_BOOT;
	if (!PP.net) party_marks_seats(NULL, viewer());
	if (PP.crew) fprintf(stderr, "party: the crew's battle taken over, %u MegaMen, my seat %u\n", PP.b.n, PP.seat);
	else fprintf(stderr, "party: the guardian's fight taken over, %u MegaMen, my seat %u, his HP %d (his own %d)\n", PP.b.n, PP.seat, fought_hp,
		own_hp);
	return true;
}

bool party_play_crew(const Encounter *e) {
	if (!PP.started) return false;
	PP.crew = true;
	crew_end.due = false;
	if (fight(e, 0, 0)) return true;
	PP.crew = false;
	return false;
}

bool party_play_crew_fighting(void) { return PP.crew && PP.stage != PP_NONE; }

static void hold_frame(uint32_t pad) {
	/* (the transport's inbox, which the held game's own frame no longer drains) */
	if (PP.net) online_mail_frame();
	party_play_frame(pad);
}

static const VisitHold hold = { party_play_held, hold_frame, party_play_draw };

void party_play_on(bool on) {
	if (!on) stop();
	PP.on = on;
	boss_party_fight = on ? fight : NULL;
	online_visit_hold = on ? &hold : NULL;
}

void party_play_dev(const PartyDev *d) {
	PP.dev = *d;
	party_play_on(d->n >= 1);
	/* (the guardian scaled for the dev's party as a party's door scales him, or by the rule measured: tools/party_guardian.py) */
	boss_party_dev(d->scale && d->n >= 1 ? d->n : 0, d->hp_per, d->steps);
}

void party_play_start(uint32_t battle, unsigned seat, unsigned n, unsigned delay, const uint32_t seats[PARTY_MOST]) {
	stop();
	party_marks_seats(seats, seat);
	PP.started = true;
	PP.battle = battle;
	PP.seat = seat < PARTY_MOST ? seat : 0;
	PP.b.n = n < 1 ? 1 : n > PARTY_MOST ? PARTY_MOST : n;
	PP.delay = PP.dev.delay > 0 ? (unsigned)PP.dev.delay : delay;
	PP.window = PP.dev.roll > 0 ? (unsigned)PP.dev.roll : WINDOW;
	arena_link_net(&PP.link, battle, 0, PP.delay, out, NULL);
	PP.delay = PP.link.link.delay;
	arena_link_net_party(&PP.link, PP.seat, PP.b.n);
	OnlineSnapshot snap;
	online_transport_snapshot(&snap);
	PP.resumes = snap.resumes;
	PP.me = snap.player_id;
	fprintf(stderr, "party: battle %u, seat %u of %u, delay %u (START said %u), a guess %u frames at most\n", (unsigned)battle, PP.seat, PP.b.n, PP.delay,
		delay, PP.window);
}

void party_play_gone(unsigned seat) {
	if (!PP.started || seat >= PARTY_MOST || seat == PP.seat || (PP.gone >> seat & 1u)) return;
	PP.gone |= 1u << seat;
	arena_link_net_gone(&PP.link, seat);
	fprintf(stderr, "party: seat %u gone at frame %u%s\n", seat, (unsigned)(PP.core ? party_core_frames(PP.core) : 0),
		PP.stage == PP_FIGHT ? ": his MegaMan leaves the field, and this game fights on alone" : "");
}

static void loadout_in(unsigned seat, const uint8_t *m, size_t n) {
	uint32_t battle;
	const uint8_t *bytes;
	size_t size;
	bool carried;
	char why[96];
	if (seat >= PARTY_MOST || !arena_wire_read_loadout(m, n, &battle, &bytes, &size) || battle != PP.battle) return;
	if (!arena_loadout_unpack(bytes, size, &PP.b.seat[seat], &carried, why, sizeof why)) {
		fprintf(stderr, "party: seat %u's MegaMan refused: %s\n", seat, why);
		return;
	}
	PP.have[seat] = true;
	fprintf(stderr, "party: seat %u's MegaMan came: HP %u of %u\n", seat, PP.b.seat[seat].hp, PP.b.seat[seat].max_hp);
}

bool party_play_mail(const uint8_t *m, size_t n) {
	unsigned seat;
	const uint8_t *inner;
	size_t size;
	if (!PP.started || m[0] != ONLINE_ARENA_SEAT || !arena_wire_seat(m, n, &seat, &inner, &size)) return false;
	if (inner[0] == ONLINE_ARENA_LOADOUT) { loadout_in(seat, inner, size); return true; }
	return arena_link_net_take(&PP.link, m, n);
}

bool party_play_held(void) { return PP.stage != PP_NONE; }

bool party_play_hp(int *hp, int *max) {
	if (!PP.core || (PP.stage != PP_FIGHT && PP.stage != PP_RESULTS)) return false;
	*hp = (int)party_core_hp(PP.core, PP.seat);
	*max = PP.b.seat[PP.seat].max_hp;
	return *max > 0;
}

/* ---- the fight's frames ---- */

static void failed(const char *why) {
	/* (the server's party battle let go of, this seat gone: the others told, a later battle or queue not refused as busy) */
	if (PP.net) {
		uint8_t m[ONLINE_ARENA_BYTES];
		if (!online_transport_send(m, arena_wire_action(m, ARENA_ACT_LEAVE, PP.battle, 0, 0))) fprintf(stderr, "party: the leave not sent\n");
	}
	if (PP.crew) {
		/* (a crew's battle: its owner's game fights it alone, as the field would have; a joiner back on his map) */
		bool owner = PP.seat == 0;
		fprintf(stderr, "party: %s; %s\n", why, owner ? "the run's core fights the crew's battle alone" : "back on the map");
		if (PP.net) arena_link_net_flush(&PP.link);
		stop();
		if (owner) emu_encounter_go();
		else emu_encounter_drop();
		return;
	}
	fprintf(stderr, "party: %s; the run's core fights the guardian alone\n", why);
	int own = PP.b.foe_from, fought;
	stop();
	/* (the run's own battle in its place: the staging's fight as it would have begun, his HP the party's) */
	const Encounter *e = boss_battle(&fought);
	emu_battle_hp(own, fought);
	emu_battle_force(e);
}

static bool all_here(void) {
	for (unsigned k = 0; k < PP.b.n; ++k)
		if (!PP.have[k] && !(PP.gone >> k & 1u)) return false;
	return true;
}

/* the battle's record as the field takes it, its list's pointer (its slot's on each game) the field's own (party_core.c's rom_in) */
static uint64_t record_hash(void) {
	uint8_t r[PARTY_RECORD_BYTES];
	memcpy(r, PP.b.record, sizeof r);
	memset(r + 12, 0, 4);
	return party_core_bytes_hash(r, sizeof r);
}

/* what every game of the party must build its field from alike, each part's hash: where two fields go apart, the part */
static void inputs_said(void) {
	fprintf(stderr, "party: the field from rom %016llx record %016llx", (unsigned long long)party_core_bytes_hash(PP.run_rom, EMU_ROM_SIZE),
		(unsigned long long)record_hash());
	for (unsigned k = 0; k < PP.b.n; ++k) {
		uint8_t bytes[ONLINE_ARENA_LOADOUT_MAX];
		size_t n = arena_loadout_pack(&PP.b.seat[k], bytes, sizeof bytes);
		fprintf(stderr, " seat%u %016llx", k, (unsigned long long)party_core_bytes_hash(bytes, n));
	}
	fputc('\n', stderr);
}

static void open_core(void) {
	char why[160] = "";
	/* (the lead's MegaMan is the field's own: with him gone before it was made there is no field) */
	if (PP.net && !PP.have[0]) { failed("the lead left before the field was made"); return; }
	PP.b.quiet = PP.net;
	/* (over the server each partner his own NaviStats, hand and Custom screen; the dev's stood-in partners a script, or with
	 * partybot=1 each his own screen on this game, the bot's keys on it) */
	PP.b.own = PP.b.n > 1 && (PP.net || PP.dev.bot);
	PP.b.scripted = !PP.net && !PP.b.own;
	PP.b.spread = !PP.dev.lead;   /* (partylead=1: the enemies' aim on the lead alone, BN6's own) */
	PP.b.end = PP.dev.end;   /* (partyend=N on every game of a party: its test of the end) */
	PP.b.del = PP.dev.del;   /* (partydel=MASK with it: those seats deleted then, the crew's pull-back's test) */
	PP.b.reward = PP.dev.reward;   /* (partyreward=N: the results' reward this entry, an HP reward's test, issue #769) */
	inputs_said();
	PP.core = party_core_open(R.data, ROM_SIZE, PP.run_rom, &PP.b, why, sizeof why);
	if (!PP.core) { failed(why); return; }
	if (!PP.net && PP.dev.twin && !(PP.twin = party_core_open(R.data, ROM_SIZE, PP.run_rom, &PP.b, why, sizeof why))) {
		failed(why);
		return;
	}
	if (!PP.net && PP.dev.roll > 0 && (PP.snap = party_core_snap_new(PP.core)) != NULL) party_core_save(PP.core, PP.snap);
	if (PP.net && !(PP.back = party_core_snap_new(PP.core))) { failed("no room to keep the field for a rollback"); return; }
	PP.mine_ready = party_core_hash(PP.core);
	fprintf(stderr, "party: ready, the field's hash %016llx\n", (unsigned long long)PP.mine_ready);
	if (PP.net) arena_link_net_ready(&PP.link, PP.mine_ready);
	PP.stage = PP.net ? PP_READY : PP_FIGHT;
}

static void ready_frame(void) {
	uint64_t theirs;
	const char *broken = PP.link.link.broken(&PP.link.link);
	if (broken) { failed(broken); return; }
	if (!arena_link_net_peer_ready(&PP.link, &theirs)) return;
	if (theirs != PP.mine_ready) {
		fprintf(stderr, "party: the fields went apart before the first frame (%016llx, mine %016llx)\n", (unsigned long long)theirs,
			(unsigned long long)PP.mine_ready);
		failed("the party's games built their fields apart");
		return;
	}
	fprintf(stderr, "party: every seat ready with %016llx; the fight begins\n", (unsigned long long)theirs);
	PP.stage = PP_FIGHT;
}

/* partyroll=W: every W frames the field taken back W frames and run again from the same keys, its hash the same */
static uint16_t rolled[256][PARTY_MOST];

static void roll_check(const uint16_t keys[PARTY_MOST]) {
	uint32_t f = party_core_frames(PP.core), w = (uint32_t)PP.dev.roll;
	if (!PP.snap || w > 255) return;
	memcpy(rolled[f % w], keys, sizeof rolled[0]);
	if (f % w) return;
	if (f) {
		uint64_t straight = party_core_hash(PP.core);
		if (!party_core_load(PP.core, PP.snap)) { fprintf(stderr, "party: the rollback's snapshot did not load\n"); party_core_save(PP.core, PP.snap); return; }
		for (uint32_t i = 1; i <= w; ++i) party_core_frame(PP.core, rolled[i % w]);
		uint64_t again = party_core_hash(PP.core);
		fprintf(stderr, "party: frame %u rolled back %u frames: %016llx straight, %016llx again: %s\n", (unsigned)f, (unsigned)w,
			(unsigned long long)straight, (unsigned long long)again, straight == again ? "the same" : "APART");
	}
	party_core_save(PP.core, PP.snap);
}

static void twin_check(const uint16_t keys[PARTY_MOST]) {
	if (!PP.twin) return;
	party_core_frame(PP.twin, keys);
	uint32_t f = party_core_frames(PP.core);
	if (f % HASH_EVERY) return;
	uint64_t a = party_core_hash(PP.core), b = party_core_hash(PP.twin);
	fprintf(stderr, "party: frame %u: the field %016llx, its twin %016llx: %s\n", (unsigned)f, (unsigned long long)a, (unsigned long long)b,
		a == b ? "the same" : "APART");
}

static void step(const uint16_t keys[PARTY_MOST]) {
	party_core_frame(PP.core, keys);
	twin_check(keys);
	roll_check(keys);
}

/* the seats gone: their MegaMen leave the field (each game's at its own
 * frame, so the hashes stop: each finishes its own battle) */
static void gone_frame(void) {
	unsigned fresh = PP.gone & ~PP.left;
	if (!fresh) return;
	for (unsigned k = 0; k < PARTY_MOST; ++k)
		if (fresh >> k & 1u) party_core_leave(PP.core, k);
	PP.left |= fresh;
	PP.apart = true;
}

/* ---- the frames over the link: guessed, and rolled back where a guess was wrong ---- */

/* the others' keys as they come, each seat's stream in order */
static void pull(void) {
	uint32_t most = party_core_frames(PP.core) + RING / 2;
	for (unsigned k = 0; k < PP.b.n; ++k) {
		uint16_t v;
		if (k == PP.seat || (PP.gone >> k & 1u)) continue;
		if (PP.known[k] < PP.delay) PP.known[k] = PP.delay;
		while (PP.known[k] < most && PP.link.link.poll_player(&PP.link.link, k, PP.known[k], &v)) PP.real[k][PP.known[k]++ % RING] = v;
	}
}

/* below this frame every seat's keys have come (a seat gone: his last, then none) */
static uint32_t sure_to(void) {
	uint32_t sure = UINT32_MAX;
	for (unsigned k = 0; k < PP.b.n; ++k) {
		uint32_t to = k == PP.seat ? PP.tick : (PP.gone >> k & 1u) ? UINT32_MAX : PP.known[k] < PP.delay ? PP.delay : PP.known[k];
		if (to < sure) sure = to;
	}
	return sure;
}

/* seat k's keys for frame f as they came, else a guess (his last that came: *guess set) */
static uint16_t key_at(unsigned k, uint32_t f, bool *guess) {
	uint32_t to = k == PP.seat ? PP.tick : PP.known[k];
	if (f < PP.delay) return 0;
	if (f < to) return PP.real[k][f % RING];
	if (PP.gone >> k & 1u) return 0;
	*guess = true;
	/* (his last keys; never a message of his Custom screen again, which comes once) */
	uint16_t last = to > PP.delay ? PP.real[k][(to - 1) % RING] : 0;
	return party_turn_guess(last);
}

/* frame f on the field, a guess where a seat's keys have not come; the
 * field kept before the first guess, and each 60th frame's hash */
static void run_frame(uint32_t f) {
	uint16_t keys[PARTY_MOST] = { 0 };
	bool guess = false;
	for (unsigned k = 0; k < PP.b.n; ++k) keys[k] = key_at(k, f, &guess);
	if (guess && !PP.guessing) {
		PP.guessing = party_core_save(PP.core, PP.back);
		PP.back_at = f;
	}
	PP.guessed += guess;
	memcpy(PP.used[f % RING], keys, sizeof keys);
	party_core_frame(PP.core, keys);
	if ((f + 1) % HASH_EVERY) return;
	PP.hash_at = f + 1;
	PP.hash = party_core_hash(PP.core);
	PP.hash_sent = false;
}

/* a guess proved wrong by the keys come since: the field back to its first
 * guessed frame and run again to where it stood */
static void settle(void) {
	if (!PP.guessing) return;
	uint32_t sure = sure_to(), now = party_core_frames(PP.core), upto = sure < now ? sure : now;
	bool wrong = false, guess = false;
	for (uint32_t f = PP.back_at; f < upto && !wrong; ++f)
		for (unsigned k = 0; k < PP.b.n; ++k) wrong |= key_at(k, f, &guess) != PP.used[f % RING][k];
	if (!wrong) {
		if (sure >= now) PP.guessing = false;
		return;
	}
	PP.guessing = false;
	if (!party_core_load(PP.core, PP.back)) { fprintf(stderr, "party: the field kept could not be put back\n"); PP.apart = true; return; }
	++PP.rollbacks;
	for (uint32_t f = PP.back_at; f < now; ++f) run_frame(f);
}

/* the last 60th frame's hash said once every seat's keys before it have come, and the others' compared */
static void hash_out(void) {
	if (PP.apart) return;
	if (!PP.hash_sent && PP.hash_at && sure_to() >= PP.hash_at) {
		PP.link.link.send_hash(&PP.link.link, PP.hash_at, PP.hash);
		PP.hash_sent = true;
	}
	uint32_t check = (PP.agreed + 1) * HASH_EVERY;
	uint64_t theirs, mine = 0;
	if (check > PP.hash_at || !PP.link.link.poll_hash(&PP.link.link, check, &theirs)) return;
	for (unsigned i = 0; i < PP.link.nmine; ++i)
		if (PP.link.mine[i].frame == check) mine = PP.link.mine[i].hash;
	++PP.agreed;
	fprintf(stderr, "party: frame %u's hash %016llx: %s\n", (unsigned)check, (unsigned long long)mine, mine == theirs ? "every seat agrees" : "APART");
	if (mine != theirs) PP.apart = true;
}

/* the link broken (the server let this game go, or the others all left):
 * every other seat gone, and this game fights on alone */
static void alone(const char *why) {
	if (PP.alone) return;
	fprintf(stderr, "party: %s; this game fights on alone from frame %u\n", why, (unsigned)party_core_frames(PP.core));
	PP.alone = true;
	PP.gone |= ((1u << PP.b.n) - 1u) & ~(1u << PP.seat);
	gone_frame();
}

/* my keys for the frame a delay on, as the wall clock gives a 60th of a
 * second (a game run faster than the GBA, headless, keeps to it: the
 * server's relay holds 120 a second), at most AHEAD_MOST past the field */
static void send_mine(uint16_t mine) {
	uint32_t now = SDL_GetTicks();
	if (!PP.started_ms) PP.started_ms = now ? now : 1;
	uint32_t due = PP.delay + (uint32_t)((uint64_t)(now - PP.started_ms) * 60u / 1000u) + 1u;
	if (PP.tick < PP.delay) PP.tick = PP.delay;
	/* (a game held back, its window dragged or a slow frame: the clock slid, never a burst of keys past the relay's bucket) */
	if (due > PP.tick + BURST_MOST) {
		PP.started_ms += (due - PP.tick - BURST_MOST) * 1000u / 60u;
		due = PP.tick + BURST_MOST;
	}
	if (PP.tick < due && PP.tick < party_core_frames(PP.core) + AHEAD_MOST && PP.link.link.send(&PP.link.link, PP.tick, mine))
		PP.real[PP.seat][PP.tick++ % RING] = mine;
	PP.link.link.tick(&PP.link.link);
	arena_link_net_flush(&PP.link);
	if (now - PP.said_ms >= 2000) {
		PP.said_ms = now;
		fprintf(stderr, "party: frame %u, my keys sent to %u (the clock's %u), every seat's come to %u; %u frames run on a guess, %u rollbacks\n",
			(unsigned)party_core_frames(PP.core), (unsigned)PP.tick, (unsigned)due, (unsigned)sure_to(), PP.guessed, PP.rollbacks);
	}
}

/* partybot=1: a bot's keys on the party's field: on the Custom screen the
 * chip under the cursor taken and the cursor a chip right every 20 frames
 * (BN6 takes the chips that fit, and at last OK), then charged shots and
 * now and then A, a chip used */
static uint16_t bot(uint32_t tick, unsigned seat) {
	if (party_core_phase(PP.core) == BN6_PHASE_CUSTOM) {
		uint32_t t = tick % 20;
		return t < 2 ? KEY_A : t >= 10 && t < 12 ? KEY_RIGHT : 0;
	}
	uint32_t t = (tick + 37u * seat) % 120;
	return t < 100u + 4 * seat ? KEY_B : t >= 114 && t < 118 ? KEY_A : 0;
}

/* seat's stream's word for the frame: a partner's own turn's (party_turn.h) where the field gives him one, else his keys */
static uint16_t word_of(unsigned seat, uint32_t pad, uint32_t tick) {
	uint16_t keys = (uint16_t)((PP.dev.bot ? bot(tick, seat) : pad) & ARENA_KEYS_MASK);
	if (!PP.b.own || seat == 0 || !PP.core) return keys;
	return party_turn_word(PP.core, PP.run_rom, seat, keys, PP.dev.bot != 0);
}

/* this game's own link down mid-fight (issue #180, docs/online/shared-dive.md,
 * A drop): nothing goes in the outbox and the field waits on the others'
 * keys while the transport tries again every half second; back in the same
 * session (the server holding the seat, party.py), my keys go again and
 * theirs are asked for. Down past HELD_MS, or back in another session, the
 * others are let go of and this game fights on alone, as decided; its
 * leave said once the link is back, so the others need not wait out the
 * seat's hold. True where the fight waits on the link */
#define HELD_MS 10000   /* (the server holds the seat 12 seconds, party.py's SEAT_HOLD) */
static bool held_through(void) {
	OnlineSnapshot s;
	online_transport_snapshot(&s);
	uint32_t now = SDL_GetTicks();
	bool down = !s.error[0] && (!s.connected || s.reconnecting);
	arena_link_net_hold(&PP.link, down);
	if (s.error[0] && !PP.alone) alone("the link to the server is gone");
	if (down) {
		online_transport_hurry();
		if (!PP.down_since) fprintf(stderr, "party: the link dropped at frame %u; holding on\n", (unsigned)party_core_frames(PP.core));
		if (!PP.down_since) PP.down_since = now | 1u;
		if ((int32_t)(now - PP.down_since) > HELD_MS && !PP.alone) alone("the link stayed down");
		return !PP.alone;
	}
	PP.down_since = 0;
	if (s.resumes == PP.resumes) return false;
	PP.resumes = s.resumes;
	if (s.player_id != PP.me && !PP.alone) alone("back in another session");
	else if (!PP.alone) arena_link_net_resumed(&PP.link);
	if (PP.alone && !PP.leave_said) {
		uint8_t m[ONLINE_ARENA_BYTES];
		PP.leave_said = online_transport_send(m, arena_wire_action(m, ARENA_ACT_LEAVE, PP.battle, 0, 0));
	}
	return false;
}

static void fight_frame(uint32_t pad) {
	/* (the dev's game alone: the bot's clock the field's frames, as no keys are sent) */
	uint16_t mine = word_of(PP.seat, pad, PP.net ? PP.tick : party_core_frames(PP.core));
	if (PP.net && held_through()) return;
	if (!PP.net || PP.alone) {
		uint16_t keys[PARTY_MOST] = { 0 };
		keys[PP.seat] = mine;
		/* (the dev's stood-in partners with their own screens: each his words, the bot's keys) */
		for (unsigned k = 0; !PP.net && PP.b.own && k < PP.b.n; ++k)
			if (k != PP.seat) keys[k] = word_of(k, 0, party_core_frames(PP.core));
		step(keys);
		return;
	}
	const char *broken = PP.link.link.broken(&PP.link.link);
	if (broken) { alone(broken); return; }
	gone_frame();
	send_mine(mine);
	pull();
	settle();
	/* (then the frames my keys have gone for, a couple a tick, on a guess of the others' as far as the window goes) */
	for (int i = 0; i < CATCH_UP; ++i) {
		uint32_t f = party_core_frames(PP.core);
		if (f >= PP.tick) break;
		if (f >= sure_to() && (PP.guessing ? f - PP.back_at : 0) >= PP.window) {
			if (++PP.waited % 600 == 0) fprintf(stderr, "party: frame %u waits on the others' keys\n", (unsigned)f);
			break;
		}
		run_frame(f);
	}
	hash_out();
}

/* ---- the end ---- */

/* a crew's battle's end kept for its caller (party_play_crew_end): the
 * reward BN6's results gave on the field, this game's to give in its own
 * run, and every seat's MegaMan as the field ended, from which each game
 * works out the crew's gifts alike (director_crew_over, crew_pull.h) */
static void crew_over(bool won) {
	CrewOver o = { won, party_core_reward(PP.core), PP.seat, { .n = PP.b.n } };
	for (unsigned k = 0; k < PP.b.n && k < CREW_PULL_SEATS; ++k) {
		o.field.hp[k] = party_core_hp(PP.core, k);
		o.field.max_hp[k] = PP.b.seat[k].max_hp;
		o.field.spent[k] = PP.b.seat[k].pulled;
		o.field.gone[k] = (PP.gone >> k & 1u) != 0;
	}
	crew_end.due = true;
	crew_end.o = o;
	fprintf(stderr, "party: the crew's battle's reward on the field %04x\n", o.reward);
}

/* the battle over: BN6's results on this game's own pad, then the run */
static void results_frame(uint32_t pad) {
	uint16_t keys[PARTY_MOST] = { (uint16_t)(pad & ARENA_KEYS_MASK) };
	bool won;
	party_core_over(PP.core, &won);
	/* (MegaMan deleted on the field: no results of BN6's, whose reward is not his (A deletion in a crew); the run says
	 * what comes of it, issue #757) */
	bool fallen = !party_core_hp(PP.core, viewer());
	if (PP.results++ < RESULTS_MOST && !fallen && !party_core_back(PP.core)) {
		party_core_frame(PP.core, keys);
		return;
	}
	unsigned hp = party_core_hp(PP.core, PP.seat);
	snprintf(last_over, sizeof last_over, "%s hp %u", fallen ? "skipped" : "shown", party_core_hp(PP.core, viewer()));
	fprintf(stderr, "party: the fight over after %u frames (%u run on a guess, %u rollbacks): %s, my MegaMan's HP %u%s\n", (unsigned)PP.over_at,
		PP.guessed, PP.rollbacks, won ? "won" : "lost", hp, hp ? "" : ": deleted, the run is over");
	if (PP.net) {
		arena_link_net_flush(&PP.link);
		if (PP.seat == 0) send_over(won);
	}
	party_core_sound(PP.core, false);
	for (unsigned k = 1; PP.b.own && k < PP.b.n; ++k)
		if (k == PP.seat || !PP.net) party_turn_report(k);
	/* (a crew's battle: its end kept for its caller, the reward BN6's results gave on the field this game's to give in
	 * its own run, party_play_crew_end) */
	bool crew = PP.crew;
	if (crew) crew_over(won);
	bool on = PP.on;
	stop();
	PP.on = on;
	if (!crew) director_party_over(won, (int)hp);
}

bool party_play_crew_end(CrewOver *out) {
	if (!crew_end.due) return false;
	*out = crew_end.o;
	crew_end.due = false;
	return true;
}

static void fighting(uint32_t pad) {
	fight_frame(pad);
	bool won;
	if (PP.core && party_core_over(PP.core, &won)) {
		PP.over_at = party_core_frames(PP.core);
		PP.stage = PP_RESULTS;
		/* (BN6's results on this game alone from here: a partner's show his own HP, not his lead's; issue #757) */
		party_core_results_as(PP.core, viewer());
		fprintf(stderr, "party: frame %u: the battle over, %s; the guardian's HP %u\n", (unsigned)PP.over_at, won ? "won" : "lost",
			party_core_foe_hp(PP.core));
	}
}

void party_play_frame(uint32_t pad) {
	switch (PP.stage) {
	case PP_BOOT: {
		int more = party_core_boot(R.data, ROM_SIZE, BOOT_SLICE);
		if (more < 0) failed("the party's core could not boot");
		else if (more == 0) PP.stage = PP_LOADOUTS;
		break;
	}
	case PP_LOADOUTS:
		if (all_here()) open_core();
		else if (++PP.waited % 600 == 0) fprintf(stderr, "party: waiting on the others' MegaMen\n");
		break;
	case PP_READY: ready_frame(); break;
	case PP_FIGHT: fighting(pad); break;
	case PP_RESULTS: results_frame(pad); break;
	default: break;
	}
	if (PP.core) party_core_sound(PP.core, PP.stage >= PP_FIGHT);
}

void (*party_play_draw_more)(void);

/* the party's field, or while it is made and the others come, the staging's
 * last picture of the run's own core */
void party_play_draw(void) {
	const uint32_t *turn = PP.core && PP.stage == PP_FIGHT ? party_turn_video(viewer()) : NULL;

	arena_view_core(turn ? turn : PP.core && PP.stage >= PP_FIGHT ? party_core_video(PP.core) : emu_video());
	/* (who is who on the field as it fights, never over a partner's own screen) */
	if (PP.core && PP.stage == PP_FIGHT && !turn) party_marks_draw(party_core_gba(PP.core), viewer(), PP.b.n);
	if (PP.crew && party_play_draw_more) party_play_draw_more();
}

void party_play_describe(FILE *f) {
	/* (the spike's battle on the run's own core: --dev party=N, the viruses' bench) */
	party_field_describe(f);
	/* (over the server: its stage and frame, the hashes agreed, this game's own link through a drop, issue #180) */
	static const char *const stages[] = { "none", "boot", "loadouts", "ready", "fight", "results" };
	if (PP.started && PP.net)
		fprintf(f, "party %s frame %u\npartyagreed %u\npartylink %s\n", (unsigned)PP.stage < 6 ? stages[PP.stage] : "other",
			(unsigned)(PP.core ? party_core_frames(PP.core) : 0), PP.agreed, PP.alone ? "alone" : PP.down_since ? "down" : "up");
	if (last_over[0]) fprintf(f, "partyover %s\n", last_over);
	if (PP.core && PP.stage == PP_RESULTS) {
		unsigned shown = party_core_navi_hp(PP.core), own = party_core_hp(PP.core, viewer());
		fprintf(f, "partyresults hp %u own %u %s\n", shown, own, shown == own ? "agree" : "APART");
	}
	if (!PP.core || PP.stage != PP_FIGHT) return;
	fprintf(f, "partyfield %u seat %u\n", PP.b.n, viewer());
	/* (a partner's own Custom screen: the HP its box shows, and his own on the field, issue #757) */
	unsigned turn = party_turn_hp(viewer());
	if (turn) fprintf(f, "partyturn hp %u own %u %s\n", turn, party_core_hp(PP.core, viewer()), turn == party_core_hp(PP.core, viewer()) ? "agree" : "APART");
	if (!party_turn_video(viewer())) party_marks_describe(f, party_core_gba(PP.core), viewer(), PP.b.n);
}

