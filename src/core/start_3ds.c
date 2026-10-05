/* start_3ds.h. The 3DS's own start, before the game's: libctru's memory
 * split replaced, the third core probed, the SD card's folders, and the
 * output to 3dslink or log.txt. */
#include "start_3ds.h"

#ifdef __3DS__
#include <stdio.h>
#include <sys/stat.h>

/* (libctru's parts, not <3ds.h>: its Friends service has a Profile too) */
#include <3ds/types.h>
#include <3ds/os.h>
#include <3ds/services/soc.h>
#include <3ds/3dslink.h>
#include <3ds/env.h>
#include <3ds/svc.h>
#include <3ds/result.h>
#include <3ds/allocator/mappable.h>
#include <3ds/services/apt.h>
#include <3ds/services/ptmsysm.h>
#include <3ds/thread.h>
#include <malloc.h>

#include "game.h"
#include "rom.h"

/* (the main thread's stack: libctru's 32 KB is tight for the game's
 * deepest calls, a layer's making; the browser build's is 1 MB too) */
u32 __stacksize__ = 1u << 20;

/* (whether the New 3DS's third core takes a thread of the game's: emu.c
 * runs the GBA core there) */
static void core2_probe(void *arg) { *(volatile bool *)arg = true; }

/* The app's memory, split before main in place of libctru's split (and
 * mGBA's fixed sizes): the heap takes all its area holds, 96 MB, the
 * linear heap (the screens' and the sound's buffers) what is left, at
 * least 8 MB. The Homebrew Launcher gives a New 3DS app 124 MB, a 3DS 64:
 * a heap of all but the linear heap's share passed the area on the one,
 * a fixed 36 MB was too small for the game's ROM copies on both. */
void __system_allocateHeaps(void);   /* (libctru's, replaced) */
void __system_allocateHeaps(void) {
	extern char *fake_heap_start, *fake_heap_end;
	extern u32 __ctru_heap, __ctru_linear_heap;
	/* (the sizes env.h reads in its own accessors, written here) */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wredundant-decls"
	extern u32 __ctru_heap_size, __ctru_linear_heap_size;
#pragma GCC diagnostic pop
	Handle limit = 0;
	s64 most = 0, used = 0;
	ResourceLimitType commit = RESLIMIT_COMMIT;
	if (R_FAILED(svcGetResourceLimit(&limit, CUR_PROCESS_HANDLE))) svcBreak(USERBREAK_PANIC);
	svcGetResourceLimitLimitValues(&most, limit, &commit, 1);
	svcGetResourceLimitCurrentValues(&used, limit, &commit, 1);
	svcCloseHandle(limit);
	u32 left = (u32)(most - used) & ~0xFFFu, linear = 8u << 20;
	if (left <= linear) svcBreak(USERBREAK_PANIC);
	u32 heap = left - linear;
	if (heap > OS_HEAP_AREA_END - OS_HEAP_AREA_BEGIN) heap = OS_HEAP_AREA_END - OS_HEAP_AREA_BEGIN;
	__ctru_heap_size = heap;
	__ctru_linear_heap_size = left - heap;
	if (R_FAILED(svcControlMemory(&__ctru_heap, OS_HEAP_AREA_BEGIN, 0, heap, MEMOP_ALLOC, MEMPERM_READ | MEMPERM_WRITE))
	    || R_FAILED(svcControlMemory(&__ctru_linear_heap, 0, 0, __ctru_linear_heap_size, MEMOP_ALLOC_LINEAR, MEMPERM_READ | MEMPERM_WRITE)))
		svcBreak(USERBREAK_PANIC);
	mappableInit(OS_MAP_AREA_BEGIN, OS_MAP_AREA_END);
	fake_heap_start = (char *)__ctru_heap;
	fake_heap_end = fake_heap_start + heap;
}

static char rom_3ds[600];   /* the 3DS's own rom folder */

/* (whether ptm:sysm, which sets the clock, answers is asked first for the
 * log) */
void start_3ds(bool data_dir_given) {
	Result sysm = ptmSysmInit();
	if (R_SUCCEEDED(sysm)) ptmSysmExit();
	osSetSpeedupEnable(true);
	volatile bool third = false;
	union { volatile bool *c; void *v; } arg = { &third };   /* (the thread writes it through a volatile) */
	Thread probe = threadCreate(core2_probe, arg.v, 0x1000, 0x30, 2, false);
	if (probe) { threadJoin(probe, U64_MAX); threadFree(probe); }
	bool n3ds = false;
	APT_CheckNew3DS(&n3ds);
	char mem[300];
	snprintf(mem, sizeof mem, "3ds: %s, %s, heap %lu KB, linear heap %lu KB; the speedup %s; the third core %s",
		n3ds ? "New 3DS" : "3DS", envIsHomebrew() ? "homebrew" : "title",
		(unsigned long)(envGetHeapSize() / 1024), (unsigned long)(envGetLinearHeapSize() / 1024),
		R_SUCCEEDED(sysm) ? "on" : "refused", third ? "free for the game" : probe ? "ran nothing" : "refused");
	if (__3dslink_host.s_addr) {
		u32 *soc = memalign(0x1000, 0x100000);
		if (soc && socInit(soc, 0x100000) == 0) link3dsStdio();
	}
	if (!data_dir_given) snprintf(g_data_dir, sizeof g_data_dir, "sdmc:/3ds/cyberworld-endless");
	snprintf(rom_3ds, sizeof rom_3ds, "%s/rom", g_data_dir);
	mkdir("sdmc:/3ds", 0777);
	mkdir(g_data_dir, 0777);
	mkdir(rom_3ds, 0777);
	/* (started from the Homebrew Launcher: the output into log.txt beside
	 * the saves, as the handhelds' launcher keeps it) */
	if (!__3dslink_host.s_addr) {
		char log[600];
		snprintf(log, sizeof log, "%s/log.txt", g_data_dir);
		if (freopen(log, "w", stdout)) setvbuf(stdout, NULL, _IOLBF, 0);
		freopen(log, "a", stderr);
	}
	printf("%s\n", mem);
}

bool start_3ds_rom(const char *rom_dir, char *msg, size_t n) {
	/* (the game's own rom folder, then where 3DS players keep GBA ROMs: a
	 * header check passes over the other games without reading them) */
	if (rom_dir) return rom_find(rom_dir, msg, n);
	bool rom_ok = false;
	const char *places[] = { rom_3ds, "sdmc:/roms/gba", "sdmc:/roms", "sdmc:/gba" };
	char first[512] = "", close[512] = "";
	for (size_t i = 0; !rom_ok && i < sizeof places / sizeof *places; ++i) {
		rom_ok = rom_find(places[i], msg, n);
		if (!i) snprintf(first, sizeof first, "%s", msg);
		if (!rom_ok && rom_find_close && !close[0]) snprintf(close, sizeof close, "%s", msg);
	}
	/* (a near miss, the wrong version found, says more than where to put one) */
	if (!rom_ok) snprintf(msg, n, "%s", close[0] ? close : first);
	return rom_ok;
}
#endif
