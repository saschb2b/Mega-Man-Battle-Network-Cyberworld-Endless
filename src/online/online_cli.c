#include "online_visit.h"

#include <stdio.h>

#include "game.h"
#include "network_prefs.h"
#include "online_visit_text.h"

bool online_visit_command(const char *url, const char *name, char *error, size_t cap) {
	NetworkPrefs prefs;
	bool identity = network_prefs_read(g_data_dir, &prefs);
	if (!identity) identity = network_identity(prefs.identity) && network_prefs_write(g_data_dir, &prefs);
	uint8_t server_identity[NETWORK_ID_BYTES];
	if (!identity || !network_server_identity(prefs.identity, url, server_identity)) {
		snprintf(error, cap, "%s", online_visit_identity_failed);
		return false;
	}
	return online_visit_begin(url, name ? name : prefs.name, server_identity, error, cap);
}
