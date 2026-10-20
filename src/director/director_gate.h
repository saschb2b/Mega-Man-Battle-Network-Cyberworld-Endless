/* The Net Square's gate in Lan's HP (director_gate.c; docs/HOME.md piece
 * 11, docs/ONLINE.md). */
#ifndef CW_DIRECTOR_GATE_H
#define CW_DIRECTOR_GATE_H

#include <stdbool.h>

/* Event flags of the gate's chats (free in BN6, docs/EMULATION.md): the
 * Yes of its questions, and the keeper's three options. */
#define GATE_YES_FLAG      0x1473
#define GATE_OFFICIAL_FLAG 0x1474
#define GATE_OTHER_FLAG    0x1475
#define GATE_NAME_FLAG     0x1476

/* A visit home begins: the player's Square settings read, the keeper's
 * words written (after lanhp_install). */
void home_gate_visit(void);
/* Each frame at home: the gate stepped on, its questions and the visit
 * they begin, the keeper's options, what is said after a visit. */
void home_gate_frame(void);
/* The run saved where MegaMan stands at home, the write confirmed. */
bool home_gate_save(void);
/* Whether the gate holds the pad (a text field open over the game). */
bool home_gate_holds(void);
/* Whether MegaMan's question at the gate is open ("To the Net Square,
 * Lan?"): the NetPass's strip shows over the game meanwhile */
bool home_gate_asking(void);

#endif
