#include "online_scene.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6_fields.h"
#include "gfx.h"
#include "online_capture.h"
#include "online_labels.h"
#include "online_motion.h"
#include "online_people.h"
#include "online_square.h"
#include "online_text.h"
#include "online_transport.h"
#include "online_ui.h"
#include "online_visit.h"
#include "platform.h"

static OnlineSnapshot snapshot;
static OnlineSnapshot shown;
static bool built, failed, arrived;
static unsigned frames;
static int local_x, local_y, local_z;
static uint8_t facing;
static FILE *trace;
static uint32_t disconnected_at, notice_at, notice_serial;

void online_scene_start(void) {
	built = failed = arrived = false;
	frames = 0;
	local_x = local_y = local_z = 0;
	facing = 3;
	memset(&snapshot, 0, sizeof snapshot);
	memset(&shown, 0, sizeof shown);
	disconnected_at = notice_at = notice_serial = 0;
	online_motion_reset(); online_labels_reset(); online_ui_reset(); online_people_load();
	online_capture_start();
	OnlineProfile profile;
	online_visit_profile(&profile);
	if (!online_transport_profile(&profile)) fprintf(stderr, "online: public Navi card could not be queued\n");
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
	return online_ui_open() || online_capture_preview() ? 0 : keys & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT | KEY_B);
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

void online_scene_update(void) {
	online_transport_snapshot(&snapshot);
	uint32_t now = SDL_GetTicks();
	if (snapshot.notice_serial != notice_serial) { notice_serial = snapshot.notice_serial; notice_at = now; }
	if (snapshot.error[0] || failed) {
		if (!disconnected_at) disconnected_at = now;
		if (btn_pressed(BTN_B) || now - disconnected_at >= 2000) { online_visit_return(); return; }
	} else if (!snapshot.connected && btn_pressed(BTN_B)) { online_visit_return(); return; }
	if (online_capture_preview()) {
		if (btn_pressed(BTN_B)) { online_visit_return(); return; }
	} else online_ui_update(&snapshot);
	if (online_visit_transitioning()) return;
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
	online_motion_update(&snapshot, now, &shown);
	online_square_peers(&shown);
	online_labels_update(&shown, now);
}

void online_scene_draw(void) {
	int x = P.core_x, y = P.core_y;
	fill_rect(x, y, EMU_W, 15, rgba(9, 24, 42, 220));
	text_draw(x + 4, y + 1, online_title, WHITE, TEXT_LEFT);
	if (snapshot.connected) text_drawf(x + EMU_W - 4, y + 1, WHITE, TEXT_RIGHT, "%u / %u", snapshot.channel, snapshot.channels);
	if (failed || !snapshot.connected) {
		const char *status = failed ? online_map_failed : snapshot.error[0] ? snapshot.error : online_connecting;
		fill_rect(x, y + 62, EMU_W, 40, rgba(9, 24, 42, 230));
		text_drawf(x + 4, y + 65, WHITE, TEXT_LEFT, "%.32s", status);
		if (strlen(status) > 32) text_drawf(x + 4, y + 79, WHITE, TEXT_LEFT, "%.32s", status + 32);
		text_draw(x + 4, y + 109, online_back, WHITE, TEXT_LEFT);
		return;
	}
	online_labels_draw();
	fill_rect(x, y + EMU_H - 15, EMU_W, 15, rgba(9, 24, 42, 255));
	text_draw(x + 4, y + EMU_H - 14, online_footer, WHITE, TEXT_LEFT);
	online_ui_draw();
	online_capture_draw();
	if (notice_at && SDL_GetTicks() - notice_at < 4000) {
		fill_rect(x, y + 112, EMU_W, 28, rgba(9, 24, 42, 240));
		text_drawf(x + 4, y + 113, WHITE, TEXT_LEFT, "%.32s", snapshot.notice);
		text_drawf(x + 4, y + 126, WHITE, TEXT_LEFT, "%.32s", strlen(snapshot.notice) > 32 ? snapshot.notice + 32 : "");
	}
}
