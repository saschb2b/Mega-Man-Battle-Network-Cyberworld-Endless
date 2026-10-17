#include "network_text.h"

static const char *const words[NW_COUNT] = {
	[NW_TITLE] = "NETWORK",
	[NW_NAME] = "Name",
	[NW_SERVER] = "Server",
	[NW_OFFICIAL] = "Official",
	[NW_OFFICIAL_HELP] = "Left / Right: choose a server",
	[NW_OFFICIAL_FIXED] = "The official server stays here. Use ADD A SERVER for another address.",
	[NW_ADD] = "ADD A SERVER",
	[NW_JOIN] = "JOIN SQUARE",
	[NW_BACK] = "BACK",
	[NW_SERVER_HELP] = "Left / Right: choose  A: edit",
	[NW_SAVE_FAILED] = "Could not save Network settings. Check the data folder.",
	[NW_RANDOM_FAILED] = "A private identity could not be created. Try again after restarting.",
	[NW_FULL] = "Four custom servers saved. Edit a custom server to replace its address.",
	[NW_NAME_INVALID] = "Use 1-16 letters, digits, spaces, _ or -. Start with a letter or digit.",
	[NW_URL_INVALID] = "Use ws://host:port/presence or wss://host/presence.",
	[NW_PRIVACY_TITLE] = "Join this server?",
	[NW_PRIVACY_BODY] = "Other players see your name, movement and Navi setup. This server receives your network address and a random identity. Your ROM and save stay on this device.",
	[NW_CONNECT] = "CONNECT",
	[NW_EDIT_NAME] = "Name (1-16 characters)",
	[NW_EDIT_SERVER] = "Server address",
	[NW_EDIT_HINT] = "Type or use pad. L: erase START: done",
	[NW_ERASE] = "ERASE",
	[NW_DONE] = "DONE",
	[NW_CANCEL] = "CANCEL",
	[NW_MENU_HINT] = "A: choose  B: back",
	[NW_SPACE] = "SP",
};

const char *network_word(int word) {
	return word >= 0 && word < NW_COUNT ? words[word] : "";
}
