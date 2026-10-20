/* trade_preview.h. */
#include "trade_preview.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "online_board.h"
#include "trade_central.h"
#include "trade_screen.h"

#define PARTNER "Beta"

static bool on;

void trade_preview(const char *spec) {
	static TradeRow all[TRADE_LIST_MOST], fit[TRADE_LIST_MOST];
	unsigned a, b, c, d;
	if (!strncmp(spec, "list", 4)) {
		int want = spec[4] ? atoi(spec + 5) : -1, n = trade_central_spares(all);
		int m = trade_list_filter(all, n, want, online_board_meets, fit);
		printf("trade preview: %d spares, %d listed\n", n, m);
		trade_screen_list(fit, m, want >= 0 ? TRADE_FOR_TAKE : TRADE_FOR_OFFER, PARTNER, want, (TradeChip){ 0, 0 });
		on = true;
	} else if (sscanf(spec, "deal %u %u %u %u", &a, &b, &c, &d) == 4) {
		trade_screen_deal((TradeChip){ (uint16_t)a, (uint8_t)b }, (TradeChip){ (uint16_t)c, (uint8_t)d }, PARTNER);
		on = true;
	} else if (!strcmp(spec, "swap")) trade_screen_swap();
	else fprintf(stderr, "trade preview: what is \"%s\"?\n", spec);
}

bool trade_preview_on(void) { return on; }

uint32_t trade_preview_keys(uint32_t keys) {
	if (!on) return keys;
	TradeChip chip;
	int e = trade_screen_event(&chip);
	if (e == TRADE_EVENT_PICKED) printf("trade preview: picked %u %u\n", chip.id, chip.code);
	else if (e) printf("trade preview: chosen %d\n", e);
	if (trade_screen_update()) return 0;
	on = false;
	return keys;
}

void trade_preview_draw(void) {
	if (on) trade_screen_draw();
}
