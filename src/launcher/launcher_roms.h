/* The ROMs as the launcher has them (launcher_roms.c): which cartridge is
 * read, what a file chosen or dropped turns out to be, the copies kept in
 * the ROM folder, the ROM folder looked at again, and what the last start
 * played with (launcher.ini in the data folder). */
#ifndef CW_LAUNCHER_ROMS_H
#define CW_LAUNCHER_ROMS_H

#include <stdbool.h>
#include <stddef.h>

/* Whether cartridge `slot` (launcher_text.h's SLOT_*) is read: BN6's ROM
 * (rom.h's R) or BN5's (XR) */
bool roms_have(int slot);
/* The copies of the ROMs read, put in `rom_dir` where they are not there
 * yet, so the next start finds them there (a ROM found in Downloads, on an
 * SD card that may be out next time) */
void roms_keep_copies(const char *rom_dir);
/* A file chosen or dropped: read where it is BN6's or BN5's, unchanged,
 * and a copy kept in `rom_dir`. The cartridge it filled (-1 none); `note`
 * says what came of it, `beside` whether BN5 came with BN6 from its folder */
int roms_take(const char *path, const char *rom_dir, char *note, size_t n, bool *beside);
/* The places the ROMs are looked for, looked at again (a phone's pick
 * copied them into `rom_dir`; a desktop's Downloads and front ends' folders) */
void roms_reload(const char *rom_dir);

/* What a start played with */
typedef struct {
	bool known;     /* (false: no start has passed the launcher yet) */
	bool bn6, bn5;
} RomRecord;
RomRecord roms_record_read(void);
/* ... this start's, written */
void roms_record_write(void);
/* Whether the launcher shows at a start (`asked`: --launcher open, the
 * icon's ROMs shortcut), with the ROMs found now and the last start's: no
 * BN6, a first start, or BN5 gone since the last (one newly found is said
 * on the title) */
static inline bool roms_launcher_wanted(bool asked, bool bn6, bool bn5, RomRecord last) {
	return asked || !bn6 || !last.known || (last.bn5 && !bn5);
}

#endif
