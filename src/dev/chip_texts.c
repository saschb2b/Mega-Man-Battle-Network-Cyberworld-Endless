/* chip_texts.h: a tab-separated line per chip id,
 *   id  game index  readers  name  description
 * the game whose entry its name and description should be (6 BN6, 5 BN5,
 * 4 BN4, 0 none: an id no chip holds) and that entry's index in its
 * archives (the registry's row, else BN6's own id); readers 1 where the
 * PET's two readers of a description (BN6_CHIP_DESC_READ, ..._READ2) take
 * the archive BN6_CHIP_DESC_OF does; then the name's and the description's
 * bytes in hex, as BN6's routines find them in the core's copy (the
 * archives' words their literals name, the id's high byte's word, its low
 * byte's entry). The bytes are the ROMs' text: the file goes in .build/,
 * never in git. */
#include "chip_texts.h"

#include <stdio.h>

#include "bn6_chips.h"
#include "chips.h"
#include "emu.h"
#include "rom.h"
#include "xchips_registry.h"
#include "xinstall.h"

#define TEXT_END 0xE6
#define TEXT_MAX 160   /* an entry's bytes at most (chips.c's ENTRY_MAX) */

/* the archive a routine whose literal is at `literal` reads chip `id`'s text from: the words the literal names, the id's
 * high byte's (xchips_wide.c: BN6's two words, or the core's four once ten-bit ids are in) */
static uint32_t archive_of(uint32_t literal, int id) { return emu_read32(emu_read32(literal) + 4u * (uint32_t)(id >> 8)); }

/* entry `id & 0xFF` of the archive at `arch` into `out` as hex, to its end (TEXT_END) */
static void entry_hex(uint32_t arch, int id, FILE *out) {
	uint32_t at = arch + emu_read16(arch + 2u * (uint32_t)(id & 0xFF));
	for (int i = 0; i < TEXT_MAX; ++i) {
		uint8_t c = emu_read8(at + (uint32_t)i);
		fprintf(out, "%02x", c);
		if (c == TEXT_END) break;
	}
}

int chip_texts_run(const char *path) {
	if (!emu_init(R.data, ROM_SIZE)) return 1;
	if (!xinstall_bn5()) return 1;
	FILE *out = fopen(path, "w");
	if (!out) return 1;
	/* (each routine's literal: its +0x18, the PET's readers' first archive's) */
	const uint32_t name_of = BN6_CHIP_NAME_OF + 0x18u, desc_of = BN6_CHIP_DESC_OF + 0x18u;
	const uint32_t reads[2] = { BN6_CHIP_DESC_READ + 0x130u, BN6_CHIP_DESC_READ2 + 0x13Cu };
	for (int id = 0; id < CHIPS_IDS; ++id) {
		const XChipReg *r = xreg_at(id);
		const XChipReg4 *r4 = r ? NULL : xreg4_at(id);
		int game = r ? 5 : r4 ? 4 : id < CHIPS_BN6_RECORDS ? 6 : 0, index = r ? r->bn5 : r4 ? r4->bn4 : id;
		uint32_t desc = archive_of(desc_of, id);
		bool agree = archive_of(reads[0], id) == desc && archive_of(reads[1], id) == desc;
		fprintf(out, "%d\t%d\t%d\t%d\t", id, game, index, agree);
		entry_hex(archive_of(name_of, id), id, out);
		fputc('\t', out);
		entry_hex(desc, id, out);
		fputc('\n', out);
	}
	bool ok = !ferror(out);
	return fclose(out) == 0 && ok ? 0 : 1;
}
