/* The network's ArenaLink (issue #120, docs/ONLINE_ARENA.md): the Arena's
 * keys and hashes over the presence server, which relays them to the
 * other player (arena_wire.h). My keys go as each frame's are sampled
 * (ARENA_NET_FLUSH: two frames a message cost a delay of 6 half its
 * ticks in stalls, measured; one cost none), and where the outbox was full
 * they wait and go together, ONLINE_ARENA_INPUT_MAX a message at most; the
 * peer's come in as the owner hands their messages over. Before the
 * battle each side says it is ready with its pair's hash at the Custom
 * screen, so that both start within the link's one way of each other and
 * a pair built apart is caught before the first frame; just before it,
 * the installs' record its cores were made with, so that a pair apart
 * because the two games' BN5 installs differ is told as such. The battle over
 * on the other side, or the peer gone, the link breaks once the keys
 * already come are spent, never hangs. ROM-free: the transport is a call. */
#ifndef CW_ARENA_LINK_NET_H
#define CW_ARENA_LINK_NET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "arena_link.h"
#include "protocol.h"

/* a message out (the transport's fast outbox): false where it is full */
typedef bool (*ArenaSend)(void *context, const uint8_t *data, size_t size);

#define ARENA_NET_PEER    512           /* the peer's frames held: a power of two, past ARENA_RING */
#define ARENA_NET_HASHES  16
#define ARENA_NET_READY   0xFFFFFFFFu   /* ARENA_HASH's frame for "ready, my pair's hash at the Custom screen" */
#define ARENA_NET_RECORD  0xFFFFFFFEu   /* ... for "my cores' installs' record" (xinstall_log.h's hash, 0 none), sent before the ready */
#define ARENA_NET_LAG     0xFFFFFFFDu   /* ... for "my least lag of your keys", its low 16 bits signed ticks (protocol v11, arena_sync.h) */
#define ARENA_NET_FLUSH   1             /* my keys a message, the outbox willing */
#define ARENA_NET_GRACE   90            /* ticks after the peer's end that its keys already come may run */

typedef struct {
	ArenaLink link;
	ArenaSend out;
	void *context;
	uint32_t match;
	uint32_t sent;                    /* my next frame */
	uint32_t batch_first;
	uint16_t batch[ONLINE_ARENA_INPUT_MAX];
	unsigned batched;
	uint16_t peer[ARENA_NET_PEER];
	uint32_t peer_next, polled;       /* the peer's frames come below peer_next; the engine's asked below polled */
	struct { uint32_t frame; uint64_t hash; } hashes[ARENA_NET_HASHES];
	unsigned nhashes;
	bool peer_ready, peer_recorded;
	uint64_t peer_ready_hash, peer_record;
	bool peer_lagged;                 /* a lag note come, not yet taken */
	int16_t peer_lag;
	bool ended;                       /* the server said the battle is over for us */
	unsigned since_end;
	char broken[96];
} ArenaNetLink;

/* The link for battle `match`, my side and the delay START gave */
ArenaLink *arena_link_net(ArenaNetLink *l, uint32_t match, unsigned side, unsigned delay, ArenaSend out, void *context);
/* A relayed message for this battle (ARENA_INPUT or ARENA_HASH): false
 * where it is not this battle's or not one */
bool arena_link_net_take(ArenaNetLink *l, const uint8_t *data, size_t size);
/* My pair at the Custom screen: ready, its hash said */
bool arena_link_net_ready(ArenaNetLink *l, uint64_t hash);
/* The peer's ready and its hash, once it has come */
bool arena_link_net_peer_ready(const ArenaNetLink *l, uint64_t *hash);
/* My cores' installs' record (0 none), said before my ready; the peer's,
 * once it has come (an older game sends none) */
bool arena_link_net_record(ArenaNetLink *l, uint64_t record);
bool arena_link_net_peer_record(const ArenaNetLink *l, uint64_t *record);
/* My keys not yet sent, sent now (before the end's word goes) */
bool arena_link_net_flush(ArenaNetLink *l);
/* The server's end of the battle (the peer left or finished) */
void arena_link_net_ended(ArenaNetLink *l);
/* The connection gone: the link broken, `why` said */
void arena_link_net_lost(ArenaNetLink *l, const char *why);

#endif
