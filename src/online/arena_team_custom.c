/* --dev arena2v2: a partner's own folder, deck and picks in the pair (arena_team_custom.h). */
#include "arena_team_custom.h"

#include <stdio.h>
#include <string.h>

#include <mgba/core/core.h>

#include "arena_input.h"
#include "bn6.h"
#include "chips.h"
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

/* his turn: no Custom screen; his side's GBA's open, his picks not in yet;
 * open with them in; closed before they came */
enum { ST_CLOSED, ST_OPEN, ST_IN, ST_WAITING };

_Static_assert(RAM_TEAM_CUSTOM + SIDES * ROOM_SIZE <= RAM_TEAM_CUSTOM_END, "the partners' decks and turns past their room");
_Static_assert(R_PICKS + TEAM_CUSTOM_PICKS <= R_TAKE_AT && R_TAKE + TEAM_CUSTOM_PICKS <= R_DEALT && R_WAIT + 2 <= R_CROSS && R_CROSS < ROOM_SIZE,
	"a partner's turn's fields overlap");

static struct {
	bool given[SIDES];
	uint16_t folder[SIDES][FOLDER];   /* the folder dealt (given, or read as the deck was dealt) */
	uint8_t regular[SIDES], tags[SIDES][2];
	uint32_t seed[SIDES][SIDES];      /* the seed each GBA's RNG gave, by the side dealt: what the output says */
	unsigned dealt[SIDES], said[SIDES], late[SIDES], set[SIDES], taken[SIDES], chips[SIDES], limits[SIDES];
	unsigned held[SIDES];             /* the most frames his side's GBA held for him in a turn */
	unsigned crosses[SIDES];          /* his Crosses said */
} C;

static uint32_t room(unsigned side) { return RAM_TEAM_CUSTOM + (side & 1u) * ROOM_SIZE; }

static uint8_t get8(struct mCore *c, uint32_t at) { return (uint8_t)c->rawRead8(c, at, -1); }
static uint16_t get16(struct mCore *c, uint32_t at) { return (uint16_t)c->rawRead16(c, at, -1); }
/* (both cores alike: what a partner's turn keeps is the same in both GBAs' RAM) */
static void put8(struct mCore *core[2], uint32_t at, uint8_t v) {
	for (unsigned g = 0; g < SIDES; ++g) core[g]->rawWrite8(core[g], at, -1, v);
}
static void put16(struct mCore *core[2], uint32_t at, uint16_t v) {
	for (unsigned g = 0; g < SIDES; ++g) core[g]->rawWrite16(core[g], at, -1, v);
}

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
static void deal(struct mCore *core[2], unsigned side) {
	struct mCore *own = core[side];
	if (!C.given[side]) lead_folder(own, side);
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
	for (unsigned g = 0; g < SIDES; ++g) C.seed[side][g] = (uint32_t)core[g]->rawRead32(core[g], BN6_RNG_PRIMARY, -1);
	uint32_t s = C.seed[side][side] ^ (0x9E3779B9u * (side + 1));
	if (!s) s = 1;
	for (unsigned i = n; i > (unsigned)fixed + 1; --i) {
		unsigned j = fixed + xorshift(&s) % (i - fixed);
		uint8_t t = deck[i - 1];
		deck[i - 1] = deck[j];
		deck[j] = t;
	}
	for (unsigned i = 0; i < FOLDER; ++i) put8(core, room(side) + R_DECK + i, i < n ? deck[i] : NONE8);
	put8(core, room(side) + R_DEALT, 1);
	++C.dealt[side];
}

static unsigned deck_left(struct mCore *c, unsigned side) {
	unsigned n = 0;
	while (n < FOLDER && get8(c, room(side) + R_DECK + n) != NONE8) ++n;
	return n;
}

unsigned arena_team_custom_deck(struct mCore *c, unsigned side, uint16_t out[30]) {
	side &= 1u;
	if (!c || !get8(c, room(side) + R_DEALT)) return 0;
	unsigned n = deck_left(c, side);
	for (unsigned i = 0; i < n; ++i) out[i] = C.folder[side][get8(c, room(side) + R_DECK + i) % FOLDER];
	return n;
}

/* his side's GBA's screen closed and his picks in: set for its iteration
 * plus the delay, taken out of his deck */
static void commit(struct mCore *core[2], unsigned side, uint32_t delay) {
	struct mCore *own = core[side];
	uint32_t r = room(side);
	unsigned n = get8(own, r + R_PICKED), left = deck_left(own, side), k = 0, d = 0;
	if (n > TEAM_CUSTOM_PICKS) n = TEAM_CUSTOM_PICKS;
	uint8_t deck[FOLDER];
	bool gone[FOLDER] = { false };
	for (unsigned i = 0; i < FOLDER; ++i) deck[i] = get8(own, r + R_DECK + i);
	for (unsigned i = 0; i < n; ++i) {
		uint8_t slot = get8(own, r + R_PICKS + i);
		if (slot >= left || gone[slot]) continue;
		gone[slot] = true;
		put8(core, r + R_TAKE + k++, deck[slot]);
	}
	for (unsigned i = 0; i < FOLDER; ++i)
		if (!gone[i]) put8(core, r + R_DECK + d++, deck[i]);
	while (d < FOLDER) put8(core, r + R_DECK + d++, NONE8);
	uint32_t at = (uint32_t)own->rawRead32(own, RAM_TEAM_TICK, -1) + delay;
	put8(core, r + R_TAKEN, (uint8_t)k);
	put16(core, r + R_TAKE_AT, (uint16_t)(at ? at : 1));
	put8(core, r + R_STATE, ST_CLOSED);
	++C.set[side];
	C.chips[side] += k;
}

/* a pick message of his stream: the pick kept; the last of them puts his
 * picks in (set at once where his side's GBA's screen has closed) */
static void pick_said(struct mCore *core[2], unsigned side, uint16_t value, uint32_t delay) {
	uint32_t r = room(side);
	unsigned count = value >> 11 & 7u, index = value >> 8 & 7u, slot = value & 0xFFu;
	uint8_t state = get8(core[side], r + R_STATE);
	/* (his Cross, said before his picks while his turn is open: the form his own CROSSSELECT committed) */
	if (count == TEAM_CROSS_COUNT) {
		if (state == ST_OPEN && slot && slot < BN6_FORMS) {
			put8(core, r + R_CROSS, (uint8_t)slot);
			++C.crosses[side];
		}
		return;
	}
	if (count > TEAM_CUSTOM_PICKS || (count && index >= count) || (state != ST_OPEN && state != ST_WAITING)) return;
	if (count) put8(core, r + R_PICKS + index, (uint8_t)slot);
	if (count && index + 1 < count) return;
	put8(core, r + R_PICKED, (uint8_t)count);
	++C.said[side];
	if (state == ST_WAITING) {
		++C.late[side];
		commit(core, side, delay);
	} else put8(core, r + R_STATE, ST_IN);
}

uint16_t arena_team_custom_frame(struct mCore *core[2], unsigned side, uint16_t value, uint32_t delay) {
	side &= 1u;
	struct mCore *own = core[side];
	uint32_t r = room(side);
	bool custom = get8(own, BN6_BATTLE_PHASE) == BN6_PHASE_CUSTOM, said = (value & TEAM_PICK_MARK) != 0;
	if (custom && !get8(own, r + R_DEALT)) deal(core, side);
	uint8_t state = get8(own, r + R_STATE);
	/* (a turn begun: picks that never came for the last one dropped) */
	if (custom && (state == ST_CLOSED || state == ST_WAITING)) {
		put8(core, r + R_STATE, ST_OPEN);
		put8(core, r + R_PICKED, 0);
		put8(core, r + R_OWED, 0);
		put16(core, r + R_WAIT, 0);
		put8(core, r + R_CROSS, 0);
	}
	if (said) pick_said(core, side, value, delay);
	state = get8(own, r + R_STATE);
	/* (his side's GBA held for him: its frames counted; past TEAM_WAIT_LIMIT his picks are in as they stand, none if none
	 * came, and it goes on) */
	if (custom && state == ST_OPEN && get8(own, r + R_OWED)) {
		unsigned held = get16(own, r + R_WAIT) + 1u;
		put16(core, r + R_WAIT, (uint16_t)held);
		if (held > C.held[side]) C.held[side] = held;
		if (held >= TEAM_WAIT_LIMIT) {
			put8(core, r + R_PICKED, 0);
			put8(core, r + R_STATE, ST_IN);
			++C.limits[side];
			printf("arena2v2: side %u's GBA held %u frames for its partner, the wait's limit: his picks none, the screen goes on\n",
				side, held);
			state = ST_IN;
		}
	}
	if (!custom && state == ST_IN) commit(core, side, delay);
	else if (!custom && state == ST_OPEN) put8(core, r + R_STATE, ST_WAITING);
	return custom || said ? 0 : (uint16_t)(value & ARENA_KEYS_MASK);
}

/* side's partner's picks into his hand at `h`, as BN6 writes a side's
 * from its Custom screen (bn6f sub_8029110): the chips' ids, powers and
 * entries, a plus chip's power added to the chip before it (sub_8029224:
 * Atk+10 and Atk+30 to an attack, Navi+20 to a Navi chip) */
static void write_hand(unsigned side, uint32_t h, bool short_one) {
	uint32_t r = room(side);
	unsigned n = hook_read8(r + R_TAKEN), k = 0;
	uint16_t id[BN6_HAND_MAX], power[BN6_HAND_MAX], bonus[BN6_HAND_MAX], entry[BN6_HAND_MAX];
	for (unsigned i = 0; i < n && i < TEAM_CUSTOM_PICKS; ++i) {
		uint8_t f = hook_read8(r + R_TAKE + i);
		if (f >= FOLDER) continue;
		uint16_t e = C.folder[side][f];
		const uint8_t *rec = chips_record((int)chip_id(e)), *before = k ? chips_record(id[k - 1]) : NULL;
		const uint8_t *at = rec ? rec + BN6_CHIP_ATTACK_POWER : NULL;
		unsigned p = at ? (unsigned)(at[0] | at[1] << 8) : 0;
		if (p >= POWER_COMPUTED) p = 0;
		unsigned plus = chip_id(e) == BN6_CHIP_ATK10 || chip_id(e) == BN6_CHIP_ATK30 ? BN6_CHIP_SHOWS_POWER :
			chip_id(e) == BN6_CHIP_NAVI20 ? BN6_CHIP_NAVI_FLAG : 0;
		if (plus && before && (before[BN6_CHIP_EFFECT_FLAGS] & plus)) { bonus[k - 1] = (uint16_t)(bonus[k - 1] + p); continue; }
		id[k] = (uint16_t)chip_id(e);
		power[k] = (uint16_t)p;
		bonus[k] = 0;
		entry[k++] = e;
	}
	/* (teampick=-1: a chip short on the second GBA alone, the mutation) */
	if (short_one && k) --k;
	uint8_t screen = (uint8_t)(hook_read8(BN6_BATTLE_SCREENS) - 1);
	hook_write8(h + BN6_HAND_AT, k ? 0 : NONE8);
	for (unsigned i = 0; i < BN6_HAND_MAX; ++i) {
		hook_write16(h + BN6_HAND_CHIPS + 2 * i, i < k ? id[i] : NONE16);
		hook_write16(h + BN6_HAND_POWERS + 2 * i, i < k ? power[i] : 0);
		hook_write16(h + BN6_HAND_BONUS + 2 * i, i < k ? bonus[i] : 0);
		hook_write16(h + BN6_HAND_EXTRA + 2 * i, 0);
		hook_write16(h + BN6_HAND_ENTRIES + 2 * i, i < k ? entry[i] : NONE16);
		hook_write8(h + BN6_HAND_SCREEN + i, screen);
		hook_write8(h + BN6_HAND_MARKS + i, 0);
	}
}

void arena_team_custom_take(uint32_t tick, const uint32_t hand[2], bool short_one) {
	for (unsigned side = 0; side < SIDES; ++side) {
		uint32_t r = room(side);
		uint16_t at = hook_read16(r + R_TAKE_AT);
		if (!at || at != (uint16_t)tick || !hand[side]) continue;
		write_hand(side, hand[side], short_one);
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
			"to the limit; his Cross said %u times\n", side, C.dealt[side], C.given[side] ? "his own" : "his lead's", (unsigned)C.seed[side][0],
			(unsigned)C.seed[side][1], C.said[side], C.late[side], C.set[side], C.chips[side], C.taken[side], C.held[side],
			C.limits[side], C.crosses[side]);
}
