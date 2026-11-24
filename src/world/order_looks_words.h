/* What AsterLand's clerk says as he sells colors (order_looks_words.c). */
#ifndef CW_ORDER_LOOKS_WORDS_H
#define CW_ORDER_LOOKS_WORDS_H

/* His two services, as options under his greeting */
extern const char *const order_looks_services[2];
/* His question for palette `p`, its price named */
const char *order_looks_ask(int p);
/* Short of the zenny; the palette sold already this visit; as he hands it over */
extern const char *const order_looks_broke;
extern const char *const order_looks_sold;
extern const char *const order_looks_thanks;

#endif
