/* Each player's look in a NetBattle (issue #854; docs/online/arena/looks.md).
 * Both players' looks are written into every Arena core's ROM copy before
 * it starts, in palettes of MegaMan's battle block BN6 never picks
 * (BN6_LOOK_PAL_SPARE), and three hooks on each core answer BN6's picks of
 * his palette by the object's player: his body (plain, pale, Full Synchro)
 * and the emotion window's face. A core's own player stands on alliance 0
 * of his GBA, so on core k alliance a is player k ^ a. Every client and
 * spectator that builds the pair with the same two looks has the same
 * bytes and the same hooks: the pair's hash holds them to it, and takes
 * the looks in (arena_looks_hash) so two games given different looks part
 * at the first hash. A Cross, a Soul and Beast Out keep BN6's colours. */
#ifndef CW_ARENA_LOOKS_H
#define CW_ARENA_LOOKS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "looks.h"

struct mCore;

/* The two players' looks, by the cable's side (side 0 the first GBA's):
 * from the next pair made on. NULL, or two of BN6's own colours: none, the
 * cores as BN6 draws them. A look not look_valid is BN6's. */
void arena_looks_set(const Look *side0, const Look *side1);
/* Whether a look is set (a core takes the palettes and the hooks) */
bool arena_looks_on(void);
/* The palettes into a core's ROM copy (`size` bytes from 0x08000000),
 * after BN5's installs: the same bytes on every core and every client */
void arena_looks_rom(uint8_t *rom, size_t size);
/* The picks' hooks on `core`, the cable's GBA `side`: false where one
 * could not be had */
bool arena_looks_hooks(struct mCore *core, unsigned side);
/* The looks, folded into a pair's hash (`h` as it stands): `h` as it was
 * where none is set */
uint64_t arena_looks_hash(uint64_t h);
/* A bout's looks for the player on the cable's `side`: `given`, --dev
 * arenalook=MINE[/THEIRS] (12 hex digits each, a Look's six bytes in its
 * order; THEIRS left out, BN6's), where set; else BN6's own on both sides,
 * as neither game yet knows the other's look (the Square's wire carries
 * none before looks-square's version), and both games must agree */
void arena_looks_bout(unsigned side, const char *given);
/* CYBERWORLD_ARENA_LOOKS_PROBE's measurement: a frame of a core seen, and
 * what was seen said (the pair's close) */
void arena_looks_probe_frame(struct mCore *core, unsigned side);
void arena_looks_probe_report(void);
/* a look read from 12 hex digits, false where it is not one */
bool arena_looks_parse(const char *text, Look *out);

#endif
