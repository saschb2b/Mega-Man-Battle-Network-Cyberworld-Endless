/* transfer.h. The code's exchange as a job on the relay (relay.h): a
 * send's new code, its keys and its saves sealed with them before each
 * connection (TAKEN: another code), a receive's keys; the server's
 * TRANSFER_STATE read as the link's state. A transfer asked for while the
 * relay is busy (a sync's pull or push) begins as soon as it is free. */
#include "transfer.h"

#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "network_prefs.h"
#include "protocol.h"
#include "relay.h"
#include "transfer_box.h"

#define TRIES 3   /* codes made where the server says one is in use */

static struct {
	bool put;              /* a send's (else a receive's) */
	TransferKeys keys;
	char code[TRANSFER_DIGITS + 1];
	uint8_t *plain;        /* a send's saves (sealed afresh for each code); a receive's once opened */
	size_t plain_n;
	uint8_t *blob;         /* a send's, sealed */
	size_t blob_n;
	int tries;
	bool wanted;           /* asked while the relay was busy */
	bool running;          /* begun on the relay and not yet ended (under the lock) */
	TransferState st;      /* (under the relay's lock) */
} T;

/* (the random bytes: the system's own, as the Square's key takes them) */
static bool random16(uint8_t out[16]) { return network_identity(out); }

/* The link's state set, the job ended */
static void finish(TransferPhase phase, TransferTrouble trouble, uint32_t value) {
	relay_lock();
	T.st.phase = phase;
	T.st.trouble = trouble;
	if (phase == TRANSFER_SENT) {
		memcpy(T.st.code, T.code, sizeof T.st.code);
		T.st.until = SDL_GetTicks() + value * 1000u;
	}
	if (trouble == TRANSFER_BUSY) T.st.wait_s = value;
	T.running = false;
	relay_unlock();
	relay_end(false);
}

/* TRANSFER: put, get or done under the lookup */
static void message(uint8_t op, uint8_t out[ONLINE_TRANSFER_BYTES]) {
	uint32_t size = op == ONLINE_TRANSFER_PUT ? (uint32_t)T.blob_n : 0;
	out[0] = ONLINE_TRANSFER;
	out[1] = ONLINE_TRANSFER_VERSION;
	out[2] = op;
	memcpy(out + 3, T.keys.lookup, ONLINE_TRANSFER_LOOKUP_BYTES);
	for (int i = 0; i < 4; ++i) out[19 + i] = (uint8_t)(size >> (24 - 8 * i));
}

static bool prepare(uint8_t first[RELAY_FIRST], size_t *first_n, const uint8_t **out, size_t *out_n) {
	if (T.put) {
		free(T.blob);
		T.blob = NULL;
		if (T.plain_n + TRANSFER_HEAD > ONLINE_TRANSFER_MAX) { finish(TRANSFER_FAILED, TRANSFER_TOO_BIG, 0); return false; }
		if (!transfer_code_make(T.code, random16) || !transfer_keys(T.code, &T.keys) ||
		    !(T.blob = transfer_seal(&T.keys, T.plain, T.plain_n, random16, &T.blob_n))) {
			finish(TRANSFER_FAILED, TRANSFER_NO_MEMORY, 0);
			return false;
		}
		*out = T.blob;
		*out_n = T.blob_n;
	} else if (!transfer_keys(T.code, &T.keys)) {
		finish(TRANSFER_FAILED, TRANSFER_NO_MEMORY, 0);
		return false;
	}
	message(T.put ? ONLINE_TRANSFER_PUT : ONLINE_TRANSFER_GET, first);
	*first_n = ONLINE_TRANSFER_BYTES;
	return true;
}

/* TRANSFER_STATE's word */
static void word(const uint8_t *d, size_t n) {
	if (d[0] != ONLINE_TRANSFER_STATE || n != ONLINE_TRANSFER_STATE_BYTES) { finish(TRANSFER_FAILED, TRANSFER_DAMAGED, 0); return; }
	uint32_t value = (uint32_t)d[2] << 24 | (uint32_t)d[3] << 16 | (uint32_t)d[4] << 8 | d[5];
	switch (d[1]) {
	case ONLINE_TRANSFER_READY:
		if (T.put) relay_send_blob();
		else finish(TRANSFER_FAILED, TRANSFER_DAMAGED, 0);
		break;
	case ONLINE_TRANSFER_HELD: finish(T.put ? TRANSFER_SENT : TRANSFER_FAILED, T.put ? TRANSFER_FINE : TRANSFER_DAMAGED, value); break;
	case ONLINE_TRANSFER_FOUND:
		if (T.put || value < TRANSFER_HEAD || value > ONLINE_TRANSFER_MAX) finish(TRANSFER_FAILED, TRANSFER_DAMAGED, 0);
		else if (!relay_take_blob(value)) finish(TRANSFER_FAILED, TRANSFER_NO_MEMORY, 0);
		break;
	case ONLINE_TRANSFER_NONE: finish(TRANSFER_FAILED, TRANSFER_NO_SAVES, 0); break;
	case ONLINE_TRANSFER_BUSY: finish(TRANSFER_FAILED, TRANSFER_BUSY, value); break;
	case ONLINE_TRANSFER_TAKEN:
		/* (sealed again under another code, TRIES at most, then the server
		 * is as good as full) */
		if (T.put && ++T.tries < TRIES) relay_end(true);
		else finish(TRANSFER_FAILED, TRANSFER_BUSY, 60);
		break;
	case ONLINE_TRANSFER_OFF: finish(TRANSFER_FAILED, TRANSFER_OFF, 0); break;
	case ONLINE_TRANSFER_REMOVED: break;
	default: finish(TRANSFER_FAILED, TRANSFER_DAMAGED, 0); break;
	}
}

/* A receive's blob in: opened, let go on the server (DONE), here */
static void taken(uint8_t *bytes, size_t n) {
	size_t plain = 0;
	if (!transfer_open(&T.keys, bytes, n, &plain)) {
		free(bytes);
		finish(TRANSFER_FAILED, TRANSFER_DAMAGED, 0);
		return;
	}
	T.plain = bytes;
	T.plain_n = plain;
	uint8_t m[ONLINE_TRANSFER_BYTES];
	message(ONLINE_TRANSFER_DONE, m);
	relay_reply(m, sizeof m);   /* (not let go: gone in its 15 minutes anyway) */
	finish(TRANSFER_RECEIVED, TRANSFER_FINE, 0);
}

static void lost(RelayLoss why) {
	/* (an ERROR: a server from before transfers, which refuses any first
	 * word but HELLO) */
	TransferTrouble t = why == RELAY_REFUSED ? TRANSFER_OFF : why == RELAY_DAMAGED ? TRANSFER_DAMAGED :
		why == RELAY_NO_MEMORY ? TRANSFER_NO_MEMORY : TRANSFER_UNREACHED;
	relay_lock();
	T.st.phase = TRANSFER_FAILED;
	T.st.trouble = t;
	T.running = false;
	relay_unlock();
}

static const RelayJob JOB = { prepare, word, taken, lost };

/* (begun on the relay where it is free: true where it is under way now) */
static bool begin_now(void) {
	if (relay_busy()) return false;
	relay_lock();
	T.running = true;
	relay_unlock();
	if (relay_start(&JOB)) return true;
	finish(TRANSFER_FAILED, TRANSFER_UNREACHED, 0);
	return true;
}

static bool running(void) {
	relay_lock();
	bool r = T.running;
	relay_unlock();
	return r;
}

/* Everything the last exchange held let go (it has ended) */
static void clear(void) {
	free(T.plain);
	free(T.blob);
	T.plain = T.blob = NULL;
	T.plain_n = T.blob_n = 0;
	SDL_memset(&T.keys, 0, sizeof T.keys);
	T.wanted = false;
	relay_lock();
	SDL_memset(&T.st, 0, sizeof T.st);
	relay_unlock();
}

static void start(bool put) {
	T.put = put;
	T.tries = 0;
	relay_lock();
	T.st.phase = TRANSFER_WORKING;
	relay_unlock();
	T.wanted = !begin_now();
}

static bool link_send_saves(uint8_t *bytes, size_t n) {
	if (!bytes || running() || T.wanted) {
		free(bytes);
		return false;
	}
	clear();
	T.plain = bytes;
	T.plain_n = n;
	start(true);
	return true;
}

static bool link_receive(const char *code) {
	if (!transfer_code_valid(code) || running() || T.wanted) return false;
	clear();
	memcpy(T.code, code, sizeof T.code);
	start(false);
	return true;
}

static void link_state(TransferState *out) {
	relay_poll();
	if (T.wanted) T.wanted = !begin_now();
	bool moving = running();
	relay_lock();
	*out = T.st;
	relay_unlock();
	if (moving) relay_progress(&out->moved, &out->size);
}

static uint8_t *link_take(size_t *n) {
	*n = 0;
	relay_lock();
	bool received = T.st.phase == TRANSFER_RECEIVED;
	relay_unlock();
	if (!received) return NULL;
	/* (the relay touches them no more once RECEIVED) */
	uint8_t *bytes = T.plain;
	*n = T.plain_n;
	T.plain = NULL;
	clear();
	return bytes;
}

static void link_cancel(void) {
	if (running()) relay_stop();
	clear();
}

static const TransferLink LINK = { link_send_saves, link_receive, link_state, link_take, link_cancel };

const TransferLink *online_transfer_link(const char *url) {
	if (!relay_here() || !url || SDL_strlen(url) >= NETWORK_URL) return NULL;
	relay_use(url);
	return &LINK;
}
