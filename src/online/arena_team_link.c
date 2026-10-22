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

#define SIDES     2
/* A side's room in RAM_TEAM_LINK, each core's own (our layout, not the game's): */
#define LINK_SIZE 0x20
#define L_OUTBOX  0x00   /* his messages waiting for his side's GBA's packets, OUTBOX u16 from L_HEAD on */
#define OUTBOX    8
#define L_HEAD    0x10   /* u8 the outbox's first, */
#define L_COUNT   0x11   /* ... and how many */
#define L_KEYS    0x12   /* u16 his keys for his side's GBA's next packet: the last the pair handed it */
#define L_IN      0x14   /* u16 his keys as the last packet of his side brought them */
#define L_WORD    0x16   /* u16 the word that packet carried */

_Static_assert(RAM_TEAM_LINK + SIDES * LINK_SIZE <= RAM_TEAM_LINK_END, "the partners' words past their room");
_Static_assert(L_OUTBOX + 2 * OUTBOX <= L_HEAD && L_WORD + 2 <= LINK_SIZE, "a partner's words' fields overlap");

/* every core's, a rollback's frames run again among them */
static struct {
	unsigned put[SIDES], said[SIDES], full[SIDES], sent[SIDES], read[SIDES][SIDES], messages[SIDES][SIDES];
	unsigned stores[2];
} L;

static uint32_t room(unsigned side) { return RAM_TEAM_LINK + (side & 1u) * LINK_SIZE; }

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
	unsigned head = (uint8_t)core->rawRead8(core, r + L_HEAD, -1), n = (uint8_t)core->rawRead8(core, r + L_COUNT, -1);
	if (n >= OUTBOX) {
		++L.full[side & 1u];
		return;
	}
	core->rawWrite16(core, r + L_OUTBOX + 2 * ((head + n) % OUTBOX), -1, value);
	core->rawWrite8(core, r + L_COUNT, -1, (uint8_t)(n + 1));
}

void team_link_say(unsigned side, uint16_t message) {
	uint32_t r = room(side);
	unsigned head = hook_read8(r + L_HEAD), n = hook_read8(r + L_COUNT);
	if (n >= OUTBOX) return;
	hook_write16(r + L_OUTBOX + 2 * ((head + n) % OUTBOX), message);
	hook_write8(r + L_COUNT, (uint8_t)(n + 1));
	++L.said[side & 1u];
}

uint16_t team_link_keys(unsigned side) { return hook_read16(room(side) + L_IN); }

/* this GBA's partner's word for the packet it builds: the outbox's first message, else his keys */
static uint16_t next_word(unsigned side) {
	uint32_t r = room(side);
	unsigned head = hook_read8(r + L_HEAD), n = hook_read8(r + L_COUNT);
	if (!n) return hook_read16(r + L_KEYS);
	uint16_t word = hook_read16(r + L_OUTBOX + 2 * head);
	hook_write8(r + L_HEAD, (uint8_t)((head + 1) % OUTBOX));
	hook_write8(r + L_COUNT, (uint8_t)(n - 1));
	++L.sent[side];
	return word;
}

/* (the pad's store into BN6_LINK_OUT, both sides' packets of the frame read before it) each side's partner's word taken
 * from his side's packet, as this GBA's battle took his lead's pad from it; then this GBA's own partner's word set beside
 * its pad (`fresh`: BN6 read the packets this frame) */
static void at_store(bool fresh, bool second) {
	for (unsigned side = 0; fresh && side < SIDES; ++side) {
		uint32_t in = BN6_LINK_IN + side * BN6_LINK_PACKET;
		/* (a packet of BN6's: its pad's mark set, as BN6 asks before it takes the pad) */
		if (!(hook_read16(in + BN6_LINK_PAD) & BN6_KEYS_VALID)) continue;
		uint16_t word = (uint16_t)(hook_read8(in + BN6_LINK_SPARE_LO) | hook_read8(in + BN6_LINK_SPARE_HI) << 8);
		++L.read[second][side];
		L.messages[second][side] += (word & TEAM_PICK_MARK) != 0;
		hook_write16(room(side) + L_WORD, word);
		hook_write16(room(side) + L_IN, arena_team_custom_link(side, word, second));
	}
	unsigned own = hook_read8(BN6_BATTLE_SIDE) & 1u;
	uint16_t word = next_word(own);
	hook_write8(BN6_LINK_OUT + BN6_LINK_SPARE_LO, (uint8_t)word);
	hook_write8(BN6_LINK_OUT + BN6_LINK_SPARE_HI, (uint8_t)(word >> 8));
	++L.stores[second];
}

static HookAct store(HookRegs *r, void *user) {
	(void)r;
	at_store(true, user != NULL);
	return HOOK_CONTINUE;
}

/* (the same store in sub_801FFD6, its argument at [sp,#4]: bit 3 set where it read no packets this frame) */
static HookAct store_b(HookRegs *r, void *user) {
	at_store(!(hook_read32(r->sp + 4) & 8u), user != NULL);
	return HOOK_CONTINUE;
}

bool team_link_hooks(struct HookCore *core, bool second) {
	void *user = second ? (void *)&L : NULL;
	return hook_on(core, BN6_LINK_PAD_STORE, store, user) && hook_on(core, BN6_LINK_PAD_STORE_B, store_b, user);
}

void team_link_report(void) {
	printf("arena2v2: the partners' words in BN6's packets: handed to their sides' GBAs %u and %u times (%u and %u messages "
		"past a full outbox), %u packets built on the first GBA and %u on the second, %u and %u messages sent; read on the "
		"first GBA %u and %u times (%u and %u messages), on the second %u and %u (%u and %u); the wait's limit said %u and %u "
		"times, every core's\n", L.put[0], L.put[1], L.full[0], L.full[1], L.stores[0], L.stores[1], L.sent[0], L.sent[1],
		L.read[0][0], L.read[0][1], L.messages[0][0], L.messages[0][1], L.read[1][0], L.read[1][1], L.messages[1][0],
		L.messages[1][1], L.said[0], L.said[1]);
}
