/* What bystander navis say on the Cyberworld's layers (the game's chat box
 * shows them, wrapped to its width). */
#include "npc_lines.h"

static const char *lines[] = {
	"Purple Mystery Data won't open without an Unlocker.",
	"They say the Graveyard hides warps to the Undernet...",
	"Viruses gather where the panels glow red. Dangerous, but the data there is worth it.",
	"Three fragments of Secret Data open a gate in the Undernet. Nobody knows what waits inside.",
	"Chips that share a code can be sent together. Stack them!",
	"Hold B to charge your buster. A charged shot can make a Navi flinch.",
	"Every third area, a Navi guards the exit. Beat them and they'll lend you their power.",
	"Chip Traders swap three chips for one. Great for clearing out junk.",
	"The Custom gauge is full when it flashes. Press L or R to pick new chips.",
	"Beyond the Undernet lies the Cybeast's nest. It feeds on the whole net...",
	"Programs from the NaviCust vendor stay installed for the whole run.",
	"AreaGrab steals the enemy's front column. More room to move, less room to hide!",
};

const char *npc_line(int i) {
	int n = (int)(sizeof lines / sizeof *lines);
	return lines[((i % n) + n) % n];
}
