#include "online_board.h"

#include <string.h>

#include "bn6_chips.h"
#include "chips.h"
#include "online_transport.h"

_Static_assert(WANT_COUNT == ONLINE_WANT_LAST + 1, "the wants are protocol.json's want_last + 1");

static struct {
	BoardOffer offers[ONLINE_MAX_PLAYERS];
	int count;
} B;

static uint16_t get16(const uint8_t *p) { return (uint16_t)(p[0] << 8 | p[1]); }
static uint32_t get32(const uint8_t *p) { return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3]; }

void online_board_reset(void) { memset(&B, 0, sizeof B); }

bool online_board_mail(const uint8_t *data, size_t size) {
	if (size < 2 || data[0] != ONLINE_BOARD || data[1] > ONLINE_MAX_PLAYERS ||
	    size != 2 + (size_t)data[1] * ONLINE_BOARD_ENTRY_BYTES) return false;
	BoardOffer got[ONLINE_MAX_PLAYERS];
	int n = 0;
	for (int k = 0; k < data[1]; ++k) {
		const uint8_t *e = data + 2 + k * ONLINE_BOARD_ENTRY_BYTES;
		BoardOffer o = { get32(e), { get16(e + 4), e[6] }, e[7] };
		if (!o.player || !o.have.id || o.have.id > ONLINE_CHIP_LAST || o.have.code > ONLINE_CODE_LAST || o.want > ONLINE_WANT_LAST) return false;
		/* (a chip this game cannot hold is never shown: the server's rule too) */
		if (online_trade_holds(o.have)) got[n++] = o;
	}
	memcpy(B.offers, got, sizeof got);
	B.count = n;
	return true;
}

int online_board_offers(const BoardOffer **out) {
	*out = B.offers;
	return B.count;
}

const BoardOffer *online_board_of(uint32_t player) {
	for (int k = 0; k < B.count; ++k) if (B.offers[k].player == player) return &B.offers[k];
	return NULL;
}

static bool send(TradeChip have, int want) {
	uint8_t m[ONLINE_OFFER_BYTES] = { ONLINE_OFFER, (uint8_t)(have.id >> 8), (uint8_t)have.id, have.code, (uint8_t)want };
	return online_transport_send(m, sizeof m);
}

bool online_board_post(TradeChip have, int want) {
	return want >= 0 && want < WANT_COUNT && online_trade_holds(have) && send(have, want);
}

bool online_board_clear(void) { return send((TradeChip){ 0, 0 }, 0); }

bool online_board_meets(int want, TradeChip chip) {
	const uint8_t *rec = chips_record(chip.id);
	if (want == WANT_ANY) return true;
	if (!rec) return false;
	if (want == WANT_MEGA) return rec[BN6_CHIP_LIBRARY_TYPE] == 1;
	/* (BN6's ChipElement, the icon a chip shows: bn6_chips.h) */
	static const uint8_t element[WANT_COUNT] = { 0, 0, 1, 2, 3, 5, 8, 9, 6, 0 };
	return want > WANT_ANY && want < WANT_MEGA && rec[BN6_CHIP_ELEMENT] == element[want];
}
