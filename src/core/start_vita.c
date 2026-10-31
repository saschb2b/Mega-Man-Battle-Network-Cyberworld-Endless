/* The PlayStation Vita's own start (start_console.h), before the game's:
 * the heap and the main thread's stack, the clocks, the network, the
 * memory card's folders, the output to log.txt, and the ROMs. */
#include "start_console.h"

#ifdef __vita__
#include <stdio.h>
#include <sys/stat.h>

#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
#include <psp2/power.h>
#include <psp2/sysmodule.h>

#include "game.h"
#include "rom.h"

/* (newlib's heap and the main thread's stack, which VitaSDK's start reads
 * before main: its 32 MB heap holds not the ROMs' copies and a run, the
 * 3DS's took 96; its stack is as the 3DS's and the browser's, 1 MB, for a
 * layer's making) */
unsigned int _newlib_heap_size_user = 256u << 20;
unsigned int sceUserMainThreadStackSize = 1u << 20;

static char rom_vita[600];   /* the game's own rom folder */

/* The network for libcurl (online play, the statistics): its module and
 * its memory pool, then its connection's state */
static bool network_start(void) {
	static char pool[1u << 20];
	if (sceSysmoduleLoadModule(SCE_SYSMODULE_NET) < 0) return false;
	/* (one a start before left up, whose netstat answers, is the game's still) */
	if (sceNetShowNetstat() < 0) {
		SceNetInitParam param = { pool, sizeof pool, 0 };
		if (sceNetInit(&param) < 0) return false;
	}
	return sceNetCtlInit() >= 0;
}

void console_start(bool data_dir_given) {
	/* (the clocks a game may ask for: the GBA's core and the engine took
	 * the 3DS's 804 MHz ARM11 and a core of its own) */
	scePowerSetArmClockFrequency(444);
	scePowerSetBusClockFrequency(222);
	scePowerSetGpuClockFrequency(222);
	scePowerSetGpuXbarClockFrequency(166);
	bool net = network_start();
	if (!data_dir_given) snprintf(g_data_dir, sizeof g_data_dir, "ux0:data/cyberworld-endless");
	snprintf(rom_vita, sizeof rom_vita, "%s/rom", g_data_dir);
	mkdir("ux0:data", 0777);
	mkdir(g_data_dir, 0777);
	mkdir(rom_vita, 0777);
	/* (the output into log.txt beside the saves, as the other consoles'
	 * and the handhelds' launcher keep it) */
	char log[600];
	snprintf(log, sizeof log, "%s/log.txt", g_data_dir);
	if (freopen(log, "w", stdout)) setvbuf(stdout, NULL, _IOLBF, 0);
	freopen(log, "a", stderr);
	printf("vita: the CPU at %d MHz, the heap %u MB; the network %s\n", scePowerGetArmClockFrequency(),
		_newlib_heap_size_user >> 20, net ? "up" : "refused");
}

bool console_rom(const char *rom_dir, char *msg, size_t n) {
	if (rom_dir) return rom_find(rom_dir, msg, n);
	/* (the game's own rom folder, then where Vita players keep GBA ROMs: a
	 * header check passes over the other games without reading them; the
	 * game's own folder's too where --data-dir gave another) */
	const char *places[] = { rom_vita, "ux0:data/cyberworld-endless/rom", "ux0:roms/gba", "ux0:roms", "ux0:data/gba" };
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
