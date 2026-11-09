/* Oran Isle as a stop on the way into an act (docs/world/bn5-town.md): what
 * its three people hold (oran.c) and say (oran_words.c). */
#ifndef CW_ORAN_H
#define CW_ORAN_H

#include <stdbool.h>
#include <stdint.h>

/* Event flags of the stop's talk, which the director reads */
#define ORAN_STALL_FLAG 0x147C   /* the stall's one chip bought */
#define ORAN_JOB_FLAG   0x147D   /* the island's job taken */
#define ORAN_JOB_BATTLES 2       /* its battles won without a scratch (JOB_CLEAN) */

/* The stall's three chips (BN5's own, of the registry), their codes and
 * price (zenny), and their names */
typedef struct {
	int chip[3], code[3], price[3];
	char name[3][20];
} OranStall;

/* The act the stop leads into: its area, its guardian, the layer it
 * begins on and the run's seed (the stall and the job the same at a
 * CONTINUE); set before town.c installs the island. */
void oran_prepare(int biome, int navi, int depth, uint32_t seed);
/* Folk `i`'s talk (TownLines' script): the guardian's word, the stall,
 * the job; -1 none. */
int oran_script(void *text, int i, int face);

/* The lines (oran_words.c) */
const char *oran_word_words(const char *navi, const char *area, const char *rumor);
const char *oran_stall_lead(void);
const char *oran_stall_ask(const char *chip, char code, int price);
const char *oran_stall_broke(void);
const char *oran_stall_sold(void);
const char *oran_stall_leave(void);
const char *oran_job_offer(int battles);
const char *oran_job_question(void);
const char *oran_job_taken(void);
const char *oran_job_held(void);
const char *oran_job_no(void);
/* MegaMan on the portal (director_stop.c): the question, and the words on
 * arriving at the dock */
const char *oran_portal_words(void);
const char *oran_portal_question(void);
const char *oran_arrival_words(void);

#endif
