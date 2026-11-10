/* Another game's NaviCust program as one of BN6's (docs/rom/navicust.md,
 * Program 48; docs/multirom/gaps.md, NaviCust programs): BN5's SoulT+1,
 * BN6's program 48 where BN5's ROM is read. What BN5's ROM says of it (its
 * record's four colour variants, its shape set in BN6's seven by seven,
 * its name and its description's lines in BN6's codes), and how BN6's
 * text takes it: the names' archive with its name in a filler's entry,
 * the descriptions' archive grown by an entry. The numbers and the text are read, never kept here.
 * ROM-free but for xprogram_soult, which reads rom.h's XR. */
#ifndef CW_XPROGRAMS_H
#define CW_XPROGRAMS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define XPROGRAM_SOULT    48   /* BN6's number for BN5's SoulT+1: the first past the NaviCust list's RUN item, 47 */
#define XPROGRAM_PROGRAMS 49   /* BN6's programs' numbers with it: 0-48 */
#define XPROGRAM_VARIANTS 4    /* a program's colour variants */
#define XPROGRAM_SHAPE    49   /* a shape in BN6's form: seven rows of seven cells */
#define XPROGRAM_SHAPE5   25   /* ... in BN5's: five of five */

typedef struct {
	uint8_t head[XPROGRAM_VARIANTS][8];   /* a variant's record's first eight bytes, BN6's layout: +0 category, +1 part, +3 colour (0 none), +4 bug's kind */
	uint8_t shape[XPROGRAM_SHAPE], small[XPROGRAM_SHAPE];   /* its shape and its compressed one, nonzero where a block stands */
	uint8_t name[12], about[64];          /* its name, and its description's lines with BN6's line break (0xE9) between: BN6's codes, no end */
	uint8_t name_len, about_len;
} XProgram;

/* BN5's program `program5` out of BN5's ROM `rom5` (8 MB); false where its
 * record, shapes, name or description do not read as one. */
bool xprogram_read(const uint8_t *rom5, int program5, XProgram *out);
/* SoulT+1 as BN5's ROM has it; NULL where the ROM is not read (read once a
 * ROM, kept). */
const XProgram *xprogram_soult(void);
/* A shape of BN5's five by five in BN6's seven by seven: a row down and a
 * column right, its middle on the other's (UnderSht's and the three MAX
 * programs', drawn alike in both games, lie so). */
void xprogram_shape7(const uint8_t five[XPROGRAM_SHAPE5], uint8_t seven[XPROGRAM_SHAPE]);
/* A text archive in memory: `size` bytes of it at most, `entries` entries
 * (BN6's descriptions' block: a word before the archive, its length << 8) */
typedef struct { const uint8_t *data; size_t size; int entries; } XArchive;

/* Text archive `a` (read to its last entry's end mark) with entry `index`
 * made `name` (`n` bytes, no end mark): the archive as it is, the name
 * after its last entry and the entry's offset on it. The entry must share
 * its text with the one after it, a filler no reader loses (BN6's names'
 * archive gives its numbers 48-63 one text, with a label's). Into `out`
 * (`max` at most); its length, 0 where the archive does not read so or
 * does not fit. */
size_t xprogram_name_add(const XArchive *a, int index, const uint8_t *name, size_t n, uint8_t *out, size_t max);
/* BN6's descriptions `raw` (a word, its length << 8, then the archive)
 * with one more entry after the last: `lines` (`ln` bytes) in the frame
 * entry `framed` has, five bytes before its lines and five after. Into
 * `out` (`max` at most); its length, 0 where it does not read as the
 * archive or does not fit. */
size_t xprogram_descs_grow(const XArchive *raw, int framed, const uint8_t *lines, size_t ln, uint8_t *out, size_t max);

#endif
