/* Online play's statistics (issue #156, docs/GUIDE.md's Anonymous
 * statistics): under the same yes as the rest (analytics.h), queued and
 * sent as they are, off the frame loop. Counts and settings only: never a
 * name, an identity, a server's address or anything typed.
 *
 *   online-visit  a visit to a Net Square over: the system, the NetPass
 *                 (pass: square or full), how it ended (the visit's own
 *                 ends, online_visit.h), the minutes in the Square where
 *                 MegaMan got there, whether the server was the official
 *                 one (a flag, never the address)
 *   netbattle     a NetBattle over: the system, the cable's side, the delay
 *                 and the rollback window in frames, the rollbacks and the
 *                 deepest, the worst tick in ms, the stalls, its seconds,
 *                 how it ended (won, lost, over, disconnected, apart,
 *                 stopped, left)
 *   online-trade  a trade done at Trade Central: whether it began with a
 *                 trade board's offer
 *   online-watch  a battle watched from the stands, to its end or not: how,
 *                 its seconds
 *   deepnet-post  the week's dive taken by the Board: its layer, act and
 *                 whether it reached the end
 *
 * The other player's system crosses no wire, so a NetBattle names ours
 * alone. */
#ifndef CW_ANALYTICS_ONLINE_H
#define CW_ANALYTICS_ONLINE_H

#include <stdbool.h>

/* A visit over: `ended` the visit's end (VisitEnd, online_visit.h), `full`
 * the NetPass's full set, `seconds` in the Square (below 0: MegaMan never
 * got there), `official` the gate's official server */
void analytics_visit(int ended, bool full, int seconds, bool official);

/* How a NetBattle ended */
enum {
	ANALYTICS_BATTLE_WON,
	ANALYTICS_BATTLE_LOST,
	ANALYTICS_BATTLE_OVER,          /* over, neither won (BN6's other results) */
	ANALYTICS_BATTLE_DISCONNECTED,  /* the other gone, or the connection lost */
	ANALYTICS_BATTLE_APART,         /* the two games' hashes differed */
	ANALYTICS_BATTLE_STOPPED,       /* the cable stuck, keys out of order */
	ANALYTICS_BATTLE_LEFT,          /* the Square left with the battle on */
};

/* ... and its network's numbers, as the Arena counted them */
typedef struct {
	int how;
	int side;                 /* 0 the cable's first, 1 its second */
	int delay, window;        /* the input delay and the rollback window, frames */
	int rollbacks, deepest;   /* the rollbacks, and the most frames at once */
	int stalls;               /* ticks waiting for the other's keys */
	double worst_tick_ms;     /* a tick's longest */
	int frames;               /* the battle's frames run (0 none) */
} AnalyticsBattle;
void analytics_netbattle(const AnalyticsBattle *b);

/* A trade done; `board` it began with a trade board's offer */
void analytics_trade(bool board);

/* A battle watched, and how it ended for the watcher */
enum {
	ANALYTICS_WATCH_END,      /* to the battle's end */
	ANALYTICS_WATCH_LEFT,     /* B: back to the Square */
	ANALYTICS_WATCH_SLOW,     /* this machine could not keep up */
	ANALYTICS_WATCH_FAILED,   /* the watched pair stopped otherwise */
	ANALYTICS_WATCH_LOST,     /* the connection lost */
};
void analytics_watch(int how, int seconds);

/* The week's dive taken by the Board: its layer, act, and won */
void analytics_dive_post(int layer, int act, bool won);

#endif
