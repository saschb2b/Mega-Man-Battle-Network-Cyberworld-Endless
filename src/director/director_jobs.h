/* Jobs at home, the director's part (director_jobs.c, docs/HOME.md piece
 * 5): the askers' talk at a visit, the job taken and settled, and its
 * course in the Net. */
#ifndef CW_DIRECTOR_JOBS_H
#define CW_DIRECTOR_JOBS_H

#include <stdbool.h>

/* Before home's places are installed: what each asker says at this visit;
 * after: their scripts' flags as the run's job has them. */
void home_jobs_visit(void);
void home_jobs_flags(void);
/* One of the jobs' flags set at home (EV_JOB): a request taken, the held
 * one settled. */
void home_jobs_flag(int flag);
/* A battle begins (MegaMan's HP), runs (its frames), and ends: won or
 * not, MegaMan's HP after it, its frames (-1: as counted). */
void home_jobs_battle_start(void);
void home_jobs_battle_frame(void);
void home_jobs_battle_end(bool won, int hp, int frames);
/* A layer left: its Mystery Data opened or not. */
void home_jobs_layer_left(void);
/* Each frame on a layer: a Mr.Prog's patch, the guardian's fall. */
void home_jobs_watch(void);
/* MegaMan's word on the job, once a word can be said. */
void home_jobs_words(void);
/* L at home: `words`, and a word on the held request where it is up for
 * settling. */
const char *home_jobs_status(const char *words);

#endif
