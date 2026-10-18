#include "online_scene.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6_fields.h"
#include "cinema.h"
#include "gfx.h"
#include "online_chat.h"
#include "online_labels.h"
#include "online_mail.h"
#include "online_motion.h"
#include "online_people.h"
#include "online_square.h"
#include "online_text.h"
#include "online_transport.h"
#include "platform.h"
#include "square_arena.h"
#include "trade_order.h"

static OnlineSnapshot snapshot;
static OnlineSnapshot shown;
static bool built, arrived;
static unsigned frames;
static int local_x, local_y, local_z;
static uint8_t facing;
static FILE *trace;
static uint32_t disconnected_at, notice_at, notice_serial;

bool online_scene_start(uint32_t seed, uint32_t player, const OnlineProfile *card) {
	built = arrived = false;
	frames = 0;
	local_x = local_y = local_z = 0;
	facing = 3;
	memset(&snapshot, 0, sizeof snapshot);
	memset(&shown, 0, sizeof shown);
	disconnected_at = notice_at = notice_serial = 0;
	online_motion_reset(); online_labels_reset(); online_chat_reset(); online_people_load();
	square_arena_reset();
	if (!online_transport_profile(card)) fprintf(stderr, "online: public Navi card could not be queued\n");
	const char *path = getenv("CYBERWORLD_ONLINE_TRACE");
	trace = path && *path ? fopen(path, "w") : NULL;
	if (path && *path && !trace) fprintf(stderr, "online: cannot write trace %s\n", path);
	built = online_square_build(seed, player);
	/* (BN6's order screen in Trade Central's words while he is here) */
	if (built && !trade_order_words(true)) fprintf(stderr, "online: the order screen keeps BN6's words\n");
	if (built) fprintf(stderr, "online: square seed=%u hash=%u player=%u\n", seed, online_square_hash(), player);
	else fprintf(stderr, "online: square could not be built\n");
	return built;
}

void online_scene_end(void) {
	trade_order_words(false);
	square_arena_end();
	if (trace) { fclose(trace); trace = NULL; }
	built = arrived = false;
}

uint32_t online_scene_keys(uint32_t keys) {
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

VisitEnd online_scene_update(void) {
	if (!built) return VISIT_END_BROKEN;
	online_transport_snapshot(&snapshot);
	online_mail_frame();
	/* (the director rests here: the cinema told when a screen of BN6's,
	 * a shop, takes MegaMan off the map, so its keys reach it) */
	cinema_on_map(emu_read8(emu_read32(BN6_TOOLKIT)) == BN6_MODE_GAME);
	uint32_t now = SDL_GetTicks();
	if (snapshot.notice_serial != notice_serial) { notice_serial = snapshot.notice_serial; notice_at = now; }
	if (snapshot.error[0] || !snapshot.connected) {
		if (!disconnected_at) disconnected_at = now;
		/* (the reason read a moment before the way home) */
		if (now - disconnected_at >= 1500) return snapshot.error_code == 14 ? VISIT_END_REMOVED : VISIT_END_LOST;
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
	if (online_chat_update(&shown)) return VISIT_END_LEFT;
	square_arena_update(&snapshot);
	online_labels_update(&shown, now);
	return VISIT_END_NONE;
}

void online_scene_draw(void) {
	if (!built || !arrived) return;
	int x = P.core_x, y = P.core_y;
	online_labels_draw();
	if (notice_at && SDL_GetTicks() - notice_at < 4000) {
		fill_rect(x, y + 112, EMU_W, 28, rgba(9, 24, 42, 240));
		text_drawf(x + 4, y + 113, WHITE, TEXT_LEFT, "%.32s", snapshot.notice);
		text_drawf(x + 4, y + 126, WHITE, TEXT_LEFT, "%.32s", strlen(snapshot.notice) > 32 ? snapshot.notice + 32 : "");
	}
}
