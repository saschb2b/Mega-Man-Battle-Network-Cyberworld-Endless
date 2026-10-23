/* What is said and shown on a liberation's board (liberation_words.c):
 * MegaMan's and Lan's words in BN6's chat boxes, and the board's labels. */
#ifndef CW_LIBERATION_WORDS_H
#define CW_LIBERATION_WORDS_H

#include <stdbool.h>

/* The act's card's line for an act whose way holds a dark layer. */
extern const char *const lib_card_line;
/* As the board begins: what the dark layer is, its DarkHoles and par. */
const char *lib_briefing_words(int holes, int par);
/* After a liberation's battle: what it freed (`panels`, beside the one
 * fought for), whether in one turn, what it held; "" for nothing to say. */
const char *lib_battle_words(int fight, bool won, bool one_turn, int panels, int holes_left, bool guardian_back);
/* An item panel liberated: Order Points (now `op`), or a heart. */
const char *lib_item_words(bool heart, int op);
/* A BonusPanel's roulette (0 full HP, 1 Order Points, 2 a guardian
 * deleted, 3 no harm this phase). */
const char *lib_bonus_words(int outcome);
/* A barrier's key found: the sealed region's, or one across the way. */
const char *lib_key_words(bool sealed);
/* A trap panel liberated: its harm, `hp`. */
const char *lib_trap_words(int hp);
/* The sealed region's route found (docs/LIBERATION.md, Special routes). */
const char *lib_route_words(void);
/* A rest (BN5's Pass). */
const char *lib_rest_words(void);
/* The enemy's phase: `hp` taken on the board, the last by `who`. */
const char *lib_hurt_words(const char *who, int hp);
/* The Darkloid deleted in `phases` against `par`. */
const char *lib_won_words(int phases, int par);
/* The units' names (UNIT_*; the Darkloid's is his Navi's) */
const char *lib_unit_name(int kind);
/* The labels: the commands (LIB_CMD_*), the banners, the corner's */
const char *lib_command_label(int cmd);
extern const char *const lib_label_player_phase, *const lib_label_enemy_phase, *const lib_label_phase, *const lib_label_par,
	*const lib_label_op, *const lib_label_menu;
/* What MegaMan faces, for the menu's information: a panel's kind
 * (liberation_board.h's LIB_*), or a unit's name */
const char *lib_faced_label(int kind);

#endif
