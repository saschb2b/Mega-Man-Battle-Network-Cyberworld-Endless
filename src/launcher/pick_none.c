/* pick.h where there is no launcher: the 3DS, a PortMaster handheld (its
 * ROM in the port's own rom/ folder) and the browser (its page chooses
 * the ROMs, web/play/app.js). */
#include "pick.h"

#if !defined(CW_DESKTOP) && !defined(CW_IOS) && !defined(__ANDROID__)
unsigned pick_kinds(void) { return 0; }

bool pick_open(int kind, int slot) {
	(void)kind;
	(void)slot;
	return false;
}

bool pick_busy(void) { return false; }

bool pick_done(PickResult *r) {
	(void)r;
	return false;
}

int pick_look(char *msg, size_t n) {
	(void)msg;
	(void)n;
	return -1;
}

bool pick_folder(char *name, size_t n) {
	(void)name;
	(void)n;
	return false;
}

bool pick_saves_put(const char *from) {
	(void)from;
	return false;
}
#else
typedef int pick_none_unused;
#endif
