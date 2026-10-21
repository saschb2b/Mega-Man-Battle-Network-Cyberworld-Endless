/* The Net Square's emotes performed (square_emote.c, docs/ONLINE.md,
 * Emotes; issue #176): each emote is MegaMan's own BN6 pose and a sound of
 * BN6's, a peer's played by his puppet's animation byte, MegaMan's own
 * through BN6's text command F8 01, on every screen alike from the
 * server's beat; MegaMan never moved by one. An emote aimed at the MegaMan
 * he faces, the one aimed at him answered with one press, a joint "!!"
 * and BN6's sparkle when it is; a burst of phrases and emotes, then one a
 * second; and the quiet that mutes everyone else's. */
#ifndef CW_SQUARE_EMOTE_H
#define CW_SQUARE_EMOTE_H

#include <stdbool.h>
#include <stdint.h>

#include "online_types.h"

/* As MegaMan arrives in the Square: the favourites and the quiet read. */
void square_emote_reset(void);
/* After the peers are placed each frame (online_square_peers): each
 * phrase's and emote's event noted, each emote begun on its beat, the
 * poses written over the peers' puppets, MegaMan's own played, the
 * sounds and the "!!" of an answer. */
void square_emote_frame(const OnlineSnapshot *shown, uint32_t now);
/* MegaMan's own emote under way: the keys held from BN6 meanwhile. */
bool square_emote_busy(void);
/* The words over player `id`'s head for the emote he performs now, where
 * his pose cannot carry it alone (online_emote_captions), NULL none. */
const char *square_emote_caption(uint32_t id, uint32_t now);
/* The player whose emote aimed at MegaMan waits for an answer, 0 none:
 * the next emote that can be aimed answers him. */
uint32_t square_emote_invited(uint32_t now);
/* Emote `emote` sent, aimed at player `facing` (0 none) where it can be,
 * at the one who aimed at MegaMan first; phrase `phrase` said. False
 * where the burst is spent (one comes back a second). */
bool square_emote_send(int emote, uint32_t facing, uint32_t now);
bool square_emote_say(int phrase, uint32_t now);
/* The four favourites, by the direction held with L (0 up, 1 right, 2
 * down, 3 left), network.ini's `favourites` or the defaults. */
int square_emote_favourite(int way);
/* Everyone else's phrases and emotes hidden and silent (their poses,
 * sounds, words and aims; names stay), kept in network.ini. */
bool square_emote_quiet(void);
void square_emote_set_quiet(bool on);

#endif
