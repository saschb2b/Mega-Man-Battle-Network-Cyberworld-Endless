/* Another game's battle records fought in BN6's own engine (xbattles.c;
 * issue #82, docs/multirom/one-net.md): a BN5 record as an Encounter of
 * BN6's ids, its field and its objects, and a territory's random battle
 * picked from its own maps' records as BN6's are from its areas'. Read
 * from the ROM files and the families installed in the core (xvirus.h):
 * a family both games have by name is BN6's (its version by tier: BN5's
 * V1 and V1EX as BN6's V1, and so on), a family moved into the engine its
 * installed ids, any other not fightable yet. */
#ifndef CW_XBATTLES_H
#define CW_XBATTLES_H

#include <stdbool.h>
#include <stdint.h>

#include "foes.h"
#include "xrecords.h"

/* Whether a record can be fought in BN6's engine, and why not */
enum {
	XB_FIGHTABLE,   /* it can */
	XB_NONE,        /* no record there, or its game's ROM is not read */
	XB_STORY,       /* a battle its game keeps for a story's condition (its byte 7) */
	XB_NAVI,        /* one of its game's Navis (what: his AI index there) */
	XB_FAMILY,      /* a virus family BN6 has none of by name, not moved into the engine (what: its AI index there) */
	XB_FIELD,       /* its field is none of BN6's (what: its number there) */
	XB_OBJECT,      /* an object of a kind BN6's records never set (what: the entity's kind byte) */
	XB_FULL,        /* more foes or objects than a battle of BN6's takes */
};

/* Record `record` of game `xrom` (a BattleSettings address in its ROM, its
 * battle tables') with its viruses scaled (`sc`; { 0, 0 } as its game has
 * it) as an Encounter of BN6's: its foes by BN6's ids at their panels, its
 * field by BN6's number (xfields.h), MegaMan's panel, its rocks as BN6 has
 * them, and its green Mystery Data by its game's rule (there where its
 * byte 2 beats a roll of 15: rows 1-5 always) with its game's finds as
 * BN6's (Encounter.gem_own; xchips.h's xchips_reward_bn6, in * with the
 * All * helper); its families and viruses for the repeat rule. XB_FIGHTABLE
 * and *out made, the Mystery Data's roll drawing from the run's random
 * numbers; else why not (*what as the enum says, *out then unchanged). */
int xbattle_encounter(int xrom, uint32_t record, XScale sc, Encounter *out, int *what);
/* ... the same with nothing drawn, its Mystery Data kept whatever its
 * chance (byte 2 still its arg: its row and chance): xbattle_encounter's
 * own first step, the dev listing's (*out made where XB_FIGHTABLE) */
int xbattle_translate(int xrom, uint32_t record, XScale sc, Encounter *out, int *what);
/* ... whether it can be, nothing drawn (the pools' filter) */
bool xbattle_fightable(int xrom, uint32_t record);
/* ... and why not, nothing drawn: XB_* as xbattle_encounter's */
int xbattle_why(int xrom, uint32_t record, int *what);
/* BN6's id for enemy `id` of game `xrom`: a family both games have, BN6's
 * at the tier of its version; one moved into the engine, its installed id;
 * -1 none (*why XB_NAVI or XB_FAMILY, *what its AI index there) */
int xbattle_enemy_bn6(int xrom, int id, int *why, int *what);

/* Whether BN6's virus family `family` (its AI index, 1-29) is one game
 * `xrom` has by name (BN5's Mettaur, Catack, Champy, WindBox, Trumpy):
 * one family of both nets, BN6's */
bool xbattle_shared(int xrom, int family);

/* Territory `area` (NET_AREAS + k, an area of another game's, rom.h): the
 * random records (byte 7 clear) of its three maps (NetAreaDef.xbattles),
 * how many, and into *fightable how many of them xbattle_fightable takes */
int xterritory_records(int area, int *fightable);
/* ... whether every one of them can be fought in BN6's engine: its random
 * battles can be its own records there (issue #82's M3) */
bool xterritory_native(int area);
/* The territory of another game's whose look a run's BN6 area `biome`
 * takes (NetAreaDef.like: BN5's Undernet the Undernet's, Nebula Area the
 * Graveyard's) where its ROM is read and it is native, -1 none */
int xterritory_like(int biome);
/* A random battle on a layer of territory `area` at `depth`, of `kind`
 * (loot.h's ENC_*), from its own records as BN6's are from its areas'
 * (docs/game/progression/): the act's band (the lower half for the run's
 * opening battles, the Gentle helper's and a challenge's the next act's),
 * the pool its game's records fit in (xrecords_pool: its layer's map, else
 * its other maps, else its game's other areas, its viruses taken up to the
 * act's version by tier: BN5 version index 2 x BN6's + 1 at most), those
 * fightable alone; an opening battle two viruses at most where the pool
 * has such, the run's first one; BN6's four passes against repeats
 * (loot.h), then a pick by the run's random numbers, reproducible from the
 * layer's seed as BN6's battles are. Its memory: from_biome the area,
 * from_pick the record's offset in its ROM. False where none can be
 * (*out unchanged). */
bool xterritory_encounter(int depth, int area, int kind, Encounter *out);
/* The virus families (a bit each, by BN6's AI index: the families moved
 * into the engine from 32) territory `area`'s plain random battles can
 * hold at `depth`, as xterritory_encounter picks them; and into weak[f]
 * (NULL: none) a bit per ELEM_* each family is weak to as they field it.
 * No random numbers drawn. */
uint64_t xterritory_families(int depth, int area, uint16_t weak[64]);

#endif
