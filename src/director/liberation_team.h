/* A liberation's team (docs/LIBERATION.md, The team): MegaMan and the Navis
 * whose BN5 Souls the run holds, each with his own HP (BN5's table), his
 * place on the board, his command and his turn a phase, as BN5's team
 * plays (romlab, mission 2): R passes play to the next Navi yet to act,
 * each Navi's action passes it on by itself, and the Navi in play walks
 * the board as BN6 walks its player, in his own body (player_body.h),
 * the others standing as the layer's objects. A teammate's battle is
 * MegaMan in that Navi's Soul with that Navi's HP; deleted there, the
 * Navi leaves the board, as BN5's do. Its words in liberation_team_words.c. */
#ifndef CW_LIBERATION_TEAM_H
#define CW_LIBERATION_TEAM_H

#include <stdbool.h>

#include "liberation_board.h"

/* The Navis the run holds for a board (MegaMan not among them), in their
 * order: the BN5 Souls whose bodies stand in BN6 (BN5's ROM read, the
 * Souls in BN6's engine), then Gregar's Cross Navis (the Cross brought,
 * else those found as an earlier act's guardian's: the run's state alone,
 * which a checkpoint's rebuild reads as the layer was made); into navi,
 * how many */
int lib_team_held(uint8_t navi[LIB_HELD]);
/* A held Navi (MATE_*): his name; his HP on a board of act `act` (a Soul's
 * BN5's for that stage, BN5_LIB_NAVI_HP; a Cross Navi's the median of
 * BN5's six there); the form his battles are fought in ("ColonelSoul",
 * "HeatCross") and the element it is weak to (NULL none) */
const char *lib_navi_name(int navi);
int lib_navi_hp(int navi, int act);
const char *lib_navi_form(int navi);
const char *lib_navi_weakness(int navi);
/* The team a dark layer of act `act` (0-based) is made for: MegaMan and
 * the Navis the run holds, as many as BN5's team of that stage holds, the
 * layer's seed choosing where more are held (the player picks on arrival:
 * lib_team_pick); every one held kept in `held` for the command rule.
 * MegaMan alone without them. Their sprites copied in as it is made. */
void lib_team_make(LibTeam *t, int act);
/* The team picked on arrival (liberation_pick.c): MegaMan and navi[0 ..
 * n - 1] in the board's places, their HP the stage's (the board's par the
 * planner's with them: lib_plan_par; their bodies lib_team_dress's) */
void lib_team_pick(const uint8_t *navi, int n);
/* The team's bodies on the board's objects as the team stands (a pick's,
 * a CONTINUE's): their sprites copied in and the objects' scripts naming
 * them (`scripts`: LayerObjs' lib_body, each object's sprite byte), for
 * the map's next entry */
void lib_team_dress(const uint32_t *scripts);
/* ... the same less its last teammate (a board the whole team finds none
 * for tries a smaller one): false where MegaMan is alone */
bool lib_team_shrink(LibTeam *t);
/* The board's team objects among the layer's (OBJ_UNIT, after the units'),
 * and the board begun: each Navi's HP, place, MegaMan in play */
void lib_team_objects(void);
void lib_team_begin(void);

/* The Navi in play (0 MegaMan), and Navi k's name, face, command (LIB_DO_*) */
int lib_in_play(void);
const char *lib_mate_name(int k);
int lib_mate_face(int k);
int lib_mate_command(int k);
/* What Navi k says, in his own habit (liberation_team_words.c): as he
 * first comes into play, after his battle (won, in its first turn, or
 * not), resting, his command with no battle (`n` its finds or teammates) */
enum { LIB_SAYS_JOIN, LIB_SAYS_WON, LIB_SAYS_ONE_TURN, LIB_SAYS_FAILED, LIB_SAYS_REST, LIB_SAYS_COMMAND };
const char *lib_mate_says(int k, int what, int n);
/* Navi k's HP and max (MegaMan's his own, NaviStats'), set (1 at least, his max at most) */
int lib_mate_hp(int k);
int lib_mate_max(int k);
void lib_mate_hp_set(int k, int hp);
/* Navi k's place in the world (the player's where he is in play) */
void lib_mate_world(int k, int *wx, int *wy);
/* The Navi nearest panel (x, y) still on the board (BN5's guardians go for
 * the closer first), -1 none */
int lib_mate_nearest(int x, int y);

/* A new player's phase: no Navi has acted, none sung for */
void lib_team_phase(void);
/* The Navi in play has acted: the next yet to act after him, -1 none (the
 * Darkloids' phase comes) */
int lib_team_next(void);
/* R: the next yet to act after the Navi in play, -1 none other */
int lib_team_other(void);
/* A frame of play passing to Navi `k`: BN5's camera to him, his body the
 * player's, the camera following him; true once he is in play */
bool lib_team_switch_frame(int k);
/* After a CONTINUE (liberation_save.c): the player's body the Navi in play's */
void lib_team_resume(void);

/* A battle the Navi in play fights: a teammate's with his HP as MegaMan's
 * and his Soul armed (liberation_battle.h); and its end: his HP his own
 * again, MegaMan's too; true where the teammate was deleted (he leaves the
 * board) */
void lib_team_fight_begin(void);
bool lib_team_fight_end(void);

#endif
