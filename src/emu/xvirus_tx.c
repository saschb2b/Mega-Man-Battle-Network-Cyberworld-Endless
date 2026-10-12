/* xvirus_tx.h: the staged writes, kept in a buffer of their own while a
 * step lasts, and the ranges moved. */
#include "xvirus_tx.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bytes.h"
#include "emu.h"
#include "xvirus.h"

#define TX_BYTES  0x80000   /* a step's bytes at most: the engine's room grown (its reward rows 40 KB), a family with its sprites,
                               or the chips' units with theirs (past 245 KB with ElemRage's), held while the step lasts */
#define TX_WRITES 512       /* (the chips' units' writes past 256 with CusVolt's: each unit's blocks, veneers, entry, sounds and art) */

static struct {
	uint8_t *bytes;
	uint32_t used;
	int n;
	struct { uint32_t at, off, len; } w[TX_WRITES];
	bool failed;
	char why[240];
} tx;

void xv_tx_begin(void) {
	if (!tx.bytes) tx.bytes = malloc(TX_BYTES);
	tx.used = 0;
	tx.n = 0;
	tx.failed = false;
	tx.why[0] = 0;
	if (!tx.bytes) xv_fail("no memory for its %u bytes", (unsigned)TX_BYTES);
}

void xv_tx_end(void) {
	free(tx.bytes);
	tx.bytes = NULL;
}

bool xv_fail(const char *fmt, ...) {
	if (!tx.failed) {
		va_list ap;
		va_start(ap, fmt);
		vsnprintf(tx.why, sizeof tx.why, fmt, ap);
		va_end(ap);
	}
	tx.failed = true;
	return false;
}

bool xv_failed(void) { return tx.failed; }
const char *xv_why(void) { return tx.why; }
uint32_t xv_used(void) { return tx.used; }

uint8_t *xv_put(uint32_t at, uint32_t len, bool fresh) {
	if (tx.failed) return NULL;
	if (tx.n >= TX_WRITES) { xv_fail("more writes than %d", TX_WRITES); return NULL; }
	if (len > TX_BYTES - tx.used) { xv_fail("more to write than %u bytes", (unsigned)TX_BYTES); return NULL; }
	if (fresh)
		for (uint32_t i = 0; i < len; ++i)
			if (emu_read8(at + i) != 0xFF) { xv_fail("%08x is not free", (unsigned)(at + i)); return NULL; }
	tx.w[tx.n].at = at;
	tx.w[tx.n].off = tx.used;
	tx.w[tx.n].len = len;
	++tx.n;
	uint8_t *p = tx.bytes + tx.used;
	memset(p, 0, len);
	tx.used += (len + 3) & ~3u;
	return p;
}

void xv_word(uint32_t at, uint32_t old, uint32_t v) {
	uint32_t now = emu_read32(at);
	if (now != old) { xv_fail("%08x reads %08x, not %08x", (unsigned)at, (unsigned)now, (unsigned)old); return; }
	uint8_t *p = xv_put(at, 4, false);
	if (p) put32(p, v);
}

void xv_half(uint32_t at, uint16_t old, uint16_t v) {
	uint16_t now = emu_read16(at);
	if (now != old) { xv_fail("%08x reads %04x, not %04x", (unsigned)at, (unsigned)now, (unsigned)old); return; }
	uint8_t *p = xv_put(at, 2, false);
	if (p) put16(p, v);
}

void xv_insn(uint32_t at, ThumbForm form, uint8_t rd, uint8_t rn, uint16_t was, uint16_t now) {
	uint16_t insn = emu_read16(at), old = insn;
	ThumbImm want = { form, rd, rn, was };
	if (!thumb_imm_patch(&insn, &want, now)) { xv_fail("the instruction at %08x (%04x) is not the one expected", (unsigned)at, (unsigned)old); return; }
	xv_half(at, old, insn);
}

void xv_commit(void) {
	for (int i = 0; i < tx.n; ++i) emu_write(tx.w[i].at, tx.bytes + tx.w[i].off, tx.w[i].len);
}

void xv_commit_where(bool (*part)(uint32_t at)) {
	for (int i = 0; i < tx.n; ++i)
		if (part(tx.w[i].at)) emu_write(tx.w[i].at, tx.bytes + tx.w[i].off, tx.w[i].len);
}

/* ---- what was moved where ---- */

static XvMoved moved[XV_MOVED_MAX];
static int nmoved;

int xvirus_moved(XvMoved *out, int max) {
	int n = nmoved < max ? nmoved : max;
	memcpy(out, moved, (size_t)n * sizeof *out);
	return n;
}

void xv_moved_add(uint32_t from, uint32_t to, uint32_t len) {
	for (int i = 0; i < nmoved; ++i)
		if (moved[i].from == from && moved[i].to == to) return;
	if (nmoved < XV_MOVED_MAX) moved[nmoved++] = (XvMoved){ from, to, len };
	else fprintf(stderr, "xvirus: %08x moved to %08x past the %d ranges a trace is told of\n", (unsigned)from, (unsigned)to, XV_MOVED_MAX);
}
