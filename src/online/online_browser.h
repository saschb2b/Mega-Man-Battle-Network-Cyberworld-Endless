/* The browser owns framing and verified TLS; callbacks stay on its main thread. */
#ifndef CW_ONLINE_BROWSER_H
#define CW_ONLINE_BROWSER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "online_negotiation.h"

typedef void (*OnlineBrowserMessage)(const uint8_t *data, size_t size);
typedef void (*OnlineBrowserClosed)(void);
bool online_browser_open(const char *url, const uint8_t *hello, size_t size,
                         OnlineBrowserMessage received, OnlineBrowserClosed closed, OnlineNegotiation *selection);
bool online_browser_send(const uint8_t *data, size_t size);
void online_browser_close(void);

#endif
