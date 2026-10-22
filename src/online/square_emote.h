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
/* The MegaMan this one faces, the nearest within about three panels in a
 * cone of his facing (BN6's talk probe's way), 0 none: an emote's aim. */
uint32_t square_emote_faced(const OnlineSnapshot *s);
/* Emote `emote` sent, aimed at player `facing` (0 none) where it can be,
 * at the one who aimed at MegaMan first. False where the burst is spent
 * (one comes back a second). */
bool square_emote_send(int emote, uint32_t facing, uint32_t now);
/* The items the strip and the palette offer (issue #178): an emote by its
 * id, a phrase past them (SQUARE_PHRASE + its id) */
#define SQUARE_PHRASE ONLINE_EMOTE_COUNT
#define SQUARE_ITEMS (ONLINE_EMOTE_COUNT + ONLINE_PHRASE_COUNT)
/* ... item `item` sent, an emote as square_emote_send sends it, a
 * phrase said, the same burst spent */
bool square_emote_send_item(int item, uint32_t facing, uint32_t now);
/* The four favourites, by the direction held with L (0 up, 1 right, 2
 * down, 3 left): items, network.ini's `favourites` or the defaults; one
 * kept on a direction (SELECT and it on the strip), written there. */
int square_emote_favourite(int way);
void square_emote_set_favourite(int way, int item);
/* What the strip previews (emote_strip.c): the animation of BN6's
 * overworld MegaMan for emote `emote` facing `facing`, `into` frames into
 * it, and how many frames it lasts */
int square_emote_anim(int emote, int facing, int into);
int square_emote_frames(int emote);
/* The player emote `emote` would be aimed at if sent now, facing player
 * `facing`: the one who aimed at MegaMan first, 0 none or not one that
 * can be aimed */
uint32_t square_emote_aim(int emote, uint32_t facing, uint32_t now);
/* Everyone else's phrases and emotes hidden and silent (their poses,
 * sounds, words and aims; names stay), kept in network.ini. */
bool square_emote_quiet(void);
void square_emote_set_quiet(bool on);
/* MegaMan names SELECT's strip in L's chat once (issue #178): due until
 * it has been said, network.ini's `hinted` after */
bool square_emote_hint_due(void);
void square_emote_hinted(void);

#endif
