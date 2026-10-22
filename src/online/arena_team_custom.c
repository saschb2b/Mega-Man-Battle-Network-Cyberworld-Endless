/* --dev arena2v2: a partner's own folder, deck and picks in the pair (arena_team_custom.h). */
#include "arena_team_custom.h"

#include <stdio.h>
#include <string.h>

#include <mgba/core/core.h>

#include "arena_input.h"
#include "arena_seats.h"
#include "arena_team_link.h"
#include "arena_team_soul.h"
#include "bn6.h"
#include "chips.h"
#include "devtools.h"
#include "free_space.h"
#include "hook.h"

#define SIDES     2
#define FOLDER    BN6_FOLDER_ENTRIES
#define NONE8     0xFFu
#define NONE16    0xFFFFu
#define POWER_COMPUTED 1000   /* a record's power from here on is a routine's number (BN6_CHIP_COMPUTED_POWER) */

/* A side's room in RAM_TEAM_CUSTOM, the same in both cores (our own
 * layout, not the game's): */
#define ROOM_SIZE  0x38
#define R_DECK     0x00   /* his deck: folder indices in the order drawn, NONE8 past the last */
#define R_STATE    0x1E   /* his turn: ST_* */
#define R_PICKED   0x1F   /* the picks his stream has said for it, */
#define R_PICKS    0x20   /* ... their slots of the hand his screen dealt (the first of his deck), in order */
#define R_TAKE_AT  0x26   /* u16 the iteration his hand takes his picks (0 none set) */
#define R_TAKEN    0x28   /* how many picks, */
#define R_TAKE     0x29   /* ... their folder indices */
#define R_DEALT    0x2E   /* 1 once his deck is dealt this battle */
#define R_OWED     0x2F   /* 1 while his side's GBA holds its Custom screen's packet back for him (its own core's alone) */
#define R_WAIT     0x30   /* u16 his side's GBA's frames held so, this turn */
#define R_CROSS    0x32   /* the form his Custom screen's CROSSSELECT committed this turn (0 none): his pending form */
#define R_PA       0x33   /* a Program Advance his screen made this turn: its first pick + 1 | (its picks - 3) << 4 (0 none), */
#define R_PA_ID    0x34   /* ... its chip less BN6_PA_FIRST; */
#define R_TAKE_PA  0x35   /* ... the two as his picks were set (R_TAKE's) */
#define R_TAKE_PA_ID 0x36
#define R_LIMIT    0x37   /* 1 once his side's GBA has said the wait's limit this turn (its own core's alone) */

/* his turn: no Custom screen; his side's GBA's open, his picks not in yet;
 * open with them in; closed before they came */
enum { ST_CLOSED, ST_OPEN, ST_IN, ST_WAITING };

_Static_assert(RAM_TEAM_CUSTOM + SIDES * ROOM_SIZE <= RAM_TEAM_CUSTOM_END, "the partners' decks and turns past their room");
_Static_assert(R_PICKS + TEAM_CUSTOM_PICKS <= R_TAKE_AT && R_TAKE + TEAM_CUSTOM_PICKS <= R_DEALT && R_WAIT + 2 <= R_CROSS && R_LIMIT < ROOM_SIZE,
	"a partner's turn's fields overlap");

static struct {
	bool given[SIDES];                /* his folder given (a carried MegaMan's), else read from his side's GBA */
	uint16_t folder[SIDES][FOLDER];   /* the folder dealt (given, or read as the deck was dealt) */
	uint8_t regular[SIDES], tags[SIDES][2];
	uint32_t seed[SIDES][SIDES];      /* the seed each GBA's RNG gave, by the side dealt: what the output says */
	unsigned dealt[SIDES], said[SIDES], late[SIDES], set[SIDES], taken[SIDES], chips[SIDES], limits[SIDES];
	unsigned held[SIDES];             /* the most frames his side's GBA held for him in a turn */
	unsigned crosses[SIDES], pas[SIDES];   /* his Crosses and Program Advances said */
} C;

static uint32_t room(unsigned side) { return RAM_TEAM_CUSTOM + (side & 1u) * ROOM_SIZE; }

static uint8_t get8(struct mCore *c, uint32_t at) { return (uint8_t)c->rawRead8(c, at, -1); }
static uint16_t get16(struct mCore *c, uint32_t at) { return (uint16_t)c->rawRead16(c, at, -1); }
/* (from a hook: this core's own, worked out on each GBA from the same packets) */
static void put8(uint32_t at, uint8_t v) { hook_write8(at, v); }
static void put16(uint32_t at, uint16_t v) { hook_write16(at, v); }

void arena_team_custom_folder(unsigned side, const uint16_t *folder, uint8_t regular, const uint8_t tags[2]) {
	side &= 1u;
	C.given[side] = folder != NULL;
	if (!folder) return;
	memcpy(C.folder[side], folder, sizeof C.folder[side]);
	C.regular[side] = regular;
	C.tags[side][0] = tags ? tags[0] : NONE8;
	C.tags[side][1] = tags ? tags[1] : NONE8;
}

void arena_team_custom_begin(void) {
	for (uint32_t a = RAM_TEAM_CUSTOM; a < RAM_TEAM_CUSTOM_END; a += 4) hook_write32(a, 0);
}

static unsigned chip_id(uint16_t e) { return e & 0x1FFu; }

/* an entry a deck takes: a chip, and one BN6's battles hold */
static bool dealable(uint16_t e) { return e != NONE16 && chip_id(e) && !chips_sits_out((int)chip_id(e)); }

/* his side's lead's folder, Regular chip and TagChips, as side's GBA holds them */
static void lead_folder(struct mCore *c, unsigned side) {
	unsigned k = get8(c, BN6_NAVI_FOLDER);
	uint32_t at = (uint32_t)c->rawRead32(c, BN6_TOOLKIT + BN6_TOOLKIT_CHIPS, -1) + (k < 2 ? k : 0) * 2u * FOLDER;
	for (unsigned i = 0; i < FOLDER; ++i) C.folder[side][i] = get16(c, at + 2 * i);
	C.regular[side] = get8(c, k == 1 ? BN6_NAVI_FOLDER2_REG : BN6_NAVI_FOLDER1_REG);
	C.tags[side][0] = get8(c, k == 1 ? BN6_NAVI_FOLDER2_TAG : BN6_NAVI_FOLDER1_TAG);
	C.tags[side][1] = get8(c, k == 1 ? BN6_NAVI_FOLDER2_TAG2 : BN6_NAVI_FOLDER1_TAG2);
}

/* (each side's GBA's core, as the pair last handed it a frame: his lead's folder read from it as his deck is dealt, a
 * folder that stays as it is for the battle, so that a core dealing before or after its partner reads the same) */
static struct mCore *side_core[SIDES];

void arena_team_custom_sampled(struct mCore *core, unsigned side) { side_core[side & 1u] = core; }

static uint32_t xorshift(uint32_t *s) {
	*s ^= *s << 13;
	*s ^= *s >> 17;
	*s ^= *s << 5;
	return *s;
}

/* his deck as BN6 deals a side's (bn6f sub_800A3E4): the Regular chip
 * first, the TagChips after it where both are set, the rest shuffled; the
 * shuffle ours, seeded by his side's GBA's random number as the deck is
 * dealt (both GBAs keep it alike), so that every client and spectator
 * deals the same */
static void deal(unsigned side, bool second) {
	if (!C.given[side] && side_core[side]) lead_folder(side_core[side], side);
	/* (teampa=1: side 0's partner's folder opens with Cannon A, B and C as its Regular chip and TagChips, dealt first: a Program
	 * Advance in his first hand; over the network his own game gives his folder so instead, arena_seats_pa_folder) */
	if (dev.team_pa && side == 0 && !arena_seats_net()) {
		for (unsigned i = 0; i < 3; ++i) C.folder[side][i] = (uint16_t)(BN6_CHIP_CANNON | i << 9);
		C.regular[side] = 0;
		C.tags[side][0] = 1;
		C.tags[side][1] = 2;
	}
	/* (teamunite=2: each stood-in partner's folder opens with Atk+10 *, a Plus chip that lights NumberSoul: a UNITE of three
	 * turns where the Souls held have it) */
	if (dev.team_unite == 2 && !arena_seats_net()) {
		C.folder[side][0] = (uint16_t)(BN6_CHIP_ATK10 | 26u << 9);
		C.regular[side] = 0;
		C.tags[side][0] = C.tags[side][1] = NONE8;
	}
	uint8_t deck[FOLDER], n = 0, fixed = 0;
	bool used[FOLDER] = { false };
	uint8_t first[3] = { C.regular[side], NONE8, NONE8 };
	if (C.tags[side][0] < FOLDER && C.tags[side][1] < FOLDER) { first[1] = C.tags[side][0]; first[2] = C.tags[side][1]; }
	for (unsigned f = 0; f < 3; ++f)
		if (first[f] < FOLDER && !used[first[f]] && dealable(C.folder[side][first[f]])) {
			used[first[f]] = true;
			deck[n++] = first[f];
		}
	fixed = n;
	for (uint8_t i = 0; i < FOLDER; ++i)
		if (!used[i] && dealable(C.folder[side][i])) deck[n++] = i;
	uint32_t s = C.seed[side][second] = hook_read32(BN6_RNG_PRIMARY);
	s ^= 0x9E3779B9u * (side + 1);
	if (!s) s = 1;
	for (unsigned i = n; i > (unsigned)fixed + 1; --i) {
		unsigned j = fixed + xorshift(&s) % (i - fixed);
		uint8_t t = deck[i - 1];
		deck[i - 1] = deck[j];
		deck[j] = t;
	}
	for (unsigned i = 0; i < FOLDER; ++i) put8(room(side) + R_DECK + i, i < n ? deck[i] : NONE8);
	put8(room(side) + R_DEALT, 1);
	++C.dealt[side];
}

static unsigned deck_left(struct mCore *c, unsigned side) {
	unsigned n = 0;
	while (n < FOLDER && get8(c, room(side) + R_DECK + n) != NONE8) ++n;
	return n;
}

static unsigned deck_here(unsigned side) {
	unsigned n = 0;
	while (n < FOLDER && hook_read8(room(side) + R_DECK + n) != NONE8) ++n;
	return n;
}

unsigned arena_team_custom_deck(struct mCore *c, unsigned side, uint16_t out[30]) {
	side &= 1u;
	if (!c || !get8(c, room(side) + R_DEALT)) return 0;
	unsigned n = deck_left(c, side);
	for (unsigned i = 0; i < n; ++i) out[i] = C.folder[side][get8(c, room(side) + R_DECK + i) % FOLDER];
	return n;
}

/* his side's screen closed and his picks in: set for the battle loop's
 * next iteration, taken out of his deck */
static void commit(unsigned side) {
	uint32_t r = room(side);
	unsigned n = hook_read8(r + R_PICKED), left = deck_here(side), k = 0, d = 0;
	if (n > TEAM_CUSTOM_PICKS) n = TEAM_CUSTOM_PICKS;
	uint8_t deck[FOLDER];
	bool gone[FOLDER] = { false };
	for (unsigned i = 0; i < FOLDER; ++i) deck[i] = hook_read8(r + R_DECK + i);
	for (unsigned i = 0; i < n; ++i) {
		uint8_t slot = hook_read8(r + R_PICKS + i);
		if (slot >= left || gone[slot]) continue;
		gone[slot] = true;
		put8(r + R_TAKE + k++, deck[slot]);
	}
	for (unsigned i = 0; i < FOLDER; ++i)
		if (!gone[i]) put8(r + R_DECK + d++, deck[i]);
	while (d < FOLDER) put8(r + R_DECK + d++, NONE8);
	uint32_t at = hook_read32(RAM_TEAM_TICK) + 1;
	put8(r + R_TAKEN, (uint8_t)k);
	put16(r + R_TAKE_AT, (uint16_t)(at ? at : 1));
	put8(r + R_TAKE_PA, hook_read8(r + R_PA));
	put8(r + R_TAKE_PA_ID, hook_read8(r + R_PA_ID));
	put8(r + R_STATE, ST_CLOSED);
	++C.set[side];
	C.chips[side] += k;
}

/* a pick message of his: the pick kept; the last of them puts his picks
 * in (set at once where his side's screen has closed) */
static void pick_said(unsigned side, uint16_t value, bool second) {
	uint32_t r = room(side);
	unsigned count = value >> 11 & 7u, index = value >> 8 & 7u, slot = value & 0xFFu;
	uint8_t state = hook_read8(r + R_STATE);
	/* (his Cross, said before his picks while his turn is open: the form his own CROSSSELECT committed) */
	if (count == TEAM_CROSS_COUNT) {
		/* (the Soul his UNITE committed, said after it: into his own Souls block, arena_team_soul.h) */
		if (index && state == ST_OPEN) team_soul_said(side, index, slot, second);
		else if (!index && state == ST_OPEN && slot && slot < BN6_FORMS) {
			put8(r + R_CROSS, (uint8_t)slot);
			++C.crosses[side];
		}
		return;
	}
	/* (a Program Advance his screen made: its first pick, how many it took and its chip, said before his picks) */
	if (count == TEAM_PA_COUNT) {
		unsigned took = 3u + (slot >> 5 & 1u), id = BN6_PA_FIRST + (slot & 0x1Fu);
		if (state == ST_OPEN && index < TEAM_CUSTOM_PICKS && id <= BN6_PA_LAST) {
			put8(r + R_PA, (uint8_t)((index + 1) | (took - 3) << 4));
			put8(r + R_PA_ID, (uint8_t)(id - BN6_PA_FIRST));
			++C.pas[side];
		}
		return;
	}
	if (count > TEAM_CUSTOM_PICKS || (count && index >= count) || (state != ST_OPEN && state != ST_WAITING)) return;
	if (count) put8(r + R_PICKS + index, (uint8_t)slot);
	if (count && index + 1 < count) return;
	put8(r + R_PICKED, (uint8_t)count);
	++C.said[side];
	if (state == ST_WAITING) {
		++C.late[side];
		commit(side);
	} else put8(r + R_STATE, ST_IN);
}

/* a turn begun: picks that never came for the last one dropped */
static void turn_open(unsigned side, uint32_t r) {
	team_soul_turn_open(side);
	put8(r + R_STATE, ST_OPEN);
	put8(r + R_PICKED, 0);
	put8(r + R_OWED, 0);
	put16(r + R_WAIT, 0);
	put8(r + R_CROSS, 0);
	put8(r + R_PA, 0);
	put8(r + R_LIMIT, 0);
}

/* (his side's GBA holding for him, its own core alone: its frames counted; at TEAM_WAIT_LIMIT it says his picks none in his
 * stream's place, which comes to both GBAs in its packet as his own would) */
static void held_frame(unsigned side, uint32_t r) {
	unsigned held = hook_read16(r + R_WAIT) + 1u;
	put16(r + R_WAIT, (uint16_t)held);
	if (held > C.held[side]) C.held[side] = held;
	if (held < TEAM_WAIT_LIMIT || hook_read8(r + R_LIMIT)) return;
	put8(r + R_LIMIT, 1);
	team_link_say(side, TEAM_PICK(0, 0, 0));
	++C.limits[side];
	printf("arena2v2: side %u's GBA held %u frames for its partner, the wait's limit: his picks none, said in his place\n", side,
		held);
}

uint16_t arena_team_custom_link(unsigned side, uint16_t word, bool second) {
	side &= 1u;
	uint32_t r = room(side);
	bool custom = hook_read8(BN6_BATTLE_PHASE) == BN6_PHASE_CUSTOM, said = (word & TEAM_PICK_MARK) != 0;
	if (custom && !hook_read8(r + R_DEALT)) deal(side, second);
	uint8_t state = hook_read8(r + R_STATE);
	if (custom && (state == ST_CLOSED || state == ST_WAITING)) turn_open(side, r);
	if (said) pick_said(side, word, second);
	state = hook_read8(r + R_STATE);
	if (custom && state == ST_OPEN && hook_read8(r + R_OWED)) held_frame(side, r);
	if (!custom && state == ST_IN) commit(side);
	else if (!custom && state == ST_OPEN) put8(r + R_STATE, ST_WAITING);
	return custom || said ? 0 : (uint16_t)(word & ARENA_KEYS_MASK);
}

/* side's partner's picks into his hand at `h`, as BN6 writes a side's
 * from its Custom screen (bn6f sub_8029110): the chips' ids, powers and
 * entries, a plus chip's power added to the chip before it (sub_8029224:
 * Atk+10 and Atk+30 to an attack, Navi+20 to a Navi chip) */
/* a chip's power as its record gives it (a computed one 0) */
static uint16_t power_of(unsigned id) {
	const uint8_t *rec = chips_record((int)id), *at = rec ? rec + BN6_CHIP_ATTACK_POWER : NULL;
	unsigned p = at ? (unsigned)(at[0] | at[1] << 8) : 0;
	return (uint16_t)(p >= POWER_COMPUTED ? 0 : p);
}

unsigned team_hand_build(const uint16_t *entries, unsigned n, unsigned pa_at, unsigned pa_took, unsigned pa_id, TeamHand *out) {
	uint16_t ids[TEAM_CUSTOM_PICKS], ents[TEAM_CUSTOM_PICKS];
	unsigned m = 0, k = 0;
	for (unsigned i = 0; i < n && i < TEAM_CUSTOM_PICKS; ++i) {
		ids[m] = (uint16_t)chip_id(entries[i]);
		ents[m++] = entries[i];
	}
	/* (BN6's OK joins a Program Advance's picks into its chip before the plus chips merge, sub_802B6F2: the picks after it on) */
	bool pa = pa_at && pa_at - 1 + pa_took <= m && pa_took >= 2;
	if (pa) {
		unsigned at = pa_at - 1;
		ids[at] = (uint16_t)pa_id;
		for (unsigned i = at + pa_took; i < m; ++i) ids[i - pa_took + 1] = ids[i];
		m -= pa_took - 1;
	}
	for (unsigned i = 0; i < m; ++i) {
		const uint8_t *before = k ? chips_record(out->id[k - 1]) : NULL;
		unsigned plus = ids[i] == BN6_CHIP_ATK10 || ids[i] == BN6_CHIP_ATK30 ? BN6_CHIP_SHOWS_POWER :
			ids[i] == BN6_CHIP_NAVI20 ? BN6_CHIP_NAVI_FLAG : 0;
		if (plus && before && (before[BN6_CHIP_EFFECT_FLAGS] & plus)) {
			out->bonus[k - 1] = (uint16_t)(out->bonus[k - 1] + power_of(ids[i]));
			continue;
		}
		out->id[k] = ids[i];
		out->power[k] = power_of(ids[i]);
		out->bonus[k] = 0;
		out->entry[k] = ents[i];
		++k;
	}
	for (unsigned i = k; i < BN6_HAND_MAX; ++i) out->entry[i] = NONE16;
	/* (with a Program Advance BN6 leaves the entries as the picks were: the hand it builds keeps them so) */
	if (pa)
		for (unsigned i = 0; i < BN6_HAND_MAX; ++i) out->entry[i] = i < n && i < TEAM_CUSTOM_PICKS ? entries[i] : NONE16;
	return k;
}

static void write_hand(unsigned side, uint32_t h, bool second) {
	uint32_t r = room(side);
	unsigned n = hook_read8(r + R_TAKEN), took = 0;
	uint16_t entries[TEAM_CUSTOM_PICKS];
	for (unsigned i = 0; i < n && i < TEAM_CUSTOM_PICKS; ++i) {
		uint8_t f = hook_read8(r + R_TAKE + i);
		if (f < FOLDER) entries[took++] = C.folder[side][f];
	}
	/* (teampa=-1: his Program Advance left out on the second GBA alone, a mutation that must part them) */
	uint8_t pa = second && dev.team_pa < 0 ? 0 : hook_read8(r + R_TAKE_PA);
	TeamHand hand;
	unsigned k = team_hand_build(entries, took, pa & 0xFu, 3u + (pa >> 4 & 1u), BN6_PA_FIRST + hook_read8(r + R_TAKE_PA_ID), &hand);
	const uint16_t *id = hand.id, *power = hand.power, *bonus = hand.bonus, *entry = hand.entry;
	if (pa & 0xFu)
		printf("arena2v2: core %d: side %u's partner's hand a Program Advance, %03X (power %u) first of %u chips, from %u picks\n",
			hook_slot(), side, id[0], power[0], k, took);
	/* (teampick=-1: a chip short on the second GBA alone, the mutation) */
	if (second && dev.team_pick < 0 && k) --k;
	uint8_t screen = (uint8_t)(hook_read8(BN6_BATTLE_SCREENS) - 1);
	hook_write8(h + BN6_HAND_AT, k ? 0 : NONE8);
	for (unsigned i = 0; i < BN6_HAND_MAX; ++i) {
		hook_write16(h + BN6_HAND_CHIPS + 2 * i, i < k ? id[i] : NONE16);
		hook_write16(h + BN6_HAND_POWERS + 2 * i, i < k ? power[i] : 0);
		hook_write16(h + BN6_HAND_BONUS + 2 * i, i < k ? bonus[i] : 0);
		hook_write16(h + BN6_HAND_EXTRA + 2 * i, 0);
		hook_write16(h + BN6_HAND_ENTRIES + 2 * i, entry[i]);
		hook_write8(h + BN6_HAND_SCREEN + i, screen);
		hook_write8(h + BN6_HAND_MARKS + i, 0);
	}
}

void arena_team_custom_take(uint32_t tick, const uint32_t hand[2], bool second) {
	for (unsigned side = 0; side < SIDES; ++side) {
		uint32_t r = room(side);
		uint16_t at = hook_read16(r + R_TAKE_AT);
		if (!at || at != (uint16_t)tick || !hand[side]) continue;
		write_hand(side, hand[side], second);
		hook_write16(r + R_TAKE_AT, 0);
		++C.taken[side];
	}
}

/* side's partner on the field: his object there, HP left (from a hook) */
static bool standing(unsigned side) {
	uint32_t o = hook_read32(BN6_BATTLE_PLAYERS + side * BN6_BATTLE_PLAYER_STEP + BN6_SIDE_PARTNER);
	return o >= BN6_EWRAM && o < BN6_EWRAM_END && hook_read16(o + BN6_T1_HP);
}

bool arena_team_custom_holds(unsigned side) {
	side &= 1u;
	uint32_t r = room(side);
	return hook_read8(r + R_DEALT) && hook_read8(r + R_STATE) == ST_OPEN && standing(side);
}

uint8_t arena_team_custom_cross(unsigned side) { return hook_read8(room(side & 1u) + R_CROSS); }
void arena_team_custom_cross_taken(unsigned side) { hook_write8(room(side & 1u) + R_CROSS, 0); }

void arena_team_custom_owe(unsigned side, bool owed) { hook_write8(room(side & 1u) + R_OWED, owed); }
bool arena_team_custom_owed(unsigned side) { return hook_read8(room(side & 1u) + R_OWED) != 0; }

unsigned arena_team_custom_waited(struct mCore *c, unsigned side) { return c ? get16(c, room(side & 1u) + R_WAIT) : 0; }
bool arena_team_custom_lead_waits(struct mCore *c, unsigned side) { return c && get8(c, room(side & 1u) + R_OWED); }
bool arena_team_custom_picking(struct mCore *c, unsigned side) {
	return c && get8(c, room(side & 1u) + R_DEALT) && get8(c, room(side & 1u) + R_STATE) == ST_OPEN;
}

uint64_t arena_team_custom_hash(uint64_t h) {
	for (unsigned side = 0; side < SIDES; ++side) {
		for (unsigned i = 0; i < FOLDER; ++i) h = (h ^ C.folder[side][i]) * UINT64_C(1099511628211);
		h = (h ^ (C.regular[side] | (uint32_t)C.tags[side][0] << 8 | (uint32_t)C.tags[side][1] << 16)) * UINT64_C(1099511628211);
	}
	return h;
}

void arena_team_custom_report(void) {
	for (unsigned side = 0; side < SIDES; ++side)
		printf("arena2v2: side %u's partner's own deck dealt %u times (%s folder; the seed %08X on the first GBA, %08X on the "
			"second), his picks said %u times in his stream (%u after his side's Custom screen closed), set %u times (%u chips) "
			"and taken into his hand %u times, every core's; his side's GBA held for him %u frames at most in a turn, %u times "
			"to the limit; his Cross said %u times, a Program Advance %u\n", side, C.dealt[side], C.given[side] ? "his own" : "his lead's", (unsigned)C.seed[side][0],
			(unsigned)C.seed[side][1], C.said[side], C.late[side], C.set[side], C.chips[side], C.taken[side], C.held[side],
			C.limits[side], C.crosses[side], C.pas[side]);
}
