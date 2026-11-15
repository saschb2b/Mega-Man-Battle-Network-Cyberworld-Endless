/* The shared dive's party at a guardian's door, and its fight (issue #589;
 * docs/online/shared-dive.md). MegaMan saved at the arena's door with
 * other divers on the layer joins its queue (QUEUE join); the staging
 * waits (boss_door_hold) while the leader, the first at the door, may go in
 * when he likes (A) and anyone may go alone (B). The leader's start makes a
 * party battle for every game in the queue: each game's guardian scaled
 * for the party (boss_party_set). Linked (protocol v19's format 3), each
 * game fights him in its own battle and his HP is the party's: seat 0's
 * game keeps it, the damage each other game deals coming to it
 * (PARTY_DAMAGE), the HP it keeps going to them (PARTY_HP). */
#ifndef CW_DIVE_PARTY_H
#define CW_DIVE_PARTY_H

#include <stddef.h>
#include <stdint.h>

#include "dive_wire.h"
#include "online_types.h"

/* The door's hold (boss_door_hold) taken, or given back (on false). */
void dive_party_on(int on);

/* Each frame of the dive, on its layer or in its battle: `room` the layer
 * MegaMan is in, `s` its divers. */
void dive_party_frame(const OnlineSnapshot *s, DiveWay room);

/* The pad's keys while MegaMan waits at the door: A and B the queue's. */
uint32_t dive_party_keys(uint32_t keys);

/* QUEUE_STATE, a party battle's ARENA_STATE and ARENA_SEAT, PARTY_HP. */
void dive_party_mail(const uint8_t *m, size_t n);

/* The door's band: who waits, and the keys. */
void dive_party_draw(int x, int y);

#endif
