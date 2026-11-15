/* dive_splits.h */
#include "dive_splits.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "director.h"
#include "dive_link.h"
#include "dive_words.h"
#include "emu.h"
#include "online_transport.h"
#include "run.h"

#define FRAMES_PER_TENTH 6   /* the GBA's 60 frames a second */
#define GUARDIAN_INDEX 255   /* the guardian's battle's index on its layer */

static struct {
	uint32_t seed;
	int depth;
	int battles;             /* fought on this layer so far */
	bool fighting;
	bool guardian;
	uint32_t frames;         /* the battle's DeleteTime so far */
	uint8_t level;           /* its Busting Level as last read */
	uint16_t cause;          /* its strongest foe's NameID */
} S;

/* the battle's strongest foe in play: a Navi's NameID before a virus's */
static uint16_t strongest(void) {
	uint16_t best = 0;
	for (uint32_t k = 0; k < BN6_T1_COUNT; ++k) {
		uint32_t o = BN6_T1_OBJECTS + k * BN6_T1_SIZE;
		if (!(emu_read8(o + BN6_T1_IN_PLAY) & 1) || emu_read8(o + BN6_T1_ALLIANCE) != 1) continue;
		uint16_t id = emu_read16(o + BN6_T1_NAME_ID);
		if (id > best) best = id;
	}
	return best;
}

static void watch(void) {
	if (!S.fighting) {
		S.fighting = true;
		S.guardian = director_guardian_battle() != 0;
		S.frames = 0;
		S.level = 0;
		S.cause = 0;
	}
	uint32_t t = emu_read32(BN6_BATTLE_TIMER);
	if (t > S.frames) S.frames = t;
	uint8_t level = emu_read8(BN6_BATTLE_BUSTING);
	if (level) S.level = level;
	uint16_t c = strongest();
	if (c) S.cause = c;
}

static void ended(void) {
	S.fighting = false;
	/* (the guardian's battle one index for every diver, past the random ones: each diver meets him after his own count of them) */
	int index = S.guardian ? GUARDIAN_INDEX : S.battles++;
	if (emu_read8(BN6_BATTLE_RESULT) != 1 || (!S.guardian && index >= GUARDIAN_INDEX)) return;
	DiveSplit s;
	memset(&s, 0, sizeof s);
	s.index = (uint8_t)index;
	s.party = 1;
	unsigned tenths = S.frames / FRAMES_PER_TENTH;
	s.tenths = (uint16_t)(tenths > 0xFFFF ? 0xFFFF : tenths ? tenths : 1);
	s.level = S.level;
	s.guardian = S.guardian;
	uint8_t m[ONLINE_DIVE_SPLIT_BYTES];
	size_t n = dive_wire_split(m, &s);
	if (n && online_transport_send(m, n))
		fprintf(stderr, "dive: split of battle %d: %u.%u s, busting %u%s\n", index, s.tenths / 10, s.tenths % 10, s.level, s.guardian ? ", the guardian's" : "");
}

void dive_splits_frame(void) {
	if (!director_on_layer()) return;
	if (S.seed != run.layer_seed || S.depth != run.depth) {
		S.seed = run.layer_seed;
		S.depth = run.depth;
		S.battles = 0;
		S.fighting = false;
	}
	int sub = emu_read8(BN6_GAMESTATE);
	if (sub == BN6_SUB_BATTLE) watch();
	else if (S.fighting && sub == BN6_SUB_MAP) ended();
}

void dive_splits_place(const DivePlace *p) {
	fprintf(stderr, "dive: battle %u's split: %u of %u, the quickest %u by %s\n", p->index, p->place, p->count, p->best, p->best_name);
	if (p->place) dive_link_line(dive_place_words(p->tenths, p->place, p->count));
}

unsigned dive_splits_cause(void) { return S.cause; }
