/* pet_screens.h. The colours screen (Colours) shows nothing on a second
 * screen: the second screen keeps what it shows under the PET. */
#include "pet_screens.h"

#include "looks_screen.h"
#include "pet_forms.h"

void pet_screens_update(void) { pet_forms_update(); }

bool pet_screens_holds(void) {
	bool forms = pet_forms_holds(), looks = looks_screen_update();
	return forms || looks;
}

bool pet_screens_second(void) { return pet_forms_shown(); }

void pet_screens_draw(void) {
	pet_forms_draw();
	looks_screen_draw();
}

void pet_screens_leave(void) {
	pet_forms_leave();
	looks_screen_shut();
}

void pet_screens_describe(FILE *f) {
	pet_forms_describe(f);
	looks_screen_describe(f);
}
