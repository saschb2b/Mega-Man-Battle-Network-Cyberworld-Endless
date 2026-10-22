/* Local Network settings. Nothing here opens a connection. */
#ifndef CW_NETWORK_PREFS_H
#define CW_NETWORK_PREFS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define NETWORK_SERVERS 5 /* the official server, then four custom addresses */
#define NETWORK_URL 256
#define NETWORK_NAME 17
#define NETWORK_ID_BYTES 16
#define NETWORK_DEFAULT_URL "wss://cyberworld.saschb2b.com/presence"

typedef struct {
	char name[NETWORK_NAME];
	char servers[NETWORK_SERVERS][NETWORK_URL];
	uint8_t identity[NETWORK_ID_BYTES];
	unsigned count, selected, consent;
	unsigned named;   /* the player chose the name (at the gate's keeper) */
	unsigned quiet;   /* everyone else's phrases and emotes hidden in the Square (square_emote.h) */
	/* L held with up, right, down or left: what is sent, each 1 + its item
	 * (0-11 an emote, 12-19 a phrase; 0 the default: square_emote.c) */
	uint8_t favourites[4];
	unsigned hinted;  /* MegaMan named SELECT's strip in L's chat once (emote_strip.h) */
} NetworkPrefs;

bool network_name_valid(const char *name);
bool network_url_valid(const char *url);
bool network_prefs_read(const char *directory, NetworkPrefs *out);
bool network_prefs_write(const char *directory, const NetworkPrefs *prefs);
/* A random local identity, created only when none was loaded. No weak fallback. */
bool network_identity(uint8_t out[NETWORK_ID_BYTES]);
/* The local secret never leaves this device. Each server gets its own key. */
bool network_server_identity(const uint8_t secret[NETWORK_ID_BYTES], const char *url, uint8_t out[NETWORK_ID_BYTES]);

#endif
