/* Linux's libcurl supplies TCP/TLS; this bounded stream supplies RFC 6455. */
#ifndef CW_ONLINE_SOCKET_H
#define CW_ONLINE_SOCKET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <SDL.h>

bool online_socket_open(const char *url, SDL_atomic_t *stop, char *error, size_t size);
bool online_socket_send(const uint8_t *data, size_t size);
/* 0: no complete message yet; >0: message bytes; -1: disconnected/error. */
int online_socket_receive(uint8_t *data, size_t size);
void online_socket_close(void);

#endif
