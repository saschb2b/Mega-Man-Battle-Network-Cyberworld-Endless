/* A moved chip's name in BN6's eight cells (xchips_cells.h). */
#include "xchips_cells.h"

#include <stdbool.h>
#include <string.h>

#include "bn6_chips.h"

#define SMALL_A 0x26   /* BN6's codes of the small letters, a to z */
#define SMALL_Z 0x3F
#define COMMAND 0xE5   /* a text command from here on, no glyph */

/* The names the series spelt short itself, each a letter dropped from the name as the install spells it (BN5's DS mark
 * the two letters D and S): BN4 Red Sun's own names of the chips it shares (its chips 241, 244 and 262, read from its
 * archive, the D and S letters there), BN6's of TomahawkMan; the rest the smallest cut the series' own make: "Man" as
 * "Mn" (BN5's NumbrMn[SP], ShadoMn[SP]), "Mn" as "M" before the mark (BN4's ProtoMDS), a vowel of Colonel. Their cells
 * are the name's, counted from 0 (docs/multirom/chip-registry.md, A name in eight cells, lists them all) */
static const struct { uint8_t game; uint16_t index; uint8_t cut; } cuts[] = {
	{ 5, 244, 4 },   /* NumbrMnDS: NumbMnDS, BN4's 244 */
	{ 5, 247, 5 },   /* ColonelDS: ColonlDS */
	{ 5, 250, 6 },   /* ShadoMnDS: ShadoMDS, as BN4's ProtoMDS (its 241) */
	{ 5, 253, 3 },   /* TmhwkMnDS: TmhkMnDS, BN6's TmhkMan */
	{ 5, 256, 6 },   /* KnigtMnDS: KnigtMDS */
	{ 5, 259, 5 },   /* ToadManDS: ToadMnDS */
	{ 5, 262, 4 },   /* ShadeMnDS: ShadMnDS, BN4's 262 */
	{ 5, 265, 5 },   /* BlizManDS: BlizMnDS */
	{ 5, 268, 6 },   /* CloudMnDS: CloudMDS */
	{ 5, 271, 6 },   /* CosmoMnDS: CosmoMDS */
	{ 5, 274, 5 },   /* LarkManDS: LarkMnDS */
	{ 5, 277, 5 },   /* GridManDS: GridMnDS */
};

int xchips_cells(const uint8_t *s, size_t n) {
	int cells = 0;
	for (size_t i = 0; i < n && s[i] < COMMAND; ++i) ++cells;
	return cells;
}

int xchips_cells_cut(int game, int index) {
	for (size_t i = 0; i < sizeof cuts / sizeof *cuts; ++i)
		if (cuts[i].game == game && cuts[i].index == index) return cuts[i].cut;
	return -1;
}

static bool small(uint8_t c) { return c >= SMALL_A && c <= SMALL_Z; }

static bool vowel(uint8_t c) {
	return c == SMALL_A || c == SMALL_A + ('e' - 'a') || c == SMALL_A + ('i' - 'a') || c == SMALL_A + ('o' - 'a') || c == SMALL_A + ('u' - 'a');
}

/* the cell to drop of `s`'s `cells` (a name past eight with no listed cut): the last small vowel past its first
 * letter, else its last small letter there, else its last */
static int smallest_cut(const uint8_t *s, int cells) {
	for (int i = cells - 1; i > 0; --i)
		if (vowel(s[i])) return i;
	for (int i = cells - 1; i > 0; --i)
		if (small(s[i])) return i;
	return cells - 1;
}

size_t xchips_cells_fit(int game, int index, uint8_t *s, size_t n) {
	int cells = xchips_cells(s, n);
	int cut = xchips_cells_cut(game, index);
	/* (a listed cut where it drops a small letter, as listed) */
	if (cells > BN6_CHIP_NAME_CELLS && cut >= 0 && cut < cells && small(s[cut])) {
		memmove(s + cut, s + cut + 1, n - (size_t)cut - 1);
		--n;
		--cells;
	}
	while (cells > BN6_CHIP_NAME_CELLS) {
		int at = smallest_cut(s, cells);
		memmove(s + at, s + at + 1, n - (size_t)at - 1);
		--n;
		--cells;
	}
	return n;
}
