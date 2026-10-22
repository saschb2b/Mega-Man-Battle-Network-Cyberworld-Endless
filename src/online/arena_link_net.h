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
 * already come are spent, never hangs. ROM-free: the transport is a call.
 *
 * Two MegaMen a side (protocol v15, issue #181): a link of four seats,
 * mine one of them (0 and 1 the GBAs' players, 2 and 3 their partners).
 * The others' messages come as ARENA_SEAT, each seat's stream, hashes,
 * ready and record kept apart; the peer is the other GBA's player (for a
 * partner, the other side's lead), whose keys and lag notes drive the time
 * sync, the rest come through poll_player. A ready, a record and a hash
 * are said once every other seat's has come: mine where all agree with
 * mine, else the first that differs, so that a game apart from any of the
 * three stops. */
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

typedef struct { uint32_t frame; uint64_t hash; } ArenaNetHash;

/* what has come of one other seat */
typedef struct {
	uint16_t keys[ARENA_NET_PEER];
	uint32_t next, polled;            /* its frames come below next; the engine's asked below polled */
	ArenaNetHash hashes[ARENA_NET_HASHES];
	unsigned nhashes;
	bool ready, recorded;
	uint64_t ready_hash, record;
	bool lagged;                      /* a lag note come, not yet taken */
	int16_t lag;
} ArenaNetSeat;

typedef struct {
	ArenaLink link;
	ArenaSend out;
	void *context;
	uint32_t match;
	unsigned seats;                   /* 2, or a 2v2's ONLINE_SEATS */
	uint32_t sent;                    /* my next frame */
	uint32_t batch_first;
	uint16_t batch[ONLINE_ARENA_INPUT_MAX];
	unsigned batched;
	ArenaNetSeat from[ONLINE_SEATS];  /* by seat (mine unused) */
	ArenaNetHash mine[ARENA_NET_HASHES];   /* my hashes, said against a 2v2's three others' */
	unsigned nmine;
	bool readied, recorded;           /* my ready and record said, and what they said */
	uint64_t ready_hash, record;
	bool ended;                       /* the server said the battle is over for us */
	unsigned since_end;
	char broken[96];
} ArenaNetLink;

/* The link for battle `match`, my side and the delay START gave */
ArenaLink *arena_link_net(ArenaNetLink *l, uint32_t match, unsigned side, unsigned delay, ArenaSend out, void *context);
/* ... a 2v2's (protocol v15), before any message: four seats, mine `seat`
 * (link.side its GBA's, seat & 1) */
void arena_link_net_seats(ArenaNetLink *l, unsigned seat);
/* A relayed message for this battle (ARENA_INPUT or ARENA_HASH, a 2v2's
 * in ARENA_SEAT): false where it is not this battle's or not one */
bool arena_link_net_take(ArenaNetLink *l, const uint8_t *data, size_t size);
/* My pair at the Custom screen: ready, its hash said */
bool arena_link_net_ready(ArenaNetLink *l, uint64_t hash);
/* The peer's ready and its hash, once it has come (a 2v2's: once every
 * other seat's has, the first apart from mine where one is) */
bool arena_link_net_peer_ready(const ArenaNetLink *l, uint64_t *hash);
/* ... seat's alone (a 2v2's way in: whose PET is still on its way) */
bool arena_link_net_seat_ready(const ArenaNetLink *l, unsigned seat);
/* My cores' installs' record (0 none), said before my ready; the peer's,
 * once it has come (an older game sends none; a 2v2's as its ready) */
bool arena_link_net_record(ArenaNetLink *l, uint64_t record);
bool arena_link_net_peer_record(const ArenaNetLink *l, uint64_t *record);
/* My keys not yet sent, sent now (before the end's word goes) */
bool arena_link_net_flush(ArenaNetLink *l);
/* The server's end of the battle (the peer left or finished) */
void arena_link_net_ended(ArenaNetLink *l);
/* The connection gone: the link broken, `why` said */
void arena_link_net_lost(ArenaNetLink *l, const char *why);

#endif
