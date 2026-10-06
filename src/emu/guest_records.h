/* Another game's battle records and Navis read from its ROM file
 * (guest_records.c): what the guest's core takes of them for a battle
 * (guest.c). The rest of the game reads them through guest.h. */
#ifndef CW_GUEST_RECORDS_H
#define CW_GUEST_RECORDS_H

#include <stdint.h>

#include "guest.h"

/* Enemy `id` of its game's ROM `d` scaled: `sc.up` versions up, to the
 * scale's vcap at most and never below its own; itself where its family
 * has no such version */
int guest_version_up(const uint8_t *d, int id, GuestScale sc);
/* The ids table's id of Navi `ai` at version `v`, 0 none */
int guest_navi_id(const uint8_t *d, int ai, int v);
/* Navi `ai`'s stats row at version `v` (BN5_ENEMY_STATS: a u16 element <<
 * 12 | HP first), its ROM offset, 0 none */
uint32_t guest_navi_stats(const uint8_t *d, int ai, int v);

#endif
