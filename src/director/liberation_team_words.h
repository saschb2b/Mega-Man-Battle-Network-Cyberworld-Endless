/* What a liberation's teammates say (liberation_team_words.c), each in
 * his own habit (docs/voice/rules.md, The guardians: BN5's Navis), said with
 * his own face: as the board begins, after his battle, at a rest, his
 * command. */
#ifndef CW_LIBERATION_TEAM_WORDS_H
#define CW_LIBERATION_TEAM_WORDS_H

#include <stdbool.h>

/* Navi `navi`'s (liberation_board.h's MATE_*) line as he joins the board */
const char *lib_mate_join_words(int navi);
/* ... after his battle: won (in its first turn), or not */
const char *lib_mate_battle_words(int navi, bool won, bool one_turn);
/* ... resting (BN5's Pass) */
const char *lib_mate_rest_words(int navi);
/* ... his command with no battle (NumberCheck's finds, SneakAttack,
 * LifeMelody's teammates sung for: `n`; MagnetBarrier, BombLiberation) */
const char *lib_mate_command_words(int navi, int n);
/* A Soul Navi's form in battle, BN5's Soul's name ("ColonelSoul") */
const char *lib_soul_form_name(int navi);

#endif
