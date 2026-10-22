/* The Net Square's chats in BN6's chat boxes (online_chat.c): L's look
 * around and the quiet, its chord's favourites, SELECT's emote strip,
 * another player's card and what to do with him, the way home, the
 * channel keeper. */
#ifndef CW_ONLINE_CHAT_H
#define CW_ONLINE_CHAT_H

#include <stdbool.h>
#include <stdint.h>

#include "online_types.h"

/* As MegaMan arrives in the Square. */
void online_chat_reset(void);
/* After each frame there, with the snapshot shown (the peers placed by
 * online_square_peers: the emotes' poses written over them): true once
 * the player chose to head home. */
bool online_chat_update(const OnlineSnapshot *s);
/* The pad's keys before BN6 takes them: L, R and A on a peer opening a
 * chat, START the NetPass and SELECT the emote strip, kept from BN6's PET. */
uint32_t online_chat_keys(uint32_t keys);
/* A player's card as it came (INSPECT, each peer's asked once), NULL not
 * yet: the question of his challenge reads it (epic #167, P5) */
const OnlineProfile *online_chat_card(uint32_t id);
/* Emote `emote` sent as the strip sends it, at the MegaMan faced where it
 * can be aimed (the dev step `emote K`, docs/DEVTOOLS.md) */
bool online_chat_emote(int emote);
/* The player whose card is open in the chat box, 0 none: the link's bars
 * shown beside it (square_ask.h) */
uint32_t online_chat_card_open(void);

#endif
