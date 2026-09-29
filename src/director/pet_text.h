/* The PET's KeyItem and E-Mail screens with the run's own words
 * (docs/PET.md): BN6's text archives rebuilt from the player's ROM with our
 * scripts in place of some, in the free ROM space, their pointers turned to
 * them; the items a run gives from the profile, and Dad's mail with the
 * battle data MegaMan has on each guardian. */
#ifndef CW_PET_TEXT_H
#define CW_PET_TEXT_H

/* The archives, once on the core's ROM copy (the key items' names and
 * descriptions, the mails' senders, subjects and bodies). */
void pet_text_install(void);
/* On a layer: the descriptions that count (the codes, the Library) made
 * again, the profile's key items given, and a mail delivered for each
 * guardian MegaMan has battle data on and the list lacks. Returns the navi
 * of a mail delivered now (the last), 0 for none. */
int pet_text_refresh(void);

#endif
