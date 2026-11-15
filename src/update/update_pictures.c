/* update_pictures.h. The file: "CWPX", its form (1) and count, then each
 * picture's width, height, colours (at most 256) and packed length, little
 * endian, its palette (RGB, a byte each) and its pixels, a byte each, in
 * the GBA's LZ77 (rom.c's lz77_decompress). */
#include "update_pictures.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "analytics_http.h"
#include "rom.h"
#include "update_manifest.h"
#include "update_sha256.h"

/* (a thread of the game's that fetches, and the heap for them) */
#if defined(ANALYTICS_HTTP) && !defined(__3DS__)
#define PICTURES_HERE 1
#endif
#define PICTURES_MAX (2 << 20)   /* (the file's bytes at most: the tool keeps it to 1 MB) */
#define HEAD 8
#define ENTRY 10

static struct {
	UpdateRelease file;     /* where they are, from update.json (written before `found`) */
	char agent[64];
	uint8_t *data;          /* the file, whole and checked (written before `ready`) */
	size_t n;
	SDL_atomic_t found, started, ready, busy, stop;
} X;

void pictures_found(const char *manifest, const char *agent) {
	if (SDL_AtomicGet(&X.found) || !update_manifest_read(manifest, "pictures", &X.file)) return;
	snprintf(X.agent, sizeof X.agent, "%s", agent);
	if (X.file.size > 0 && X.file.size <= PICTURES_MAX) SDL_AtomicSet(&X.found, 1);
}

bool update_pictures_here(void) {
#ifdef PICTURES_HERE
	return SDL_AtomicGet(&X.found) != 0;
#else
	return false;
#endif
}

#ifdef PICTURES_HERE

typedef struct {
	uint8_t *data;
	size_t n;
	Sha256 hash;
} Fetching;

static bool keep(void *user, const void *data, size_t n) {
	Fetching *f = user;
	if (f->n + n > (size_t)X.file.size) return false;
	memcpy(f->data + f->n, data, n);
	sha256_feed(&f->hash, data, n);
	f->n += n;
	return true;
}

static int fetch(void *arg) {
	(void)arg;
	Fetching f = { malloc((size_t)X.file.size), 0, { { 0 }, 0, { 0 } } };
	sha256_begin(&f.hash);
	char hex[65];
	bool ok = f.data && analytics_http_get(X.file.url, X.agent, keep, &f, &X.stop);
	sha256_end(&f.hash, hex);
	ok = ok && f.n == (size_t)X.file.size && !strcmp(hex, X.file.sha256) && f.n >= HEAD && !memcmp(f.data, "CWPX", 4);
	printf("update: the notes' pictures %s\n", ok ? "fetched" : "not fetched");
	if (ok) {
		X.data = f.data;
		X.n = f.n;
		SDL_AtomicSet(&X.ready, 1);
	} else free(f.data);
	SDL_AtomicSet(&X.busy, 0);
	return 0;
}

void update_pictures_want(void) {
	if (!SDL_AtomicGet(&X.found) || !SDL_AtomicCAS(&X.started, 0, 1)) return;
	SDL_AtomicSet(&X.busy, 1);
	SDL_Thread *t = SDL_CreateThreadWithStackSize(fetch, "update pictures", 256 * 1024, NULL);
	if (t) SDL_DetachThread(t);
	else SDL_AtomicSet(&X.busy, 0);
}

void pictures_end(void) {
	SDL_AtomicSet(&X.stop, 1);
	for (int i = 0; i < 30 && SDL_AtomicGet(&X.busy); ++i) SDL_Delay(50);
}

#else

void update_pictures_want(void) {}

void pictures_end(void) {}

#endif

static unsigned le16(const uint8_t *p) { return p[0] | p[1] << 8; }
static size_t le32(const uint8_t *p) { return (size_t)p[0] | (size_t)p[1] << 8 | (size_t)p[2] << 16 | (size_t)p[3] << 24; }

/* picture `i`'s entry in the file: its head, NULL where the file has none */
static const uint8_t *entry(int i) {
	if (!SDL_AtomicGet(&X.ready) || i < 0 || (unsigned)i >= le16(X.data + 6)) return NULL;
	size_t at = HEAD;
	for (int k = 0; ; ++k) {
		if (at + ENTRY > X.n) return NULL;
		const uint8_t *e = X.data + at;
		size_t size = ENTRY + le16(e + 4) * 3u + le32(e + 6);
		if (at + size > X.n) return NULL;
		if (k == i) return e;
		at += size;
	}
}

bool update_picture(int i, int *w, int *h, uint32_t **argb) {
	const uint8_t *e = entry(i);
	if (!e) return false;
	unsigned pw = le16(e), ph = le16(e + 2), colours = le16(e + 4);
	size_t packed = le32(e + 6), n = 0;
	const uint8_t *pal = e + ENTRY;
	if (!pw || !ph || pw > 240 || ph > 160 || !colours || colours > 256) return false;
	uint8_t *idx = lz77_decompress(pal + colours * 3, packed, &n);
	uint32_t *px = idx && n == (size_t)pw * ph ? malloc(n * sizeof *px) : NULL;
	for (size_t k = 0; px && k < n; ++k) {
		unsigned c = idx[k] < colours ? idx[k] : 0;
		px[k] = 0xFF000000u | (uint32_t)pal[c * 3] << 16 | (uint32_t)pal[c * 3 + 1] << 8 | pal[c * 3 + 2];
	}
	free(idx);
	if (!px) return false;
	*w = (int)pw;
	*h = (int)ph;
	*argb = px;
	return true;
}
