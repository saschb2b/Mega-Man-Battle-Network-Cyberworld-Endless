/* The ROMs as the launcher has them (launcher_roms.c): which cartridge is
 * read, what a file chosen or dropped turns out to be, the copies kept in
 * the ROM folder, the ROM folder looked at again, and what the last start
 * played with (launcher.ini in the data folder). */
#ifndef CW_LAUNCHER_ROMS_H
#define CW_LAUNCHER_ROMS_H

#include <stdbool.h>
#include <stddef.h>

#include "launcher_text.h"

/* Whether cartridge `slot` (launcher_text.h's SLOT_*) is read: BN6's ROM
 * (rom.h's R) or an extra one's (XR) */
bool roms_have(int slot);
/* ... whether it is on the screen: BN6's always, an extra one's where its
 * ROM is offered (rom.h's xrom_offered) */
bool roms_shown(int slot);
/* The cartridges shown, as bits (1 << SLOT_*); `read` those read alone */
unsigned roms_shown_set(bool read);
/* The copies of the ROMs read, put in `rom_dir` where they are not there
 * yet, so the next start finds them there (a ROM found in Downloads, on an
 * SD card that may be out next time) */
void roms_keep_copies(const char *rom_dir);
/* A file chosen or dropped: read where it is BN6's or an extra one shown,
 * unchanged, and a copy kept in `rom_dir`. The cartridge it filled (-1
 * none); `note` says what came of it, `beside` the extra cartridges that
 * came with BN6 from its folder (bits) */
int roms_take(const char *path, const char *rom_dir, char *note, size_t n, unsigned *beside);
/* The places the ROMs are looked for, looked at again (a phone's pick
 * copied them into `rom_dir`; a desktop's Downloads and front ends' folders) */
void roms_reload(const char *rom_dir);

/* What a start played with: each cartridge's ROM read or not
 * (launcher_text.h's SLOT_*) */
typedef struct {
	bool known;     /* (false: no start has passed the launcher yet) */
	bool have[SLOTS];
} RomRecord;
RomRecord roms_record_read(void);
/* ... this start's: as read now, and written */
RomRecord roms_record_now(void);
void roms_record_write(void);
/* Whether the launcher shows at a start (`asked`: --launcher open, the
 * icon's ROMs shortcut), with the ROMs found now and the last start's: no
 * BN6, a first start, or an extra one gone since the last (one newly
 * found is said on the title) */
static inline bool roms_launcher_wanted(bool asked, RomRecord now, RomRecord last) {
	bool gone = false;
	for (int s = 0; s < SLOTS; ++s) gone |= last.have[s] && !now.have[s];
	return asked || !now.have[SLOT_BN6] || !last.known || gone;
}

#endif
