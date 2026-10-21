/* Linux's libcurl supplies TCP/TLS; this bounded stream supplies RFC 6455. */
#ifndef CW_ONLINE_SOCKET_H
#define CW_ONLINE_SOCKET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <SDL.h>


bool online_socket_open(const char *url, SDL_atomic_t *stop, char *error, size_t size);
bool online_socket_send(const uint8_t *data, size_t size);
/* 0: no complete message yet; >0: message bytes; -1: disconnected or
 * closed; -2: the server broke the WebSocket protocol (never tried again,
 * online_transport.h; Windows' WinHTTP says it of what it hands over, a
 * text message, an empty one or one too long; the browser's says -1 for
 * both). */
int online_socket_receive(uint8_t *data, size_t size);
void online_socket_close(void);
/* The worker's rest between two looks at the link (issue #167): until a
 * message may have come in, online_socket_wake was called, or `ms` have
 * passed, never a spin. Where the system tells the worker of a message
 * (curl's socket polled, WinHTTP's and NSURLSession's completions) it
 * wakes at once; Android's and the 3DS's streams are looked at every
 * millisecond. */
void online_socket_wait(uint32_t ms);
/* From any thread: the worker's rest ended, a message waiting to go */
void online_socket_wake(void);

#endif
