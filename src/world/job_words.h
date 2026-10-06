/* The askers' talk at home (job_words.c, docs/HOME.md piece 5; docs/
 * VOICE.md): a request offered, taken, waited on and settled, and what
 * MegaMan says of one in the Net. */
#ifndef CW_JOB_WORDS_H
#define CW_JOB_WORDS_H

#include <stdbool.h>

#include "jobs.h"
#include "text.h"

/* The event flags their scripts set: request k taken (Yes), a request
 * held (taken and not yet settled), the held one settled (paid, handed
 * over, or let go). The director keeps them at each visit and acts on
 * them as they are set. */
#define JOB_TAKE_FLAG    0x1460   /* + asker, JOB_ASKERS of them */
#define JOB_HELD_FLAG    0x1463
#define JOB_SETTLED_FLAG 0x146A

/* What an asker says at this visit, set by the director before home's
 * places are installed: nothing of a job, a request to offer, the one
 * held from them to wait on or to settle. */
enum { JOB_TALK_NONE, JOB_TALK_OFFER, JOB_TALK_WAIT, JOB_TALK_SETTLE };
#define JOB_BRING_MAX 8
typedef struct {
	int mode;
	Job job;
	/* a request for a chip: those of its element MegaMan holds (Pack or
	 * Folder, the script checking the Pack as Lan talks) */
	int nbring, bring_chip[JOB_BRING_MAX], bring_code[JOB_BRING_MAX];
	/* the last run ended holding their request: their word on it first */
	bool lost;
} JobTalk;
extern JobTalk job_talk[JOB_ASKERS];

/* Asker `asker`'s script at this visit, spoken with `face`; `plain` its
 * words where it has no job to talk of. */
int job_script(TextArchive *t, int asker, int face, const char *plain);
/* What AsterLand's request board reads (A at it): the board's post. */
const char *job_board_words(void);

/* MegaMan at home on the held request, now up for settling (`job`): its
 * reward waiting, or the asker to tell, and where they are. */
const char *job_due_words(const Job *job);
/* MegaMan in the Net: a request done, a vow broken, the vow's reminder. */
const char *job_done_words(int asker);
const char *job_broken_words(void);
const char *job_vow_words(void);

#endif
