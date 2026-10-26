/* A server's address as a player types it at the gate's keeper (docs/online/square/overview.md,
 * The gate): "host", "host:port" or a whole ws(s):// URL, made the URL the
 * transport opens. A host that can only be on the player's own network (a
 * private address, localhost, a .local or .lan name, a name with no dot) is
 * a Square started bare on a computer there, plain WebSocket on the server's
 * port: ws://HOST:8765. Any other is wss:// on its own port. A scheme or a
 * port typed is kept as typed. Nothing here opens a connection. */
#ifndef CW_NETWORK_ADDRESS_H
#define CW_NETWORK_ADDRESS_H

#include <stdbool.h>
#include <stddef.h>

#define NETWORK_LOCAL_PORT "8765" /* the server's own, started with nothing set (server/README.md) */

/* Whether `host` (no port; an IPv6 address in brackets) can only be on this
 * network: 10/8, 172.16/12, 192.168/16, 100.64/10 (a carrier's or a VPN's
 * like Tailscale's), 127/8, 169.254/16; localhost; a name ending .local,
 * .lan, .home, .home.arpa, .internal or .localdomain, or with no dot;
 * [::1], fc00::/7 and fe80::/10. */
bool network_address_local(const char *host);
/* The server's URL from what was typed, wss:// or ws:// and /presence added
 * where left out; false where it is no valid one or does not fit `cap`. */
bool network_address_url(const char *typed, char *out, size_t cap);

#endif
