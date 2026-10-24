/* The set-once text fields of going online (docs/ONLINE.md): the player's
 * name in the Net Square and a server's address, typed once at the gate's
 * keeper in Lan's HP. The one screen of its own the Square needs, over the
 * game: a keyboard on the pad and by touch, the system's own keyboard and
 * paste where there is one (text_input.c). */
#ifndef CW_ONLINE_FIELD_H
#define CW_ONLINE_FIELD_H

#include <stdbool.h>
#include <stddef.h>

typedef enum { FIELD_NAME, FIELD_ADDRESS } FieldKind;
enum { FIELD_EDITING, FIELD_DONE, FIELD_CANCELLED };

/* Opens the field of `kind` holding `initial`. */
void online_field_open(FieldKind kind, const char *initial);
bool online_field_shown(void);
/* Each frame while shown: FIELD_EDITING, or the field closed, FIELD_DONE
 * with what was typed in `out` (a valid name, or a server's full URL,
 * its scheme, port and /presence added where left out: network_address.h),
 * or FIELD_CANCELLED. */
int online_field_update(char *out, size_t cap);
void online_field_draw(void);
/* A URL's host as a person reads it ("cyberworld.saschb2b.com"), at most
 * `cap` - 1 characters. */
void online_field_host(const char *url, char *out, size_t cap);

#endif
