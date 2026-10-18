/* A Net Square visit (docs/ONLINE.md): asked for at Lan's HP's gate, where
 * the director has just saved the run, and back to that checkpoint after.
 * One core all the way: the Square is built into the running game, and
 * the way back loads the gate's checkpoint as CONTINUE does, with no new
 * boot. While MegaMan is away the run's director is stopped and nothing is
 * saved; closing the game then keeps the gate's checkpoint. */
#ifndef CW_ONLINE_VISIT_H
#define CW_ONLINE_VISIT_H

#include <stdbool.h>
#include <stdint.h>

#include "online_snapshot.h"
#include "online_types.h"

typedef enum {
	VISIT_HOME,      /* no visit */
	VISIT_CALLING,   /* connecting from the gate: MegaMan held in Lan's HP, the director on */
	VISIT_AWAY,      /* in the Square: the director stopped */
} VisitStage;

/* How the last visit ended, for what is said at the gate after it. */
typedef enum {
	VISIT_END_NONE,
	VISIT_END_LEFT,          /* the player left the Square */
	VISIT_END_CANCELLED,     /* B while connecting */
	VISIT_END_UNREACHABLE,   /* no connection to the server */
	VISIT_END_VERSION,       /* the server runs another version of the game */
	VISIT_END_ROM,           /* ... or wants another BN6 */
	VISIT_END_NAME,          /* the server refused the name */
	VISIT_END_FULL,          /* every channel full */
	VISIT_END_BARRED,        /* banned, or not on the server's allowlist */
	VISIT_END_REMOVED,       /* a moderator removed MegaMan */
	VISIT_END_LOST,          /* the connection dropped in the Square */
	VISIT_END_BROKEN,        /* the Square could not be built */
} VisitEnd;

/* The visit to server `url` as `name`, with this server's identity: the
 * connection opened, MegaMan held at the gate until it answers. The
 * caller has saved the run's checkpoint where MegaMan stands. */
bool online_visit_begin(const char *url, const char *name, const uint8_t identity[ONLINE_IDENTITY_BYTES]);
VisitStage online_visit_stage(void);
static inline bool online_visit_active(void) { return online_visit_stage() != VISIT_HOME; }
static inline bool online_visit_away(void) { return online_visit_stage() == VISIT_AWAY; }
/* A NetBattle from the Square under way (square_arena.h): the game's core
 * held still, a frame of the battle with the pad's keys in its place, and
 * online_visit_draw draws the battle's picture alone */
bool online_visit_held(void);
void online_visit_held_frame(uint32_t keys);
/* After each frame of the core while a visit is on. */
void online_visit_update(void);
uint32_t online_visit_keys(uint32_t keys);
/* The Square over the game, or the gate's field where one is open. */
void online_visit_draw(void);
/* Back to the gate's checkpoint (a reset chord, the Square's exit). */
void online_visit_leave(VisitEnd why);
/* How the last visit ended, once: VISIT_END_NONE after. */
VisitEnd online_visit_ended(void);
/* The game closing or sent away mid-visit: the connection closed, the
 * gate's checkpoint left as it is. */
void online_visit_shutdown(void);
/* The public card of the MegaMan carried (once in the Square). */
void online_visit_profile(OnlineProfile *out);
/* The MegaMan carried, as read at the gate (the Arena's loadout); NULL
 * where none was read */
const OnlineLoadout *online_visit_loadout(void);

#endif
