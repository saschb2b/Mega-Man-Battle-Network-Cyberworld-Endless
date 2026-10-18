/* The Arena's inputs (issue #120, docs/ONLINE_ARENA.md): both players'
 * GBA keys by frame, the only thing that crosses the link. Each client
 * runs both linked BN6 cores; a frame of the pair runs once both players'
 * keys for it and the three frames after it are known, since one linked
 * core can lead its cable partner by two frames and takes the keys of its
 * own frame clock (docs/ONLINE_FRAME.md). ROM-free: tests/test_core.c. */
#ifndef CW_ARENA_INPUT_H
#define CW_ARENA_INPUT_H

#include <stdbool.h>
#include <stdint.h>

#define ARENA_LOOKAHEAD 4      /* a frame's keys and the three after it */
#define ARENA_RING      256    /* frames held: a power of two, past the longest delay */
#define ARENA_DELAY_MIN (ARENA_LOOKAHEAD - 1)
#define ARENA_DELAY_MAX 60     /* a second: past it a link is too slow to play over */
#define ARENA_KEYS_MASK 0x3FFu /* the GBA's ten buttons */

typedef struct {
	uint16_t keys[ARENA_RING][2];
	uint32_t known[2];   /* player i's keys are known for the frames below this (each stream in order) */
	uint32_t next;       /* the next frame of the pair to run */
	unsigned delay;      /* D: a key sampled at tick t is frame t + D's */
} ArenaInput;

/* What arena_input_put made of a frame's keys */
typedef enum {
	ARENA_PUT_OK,        /* the stream's next frame, taken */
	ARENA_PUT_OLD,       /* a frame already known: left as it was (a resent message) */
	ARENA_PUT_GAP,       /* past the stream's next frame: one went missing */
	ARENA_PUT_FULL,      /* further ahead than the ring holds */
} ArenaPut;

/* An empty buffer with the delay clamped to [ARENA_DELAY_MIN,
 * ARENA_DELAY_MAX]: both players' first D frames released keys, as every
 * client starts them */
void arena_input_init(ArenaInput *in, unsigned delay);
/* player's keys for `frame`, its stream's next */
ArenaPut arena_input_put(ArenaInput *in, unsigned player, uint32_t frame, uint16_t keys);
/* both players' keys known for the next frame and the three after it */
bool arena_input_ready(const ArenaInput *in);
/* ... taken: out[k][player] the keys of frame next + k, and next moved on */
bool arena_input_take(ArenaInput *in, uint16_t out[ARENA_LOOKAHEAD][2]);

#endif
