/* The tracks' installs written down as they are made on BN6's core
 * (issue #120, docs/online/arena/from-square/full-rules.md): every byte BN5's
 * installs write into the core's ROM copy (xinstall.h's tracks, the chip
 * registry, xchips_install.h, the Souls' soulproxy.h and soulunite.h) and
 * every hook they set, in their order, so
 * that the Arena's cores (src/online/arena_pair.c) get the same: the bytes
 * into each one's ROM copy before it starts, the hooks onto each one's own
 * table (hook_core). Two games with the same ROMs and the same build make
 * the same record, which its hash says.
 *
 * The record is the ROM's alone: what an install writes into RAM is kept
 * patched by the core (emu_keep) where the game copies it from a ROM image
 * the install patched as well, so a core started on the record's ROM has
 * it. Event hooks are the director's and are left out; so is the record
 * on the New 3DS, which offers no NetBattle, but for --dev rollbench and
 * --dev arena. */
#ifndef CW_XINSTALL_LOG_H
#define CW_XINSTALL_LOG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hook.h"

struct mCore;

/* An install's writes and hooks, from begin to end (nested), recorded */
void xlog_begin(void);
void xlog_end(void);
/* (emu.c's writes and hooks, while one records) */
void xlog_write(uint32_t at, const void *data, size_t len);
void xlog_hook(uint32_t at, EmuHook fn, void *user);
/* An event hook met while one records: not carried (told on stderr) */
void xlog_event(uint32_t at);

/* The record's ROM bytes and hooks, and its hash (0: nothing recorded) */
size_t xlog_bytes(void);
int xlog_hooks(void);
uint64_t xlog_hash(void);
/* The recorded bytes into a ROM copy of `size` bytes (the bus's 0x08000000
 * at its start), in the order written */
void xlog_apply(uint8_t *rom, size_t size);
/* The recorded hooks onto `core`'s own table (hook_core): false where its
 * table could not be had or a hook not set */
bool xlog_hook_core(struct mCore *core);
/* All forgotten (a test's next case) */
void xlog_reset(void);

#endif
