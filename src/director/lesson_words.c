/* MegaMan's word on R pressed in the town away from its port (docs/VOICE.md):
 * where the port is, never how to jack in, BN6's own (Rush's bone panels'
 * lesson went in the voice sweep of 19 October 2026: BN6's RushFood, and
 * A named on every layer; docs/voice/sweep/net.md). */
#include "lesson_words.h"

#include <stdio.h>

#include "town.h"

/* MegaMan's word on R pressed in the town away from the port: almost at a
 * jack-in cell (`way` the way to it), close to the port (`way` the walk's),
 * or nowhere near it */
const char *port_words(int how, const char *way) {
	static char buf[160];
	if (how == PORT_ROOM)
		snprintf(buf, sizeof buf, "@M The PC's %s,Lan!", way);
	else if (how == PORT_HOUSE)
		snprintf(buf, sizeof buf, "@M Your room's %s,Lan!|@M The PC's up there!", way);
	else if (how == PORT_HOME)
		snprintf(buf, sizeof buf, "@M Home's %s,Lan!|@M The PC's up in your room!", way);
	else if (how == PORT_OUT)
		snprintf(buf, sizeof buf, "@M The way out's %s,Lan!|@M Your PC's back home,remember?", way);
	else if (town_is_home())
		snprintf(buf, sizeof buf, "%s", how == PORT_AWAY ? "@M No port here,Lan!|@M I jack in from your PC at home!" :
			"@M Home's right here,Lan!|@M The PC's up in your room!");
	else if (how == PORT_ALMOST_CELL || how == PORT_ALMOST)
		snprintf(buf, sizeof buf, "@M Almost,Lan! The %s's %s.", town_info()->landmark, way);
	else
		snprintf(buf, sizeof buf, "@M No port here,Lan!|@M It's by the %s!", town_info()->landmark_at);
	return buf;
}
