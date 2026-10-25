/* The Arena's link to the other player (issue #120): what a transport
 * carries for a linked battle, and nothing more. Keys go one way per
 * frame, in order, each frame once; the pair's state hashes go now and
 * then, so that two clients that drift apart say so (docs/online/arena/).
 *
 * The local link (arena_link_local) stands in for a peer on this machine,
 * its messages held for a set number of ticks; the network's comes later,
 * over the presence server, written against this interface. */
#ifndef CW_ARENA_LINK_H
#define CW_ARENA_LINK_H

#include <stdbool.h>
#include <stdint.h>

#include "arena_input.h"

typedef struct ArenaLink ArenaLink;
struct ArenaLink {
	/* my keys for `frame` (a tick's own plus the delay): each frame once, in order */
	bool (*send)(ArenaLink *link, uint32_t frame, uint16_t keys);
	/* the peer's keys for `frame`, once they have come: true and *keys */
	bool (*poll)(ArenaLink *link, uint32_t frame, uint16_t *keys);
	/* my pair's hash after `frame`; the peer's for that frame once it has come */
	void (*send_hash)(ArenaLink *link, uint32_t frame, uint64_t hash);
	bool (*poll_hash)(ArenaLink *link, uint32_t frame, uint64_t *hash);
	/* a tick of wall time (a 60th of a second) has passed: once a tick, before the sends */
	void (*tick)(ArenaLink *link);
	/* the link gone (the peer left, a transport error): its reason, else NULL */
	const char *(*broken)(ArenaLink *link);
	void (*close)(ArenaLink *link);
	unsigned side;    /* my player: 0 the cable's first GBA, 1 the second */
	unsigned delay;   /* D: frames from a key's sampling to its frame (ARENA_DELAY_MIN..MAX) */
	/* my least lag of the other's keys over the last second, in ticks, for
	 * his time sync (arena_sync.h, protocol v11); and his, once a new note
	 * has come. NULL where the link carries none (the local one) */
	void (*send_lag)(ArenaLink *link, int lag);
	bool (*poll_lag)(ArenaLink *link, int *lag);
	/* another player's keys for `frame` (ARENA_PLAYERS: a partner's, 2 or
	 * 3; in a 2v2 over the network any seat's but mine and the peer's),
	 * once they have come, as poll's: NULL where the link carries none (a
	 * NetBattle of two over the network, docs/online/arena/two-a-side/overview.md, Two MegaMen
	 * a side) */
	bool (*poll_player)(ArenaLink *link, unsigned player, uint32_t frame, uint16_t *keys);
	/* a 2v2's partner's seat (2 or 3: his stream mine, his side's GBA
	 * `side`'s; protocol v15, issue #181); 0 my side's player's, as ever */
	unsigned seat;
};

/* the stream my keys go in: my seat's, else my side's player's */
static inline unsigned arena_link_seat(const ArenaLink *l) { return l->seat >= 2 ? l->seat : l->side; }

/* A peer on this machine: its keys for a frame (an idle one gives 0) */
typedef uint16_t (*ArenaPeerKeys)(void *context, uint32_t frame);
/* ... and a partner's (player 2 or 3) */
typedef uint16_t (*ArenaPartnerKeys)(void *context, unsigned player, uint32_t frame);

#define ARENA_LOCAL_QUEUE 128   /* messages under way: a power of two */
#define ARENA_LOCAL_HASHES 16

typedef struct {
	ArenaLink link;
	unsigned latency;   /* ticks a message takes, one way */
	uint32_t now;       /* the tick under way, 0 the battle's first */
	bool started;       /* (its first tick begun) */
	/* each remote player's messages under way: the peer's (player 1), the partners' (2 and 3) */
	struct { uint32_t at, frame; uint16_t keys; } queue[ARENA_PLAYERS][ARENA_LOCAL_QUEUE];
	uint32_t head[ARENA_PLAYERS], tail[ARENA_PLAYERS];
	struct { uint32_t at, frame; uint64_t hash; } hashes[ARENA_LOCAL_HASHES];
	unsigned nhashes;
	ArenaPeerKeys peer;
	void *peer_context;
	ArenaPartnerKeys partners;   /* NULL: no partners' streams */
	void *partners_context;
	uint32_t sent;      /* my frames sent: the next is this one */
	bool lost;          /* a message dropped (the queue full): broken */
} ArenaLocalLink;

/* The local link, I the cable's first GBA (side 0): the peer's keys for
 * frame t + delay made at tick t, each message delivered `latency` ticks
 * later; the hashes come back as sent, after the same latency */
ArenaLink *arena_link_local(ArenaLocalLink *l, unsigned delay, unsigned latency, ArenaPeerKeys peer, void *context);
/* ... with both partners on it too (--dev arena2v2: players 2 and 3, by
 * the frame as the peer's, after the same latency), before its first tick */
void arena_link_local_partners(ArenaLocalLink *l, ArenaPartnerKeys partners, void *context);

#endif
