/* start_3ds.h. The 3DS's own start, before the game's: libctru's memory
 * split replaced, mGBA's unused ROM buffer given back, the third core
 * probed, the SD card's folders, the output to 3dslink or log.txt, the
 * ROMs (an extra one, BN5's, where the heap has room for it) and the
 * heap's use. */
#include "start_3ds.h"

#ifdef __3DS__
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
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

#include "emu.h"
#include "game.h"
#include "rom.h"
#include "start_console.h"

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

/* mGBA's 3DS setup (its ctru-heap.c, linked in with the core) takes a ROM
 * buffer from the heap before main, 16 MB (docker/mgba.sh), for its own
 * loaders (mCorePreloadVF, mCoreLoadFile), which the game never calls:
 * emu.c hands the core its padded copy as a file in memory, which the core
 * maps as it is (GBALoadROM maps a 16 MB ROM, and patches it in place).
 * Given back, the heap holds an extra ROM's file (BN5's) with 8 MB to spare
 * (docs/engine/emulation/3ds-memory.md). */
extern uint32_t *romBuffer;
extern size_t romBufferSize;

static void buffer_give_back(void) {
	size_t had = romBuffer ? romBufferSize : 0;
	free(romBuffer);
	romBuffer = NULL;
	romBufferSize = 0;
	printf("3ds: mGBA's ROM buffer given back to the heap, %lu KB (the core runs the game's own copy)\n", (unsigned long)(had / 1024));
}

/* (the heap's use, mallinfo's: with trimming off, the space taken from the
 * heap's area never shrinks, so it is the high-water mark) */
void start_3ds_heap(char *out, size_t n) {
	struct mallinfo m = mallinfo();
	snprintf(out, n, "heap %lu KB in use, %lu KB at most of %lu KB", (unsigned long)(m.uordblks / 1024),
		(unsigned long)(m.arena / 1024), (unsigned long)(envGetHeapSize() / 1024));
}

/* (what the heap can still give: its area past what malloc took, and what
 * malloc holds free) */
static size_t heap_free(void) {
	struct mallinfo m = mallinfo();
	size_t heap = envGetHeapSize();
	return (heap > m.arena ? heap - m.arena : 0) + m.fordblks;
}

/* (whether ptm:sysm, which sets the clock, answers is asked first for the
 * log) */
void console_start(bool data_dir_given) {
	/* (the heap's high-water mark kept: start_3ds_heap) */
	mallopt(M_TRIM_THRESHOLD, INT_MAX);
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
		/* (a line at a time, as log.txt: what a run that stops said reaches 3dslink) */
		if (soc && socInit(soc, 0x100000) == 0 && link3dsStdio() >= 0) setvbuf(stdout, NULL, _IOLBF, 0);
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
	buffer_give_back();
}

/* The extra ROMs offered (rom.h's xrom_offered: BN5's; BN4's once it is)
 * read beside BN6's, in their order, where the heap holds one with room
 * left: its 8 MB, the core's 16 MB copy still to come and what a run takes
 * beside them, 50 MB at most where a layer's area is learned
 * (tools/heap3ds.py), and some to spare. A New 3DS holds one extra ROM
 * at most: with BN5's and BN4's both read, a long run's play outgrows
 * the heap (tools/heap3ds.py, docs/engine/emulation/3ds-memory.md). One
 * found past that, or short of room, is left unread, the run going on
 * without it, said in the log and on the PET. */
#define XROM_ROOM ((size_t)ROM_SIZE + EMU_ROM_SIZE + (56u << 20))
#define XROMS_3DS 1
static char unread[48];   /* (the tags of those found and left unread: "BN4") */

/* Extra ROM `xrom`'s .gba in `dir`, told by its header's game code before
 * it is read: the first one's path in `first` (where it is still ""),
 * and the first that reads and checks read where `load` */
static bool xrom_in(const char *dir, int xrom, bool load, char *first, size_t n) {
	DIR *d = opendir(dir);
	struct dirent *e;
	while (d && !XR[xrom].data && (e = readdir(d))) {
		size_t len = strlen(e->d_name);
		if (len < 4 || strcasecmp(e->d_name + len - 4, ".gba")) continue;
		char path[700], code[5] = "";
		snprintf(path, sizeof path, "%s/%s", dir, e->d_name);
		FILE *f = fopen(path, "rb");
		bool its = f && !fseek(f, 0, SEEK_END) && ftell(f) == ROM_SIZE && !fseek(f, 0xAC, SEEK_SET) && fread(code, 1, 4, f) == 4 &&
			xrom_of_code(code) == xrom;
		if (f) fclose(f);
		if (!its) continue;
		if (!first[0]) snprintf(first, n, "%s", path);
		if (load) xrom_load(path);
	}
	if (d) closedir(d);
	return first[0] != 0;
}

/* ... each looked for beside BN6's ROM first, then in the 3DS's other ROM
 * folders (`places`) */
static void extras_3ds(const char *const *places, size_t count) {
	char beside[600];
	snprintf(beside, sizeof beside, "%s", R.path);
	char *slash = strrchr(beside, '/');
	if (slash) *slash = 0;
	int read = 0;
	for (int x = 0; x < XROM_COUNT; ++x) {
		if (!xrom_offered(x)) continue;
		const char *tag = xrom_layout(x)->tag;
		char first[700] = "";
		u64 t0 = osGetTime();
		size_t room = heap_free();
		bool fits = read < XROMS_3DS && room >= XROM_ROOM;
		for (size_t i = 0; i <= count && !XR[x].data; ++i) {
			const char *dir = i ? places[i - 1] : beside;
			if (i && !strcmp(dir, beside)) continue;
			xrom_in(dir, x, fits, first, sizeof first);
		}
		char heap[120];
		start_3ds_heap(heap, sizeof heap);
		if (XR[x].data) {
			++read;
			printf("3ds: %s read and checked in %llu ms (%s); %s\n", tag, (unsigned long long)(osGetTime() - t0), XR[x].path, heap);
		} else if (first[0] && !fits) {
			size_t m = strlen(unread);
			snprintf(unread + m, sizeof unread - m, "%s%s", m ? " + " : "", tag);
			if (read >= XROMS_3DS)
				printf("3ds: %s's ROM (%s) left unread: a New 3DS holds %d extra ROM beside BN6's, read already\n", tag, first, XROMS_3DS);
			else
				printf("3ds: %s's ROM (%s) left unread: the heap has %lu KB free, %s asks %lu KB (its file, the core's copy and a run's"
					" play); a run without it\n", tag, first, (unsigned long)(room / 1024), tag, (unsigned long)(XROM_ROOM / 1024));
		} else if (first[0]) printf("3ds: %s is not %s unchanged, or did not read; a run without it\n", first, xrom_layout(x)->name);
		else printf("3ds: no %s ROM beside BN6's or in the 3DS's ROM folders; a run without it\n", tag);
	}
}

const char *start_3ds_unread(void) { return unread[0] ? unread : NULL; }

bool console_rom(const char *rom_dir, char *msg, size_t n) {
	/* (the game's own rom folder, then where 3DS players keep GBA ROMs: a
	 * header check passes over the other games without reading them) */
	if (rom_dir) {
		bool found = rom_find(rom_dir, msg, n);
		if (found) extras_3ds(NULL, 0);
		return found;
	}
	bool rom_ok = false;
	/* (the game's own folder's too where --data-dir gave another, a test's over 3dslink) */
	const char *places[] = { rom_3ds, "sdmc:/3ds/cyberworld-endless/rom", "sdmc:/roms/gba", "sdmc:/roms", "sdmc:/gba" };
	char first[512] = "", close[512] = "";
	for (size_t i = 0; !rom_ok && i < sizeof places / sizeof *places; ++i) {
		rom_ok = rom_find(places[i], msg, n);
		if (!i) snprintf(first, sizeof first, "%s", msg);
		if (!rom_ok && rom_find_close && !close[0]) snprintf(close, sizeof close, "%s", msg);
	}
	/* (a near miss, the wrong version found, says more than where to put one) */
	if (!rom_ok) snprintf(msg, n, "%s", close[0] ? close : first);
	else extras_3ds(places, sizeof places / sizeof *places);
	return rom_ok;
}
#endif
