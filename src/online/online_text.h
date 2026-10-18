#ifndef CW_ONLINE_TEXT_H
#define CW_ONLINE_TEXT_H

#include <stddef.h>
#include <stdint.h>

#include "protocol.h"

extern const char online_title[];
/* The set-once fields (online_field.c) */
extern const char online_field_name_title[], online_field_address_title[], online_field_name_hint[], online_field_address_hint[];
extern const char online_field_bad_name[], online_field_bad_address[], online_field_space[], online_field_keys_hint[];
extern const char *const online_field_buttons[3];
extern const char online_waiting[];
extern const char *const online_menu[5], *const online_phrases[ONLINE_PHRASE_COUNT], *const online_emotes[ONLINE_EMOTE_COUNT];
extern const char *const online_emote_glyphs[ONLINE_EMOTE_COUNT];
extern const char online_select_help[], online_card_help[], online_card_wait[], online_people_failed[];
extern const char online_people_title[], online_phrases_title[], online_emotes_title[], online_channel_title[], online_channel_help[];
extern const char online_muted[], online_blocked[], online_saved[];
void online_card_stats(char lines[5][48], unsigned hp, unsigned maximum, unsigned depth, unsigned act, unsigned best);
void online_card_powers(char *out, size_t size, uint16_t mask, int souls);
void online_card_buster(char *out, size_t size, unsigned attack, unsigned speed, unsigned charge);
void online_card_marks(char *out, size_t size, unsigned marks);

#endif
