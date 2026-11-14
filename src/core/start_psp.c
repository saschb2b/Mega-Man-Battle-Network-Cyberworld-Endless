/* The PSP's own start (start_console.h), before the game's: the module's
 * name and its main thread, the heap, the CPU's clock, the memory stick's
 * folders, the output to log.txt, and the ROMs. */
#include "start_console.h"

#ifdef __PSP__
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <pspkernel.h>
#include <psppower.h>

#include "game.h"
#include "rom.h"

/* (SDL2main names the module and starts the main thread in user mode
 * with the VFPU, and its HOME button's exit; here the stack the 3DS's and
 * the Vita's have, 1 MB, for a layer's making, and the heap all the memory
 * the PSP gives the game but a megabyte, for the system's own allocations:
 * MEMSIZE=1 in PARAM.SFO asks for a PSP-2000's 52 MB) */
PSP_MAIN_THREAD_STACK_SIZE_KB(1024);
PSP_HEAP_SIZE_KB(-1024);

/* (mGBA built with FIXED_ROM_BUFFER, as on the 3DS, so the game's 16 MB
 * copy is patched in place: the buffer its own loaders would fill, which
 * the game never calls, is none) */
uint32_t *romBuffer;
size_t romBufferSize;

/* (mGBA's file layer asks for it; newlib's PSP lacks it, and the game
 * never shortens a file through mGBA) */
int ftruncate(int fd, off_t len) { (void)fd; (void)len; return -1; }

static char rom_psp[600];   /* the game's own rom folder */

void console_heap(char *out, size_t n) {
	static size_t most;
	struct mallinfo mi = mallinfo();
	if ((size_t)mi.uordblks > most) most = (size_t)mi.uordblks;
	snprintf(out, n, "heap %u KB in use, %u KB at most of %u KB", (unsigned)(mi.uordblks / 1024), (unsigned)(most / 1024),
		(unsigned)(mi.arena / 1024));
}

void console_start(bool data_dir_given) {
	/* (the PSP's highest clock: its 333 MHz Allegrex runs the GBA's core
	 * and the engine on one core, docs/engine/emulation/psp-and-ds.md) */
	scePowerSetClockFrequency(333, 333, 166);
	/* (the game's folder beside its EBOOT, where PSP homebrew keeps its
	 * files: ms0:/PSP/GAME/cyberworld-endless, or ef0: on a PSP Go's own
	 * memory) */
	if (!data_dir_given) {
		char cwd[400];
		if (getcwd(cwd, sizeof cwd)) snprintf(g_data_dir, sizeof g_data_dir, "%s", cwd);
		else snprintf(g_data_dir, sizeof g_data_dir, "ms0:/PSP/GAME/cyberworld-endless");
	}
	snprintf(rom_psp, sizeof rom_psp, "%s/rom", g_data_dir);
	mkdir(g_data_dir, 0777);
	mkdir(rom_psp, 0777);
	char log[600];
	snprintf(log, sizeof log, "%s/log.txt", g_data_dir);
	if (freopen(log, "w", stdout)) setvbuf(stdout, NULL, _IOLBF, 0);
	freopen(log, "a", stderr);
	printf("psp: the CPU at %d MHz, %u KB free for the heap\n", scePowerGetCpuClockFrequency(),
		(unsigned)(sceKernelTotalFreeMemSize() / 1024));
}

bool console_rom(const char *rom_dir, char *msg, size_t n) {
	if (rom_dir) return rom_find(rom_dir, msg, n);
	/* (the game's own rom folder, then where PSP players keep GBA ROMs for
	 * their emulators: a header check passes over the other games) */
	const char *places[] = { rom_psp, "ms0:/ROMS/GBA", "ms0:/ROMS", "ms0:/PSP/GAME/gpSP", "ef0:/ROMS/GBA" };
	size_t count = sizeof places / sizeof *places;
	char first[512] = "", close[512] = "";
	bool rom_ok = false;
	for (size_t i = 0; !rom_ok && i < count; ++i) {
		rom_ok = rom_find(places[i], msg, n);
		if (!i) snprintf(first, sizeof first, "%s", msg);
		if (!rom_ok && rom_find_close && !close[0]) snprintf(close, sizeof close, "%s", msg);
	}
	if (!rom_ok) snprintf(msg, n, "%s", close[0] ? close : first);
	/* (BN6's net alone: a PSP has no room for BN5's or BN4's ROM beside it,
	 * psp/README.md) */
	return rom_ok;
}
#endif
