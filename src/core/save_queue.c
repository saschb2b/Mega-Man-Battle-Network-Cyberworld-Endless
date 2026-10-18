/* save_queue.h. One thread writes the queued files, first come first
 * written; it sleeps while there are none. */
#include "save_queue.h"

#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>

#include "compat.h"

static SDL_atomic_t failures;

unsigned save_write_failures(void) { return (unsigned)SDL_AtomicGet(&failures); }
void save_write_failed(void) { SDL_AtomicAdd(&failures, 1); }

bool save_file_write(const char *path, const void *data, size_t n) {
	char tmp[620];
	snprintf(tmp, sizeof tmp, "%s.tmp", path);
	FILE *f = fopen(tmp, "wb");
	if (!f) { save_write_failed(); return false; }
	bool ok = fwrite(data, 1, n, f) == n;
	ok = fflush(f) == 0 && ok;
	ok = fclose(f) == 0 && ok;
	ok = ok && cw_rename(tmp, path);
	if (!ok) { remove(tmp); save_write_failed(); }
	return ok;
}

#ifdef __EMSCRIPTEN__
/* (the browser's build: no threads, each file written at once) */
bool save_queue_file(const char *path, void *data, size_t n) {
	(void)path;
	(void)data;
	(void)n;
	return false;
}

void save_queue_wait(void) {}

void save_queue_quit(void) {}
#else
typedef struct Job {
	struct Job *next;
	char path[600];
	void *data;
	size_t n;
} Job;

static SDL_Thread *thread;
static SDL_mutex *lock;
static SDL_cond *wake, *idle;
static Job *head, *tail;
static bool busy, stopping, no_thread;

static int writer(void *unused) {
	(void)unused;
	/* (below the game's frame: its file system's waits are this thread's) */
	SDL_SetThreadPriority(SDL_THREAD_PRIORITY_LOW);
	SDL_LockMutex(lock);
	for (;;) {
		while (!head && !stopping) SDL_CondWait(wake, lock);
		if (!head) break;
		Job *j = head;
		head = j->next;
		if (!head) tail = NULL;
		busy = true;
		SDL_UnlockMutex(lock);
		if (!save_file_write(j->path, j->data, j->n)) fprintf(stderr, "saves: %s could not be written\n", j->path);
		free(j->data);
		free(j);
		SDL_LockMutex(lock);
		busy = false;
		if (!head) SDL_CondBroadcast(idle);
	}
	SDL_UnlockMutex(lock);
	return 0;
}

static void let_go(void) {
	if (idle) SDL_DestroyCond(idle);
	if (wake) SDL_DestroyCond(wake);
	if (lock) SDL_DestroyMutex(lock);
	idle = wake = NULL;
	lock = NULL;
}

/* the thread, started at the first file queued */
static bool started(void) {
	if (thread) return true;
	if (no_thread) return false;
	lock = SDL_CreateMutex();
	wake = SDL_CreateCond();
	idle = SDL_CreateCond();
	thread = lock && wake && idle ? SDL_CreateThread(writer, "saves", NULL) : NULL;
	if (!thread) {
		fprintf(stderr, "saves: no thread to write them (%s): each written at once\n", SDL_GetError());
		let_go();
		no_thread = true;
	}
	return thread != NULL;
}

bool save_queue_file(const char *path, void *data, size_t n) {
	if (!started()) return false;
	Job *j = malloc(sizeof *j);
	if (!j) return false;
	snprintf(j->path, sizeof j->path, "%s", path);
	j->data = data;
	j->n = n;
	j->next = NULL;
	SDL_LockMutex(lock);
	if (tail) tail->next = j;
	else head = j;
	tail = j;
	SDL_CondSignal(wake);
	SDL_UnlockMutex(lock);
	return true;
}

void save_queue_wait(void) {
	if (!thread) return;
	SDL_LockMutex(lock);
	while (head || busy) SDL_CondWait(idle, lock);
	SDL_UnlockMutex(lock);
}

void save_queue_quit(void) {
	if (!thread) return;
	SDL_LockMutex(lock);
	stopping = true;
	SDL_CondSignal(wake);
	SDL_UnlockMutex(lock);
	SDL_WaitThread(thread, NULL);
	thread = NULL;
	stopping = false;
	let_go();
}
#endif
