/* Two MegaMen a side over the network (arena_seats.h). */
#include "arena_seats.h"

#include <stdio.h>
#include <stdlib.h>

#include "arena_carry.h"
#include "arena_team_custom.h"
#include "arena_team_navi.h"
#include "arena_team_navicust.h"
#include "devtools.h"

static bool four;
/* each side's partner's MegaMan whose NaviCust the way in compiles (arena_seats_compile), as every game was given it */
static OnlineLoadout partners[2];
static bool given[2];

bool arena_seats_on(void) { return four || dev.arena_2v2 > 0; }

void arena_seats_set(bool on) { four = on; }

bool arena_seats_net(void) { return four; }

void arena_seats_unite_folder(OnlineLoadout *l) {
	l->souls |= 1u << 2;   /* (NumberSoul: BN5's Soul 9, bit k Soul 7 + k) */
	l->folder[0] = (uint16_t)(BN6_CHIP_ATK10 | 26u << 9);   /* (Atk+10 *: a Plus chip, NumberSoul's kind) */
	l->regular = 0;
	l->tag[0] = l->tag[1] = 0xFF;
}

void arena_seats_pa_folder(OnlineLoadout *l) {
	for (unsigned i = 0; i < 3; ++i) l->folder[i] = (uint16_t)(BN6_CHIP_CANNON | i << 9);
	l->regular = 0;
	l->tag[0] = 1;
	l->tag[1] = 2;
}

void arena_seats_partners(const OnlineLoadout *partner[2]) {
	for (unsigned side = 0; side < 2; ++side) {
		const OnlineLoadout *l = partner[side];
		arena_seats_partner(side, l);
		/* (what his messages may say: what his MegaMan holds, the same on every game, team_check.h) */
		const TeamAllowed allowed = { l ? (uint8_t)(l->crosses & 0x1Fu) : 0, l ? l->souls : 0, l && l->beast };
		arena_team_custom_allow(side, &allowed);
		if (!l) {
			arena_team_custom_folder(side, NULL, 0, NULL);
			arena_team_navi_loadout(side, 0, 0, NULL);
			continue;
		}
		arena_team_custom_folder(side, l->folder, l->regular, l->tag);
		const uint8_t buster[3] = { l->attack, l->speed, l->charge };
		arena_team_navi_loadout(side, l->max_hp, l->base_hp, buster);
		printf("arena: side %u's partner: HP %u, buster %u %u %u, the folder's first chips %04x %04x %04x, Crosses %#x, Beast Out %u\n",
			side, l->max_hp, l->attack, l->speed, l->charge, l->folder[0], l->folder[1], l->folder[2], l->crosses, l->beast);
	}
}

void arena_seats_partner(unsigned side, const OnlineLoadout *l) {
	side &= 1u;
	given[side] = l != NULL;
	if (l) partners[side] = *l;
	/* (his record his side's, his loadout's numbers over it, until the way in compiles his NaviCust) */
	arena_team_navi_compiled(side, NULL);
}

static unsigned u16_at(const uint8_t *bytes, unsigned at) { return bytes[at] | (unsigned)bytes[at + 1] << 8; }

bool arena_seats_compile(const uint8_t *base, size_t n, char *why, size_t wn) {
	for (unsigned side = 0; side < 2; ++side) {
		if (!given[side]) continue;
		static uint8_t stats[2][BN6_BATTLE_NAVI_SIZE];
		/* (teamcust=-2 on one game: over its own checkpoint's state instead, as before issue #181's review: a mutation that
		 * must stop the four games apart at the ready) */
		size_t own_size = 0;
		uint8_t *own = dev.team_cust == -2 ? arena_carry_state(&own_size) : NULL;
		bool made = team_navicust_compile(own ? own : base, own ? own_size : n, &partners[side], stats[side], why, wn);
		free(own);
		if (!made) {
			printf("arena2v2: side %u's partner's NaviCust not compiled over the way in's state: %s: no battle\n", side, why);
			return false;
		}
		const uint8_t *s = stats[side];
		arena_team_navi_compiled(side, s);
		printf("arena2v2: side %u's partner's NaviCust compiled by BN6 (%08X) over the way in's state%s: HP %u (base %u), Custom %u, "
			"buster %u %u %u, SuperArmor %u, FloatShoe %u, AirShoe %u, UnderShirt %u\n", side, BN6_NAVICUST_RELOAD,
			own ? " (teamcust=-2: over this game's own checkpoint)" : "", u16_at(s, BN6_NS_MAX_HP), u16_at(s, BN6_NS_BASE_HP),
			s[BN6_NS_CUSTOM], s[BN6_NS_ATTACK] + 1, s[BN6_NS_SPEED] + 1, s[BN6_NS_CHARGE] + 1, s[BN6_NS_SUPER_ARMOR],
			s[BN6_NS_FLOAT_SHOE], s[BN6_NS_AIR_SHOE], s[BN6_NS_UNDER_SHIRT]);
	}
	return true;
}
