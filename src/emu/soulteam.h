/* Team ProtoMan's Souls in BN6's engine (docs/game/team-protoman.md, The
 * build, units 8 and 9; docs/multirom/souls.md): BN5's Souls 1-6, which
 * Team Colonel's ROM holds whole (their forms' tables, kinds, sprites and
 * code) but never gives (their flags 0xFF), on the proxy form as Team
 * Colonel's six are. Their numbers in BN6's engine: a Soul's BN5's own
 * (BN6_SOUL_ID, a used bit), its bit in a MegaMan's Souls held past
 * BN4's twelve; the room their code, art and tables take
 * (FREE_TEAM_SOULS, free_space.h). */
#ifndef CW_SOULTEAM_H
#define CW_SOULTEAM_H

#include "soulbn4.h"

#define SOUL_TEAM_FIRST 1    /* BN5's number (BN6_SOUL_ID) of Team ProtoMan's first Soul: ProtoSoul 1, GyroSoul 2, SearchSoul 3, */
#define SOULS_TEAM      6    /* ... NapalmSoul 4, MagnetSoul 5, MeddySoul 6 */
#define SOUL_PROTO  1
#define SOUL_GYRO   2
#define SOUL_SEARCH 3
#define SOUL_NAPALM 4
#define SOUL_MAGNET 5
#define SOUL_MEDDY  6
#define SOULS_HELD_BN4_ALL 12   /* BN4's Souls' bits in the Souls held, SOULS_HELD_BN4 on (their twelve, moved or not) */
#define SOULS_HELD_TEAM 18      /* the bit of ProtoSoul in the Souls held (soulunite_held): past BN5's Team Colonel's six and BN4's twelve */
#define SOULS_HELD_BN4_MASK (((1u << SOULS_HELD_BN4_ALL) - 1) << SOULS_HELD_BN4)
#define SOULS_HELD_TEAM_MASK (((1u << SOULS_TEAM) - 1) << SOULS_HELD_TEAM)
_Static_assert(SOULS_HELD_BN4 + SOULS_HELD_BN4_ALL <= SOULS_HELD_TEAM, "Team ProtoMan's held bits past BN4's");
/* (the differential's BN5 side, souls5: BN5's flag a Team ProtoMan Soul n is given in its own battle, BN5_SOUL_FLAGS' entry
 * written in the guest's ROM copy: Team Colonel's Soul at the same place's, BN5_FLAG_SOUL + n - 1, which only UNITE's check reads) */
#define SOUL_TEAM_FLAG(n) (0x0008 + (n) - SOUL_TEAM_FIRST)
/* (their sounds Gregar has another song of, at the slots xsong.c's XSOUNDS gives them, the claim's 0x340 on: GyroSoul's propeller
 * set, BN5's 0x167, copied by soulpower.c) */
#define SOUL_TEAM_PROPELLER_SONG 0x167
#define SOUL_TEAM_PROPELLER_SLOT 0x340
#define SOUL_TEAM_GREY_SONG 0x69    /* ... MeddySoul's capsule refused, A on a grey one (BN5's 0x08024A20; soulcapsule.c's Thumb), */
#define SOUL_TEAM_GREY_SLOT 0x341

/* Whether BN5's Soul n (1-6) is one of Team ProtoMan's */
static inline int soulteam_is(int soul) { return soul >= SOUL_TEAM_FIRST && soul < SOUL_TEAM_FIRST + SOULS_TEAM; }

#endif
