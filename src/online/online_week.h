/* The week's net on the wire (online_week.c, docs/ONLINE.md, The week's
 * net; protocol v6): the Board's dives as the server last said
 * (WEEK_BOARD), and a dive's end posted (WEEK_POST). */
#ifndef CW_ONLINE_WEEK_H
#define CW_ONLINE_WEEK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "protocol.h"

typedef struct {
	char name[ONLINE_NAME_BYTES + 1];
	uint16_t depth, minutes;
	uint8_t act;
	bool won;
} WeekEntry;

typedef struct {
	uint32_t serial;     /* WEEK_BOARDs come in this visit, 0 none yet */
	uint16_t week;
	int place;           /* MegaMan's own best's place this week, 0 none */
	WeekEntry mine;
	int ntop, nlast;
	WeekEntry top[ONLINE_WEEK_TOP], last[ONLINE_WEEK_LAST_TOP];
} WeekBoard;

/* As MegaMan arrives in the Square: none. */
void online_week_reset(void);
/* A WEEK_BOARD come in; false where it is malformed. */
bool online_week_mail(const uint8_t *data, size_t size);
const WeekBoard *online_week_board(void);
/* A dive's end posted (WEEK_POST); false where it was not sent. */
bool online_week_post(uint16_t week, uint32_t seed, int depth, int act, bool won, unsigned minutes, uint32_t hash);
/* Week `week`'s Monday, seconds since 1970 (UTC). */
int64_t online_week_start(uint16_t week);

#endif
