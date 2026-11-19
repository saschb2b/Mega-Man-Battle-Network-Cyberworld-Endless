/* discord_text.h. Discord's best practice for a presence: snippets, not
 * sentences, each on one line of a friend's list (docs/engine/discord.md,
 * What friends see). Where first, then what; the picture's tooltip holds
 * the player's best. */
#include "discord_text.h"

#include <stdio.h>
#include <string.h>

/* (a middle dot between snippets, UTF-8) */
#define DOT " \xC2\xB7 "

/* ---- the activity ---- */

/* The first line: where */
static void details(const DiscordFacts *f, char *s, size_t n) {
	switch (f->place) {
	case DISCORD_TITLE: snprintf(s, n, "On the title screen"); break;
	case DISCORD_SETUP: snprintf(s, n, "Setting up a run"); break;
	case DISCORD_SUMMARY: snprintf(s, n, f->won ? "Won the run on layer %d" : "Run over on layer %d", f->layer); break;
	case DISCORD_LAYER:
		if (f->last) snprintf(s, n, "Layer %d of %d" DOT "%s", f->layer, f->last, f->area);
		else snprintf(s, n, "Layer %d" DOT "%s", f->layer, f->area);
		break;
	case DISCORD_HOME: snprintf(s, n, "Home" DOT "layer %d next", f->layer); break;
	case DISCORD_SQUARE: snprintf(s, n, "In the Net Square"); break;
	default: snprintf(s, n, "In the menus"); break;
	}
}

/* ... on a layer, what MegaMan does */
static const char *doing(const DiscordFacts *f, char *s, size_t n) {
	switch (f->doing) {
	case DISCORD_BATTLE: return "Busting viruses";
	case DISCORD_GUARDIAN: snprintf(s, n, "Fighting %s", f->foe); return s;
	case DISCORD_DUEL: return "Dueling ProtoMan";
	default: return "Exploring";
	}
}

/* The second line: what */
static void state(const DiscordFacts *f, char *s, size_t n) {
	char what[64];
	s[0] = 0;
	switch (f->place) {
	case DISCORD_SUMMARY:
		if (f->new_best) snprintf(s, n, "New best!");
		break;
	case DISCORD_LAYER:
		if (f->threat) snprintf(s, n, "%s" DOT "Threat %d", doing(f, what, sizeof what), f->threat);
		else snprintf(s, n, "%s", doing(f, what, sizeof what));
		break;
	case DISCORD_HOME: snprintf(s, n, "%s", f->doing == DISCORD_LAB ? "Battle Lab" : f->home); break;
	case DISCORD_SQUARE: snprintf(s, n, "%s", f->doing == DISCORD_NETBATTLE ? "NetBattling" : "Online"); break;
	default: break;
	}
}

void discord_words(const DiscordFacts *f, DiscordActivity *a) {
	details(f, a->details, sizeof a->details);
	state(f, a->state, sizeof a->state);
	if (f->best > 0) snprintf(a->large_text, sizeof a->large_text, "Best: layer %d" DOT "%d run%s", f->best, f->runs, f->runs == 1 ? "" : "s");
	else snprintf(a->large_text, sizeof a->large_text, "Cyberworld Endless %s", f->version);
}

/* ---- settings.ini ---- */

const char *discord_setting_note(void) {
	return "\n# discord: on shows your friends on Discord that you play and how far\n"
		"# your run has come, through the Discord app on this computer (as its\n"
		"# own Activity Privacy settings allow). off shows nothing.\n";
}
