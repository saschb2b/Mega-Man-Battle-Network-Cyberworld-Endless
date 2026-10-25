/* The team pick's words (liberation_pick_text.c): the screen's name, its
 * one action, the labels and what the cursor's Navi brings, in the plain
 * style of the game's menus (docs/voice/rules.md), apart from the logic that
 * says when. The Navis' and their forms' names are liberation_team.c's,
 * the commands' liberation_words.c's. */
#ifndef CW_LIBERATION_PICK_TEXT_H
#define CW_LIBERATION_PICK_TEXT_H

#include <stddef.h>

typedef enum {
	PW_TITLE,        /* the band's name */
	PW_GO,           /* the one action, at the list's foot */
	PW_COMMAND,      /* the command's box label */
	PW_BATTLE,       /* the battle form's line's label */
	PW_STONEBODY,    /* KnightMan's command: none but his StoneBody */
	PW_WEAKNESS,     /* the line over his battle form's weakness */
	PW_NONE,         /* ... none */
	PW_COUNT
} PickWord;
const char *lib_pick_word(PickWord w);

/* The band's count: `picked` of `places` ("1/2") */
const char *lib_pick_places(char *buf, size_t n, int picked, int places);
/* The Navi's HP ("HP 350") */
const char *lib_pick_hp(char *buf, size_t n, int hp);
/* What command `command` (liberation_board.h's LIB_DO_*) costs, in small
 * capitals for its tag: a battle, none, or always on */
const char *lib_pick_command_note(int command);

#endif
