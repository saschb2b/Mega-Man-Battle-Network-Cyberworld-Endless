/* MegaMan's lessons (lesson_words.c), for the director's other parts. */
#ifndef CW_LESSON_WORDS_H
#define CW_LESSON_WORDS_H

void rush_hint(void);
void pack_words(void);
const char *gem_words(void);
/* how near the port R was pressed (port_words) */
enum { PORT_ALMOST_CELL, PORT_ALMOST, PORT_AWAY };
const char *port_words(int how, const char *way);

#endif
