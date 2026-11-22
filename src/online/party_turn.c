/* party_turn.h */
#include "party_turn.h"

#include <stdio.h>
#include <string.h>

#include "arena_input.h"
#include "arena_team_custom.h"
#include "arena_team_screen.h"
#include "bn6.h"
#include "emu.h"
#include "rom.h"

#define OUT_MOST (TEAM_CUSTOM_PICKS + 2)   /* a turn's messages: a Program Advance, his picks (or none) */

typedef struct {
	TeamScreen *screen;
	bool turn;                      /* his turn under way on his game (his screen opened for it) */
	uint16_t deck[BN6_DECK_SIZE];   /* ... his deck as the field dealt it */
	unsigned dealt;
	uint16_t out[OUT_MOST];         /* ... what his OK said, one a frame */
	unsigned queued, sent;
	uint32_t t;                     /* ... his screen's frames */
	unsigned opened, oks, at_limit, closed_first, pas, failed;
	unsigned seen_hp;               /* the HP his screen's HP box last showed (the state's, issue #757) */
} Turn;

/* by seat (1-3): one on a partner's game, each stood-in partner's under the dev's partycore */
static Turn T[PARTY_MOST];

void party_turn_reset(void) {
	for (unsigned k = 0; k < PARTY_MOST; ++k) {
		Turn *t = &T[k];
		team_screen_free(t->screen);
		t->screen = NULL;
		t->turn = false;
		t->queued = t->sent = 0;
	}
}

/* his picks (and the Program Advance his OK made) into his stream, one a frame */
static void say(Turn *t, unsigned seat, const uint8_t *picks, unsigned n) {
	t->queued = t->sent = 0;
	uint16_t built[BN6_HAND_MAX], pa = team_screen_pa(t->screen, built);
	unsigned at, took;
	if (pa && team_pa_found(t->deck, t->dealt, picks, n, pa, built, &at, &took)) {
		t->out[t->queued++] = TEAM_PA(at, took, pa);
		++t->pas;
		fprintf(stderr, "party: seat %u's Program Advance %03X from his picks %u to %u, as BN6's OK built it on his screen\n", seat, pa,
			at, at + took - 1);
	}
	if (!n) t->out[t->queued++] = TEAM_PICK(0, 0, 0);
	for (unsigned i = 0; i < n && i < TEAM_CUSTOM_PICKS; ++i) t->out[t->queued++] = TEAM_PICK(n, i, picks[i]);
	uint16_t chip[TEAM_CUSTOM_PICKS] = { 0 };
	for (unsigned i = 0; i < n && i < TEAM_CUSTOM_PICKS; ++i) chip[i] = picks[i] < t->dealt ? (uint16_t)(t->deck[picks[i]] & 0x1FFu) : 0;
	fprintf(stderr, "party: seat %u's turn %u: %u picks said (slots %u %u %u %u %u: chips %03X %03X %03X %03X %03X)\n", seat, t->opened, n,
		n > 0 ? picks[0] : 0, n > 1 ? picks[1] : 0, n > 2 ? picks[2] : 0, n > 3 ? picks[3] : 0, n > 4 ? picks[4] : 0, chip[0], chip[1],
		chip[2], chip[3], chip[4]);
}

/* the field's screen closed before his OK: the picks he has go out */
static void closed_first(Turn *t, unsigned seat) {
	if (!t->turn || !team_screen_open_now(t->screen)) return;
	uint8_t picks[TEAM_CUSTOM_PICKS];
	unsigned n = team_screen_picks(t->screen, picks);
	++t->closed_first;
	say(t, seat, picks, n);
	team_screen_close(t->screen);
}

/* partybot=1 on his screen: the chip under the cursor taken, the cursor a chip right, every 20 frames (BN6 takes the chips
 * that fit, and at last OK: the Arena's stood-in partners' screen_keys) */
static uint16_t bot_keys(uint32_t t) {
	unsigned f = t % 20;
	return f < 2 ? KEY_A : f >= 10 && f < 12 ? KEY_RIGHT : 0;
}

/* his turn begun: his screen made once (its cost the first turn's), the field copied in with his deck */
static void turn_open(Turn *t, PartyCore *field, const uint8_t *rom, unsigned seat) {
	if (!t->screen && !(t->screen = team_screen_one(R.data, ROM_SIZE, rom, EMU_ROM_SIZE))) {
		if (!t->failed++) fprintf(stderr, "party: seat %u's Custom screen's core could not be made: his picks none\n", seat);
		return;
	}
	unsigned n = t->dealt = party_core_deck(field, seat, t->deck);
	unsigned hp, max;
	party_core_seat_hp(field, seat, &hp, &max);
	t->turn = n && team_screen_open_one(t->screen, party_core_gba(field), seat - 1, t->deck, n, hp, max);
	t->queued = t->sent = 0;
	t->t = 0;
	t->opened += t->turn;
	if (t->turn)
		fprintf(stderr, "party: seat %u's turn %u: his own screen open at the field's frame %u, his deck %u chips (the first %03X)\n", seat,
			t->opened, (unsigned)party_core_frames(field), n, t->deck[0] & 0x1FFu);
}

uint16_t party_turn_word(PartyCore *field, const uint8_t *rom, unsigned seat, uint16_t pad, bool bot) {
	if (!seat || seat >= PARTY_MOST) return (uint16_t)(pad & ARENA_KEYS_MASK);
	Turn *t = &T[seat];
	bool custom = party_core_phase(field) == BN6_PHASE_CUSTOM && party_core_hp(field, seat);
	if (!custom) {
		closed_first(t, seat);
		t->turn = false;
	} else if (!t->turn && party_core_picking(field, seat)) turn_open(t, field, rom, seat);
	if (t->sent < t->queued) return t->out[t->sent++];
	if (!t->turn) return custom ? 0 : (uint16_t)(pad & ARENA_KEYS_MASK);
	if (!team_screen_open_now(t->screen)) return 0;
	uint8_t picks[TEAM_CUSTOM_PICKS];
	unsigned n;
	/* (near the turn's limit, someone's OK TEAM_TURN_LIMIT frames ago: the picks he has go out, in time to be his) */
	if (party_core_clock(field) + TEAM_WAIT_SAY >= TEAM_TURN_LIMIT) {
		n = team_screen_picks(t->screen, picks);
		++t->at_limit;
		say(t, seat, picks, n);
		team_screen_close(t->screen);
		return t->out[t->sent++];
	}
	uint16_t keys = bot ? bot_keys(t->t) : (uint16_t)(pad & ARENA_KEYS_MASK);
	++t->t;
	t->seen_hp = team_screen_one_hp(t->screen);
	if (team_screen_frame(t->screen, keys, picks, &n)) {
		++t->oks;
		say(t, seat, picks, n);
		return t->out[t->sent++];
	}
	return 0;
}

const uint32_t *party_turn_video(unsigned seat) {
	const Turn *t = seat && seat < PARTY_MOST ? &T[seat] : NULL;
	return t && t->turn && team_screen_open_now(t->screen) ? team_screen_video(t->screen) : NULL;
}

unsigned party_turn_hp(unsigned seat) { return seat && seat < PARTY_MOST ? T[seat].seen_hp : 0; }

void party_turn_report(unsigned seat) {
	const Turn *t = seat && seat < PARTY_MOST ? &T[seat] : NULL;
	if (!t) return;
	fprintf(stderr, "party: seat %u's turns: %u opened, %u OKs, %u at the limit, %u closed first, %u Program Advances; his screen's frame "
		"%.2f ms (the worst %.2f), its copy %.2f ms (the worst %.2f)\n", seat, t->opened, t->oks, t->at_limit, t->closed_first, t->pas,
		t->screen ? team_screen_ms(t->screen, false) : 0.0, t->screen ? team_screen_ms(t->screen, true) : 0.0,
		t->screen ? team_screen_copy_ms(t->screen, false) : 0.0, t->screen ? team_screen_copy_ms(t->screen, true) : 0.0);
}

uint16_t party_turn_guess(uint16_t last) { return party_turn_message(last) ? 0 : last; }

bool party_turn_message(uint16_t word) { return (word & TEAM_PICK_MARK) != 0; }

int party_turn_mode(unsigned seat) {
	const Turn *t = seat && seat < PARTY_MOST ? &T[seat] : NULL;
	return t && t->turn && team_screen_open_now(t->screen) ? (int)team_screen_mode(t->screen) : -1;
}
