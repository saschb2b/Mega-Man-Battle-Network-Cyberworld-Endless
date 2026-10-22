/* Two MegaMen a side: the partners' words in BN6's packet (arena_team_link.h). */
#include "arena_team_link.h"

#include <stdio.h>

#include <mgba/core/core.h>

#include "arena_input.h"
#include "arena_team_custom.h"
#include "bn6.h"
#include "bn6_link.h"
#include "free_space.h"
#include "hook.h"
#include "team_outbox.h"

#define SIDES     2
/* A side's room in RAM_TEAM_LINK, each core's own (our layout, not the game's): his outbox first (team_outbox.h), then */
#define LINK_SIZE 0x20
#define L_KEYS    0x1A   /* u16 his keys for his side's GBA's next packet: the last the pair handed it */
#define L_IN      0x1C   /* u16 his keys as the last packet of his side brought them */
#define L_WORD    0x1E   /* u16 the word that packet carried */

_Static_assert(RAM_TEAM_LINK + SIDES * LINK_SIZE <= RAM_TEAM_LINK_END, "the partners' words past their room");
_Static_assert(TEAM_OUTBOX_SIZE <= L_KEYS && L_WORD + 2 <= LINK_SIZE, "a partner's words' fields overlap");

/* every core's, a rollback's frames run again among them */
static struct {
	unsigned put[SIDES], said[SIDES], full[SIDES], sent[SIDES], read[SIDES][SIDES], messages[SIDES][SIDES];
	unsigned stores[2], second_routine[2];
} L;

static uint32_t room(unsigned side) { return RAM_TEAM_LINK + (side & 1u) * LINK_SIZE; }

/* the outbox in a core's RAM: the pair's (between run loops, a core it names) and a hook's (the core it runs on) */
static uint8_t core_read8(void *c, uint32_t at) { return (uint8_t)((struct mCore *)c)->rawRead8(c, at, -1); }
static void core_write8(void *c, uint32_t at, uint8_t v) { ((struct mCore *)c)->rawWrite8(c, at, -1, v); }
static uint8_t here_read8(void *c, uint32_t at) { (void)c; return hook_read8(at); }
static void here_write8(void *c, uint32_t at, uint8_t v) { (void)c; hook_write8(at, v); }
static const TeamMem here = { here_read8, here_write8, NULL };

void team_link_begin(void) {
	for (uint32_t a = RAM_TEAM_LINK; a < RAM_TEAM_LINK_END; a += 4) hook_write32(a, 0);
}

void team_link_put(struct mCore *core, unsigned side, uint16_t value) {
	uint32_t r = room(side);
	++L.put[side & 1u];
	arena_team_custom_sampled(core, side);
	if (!(value & TEAM_PICK_MARK)) {
		core->rawWrite16(core, r + L_KEYS, -1, (uint16_t)(value & ARENA_KEYS_MASK));
		return;
	}
	const TeamMem m = { core_read8, core_write8, core };
	if (team_outbox_push(&m, r, value)) return;
	/* (refused, said: a turn's messages are nine at most, the outbox twelve; on every game alike, as the core's RAM is) */
	++L.full[side & 1u];
	printf("arena2v2: side %u's partner's outbox full (%u messages): his message %04X refused\n", side & 1u,
		team_outbox_count(&m, r), value);
}

bool team_link_say(unsigned side, uint16_t message) {
	if (!team_outbox_push(&here, room(side), message)) return false;
	++L.said[side & 1u];
	return true;
}

uint16_t team_link_keys(unsigned side) { return hook_read16(room(side) + L_IN); }

/* this GBA's partner's word for the packet it builds: the outbox's first message, else his keys */
static uint16_t next_word(unsigned side) {
	uint16_t word;
	if (!team_outbox_pop(&here, room(side), &word)) return hook_read16(room(side) + L_KEYS);
	++L.sent[side];
	return word;
}

/* (where BN6 has just read both sides' packets of the frame) each side's partner's word taken from his side's packet, as
 * this GBA's battle took his lead's pad from it: side 0's where its pad's mark is set, side 1's where it is or where the link
 * takes side 1's whatever its mark (BN6_LINK_TAKES_ANY 0, the routines' own test) */
static void take_words(bool second) {
	arena_team_custom_clock();
	for (unsigned side = 0; side < SIDES; ++side) {
		uint32_t in = BN6_LINK_IN + side * BN6_LINK_PACKET;
		bool marked = (hook_read16(in + BN6_LINK_PAD) & BN6_KEYS_VALID) != 0;
		if (!marked && (side == 0 || hook_read8(BN6_LINK_TAKES_ANY))) continue;
		uint16_t word = (uint16_t)(hook_read8(in + BN6_LINK_SPARE_LO) | hook_read8(in + BN6_LINK_SPARE_HI) << 8);
		++L.read[second][side];
		L.messages[second][side] += (word & TEAM_PICK_MARK) != 0;
		hook_write16(room(side) + L_WORD, word);
		hook_write16(room(side) + L_IN, arena_team_custom_link(side, word, second));
	}
}

/* (as BN6 stores this frame's pad into BN6_LINK_OUT) this GBA's own partner's word set beside it */
static void put_word(bool second) {
	unsigned own = hook_read8(BN6_BATTLE_SIDE) & 1u;
	uint16_t word = next_word(own);
	hook_write8(BN6_LINK_OUT + BN6_LINK_SPARE_LO, (uint8_t)word);
	hook_write8(BN6_LINK_OUT + BN6_LINK_SPARE_HI, (uint8_t)(word >> 8));
	++L.stores[second];
}

/* (sub_801FF18's pad store: it reads both packets and stores the pad together, every frame it runs) */
static HookAct store(HookRegs *r, void *user) {
	(void)r;
	take_words(user != NULL);
	put_word(user != NULL);
	return HOOK_CONTINUE;
}

/* (sub_801FFD6, the battle's other link routine, whose argument's bit 3 skips the read and bit 4 the store: its read
 * path's end, where side 0's packet has been read, and its store, each as BN6 runs it) */
static HookAct read_b(HookRegs *r, void *user) {
	(void)r;
	++L.second_routine[user != NULL];
	take_words(user != NULL);
	return HOOK_CONTINUE;
}

static HookAct store_b(HookRegs *r, void *user) {
	(void)r;
	put_word(user != NULL);
	return HOOK_CONTINUE;
}

bool team_link_hooks(struct HookCore *core, bool second) {
	void *user = second ? (void *)&L : NULL;
	return hook_on(core, BN6_LINK_PAD_STORE, store, user) && hook_on(core, BN6_LINK_READ_B, read_b, user) &&
		hook_on(core, BN6_LINK_PAD_STORE_B, store_b, user);
}

void team_link_report(void) {
	printf("arena2v2: the partners' words in BN6's packets: handed to their sides' GBAs %u and %u times (%u and %u messages "
		"refused at a full outbox), %u packets built on the first GBA and %u on the second, %u and %u messages sent; read on "
		"the first GBA %u and %u times (%u and %u messages), on the second %u and %u (%u and %u); the wait's limit said %u and "
		"%u times; the link's other routine read %u and %u times, every core's\n", L.put[0], L.put[1], L.full[0], L.full[1],
		L.stores[0], L.stores[1], L.sent[0], L.sent[1], L.read[0][0], L.read[0][1], L.messages[0][0], L.messages[0][1],
		L.read[1][0], L.read[1][1], L.messages[1][0], L.messages[1][1], L.said[0], L.said[1], L.second_routine[0],
		L.second_routine[1]);
}
