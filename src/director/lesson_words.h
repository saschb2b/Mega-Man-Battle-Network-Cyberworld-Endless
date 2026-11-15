/* MegaMan's word on R in the town and on a layer (lesson_words.c), for the director's other parts. */
#ifndef CW_LESSON_WORDS_H
#define CW_LESSON_WORDS_H

#include <stdbool.h>

/* how near the port R was pressed (port_words); home's port is Lan's PC
 * (docs/world/home/): R in his house or room off it, and L in the home town
 * (`way` the way to Lan's front door) and in AsterLand and the Academy
 * (`way` the way out) */
enum { PORT_ALMOST_CELL, PORT_ALMOST, PORT_AWAY, PORT_HOUSE, PORT_ROOM, PORT_HOME, PORT_OUT };
const char *port_words(int how, const char *way);
/* R on a layer, BN6's jack-out held off for the run: where the way home
 * is, past the act's guardian (`exit_open`: he has fallen here) */
const char *jack_out_words(bool exit_open);

#endif
