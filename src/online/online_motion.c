#include "online_motion.h"

#include <string.h>

#include "online_people.h"

static OnlineSnapshot history[4];
static unsigned count;
static uint32_t serial;

void online_motion_reset(void) { count = serial = 0; memset(history, 0, sizeof history); }

static const OnlinePeer *find_peer(const OnlineSnapshot *snapshot, uint32_t id) {
	for (size_t i = 0; i < snapshot->peer_count; ++i) if (snapshot->peers[i].id == id) return &snapshot->peers[i];
	return NULL;
}

static int32_t blend(int32_t a, int32_t b, uint32_t elapsed, uint32_t span) {
	return span ? a + (int32_t)((int64_t)(b - a) * elapsed / span) : b;
}

void online_motion_update(const OnlineSnapshot *latest, uint32_t now, OnlineSnapshot *shown) {
	if (count && history[count - 1].channel != latest->channel) online_motion_reset();
	if (latest->position_serial != serial) {
		if (count == 4) { memmove(history, history + 1, 3 * sizeof *history); --count; }
		history[count++] = *latest;
		serial = latest->position_serial;
	}
	*shown = *latest;
	shown->peer_count = 0;
	if (!latest->connected || !count) return;
	uint32_t target = now - 100;
	const OnlineSnapshot *a = &history[0], *b = &history[0];
	for (unsigned i = 0; i < count; ++i) {
		b = &history[i];
		if ((int32_t)(b->position_ticks - target) >= 0) break;
		a = b;
	}
	uint32_t span = b->position_ticks - a->position_ticks;
	uint32_t elapsed = (int32_t)(target - a->position_ticks) < 0 ? 0 : target - a->position_ticks;
	if (elapsed > span) elapsed = span;
	for (size_t i = 0; i < latest->peer_count; ++i) {
		const OnlinePeer *current = &latest->peers[i];
		if (online_people_flags(current->identity) & ONLINE_BLOCKED) continue;
		OnlinePeer *out = &shown->peers[shown->peer_count++];
		*out = *current;
		const OnlinePeer *pa = find_peer(a, current->id), *pb = find_peer(b, current->id);
		if (!pa || !pb) continue;
		out->x = blend(pa->x, pb->x, elapsed, span);
		out->y = blend(pa->y, pb->y, elapsed, span);
		out->z = blend(pa->z, pb->z, elapsed, span);
		out->facing = pb->facing;
	}
}
