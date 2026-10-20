/* The Arena's inputs (issue #120, docs/ONLINE_ARENA.md): both players'
 * GBA keys by frame, the only thing that crosses the link. Each client
 * runs both linked BN6 cores; a frame of the pair needs both players' keys
 * for it and the frame after it, since one linked core crosses into its
 * next frame a little before its partner and takes the keys of its own
 * frame clock (docs/ONLINE_ARENA.md, Rollback: the two cores within a few
 * thousand cycles of each other). With a rollback window (issue #128) a
 * frame may run before the other player's keys for it have come, each one
 * missing guessed as his last held; a key that comes otherwise than
 * guessed says the frame to roll back to. ROM-free: tests/test_arena.c. */
#ifndef CW_ARENA_INPUT_H
#define CW_ARENA_INPUT_H

#include <stdbool.h>
#include <stdint.h>

#define ARENA_LOOKAHEAD 2      /* a frame's keys and the one after it */
#define ARENA_RING      256    /* frames held: a power of two, past the longest delay */
#define ARENA_DELAY_MIN (ARENA_LOOKAHEAD - 1)
#define ARENA_DELAY_MAX 60     /* a second: past it a link is too slow to play over */
#define ARENA_KEYS_MASK 0x3FFu /* the GBA's ten buttons */
#define ARENA_NO_FRAME  UINT32_MAX

typedef struct {
	uint16_t keys[ARENA_RING][2];
	uint32_t known[2];   /* player i's keys are known for the frames below this (each stream in order) */
	uint32_t guessed[2]; /* ... and guessed (his last known held) from there up to below this */
	uint32_t wrong;      /* the first frame whose key came otherwise than guessed: ARENA_NO_FRAME none */
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
/* player's keys for `frame`, its stream's next (where it was guessed
 * otherwise, `wrong` says so) */
ArenaPut arena_input_put(ArenaInput *in, unsigned player, uint32_t frame, uint16_t keys);
/* the frames below this have both players' keys known for them and the
 * one after: whatever ran them is the truth, never run again */
uint32_t arena_input_settled(const ArenaInput *in);
/* the next frame may run: at most `window` frames on a guess (0: its keys
 * and the one after all known, the delay alone) */
bool arena_input_ready(const ArenaInput *in, unsigned window);
/* ... taken: out[k][player] the keys of frame next + k, those not yet come
 * guessed, and next moved on */
bool arena_input_take(ArenaInput *in, unsigned window, uint16_t out[ARENA_LOOKAHEAD][2]);
/* the first frame that ran on a key guessed wrong, to roll back to:
 * ARENA_NO_FRAME none */
uint32_t arena_input_wrong(const ArenaInput *in);
/* the frames from `frame` on to run again: next back there, the keys not
 * yet come guessed anew */
void arena_input_rewind(ArenaInput *in, uint32_t frame);

#endif
