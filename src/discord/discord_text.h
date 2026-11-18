/* What Discord shows of the game (discord_text.c): the lines composed
 * from the run's facts, which discord_game.c reads from the game, so that
 * the words read without it; and settings.ini's note on its line
 * (Options' line has its words in options_text.c, with the others'). */
#ifndef CW_DISCORD_TEXT_H
#define CW_DISCORD_TEXT_H

#include <stdbool.h>

#include "discord.h"

/* Where the player is: a screen before the game (the ROMs, SAVES, the
 * intro), the title, the run's setup, a finished run's summary, a
 * layer, home between acts, the Net Square */
typedef enum { DISCORD_MENUS, DISCORD_TITLE, DISCORD_SETUP, DISCORD_SUMMARY, DISCORD_LAYER, DISCORD_HOME, DISCORD_SQUARE } DiscordPlace;
/* ... and what MegaMan does there */
typedef enum { DISCORD_WALKING, DISCORD_BATTLE, DISCORD_GUARDIAN, DISCORD_DUEL, DISCORD_LAB, DISCORD_NETBATTLE } DiscordDoing;

typedef struct {
	DiscordPlace place;
	DiscordDoing doing;
	int layer;            /* the layer MegaMan is on; at home, the next; on the summary, the one reached */
	int last;             /* the short net's last layer, 0 on the endless net */
	int threat;           /* the run's threat rung, 0 none */
	const char *area;     /* the layer's area: "Sky Area" */
	const char *home;     /* at home, where: "ACDC Town", "Lan's HP" */
	const char *foe;      /* the guardian fought: "ElecMan" */
	bool won, new_best;   /* the summary's run won; deeper than any before */
	int best, runs;       /* the profile's deepest layer, its runs finished */
	const char *version;  /* the build's */
} DiscordFacts;

/* The activity's lines for `f` (its start left as it was) */
void discord_words(const DiscordFacts *f, DiscordActivity *a);

/* settings.ini's comment over the discord line */
const char *discord_setting_note(void);

#endif
