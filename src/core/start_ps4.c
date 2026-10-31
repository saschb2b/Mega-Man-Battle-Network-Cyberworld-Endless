/* The PS4's own start (start_console.h), before the game's: the internal
 * drive's folders, the output to log.txt, and the ROMs, there or on a USB
 * drive. A jailbroken PS5 runs the same package. */
#include "start_console.h"

#ifdef CW_PS4
#include <stdio.h>
#include <sys/stat.h>

#include "game.h"
#include "rom.h"

/* (musl's own, which OpenOrbis's libc calls from its timed waits, SDL's
 * semaphores', and leaves undefined beside libkernel's threads: none of
 * the game's threads is ever cancelled) */
int __pthread_setcancelstate(int state, int *old);
int __pthread_setcancelstate(int state, int *old) {
	(void)state;
	if (old) *old = 0;
	return 0;
}

static char rom_ps4[600];   /* the game's own rom folder */

void console_start(bool data_dir_given) {
	if (!data_dir_given) snprintf(g_data_dir, sizeof g_data_dir, "/data/cyberworld-endless");
	snprintf(rom_ps4, sizeof rom_ps4, "%s/rom", g_data_dir);
	mkdir("/data", 0777);
	mkdir(g_data_dir, 0777);
	mkdir(rom_ps4, 0777);
	/* (the output into log.txt beside the saves, as the other consoles keep it) */
	char log[600];
	snprintf(log, sizeof log, "%s/log.txt", g_data_dir);
	if (freopen(log, "w", stdout)) setvbuf(stdout, NULL, _IOLBF, 0);
	freopen(log, "a", stderr);
	printf("ps4: the saves in %s\n", g_data_dir);
}

bool console_rom(const char *rom_dir, char *msg, size_t n) {
	if (rom_dir) return rom_find(rom_dir, msg, n);
	/* (the game's own rom folder on the internal drive, then where PS4
	 * players keep GBA ROMs, there or on a USB drive: a header check passes
	 * over the other games without reading them) */
	const char *places[] = { rom_ps4, "/data/cyberworld-endless/rom", "/data/roms/gba", "/mnt/usb0/roms/gba", "/mnt/usb0/gba",
		"/mnt/usb0", "/mnt/usb1/roms/gba", "/mnt/usb1" };
	size_t count = sizeof places / sizeof *places;
	char first[512] = "", close[512] = "";
	bool rom_ok = false;
	for (size_t i = 0; !rom_ok && i < count; ++i) {
		rom_ok = rom_find(places[i], msg, n);
		if (!i) snprintf(first, sizeof first, "%s", msg);
		if (!rom_ok && rom_find_close && !close[0]) snprintf(close, sizeof close, "%s", msg);
	}
	/* (a near miss, the wrong version found, says more than where to put one) */
	if (!rom_ok) {
		snprintf(msg, n, "%s", close[0] ? close : first);
		return false;
	}
	/* (BN5's beside BN6's is read with it, rom.c; else in the other places) */
	for (size_t i = 0; !XR[XROM_BN5_COLONEL_US].data && i < count; ++i) xrom_find(places[i]);
	return true;
}
#endif
