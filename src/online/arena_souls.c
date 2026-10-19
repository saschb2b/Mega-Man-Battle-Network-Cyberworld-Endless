/* The Souls across the Arena's cable (arena_souls.h). */
#include "arena_souls.h"

#include <stdio.h>

#include <mgba/core/core.h>

#include "bn5.h"
#include "bn6.h"
#include "bn6_navis.h"
#include "soulchaos.h"

/* a side's field of the Souls' block (bn6_forms.h) */
static uint32_t field(unsigned side, uint32_t f) { return BN6_SOULS + side * BN6_SOULS_SIDE + f; }

/* ColonelSoul's button: the chip his player's OK kept in it (BN6_SOUL_NEXT_CHIP, soulcolonel.c's, written on his own
 * GBA alone, 0 none) into the other GBA's block wherever it differs, before the turn's start makes it his charged shot
 * there too */
static void carry_chip(struct mCore *from, struct mCore *to, unsigned side) {
	uint16_t chip = (uint16_t)from->rawRead16(from, field(side, BN6_SOUL_NEXT_CHIP), -1);
	if (chip == (uint16_t)to->rawRead16(to, field(side, BN6_SOUL_NEXT_CHIP), -1)) return;
	to->rawWrite16(to, field(side, BN6_SOUL_NEXT_CHIP), -1, chip);
	printf("arena: side %u's ColonelSoul chip %u carried to the other GBA\n", side, chip & 0x1FFu);
}

void arena_souls_carry(struct mCore *core[2], ArenaSoulCarried carried[2]) {
	for (unsigned k = 0; k < 2; ++k) {
		struct mCore *from = core[k], *to = core[k ^ 1];
		/* (his side the cable's: the core's MegaMan's, BN6_BATTLE_SIDE) */
		unsigned side = (unsigned)from->rawRead8(from, BN6_BATTLE_SIDE, -1) & 1u;
		carry_chip(from, to, side);
		uint8_t next = (uint8_t)from->rawRead8(from, field(side, BN6_SOUL_NEXT), -1);
		uint32_t used = from->rawRead32(from, field(side, BN6_SOUL_USED), -1);
		if (!next || (next == carried[k].next && used == carried[k].used)) continue;
		static const uint32_t bytes[] = { BN6_SOUL_NEXT, BN6_SOUL_NEXT_CHAOS, BN6_SOUL_NEXT_TURNS };
		for (unsigned i = 0; i < sizeof bytes / sizeof *bytes; ++i)
			to->rawWrite8(to, field(side, bytes[i]), -1, (uint8_t)from->rawRead8(from, field(side, bytes[i]), -1));
		to->rawWrite16(to, field(side, BN6_SOUL_NEXT_CHIP), -1, (uint16_t)from->rawRead16(from, field(side, BN6_SOUL_NEXT_CHIP), -1));
		to->rawWrite32(to, field(side, BN6_SOUL_USED), -1, used);
		carried[k] = (ArenaSoulCarried){ next, used };
		printf("arena: side %u's Soul %u (its turns %u) carried to the other GBA\n", side, next,
			(unsigned)from->rawRead8(from, field(side, BN6_SOUL_NEXT_TURNS), -1));
	}
}

void arena_souls_lists(struct mCore *core[2]) {
	for (unsigned k = 0; k < 2; ++k) {
		struct mCore *own = core[k];
		unsigned side = (unsigned)own->rawRead8(own, BN6_BATTLE_SIDE, -1) & 1u;
		uint32_t at = own->rawRead32(own, BN6_TOOLKIT + BN6_TOOLKIT_CHIPS, -1) +
			2u * BN6_FOLDER_ENTRIES * (uint32_t)own->rawRead8(own, BN6_NAVI_FOLDER, -1);
		uint16_t folder[BN6_FOLDER_ENTRIES], chips[SOULCHAOS_LIST_CHIPS];
		for (uint32_t i = 0; i < BN6_FOLDER_ENTRIES; ++i) folder[i] = (uint16_t)own->rawRead16(own, at + 2 * i, -1);
		int n = soulchaos_list_make(folder, chips);
		uint32_t list = BN6_DARK_LISTS + side * BN5_DARK_LIST_SIZE;
		for (unsigned c = 0; c < 2; ++c) {
			for (uint32_t i = 0; i < SOULCHAOS_LIST_CHIPS; ++i) core[c]->rawWrite16(core[c], list + 2 * i, -1, chips[i]);
			core[c]->rawWrite32(core[c], list + BN5_DARK_LIST_COUNT, -1, (uint32_t)n);
		}
		printf("arena: side %u's chips for a Dark Soul, %d of its folder's, on both GBAs\n", side, n);
	}
}
