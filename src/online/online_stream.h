/* The worker's byte stream. TLS verifies both the certificate and hostname. */
#ifndef CW_ONLINE_STREAM_H
#define CW_ONLINE_STREAM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <SDL.h>

bool online_stream_open(const char *url, SDL_atomic_t *stop, char *error, size_t size);
bool online_stream_send(const uint8_t *data, size_t size);
int online_stream_receive(uint8_t *data, size_t size);
bool online_stream_random(uint8_t *out, size_t size);
void online_stream_close(void);

#endif
