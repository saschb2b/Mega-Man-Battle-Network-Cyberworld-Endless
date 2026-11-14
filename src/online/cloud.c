/* cloud.h. One SYNC at a time on the relay: a pull from the receipt's
 * generation, a push from the generation its saves were made from, a
 * drop. An op asked for while the relay is busy waits for it (a push the
 * newest saves: one waiting is replaced). The receipt is written to
 * cloud.ini on the game's thread, in state(). */
#include "cloud.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <SDL.h>

#include "game.h"
#include "network_prefs.h"
#include "platform.h"
#include "protocol.h"
#include "relay.h"
#include "transfer_box.h"

/* (the saves' head in the sealed blob: a mark and a byte of flags, none yet) */
static const uint8_t HEAD[4] = { 'C', 'W', 'S', 'Y' };
#define PLAIN_HEAD 5
/* (a link's bytes: a mark and the slot's secret) */
static const uint8_t LINK_MARK[4] = { 'C', 'W', 'L', 'K' };
#define LINK_BYTES (4 + TRANSFER_SECRET)

static struct {
	/* the slot and the receipt (the receipt under the relay's lock) */
	bool linked;
	uint8_t secret[TRANSFER_SECRET];
	TransferSlot slot;
	uint32_t generation, hash;
	uint64_t synced_at;
	bool receipt_due;          /* to be written to cloud.ini on the game's thread */
	/* the op under way (the relay's while running) */
	CloudDoing doing;          /* (under the lock) */
	CloudTrouble trouble;      /* (under the lock) */
	bool running;              /* (under the lock) */
	uint32_t ends;             /* (under the lock) */
	uint8_t *plain;            /* a push's saves */
	size_t plain_n;
	uint32_t from, push_hash;
	uint8_t *blob;             /* a push's, sealed */
	size_t blob_n;
	uint32_t found;            /* a pull's FOUND generation */
	uint8_t *pulled;           /* a pull's saves, waiting (under the lock) */
	size_t pulled_n;
	uint32_t pulled_gen;
	/* the op waiting for the relay (the game's thread's) */
	CloudDoing wanted;
	uint8_t *want_plain;
	size_t want_n;
	uint32_t want_from, want_hash;
} C;

static bool random16(uint8_t out[16]) { return network_identity(out); }

static uint64_t now_s(void) {
	time_t t = time(NULL);
	return t > 0 ? (uint64_t)t : 1;
}

/* ---- cloud.ini ---- */

static void ini_path(char *out, size_t n) { snprintf(out, n, "%s/cloud.ini", g_data_dir); }

static void receipt_write(void) {
	char path[600], tmp[620];
	ini_path(path, sizeof path);
	if (!C.linked) {
		remove(path);
		platform_persist();
		return;
	}
	snprintf(tmp, sizeof tmp, "%s.tmp", path);
	FILE *f = fopen(tmp, "w");
	if (!f) return;
	fprintf(f, "secret ");
	for (int i = 0; i < TRANSFER_SECRET; ++i) fprintf(f, "%02x", C.secret[i]);
	relay_lock();
	fprintf(f, "\ngeneration %u\nhash %08x\nsynced %llu\n", (unsigned)C.generation, (unsigned)C.hash, (unsigned long long)C.synced_at);
	relay_unlock();
	bool ok = fclose(f) == 0;
	if (ok && rename(tmp, path) != 0) {
		remove(path);
		ok = rename(tmp, path) == 0;
	}
	if (!ok) remove(tmp);
	platform_persist();
}

static bool hex_bytes(const char *s, uint8_t *out, size_t n) {
	for (size_t i = 0; i < n; ++i) {
		unsigned v;
		if (sscanf(s + 2 * i, "%2x", &v) != 1) return false;
		out[i] = (uint8_t)v;
	}
	return true;
}

static void receipt_read(void) {
	char path[600], line[200];
	ini_path(path, sizeof path);
	FILE *f = fopen(path, "r");
	if (!f) return;
	bool secret = false;
	unsigned generation = 0, hash = 0;
	unsigned long long synced = 0;
	while (fgets(line, sizeof line, f)) {
		if (!strncmp(line, "secret ", 7) && strlen(line) >= 7 + 2 * TRANSFER_SECRET) secret = hex_bytes(line + 7, C.secret, TRANSFER_SECRET);
		else if (sscanf(line, "generation %u", &generation) == 1) {}
		else if (sscanf(line, "hash %x", &hash) == 1) {}
		else if (sscanf(line, "synced %llu", &synced) == 1) {}
	}
	fclose(f);
	if (!secret) return;
	C.linked = true;
	transfer_slot(C.secret, &C.slot);
	C.generation = generation;
	C.hash = hash;
	C.synced_at = synced;
}

/* ---- the job ---- */

static void ended(CloudTrouble trouble) {
	relay_lock();
	C.doing = CLOUD_IDLE;
	C.trouble = trouble;
	if (C.running) ++C.ends;
	C.running = false;
	relay_unlock();
}

static void finish(CloudTrouble trouble) {
	ended(trouble);
	relay_end(false);
}

/* SYNC: the op on the slot from `generation`, `size` a push's */
static void message(uint8_t op, uint32_t generation, uint32_t size, uint8_t out[ONLINE_SYNC_BYTES]) {
	out[0] = ONLINE_SYNC;
	out[1] = ONLINE_SYNC_VERSION;
	out[2] = op;
	memcpy(out + 3, C.slot.slot, ONLINE_SYNC_SLOT_BYTES);
	memcpy(out + 19, C.slot.auth, ONLINE_SYNC_AUTH_BYTES);
	for (int i = 0; i < 4; ++i) {
		out[51 + i] = (uint8_t)(generation >> (24 - 8 * i));
		out[55 + i] = (uint8_t)(size >> (24 - 8 * i));
	}
}

static CloudDoing doing(void) {
	relay_lock();
	CloudDoing d = C.doing;
	relay_unlock();
	return d;
}

static bool prepare(uint8_t first[RELAY_FIRST], size_t *first_n, const uint8_t **out, size_t *out_n) {
	CloudDoing d = doing();
	*first_n = ONLINE_SYNC_BYTES;
	if (d == CLOUD_PULLING) {
		relay_lock();
		uint32_t from = C.generation;
		relay_unlock();
		message(ONLINE_SYNC_PULL, from, 0, first);
		return true;
	}
	if (d == CLOUD_DROPPING) {
		message(ONLINE_SYNC_DROP, 0, 0, first);
		return true;
	}
	free(C.blob);
	C.blob = NULL;
	size_t n = PLAIN_HEAD + C.plain_n;
	if (n + TRANSFER_HEAD > ONLINE_SYNC_MAX) { ended(CLOUD_TOO_BIG); return false; }
	uint8_t *plain = malloc(n);
	if (plain) {
		memcpy(plain, HEAD, sizeof HEAD);
		plain[4] = 0;
		memcpy(plain + PLAIN_HEAD, C.plain, C.plain_n);
		C.blob = transfer_seal(&C.slot.keys, plain, n, random16, &C.blob_n);
		free(plain);
	}
	if (!C.blob) { ended(CLOUD_NO_MEMORY); return false; }
	message(ONLINE_SYNC_PUSH, C.from, (uint32_t)C.blob_n, first);
	*out = C.blob;
	*out_n = C.blob_n;
	return true;
}

/* (the receipt moved on: a push kept, or the slot's own generation agreed) */
static void kept(uint32_t generation, uint32_t hash) {
	relay_lock();
	C.generation = generation;
	C.hash = hash;
	C.synced_at = now_s();
	C.receipt_due = true;
	relay_unlock();
}

static void pull_word(uint8_t status, uint32_t generation, uint32_t size) {
	if (status == ONLINE_SYNC_SAME) {
		relay_lock();
		C.synced_at = now_s();
		C.receipt_due = true;
		relay_unlock();
		finish(CLOUD_FINE);
	} else if (status == ONLINE_SYNC_FOUND) {
		if (size < TRANSFER_HEAD + PLAIN_HEAD || size > ONLINE_SYNC_MAX) finish(CLOUD_DAMAGED);
		else if (!relay_take_blob(size)) finish(CLOUD_NO_MEMORY);
		else C.found = generation;
	} else finish(CLOUD_DAMAGED);
}

static void push_word(uint8_t status, uint32_t generation) {
	if (status == ONLINE_SYNC_READY) relay_send_blob();
	else if (status == ONLINE_SYNC_STORED) {
		kept(generation, C.push_hash);
		finish(CLOUD_FINE);
	} else if (status == ONLINE_SYNC_CONFLICT) finish(CLOUD_CONFLICT);
	else finish(CLOUD_DAMAGED);
}

/* SYNC_STATE's word */
static void word(const uint8_t *d, size_t n) {
	if (d[0] != ONLINE_SYNC_STATE || n != ONLINE_SYNC_STATE_BYTES) { finish(CLOUD_DAMAGED); return; }
	uint32_t generation = (uint32_t)d[2] << 24 | (uint32_t)d[3] << 16 | (uint32_t)d[4] << 8 | d[5];
	uint32_t size = (uint32_t)d[6] << 24 | (uint32_t)d[7] << 16 | (uint32_t)d[8] << 8 | d[9];
	switch (d[1]) {
	case ONLINE_SYNC_NONE: finish(CLOUD_GONE); return;
	case ONLINE_SYNC_DENIED: finish(CLOUD_DENIED); return;
	case ONLINE_SYNC_BUSY: finish(CLOUD_BUSY); return;
	case ONLINE_SYNC_OFF: finish(CLOUD_OFF); return;
	case ONLINE_SYNC_DROPPED: finish(CLOUD_FINE); return;
	default: break;
	}
	CloudDoing at = doing();
	if (at == CLOUD_PULLING) pull_word(d[1], generation, size);
	else if (at == CLOUD_PUSHING) push_word(d[1], generation);
	else finish(CLOUD_DAMAGED);
}

/* A pull's blob in: opened, its saves left for SAVES */
static void taken(uint8_t *bytes, size_t n) {
	size_t plain = 0;
	if (!transfer_open(&C.slot.keys, bytes, n, &plain) || plain < PLAIN_HEAD || memcmp(bytes, HEAD, sizeof HEAD)) {
		free(bytes);
		finish(CLOUD_DAMAGED);
		return;
	}
	memmove(bytes, bytes + PLAIN_HEAD, plain - PLAIN_HEAD);
	relay_lock();
	free(C.pulled);
	C.pulled = bytes;
	C.pulled_n = plain - PLAIN_HEAD;
	C.pulled_gen = C.found;
	relay_unlock();
	finish(CLOUD_FINE);
}

static void lost(RelayLoss why) {
	ended(why == RELAY_REFUSED ? CLOUD_OFF : why == RELAY_DAMAGED ? CLOUD_DAMAGED :
		why == RELAY_NO_MEMORY ? CLOUD_NO_MEMORY : CLOUD_UNREACHED);
}

static const RelayJob JOB = { prepare, word, taken, lost };

static bool running(void) {
	relay_lock();
	bool r = C.running;
	relay_unlock();
	return r;
}

/* (the op waiting begun, where the relay is free) */
static void begin_wanted(void) {
	if (C.wanted == CLOUD_IDLE || running() || relay_busy()) return;
	if (C.wanted == CLOUD_PUSHING) {
		free(C.plain);
		C.plain = C.want_plain;
		C.plain_n = C.want_n;
		C.from = C.want_from;
		C.push_hash = C.want_hash;
		C.want_plain = NULL;
	}
	relay_lock();
	C.doing = C.wanted;
	C.trouble = CLOUD_FINE;
	C.running = true;
	relay_unlock();
	C.wanted = CLOUD_IDLE;
	if (!relay_start(&JOB)) ended(CLOUD_UNREACHED);
}

/* ---- the link ---- */

static void link_state(CloudState *out) {
	relay_poll();
	begin_wanted();
	bool due;
	relay_lock();
	due = C.receipt_due;
	C.receipt_due = false;
	relay_unlock();
	if (due) receipt_write();
	memset(out, 0, sizeof *out);
	relay_lock();
	out->linked = C.linked;
	out->doing = C.running ? C.doing : C.wanted;
	out->trouble = C.trouble;
	out->pulled = C.pulled != NULL;
	out->generation = C.generation;
	out->hash = C.hash;
	out->synced_at = C.synced_at;
	out->ends = C.ends;
	relay_unlock();
	if (out->doing != CLOUD_IDLE && running()) relay_progress(&out->moved, &out->size);
}

/* (whatever is under way or waiting given up) */
static void stop_all(void) {
	if (running()) relay_stop();
	ended(CLOUD_FINE);
	C.wanted = CLOUD_IDLE;
	free(C.want_plain);
	C.want_plain = NULL;
	relay_lock();
	free(C.pulled);
	C.pulled = NULL;
	relay_unlock();
}

static void forget(void) {
	stop_all();
	SDL_memset(C.secret, 0, sizeof C.secret);
	SDL_memset(&C.slot, 0, sizeof C.slot);
	relay_lock();
	C.linked = false;
	C.generation = C.hash = 0;
	C.synced_at = 0;
	relay_unlock();
	receipt_write();
}

static void linked_to(const uint8_t secret[TRANSFER_SECRET]) {
	stop_all();
	memcpy(C.secret, secret, TRANSFER_SECRET);
	transfer_slot(C.secret, &C.slot);
	relay_lock();
	C.linked = true;
	C.generation = C.hash = 0;
	C.synced_at = 0;
	relay_unlock();
	receipt_write();
}

static bool link_create(void) {
	uint8_t secret[32];
	if (!random16(secret) || !random16(secret + 16)) return false;
	linked_to(secret);
	SDL_memset(secret, 0, sizeof secret);
	return true;
}

static bool link_join(const uint8_t *bytes, size_t n) {
	if (n != LINK_BYTES || memcmp(bytes, LINK_MARK, sizeof LINK_MARK)) return false;
	linked_to(bytes + sizeof LINK_MARK);
	return true;
}

static uint8_t *link_bytes(size_t *n) {
	*n = 0;
	if (!C.linked) return NULL;
	uint8_t *b = malloc(LINK_BYTES);
	if (!b) return NULL;
	memcpy(b, LINK_MARK, sizeof LINK_MARK);
	memcpy(b + sizeof LINK_MARK, C.secret, TRANSFER_SECRET);
	*n = LINK_BYTES;
	return b;
}

static bool want(CloudDoing op) {
	if (!C.linked) return false;
	/* (a pull or a drop never waits behind another of its own) */
	if (C.wanted != CLOUD_IDLE && C.wanted != CLOUD_PUSHING && op != CLOUD_PUSHING) return true;
	C.wanted = op;
	begin_wanted();
	return true;
}

static bool link_pull(void) { return want(CLOUD_PULLING); }

static bool link_push(uint8_t *saves, size_t n, uint32_t from, uint32_t hash) {
	if (!saves || !C.linked) {
		free(saves);
		return false;
	}
	free(C.want_plain);
	C.want_plain = saves;
	C.want_n = n;
	C.want_from = from;
	C.want_hash = hash;
	return want(CLOUD_PUSHING);
}

static bool link_drop(void) {
	if (!C.linked) return false;
	stop_all();
	bool sent = want(CLOUD_DROPPING);
	/* (forgotten here at once: the slot is gone for this system whether
	 * the server heard or not, and goes by itself in 180 days unused) */
	C.linked = false;
	C.generation = C.hash = 0;
	receipt_write();
	return sent;
}

static void link_unlink(void) { forget(); }

static uint8_t *link_take_pulled(size_t *n, uint32_t *generation) {
	relay_lock();
	uint8_t *b = C.pulled;
	*n = C.pulled_n;
	*generation = C.pulled_gen;
	C.pulled = NULL;
	relay_unlock();
	return b;
}

static void link_synced(uint32_t generation, uint32_t hash) {
	kept(generation, hash);
	relay_lock();
	C.receipt_due = false;
	relay_unlock();
	receipt_write();
}

static void link_wait(uint32_t ms) {
	uint32_t since = SDL_GetTicks();
	do {
		begin_wanted();
		relay_wait(ms > 50 ? 50 : ms);
	} while ((running() || C.wanted != CLOUD_IDLE) && SDL_GetTicks() - since < ms);
	CloudState s;
	link_state(&s);
}

static const CloudLink LINK = {
	link_state, link_create, link_join, link_bytes, link_unlink, link_pull, link_push, link_drop,
	link_take_pulled, link_synced, link_wait,
};

const CloudLink *online_cloud_link(const char *url) {
	if (!relay_here() || !url || SDL_strlen(url) >= NETWORK_URL) return NULL;
	relay_use(url);
	receipt_read();
	return &LINK;
}
