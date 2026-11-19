/* The pipe to the Discord app on the same machine (discord.h): Windows'
 * named pipe \\.\pipe\discord-ipc-N, a Unix socket discord-ipc-N in the
 * runtime or temporary folder elsewhere (the Flatpak's and the Snap's
 * Discord in their own folders there), N from 0 to 9, the first that
 * answers. Bytes alone; discord_activity.c says what they mean. Never
 * waits: what Discord has sent is read where it is there. */
#ifndef CW_DISCORD_IPC_H
#define CW_DISCORD_IPC_H

#include <stdbool.h>
#include <stddef.h>

/* The builds that can reach a Discord app: the desktop's */
#if defined(CW_DESKTOP) && (defined(_WIN32) || defined(__linux__) || defined(__APPLE__))
#define DISCORD_IPC 1
#endif

/* Opened on the first pipe that answers; false where none did */
bool discord_ipc_open(void);
/* `n` bytes written whole; false where the pipe broke (it is closed then) */
bool discord_ipc_send(const void *data, size_t n);
/* What has come, at most `n` bytes: how many, 0 none yet, -1 the pipe
 * closed (it is closed then) */
long discord_ipc_recv(void *buf, size_t n);
void discord_ipc_close(void);
/* The game's process, which Discord ties the activity to */
long discord_ipc_pid(void);

#endif
