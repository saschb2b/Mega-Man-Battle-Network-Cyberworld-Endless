/* tools/heap3ds.c: the New 3DS's heap modelled on a computer, preloaded
 * (LD_PRELOAD) into the host build by tools/heap3ds.py (3ds/README.md,
 * Testing on a 3DS; docs/engine/emulation/3ds-memory.md).
 *
 * The game's allocations are the same on both: the ROMs' copies, the
 * tiles a layer's area learns, the installs. What differs is held apart:
 * the heap is HEAP3DS_CAP bytes (96 MB, start_3ds.c's split), HEAP3DS_FIXED
 * of it taken before the game's start (mGBA's ROM buffer, 16 MB, where it
 * is not given back), and the core's ROM copy (HEAP3DS_KEEP bytes, 16 MB),
 * which a computer's core copies into memory of its own and frees, stays
 * on the 3DS for the core's life (src/emu/emu.c). Every block counts as
 * much as malloc_usable_size gives (newlib's chunks are alike); one that
 * would take the heap past its size is refused (NULL), as the 3DS's malloc
 * refuses it, and the first few refusals are said with where they were
 * asked for. Fragmentation is not modelled: the 3DS's own number is the
 * log's heap line (start_3ds.c). At the end: the peak, the end's use and
 * the refusals, on stderr. */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <errno.h>
#include <execinfo.h>
#include <malloc.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern void *__libc_malloc(size_t);
extern void *__libc_calloc(size_t, size_t);
extern void *__libc_realloc(void *, size_t);
extern void __libc_free(void *);
extern void *__libc_memalign(size_t, size_t);

static long cap = 96L << 20, keep_size = 16L << 20;
static atomic_long live, peak, refused;
static void *_Atomic kept;   /* (the core's ROM copy: freed on a computer, kept on the 3DS) */
static atomic_int said, set;

static long env_bytes(const char *name, long otherwise) {
	const char *v = getenv(name);
	return v && *v ? strtol(v, NULL, 0) : otherwise;
}

static void say(const char *s, size_t n);
static void frames(int skip);

/* (a crash said where it was, as a 3DS's would be in Luma3DS's dump) */
static void crashed(int sig) {
	char s[64];
	int k = snprintf(s, sizeof s, "heap3ds: crashed (signal %d)\n", sig);
	say(s, (size_t)k);
	frames(2);
	_exit(128 + sig);
}

static void setup(void) {
	int no = 0;
	if (!atomic_compare_exchange_strong(&set, &no, 1)) return;
	signal(SIGSEGV, crashed);
	signal(SIGABRT, crashed);
	cap = env_bytes("HEAP3DS_CAP", cap);
	keep_size = env_bytes("HEAP3DS_KEEP", keep_size);
	atomic_store(&live, env_bytes("HEAP3DS_FIXED", 0));
	atomic_store(&peak, atomic_load(&live));
}

static void say(const char *s, size_t n) {
	if (write(2, s, n) < 0) {}
}
/* (the frames that asked, past `skip`: module+offset, which heap3ds.py
 * names with addr2line) */
static void frames(int skip) {
	void *bt[12];
	int m = backtrace(bt, 12);
	for (int i = skip; i < m; ++i) {
		Dl_info di;
		if (!dladdr(bt[i], &di) || !di.dli_fname) continue;
		char s[160];
		int k = snprintf(s, sizeof s, "heap3ds:   at %s+0x%lx\n", di.dli_fname, (unsigned long)((char *)bt[i] - (char *)di.dli_fbase));
		say(s, (size_t)k);
	}
}

/* (a refusal said, with the frames that asked for it) */
static void refusal(size_t n) {
	atomic_fetch_add(&refused, 1);
	if (atomic_fetch_add(&said, 1) >= 8) return;
	char s[160];
	int k = snprintf(s, sizeof s, "heap3ds: refused %zu bytes with %.2f MB in use\n", n, atomic_load(&live) / 1048576.0);
	say(s, (size_t)k);
	frames(3);
}

/* Whether block `p` (just given) fits: counted, or freed and refused */
static void *admit(void *p, size_t n) {
	if (!p) return NULL;
	setup();
	long size = (long)malloc_usable_size(p);
	long now = atomic_fetch_add(&live, size) + size;
	if (now > cap) {
		atomic_fetch_sub(&live, size);
		__libc_free(p);
		refusal(n);
		errno = ENOMEM;
		return NULL;
	}
	long was = atomic_load(&peak);
	while (now > was && !atomic_compare_exchange_weak(&peak, &was, now)) {}
	if ((long)n == keep_size && !atomic_load(&kept)) atomic_store(&kept, p);
	return p;
}

static void release(void *p) {
	if (!p) return;
	void *k = p;
	if (atomic_load(&kept) == p && atomic_compare_exchange_strong(&kept, &k, (void *)1)) { __libc_free(p); return; }
	atomic_fetch_sub(&live, (long)malloc_usable_size(p));
	__libc_free(p);
}

void *malloc(size_t n) { return admit(__libc_malloc(n), n); }
void *calloc(size_t a, size_t b) { return admit(__libc_calloc(a, b), a * b); }
void free(void *p) { release(p); }
void *memalign(size_t a, size_t n) { return admit(__libc_memalign(a, n), n); }
void *aligned_alloc(size_t a, size_t n) { return memalign(a, n); }

int posix_memalign(void **out, size_t a, size_t n) {
	void *p = memalign(a, n);
	if (!p) return ENOMEM;
	*out = p;
	return 0;
}

void *realloc(void *q, size_t n) {
	if (!q) return malloc(n);
	if (!n) { free(q); return NULL; }
	setup();
	long was = (long)malloc_usable_size(q);
	/* (grown past the heap: refused, the old block kept, as C's realloc) */
	if (atomic_load(&live) - was + (long)n > cap) { refusal(n); errno = ENOMEM; return NULL; }
	void *p = __libc_realloc(q, n);
	if (!p) return NULL;
	long now = atomic_fetch_add(&live, (long)malloc_usable_size(p) - was) + (long)malloc_usable_size(p) - was;
	long top = atomic_load(&peak);
	while (now > top && !atomic_compare_exchange_weak(&peak, &top, now)) {}
	return p;
}

__attribute__((destructor)) static void bye(void) {
	char s[200];
	int k = snprintf(s, sizeof s, "heap3ds: peak %.2f MB of %.2f MB, %.2f MB in use at the end, %ld refused\n",
		atomic_load(&peak) / 1048576.0, cap / 1048576.0, atomic_load(&live) / 1048576.0, atomic_load(&refused));
	say(s, (size_t)k);
}
