/* The release notes' pictures (docs/engine/updates.md, The pictures): each
 * entry's first, at the game's frame or smaller, in update-pictures.bin, a
 * file of the release's that update.json names under "pictures" with its
 * size and SHA-256 (tools/update_manifest.py writes it). Fetched on a
 * thread of its own once the update box is due or opened, checked whole,
 * kept packed; a picture unpacked for the box when its page shows. Where
 * the system fetches on no thread of the game's (Android's Java, the
 * browser) or has the heap for nothing more (the 3DS), there are none and
 * the box keeps their room empty. */
#ifndef CW_UPDATE_PICTURES_H
#define CW_UPDATE_PICTURES_H

#include <stdbool.h>
#include <stdint.h>

/* (update.c's) update.json's text, its newer version found: where the
 * pictures are, `agent` the requests' User-Agent */
void pictures_found(const char *manifest, const char *agent);
/* (update.c's, as the game exits) a fetch under way stopped and waited for */
void pictures_end(void);

/* Whether this system shows them: it fetches, and the release has them
 * (the box keeps their room only where they can come) */
bool update_pictures_here(void);
/* The pictures fetched, where they are not yet (the box's, as it opens) */
void update_pictures_want(void);
/* Picture `i` unpacked: its size and its pixels as ARGB8888 (the caller's
 * to free); false where it is not there (not fetched yet, none, or bad) */
bool update_picture(int i, int *w, int *h, uint32_t **argb);

#endif
