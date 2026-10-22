/* Two MegaMen a side: a partner's UNITE (arena_team_soul.h). */
#include "arena_team_soul.h"

#include <stdio.h>

#include "arena_team_navi.h"
#include "bn6.h"
#include "devtools.h"
#include "hook.h"

#define SIDES      2
#define SOULS      12    /* BN5's Souls 1-12 */
#define TURNS_SOUL 3     /* a Soul's turns before his NaviStats' extra (soulunite.c's, BN5's OK) */
#define TURNS_MAX  9

static unsigned said[SIDES], chips[SIDES];   /* every core's: what the output says */

static uint32_t field(unsigned side, uint32_t f) { return BN6_SOULS_PARTNERS + (side & 1u) * BN6_SOULS_SIDE + f; }

/* his Soul's turns as BN5's OK gives them (soulunite.c's commit): 1 for a Chaos Unison, else 3 and his own NaviStats' extra */
static uint8_t turns_of(unsigned side, unsigned chaos) {
	uint32_t ns = arena_team_navi_record_at(side);
	int turns = chaos ? 1 : TURNS_SOUL + (ns ? (int8_t)hook_read8(ns + BN6_NAVI_TURN_EXTRA) : 0);
	return (uint8_t)(turns > TURNS_MAX ? TURNS_MAX : turns < 1 ? 1 : turns);
}

void team_soul_said(unsigned side, unsigned index, unsigned slot, bool second) {
	side &= 1u;
	if (second && dev.team_unite < 0) return;
	if (index == 1) {
		unsigned soul = slot & 0xFu, chaos = slot >> 4 & 1u;
		if (!soul || soul > SOULS) return;
		hook_write8(field(side, BN6_SOUL_NEXT), (uint8_t)soul);
		hook_write8(field(side, BN6_SOUL_NEXT_CHAOS), (uint8_t)chaos);
		hook_write8(field(side, BN6_SOUL_NEXT_TURNS), turns_of(side, chaos));
		uint32_t used = field(side, BN6_SOUL_USED);
		hook_write32(used, hook_read32(used) | (1u << soul) << (chaos ? 16 : 0));
		++said[side];
	} else if (index == 2 || index == 3) {
		hook_write16(field(side, BN6_SOUL_NEXT_CHIP), (uint16_t)((index - 2) << 8 | (slot & 0xFFu)));
		++chips[side];
	}
}

void team_soul_turn_open(unsigned side) {
	side &= 1u;
	hook_write8(field(side, BN6_SOUL_NEXT), 0);
	hook_write8(field(side, BN6_SOUL_NEXT_CHAOS), 0);
	hook_write8(field(side, BN6_SOUL_NEXT_TURNS), 0);
	hook_write16(field(side, BN6_SOUL_NEXT_CHIP), 0);
}

uint8_t team_soul_chaos(unsigned side) { return hook_read8(field(side, BN6_SOUL_NEXT_CHAOS)); }
uint8_t team_soul_turns(unsigned side) { return hook_read8(field(side, BN6_SOUL_NEXT_TURNS)); }

void team_soul_report(void) {
	printf("arena2v2: the partners' Souls: said %u and %u times, ColonelSoul's chip %u and %u times, every core's\n", said[0], said[1],
		chips[0], chips[1]);
}
