#include "online_labels.h"

#include <string.h>

#include "bn6_fields.h"
#include "gfx.h"
#include "online_people.h"
#include "online_text.h"
#include "pixfont.h"
#include "square_life.h"
#include "platform.h"

static struct { int x, y; char name[ONLINE_NAME_BYTES + 1]; const char *phrase; } labels[ONLINE_MAX_PLAYERS + 1];
static unsigned count;

void online_labels_reset(void) { count = 0; }

static const char *phrase_for(const OnlineSnapshot *snapshot, uint32_t id, uint32_t now) {
	const OnlineEvent *last = NULL;
	for (unsigned i = 0; i < ONLINE_EVENT_HISTORY; ++i) {
		const OnlineEvent *e = &snapshot->events[i];
		if (e->serial && e->player_id == id && now - e->ticks < 4000 && (!last || (int32_t)(e->serial - last->serial) > 0)) last = e;
	}
	return last ? (last->kind ? online_emote_glyphs[last->value] : online_phrases[last->value]) : NULL;
}

static void label_at(int x, int y, int z, const int camera[3]) {
	/* BN6's overworld projection (bn6f sub_300631C), including the
	 * displayed camera rather than assuming MegaMan stays centered. */
	labels[count].x = 120 + x - camera[0] + y - camera[1];
	labels[count].y = 82 + (y - camera[1] - x + camera[0]) / 2 - z + camera[2] - 31;
}

void online_labels_update(const OnlineSnapshot *shown, uint32_t now) {
	count = 0;
	if (!shown->connected) return;
	uint32_t at = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CAMERA);
	if (at < BN6_EWRAM || at > BN6_EWRAM_END - 80) return;
	int camera[3] = { (int32_t)emu_read32(at + BN6_CAMERA_RENDER_X) >> 16,
		(int32_t)emu_read32(at + BN6_CAMERA_RENDER_Y) >> 16, (int32_t)emu_read32(at + BN6_CAMERA_RENDER_Z) >> 16 };
	for (size_t i = 0; i < shown->peer_count; ++i) {
		const OnlinePeer *peer = &shown->peers[i];
		label_at(peer->x, peer->y, peer->z, camera);
		memcpy(labels[count].name, peer->name, sizeof peer->name);
		unsigned flags = online_people_flags(peer->identity);
		labels[count].phrase = flags & (ONLINE_MUTED | ONLINE_BLOCKED) ? NULL : phrase_for(shown, peer->id, now);
		++count;
	}
	label_at(bn6_player_x(), bn6_player_y(), bn6_player_z(), camera);
	labels[count].name[0] = '\0';
	labels[count].phrase = phrase_for(shown, shown->player_id, now);
	++count;
	/* (a word over one of the Square's own: the greeter's, a walker's) */
	const char *bark;
	int x, y, z;
	if (count < sizeof labels / sizeof *labels && square_life_bark(&bark, &x, &y, &z)) {
		label_at(x, y, z, camera);
		labels[count].name[0] = '\0';
		labels[count++].phrase = bark;
	}
}

void online_labels_draw(void) {
	for (unsigned i = 0; i < count; ++i) {
		int x = labels[i].x, y = labels[i].y;
		if (x < 0 || x >= EMU_W || y < 18 || y > EMU_H - 20) continue;
		const char *text = labels[i].phrase ? labels[i].phrase : labels[i].name;
		if (!text[0]) continue;
		int w = pixfont_width(text, 1);
		if (x < w / 2 + 2) x = w / 2 + 2;
		if (x > EMU_W - w / 2 - 2) x = EMU_W - w / 2 - 2;
		fill_rect(P.core_x + x - w / 2 - 2, P.core_y + y - 1, w + 4, 11, rgba(9, 24, 42, 210));
		pixfont_draw(P.core_x + x, P.core_y + y, text, WHITE, BLACK, PIXFONT_CENTER, 1);
	}
}
