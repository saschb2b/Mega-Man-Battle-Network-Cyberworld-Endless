/* Alternative research link unit; never linked with the starter script. */
#include "online_carried_script.h"
#include "online_link_script.h"

#include <mgba/core/core.h>

#include "bn6.h"

/* GBA keys: A=1, B=2, Start=8, Right=16, Up=64. These are controller
 * inputs, not a battle injection or changes to either game's RAM. */
#define BOTH(n, k) {n, {k, k}}
#define CONFIRM BOTH(4, 1), BOTH(80, 0)
#define RIGHT BOTH(4, 16), BOTH(8, 0)
#define CHARGE {110, {2, 0}}, BOTH(30, 0)
static const OnlineLinkInput entry[] = {
	BOTH(30, 0), BOTH(4, 8), BOTH(40, 0),
	BOTH(4, 64), BOTH(12, 0), BOTH(4, 64), BOTH(12, 0),
	BOTH(4, 1), BOTH(100, 0),
	/* Cable, save, saved-message dismissal, NetBattle, Single, Practice.
	 * Each menu opens on its first row: Practice (bn6f CompText86DA1B8,
	 * scripts 17/25), above Battle (18/26). No directional input changes
	 * that choice; the local screen captures verify the selected labels. */
	CONFIRM, CONFIRM, CONFIRM, CONFIRM, CONFIRM, CONFIRM,
	/* Begin connection, then wait through negotiation to the Custom screen. */
	BOTH(4, 1), BOTH(240, 0)
};
static OnlineLinkInput battle[] = {
	/* Player zero chooses the native first-slot Cannon A, then both choose OK. */
	{4, {1, 0}}, BOTH(12, 0), RIGHT, RIGHT, RIGHT, RIGHT, RIGHT,
	BOTH(4, 1), BOTH(200, 0), {4, {1, 0}}, BOTH(60, 0),
	CHARGE, CHARGE, CHARGE, CHARGE, CHARGE, CHARGE, BOTH(300, 0)
};
#undef CHARGE
#undef RIGHT
#undef CONFIRM
#undef BOTH

void online_carried_script_no_chip(void) { battle[0].keys[0] = 0; }

const OnlineLinkInput *online_link_entry_steps(unsigned *count) {
	*count = sizeof entry / sizeof *entry;
	return entry;
}

const OnlineLinkInput *online_link_battle_steps(unsigned *count) {
	*count = sizeof battle / sizeof *battle;
	return battle;
}

static unsigned total_frames(const OnlineLinkInput *steps, unsigned count) {
	unsigned frames = 0;
	for (unsigned i = 0; i < count; ++i) frames += steps[i].frames;
	return frames;
}

unsigned online_link_battle_frames(void) { return total_frames(battle, sizeof battle / sizeof *battle); }
unsigned online_link_entry_frames(void) { return total_frames(entry, sizeof entry / sizeof *entry); }
unsigned online_link_full_frames(void) { return online_link_entry_frames() + online_link_battle_frames(); }

static void script_input(struct mCore *core, unsigned side, unsigned frame, const OnlineLinkInput *steps, unsigned count) {
	uint32_t keys = 0;
	for (unsigned i = 0; i < count; ++i) {
		if (frame < steps[i].frames) { keys = steps[i].keys[side]; break; }
		frame -= steps[i].frames;
	}
	core->setKeys(core, keys);
}

void online_link_battle_input(struct mCore *core, unsigned side, unsigned frame, void *context) {
	(void)context;
	script_input(core, side, frame, battle, sizeof battle / sizeof *battle);
}

void online_link_full_input(struct mCore *core, unsigned side, unsigned frame, void *context) {
	unsigned before_battle = online_link_entry_frames();
	if (frame < before_battle) script_input(core, side, frame, entry, sizeof entry / sizeof *entry);
	else online_link_battle_input(core, side, frame - before_battle, context);
}

/* Logical observations only; core-local UI, key buffers and side differ. */
void online_link_battle_sample(struct mCore *core, unsigned side, uint32_t out[8], void *context) {
	(void)side; (void)context;
	out[0] = core->rawRead8(core, BN6_BATTLE_RESULT, -1);
	out[1] = core->rawRead8(core, BN6_BATTLE_SIDE, -1);
	out[2] = core->rawRead32(core, BN6_BATTLE_TIMER, -1);
	out[3] = core->rawRead16(core, BN6_T1_OBJECTS + BN6_T1_HP, -1) |
		(uint32_t)core->rawRead16(core, BN6_T1_OBJECTS + BN6_T1_SIZE + BN6_T1_HP, -1) << 16;
	out[4] = core->rawRead8(core, BN6_T1_OBJECTS + BN6_T1_PANEL_X, -1) |
		(uint32_t)core->rawRead8(core, BN6_T1_OBJECTS + BN6_T1_PANEL_Y, -1) << 8 |
		(uint32_t)core->rawRead8(core, BN6_T1_OBJECTS + BN6_T1_SIZE + BN6_T1_PANEL_X, -1) << 16 |
		(uint32_t)core->rawRead8(core, BN6_T1_OBJECTS + BN6_T1_SIZE + BN6_T1_PANEL_Y, -1) << 24;
	out[5] = core->frameCounter(core);
	uint32_t main = core->rawRead32(core, BN6_TOOLKIT, -1);
	uint32_t menu = core->rawRead32(core, BN6_TOOLKIT + BN6_TOOLKIT_SUBMENU, -1);
	out[6] = core->rawRead8(core, main, -1) | core->rawRead32(core, menu, -1) << 8;
	out[7] = core->rawRead32(core, BN6_BATTLE_STATE, -1);
}

int online_link_battle_agrees(const uint32_t left[8], const uint32_t right[8]) {
	return left[0] == 1 && right[0] == 2 && left[1] == 0 && right[1] == 1 &&
		left[2] > 0 && left[2] == right[2] && left[3] == (100u << 16) && right[3] == (100u << 16) &&
		left[4] == right[4] && left[6] == right[6] && left[7] == right[7] &&
		(left[6] & 0xFFFFu) == (BN6_SUBMENU_COMM << 8 | BN6_MODE_SUBMENU);
}

bool online_link_battle_active(const uint32_t left[8], const uint32_t right[8]) {
	return left[2] > 0 && right[2] > 0 && (left[7] & 65535u) == 0x0C04u && (right[7] & 65535u) == 0x0C04u;
}
