/* The Battle Lab's builder's words (lab_text.c): the PET's text of a
 * screen of ours, in KeyItem's voice (docs/voice/rules.md). */
#ifndef CW_LAB_TEXT_H
#define CW_LAB_TEXT_H

#include <stddef.h>

#include "lab_battle.h"

extern const char lab_title[], lab_go[], lab_info_label[];
extern const char *const lab_tabs[4];
extern const char lab_row_preset[], lab_row_area[], lab_row_song[], lab_own_song[], lab_plain_field[];
/* A panel type's name (BN6_PANEL_TYPE), "" for none of BN6's */
const char *lab_panel_name(int type);
/* "V2", "SP", "Rare" as a row names a version (0-5) */
const char *lab_version_word(int version);
/* A count of the foes set, "2/4" */
void lab_count_words(char *out, size_t n, int foes, int most);
/* BN6's field `field` as the preset's value */
void lab_preset_words(char *out, size_t n, int field);
/* The information box: a family's row (complete: its HP and deletions;
 * not yet: its gauge), a family lifted, a panel, the button with nothing
 * set, and how the last battle ended */
void lab_info_family(char *out, size_t n, int hp, int deleted);
void lab_info_gauge(char *out, size_t n, int deleted, int complete);
void lab_info_lifted(char *out, size_t n, const char *name, int foes, int most);
void lab_info_panel(char *out, size_t n, const char *panel, const char *foe);
void lab_info_empty(char *out, size_t n);
void lab_info_result(char *out, size_t n, const LabResult *r);
/* A kept set's row ("Set 2"), its value (its foes, or that it holds none)
 * and what the information box says of it: its best win, or none yet */
void lab_set_words(char *out, size_t n, int slot);
void lab_set_value(char *out, size_t n, int foes);
void lab_info_set(char *out, size_t n, int foes, int bust, int frames, int hp_lost);

#endif
