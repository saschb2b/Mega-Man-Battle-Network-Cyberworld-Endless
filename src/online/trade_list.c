/* trade_list.h. */
#include "trade_list.h"

#include <stdlib.h>

int trade_list_spare(int have, int in_folders) {
	int spare = have - in_folders;
	return spare <= 0 ? 0 : spare > TRADE_COUNT_MOST ? TRADE_COUNT_MOST : spare;
}

int trade_list_filter(const TradeRow *all, int n, int want, bool (*meets)(int want, TradeChip chip), TradeRow *out) {
	int k = 0;
	for (int i = 0; i < n && k < TRADE_LIST_MOST; ++i) {
		if (!all[i].count || !all[i].chip.id) continue;
		if (want >= 0 && (!meets || !meets(want, all[i].chip))) continue;
		out[k++] = all[i];
	}
	return k;
}

static int sort_key;

/* a row's key: its code's place after Z for * (26), as BN6's Code sort */
static uint32_t key_of(const TradeRow *r) {
	switch (sort_key) {
	case TRADE_SORT_ABCDE: return r->abc_order;
	case TRADE_SORT_CODE: return r->chip.code;
	default: return r->id_order;
	}
}

static int by_key(const void *a, const void *b) {
	const TradeRow *x = a, *y = b;
	uint32_t kx = key_of(x), ky = key_of(y);
	if (kx != ky) return kx < ky ? -1 : 1;
	/* (ties by ID, then code, then the chip's id: one order whatever the list's) */
	if (x->id_order != y->id_order) return x->id_order < y->id_order ? -1 : 1;
	if (x->chip.code != y->chip.code) return x->chip.code < y->chip.code ? -1 : 1;
	return (x->chip.id > y->chip.id) - (x->chip.id < y->chip.id);
}

void trade_list_sort(TradeRow *rows, int n, int key) {
	sort_key = key >= 0 && key < TRADE_SORT_KEYS ? key : TRADE_SORT_ID;
	if (n > 1) qsort(rows, (size_t)n, sizeof *rows, by_key);
}

int trade_list_next_key(int key) { return key + 1 >= TRADE_SORT_KEYS || key < 0 ? TRADE_SORT_ID : key + 1; }
