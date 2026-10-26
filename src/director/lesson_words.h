/* MegaMan's word on R in the town (lesson_words.c), for the director's other parts. */
#ifndef CW_LESSON_WORDS_H
#define CW_LESSON_WORDS_H

/* how near the port R was pressed (port_words); home's port is Lan's PC
 * (docs/world/home/): R in his house or room off it, and L in the home town
 * (`way` the way to Lan's front door) and in AsterLand and the Academy
 * (`way` the way out) */
enum { PORT_ALMOST_CELL, PORT_ALMOST, PORT_AWAY, PORT_HOUSE, PORT_ROOM, PORT_HOME, PORT_OUT };
const char *port_words(int how, const char *way);

#endif
