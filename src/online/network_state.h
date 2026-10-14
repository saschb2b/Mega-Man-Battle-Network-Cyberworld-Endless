#ifndef CW_NETWORK_STATE_H
#define CW_NETWORK_STATE_H

#include <SDL.h>

#include "network_prefs.h"

enum { NETWORK_HOME, NETWORK_EDIT_NAME, NETWORK_EDIT_SERVER, NETWORK_PRIVACY };
enum { NETWORK_ROW_NAME, NETWORK_ROW_SERVER, NETWORK_ROW_ADD, NETWORK_ROW_JOIN, NETWORK_ROW_BACK, NETWORK_ROWS };

typedef struct {
	NetworkPrefs prefs;
	int view, row, key, privacy;
	bool identity_ok, adding, wait_touch;
	char edit[NETWORK_URL], note[160];
} NetworkState;

extern NetworkState N;
void network_draw(void);
SDL_Rect network_home_rect(int row);
void network_edit_begin(bool name, bool adding);
void network_edit_update(void);
void network_edit_draw(void);
bool network_back(void);
bool network_store(void);

#endif
