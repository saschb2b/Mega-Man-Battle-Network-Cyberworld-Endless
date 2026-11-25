/* What is said as MegaMan gets a palette (looks_words.c; docs/game/looks-earning.md,
 * Earning; docs/voice/rules.md). */
#ifndef CW_LOOKS_WORDS_H
#define CW_LOOKS_WORDS_H

/* MegaMan's talk for the palettes `palettes` (a bit each) he has not yet
 * said he got: his word on where each came from, then BN6's "MegaMan
 * got:" box naming it; `found` the ones BN6's own box already named as a
 * Mystery Data's ColrData, which his word names instead. */
const char *looks_got_words(unsigned palettes, unsigned found);
/* Dad's mail on the colours, its subject and its body (pages split by
 * form feeds), the first time the profile has more than Classic. */
const char *looks_mail_subject(void);
const char *looks_mail_text(void);

#endif
