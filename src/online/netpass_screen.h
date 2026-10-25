/* The NetPass on the game's screen (issue #131, docs/online/netpass.md, The
 * NetPass): over BN6's own picture, its 240 x 160, the game held under it
 * (no key reaches the core while it is up). Opened from the PET's NetPass
 * key item in a run (pet.c), by START in the Net Square (online_chat.c,
 * where BN6's PET stays shut), and by the greeter for the stamp moment:
 * the first visit to the Square after the pass became full, the new
 * stamps landing one by one, each with BN6's own jingle, then the full
 * pass. The most stamps seen in the Square is kept in the data folder's
 * netpass.ini, never in the run's save. A or B closes it; while the
 * stamps land, it lands them all at once. */
#ifndef CW_NETPASS_SCREEN_H
#define CW_NETPASS_SCREEN_H

#include <stdbool.h>

/* The pass as it stands, opened */
void netpass_screen_open(void);
bool netpass_screen_shown(void);
/* The frame's course while it is up (the stamping, A or B closing it):
 * true while it holds the game, its keys to the core none */
bool netpass_screen_update(void);
/* ... drawn over the game's picture (P.core_x, P.core_y) while it is up */
void netpass_screen_draw(void);
/* The pass's strip as it stands, over the top of the game's picture (the
 * gate's question under it, docs/world/home/square-gate.md) */
void netpass_screen_strip(void);

/* The stamp moment: due where the pass is full and fewer of its stamps
 * have been seen in the Square (netpass.ini); begun, the pass opens with
 * the new ones landing and the file says all of them seen */
bool netpass_stamp_due(void);
void netpass_stamp_begin(void);

#endif
