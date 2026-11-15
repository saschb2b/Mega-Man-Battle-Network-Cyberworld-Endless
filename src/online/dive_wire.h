/* The shared dive's messages on the wire (protocol v19, issue #589;
 * shared/online/README.md, Protocol v19):
 * what a diver's game sends (DIVE's actions, CREW, QUEUE, PARTY_DAMAGE and
 * PARTY_HP) built, and what the server tells it (DIVE_ROOM, DIVE_TRACES,
 * DIVE_PLACE, DIVE_FEED, DIVERS, CREW_STATE, QUEUE_STATE, a linked
 * fighter's damage at seat 0, PARTY_HP, WEEK_SPLITS and a party battle's
 * ARENA_STATE) read into plain structs, each shape and bound checked as the
 * server checks it. Big-endian; no game state, no I/O. ROM-free:
 * tests/test_online.c. */
#ifndef CW_DIVE_WIRE_H
#define CW_DIVE_WIRE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "protocol.h"

/* (the most entries of each DIVE_TRACES kind one message holds) */
#define DIVE_TRACES_ROOM (ONLINE_MAX_MESSAGE - ONLINE_TRACE_HEAD)
#define DIVE_MARKS_MOST (DIVE_TRACES_ROOM / ONLINE_TRACE_MARK_BYTES)
#define DIVE_PHRASES_MOST (DIVE_TRACES_ROOM / ONLINE_TRACE_PHRASE_BYTES)
#define DIVE_FINDS_MOST (DIVE_TRACES_ROOM / ONLINE_TRACE_FIND_BYTES)
#define DIVE_OPENED_MOST 255
#define DIVE_SPLITS_MOST (DIVE_TRACES_ROOM / ONLINE_TRACE_SPLIT_BYTES)
#define DIVE_GONE_MOST (DIVE_TRACES_ROOM / 4)

/* A layer's place among the week's: its depth (1..ONLINE_WEEK_LAYERS, 0 none),
 * its side kind (0 the main path) and its way (its area) */
typedef struct {
	uint8_t depth, side, way;
} DiveWay;

typedef struct {
	uint16_t week;
	DiveWay way;
	uint16_t instance;      /* 1 the first room of the layer */
	uint8_t divers;         /* in the room, the recipient among them */
} DiveRoom;

typedef struct {
	char name[ONLINE_NAME_BYTES + 1];
	int16_t x, y, z;
	uint16_t cause;         /* what deleted him, 0 unknown */
	uint8_t how;            /* ONLINE_DIVE_DELETED or ONLINE_DIVE_WON */
} DiveMark;

typedef struct {
	uint32_t id;            /* the server's, for a Wave */
	char name[ONLINE_NAME_BYTES + 1];
	uint8_t phrase;         /* the Square's fixed phrase */
	int16_t x, y, z;
	uint16_t waves;
} DivePhrase;

typedef struct {
	uint8_t index;          /* the Mystery Data's on the layer */
	char name[ONLINE_NAME_BYTES + 1];
} DiveFind;

typedef struct {
	uint8_t index, party;   /* the battle's on the layer; the party's size, 1 alone */
	uint16_t tenths;
	uint8_t level;          /* BN6's Busting Level, 11 for S */
	bool guardian;
	char name[ONLINE_NAME_BYTES + 1];
} DiveSplit;

/* DIVE_TRACES: one kind's entries (ONLINE_TRACE_*); `count` of them in its array */
typedef struct {
	uint8_t kind, count;
	union {
		DiveMark marks[DIVE_MARKS_MOST];
		DivePhrase phrases[DIVE_PHRASES_MOST];
		DiveFind finds[DIVE_FINDS_MOST];
		uint8_t opened[DIVE_OPENED_MOST];
		DiveSplit splits[DIVE_SPLITS_MOST];
		uint32_t gone[DIVE_GONE_MOST];
	} u;
} DiveTraces;

typedef struct {
	uint8_t index, party;
	uint16_t tenths, place, count, best;   /* the bust sent; its place among the week's; how many busted it; the quickest */
	char best_name[ONLINE_NAME_BYTES + 1];
} DivePlace;

typedef struct {
	uint8_t kind;           /* ONLINE_FEED_* */
	char name[ONLINE_NAME_BYTES + 1];
	DiveWay way;            /* where: a flare's door */
	uint16_t cause;         /* a deletion's */
	uint8_t party;          /* a guardian down: the party's size */
} DiveFeed;

typedef struct {
	char name[ONLINE_NAME_BYTES + 1];
	uint8_t depth, act, flags;   /* flags: ONLINE_DIVERS_FIGHTING */
} DiverEntry;

typedef struct {
	uint16_t total;         /* how many dive now */
	uint8_t count;
	DiverEntry e[ONLINE_DIVERS_MOST];
} DiveDivers;

typedef struct {
	uint32_t session;
	char name[ONLINE_NAME_BYTES + 1];
	DiveWay way;            /* the fork's portal he stands on, depth 0 none */
} CrewMember;

typedef struct {
	uint8_t stage;          /* ONLINE_CREW_ASKED .. ONLINE_CREW_ALONE */
	uint32_t other;
	uint8_t value;          /* an offer's battle index */
	uint8_t count;
	CrewMember m[ONLINE_PARTY_MOST];
} CrewState;

typedef struct {
	uint32_t session;
	char name[ONLINE_NAME_BYTES + 1];
	uint8_t flags;          /* ONLINE_QUEUE_ARENA */
} QueueEntry;

typedef struct {
	uint8_t stage;          /* ONLINE_QUEUE_WAITING .. ONLINE_QUEUE_ALONE */
	DiveWay door;
	uint8_t place, count;   /* the recipient's place, 1 the leader, 0 none */
	QueueEntry e[ONLINE_PARTY_MOST];
} QueueState;

typedef struct {
	uint16_t tenths;        /* 0 none */
	char name[ONLINE_NAME_BYTES + 1];
} WeekSplit;

typedef struct {
	uint16_t week;
	WeekSplit best[4][ONLINE_PARTY_MOST];   /* [act - 1][party - 1] */
} WeekSplits;

/* A party battle's ARENA_STATE: START (its format ONLINE_ARENA_PARTY or
 * ONLINE_ARENA_LINKED, the recipient's seat, the seats' sessions), a seat gone
 * (ONLINE_ARENA_SEAT_GONE, `seat` the one gone, `other` its session) */
typedef struct {
	uint8_t stage, seat, delay, form;
	uint32_t battle, other;
	uint32_t seats[ONLINE_SEATS];
} PartyState;

/* -- built: each its size, 0 where a value is out of its bounds -- */
size_t dive_wire_enter(uint8_t out[ONLINE_DIVE_ENTER_BYTES], uint16_t week, uint32_t seed, DiveWay way);
size_t dive_wire_leave(uint8_t out[ONLINE_DIVE_LEAVE_BYTES]);
size_t dive_wire_end(uint8_t out[ONLINE_DIVE_END_BYTES], unsigned how, int x, int y, int z, unsigned cause);
size_t dive_wire_found(uint8_t out[ONLINE_DIVE_FOUND_BYTES], unsigned index);
size_t dive_wire_phrase(uint8_t out[ONLINE_DIVE_PHRASE_BYTES], unsigned phrase, int x, int y, int z);
size_t dive_wire_wave(uint8_t out[ONLINE_DIVE_WAVE_BYTES], uint32_t id);
size_t dive_wire_split(uint8_t out[ONLINE_DIVE_SPLIT_BYTES], const DiveSplit *split);
size_t dive_wire_crew(uint8_t out[ONLINE_CREW_BYTES], unsigned action, uint32_t ref, unsigned value, DiveWay way);
size_t dive_wire_queue(uint8_t out[ONLINE_QUEUE_BYTES], unsigned action, DiveWay door);
/* PARTY_DAMAGE (a linked fighter's damage to the guardian so far and his HP) or PARTY_HP (seat 0's: the guardian's HP and max) */
size_t dive_wire_party(uint8_t out[ONLINE_PARTY_BYTES], unsigned kind, uint32_t battle, unsigned first, unsigned second);

/* -- read: false (or -1) where malformed -- */
bool dive_wire_room(const uint8_t *data, size_t size, DiveRoom *out);
bool dive_wire_traces(const uint8_t *data, size_t size, DiveTraces *out);
bool dive_wire_place(const uint8_t *data, size_t size, DivePlace *out);
bool dive_wire_feed(const uint8_t *data, size_t size, DiveFeed *out);
bool dive_wire_divers(const uint8_t *data, size_t size, DiveDivers *out);
bool dive_wire_crew_state(const uint8_t *data, size_t size, CrewState *out);
bool dive_wire_queue_state(const uint8_t *data, size_t size, QueueState *out);
/* seat 0's ARENA_SEAT carrying a linked fighter's PARTY_DAMAGE: his seat, the battle, his damage and HP */
bool dive_wire_damage(const uint8_t *data, size_t size, unsigned *seat, uint32_t *battle, unsigned *damage, unsigned *hp);
bool dive_wire_party_hp(const uint8_t *data, size_t size, uint32_t *battle, unsigned *hp, unsigned *max_hp);
bool dive_wire_splits(const uint8_t *data, size_t size, WeekSplits *out);
/* a party battle's ARENA_STATE: START of a party or a linked fighter, a seat gone, or ended */
bool dive_wire_party_state(const uint8_t *data, size_t size, PartyState *out);

#endif
