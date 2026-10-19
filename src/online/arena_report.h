/* The report of a NetBattle for the stands (issue #128, docs/ONLINE.md,
 * The stands): the cable's first game reads both MegaMen of its pair's
 * first core (each side's player object, BN6_BATTLE_PLAYERS: HP, max HP,
 * and a chip as he starts using it) and sends what it read now and then
 * (ARENA_STATUS), and the winner at the end, for every game of the
 * channel, one that cannot watch among them. It reads, never writes: the
 * battle is the same with it or without. */
#ifndef CW_ARENA_REPORT_H
#define CW_ARENA_REPORT_H

#include <stdbool.h>
#include <stdint.h>

#include "arena.h"
#include "stands_report.h"

/* A battle begun (its id), as the cable's first: nothing read yet */
void arena_report_begin(uint32_t match);
/* After each tick of the battle: its pair's first core read; sent when it changed, at most every REPORT_EVERY frames */
void arena_report_frame(const Arena *a);
/* The battle over: the cable's first player's `result` (1 won, 2 lost), sent at once */
void arena_report_end(unsigned result);

#endif
