#include "network_text.h"

const char *network_word(int word) {
	switch (word) {
	case NW_TITLE: return "NETWORK";
	case NW_NAME: return "Name";
	case NW_SERVER: return "Server";
	case NW_OFFICIAL: return "Official";
	case NW_OFFICIAL_HELP: return "Left / Right: choose a server";
	case NW_OFFICIAL_FIXED: return "The official server stays here. Use ADD A SERVER for another address.";
	case NW_ADD: return "ADD A SERVER";
	case NW_JOIN: return "JOIN SQUARE";
	case NW_BACK: return "BACK";
	case NW_SERVER_HELP: return "Left / Right: choose  A: edit";
	case NW_SAVE_FAILED: return "Could not save Network settings. Check the data folder.";
	case NW_RANDOM_FAILED: return "A private identity could not be created. Try again after restarting.";
	case NW_FULL: return "Four custom servers saved. Edit a custom server to replace its address.";
	case NW_NAME_INVALID: return "Use 1-16 letters, digits, spaces, _ or -. Start with a letter or digit.";
	case NW_URL_INVALID: return "Use ws://host:port/presence or wss://host/presence.";
	case NW_PRIVACY_TITLE: return "Join this server?";
	case NW_PRIVACY_BODY: return "Other players see your name, movement and Navi setup. This server receives your network address and a random identity. Your ROM and save stay on this device.";
	case NW_CONNECT: return "CONNECT";
	case NW_EDIT_NAME: return "Name (1-16 characters)";
	case NW_EDIT_SERVER: return "Server address";
	case NW_EDIT_HINT: return "Type or use pad. L: erase START: done";
	case NW_ERASE: return "ERASE";
	case NW_DONE: return "DONE";
	case NW_CANCEL: return "CANCEL";
	case NW_MENU_HINT: return "A: choose  B: back";
	case NW_SPACE: return "SP";
	default: return "";
	}
}
