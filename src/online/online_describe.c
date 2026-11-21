/* online_describe.h. Each part says its own lines: the visit and how the
 * last one ended, the Square (its part under MegaMan, who else is there,
 * what was last said) and the NetPass here; Trade Central's trade and its
 * spares, the Arena's bout, the week's net and a dive's divers each in
 * its own module (online_trade_describe, trade_screen_describe,
 * online_board_describe, square_arena_describe, square_week_describe,
 * dive_link_describe). A line's first word is its own; its
 * value one word where a test reads it. */
#include "online_describe.h"

#include "bn6_fields.h"
#include "director_crew.h"
#include "director_hold.h"
#include "dive_link.h"
#include "emu.h"
#include "netmap.h"
#include "netpass.h"
#include "online_board.h"
#include "online_square.h"
#include "online_trade.h"
#include "online_transport.h"
#include "online_visit.h"
#include "party_play.h"
#include "square_arena.h"
#include "square_pet.h"
#include "square_watch.h"
#include "square_week.h"
#include "talk.h"
#include "trade_screen.h"
#include "weekly.h"


/* the part of the Square world (x, y) stands in */
static const char *part_at(int wx, int wy) {
	static const char *const names[SQ_PARTS] = { "plaza", "trade", "arena", "stage" };
	const SquarePlan *p = online_square_plan();
	int gx, gy;
	int part = p && netmap_panel(wx, wy, &gx, &gy) ? square_plan_part(p, gx, gy) : -1;
	return part >= 0 && part < SQ_PARTS ? names[part] : "none";
}

/* the visit: where it stands, and how the last one ended (kept: the gate
 * takes online_visit_ended's once) */
static void visit(FILE *f) {
	static const char *const ends[] = { "none", "left", "cancelled", "unreachable", "version", "rom", "name", "full", "barred",
		"removed", "lost", "broken", "dive", "maintenance", "oldserver", "moved" };
	const char *stage = "home";
	if (online_visit_stage() == VISIT_CALLING) stage = "calling";
	else if (online_visit_away()) {
		stage = online_visit_waiting() ? "waiting" : square_arena_on() ? "battle" : square_watch_on() ? "watching"
			: square_pet_open() ? "pet" : "square";
	}
	unsigned end = (unsigned)online_visit_last_end();
	fprintf(f, "visit %s\nvisits %u\nvisitended %s\n", stage, online_visit_count(), end < sizeof ends / sizeof *ends ? ends[end] : "other");
}

/* the Square: its seed and channel, the part MegaMan stands in, each
 * other player (his name, place, part and what he is busy with), the
 * last phrase or emote of MegaMan's own and of another's, the link */
static void square(FILE *f, const OnlineSnapshot *s) {
	static const char *const busy[] = { "free", "battle", "watch", "trade" };
	/* (the chat box and what holds MegaMan, as the run's state says them: the director is stopped here) */
	fprintf(f, "chat %s\nheld %s\n", emu_read8(BN6_CHATBOX) || talk_busy() ? "open" : "closed", held_names(bn6_held()));
	talk_describe(f);
	/* (where MegaMan stands, a number a line: a walk's `until`) */
	fprintf(f, "squarex %d\nsquarey %d\nfacing %d\n", bn6_player_x(), bn6_player_y(), emu_read8(BN6_PLAYER_FACING) & 7);
	fprintf(f, "squareseed %u\nchannel %u/%u\npart %s\npeers %u", (unsigned)s->seed, s->channel, s->channels,
		part_at(bn6_player_x(), bn6_player_y()), (unsigned)s->peer_count);
	for (size_t i = 0; i < s->peer_count; ++i) fprintf(f, "%s%s", i ? " " : ": ", s->peers[i].name);
	fputc('\n', f);
	for (size_t i = 0; i < s->peer_count; ++i) {
		const OnlinePeer *p = &s->peers[i];
		fprintf(f, "peer %s at %d %d in %s %s%s\n", p->name, (int)p->x, (int)p->y, part_at(p->x, p->y),
			p->busy < sizeof busy / sizeof *busy ? busy[p->busy] : "busy", p->diving ? " diving" : "");
	}
	const OnlineEvent *sent = NULL, *heard = NULL;
	unsigned sents = 0, heards = 0;
	for (uint32_t k = 0; k < ONLINE_EVENT_HISTORY && k < s->event_serial; ++k) {
		const OnlineEvent *e = &s->events[(s->event_serial - k) % ONLINE_EVENT_HISTORY];
		if (e->player_id == s->player_id) { ++sents; if (!sent) sent = e; }
		else { ++heards; if (!heard) heard = e; }
	}
	if (sent) fprintf(f, "sentlast %s %u (%u)\n", sent->kind ? "emote" : "phrase", sent->value, sents);
	if (heard) {
		const char *name = square_arena_name_of(heard->player_id);
		fprintf(f, "heardlast %s %u from %s (%u)\n", heard->kind ? "emote" : "phrase", heard->value, name ? name : "?", heards);
	}
	fprintf(f, "link %s\nresumes %u\n", s->reconnecting ? "reconnecting" : s->quiet ? "quiet" : "up", (unsigned)s->resumes);
	if (s->notice_serial) fprintf(f, "notice %u\n", s->notice_code);
}

void online_describe(FILE *f) {
	static const char *const passes[] = { "square", "square", "trade", "trade", "full" };
	visit(f);
	fprintf(f, "netpass %s\nstamps %d\n", passes[netpass_stamps() <= STAMP_COUNT ? netpass_stamps() : 0], netpass_stamps());
	fprintf(f, "dive %s\ndivelink %s\n", weekly_on() ? "on" : "none", dive_link_on() ? "on" : "off");
	if (weekly_on()) {
		fprintf(f, "pullback %s\n", director_crew_pull_spent() ? "spent" : "free");
		director_crew_describe(f);
	}
	/* (each part's own lines: the spares a trade could give and a swap kept, a dive's divers and crew; in the Square
	 * the trade under way, its screens and board, the Arena's challenge, battle and card, the week's net) */
	online_trade_describe(f, online_visit_away());
	dive_link_describe(f);
	party_play_describe(f);
	if (!online_visit_away()) return;
	OnlineSnapshot s;
	online_transport_snapshot(&s);
	square(f, &s);
	trade_screen_describe(f);
	online_board_describe(f, s.player_id);
	square_arena_describe(f);
	square_week_describe(f);
}
