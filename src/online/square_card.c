/* The result card's course and the Arena's two PET screens drawn from
 * the Net Square's state (square_arena_state.h, arena_card.h): the card's
 * keys (the rematch's Yes and No, the cursor on No; B or A back to the
 * Square), the other gone, its time and its fade out; the stall note's
 * way out; the way in's panel
 * (both players, each MegaMan as carried, the set's score, whose PET is
 * still on its way) and the battle under the other's plate, the link's
 * badge and a stall's note. */
#include "square_arena_state.h"

#include <stdio.h>
#include <string.h>

#include "arena_view.h"
#include "bn6.h"
#include "online_visit.h"
#include "platform.h"

/* the card up from the battle's end (or a stop), from black */
void square_card_up(unsigned result, ArenaAsk ask) {
	SA.card.up = true;
	SA.card.closing = 0;
	SA.card.result = result;
	SA.card.ask = ask;
	SA.card.yes = false;
	SA.card.theirs = ARENA_THEIRS_UNSAID;
	if (SA.arena) arena_tally(SA.arena, &SA.card.tally);
	else memset(&SA.card.tally, 0, sizeof SA.card.tally);
	SA.t = 0;
	arena_card_cue(ARENA_CUE_OPEN);
}

/* the rematch on the card answered No, or the other gone */
void square_card_gone(void) {
	SA.card.theirs = ARENA_THEIRS_GONE;
	SA.card.ask = ARENA_ASK_NONE;
	arena_card_cue(ARENA_CUE_NO);
}

/* Lan's answer on the card: Yes sends the rematch (or takes his, held),
 * No closes the card */
static void card_answer(bool yes) {
	if (!yes) { SA.card.closing = 1; arena_card_cue(ARENA_CUE_NO); return; }
	arena_card_cue(ARENA_CUE_YES);
	SA_AGAIN.answer = AGAIN_YES;
	SA.card.ask = ARENA_ASK_SENT;
	printf("arena: a rematch with %s: yes on the card\n", SA_AGAIN.name);
	uint32_t held = SA_AGAIN.held;
	SA_AGAIN.held = 0;
	if (held) square_arena_again_take(held);
	else square_arena_again_send();
}

/* The card's frame: its keys, the other gone from the Square, its time,
 * and its fade out back to the Square */
void square_card_frame(void) {
	if (SA.card.closing) {
		if (++SA.card.closing > FADE) square_arena_back();
		return;
	}
	if (SA.card.ask != ARENA_ASK_NONE && !square_arena_name_of(SA.other)) square_card_gone();
	bool keys = SA.t >= OVER_HELD;
	if (SA.card.ask == ARENA_ASK_OPEN && keys && (btn_pressed(BTN_LEFT) || btn_pressed(BTN_RIGHT))) {
		SA.card.yes = !SA.card.yes;
		arena_card_cue(ARENA_CUE_CURSOR);
	} else if (SA.card.ask == ARENA_ASK_OPEN && keys && btn_pressed(BTN_A)) card_answer(SA.card.yes);
	else if (keys && (btn_pressed(BTN_B) || (SA.card.ask != ARENA_ASK_OPEN && btn_pressed(BTN_A)))) {
		SA.card.closing = 1;
		arena_card_cue(ARENA_CUE_CLOSE);
	}
	/* (its time: a rematch's answer kept, a stop's card a while) */
	bool asked = SA.card.ask != ARENA_ASK_NONE || SA.card.theirs != ARENA_THEIRS_UNSAID;
	if (!SA.card.closing && SA.t >= (asked ? AGAIN_MS * 60u / 1000 : OVER_TICKS)) SA.card.closing = 1;
}

bool square_card_leave(void) { return SA.arena && arena_waited(SA.arena) >= STALL_OFFER && btn_pressed(BTN_B); }

/* the way in's panel: both players, each MegaMan as carried, the set's
 * score, whose PET is still on its way */
static void way_in(void) {
	ArenaWayView v = { .name = { online_visit_name(), SA.name }, .known = { true, SA.theirs_in }, .t = SA.t };
	v.loadout[0] = SA.mine_carried ? &SA.mine : NULL;
	v.loadout[1] = SA.theirs_carried ? &SA.theirs : NULL;
	uint64_t hash;
	v.wait[0] = SA.stage == SA_READY ? ARENA_SIDE_READY : ARENA_SIDE_LINKING;
	v.wait[1] = !SA.theirs_in ? ARENA_SIDE_COMING : arena_link_net_peer_ready(&SA.link, &hash) ? ARENA_SIDE_READY : ARENA_SIDE_LINKING;
	v.progress = SA.arena && SA.stage >= SA_CONNECTING ? arena_progress(SA.arena) : 0;
	const SquareBout *b = &SA_LAST.bout;
	v.set = SA_LAST.other == SA.other && !strcmp(SA_LAST.name, SA.name) && b->won + b->lost > 0;
	v.won = b->won;
	v.lost = b->lost;
	arena_card_way(&v);
	arena_card_fade(255 - (int)SA.t * 255 / FADE, true);
}

/* the result card: the battle's tally as this side reads it */
static void card(void) {
	const ArenaTally *t = &SA.card.tally;
	unsigned me = SA.link.link.side & 1u;
	ArenaCardView v = { .name = SA.name, .result = SA.card.result, .t = SA.t, .square = true,
		.ask = SA.card.ask, .yes = SA.card.yes, .theirs = SA.card.theirs, .tally = t->seen, .frames = t->fought };
	if (SA.card.result >= 4) v.why = arena_stop_line(SA.stop, SA.name);
	for (unsigned k = 0; k < 2; ++k) {
		v.hp[k] = t->hp[me ^ k];
		v.max_hp[k] = t->max_hp[me ^ k];
		v.used[k] = t->used[me ^ k];
	}
	const SquareBout *b = &SA_LAST.bout;
	v.set = SA.card.result != 5 && b->won + b->lost > 0;
	v.won = b->won;
	v.lost = b->lost;
	arena_card_result(&v);
	arena_card_fade(SA.card.closing ? SA.card.closing * 255 / FADE : 255 - (int)SA.t * 255 / FADE, false);
}

void square_arena_draw(void) {
	arena_view_clear();
	if (SA.card.up) { card(); return; }
	const uint32_t *v = SA.arena ? arena_video(SA.arena) : NULL;
	if (SA.stage == SA_BATTLE && v) {
		arena_view_core(v);
		arena_view_over(SA.name, arena_phase(SA.arena, SA.link.link.side) == BN6_PHASE_CUSTOM, arena_link_state(SA.arena), SA.t);
		uint32_t waited = arena_waited(SA.arena);
		if (waited >= STALL_NOTE)
			arena_view_stall(SA.name, waited < STALL_END ? STALL_END - waited : 0, STALL_END - STALL_NOTE, waited >= STALL_OFFER);
		arena_card_fade(255 - (int)SA.t * 255 / FADE, true);
		return;
	}
	way_in();
}
