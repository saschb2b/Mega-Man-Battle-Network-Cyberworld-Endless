/* Fast forward (docs/GUIDE.md, Fast forward): the game's own frames played
 * two to four to a frame shown where nothing is decided against the clock,
 * the walk, the chats, the fades and a battle's opening and close; never
 * in the fight, from its first Custom screen to its last deletion, nor on
 * BN6's menus or online. A press of its button (BTN_FAST) turns it on or
 * off; held, it is the other way while held. The rules alone, tested
 * without SDL (tests/test_core.c); fast_forward_game.c runs them on BN6. */
#ifndef CW_FAST_FORWARD_H
#define CW_FAST_FORWARD_H

#include <stdbool.h>

/* settings.ini's `fast_forward` (Options): 1 off, else the game's frames
 * to one shown at most */
#define FAST_OFF 1
#define FAST_MOST 4
#define FAST_DEFAULT 3
void fast_speed_set(int speed);
int fast_speed(void);
/* "off", "2x" ... and back (FAST_DEFAULT for a word it does not know) */
const char *fast_speed_word(int speed);
int fast_speed_read(const char *word);
/* Options' Left and Right: off, 2x, 3x, 4x and round */
int fast_speed_next(int speed, int dir);

/* The button, a frame at a time: a press turns fast forward on or off,
 * and one held past FAST_HOLD frames turns it back as it is let go */
#define FAST_HOLD 20
typedef struct { bool on; int held; } FastButton;
void fast_button(FastButton *b, bool down);

/* Where the game is, as BN6's state says (fast_forward_game.c reads it) */
typedef struct {
	bool run;                 /* the run's own game on the screen: a layer, the town, Lan's HP */
	bool apart;               /* online, the PET's Jukebox, the dev menu: the run's frames are not the game's */
	int toolkit, sub, phase;  /* BN6_TOOLKIT's mode, BN6_GAMESTATE's sub-mode, BN6_BATTLE_PHASE */
} FastScene;
bool fast_scene_allows(const FastScene *s);

/* Whether a batch that has played `played` frames, `spent` of the update's
 * time on them (any unit, `budget` the most), plays another: up to the
 * speed, and while the last one's time still fits */
bool fast_batch_more(int speed, int played, double spent, double last, double budget);

#endif
