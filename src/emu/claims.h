/* The bytes each installer writes into the core's copy as the core starts
 * (issue #82: the families' room and the families, BN5's battlefields,
 * lava and sea, the chip registry; free_space.h's plan), claimed as they
 * are written, so that no two installers write the same bytes: the free
 * space they fill, BN6's own words and instructions they change, RAM they
 * keep patched and the instructions their hooks stand on. Every hook set
 * is claimed too, an installer's as its own and any other as the hooks',
 * so that no hook stands on bytes an installer changed. A claim that
 * overlaps another's bytes is told on stderr and counted; the installers
 * check their own bytes before they write, so this is what tells two of
 * them apart where both checks would pass. */
#ifndef CW_CLAIMS_H
#define CW_CLAIMS_H

#include <stdbool.h>
#include <stdint.h>

#define CLAIMS_HOOKS "the hooks"   /* who a hook set outside any installer's writes is */

/* An installer's writes, from claims_begin to claims_end (nested: the
 * innermost's), are its own; outside, writes claim nothing. */
void claims_begin(const char *who);
void claims_end(void);
/* `len` bytes at `at` written: claimed by the installer writing, where one
 * is (emu.c's writes call it); false where they overlap another's. */
bool claims_note(uint32_t at, uint32_t len);
/* A hook's instruction at `at`: claimed by the installer writing, or by
 * CLAIMS_HOOKS outside one; false where it overlaps another's. */
bool claims_hook(uint32_t at);
/* Who claimed the byte at `at` (NULL: no one), and the overlaps told so
 * far. */
const char *claims_who(uint32_t at);
int claims_overlaps(void);
/* On stderr: the ranges claimed, by how many writers, and the overlaps
 * (CYBERWORLD_EMU_DEBUG's line once the core's installers have run). */
void claims_report(void);
/* All forgotten (a test's next case); and the overlaps counted without a
 * word on stderr (a test's own, made on purpose). */
void claims_reset(void);
void claims_quiet(bool on);

#endif
