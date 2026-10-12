/* The differential's hooks on both cores (issue #82; docs/DEVTOOLS.md, The
 * differential): each core's random draws, the battle objects it spawns
 * (every spawner of the first, third and fourth kinds), the sounds it
 * starts, its effects, setAttack, a panel reserved, an object's blink and
 * its sprite set up,
 * each logged with the frame, the caller, the family's own code that led to
 * it and the outermost caller; and the subject's draws replayed from the
 * reference's (rng=replay). BN5's hooks are on the guest's core (per-core
 * hooks, hook.h), BN6's on BN6's: the run's battles have none. Local only,
 * as src/dev/xdiff.c, which writes the events into its traces. */
#ifndef CW_XDIFF_EVENTS_H
#define CW_XDIFF_EVENTS_H

#include <stdbool.h>
#include <stdint.h>

/* An event's kind */
enum {
	XD_DRAW = 1,   /* a random number drawn: sub the routine (0 GetRNG, 1 its positive, 2 the second word's, 3 its positive) */
	XD_SPAWN,      /* a battle object spawned: sub its kind (1, 3, 4) */
	XD_SOUND,      /* a sound started (PlaySoundEffect) */
	XD_EFFECT,     /* an effect spawned (the fourth kind's type 0) */
	XD_ATTACK,     /* setAttack: sub its N */
	XD_RESERVE,    /* a panel reserved */
	XD_BLINK,      /* the blink step of an object flashing */
	XD_SPRITE,     /* an object's sprite set up (sprite_load) */
	XD_MOOD,       /* (BN5's alone) a side's mood raised as a hit lands: a the side, b what it rose by (BN5_MOOD_GAIN) */
};

/* A draw's flags */
enum {
	XD_KEPT = 1,       /* the reference's, kept for the subject's replay: ref its number + 1 */
	XD_REPLAYED = 2,   /* the subject's, the reference's draw ref - 1's word written into the RNG word first */
	XD_UNMATCHED = 4,  /* the subject's, with no draw of its key left in the reference's frame: drawn as it is */
	XD_UNMAPPED = 8,   /* the subject's, its caller neither the family's code nor a twin of BN5's: drawn as it is */
};

/* An event, ten words in the trace (src/dev/xdiff.c): kind, sub and flags;
 * the core's frame and the battle's timer before it; the caller (a
 * spawn's: its spawner's), the family's own code that led to it (the
 * innermost return address in a family's moved code, BN6's copy or BN5's
 * own; 0 none), the outermost caller (the outermost return address under
 * the object runner's); the object (a spawn's: the one spawned; a draw's,
 * a sound's, an effect's, setAttack's: the one running; a reservation's and
 * a blink's and a sprite's: its own); a and b (a draw's word before and the word drawn
 * from; a spawn's type and the object running; a sound's song; an effect's
 * id; setAttack's CurAction; a blink's flash timer and 1 where this game
 * hides the object this frame); ref (a draw's: the reference's draw it was
 * replayed from, or kept as, + 1) */
typedef struct {
	uint8_t kind, sub;
	uint16_t flags;
	uint32_t frame, t, caller, family, outer, object, a, b, ref;
} XdEvent;
#define XD_EVENT_WORDS 10

/* A BN6 routine whose body is BN5's: a draw from within it is keyed by the
 * same place in its BN5 twin (BN6's address, BN5's, its bytes), and by the
 * word BN5's draws there take where it is not BN6's (0 the first, 1 the
 * second; -1 the same): BN6's camera shakes from its second word, BN5's
 * from its first */
typedef struct { uint32_t bn6, bn5, len; int word; } XdTwin;

typedef struct XdLog XdLog;

/* The log of game 5 (the reference, the guest's core) or 6 (the subject,
 * BN6's), emptied, its hooks placed on its core: NULL where one could not
 * be. The families' code is xvirus_moved's (BN6's copy, BN5's own). */
XdLog *xd_log_start(int game);
/* The subject's draws replayed from the reference's: each keyed by the
 * family's own code that led to it (its BN5 address), else by its caller
 * (a BN6 routine's through its twin), the k-th of its key in a frame the
 * reference's k-th in the same frame by the battle's timer */
void xd_replay(XdLog *subject, const XdLog *reference, const XdTwin *twins, int ntwins);
/* Before each frame: the core's frame and the battle's timer, events
 * logged (its battle under way), draws kept and replayed (its fight seeded) */
void xd_frame(XdLog *log, uint32_t frame, uint32_t t, bool on, bool seeded);
/* The events since the last call, oldest first: how many (at most max) */
int xd_take(XdLog *log, XdEvent *out, int max);
/* What happened to its draws (the subject's: replayed, unmatched,
 * unmapped) and the events lost to a full log, in that order */
#define XD_COUNTS 4
void xd_counts(const XdLog *log, int out[XD_COUNTS]);

/* (CYBERWORLD_XDRAWS=keep|PATH: tools/xcontrol.py, the control's runs,
 * BN6's against BN6's; src/emu/battle_trace.c) After each frame of the
 * game: from a battle's first
 * frame on, BN6's draws keyed as the replay keys them, the control's code
 * (BN6's own Mettaur) at its own place and its moved copy alike, the
 * engine's by their caller; with keep, kept into the data folder's
 * xdraws.bin, else each replayed from that file of another run (its
 * path); and as its results begin, what became of them on stdout
 * ("xdraws: ..."). The run with the same draws plays the same. */
void xd_control_frame(const char *how);

/* The song a BN6 slot plays a copy of (xsong.h): its number in its own
 * game, -1 for one of BN6's own: a sound read in BN5's numbers. */
int xd_song_copy(int slot);

#endif
