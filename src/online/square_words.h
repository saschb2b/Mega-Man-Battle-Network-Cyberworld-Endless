/* What is said in the Net Square (square_words.c, docs/ONLINE.md,
 * docs/VOICE.md): MegaMan to Lan, the channel keeper Mr.Prog, and MegaMan
 * reading another player's MegaMan. The quick phrases and emotes
 * themselves are the protocol's (online_text.c). */
#ifndef CW_SQUARE_WORDS_H
#define CW_SQUARE_WORDS_H

#include "online_types.h"

/* L: look around (square_look_words) or the quiet's option; the first
 * time, MegaMan naming SELECT's emotes and phrases (emote_strip.h). */
const char *square_look_option(void);
const char *square_strip_hint(void);
/* The greeter's word before he stamps a NetPass just come full (the
 * stamp moment, netpass_screen.h) */
const char *square_greeter_stamp_words(void);
/* ... a box of options goes on with this (a peer's card's). */
const char *square_more(void);
/* ... L's option to hide everyone else's phrases and emotes, or, where
 * hidden (`quiet`), to show them again (square_emote.h). */
const char *square_quiet_option(bool quiet);
const char *square_back(void);
/* R, or the pad home: MegaMan's question. */
const char *square_home_question(void);
/* A facing another player's MegaMan: who he is and his card (`card`
 * NULL where it has not come), his NetPass where it is not full
 * (`pass_full`: netpass.h, by his ROMs), and the options for him, `muted`
 * naming the second Unmute. */
const char *square_peer_words(const char *name, const OnlineProfile *card, bool pass_full);
/* ... the card's facts alone, in pages (HP, act and layer, Crosses, Souls;
 * never the folder): empty where no card has come */
const char *square_card_facts(const OnlineProfile *card);
const char *square_peer_option(int k, bool muted);
#define SQUARE_PEER_OPTIONS 3
/* ... after a block. */
const char *square_blocked_words(const char *name);
/* The channel keeper: the channel MegaMan is in of `channels`, how many
 * Navis stand in it, his question and his two options. */
const char *square_keeper_words(unsigned channel, unsigned channels, unsigned navis);
extern const char *const square_keeper_options[2];
/* ... a channel's move asked for. */
const char *square_keeper_moving(void);

#endif
