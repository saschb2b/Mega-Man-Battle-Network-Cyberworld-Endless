/* The payloads' writer (analytics.c), for the statistics' other files of
 * events (analytics_online.c): a payload is begun with Umami's head and
 * the event's name, its data's members written, then handed to the system
 * where the session sends. Not for the rest of the game, which calls the
 * events (analytics.h, analytics_online.h). */
#ifndef CW_ANALYTICS_PAYLOAD_H
#define CW_ANALYTICS_PAYLOAD_H

#include <stdbool.h>
#include <stddef.h>

/* JSON written into a buffer: members after the first get their comma;
 * what does not fit makes the whole payload unsent */
typedef struct {
	char *s;
	size_t n, len;
	bool first, full;
} Json;

/* a member of the object open: a string (escaped), a number, a flag */
void json_member(Json *j, const char *k, const char *v);
void json_int(Json *j, const char *k, int v);
void json_bool(Json *j, const char *k, bool v);

/* Whether the session sends: a yes, and a player's start or a test's own
 * address */
bool analytics_sends(void);
/* The system, as the events' address's path names it ("linux", "3ds") */
const char *analytics_system(void);
/* An event named `name` begun in `out` (Umami's head: the website, the
 * system's path, the version, the screen), its data's object open ... */
void analytics_event_begin(Json *j, char *out, size_t n, const char *name);
/* ... closed and handed to the system, where the session sends and it fit */
void analytics_event_send(Json *j);

#endif
