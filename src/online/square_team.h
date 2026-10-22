/* Team NetBattles in the Net Square (epic #167, docs/ONLINE_ARENA.md,
 * Team NetBattles): a relay of today's NetBattle between two teams of two.
 * "Team up" on another player's card asks him (his MegaMan asks Lan, the
 * cursor on No); on a partner's card it reads "Split up". A Battle between
 * players of two teams is a team challenge (square_arena.c). Its Yes asks
 * each team's captain, the one who asked to team up, who goes first, blind
 * (the server shows both orders once both are in). Each bout is the
 * Arena's NetBattle as ever: its two fighters' games go in once their last
 * screen is closed, the winner carrying the HP he has left (written into
 * the battle on both GBAs, arena_boot.c), and the others' games watch it
 * from the stands, unasked (square_watch.c). The result card shows the
 * MegaMen standing and who is next, and at the match's end the team's
 * result; MegaMan says it back in the Square. Nothing is won or lost. */
#ifndef CW_SQUARE_TEAM_H
#define CW_SQUARE_TEAM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "online_types.h"

/* As MegaMan arrives in the Square, and as he leaves it (the team split) */
void square_team_reset(void);
void square_team_end(void);
/* TEAM_STATE come in */
void square_team_mail(const uint8_t *data, size_t size);
/* The card's choice for `peer`: its label, and chosen */
const char *square_team_option(const OnlinePeer *peer);
void square_team_chosen(const OnlinePeer *peer);
/* A NOTICE come (its serial and code): true where it refuses this game's
 * ask to team up, which MegaMan says in the chat box, the corner's notice
 * not shown */
bool square_team_notice(uint32_t serial, uint16_t code);
/* My partner's session, 0 none */
uint32_t square_team_partner(void);
/* In a team match (a team challenge answered, a bout, the order) */
bool square_team_on(void);
/* After each frame in the Square (`held` false: questions asked, words
 * said) or of a battle or a watch held over it (the next bout gone into,
 * a bout watched) */
void square_team_update(const OnlineSnapshot *s, bool held);

/* A team challenge's format, asked of Lan as it goes (protocol v15,
 * issue #181): the question opened (false where a chat is); as it closes,
 * ONLINE_TEAM_RELAY or ONLINE_TEAM_SIDE_BY_SIDE, -1 none (B) */
bool square_team_format_ask(void);
int square_team_format_picked(void);
/* A player of the match, his name as kept (one gone keeps his); NULL none */
const char *square_team_name(uint32_t id);

/* What the result card shows of a team match */
typedef struct {
	bool on;                       /* a team bout's card */
	bool side_by_side;             /* ... of a match side by side: no standing, no next */
	unsigned standing[2];          /* MegaMen standing: ours, theirs */
	const char *next[2];           /* the next bout's two (NULL: the match over) */
	unsigned result;               /* the match's end, TEAM_WON..TEAM_DRAWN; 0 not yet */
} SquareTeamCard;
void square_team_card(SquareTeamCard *out);

#endif
