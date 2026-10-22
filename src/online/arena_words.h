/* What MegaMan says of the Arena in the Net Square (arena_words.c,
 * square_arena.c, docs/VOICE.md): the Battle choice for another player's
 * MegaMan, a challenge sent and answered, the question a challenge asks,
 * a MegaMan who cannot go, and the word back in the Square, a rematch
 * asked with it. */
#ifndef CW_ARENA_WORDS_H
#define CW_ARENA_WORDS_H

#include <stdbool.h>

/* The peer menu's choice */
const char *arena_battle_option(void);
/* A challenge sent to `name`; the answer No, or the battle gone before it
 * began */
const char *arena_sent_words(const char *name);
const char *arena_declined_words(void);
const char *arena_gone_words(void);
/* ... refused by the server: `name` busy (in a NetBattle, watching one, at
 * Trade Central), no longer here, the Arena closed on this server, his
 * NetPass or ours without the Arena's stamp */
const char *arena_busy_words(const char *name);
const char *arena_not_here_words(const char *name);
const char *arena_closed_words(void);
const char *arena_no_stamp_words(void);
/* Challenged by `name`: the boxes, his MegaMan's card after them
 * (square_card_facts' pages; NULL or empty: none come), read before the
 * question (its end a '\n'; epic #167, P5) */
const char *arena_challenged_card_words(const char *name, const char *facts);
/* A challenge of ours open (protocol v13): Battle's place on his card, its
 * Call off; MegaMan's word on it; Battle on another meanwhile */
const char *arena_call_off_option(void);
const char *arena_called_off_words(const char *name);
const char *arena_asked_already_words(const char *name);
/* The folder to carry: folder k's name as BN6 names it; MegaMan's words
 * before the choice; no folder holds a chip */
const char *arena_folder_name(int k);
const char *arena_folder_words(void);
const char *arena_no_folder_words(void);
const char *arena_challenged_question(void);
/* Our MegaMan can't go into a NetBattle: a chip or a number no pair core
 * holds (the last guard, arena_loadout_check) */
const char *arena_cant_words(void);
/* Back in the Square (the rematch is asked on the result card,
 * arena_card.h): won, lost, over with no result, stopped */
const char *arena_won_words(void);
const char *arena_lost_words(void);
const char *arena_over_words(void);
const char *arena_stopped_words(void);
/* ... stopped before it began, the two games' installs apart */
const char *arena_installs_apart_words(void);

/* Team NetBattles (epic #167, square_team.c): the card's choice (a team
 * asked for, or split with `partner` himself); a team asked of `name`,
 * asked by him (the boxes, then the question), formed, turned down,
 * split; one asked while we have a partner */
const char *arena_team_option(bool partner);
const char *arena_team_sent_words(const char *name);
const char *arena_team_asked_words(const char *name);
const char *arena_team_asked_question(void);
const char *arena_team_formed_words(const char *name);
const char *arena_team_declined_words(const char *name);
const char *arena_team_split_words(const char *name);
const char *arena_team_have_words(const char *name);
/* ... a team asked of `name`, who is on one already (NOTICE 29) */
const char *arena_team_taken_words(const char *name);
/* A Battle refused for teams (NOTICE 29): `name` alone and we a team, or
 * the other way round, or `name` our own partner; a team challenge asked
 * of us (the boxes) */
const char *arena_team_lone_words(const char *name, bool we_teamed);
const char *arena_team_partner_words(const char *name);
const char *arena_team_challenged_words(const char *name, bool side_by_side);
/* A team challenge's format, asked of its sender (protocol v15, issue
 * #181): the question, then its two choices, one bout at a time (the
 * relay) or two MegaMen a side on one field */
const char *arena_team_format_words(void);
const char *arena_team_format_relay(void);
const char *arena_team_format_side(void);
/* The order, the captain's: the question and the first choice (us; the
 * second is the partner's name); the order shown, our MegaMan up first or
 * `ours`, against `theirs` */
const char *arena_team_order_words(void);
const char *arena_team_order_us(void);
const char *arena_team_first_words(bool we_lead, const char *ours, const char *theirs);
/* The match over, back in the Square: TEAM_WON, TEAM_LOST, TEAM_BROKE_OFF, TEAM_DRAWN */
const char *arena_team_over_words(unsigned result);

#endif
