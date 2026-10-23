/* Trade Central's two screens' PET text (trade_screen_text.c, issue #145;
 * docs/ONLINE.md, Trade Central; docs/VOICE.md): the spares list's and
 * the chip trade's headings, notes and keys. What the clerk and MegaMan
 * say around them is trade_words.c's. */
#ifndef CW_TRADE_SCREEN_TEXT_H
#define CW_TRADE_SCREEN_TEXT_H

#include "trade_list.h"

/* The spares list: its heading by what the chip is for (TRADE_FOR_*,
 * trade_screen.h), the slot beside it (`partner`'s name, or the board) */
const char *trade_text_list_title(int purpose);
const char *trade_text_list_for(int purpose, const char *partner);
/* ... the list's head, its sort's name (TRADE_SORT_*) */
extern const char trade_text_spares[];
/* ... START's pop-up's label (the folder editor's SORT) */
extern const char trade_text_sort[];
extern const char *const trade_text_sorts[TRADE_SORT_KEYS];
/* ... the note under it: the spares that meet `want` (WANT_* of
 * online_board.h), or for an answer what `partner` offers (`theirs`, the
 * chip's name and code); "" when the list is every spare */
const char *trade_text_list_note(int purpose, int want, const char *partner, const char *theirs);

/* The chip trade: its heading, the slot beside it */
extern const char trade_text_deal_title[];
const char *trade_text_deal_with(const char *partner);
/* ... MegaMan in its NAVIGATOR box: the box's label; the trade asked
 * (`mine`, `theirs` a chip's name and code), its answers (the cursor
 * starts on No); waiting on the other's Yes; the cards crossing; done */
extern const char trade_text_navigator[];
const char *trade_text_ask(const char *mine, const char *partner, const char *theirs);
extern const char trade_text_yes[];
extern const char trade_text_no[];
const char *trade_text_waiting(const char *partner);
extern const char trade_text_swapping[];
const char *trade_text_traded(const char *theirs);

#endif
