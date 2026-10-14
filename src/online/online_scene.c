#include "online_scene.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6_fields.h"
#include "gfx.h"
#include "online_square.h"
#include "online_snapshot.h"
#include "online_text.h"
#include "online_transport.h"
#include "platform.h"

static OnlineSnapshot snapshot;
static bool built, failed, arrived;
static unsigned frames;
static int local_x, local_y, local_z;
static uint8_t facing;
static FILE *trace;
static OnlineLoadout loadout;

void online_scene_start(void) {
	built = failed = arrived = false;
	frames = 0;
	local_x = local_y = local_z = 0;
	facing = 3;
	memset(&snapshot, 0, sizeof snapshot);
	/* This boot-room sample tests the reader; carrying a run into the
	 * visit is a later integration gate, documented in ONLINE.md. */
	if (online_snapshot_read(&loadout, 0))
		fprintf(stderr, "online: loadout bytes=%zu hp=%u max_hp=%u\n", sizeof loadout, loadout.hp, loadout.max_hp);
	const char *path = getenv("CYBERWORLD_ONLINE_TRACE");
	trace = path && *path ? fopen(path, "w") : NULL;
	if (path && *path && !trace) fprintf(stderr, "online: cannot write trace %s\n", path);
}

void online_scene_end(void) {
	if (trace) { fclose(trace); trace = NULL; }
	online_transport_end();
	built = arrived = false;
}

bool online_scene_enabled(void) { return online_transport_enabled(); }
bool online_scene_ready(void) { return built; }
bool online_scene_arrived(void) { return arrived; }

uint32_t online_scene_keys(uint32_t keys) {
	/* This technical room only exercises walking and running. The PET and
	 * jack-out flows belong to later phases, and cannot save a square. */
	return keys & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT | KEY_B);
}

static void trace_frame(void) {
	if (!trace || frames % 6u) return;
	fprintf(trace, "{\"frame\":%u,\"ticks_ms\":%u,\"latency_ms\":%u,\"seed\":%u,\"world_hash\":%u,\"player_id\":%u,\"connected\":%s,"
		"\"local\":{\"x\":%d,\"y\":%d,\"z\":%d,\"facing\":%u},\"peers\":[",
		frames, SDL_GetTicks(), snapshot.latency_ms, snapshot.seed, online_square_hash(), snapshot.player_id, snapshot.connected ? "true" : "false",
		local_x, local_y, local_z, (unsigned)facing);
	for (size_t i = 0; i < snapshot.peer_count; ++i) {
		const OnlinePeer *p = &snapshot.peers[i];
		int x = 0, y = 0, z = 0;
		bool found = online_square_peer_position((unsigned)i, &x, &y, &z);
		fprintf(trace, "%s{\"id\":%u,\"x\":%d,\"y\":%d,\"z\":%d,\"npc_match\":%s,"
			"\"npc_x\":%d,\"npc_y\":%d,\"npc_z\":%d}", i ? "," : "", p->id,
			p->x, p->y, p->z, found ? "true" : "false", x, y, z);
	}
	fputs("]}\n", trace);
	fflush(trace);
}

void online_scene_update(void) {
	online_transport_snapshot(&snapshot);
	if (!built) {
		if (!snapshot.connected || failed) return;
		built = online_square_build(snapshot.seed, snapshot.player_id);
		failed = !built;
		if (built) fprintf(stderr, "online: square seed=%u hash=%u player=%u\n", snapshot.seed, online_square_hash(), snapshot.player_id);
		else fprintf(stderr, "online: square could not be built\n");
		return;
	}
	arrived = online_square_arrived();
	if (!arrived) return;
	++frames;
	local_x = bn6_player_x(); local_y = bn6_player_y(); local_z = bn6_player_z();
	facing = emu_read8(BN6_PLAYER_FACING) & 7u;
	online_transport_poll(local_x, local_y, local_z, facing);
	trace_frame();
	online_square_peers(&snapshot);
}

void online_scene_draw(void) {
	int x = P.core_x, y = P.core_y;
	fill_rect(x, y, EMU_W, 15, rgba(9, 24, 42, 220));
	text_draw(x + 4, y + 1, online_title, WHITE, TEXT_LEFT);
	if (failed || !snapshot.connected) {
		const char *status = failed ? online_map_failed : snapshot.error[0] ? snapshot.error : online_connecting;
		fill_rect(x, y + 62, EMU_W, 40, rgba(9, 24, 42, 230));
		text_drawf(x + 4, y + 65, WHITE, TEXT_LEFT, "%.32s", status);
		if (strlen(status) > 32) text_drawf(x + 4, y + 79, WHITE, TEXT_LEFT, "%.32s", status + 32);
		return;
	}
	fill_rect(x, y + EMU_H - 15, EMU_W, 15, rgba(9, 24, 42, 210));
	char peers[64];
	if (snapshot.peer_count) online_peer_text(peers, sizeof peers, snapshot.peers[0].name, (unsigned)snapshot.peer_count);
	text_draw(x + 4, y + EMU_H - 14, snapshot.peer_count ? peers : online_waiting, WHITE, TEXT_LEFT);
}
