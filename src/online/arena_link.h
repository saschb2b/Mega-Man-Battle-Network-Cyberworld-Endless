/* The Arena's link to the other player (issue #120): what a transport
 * carries for a linked battle, and nothing more. Keys go one way per
 * frame, in order, each frame once; the pair's state hashes go now and
 * then, so that two clients that drift apart say so (docs/ONLINE_ARENA.md).
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
};

/* A peer on this machine: its keys for a frame (an idle one gives 0) */
typedef uint16_t (*ArenaPeerKeys)(void *context, uint32_t frame);

#define ARENA_LOCAL_QUEUE 128   /* messages under way: a power of two */
#define ARENA_LOCAL_HASHES 16

typedef struct {
	ArenaLink link;
	unsigned latency;   /* ticks a message takes, one way */
	uint32_t now;       /* the tick under way, 0 the battle's first */
	bool started;       /* (its first tick begun) */
	struct { uint32_t at, frame; uint16_t keys; } queue[ARENA_LOCAL_QUEUE];
	uint32_t head, tail;
	struct { uint32_t at, frame; uint64_t hash; } hashes[ARENA_LOCAL_HASHES];
	unsigned nhashes;
	ArenaPeerKeys peer;
	void *peer_context;
	uint32_t sent;      /* my frames sent: the next is this one */
	bool lost;          /* a message dropped (the queue full): broken */
} ArenaLocalLink;

/* The local link, I the cable's first GBA (side 0): the peer's keys for
 * frame t + delay made at tick t, each message delivered `latency` ticks
 * later; the hashes come back as sent, after the same latency */
ArenaLink *arena_link_local(ArenaLocalLink *l, unsigned delay, unsigned latency, ArenaPeerKeys peer, void *context);

#endif
