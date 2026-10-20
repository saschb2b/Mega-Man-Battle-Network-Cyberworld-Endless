/* The wait for a dropped link's PET text (reconnect_text.c, issue #153;
 * docs/ONLINE.md, A dropped link; docs/VOICE.md): its heading, its state,
 * the time left and its key. Drawn by reconnect_draw.c. */
#ifndef CW_RECONNECT_TEXT_H
#define CW_RECONNECT_TEXT_H

#include <stddef.h>

extern const char reconnect_text_title[];
/* the heading's slot: the link lost, and back */
extern const char reconnect_text_lost[];
extern const char reconnect_text_back[];
/* the state, its dots `dots` (0-3) */
const char *reconnect_text_trying(int dots);
/* under it: what the wait keeps */
extern const char reconnect_text_kept[];
/* the time left to try, `seconds` */
const char *reconnect_text_left(unsigned seconds);
/* its key */
extern const char reconnect_text_key_home[];

#endif
