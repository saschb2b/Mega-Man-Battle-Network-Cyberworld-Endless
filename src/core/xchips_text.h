/* The PET's words for a chip of Battle Network 5's where its ROM is not
 * read (docs/MULTIROM.md, One net: the design, call 4): the placeholder a
 * saved folder, pack and Library keep it as, which sits out of battles. */
#ifndef CW_XCHIPS_TEXT_H
#define CW_XCHIPS_TEXT_H

#include <stddef.h>

/* Its name, by BN5's chip id ("BN5-12": its number there) */
void xchips_text_name(int bn5, char *out, size_t n);
/* Its description, three lines as the folder's box shows them (nine
 * letters at most, as BN6's own) */
extern const char *const xchips_text_desc[3];

#endif
