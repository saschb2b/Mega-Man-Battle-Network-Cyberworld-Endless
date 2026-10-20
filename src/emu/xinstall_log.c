/* The installs' record (xinstall_log.h): the ROM's bytes as runs in one
 * pool, a run joined to the last where it follows on, and the hooks in a
 * list of their own. */
#include "xinstall_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __3DS__
#include "devtools.h"
#endif

#define BUS      0x08000000u
#define ROM_BUS  0x02000000u   /* the ROM's span on the bus, mirrored at 0x0A and 0x0C (its wait states) */

typedef struct { uint32_t at, len; size_t off; } Run;
typedef struct { uint32_t at; EmuHook fn; void *user; } Logged;

static struct {
	int depth;
	Run *runs;
	int nruns, cap_runs;
	uint8_t *pool;
	size_t used, cap;
	Logged hooks[HOOK_MAX];
	int nhooks;
	bool failed;            /* (memory ran out: the record is not whole, and says so by its hash, 0) */
	uint64_t hash;          /* 0: to be made */
} L;

/* (kept where the Arena's cores read it: every system that offers
 * NetBattles; on the New 3DS, which offers none, its 2 MB stay out of a
 * run's heap but for --dev rollbench and --dev arena, which run the pair
 * there: src/online/arena_pair.h, CW_ARENA) */
static bool kept(void) {
#ifdef __3DS__
	return dev.rollbench || dev.arena;
#else
	return true;
#endif
}

void xlog_begin(void) { ++L.depth; }
void xlog_end(void) { if (L.depth > 0) --L.depth; }

static bool grow(void **p, size_t *cap, size_t need, size_t unit) {
	if (need <= *cap) return true;
	size_t n = *cap ? *cap : 64;
	while (n < need) n *= 2;
	void *q = realloc(*p, n * unit);
	if (!q) return false;
	*p = q;
	*cap = n;
	return true;
}

void xlog_write(uint32_t at, const void *data, size_t len) {
	if (!L.depth || !len || L.failed || !kept()) return;
	/* (the ROM's alone: xinstall_log.h) */
	if (at < BUS || at >= BUS + 3 * ROM_BUS) return;
	at = BUS + (at - BUS) % ROM_BUS;
	L.hash = 0;
	size_t cap_runs = (size_t)L.cap_runs;
	Run *last = L.nruns ? &L.runs[L.nruns - 1] : NULL;
	bool joined = last && last->at + last->len == at && last->off + last->len == L.used;
	if (!grow((void **)&L.pool, &L.cap, L.used + len, 1) || (!joined && !grow((void **)&L.runs, &cap_runs, (size_t)L.nruns + 1, sizeof *L.runs))) {
		fprintf(stderr, "xlog: out of memory: the installs' record is not whole\n");
		L.failed = true;
		return;
	}
	L.cap_runs = (int)cap_runs;
	memcpy(L.pool + L.used, data, len);
	if (joined) L.runs[L.nruns - 1].len += (uint32_t)len;
	else L.runs[L.nruns++] = (Run){ at, (uint32_t)len, L.used };
	L.used += len;
}

void xlog_hook(uint32_t at, EmuHook fn, void *user) {
	if (!L.depth || !kept()) return;
	L.hash = 0;
	if (L.nhooks == HOOK_MAX) { L.failed = true; return; }
	L.hooks[L.nhooks++] = (Logged){ at, fn, user };
}

void xlog_event(uint32_t at) {
	if (L.depth) fprintf(stderr, "xlog: an event hook at %08x while an install records: not carried\n", (unsigned)at);
}

size_t xlog_bytes(void) { return L.used; }
int xlog_hooks(void) { return L.nhooks; }

static void hash_bytes(uint64_t *h, const void *data, size_t n) {
	const uint8_t *b = data;
	for (size_t i = 0; i < n; ++i) {
		*h ^= b[i];
		*h *= UINT64_C(1099511628211);
	}
}

static void hash_word(uint64_t *h, uint32_t w) {
	const uint8_t b[4] = { (uint8_t)w, (uint8_t)(w >> 8), (uint8_t)(w >> 16), (uint8_t)(w >> 24) };
	hash_bytes(h, b, sizeof b);
}

uint64_t xlog_hash(void) {
	if (L.failed || (!L.nruns && !L.nhooks)) return 0;
	if (L.hash) return L.hash;
	uint64_t h = UINT64_C(14695981039346656037);
	for (int i = 0; i < L.nruns; ++i) {
		hash_word(&h, L.runs[i].at);
		hash_word(&h, L.runs[i].len);
		hash_bytes(&h, L.pool + L.runs[i].off, L.runs[i].len);
	}
	/* (the hooks by their places: their routines are this build's, the same in every game of it) */
	for (int i = 0; i < L.nhooks; ++i) hash_word(&h, L.hooks[i].at);
	return L.hash = h ? h : 1;
}

void xlog_apply(uint8_t *rom, size_t size) {
	for (int i = 0; i < L.nruns; ++i) {
		size_t off = L.runs[i].at - BUS;
		if (off < size && L.runs[i].len <= size - off) memcpy(rom + off, L.pool + L.runs[i].off, L.runs[i].len);
	}
}

bool xlog_hook_core(struct mCore *core) {
	if (!L.nhooks) return true;
	HookCore *c = hook_core(core);
	bool ok = c != NULL;
	for (int i = 0; ok && i < L.nhooks; ++i) ok = hook_on(c, L.hooks[i].at, L.hooks[i].fn, L.hooks[i].user);
	return ok;
}

void xlog_reset(void) {
	free(L.runs);
	free(L.pool);
	memset(&L, 0, sizeof L);
}
