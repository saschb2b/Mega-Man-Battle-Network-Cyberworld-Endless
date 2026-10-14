#ifndef CW_ONLINE_TEXT_H
#define CW_ONLINE_TEXT_H

#include <stddef.h>
#include <stdint.h>

#include "protocol.h"

extern const char online_title[], online_map_failed[], online_connecting[];
extern const char online_waiting[];
extern const char *const online_menu[5], *const online_phrases[ONLINE_PHRASE_COUNT], *const online_emotes[ONLINE_EMOTE_COUNT];
extern const char *const online_emote_glyphs[ONLINE_EMOTE_COUNT];
extern const char online_footer[], online_select_help[], online_card_help[], online_card_wait[], online_people_failed[];
extern const char online_people_title[], online_phrases_title[], online_emotes_title[], online_channel_title[], online_channel_help[];
extern const char online_muted[], online_blocked[], online_saved[], online_leaving[], online_back[];
void online_card_stats(char lines[5][48], unsigned hp, unsigned maximum, unsigned depth, unsigned act, unsigned best);
void online_card_powers(char *out, size_t size, uint16_t mask, int souls);
void online_card_buster(char *out, size_t size, unsigned attack, unsigned speed, unsigned charge);
void online_card_marks(char *out, size_t size, unsigned marks);

#endif
