/* The installer's staged writes (xvirus.h, issue #82): a step's writes into
 * the core's ROM copy are kept until every one of its checks has passed,
 * then made at once, or none where any failed; and what was moved where,
 * for a trace's reader. Shared by the engine's room grown once a core
 * (xvirus_grow.c) and each family's install (xvirus.c). */
#ifndef CW_XVIRUS_TX_H
#define CW_XVIRUS_TX_H

#include <stdbool.h>
#include <stdint.h>

#include "thumb_reloc.h"

/* A step begun: nothing staged, nothing failed (its buffer taken where
 * none is held); and ended, the buffer let go. */
void xv_tx_begin(void);
void xv_tx_end(void);
/* The step failed: the first reason kept (xv_why); false, for a caller's
 * return. */
bool xv_fail(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
bool xv_failed(void);
const char *xv_why(void);
/* Room for len bytes to be written at `at`, zeroed; `fresh`: where the
 * core's copy must still be free there (0xFF, as emu.c pads it). NULL once
 * the step has failed. */
uint8_t *xv_put(uint32_t at, uint32_t len, bool fresh);
/* A word (a halfword) of BN6's own, or of what is already installed,
 * changed: it must read `old` in the core's copy now. */
void xv_word(uint32_t at, uint32_t old, uint32_t v);
void xv_half(uint32_t at, uint16_t old, uint16_t v);
/* An instruction of BN6's own given another immediate: it must be the one
 * named (its form, registers and immediate `was`) in the core's copy now. */
void xv_insn(uint32_t at, ThumbForm form, uint8_t rd, uint8_t rn, uint16_t was, uint16_t now);
/* the bytes staged so far */
uint32_t xv_used(void);
/* The step's writes made, in the order they were staged. */
void xv_commit(void);
/* ... each claimed as its writer's (claims.h): `writer` names who for the
 * address it is staged at (a Navi's install writes the families' grown
 * tables as theirs, its own room as its own) */
void xv_commit_by(const char *(*writer)(uint32_t at));

/* What was moved where (xvirus.h's xvirus_moved): an address range of a
 * family's game, or of BN6's own table, and its copy. */
void xv_moved_add(uint32_t from, uint32_t to, uint32_t len);

#endif
