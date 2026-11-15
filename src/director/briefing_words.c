/* L's briefing (docs/voice/rules.md): what guards the layer, two things
 * worth a detour at most (the heal while he is hurt among them) and the way
 * on; and his word on the last stop before a guardian's arena. */
#include "briefing_words.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "blockers.h"
#include "boss.h"
#include "cinema.h"
#include "director_folder.h"
#include "director_home.h"
#include "director_layer.h"
#include "director_map.h"
#include "director_see.h"
#include "director_state.h"
#include "director_way.h"
#include "emu.h"
#include "guardians.h"
#include "loot.h"
#include "netmap.h"
#include "save.h"
#include "talk.h"
#include "town.h"

/* The way's winding, said in a box of its own, which the words around
 * end: for the heal and the last stop's services, where the arrow leads */
#define WINDS ".|@M It's a winding walk"

/* Where world (wx, wy) is, for L's words, in buf: its lie and how far the
 * walk there is (as the crow flies, "close by" named ProtoMan across a
 * gap). Not whether it winds, a box each that no arrow follows (the main
 * path's words cut, docs/game/core-loop.md). */
static const char *spot_where(int wx, int wy, char *buf, size_t n) {
	static const char *const dist[3] = { "close by", "a ways off", "far off" };
	int far;
	bool winds;
	const char *lies = lie_and_walk(wx, wy, &far, &winds);
	snprintf(buf, n, "%s,%s", lies, dist[far]);
	return buf;
}

/* Where ProtoMan waits, for L's words: the lie of his pink mark, how far
 * the walk to him is, and whether it winds; NULL while no duel waits. */
static const char *rival_where(void) {
	static char buf[96];
	int wx, wy;
	return duel_waiting(&wx, &wy) ? spot_where(wx, wy, buf, sizeof buf) : NULL;
}

/* MegaMan's words on the layer's guardian, on arriving: who he is once
 * they have fought him, in any run, and of how he fights only the move
 * that deletes unwarned (guardian_warning): the rest is Dad's mail's, one
 * channel a fact (the main path's words cut, docs/game/core-loop.md). The
 * whole tip, strategy, stays here for BN5's guardians and the super
 * bosses, whom no mail covers (noted for the BBS, docs/voice/sweep/net.md).
 * Before a fight only hearsay or a strong signal, as naming him would
 * spend the first fight's discovery (and how could he know?). The yellow
 * panels and EraseCross's bug are BN6's own, unsaid (the voice sweep).
 * Appended to `buf` at `k`; the new length. */
static int guardian_words(char *buf, int k, int size) {
	#define ADD(...) (k += snprintf(buf + k, k < size ? (size_t)(size - k) : 0, __VA_ARGS__))
	int navi = D.objs.guardian.navi;
	const char *tip = guardian_mailed(navi) ? guardian_warning(navi) : guardian_tip(navi);
	/* (what a Navi on the net said, as hearsay; a super boss sensed,
	 * docs/game/bosses/super-bosses.md) */
	if (guardian_known(navi)) {
		ADD("@M %s waits at the end!|", guardian(navi)->name);
		if (tip && !guardian_mailed(navi)) ADD("@M We've got battle data on him!|");
		if (tip) ADD("@M %s|", tip);
	} else if (navi == SUPER_CYBEAST) ADD("@M Something huge waits at the end...|@M It's growling,Lan. Like the beast in me.|");
	else if (navi == SUPER_BASS) ADD("@M A dark signal waits at the end...|@M It doesn't feel like a copy,Lan.|");
	else if (guardian_heard()) ADD("@M Word is,%s waits at the end!|", guardian(navi)->name);
	else ADD("@M A strong Navi's signal waits at the end...|");
	/* (the Net's clock: his copy a tenth stronger a notch, docs/world/home/) */
	if (run.clock) ADD("@M The Net kept copying. %s tougher,Lan!|", guardian_known(navi) ? "He's" : "It's");
	#undef ADD
	return k;
}

/* L's word on a heal while MegaMan is hurt, appended to `buf` at `k`; the
 * new length: which way the Recovery Mr. Prog is, the arrow's way while he
 * is hurt (goal_way), else the Net Dealer, who always has a MiniEnrg (it
 * was on no map yet, and never found: a playtester at 90 of 240 ran to the
 * exit past the dealer, then a dozen moves back). */
static int heal_note(char *buf, int k, int size, bool heal) {
	static const char *const near_far[3] = { "close by", "a ways off", "far off" };
	for (int i = 0; i < layer.nobj; ++i) {
		if (layer.obj[i].type != (heal ? OBJ_HEAL : OBJ_SHOP)) continue;
		int wx, wy, hf;
		bool winds;
		netmap_world((int)layer.obj[i].x, (int)layer.obj[i].y, &wx, &wy);
		/* (where it lies: the way's first leg pointed off from it; and
		 * that the way winds where it does, as the arrow follows it: "it's
		 * straight down" under an arrow pointing up and to the left read as
		 * two ways, session 62) */
		const char *hw = lie_and_walk(wx, wy, &hf, &winds);
		const char *wind = winds ? WINDS : "";
		k += snprintf(buf + k, k < size ? (size_t)(size - k) : 0,
			heal ? "@M Let's visit the Recovery Mr.Prog!|@M He's %s,%s%s.|"
			     : "@M The Net Dealer sells MiniEnrg,Lan!|@M He's %s,%s%s.|", hw, near_far[hf], wind);
		break;
	}
	return k;
}

/* L's last words, the way on, appended to `buf` at `k`; the new length:
 * along the floor where MegaMan can walk it (the arrow's way); where the
 * walk sets off well away from where the goal lies, where it lies, which
 * holds still as the walk winds; after the heal's words (`to_heal`, the
 * arrow leading there first), where the goal lies: "the way on" read as
 * the arrow's, which led to the heal, and its first leg is from here, not
 * from the heal (session 62). */
static int goal_words(char *buf, int k, int size, bool to_heal, bool told) {
	#define ADD(...) (k += snprintf(buf + k, k < size ? (size_t)(size - k) : 0, __VA_ARGS__))
	int far;
	bool to_guardian = D.objs.guardian.navi && !boss_beaten();
	int gx = D.objs.exit_x, gy = D.objs.exit_y;
	if (to_guardian) { gx = D.objs.guardian.x; gy = D.objs.guardian.y; }
	const char *lies = way_to(gx, gy, &far), *what = to_guardian ? "guardian" : "exit";
	int lies_dir = way_last(), seen_far = far;
	const char *way = route_to(gx, gy, &far);
	if (!way) way = way_to(gx, gy, &far);
	int apart = to_heal ? 0 : abs(way_last() - lies_dir);
	if (apart > 4) apart = 8 - apart;
	static const char *const how_far[3] = { "It's close!", "It's a ways off.", "It's a long way yet." };
	if (D.objs.guardian.navi && !boss_done() && boss_beaten()) ADD("@M Let's grab the Guardian Data,Lan!");
	/* (the words where it lies, the arrow the walk: say they part) */
	else if (apart >= 2 && told) ADD("@M The %s's %s!|@M It's a winding walk. Follow the arrow!", what, lies);
	else if (apart >= 2) ADD("@M The %s's %s. %s|@M It's a winding walk. Follow the arrow!", what, lies, how_far[far]);
	else if (to_heal) ADD("@M Then the %s's %s. %s", what, lies, how_far[seen_far]);
	else ADD("@M Let's head %s! %s", way, how_far[far]);
	#undef ADD
	return k;
}

/* Where a set piece keeps Mystery Data k, as L says it ("behind the
 * Security Cube"), or NULL: the counters count data MegaMan senses there,
 * and a playtester's read 0/1 with nothing in sight and no word of where
 * (session 63). The lock it is behind in *lock (layer.block[]), -1 none. */
static const char *prize_where(int k, int *lock) {
	const NetObj *o = &layer.obj[D.objs.md_obj[k]];
	int x = (int)o->x, y = (int)o->y;
	*lock = -1;
	if (!o->prize) return NULL;
	for (int i = 0; i < layer.nblocks; ++i) {
		const NetBlock *b = &layer.block[i];
		if (b->rx != x || b->ry != y) continue;
		*lock = i;
		return b->kind < BLOCK_KINDS ? "behind the Link Navi's obstacle" : b->kind == BLOCK_SKULL ? "behind the skull door"
			: b->kind == BLOCK_NUMBER ? "behind the number door" : blocker_is_cube(i) ? "behind the Security Cube" : "behind the cyberbarrier";
	}
	if (layer.nteleports && layer.teleport_island && abs(x - layer.teleport_x[0]) <= 3 && abs(y - layer.teleport_y[0]) <= 3)
		return "past the teleport pads";
	for (int g = 0; g < layer.ngaps; ++g)
		if (layer.gap[g].island) return "across the gap by the bone panels";
	return NULL;
}

/* Where lock `lock` stands while it is shut, for L's words after the data
 * he senses behind it, as his words on ProtoMan and the heal point (a
 * playtester heard "behind the security cube", was told its P-Code by a
 * navi and never found the cube, session 64): appended to `buf` at `k`,
 * the new length. */
static int lock_words(char *buf, int k, int size, int lock) {
	if (lock < 0 || !lock_shut(lock)) return k;
	const NetBlock *b = &layer.block[lock];
	const char *what = b->kind < BLOCK_KINDS ? "obstacle" : b->kind == BLOCK_SKULL || b->kind == BLOCK_NUMBER ? "door" : blocker_is_cube(lock) ? "cube" : "cyberbarrier";
	char where[96];
	int wx, wy;
	netmap_world(b->x, b->y, &wx, &wy);
	return k + snprintf(buf + k, k < size ? (size_t)(size - k) : 0, "@M The %s's %s!|", what, spot_where(wx, wy, where, sizeof where));
}

/* The data MegaMan senses but has not seen, said where it lies (`most`
 * of them, two at most), and the locks it is behind, appended to `buf` at
 * `k`; the new length. */
static int prize_note(char *buf, int k, int size, int most) {
	const char *where[2];
	int n = 0, lock[2] = { -1, -1 };
	for (int i = 0; i < D.objs.nmd && n < most && n < 2; ++i) {
		int at = -1;
		const char *w = flag_get(MAPSLOT_MD_FLAG + i) || md_on_map(i) || !md_known(i, false) ? NULL : prize_where(i, &at);
		if (w && !(n && !strcmp(w, where[0]))) { lock[n] = at; where[n++] = w; }
	}
	if (n == 2) k += snprintf(buf + k, k < size ? (size_t)(size - k) : 0, "@M I sense Mystery Data %s!|@M And more %s!|", where[0], where[1]);
	else if (n == 1) k += snprintf(buf + k, k < size ? (size_t)(size - k) : 0, "@M I sense Mystery Data %s!|", where[0]);
	for (int j = 0; j < n; ++j) k = lock_words(buf, k, size, lock[j]);
	return k;
}

/* What stands on the layer, for L's words. */
typedef struct {
	bool shop, heal;
} Here;

/* The flame of darkness still burning on the layer (docs/game/meta/): L
 * names it and where, and the map marks it, an Event; a playtester walked
 * two layers of ACDC Area past one, L naming every service but it, the
 * map marking only ProtoMan (session 66). */
bool flame_here(void) { return D.objs.dark_flame_obj >= 0 && !flag_get(LAYER_DARK_TAKEN_FLAG); }

static int flame_words(char *buf, int k, int size) {
	static const char *const near_far[3] = { "close by", "a ways off", "far off" };
	const NetObj *o = &layer.obj[D.objs.dark_flame_obj];
	int wx, wy, far;
	bool winds;
	netmap_world((int)o->x, (int)o->y, &wx, &wy);
	const char *lies = lie_and_walk(wx, wy, &far, &winds);
	return k + snprintf(buf + k, k < size ? (size_t)(size - k) : 0, "@M A flame of darkness burns %s,%s!|", lies, near_far[far]);
}

static void here_scan(Here *h) {
	memset(h, 0, sizeof *h);
	for (int i = 0; i < layer.nobj; ++i) {
		h->shop |= layer.obj[i].type == OBJ_SHOP;
		h->heal |= layer.obj[i].type == OBJ_HEAL && !heal_spent();
	}
}

/* L's first words on a layer: what guards it, a box each ended by "|",
 * appended to `buf` at `k`; the new length. The layer and the area are
 * the screen's corner and the act card's, unsaid (the main path's words
 * cut, one channel a fact: docs/game/core-loop.md); so is whose viruses
 * an older Net's area holds, the card's "Older Net" and MegaMan's words on
 * arriving at the first (docs/multirom/one-net.md: L said it on every
 * layer there, session 77). */
static int first_words(char *buf, int k, int size) {
	#define ADD(...) (k += snprintf(buf + k, k < size ? (size_t)(size - k) : 0, __VA_ARGS__))
	/* (a trip back: its way home, docs/world/home/) */
	if (run.side_kind == LAYER_BACK) ADD("@M We're back in %s,Lan!|@M The exit pad takes us home.|", guardian_area_in_text(run.biome, run.side_kind));
	/* (on his own layer only: before it, the act card names him as the act
	 * begins and the Net Dealer at his pick, one channel a decision) */
	if (D.objs.guardian.navi && !boss_beaten()) k = guardian_words(buf, k, size);
	#undef ADD
	return k;
}

/* The map's violet marks L names among the detours, by their worth: a
 * way on (the Undernet's warp, the Secret Area's gate, a Navi's gate), a
 * prize for a fight (a Server's), a vault, Chaud's gate last. */
static const struct { int type; const char *name; } event_talk[] = {
	{ OBJ_UNDERNET, "a dark warp" }, { OBJ_SECRET_GATE, "the golden gate" }, { OBJ_NAVI_GATE, NULL },
	{ OBJ_CHALLENGE, "a strong virus signal" }, { OBJ_VAULT, "a collector's vault" }, { OBJ_OFFICIAL, "an official gate" },
};

/* The first of the map's violet marks on the layer, as L says it ("I
 * sense a dark warp to the upper left!"), appended to `buf` at `k`; the
 * new length, `k` for none. Where it lies, not how far: the map marks it. */
static int event_words(char *buf, int k, int size) {
	for (unsigned e = 0; e < sizeof event_talk / sizeof *event_talk; ++e)
		for (int i = 0; i < layer.nobj; ++i) {
			if (layer.obj[i].type != event_talk[e].type) continue;
			char sealed[48];
			const char *name = event_talk[e].name;
			if (!name) {
				snprintf(sealed, sizeof sealed, "a gate with %s's code", guardian(D.objs.gate_navi)->name);
				name = sealed;
			}
			int wx, wy, far;
			bool winds;
			netmap_world((int)layer.obj[i].x, (int)layer.obj[i].y, &wx, &wy);
			return k + snprintf(buf + k, k < size ? (size_t)(size - k) : 0, "@M I sense %s %s!|", name, lie_and_walk(wx, wy, &far, &winds));
		}
	return k;
}

/* L's first words on a layer, after what guards it: the things worth a
 * detour, `most` of them, by their worth (ProtoMan waiting, a flame of
 * darkness, one of the map's violet marks, Mystery Data sensed behind a
 * lock); appended to `buf` at `k`, the new length. The services and the
 * rest are the map's key's, and each piece says itself at A: L named them
 * all at a second press, nine boxes after six (session 76), and before that
 * on arriving (eight boxes in act 3). The tips that rode along went where
 * they are needed (session 77: ten and twelve boxes on arriving, two
 * detours among them): the area's battlefields in the arrival's words
 * (arrival_words), a program off the board after a battle and the PET
 * (off_board_words), the map at L's next press (map_tip). */
static int detour_words(char *buf, int k, int size, int most) {
	int said = 0;
	/* (where the rival waits: the map showed him as the official gate's
	 * violet, and a playtester's session ran out at the gate, alone,
	 * looking for him; his race named in Chaud's call, which says why) */
	const char *rw = said < most ? rival_where() : NULL;
	if (rw) { k += snprintf(buf + k, k < size ? (size_t)(size - k) : 0, "@M ProtoMan's waiting %s!|", rw); ++said; }
	if (said < most && flame_here()) { k = flame_words(buf, k, size); ++said; }
	if (said < most) {
		int k0 = k;
		k = event_words(buf, k, size);
		if (k > k0) ++said;
	}
	if (said < most) k = prize_note(buf, k, size, 1);
	return k;
}

/* The map's tip, at L's second press on a layer until the map has been
 * held, once a game: a player who asks again for the way is the one it
 * helps. It rode along on L's first words on the run's first two layers,
 * and a CONTINUE heard it again (session 77). Appended to `buf` at `k`;
 * the new length. */
static int map_tip(char *buf, int k, int size) {
	static bool said;
	if (said || map_was_held()) return k;
	said = true;
	return k + snprintf(buf + k, k < size ? (size_t)(size - k) : 0, "@M Hold SELECT for a map of where we've been!|");
}

/* What MegaMan says when L is pressed: where they are, what is ahead. */
const char *status_words(void) {
	static char buf[1400];
	int k = 0;
	#define ADD(...) (k += snprintf(buf + k, k < (int)sizeof buf ? sizeof buf - (size_t)k : 0, __VA_ARGS__))
	/* (home's own way on, docs/world/home/: no landmark's port there) */
	if (D.town && town_is_home()) return home_status();
	if (D.town) {
		int far;
		const char *way = town_way(&far);
		/* the landmark the first time, then only the way (how to jack in
		 * is BN6's own: the voice sweep, docs/voice/sweep/net.md) */
		if (!D.port_told) ADD("@M The port's %s,by the %s!", way, town_info()->landmark);
		else ADD("@M The port's %s,Lan!", way);
		D.port_told = true;
		return buf;
	}
	/* what guards it, what is worth a detour, then the heal while hurt and
	 * the way on (a playtester paged twelve and thirteen boxes on
	 * arriving, and asked for the guardian, the heal and the way first,
	 * session 63); after that, the heal while hurt, ProtoMan while he
	 * waits, the map's tip once and the way */
	D.layer_told |= flag_get(LAYER_TOLD_FLAG);
	bool told = D.layer_told;
	Here h;
	here_scan(&h);
	bool hurt = hurt_now(), vow = hurt && jobs_vow_holds(&run.job, run_reached());
	/* (the heal while hurt is one of the two things worth a detour: the
	 * arrow leads there, and a playtester counted it a third, session 77) */
	bool heal_said = vow || (hurt && (h.heal || h.shop));
	heal_turn_reset();
	if (!told) {
		k = first_words(buf, k, (int)sizeof buf);
		k = detour_words(buf, k, (int)sizeof buf, heal_said ? 1 : 2);
		D.layer_told = true;
		flag_set(LAYER_TOLD_FLAG);
	}
	/* (ProtoMan before the heal, as on arriving: "Then the exit's" after
	 * him read as the way on from ProtoMan) */
	const char *rival = told ? rival_where() : NULL;
	if (rival) ADD("@M ProtoMan's waiting %s!|", rival);
	if (told && !(heal_said && rival)) k = map_tip(buf, k, (int)sizeof buf);
	/* (a vow held: no patch for us, the dealer's MiniEnrg instead) */
	if (vow) ADD("@M No patch-ups,Lan. We made a vow!|");
	if (hurt && (h.heal || h.shop)) k = heal_note(buf, k, (int)sizeof buf, h.heal);
	k = goal_words(buf, k, (int)sizeof buf, hurt && h.heal, told);
	#undef ADD
	goal_way();   /* (the arrow's way, which the words that follow start) */
	return buf;
}

/* The last stop before a guardian's arena: the Net Dealer and a heal stand
 * in the room before it or one beside it (net_gen.c), off the arrow's
 * line, and a playtester walked past both to SpoutMan at 120 of 140 HP
 * with 1150 zenny unspent. On the way in (the room before the arena, its
 * bridge), the arena within ARENA_AHEAD panels' walk, MegaMan names those
 * not yet used, once, and which way each is. */
/* (where a service lies from MegaMan, as L and the map give it, and how
 * far the walk there is: "right here,to the left", "to the lower right",
 * "a long way back,straight up"; where the walk sets off another
 * way, that it winds: the walk's first step, "up and to the right", named
 * a heal that L and the map put up and to the left) */
static const char *service_where(int wx, int wy, char *buf, size_t n) {
	int far;
	bool winds;
	const char *lies = lie_and_walk(wx, wy, &far, &winds);
	snprintf(buf, n, far == 0 ? "right here,%s%s" : far == 1 ? "%s%s" : "a long way back,%s%s", lies, winds ? WINDS : "");
	return buf;
}

/* The last stop's first box: whose arena is just ahead (a super boss's
 * sensed until they have met: docs/game/bosses/super-bosses.md); its length */
static int arena_ahead(char *buf, size_t n) {
	int navi = D.objs.guardian.navi;
	bool known = guardian_known(navi);
	if (navi == SUPER_CYBEAST)
		return snprintf(buf, n, "%s", known ? "@M The Cybeast's den is just ahead,Lan!|@M " : "@M The growling's coming from just ahead!|@M ");
	if (navi == SUPER_BASS) return snprintf(buf, n, "%s", known ? "@M Bass is just ahead,Lan!|@M " : "@M That dark signal's just ahead,Lan!|@M ");
	if (known || guardian_heard()) return snprintf(buf, n, "@M %s's arena is just ahead,Lan!|@M ", guardian(navi)->name);
	return snprintf(buf, n, "@M The guardian's arena is just ahead,Lan!|@M ");
}

/* Whether the last stop names heal `o`: one not yet told of while MegaMan
 * is hurt, and not under a vow: L had just said "No Mr.Prog patch-ups this
 * time!", and MegaMan named the Mr.Prog at the arena's door (session 72) */
static bool heal_to_name(const NetObj *o, int hp, int max) {
	return o->type == OBJ_HEAL && !flag_get(LAYER_HEAL_TOLD_FLAG) && !heal_spent() && hp < max;
}

/* (how near the arena is "just ahead": said on stepping into the room
 * before it, fourteen panels' walk across an 8x8 room from SpoutMan's,
 * and the map's mark of him on its frame's edge, a playtester heard it far
 * off; eight is the bridge and a few steps of the room) */
#define ARENA_AHEAD 8

void last_stop(int cx, int cy) {
	if (D.last_stop_told || layer.ante < 0 || !D.objs.guardian.navi || boss_beaten() || boss_fighting()) return;
	if (emu_read8(BN6_CHATBOX) || talk_busy() || cinema_busy() || D.warping || D.map_shown) return;
	int walk = arena_walk(cx, cy);
	if (walk < 0 || walk > ARENA_AHEAD) return;
	D.last_stop_told = true;
	int hp = emu_read16(BN6_NAVI_HP), max = emu_read16(BN6_NAVI_MAX_HP);
	const char *dealer = NULL, *heal = NULL;
	static char dway[80], hway[80];
	for (int i = 0; i < layer.nobj; ++i) {
		const NetObj *o = &layer.obj[i];
		int wx, wy;
		netmap_world((int)o->x, (int)o->y, &wx, &wy);
		if (o->type == OBJ_SHOP && !dealer && !flag_get(LAYER_DEALER_TOLD_FLAG)) dealer = service_where(wx, wy, dway, sizeof dway);
		if (!heal && heal_to_name(o, hp, max)) heal = service_where(wx, wy, hway, sizeof hway);
	}
	if (!dealer && !heal) return;
	static char buf[400];
	int k = arena_ahead(buf, sizeof buf);
	/* (no running once in is BN6's own, as from its story bosses, unsaid:
	 * the voice sweep, docs/voice/sweep/net.md) */
	/* (a suggestion, not a question: "Want to get ready first?" asked one
	 * that took no answer, session 77) */
	if (dealer && heal) snprintf(buf + k, sizeof buf - (size_t)k, "Let's get ready first!|@M The Net Dealer's %s.|@M A Recovery Mr.Prog's %s.", dealer, heal);
	else if (dealer) snprintf(buf + k, sizeof buf - (size_t)k, "Let's get ready first!|@M The Net Dealer's %s.", dealer);
	else snprintf(buf + k, sizeof buf - (size_t)k, "Let's heal up first!|@M A Recovery Mr.Prog's %s.", heal);
	talk_start(buf, FACE_MEGAMAN);
}
