/* claims.h: the installers' bytes, a list of ranges by who wrote them,
 * each new range looked for among the others'. */
#include "claims.h"

#include <stdio.h>
#include <string.h>

#define CLAIMS_MAX 2048   /* ranges kept: a boot's installers write some hundreds, contiguous ones joined */
#define DEPTH_MAX  4      /* installers within installers (the families' within xvirus_install, the panels' within the fields') */

typedef struct {
	uint32_t at, end;   /* [at, end) */
	const char *who;
} Claim;

static Claim claims[CLAIMS_MAX];
static int nclaims, overlaps;
static const char *writer[DEPTH_MAX];
static int depth;
static bool full, quiet;

void claims_begin(const char *who) {
	if (depth < DEPTH_MAX) writer[depth] = who;
	++depth;
}

void claims_end(void) {
	if (depth > 0) --depth;
}

static const char *current(void) { return depth > 0 && depth <= DEPTH_MAX ? writer[depth - 1] : NULL; }

/* (two names, the same installer: the same string, or the same words) */
static bool same(const char *a, const char *b) { return a == b || (a && b && !strcmp(a, b)); }

/* [at, end) claimed by `who`: another's overlapping range told; joined to
 * a range of its own it touches */
static bool claim(const char *who, uint32_t at, uint32_t end) {
	bool ok = true;
	for (int i = 0; i < nclaims; ++i) {
		const Claim *c = &claims[i];
		if (at < c->end && c->at < end && !same(c->who, who)) {
			if (!quiet)
				fprintf(stderr, "claims: %s writes %08x-%08x, where %s wrote %08x-%08x\n", who, (unsigned)at, (unsigned)(end - 1), c->who,
					(unsigned)c->at, (unsigned)(c->end - 1));
			++overlaps;
			ok = false;
		}
	}
	for (int i = 0; i < nclaims; ++i) {
		Claim *c = &claims[i];
		if (same(c->who, who) && at <= c->end && c->at <= end) {
			if (at < c->at) c->at = at;
			if (end > c->end) c->end = end;
			return ok;
		}
	}
	if (nclaims < CLAIMS_MAX) claims[nclaims++] = (Claim){ at, end, who };
	else if (!full) {
		fprintf(stderr, "claims: more than %d ranges, the rest not looked at\n", CLAIMS_MAX);
		full = true;
	}
	return ok;
}

bool claims_note(uint32_t at, uint32_t len) {
	const char *who = current();
	if (!who || !len) return true;
	return claim(who, at, at + len);
}

bool claims_hook(uint32_t at) {
	const char *who = current();
	at &= ~1u;
	return claim(who ? who : CLAIMS_HOOKS, at, at + 2);
}

const char *claims_who(uint32_t at) {
	for (int i = 0; i < nclaims; ++i)
		if (at >= claims[i].at && at < claims[i].end) return claims[i].who;
	return NULL;
}

int claims_overlaps(void) { return overlaps; }

void claims_report(void) {
	int writers = 0;
	for (int i = 0; i < nclaims; ++i) {
		int k = 0;
		while (k < i && !same(claims[k].who, claims[i].who)) ++k;
		writers += k == i;
	}
	fprintf(stderr, "claims: %d ranges of %d writers (the installers and the hooks), %d over another's\n", nclaims, writers, overlaps);
}

void claims_reset(void) {
	nclaims = overlaps = depth = 0;
	full = false;
}

void claims_quiet(bool on) { quiet = on; }
