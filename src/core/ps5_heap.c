/* The PS5's heap for the game. libc's malloc, which the template's libc.prx
 * hands to libSceLibcInternal, draws there on the kernel's internal memory,
 * some 14 MB, where the game wants a few hundred (SDL's 1080p window
 * surface alone is 8 MB). The linker sends the game's and its libraries'
 * malloc, calloc, realloc, posix_memalign and free here (the Makefile's
 * --wrap): an mspace of libSceLibcInternal's on flexible memory, which
 * sce_sys/param.json's kernel.flexibleMemorySize gives the app. A pointer
 * from outside it (libc's own strdup, getcwd and realpath allocate as the
 * system does) goes back to the system's. */
#ifdef CW_PS5
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>

#define HEAP_BYTES ((size_t)512 << 20)

typedef void *Mspace;
Mspace sceLibcMspaceCreate(const char *name, void *base, size_t capacity, unsigned int flags);
void *sceLibcMspaceMalloc(Mspace space, size_t size);
void *sceLibcMspaceCalloc(Mspace space, size_t count, size_t size);
void *sceLibcMspaceRealloc(Mspace space, void *ptr, size_t size);
int sceLibcMspacePosixMemalign(Mspace space, void **out, size_t alignment, size_t size);
int sceLibcMspaceFree(Mspace space, void *ptr);

void *__real_malloc(size_t size);
void *__real_calloc(size_t count, size_t size);
void *__real_realloc(void *ptr, size_t size);
int __real_posix_memalign(void **out, size_t alignment, size_t size);
void __real_free(void *ptr);
void *__wrap_malloc(size_t size);
void *__wrap_calloc(size_t count, size_t size);
void *__wrap_realloc(void *ptr, size_t size);
int __wrap_posix_memalign(void **out, size_t alignment, size_t size);
void __wrap_free(void *ptr);

static Mspace space;
static uintptr_t low, high;
static bool tried;

/* (made at the first allocation, before the game's threads: a failure
 * leaves the system's heap to everything) */
static bool ready(void) {
	if (space) return true;
	if (tried) return false;
	tried = true;
	void *base = mmap(NULL, HEAP_BYTES, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (base == MAP_FAILED) return false;
	space = sceLibcMspaceCreate("cyberworld", base, HEAP_BYTES, 0);
	if (!space) {
		munmap(base, HEAP_BYTES);
		return false;
	}
	low = (uintptr_t)base;
	high = low + HEAP_BYTES;
	return true;
}

static bool ours(const void *ptr) { return (uintptr_t)ptr >= low && (uintptr_t)ptr < high; }

void *__wrap_malloc(size_t size) { return ready() ? sceLibcMspaceMalloc(space, size) : __real_malloc(size); }

void *__wrap_calloc(size_t count, size_t size) {
	return ready() ? sceLibcMspaceCalloc(space, count, size) : __real_calloc(count, size);
}

void *__wrap_realloc(void *ptr, size_t size) {
	if (ptr && !ours(ptr)) return __real_realloc(ptr, size);
	return ready() ? sceLibcMspaceRealloc(space, ptr, size) : __real_realloc(ptr, size);
}

int __wrap_posix_memalign(void **out, size_t alignment, size_t size) {
	return ready() ? sceLibcMspacePosixMemalign(space, out, alignment, size) : __real_posix_memalign(out, alignment, size);
}

void __wrap_free(void *ptr) {
	if (!ptr) return;
	if (ours(ptr)) sceLibcMspaceFree(space, ptr);
	else __real_free(ptr);
}
#endif
