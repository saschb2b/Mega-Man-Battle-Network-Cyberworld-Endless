/* The Net Square's open challenge and the link before one (square_ask.h). */
#include "square_ask.h"

#include "arena_text.h"
#include "arena_view.h"
#include "emu.h"
#include "gfx.h"
#include "online_chat.h"
#include "pixfont.h"
#include "platform.h"
#include "protocol.h"
#include "second_frame.h"
#include "square_arena.h"

#define STRIP_H     24
#define ANSWER_MS   30000u   /* the server's time for an answer, from the question shown (arena.py's PENDING) */
#define SWEEP_MS    1200u    /* a ringing PET's bar: a mark going to and fro */
#define BAR_W       150

static unsigned bars_of(uint32_t ms) {
	if (!ms) return 0;
	return ms < ONLINE_LINK_GOOD_MS ? 3u : ms < ONLINE_LINK_FAIR_MS ? 2u : 1u;
}

static unsigned square_link_bars(uint32_t mine_ms, unsigned his) {
	/* (mine always known: a link on this machine measures 0 ms) */
	unsigned mine = bars_of(mine_ms ? mine_ms : 1u);
	if (!mine || !his) return mine ? mine : his;
	return mine < his ? mine : his;
}

static const OnlinePeer *peer_of(const OnlineSnapshot *s, uint32_t id) {
	for (size_t i = 0; id && i < s->peer_count; ++i) if (s->peers[i].id == id) return &s->peers[i];
	return NULL;
}

/* the bars at the picture's top right, under a word of what they are */
static void corner_bars(unsigned bars) {
	int x = P.core_x + EMU_W - ARENA_BARS_W - 2, y = P.core_y + 2;
	arena_view_bars(x, y, bars);
}

/* the challenger's strip: whom it asks; his PET ringing (a mark sweeping)
 * until he reads it, then his time running down; the way to call it off */
static void strip(const OnlineSnapshot *s, uint32_t other, const char *name, uint32_t seen_ms, uint32_t now) {
	int x = P.core_x, y = P.core_y;
	fill_rect(x, y, EMU_W, STRIP_H, rgba(0, 49, 74, 220));
	fill_rect(x, y + STRIP_H - 1, EMU_W, 1, PET_CYAN);
	pixfont_draw(x + 4, y + 3, seen_ms ? arena_reading_it(name) : arena_ringing(name), PET_WHITE, rgba(0, 49, 74, 255), PIXFONT_LEFT, 1);
	int bx = x + 4, by = y + 13;
	fill_rect(bx, by, BAR_W, 3, PET_SLOT);
	if (seen_ms) {
		uint32_t left = seen_ms < ANSWER_MS ? ANSWER_MS - seen_ms : 0;
		fill_rect(bx, by, (int)((uint64_t)BAR_W * left / ANSWER_MS), 3, PET_CYAN_HI);
	} else {
		uint32_t t = now % (2 * SWEEP_MS), at = t < SWEEP_MS ? t : 2 * SWEEP_MS - t;
		fill_rect(bx + (int)((uint64_t)(BAR_W - 20) * at / SWEEP_MS), by, 20, 3, PET_GOLD);
	}
	pixfont_draw(x + EMU_W - ARENA_BARS_W - 6, y + 14, arena_call_off_key(name), PET_GOLD, rgba(0, 49, 74, 255), PIXFONT_RIGHT, 1);
	const OnlinePeer *p = peer_of(s, other);
	arena_view_bars(x + EMU_W - ARENA_BARS_W - 2, y + 2, square_link_bars(s->latency_ms, p ? p->link : 0));
}

void square_ask_draw(const OnlineSnapshot *s, uint32_t now) {
	uint32_t other, seen_ms;
	const char *name;
	if (square_arena_open(&other, &name, &seen_ms)) { strip(s, other, name, seen_ms, now); return; }
	/* (beside a card open, or the question of a challenge: the link with him) */
	uint32_t id = online_chat_card_open();
	if (!id) id = square_arena_asking();
	const OnlinePeer *p = peer_of(s, id);
	if (p) corner_bars(square_link_bars(s->latency_ms, p->link));
}
