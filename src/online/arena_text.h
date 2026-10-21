/* The Arena's PET text (arena_text.c): the way in's panel and the result
 * card (arena_card.c), the stall's note (arena_view.c) and a stop's line,
 * each in the PET's words, never a character's */
#ifndef CW_ARENA_TEXT_H
#define CW_ARENA_TEXT_H

/* The panel's and the card's headings and labels */
extern const char arena_title[], arena_practice[], arena_vs[], arena_hp[], arena_cross[], arena_soul[], arena_none[];
extern const char arena_set[], arena_time[], arena_hp_left[], arena_chips_used[], arena_you[], arena_other[];
/* Whose wait the way in is: the other's MegaMan on its way, a PET
 * linking, a PET ready */
extern const char arena_coming[], arena_linking[], arena_ready[];
/* The card's headline: won, lost, over otherwise, broken off, stopped
 * before it began */
extern const char arena_won[], arena_lost[], arena_ended[], arena_broke[], arena_stopped[];
/* The rematch on the card: its two answers */
extern const char arena_yes[], arena_no[];
/* The card's keys: the rematch's, back alone (to the Square, to the title) */
extern const char arena_keys_ask[], arena_keys_back[], arena_back[];
/* The stall's way out, once its bar has run down */
extern const char arena_leave_key[];
/* A Team NetBattle (epic #167): the band's place, the MegaMen standing,
 * the match's end (TEAM_WON, TEAM_LOST, TEAM_BROKE_OFF, TEAM_DRAWN) */
extern const char arena_team[], arena_standing[];
const char *arena_team_end(unsigned result);
/* ... the next bout's two, or the order (bout 1) */
const char *arena_team_next(const char *ours, const char *theirs);

/* "VS NAME" */
const char *arena_vs_name(const char *name);
/* The set's running score, mine first: "2 - 1" */
const char *arena_score(unsigned won, unsigned lost);
/* A carried MegaMan's Crosses or Souls (bit masks), their names in a row */
const char *arena_crosses_named(unsigned mask);
const char *arena_souls_named(unsigned mask);
/* The rematch asked, and the other's answer as it comes: wants another,
 * my Yes waiting on his, gone or not this time */
const char *arena_again_asked(const char *name);
const char *arena_again_wanted(const char *name);
const char *arena_again_waiting(const char *name);
const char *arena_again_gone(const char *name);
/* The stall's note: whose PET the battle waits for */
const char *arena_waiting_for(const char *name);

/* Why a NetBattle stopped, the PET's line on the card (the log keeps the
 * cause in full) */
typedef enum {
	ARENA_STOP_ERROR,       /* the link's or the cable's: a communication error */
	ARENA_STOP_APART,       /* the two games' battles went apart */
	ARENA_STOP_LEFT,        /* the other left */
	ARENA_STOP_WE_LEFT,     /* we did, from the stall's note */
	ARENA_STOP_SILENT,      /* the other's PET stopped answering: the stall's end */
	ARENA_STOP_LOST,        /* the connection to the Square lost */
	ARENA_STOP_NOT_READY,   /* the other's PET never got ready */
	ARENA_STOP_NEVER_CAME,  /* the other's MegaMan never came */
	ARENA_STOP_CANT,        /* the other's MegaMan can't go */
	ARENA_STOP_INSTALLS,    /* the two PETs' programs apart (BN5's installs) */
	ARENA_STOP_MEMORY,      /* no room for the two PETs here */
} ArenaStop;
const char *arena_stop_line(ArenaStop why, const char *name);

#endif
