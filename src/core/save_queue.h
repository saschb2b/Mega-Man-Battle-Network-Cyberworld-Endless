#ifndef SAVE_QUEUE_H
#define SAVE_QUEUE_H

#include <stdbool.h>
#include <stddef.h>

/* The saves' files written off the frame. A checkpoint writes some ten
 * files, each beside its place and then renamed over it (a power cut keeps
 * the old one), and on a 3DS's SD card each of those steps is a call to
 * its file system: a checkpoint held the game still for 0.8 to 1.1
 * seconds. The frame now only copies the files' bytes; a thread of their
 * own writes them in the order they came, the same way. Whatever reads,
 * removes or copies the saves waits for it first (save_queue_wait). The
 * browser's build has no threads: there each file is written at once, as
 * before. */

/* `data` (n bytes, malloc'd) to be written to `path`, beside it and then
 * renamed over it: true where it is queued (the queue frees it), false
 * where the caller is to write it now (the browser's build, or no thread) */
bool save_queue_file(const char *path, void *data, size_t n);
/* Until every file queued is written */
void save_queue_wait(void);
/* Every file queued written, and the thread gone (the game's end) */
void save_queue_quit(void);
/* The bytes written to `path` now, beside it and then renamed over it
 * (the queue's own way, and the callers' where it takes nothing) */
bool save_file_write(const char *path, const void *data, size_t n);

#endif
