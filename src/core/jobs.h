/* Jobs at home (jobs.c, docs/HOME.md piece 5, issue #88): at each visit
 * three people post a request for the act ahead, and Lan takes one or
 * none, BN6's Request BBS's rule; the one who asked settles it at the next
 * visit. Its kind is its price: a job asks what an act can give, and pays
 * by what it costs. */
#ifndef CW_JOBS_H
#define CW_JOBS_H

#include <stdbool.h>
#include <stdint.h>

/* What a job asks: battles won without losing HP; battles won in
 * JOB_QUICK_FRAMES at most; every Mystery Data of one layer opened (three
 * at least); a chip of an element handed over at home; no Mr.Prog's patch
 * until the act's guardian falls. */
enum { JOB_NONE, JOB_CLEAN, JOB_QUICK, JOB_EXPLORE, JOB_BRING, JOB_VOW, JOB_KINDS };
#define JOB_QUICK_FRAMES 600   /* 10 seconds */
#define JOB_EXPLORE_LEAST 3
/* Who posts them: AsterLand's request board, the Academy's NetBattle club,
 * the man from the lab in town. */
enum { JOB_BOARD, JOB_CLUB, JOB_LAB, JOB_ASKERS };
/* What one pays. */
enum { PAY_ZENNY, PAY_BUGFRAGS, PAY_CHIP, PAY_HPMEMORY };
/* Where one stands. */
enum { JOB_TAKEN = 1, JOB_DONE, JOB_FAILED };

typedef struct {
	uint8_t kind, asker;
	uint8_t need;        /* battles (CLEAN, QUICK); the element, ELEM_* (BRING) */
	uint8_t got;         /* battles so far */
	uint8_t state;       /* 0 an offer, JOB_TAKEN, JOB_DONE, JOB_FAILED */
	uint8_t act;         /* the act it is for (the shops' act: act + 7 a cycle) */
	uint8_t pay_kind;
	uint8_t code;        /* a chip's code, A=0 .. *=26 */
	uint16_t pay;        /* zenny, BugFrags or HPMemory; a chip's id */
	uint16_t depth;      /* the run's layer the act it is for begins on */
} Job;

/* The act a layer's job is for, as the shops price by (pacing_act and its
 * cycles). */
int jobs_act(int depth);
/* The visit's offers for the act beginning on layer `depth`, one an asker,
 * from the run's seed (the same at a CONTINUE): kinds of their own, none
 * twice. */
void jobs_offers(uint32_t seed, int depth, Job out[JOB_ASKERS]);
/* Whether job `j`, taken, is up for settling at a visit before layer
 * `depth`: its act was played. */
bool jobs_due(const Job *j, int depth);

/* A taken job's progress; each returns whether it just came done (or, for
 * a heal, broke its vow). A battle: won or not, the HP MegaMan lost in it
 * and how long it ran (frames); a guardian's own counts for none. */
bool jobs_battle(Job *j, bool won, int hp_lost, int frames);
/* A layer left: its Mystery Data, and how many of them were opened. */
bool jobs_layer_left(Job *j, int nmd, int opened);
/* A Mr.Prog's patch taken. */
bool jobs_heal(Job *j);
/* The act's guardian fell. */
bool jobs_guardian(Job *j);

/* For a test (--dev job=K,jobstate=S): `j` a request of kind K (none: 0)
 * in state S (0: taken), offered at the visit before layer `depth`, or at
 * the one before it at `home` (the act just played). */
void jobs_dev(Job *j, uint32_t seed, int depth, bool home, int kind, int state);

#endif
