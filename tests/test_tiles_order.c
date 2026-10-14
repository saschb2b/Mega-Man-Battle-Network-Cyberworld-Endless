/* Equal-frequency tile choices must have the same order on every libc. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "tiles_parts.h"

int main(void) {
	static const TileCand expected[] = {
		{ .key = 1, .count = 9, .e0 = 8 },
		{ .key = 1, .count = 4, .e0 = 2, .e1 = 1 },
		{ .key = 1, .count = 4, .e0 = 2, .e1 = 1, .pad = 1 },
		{ .key = 1, .count = 4, .e0 = 2, .e1 = 3 },
		{ .key = 1, .count = 4, .e0 = 7 },
		{ .key = 2, .count = 20, .e0 = 1 },
	};
	/* Each permutation must produce the same picks, without relying on a
	 * particular qsort implementation retaining input order at ties. */
	unsigned checked = 0;
	for (unsigned code = 0; code < 46656; ++code) {
		unsigned value = code, used = 0;
		TileCand input[6];
		for (unsigned i = 0; i < 6; ++i) {
			unsigned index = value % 6;
			value /= 6;
			used |= 1u << index;
			input[i] = expected[index];
		}
		if (used != 63) continue;
		qsort(input, 6, sizeof *input, tl_cmp_count);
		for (unsigned i = 0; i < 6; ++i) {
			assert(input[i].key == expected[i].key);
			assert(input[i].count == expected[i].count);
			assert(input[i].e0 == expected[i].e0 && input[i].e1 == expected[i].e1);
			assert(input[i].pad == expected[i].pad);
		}
		++checked;
	}
	assert(checked == 720);
	puts("tiles: 720 permutations keep equal-frequency choices deterministic");
	return 0;
}
