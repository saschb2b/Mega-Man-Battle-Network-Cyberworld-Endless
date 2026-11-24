/* Each seat's MegaMan in his own player's look on the shared dive's field
 * (issue #879; docs/online/shared-dive.md, Looks on the field): the field
 * built from BN6's own colours whatever each game wears, then every seat's
 * look written into the battle sprite's spare palettes, as a NetBattle's
 * are (arena_looks.h), and BN6's picks of his palette answered by seat. */
#ifndef CW_PARTY_LOOKS_H
#define CW_PARTY_LOOKS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "hook.h"
#include "online_snapshot.h"
#include "party_field.h"

struct mCore;

/* The field's ROM copy (`size` bytes from 0x08000000): BN6's own colours
 * back where this game's look was written, and seats 0..n-1's looks (the
 * looked ones: OnlineLoadout's `looked`) into the spare palettes */
void party_looks_rom(uint8_t *rom, size_t size, const OnlineLoadout *seat, unsigned n);
/* The picks' hooks on the field's core where any seat is looked (true
 * where none is): `f` the field, whose slots say which seat an object is */
bool party_looks_hooks(HookCore *core, PartyField *f);
/* The state in words: `partylooks` and each seat's, `-` BN6's colours,
 * `drawn` where his palette is in the field's OBJ palettes this frame,
 * else `unseen` */
void party_looks_describe(FILE *out, struct mCore *field, unsigned n);

#endif
