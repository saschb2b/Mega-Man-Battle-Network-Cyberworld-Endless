/* The locks' words (lock_words.c), for the obstacles' talks (blockers.c). */
#ifndef CW_LOCK_WORDS_H
#define CW_LOCK_WORDS_H

const char *obstacle_stuck_words(const char *what, const char *a, const char *b);
const char *obstacle_cross_words(const char *what, const char *cross, const char *deed);
const char *obstacle_answer_words(void);

#endif
