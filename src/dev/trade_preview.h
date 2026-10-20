/* Trade Central's screens opened in a run, out of the Net Square, for a
 * scripted capture (trade_preview.c, issue #145; docs/DEVTOOLS.md): the
 * dev step "trade list [WANT]" lists the run's spares (the `chip` step
 * gives some), "trade deal ID CODE ID CODE" shows the chip trade of the
 * two (MegaMan's first) and "trade swap" swaps its cards as a done trade
 * does. The other player is "Beta"; nothing is sent, and what is chosen is
 * printed. */
#ifndef CW_TRADE_PREVIEW_H
#define CW_TRADE_PREVIEW_H

#include <stdbool.h>
#include <stdint.h>

void trade_preview(const char *spec);
bool trade_preview_on(void);
/* While one is up: its frame, the game's keys none */
uint32_t trade_preview_keys(uint32_t keys);
void trade_preview_draw(void);

#endif
