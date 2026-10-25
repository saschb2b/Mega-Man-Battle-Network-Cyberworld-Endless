/* What is said and shown on a liberation's board (docs/LIBERATION.md):
 * Lan and MegaMan knew BN5's Liberation Missions, Nebula's dark areas
 * taken back panel by panel, so they name what they see in its words. */
#include "liberation_words.h"

#include <stdio.h>
#include <string.h>

#include "liberation_state.h"

const char *const lib_card_line = "A layer here has gone dark";

const char *lib_briefing_words(int holes, int par) {
	static char words[320];
	snprintf(words, sizeof words, "@L Whoa! This whole layer's gone dark!|@M Like Nebula's dark areas,Lan!|"
		"@M %s out there... And a Darkloid behind %s.|@L Then let's liberate it! Par's %d phases.",
		holes == 1 ? "A DarkHole" : holes == 2 ? "Two DarkHoles" : holes == 3 ? "Three DarkHoles" : holes == 4 ? "Four DarkHoles" : "Five DarkHoles",
		holes == 1 ? "it" : "them", par);
	return words;
}

const char *lib_battle_words(int fight, bool won, bool one_turn, int panels, int holes_left, bool guardian_back) {
	static char words[200];
	if (!won) return "@M Phew... It's still dark there.";
	if (fight == LIB_FIGHT_DARKLOID) return "";
	if (fight == LIB_FIGHT_HOLE)
		return holes_left ? "@M The DarkHole's closed!" : "@M That's every DarkHole!|@L The Darkloid's open now. Let's go!";
	if (fight == LIB_FIGHT_GUARDIAN)
		return guardian_back ? "@M Got it!|@M ...Its DarkHole will send another." : "@M Got it!";
	if (one_turn && panels > 0) {
		snprintf(words, sizeof words, "@M One turn! %d more panel%s cleared!", panels, panels == 1 ? "" : "s");
		return words;
	}
	return "";
}

const char *lib_item_words(bool heart, int op) {
	static char words[80];
	if (heart) return "@M A heart! That's better.";
	snprintf(words, sizeof words, "@M Order Points! We've got %d now.", op);
	return words;
}

const char *lib_bonus_words(int outcome) {
	static const char *const said[4] = {
		"@M A BonusPanel! HP full!",
		"@M A BonusPanel! Five Order Points!",
		"@M A BonusPanel! It deleted a guardian!",
		"@M A BonusPanel! Nothing can hit us this phase!",
	};
	return said[outcome & 3];
}

const char *lib_key_words(bool sealed) {
	return sealed ? "@M A key! The barrier round that dark spot just opened!" : "@M A key! A BarrierPanel just opened!";
}

const char *lib_trap_words(int hp) {
	static char words[80];
	snprintf(words, sizeof words, "@M Ow! That panel was trapped! %d damage...", hp);
	return words;
}

const char *lib_route_words(void) {
	return "@M Lan,look! A way out of here nobody's used...|@L Another way on? Let's remember it!";
}

const char *lib_rest_words(void) { return "@M Phew... Resting helps."; }

const char *lib_team_words(const char *names) {
	static char words[160];
	if (!names || !names[0]) return "";
	snprintf(words, sizeof words, "@M But we're not alone this time!|@L Yeah! %s came along!", names);
	return words;
}

const char *lib_mate_down_words(const char *name) {
	static char words[120];
	snprintf(words, sizeof words, "@M %s!!|@L He's down... We'll have to go on without him.", name);
	return words;
}

const char *lib_strike_words(int kind, const char *darkloid) {
	/* (the guardians' as BN5's speak theirs: a cry and their attack's name;
	 * the Darkloid's in his own habit of speech, docs/VOICE.md) */
	static const char *const guardians[4] = { "", "@N Grrrah!! BeastBreath!!", "@N Kyaaah!! HawkAttack!!", "@N Out of my way!! DarkSlash!!" };
	static const struct { const char *name, *line; } darkloids[] = {
		{ "HeatMan", "C'mon!! Feel the heat!" },
		{ "ElecMan", "...tzz. Feel the current." },
		{ "SlashMan", "Slaaash!! Gotcha!" },
		{ "EraseMan", "Hyahaha! You're mine!" },
		{ "ChargeMan", "Choo,choo!! Full steam ahead!" },
		{ "SpoutMan", "G-Go away,drip!" },
		{ "TomahawkMan", "There's a saying... Strike first!" },
		{ "TenguMan", "Hmph! Too slow,grasshopper!" },
		{ "GroundMan", "Whiiir! Dig in!" },
		{ "DustMan", "Gahaha! Into the trash!" },
		{ "BlastMan", "Kwohohoho... Burn!" },
		{ "DiveMan", "Awooga! Torpedoes away!" },
		{ "CircusMan", "Ahoo,hoo,hoo! Showtime!" },
		{ "JudgeMan", "Order! Hear thy sentence!" },
		{ "ElementMan", "PIKIRARA!!|...You are in reach." },
		{ "Colonel", "Stand down. Now!" },
		{ "KnightMan", "Forgive me. I must strike!" },
		{ "ShadowMan", "...Too close." },
		{ "NumberMan", "Your number's up!" },
		{ "ToadMan", "Ribbit! Take this!" },
	};
	if (kind != UNIT_DARKLOID) return guardians[kind & 3];
	for (size_t i = 0; darkloid && i < sizeof darkloids / sizeof *darkloids; ++i)
		if (!strcmp(darkloids[i].name, darkloid)) return darkloids[i].line;
	return "You're in my reach!";
}

const char *lib_won_words(int phases, int par) {
	static char words[200];
	const char *how = phases <= par - 2 ? "@L Two under par! Way to go!" : phases <= par ? "@L Right on par!" : "@L Over par... But we did it!";
	snprintf(words, sizeof words, "@M The Darkloid's deleted!|@L %d phases...|%s|@M The exit's open,Lan!", phases, how);
	return words;
}

const char *lib_unit_name(int kind) {
	static const char *const names[4] = { "Darkloid", "BigBrute", "TinHawk", "Bladia" };
	return names[kind & 3];
}

const char *lib_command_label(int cmd, int command) {
	static const char *const labels[LIB_CMDS] = { "Liberation", "LongSwrd", "Pass" };
	/* (BN5's own names for its Navis' commands) */
	static const char *const commands[] = {
		[LIB_DO_NONE] = "", [LIB_DO_LONGSWRD] = "LongSwrd", [LIB_DO_SCRENDIV] = "ScrenDiv", [LIB_DO_TOMAHAWK] = "TomahawkSwing",
		[LIB_DO_NUMBER] = "NumberCheck", [LIB_DO_SNEAK] = "SneakAttack", [LIB_DO_MELODY] = "LifeMelody",
	};
	if (cmd == LIB_CMD_LONGSWRD && command >= 0 && command <= LIB_DO_MELODY) return commands[command];
	return cmd >= 0 && cmd < LIB_CMDS ? labels[cmd] : "";
}

const char *const lib_label_player_phase = "PLAYER PHASE", *const lib_label_enemy_phase = "DARKLOID PHASE",
	*const lib_label_phase = "PHASE", *const lib_label_par = "PAR", *const lib_label_op = "ORDER", *const lib_label_menu = "COMMAND";

const char *lib_faced_label(int kind) {
	switch (kind) {
	case LIB_DARK: case LIB_ITEM: return "Dark panel";
	case LIB_HOLE: return "DarkHole";
	case LIB_RING: return "The Darkloid's ring";
	case LIB_BONUS: return "BonusPanel";
	case LIB_TRAP: return "Trapped panel";
	case LIB_BAR: return "BarrierPanel";
	case LIB_CLEAN: return "Clean panel";
	default: return "";
	}
}
