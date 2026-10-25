/* What is said at the Net Square's gate in Lan's HP (gate_words.c,
 * docs/world/home/square-gate.md, docs/online/, docs/voice/rules.md): its keeper, a
 * Mr.Prog, and MegaMan. */
#ifndef CW_GATE_WORDS_H
#define CW_GATE_WORDS_H

#include <stdbool.h>

#include "online_visit.h"

/* The keeper's options when talked to (ta_choose's three). */
#define GATE_OPTIONS 3
extern const char *const gate_options[GATE_OPTIONS];

/* The keeper talked to: who he is, where the gate leads (the official
 * Square or another server's, never its address), then his question over
 * the options. */
const char *gate_keeper_hello(bool official);
const char *gate_keeper_ask(void);
/* ... the official Square picked while the gate leads elsewhere: his
 * question first (talk_ask's, the cursor on No); while it leads there
 * already, his word on it */
const char *gate_keeper_official_ask(void);
const char *gate_keeper_official_question(void);
const char *gate_keeper_official_already(void);
/* ... and his answers: the gate set to the official Square; B. */
const char *gate_keeper_official(void);
const char *gate_keeper_bye(void);
/* ... the gate set to another server's address; the name `name` saved. */
const char *gate_keeper_set(void);
const char *gate_keeper_named(const char *name);
/* The gate stepped on with no name yet: the keeper asks for one. */
const char *gate_keeper_name_first(void);
/* The first time the gate leads to a server: what that server sees, and
 * his question (talk_ask's). */
const char *gate_keeper_privacy(void);
const char *gate_keeper_privacy_question(void);
/* MegaMan's question on the gate, the way known. */
const char *gate_question(void);
/* The checkpoint before a visit not written: MegaMan stays (the game's
 * own note says why, never a character: docs/voice/rules.md, What nobody
 * says). */
const char *gate_save_failed(void);
/* How a visit ended (online_visit.h), said at the gate after it, and
 * whose face says it (FACE_*); NULL nothing to say. */
const char *gate_end_words(VisitEnd why, int *face);

#endif
