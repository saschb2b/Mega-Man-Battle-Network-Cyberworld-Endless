/* One exchange at a time with the Net Square's server outside a visit
 * (shared/online/README.md: saves sent by a code, TRANSFER; saves kept in
 * step, SYNC): a connection to /presence, a first message, then a blob
 * sent after the server's word to send it, or one taken in after its word
 * that one comes. On a thread of its own where the system's socket waits
 * (online_socket.h); on the browser's main thread its callbacks and
 * relay_poll moving it on. What the server's words mean is the job's
 * (transfer.c, cloud.c). Never while a visit is on: the socket is the
 * Square's, and the jobs run over the title, the ROMs screen, SAVES, and
 * a run's checkpoints, where no visit is. */
#ifndef CW_RELAY_H
#define CW_RELAY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Why a connection ended before its job said it was done */
typedef enum { RELAY_UNREACHED, RELAY_REFUSED, RELAY_DAMAGED, RELAY_NO_MEMORY } RelayLoss;

/* The first message's bytes at most */
#define RELAY_FIRST 64

typedef struct {
	/* Before each connection (the relay's thread): the first message and
	 * the blob to send if the server asks (the job's bytes, left alone till
	 * the next prepare or the end); false where the job ended, saying why */
	bool (*prepare)(uint8_t first[RELAY_FIRST], size_t *first_n, const uint8_t **out, size_t *out_n);
	/* A message from the server other than the blob's DATA: the job's to
	 * answer through relay_send_blob, relay_take_blob, relay_reply and
	 * relay_end */
	void (*word)(const uint8_t *d, size_t n);
	/* The blob taken in, whole: its bytes, the job's to free */
	void (*taken)(uint8_t *bytes, size_t n);
	/* The connection lost before relay_end: why (an ERROR is REFUSED) */
	void (*lost)(RelayLoss why);
} RelayJob;

/* Whether this system has the Square's socket (none: the PSP, a 3DS build
 * without libcurl) */
bool relay_here(void);
/* The server's address (the official Square's, or --dev online's) */
void relay_use(const char *url);
/* `job` begun: false where one is under way (or there is no thread) */
bool relay_start(const RelayJob *job);
bool relay_busy(void);
/* The browser's: each frame, the blob's next bytes, the time, another try */
void relay_poll(void);
/* The one under way given up, its thread let finish */
void relay_stop(void);
/* The one under way waited for, `ms` at most (the game closing) */
void relay_wait(uint32_t ms);
/* The bytes moved of the blob going or coming, of all */
void relay_progress(uint32_t *moved, uint32_t *size);

/* The job's answers, from its word() and taken(): */
/* The blob goes now (the server's READY) */
void relay_send_blob(void);
/* A blob of `size` bytes comes now (FOUND): false where there is no room */
bool relay_take_blob(uint32_t size);
/* A message back on the same connection (a DONE) */
void relay_reply(const uint8_t *d, size_t n);
/* This connection's exchange done; `again`: prepared and made once more */
void relay_end(bool again);

/* The jobs' own states, read by the game's thread under it */
void relay_lock(void);
void relay_unlock(void);

#endif
