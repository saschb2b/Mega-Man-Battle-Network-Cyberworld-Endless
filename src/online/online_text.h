/* The Net Square's own words on screen (online_text.c): the protocol's
 * quick phrases and emotes, and the set-once fields' (online_field.c). */
#ifndef CW_ONLINE_TEXT_H
#define CW_ONLINE_TEXT_H

#include "protocol.h"

extern const char *const online_phrases[ONLINE_PHRASE_COUNT], *const online_emotes[ONLINE_EMOTE_COUNT];
/* ... an emote as it shows over a MegaMan's head */
extern const char *const online_emote_glyphs[ONLINE_EMOTE_COUNT];
extern const char online_field_name_title[], online_field_address_title[], online_field_name_hint[], online_field_address_hint[];
extern const char online_field_bad_name[], online_field_bad_address[], online_field_space[], online_field_keys_hint[];
extern const char *const online_field_buttons[3];

#endif
