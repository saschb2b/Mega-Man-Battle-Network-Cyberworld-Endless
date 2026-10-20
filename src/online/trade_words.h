/* What is said at Trade Central (trade_words.c, docs/ONLINE.md,
 * docs/VOICE.md): its clerk, a Mr.Prog in capitals, and MegaMan. Chips
 * are named as BN6 names them, their code after ("Cannon A"). */
#ifndef CW_TRADE_WORDS_H
#define CW_TRADE_WORDS_H

#include <stdbool.h>

/* The clerk, nothing under way: who he is, and his options (B: bye). */
const char *trade_hello(void);
extern const char *const trade_options[3];
const char *trade_how(void);
const char *trade_bye(void);
/* ... whom to trade with, a page of names. */
const char *trade_who(void);
const char *trade_none_here(void);
/* ... no other Navi here with the Trade stamp on his NetPass */
const char *trade_none_stamped(void);
/* ... MegaMan without a spare (the spares list's chips, trade_screen.h). */
const char *trade_no_spares(void);
const char *trade_sent(const char *name);
/* ... an offer to MegaMan: who and what, and the options. */
const char *trade_offered(const char *name, const char *chip);
extern const char *const trade_offer_options[2];
/* ... waiting on the other player, and the options. */
const char *trade_waiting(const char *name);
extern const char *const trade_wait_options[2];
/* MegaMan as it goes: an offer come in; an answer to his, or his board
 * offer taken (the chip trade's screen after it, trade_screen.h); a deal
 * made, after its cards swapped; a trade off. */
const char *trade_heard(const char *name, const char *chip);
const char *trade_answered(const char *name);
const char *trade_board_taken(const char *name);
const char *trade_done(const char *chip);
const char *trade_off(void);
/* At home, the swap given to the run. */
const char *trade_settled(const char *gave, const char *got);

/* The board (docs/ONLINE.md, Trade Central: the board). A want's words,
 * "a Fire chip" (WANT_* of online_board.h, 0 "anything"). */
const char *trade_want(int want);
extern const char *const trade_want_options[10];
/* The clerk on the board: how many offers are up; one offer, whose
 * (`name`), the chip he has and what he wants; the options under it
 * (answer, the next offer) and MegaMan's own (post one, take his down,
 * never mind). */
const char *trade_board_intro(int offers);
const char *trade_board_offer(const char *name, const char *chip, int want);
extern const char *const trade_board_options[2];
const char *trade_board_mine(bool posted);
const char *trade_board_leave(void);
/* ... answering one: none of MegaMan's spares meeting its want. */
const char *trade_take_none(const char *name, int want);
/* ... posting one: the want for the chip picked; it's up; taken down. */
const char *trade_post_want(void);
const char *trade_posted(const char *chip, int want);
const char *trade_post_down(void);
/* ... one trade a visit: MegaMan has his, to take home first. */
const char *trade_traded(void);
/* ... and to a Navi on the week's dive: no trades till it ends. */
const char *trade_diving(void);
/* ... and to a NetPass without the Trade stamp (netpass.h). */
const char *trade_no_stamp(void);

#endif
