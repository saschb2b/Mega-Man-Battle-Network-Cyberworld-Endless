/* The Arena from the Net Square (issue #120, docs/ONLINE_ARENA.md): a
 * Practice NetBattle asked of another player's MegaMan in BN6's chat boxes
 * (online_chat.c's Battle), his player asked in one ("... wants a Practice
 * NetBattle! Take it?", the cursor on No), and both games then off the
 * Square's map into the Arena's two linked cores over the network's link
 * (arena_link_net.h). The Square's core holds still meanwhile, its
 * connection kept, MegaMan standing where he stood on the server's view;
 * after the battle he is back there, and says how it went. A battle with a
 * result asks Lan for another with the same player on its result card
 * (the cursor on No): both games' Yes, the same rules and folder, and they
 * go again. Not
 * offered where a frame of the pair does not fit a 60th of a second
 * (CW_ARENA: the New 3DS). */
#ifndef CW_SQUARE_ARENA_H
#define CW_SQUARE_ARENA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "online_types.h"

/* the Battle choice and watching offered at all on this system (CW_ARENA:
 * not the New 3DS's); on a card, where both games' NetPass holds its Arena
 * stamp too (online_chat.c, netpass.h) */
bool square_arena_offered(void);
/* The last battle fought from the Square in this game, as its referee
 * tells it. */
typedef struct {
	unsigned result;      /* 1 won, 2 lost, 3 over otherwise, 4 broken off (a stop: neither won nor lost), 0 none */
	const char *name;     /* the other's, NULL none */
	unsigned won, lost;   /* the bouts with him in a row, this one among them */
	unsigned bouts;       /* every battle fought from the Square (a new one changes it) */
} SquareBout;
const SquareBout *square_arena_last(void);
/* As MegaMan arrives in the Square, and as he leaves it: any battle left */
void square_arena_reset(void);
void square_arena_end(void);
/* Battle chosen for `peer`, my latency in ms */
void square_arena_challenge(const OnlinePeer *peer, uint32_t latency_ms);
/* An Arena message come in (ARENA_STATE and the relayed three) */
void square_arena_mail(const uint8_t *data, size_t size);
/* A NOTICE come (its serial and code): true where it refuses this game's
 * challenge, which MegaMan says in the chat box (or the card shows), the
 * corner's notice not shown */
bool square_arena_notice(uint32_t serial, uint16_t code);
/* After each frame in the Square: the question asked, the word said (a
 * Team NetBattle's too, square_team.h; as reset and end take the team's) */
void square_arena_update(const OnlineSnapshot *s);
/* After each frame of a battle or a watch held over the Square: a Team
 * NetBattle's next bout gone into, the others' bout watched */
void square_arena_held_frame(void);
/* Team NetBattles (square_team.h): a bout of a team match gone into (its
 * ARENA battle's id, the other fighter, the HP my MegaMan carries, 0
 * whole), false where something is still under way here; the team
 * challenge's own state dropped once the match begins; nothing on the
 * picture or asked; my MegaMan's HP at the last battle's end */
bool square_arena_bout(uint32_t bout, uint32_t other, const char *name, uint16_t carry);
/* A player's name by his session, as the Square last listed him (NULL gone) */
const char *square_arena_name_of(uint32_t id);
void square_arena_team_begun(void);
bool square_arena_idle(void);
uint16_t square_arena_last_hp(void);
/* The battle under way (the Square's core held still): a frame of it
 * with the pad's keys, and its picture, instead of the Square's */
bool square_arena_on(void);
void square_arena_frame(uint32_t keys);
void square_arena_draw(void);

#endif
