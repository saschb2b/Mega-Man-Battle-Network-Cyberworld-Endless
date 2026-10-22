#include "online_scene.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6_fields.h"
#include "cinema.h"
#include "emote_strip.h"
#include "gfx.h"
#include "online_chat.h"
#include "online_labels.h"
#include "online_mail.h"
#include "online_motion.h"
#include "online_people.h"
#include "online_square.h"
#include "online_trade.h"
#include "online_transport.h"
#include "netpass_screen.h"
#include "platform.h"
#include "reconnect_draw.h"
#include "square_arena.h"
#include "square_watch.h"
#include "trade_screen.h"
#include "square_week.h"

static OnlineSnapshot snapshot;
static OnlineSnapshot shown;
static bool built, arrived;
static unsigned frames;
static int local_x, local_y, local_z;
static uint8_t facing;
static FILE *trace;
static uint32_t disconnected_at, notice_at, notice_serial;
/* a dropped link (issue #153): the wait on, the resumes taken up, when the
 * link came back (its band shown a moment), and what a resume checks and
 * sends again: the Square's seed, this session, MegaMan's card */
#define BACK_SHOWN_MS 1200
static bool waiting;
static uint32_t resumes_seen, back_at, built_seed, session;
static OnlineProfile carried_card;

bool online_scene_start(uint32_t seed, uint32_t player, const OnlineProfile *card) {
	built = arrived = false;
	frames = 0;
	local_x = local_y = local_z = 0;
	facing = 3;
	memset(&snapshot, 0, sizeof snapshot);
	memset(&shown, 0, sizeof shown);
	disconnected_at = notice_at = notice_serial = 0;
	waiting = false;
	resumes_seen = back_at = 0;
	built_seed = seed;
	session = player;
	carried_card = *card;
	online_motion_reset(); online_labels_reset(); online_chat_reset(); online_people_load();
	square_arena_reset();
	square_watch_reset();
	if (!online_transport_profile(card)) fprintf(stderr, "online: public Navi card could not be queued\n");
	const char *path = getenv("CYBERWORLD_ONLINE_TRACE");
	trace = path && *path ? fopen(path, "w") : NULL;
	if (path && *path && !trace) fprintf(stderr, "online: cannot write trace %s\n", path);
	built = online_square_build(seed, player);
	if (built) fprintf(stderr, "online: square seed=%u hash=%u player=%u\n", seed, online_square_hash(), player);
	else fprintf(stderr, "online: square could not be built\n");
	return built;
}

void online_scene_end(void) {
	emote_strip_close();
	square_arena_end();
	square_watch_end();
	if (trace) { fclose(trace); trace = NULL; }
	built = arrived = false;
}

uint32_t online_scene_keys(uint32_t keys) {
	/* (Trade Central's screens up over the Square: the game held under them) */
	if (trade_screen_update()) return 0;
	return online_chat_keys(keys);
}

static void trace_frame(void) {
	if (!trace || frames % 6u) return;
	fprintf(trace, "{\"frame\":%u,\"ticks_ms\":%u,\"latency_ms\":%u,\"seed\":%u,\"world_hash\":%u,\"player_id\":%u,\"connected\":%s,"
		"\"channel\":%u,\"card_serial\":%u,\"card_player\":%u,\"card_hp\":%u,\"event_serial\":%u,\"visible_peers\":%zu,"
		"\"local\":{\"x\":%d,\"y\":%d,\"z\":%d,\"facing\":%u},\"peers\":[",
		frames, SDL_GetTicks(), snapshot.latency_ms, snapshot.seed, online_square_hash(), snapshot.player_id, snapshot.connected ? "true" : "false",
		snapshot.channel, snapshot.card_serial, snapshot.card_player, snapshot.card.hp, snapshot.event_serial, shown.peer_count,
		local_x, local_y, local_z, (unsigned)facing);
	for (size_t i = 0; i < snapshot.peer_count; ++i) {
		const OnlinePeer *p = &snapshot.peers[i];
		int x = 0, y = 0, z = 0;
		const OnlinePeer *rendered = NULL;
		bool found = false;
		for (size_t j = 0; j < shown.peer_count; ++j) if (shown.peers[j].id == p->id) {
			rendered = &shown.peers[j];
			found = online_square_peer_position((unsigned)j, &x, &y, &z);
		}
		fprintf(trace, "%s{\"id\":%u,\"x\":%d,\"y\":%d,\"z\":%d,\"npc_match\":%s,"
			"\"npc_x\":%d,\"npc_y\":%d,\"npc_z\":%d,\"rendered_x\":%d,\"rendered_y\":%d,\"rendered_z\":%d}", i ? "," : "", p->id,
			p->x, p->y, p->z, found ? "true" : "false", x, y, z,
			rendered ? rendered->x : 0, rendered ? rendered->y : 0, rendered ? rendered->z : 0);
	}
	fputs("]}\n", trace);
	fflush(trace);
}

/* the players this one has blocked out of the view (their MegaMen, names
 * and words) */
static void unblocked(OnlineSnapshot *v) {
	size_t n = 0;
	for (size_t i = 0; i < v->peer_count; ++i)
		if (!(online_people_flags(v->peers[i].identity) & ONLINE_BLOCKED)) v->peers[n++] = v->peers[i];
	v->peer_count = n;
}

/* the link back on a resume (the transport's resumes): another Square ends
 * the visit; else what was under way is over, as the server ended it at the
 * drop (server/cyberworld_server/holds.py), and a new session (a server
 * restarted) never knew of a trade, nor of MegaMan's card */
static VisitEnd resumed(void) {
	resumes_seen = snapshot.resumes;
	fprintf(stderr, "online: back in the Square (seed %u, player %u, channel %u)\n", snapshot.seed, snapshot.player_id, snapshot.channel);
	if (snapshot.seed != built_seed) return VISIT_END_MOVED;
	square_arena_reset();
	square_watch_reset();
	if (snapshot.player_id != session) online_trade_lost();
	session = snapshot.player_id;
	if (!online_transport_profile(&carried_card)) fprintf(stderr, "online: public Navi card could not be queued\n");
	return VISIT_END_NONE;
}

bool online_scene_waiting(void) { return built && waiting; }

VisitEnd online_scene_wait(void) {
	online_transport_snapshot(&snapshot);
	if (snapshot.error[0]) return online_visit_refusal(&snapshot, true);
	if (btn_pressed(BTN_B)) {
		fprintf(stderr, "online: the wait for the link given up\n");
		return VISIT_END_LOST;
	}
	if (!snapshot.connected || snapshot.quiet || snapshot.reconnecting) return VISIT_END_NONE;
	waiting = false;
	back_at = SDL_GetTicks();
	return VISIT_END_NONE;
}

VisitEnd online_scene_update(void) {
	if (!built) return VISIT_END_BROKEN;
	online_transport_snapshot(&snapshot);
	/* (back on a resume: what was under way ended before the new session's
	 * mail is read, a trade it offers among it) */
	bool up = !snapshot.error[0] && snapshot.connected && !snapshot.quiet && !snapshot.reconnecting;
	if (up && snapshot.resumes != resumes_seen) {
		VisitEnd why = resumed();
		if (why != VISIT_END_NONE) return why;
	}
	online_mail_frame();
	/* (the director rests here: the cinema told when a screen of BN6's,
	 * a shop, takes MegaMan off the map, so its keys reach it) */
	cinema_on_map(emu_read8(emu_read32(BN6_TOOLKIT)) == BN6_MODE_GAME);
	uint32_t now = SDL_GetTicks();
	/* (a challenge's refusal MegaMan says himself: square_arena_notice) */
	if (snapshot.notice_serial != notice_serial) {
		notice_serial = snapshot.notice_serial;
		if (!square_arena_notice(snapshot.notice_serial, snapshot.notice_code)) notice_at = now;
	}
	/* (the link quiet or dropped, and tried again: the game held under the
	 * wait from the next frame, online_scene_wait) */
	if (!snapshot.error[0] && (snapshot.quiet || snapshot.reconnecting)) {
		if (!waiting) fprintf(stderr, "online: the link to the Square is %s; holding\n", snapshot.reconnecting ? "down" : "quiet");
		waiting = true;
		return VISIT_END_NONE;
	}
	if (snapshot.error[0] || !snapshot.connected) {
		if (!disconnected_at) disconnected_at = now;
		/* (the reason read a moment before the way home) */
		if (now - disconnected_at >= 1500) return online_visit_refusal(&snapshot, true);
		return VISIT_END_NONE;
	}
	arrived = online_square_arrived();
	if (!arrived) return VISIT_END_NONE;
	++frames;
	local_x = bn6_player_x(); local_y = bn6_player_y(); local_z = bn6_player_z();
	facing = emu_read8(BN6_PLAYER_FACING) & 7u;
	online_transport_poll(local_x, local_y, local_z, facing);
	trace_frame();
	online_motion_update(&snapshot, now, &shown);
	unblocked(&shown);
	online_square_peers(&shown);
	if (online_chat_update(&shown)) return square_week_leaving() ? VISIT_END_DIVE : VISIT_END_LEFT;
	square_arena_update(&snapshot);
	square_watch_update(&snapshot);
	online_labels_update(&shown, now);
	return VISIT_END_NONE;
}

void online_scene_draw(void) {
	if (!built) return;
	int x = P.core_x, y = P.core_y;
	uint32_t now = SDL_GetTicks();
	/* (the wait over the Square held still, and the moment the link is back) */
	if (waiting || (back_at && now - back_at < BACK_SHOWN_MS)) {
		ReconnectView v = { .back = !waiting, .elapsed = now - snapshot.lost_at, .window = ONLINE_RECONNECT_MS, .t = now };
		if (waiting) { reconnect_draw(x, y, &v); return; }
		if (arrived) online_labels_draw();
		reconnect_draw(x, y, &v);
		return;
	}
	if (!arrived) return;
	/* (the names and marks over heads, an open challenge's strip and the link beside a card: online_labels.h) */
	online_labels_draw();
	/* (SELECT's emote strip: online_chat.c) */
	emote_strip_draw();
	/* (Trade Central's screens, and the NetPass over it all, START's: online_chat.c) */
	trade_screen_draw();
	netpass_screen_draw();
	if (notice_at && SDL_GetTicks() - notice_at < 4000) {
		fill_rect(x, y + 112, EMU_W, 28, rgba(9, 24, 42, 240));
		text_drawf(x + 4, y + 113, WHITE, TEXT_LEFT, "%.32s", snapshot.notice);
		text_drawf(x + 4, y + 126, WHITE, TEXT_LEFT, "%.32s", strlen(snapshot.notice) > 32 ? snapshot.notice + 32 : "");
	}
}
