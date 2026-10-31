/* The Switch's own start (start_console.h), before the game's: the
 * sockets, the SD card's folders, the output to nxlink or log.txt, the
 * memory the Homebrew Menu gave, and the ROMs. */
#include "start_console.h"

#ifdef __SWITCH__
#include <stdio.h>
#include <sys/stat.h>

/* (libnx's parts, not <switch.h>) */
#include <switch/types.h>
#include <switch/result.h>
#include <switch/kernel/svc.h>
#include <netinet/in.h>
#include <switch/services/nifm.h>
#include <switch/services/ssl.h>
#include <switch/runtime/devices/socket.h>
#include <switch/runtime/nxlink.h>
#include <switch/services/applet.h>

#include "game.h"
#include "rom.h"

static char rom_switch[600];   /* the game's own rom folder */

void console_start(bool data_dir_given) {
	bool net = R_SUCCEEDED(socketInitializeDefault());
	bool linked = net && __nxlink_host.s_addr && nxlinkStdio() >= 0;
	if (!data_dir_given) snprintf(g_data_dir, sizeof g_data_dir, "sdmc:/switch/cyberworld-endless");
	snprintf(rom_switch, sizeof rom_switch, "%s/rom", g_data_dir);
	mkdir("sdmc:/switch", 0777);
	mkdir(g_data_dir, 0777);
	mkdir(rom_switch, 0777);
	/* (started from the Homebrew Menu: the output into log.txt beside the
	 * saves, as the 3DS's and the handhelds' launcher keep it) */
	if (!linked) {
		char log[600];
		snprintf(log, sizeof log, "%s/log.txt", g_data_dir);
		if (freopen(log, "w", stdout)) setvbuf(stdout, NULL, _IOLBF, 0);
		freopen(log, "a", stderr);
	}
	/* (the Album's Homebrew Menu runs the game as an applet, with less
	 * memory than a title's, which a game held with R gives) */
	u64 total = 0, used = 0;
	svcGetInfo(&total, InfoType_TotalMemorySize, CUR_PROCESS_HANDLE, 0);
	svcGetInfo(&used, InfoType_UsedMemorySize, CUR_PROCESS_HANDLE, 0);
	AppletType type = appletGetAppletType();
	printf("switch: %s, %s, memory %lu MB, %lu MB in use; the sockets %s\n",
		type == AppletType_Application || type == AppletType_SystemApplication ? "a title's" : "an applet",
		appletGetOperationMode() == AppletOperationMode_Console ? "docked" : "handheld",
		(unsigned long)(total >> 20), (unsigned long)(used >> 20), net ? "up" : "refused");
}

bool console_rom(const char *rom_dir, char *msg, size_t n) {
	if (rom_dir) return rom_find(rom_dir, msg, n);
	/* (the game's own rom folder, then where Switch players keep GBA ROMs:
	 * a header check passes over the other games without reading them;
	 * the game's own folder's too where --data-dir gave another) */
	const char *places[] = { rom_switch, "sdmc:/switch/cyberworld-endless/rom", "sdmc:/roms/gba", "sdmc:/roms", "sdmc:/gba" };
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
