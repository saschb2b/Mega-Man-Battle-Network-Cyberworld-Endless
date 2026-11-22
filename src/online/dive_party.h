/* The shared dive's party at a guardian's door, and its fight (issue #589;
 * docs/online/shared-dive.md). MegaMan saved at the arena's door with
 * other divers on the layer joins its queue (QUEUE join); the staging
 * waits (boss_door_hold) while the leader, the first at the door, may go in
 * when he likes (A) and anyone may go alone (B); a fifth at a full door
 * is told so and keeps his keys. The leader's start makes a
 * party battle for every game in the queue: each game's guardian scaled
 * for the party (boss_party_set). Linked (protocol v19's format 3), each
 * game fights him in its own battle and his HP is the party's: seat 0's
 * game keeps it, the damage each other game deals coming to it
 * (PARTY_DAMAGE), the HP it keeps going to them (PARTY_HP). */
#ifndef CW_DIVE_PARTY_H
#define CW_DIVE_PARTY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
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

/* The door's band line, NULL none: who waits, and the keys; or that the
 * queue holds its four already (a join refused, issue #753: MegaMan held at
 * the door, A goes in alone, his other keys his own, a place asked for
 * again every few seconds). The dive's band draws it before the feed's
 * lines: a queued diver's own line is never put off by them (issue #778). */
const char *dive_party_band(void);
/* The party forming at the door, MegaMan among them, the leader first (0
 * none: not queued, or queued alone); the crew's plates show them there
 * (dive_crew_view.h). */
int dive_party_members(const QueueEntry **out);
/* The door in the state's words: `door none|alone|queued: NAMES|full|started`. */
void dive_party_describe(FILE *f);

#endif
