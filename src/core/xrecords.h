/* Another game's battle records and Navis, read from its ROM file with no
 * core needed (xrecords.c; docs/MULTIROM.md): a net map's records, the
 * pool a battle is picked from to the act's band, a record's viruses taken
 * up their versions, its Navis' records and stats. Its battles in BN6's
 * engine (xbattles.h) and in its own on the guest core (guest.h) read
 * them. */
#ifndef CW_XRECORDS_H
#define CW_XRECORDS_H

#include <stdbool.h>
#include <stdint.h>

#include "rom.h"

/* A battle's scaling to the act (docs/PROGRESSION.md, BN5's battles): its
 * record's viruses `up` versions up, each to version `vcap` at most (its
 * game's version index: BN5's six V1, V1EX, V2, V2EX, V3, V3EX) and never
 * below its own; within a family (its ids table's version and AI index)
 * the formation stays, its viruses grow. */
typedef struct { int up, vcap; } XScale;

/* The act's band for a record (pacing.c): its viruses' HP together from
 * `lo` (the floor) to `hi` (the cap), their strongest hit `cap` at most,
 * their versions `vcap` at most (as XScale's) */
typedef struct { int lo, hi, cap, vcap; } XBand;

/* Enemy `id` of its game's ROM `d` scaled: `sc.up` versions up, to the
 * scale's vcap at most and never below its own; itself where its family
 * has no such version */
int xrecords_version_up(const uint8_t *d, int id, XScale sc);
/* Enemy `id`'s triple in game `xrom`'s ids table (its version, type: 0 a
 * virus, 1 a Navi, 2 an object; its AI index), NULL none */
const uint8_t *xrecords_triple(int xrom, int id);
/* The id of virus family `ai` (its AI index there) at version `v` in game
 * `xrom`, 0 none */
int xrecords_family_id(int xrom, int ai, int v);
/* Its game's battle records for net map (group, number): how many, and
 * the i-th's address. */
int xrecords_count(int xrom, int group, int number);
uint32_t xrecords_at(int xrom, int group, int number, int i);
/* A record's strength with its viruses scaled ({ 0, 0 } as its game has
 * it), to hold its battles to the run's pacing: its viruses' HP together
 * and the strongest one's damage, from its game's enemy tables; how many
 * viruses it sets (a win counts them as the run's deleted), 0 for one the
 * run never draws: a battle its game keeps for a story's condition (its
 * byte 7), a Navi's, or none at all. */
int xrecords_scaled(int xrom, uint32_t record, XScale sc, int *hp, int *damage);
/* ... and its enemies by its game's ids, so scaled, the first `max` into
 * `ids`; how many (a Navi, a story's battle among them) */
int xrecords_foes_scaled(int xrom, uint32_t record, XScale sc, int *ids, int max);

/* The records a battle on net map (group, number) of `area` (an area of
 * game `xrom`) is picked from, each at the most versions up (ups[i], to
 * the band's vcap) that keep it under the band's cap: inside the band (HP
 * from its floor) the map's own, else the area's maps', else every area
 * its game lends; none inside anywhere, the same three under the floor;
 * else the map's own weakest alone (`fits` false). Those `keep` refuses
 * are left out of all of it (NULL: none), each measured by `measure`. */
typedef struct {
	int xrom;
	const NetAreaDef *area;
	int group, number;
	const XBand *band;
	bool (*keep)(int xrom, uint32_t record);
	/* its strength so scaled as the battle fights it (xrecords_scaled's:
	 * how many foes, 0 none; HP together and the strongest hit), NULL:
	 * xrecords_scaled itself, its own game's stats */
	int (*measure)(int xrom, uint32_t record, XScale sc, int *hp, int *damage);
	uint32_t *out;     /* the records, */
	uint8_t *ups;      /* ... each one's versions up */
	int max;
	bool fits;         /* (out) one fitted under the band's cap */
} XPool;
/* Into p->out and p->ups, how many (0: none at all). Read from the ROM
 * file, no core needed: the director's pick and the pacing report's. */
int xrecords_pool(XPool *p);

/* ---- its Navis (docs/BOSSES.md, BN5's Navis) ---- */

/* The ids table's id of Navi `ai` at version `v`, 0 none */
int xrecords_navi_id(const uint8_t *d, int ai, int v);
/* Navi `ai`'s stats row at version `v` (BN5_ENEMY_STATS: a u16 element <<
 * 12 | HP first), its ROM offset, 0 none */
uint32_t xrecords_navi_stats(const uint8_t *d, int ai, int v);
/* Navi `ai`'s HP at `version` in its game's stats, -1 none; his element
 * into *element where it is not NULL (BN6's numbering, ELEM_*: 0 none, 1
 * Fire, 2 Aqua, 3 Elec, 4 Wood) */
int xrecords_navi_hp(int xrom, int ai, int version, int *element);
/* The chip kind (BN5_CHIP_KIND) Navi `ai`'s Soul unites with in its game,
 * -1 none */
int xrecords_soul_kind(int xrom, int ai);
/* Its game's own battle record for Navi `ai` at `version` (0 V1 .. 3 SP):
 * its story's first that sets him alone, else its net maps' (his SP, where
 * he roams); 0 none. */
uint32_t xrecords_navi_record(int xrom, int ai, int version);

#endif
