/* The PET's KeyItem and E-Mail screens with the run's own words
 * (docs/PET.md): BN6's text archives rebuilt from the player's ROM with our
 * scripts in place of some, in the free ROM space, their pointers turned to
 * them; the items a run gives from the profile, and Dad's mail with the
 * battle data MegaMan has on each guardian. */
#ifndef CW_PET_TEXT_H
#define CW_PET_TEXT_H

#include <stdbool.h>

/* The NetPass's key item (docs/PET.md): one of the ids BN6 leaves
 * nameless; A on it opens the pass (pet.h) */
#define PET_ITEM_NETPASS 0x4A
/* The Jukebox's (docs/PET.md, The Jukebox): A on it opens it (pet.h) */
#define PET_ITEM_JUKEBOX 0x4B

/* The archives, once on the core's ROM copy (the key items' names and
 * descriptions, the mails' senders, subjects and bodies). */
void pet_text_install(void);
/* The descriptions that count (the codes, the Library) made again and the
 * profile's key items given (the NaviCodes, the DarkPass, the LibCard, the
 * NetPass, the Jukebox): on a layer as part of pet_text_refresh, at home
 * wherever the game's state lacks them (pet.c), where a NEW GAME's PET held
 * only the PET until the first layer (session 74). */
void pet_text_items(void);
/* Whether the game's state holds them (the NetPass's and the Jukebox's counts). */
bool pet_text_items_held(void);
/* On a layer: pet_text_items, and a mail delivered for each
 * guardian MegaMan has battle data on and the list lacks. Returns the navi
 * of a mail delivered now (the last), 0 for none. */
int pet_text_refresh(void);
/* The face of mail `m`'s sender, as the run writes its mails: Dad's for
 * the lab's and the guardians'; FACE_NONE for the BBS's (a board, its
 * posts in their posters' faces) and BN6's own, which a run never sends. */
int pet_mail_face(int m);

#endif
