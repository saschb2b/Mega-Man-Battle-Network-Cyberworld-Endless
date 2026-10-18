/* What is said at Trade Central (trade_words.c, docs/ONLINE.md,
 * docs/VOICE.md): its clerk, a Mr.Prog in capitals, and MegaMan. Chips
 * are named as BN6 names them, their code after ("Cannon A"). */
#ifndef CW_TRADE_WORDS_H
#define CW_TRADE_WORDS_H

#include <stdbool.h>

/* The clerk, nothing under way: who he is, and his options (B: bye). */
const char *trade_hello(void);
extern const char *const trade_options[2];
const char *trade_how(void);
const char *trade_bye(void);
/* ... whom to trade with, a page of names. */
const char *trade_who(void);
const char *trade_none_here(void);
/* ... the chip MegaMan offers, on BN6's order screen after this
 * (`bn6_only`: BN5's spares kept back, the partner's game without its ROM). */
const char *trade_pick(bool bn6_only);
const char *trade_no_spares(void);
/* ... no spare fits: `name`'s game trades BN6's chips alone (no BN5 ROM)
 * and MegaMan's are all BN5's; or no Navi here trades BN5's, and his are. */
const char *trade_bn6_only(const char *name);
const char *trade_bn5_spares(void);
const char *trade_sent(const char *name);
/* ... an offer to MegaMan: who and what, and the options. */
const char *trade_offered(const char *name, const char *chip);
extern const char *const trade_offer_options[2];
/* ... waiting on the other player, and the options. */
const char *trade_waiting(const char *name);
extern const char *const trade_wait_options[2];
/* MegaMan as it goes: an offer come in, both chips to say yes to (the
 * question, talk_ask's), a deal made, a trade off. */
const char *trade_heard(const char *name, const char *chip);
const char *trade_review(const char *name, const char *mine, const char *theirs);
const char *trade_deal_question(void);
const char *trade_done(const char *chip);
const char *trade_off(void);
/* BN6's order screen as Trade Central's picker (trade_order.c): its prompt,
 * the way back, the offer readied; MegaMan's question on leaving it
 * (BN6's Yes and No follow); the clerk's question and MegaMan's word, each
 * before the chip's name in quotes (its line break theirs). */
const char *trade_order_prompt(void);
const char *trade_order_after(void);
const char *trade_order_all(void);
const char *trade_order_once(void);
const char *trade_order_back(void);
const char *trade_order_ready(void);
const char *trade_order_leave(void);
const char *trade_order_offer(void);
const char *trade_order_said(void);
/* At home, the swap given to the run. */
const char *trade_settled(const char *gave, const char *got);

#endif
