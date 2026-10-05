/* L's briefing (docs/VOICE.md): where MegaMan and Lan are, what guards the
 * layer, what else it holds, the heal while he is hurt and the way on; and
 * his word on the last stop before a guardian's arena. */
#include "briefing_words.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "board_words.h"
#include "boss.h"
#include "cinema.h"
#include "director_board.h"
#include "director_folder.h"
#include "director_layer.h"
#include "director_map.h"
#include "director_see.h"
#include "director_state.h"
#include "director_way.h"
#include "emu.h"
#include "encounter.h"
#include "guardians.h"
#include "loot.h"
#include "netmap.h"
#include "save.h"
#include "talk.h"
#include "town.h"

/* Where world (wx, wy) is, for L's words, in buf: its lie, how far the
 * walk there is, and whether it winds (as the crow flies, "close by" named
 * ProtoMan across a gap). The winding said in a box of its own (WINDS),
 * which the words around end. */
#define WINDS ".|@M It's a winding walk"
static const char *spot_where(int wx, int wy, char *buf, size_t n) {
	static const char *const dist[3] = { "close by", "a ways off", "far off" };
	int far;
	bool winds;
	const char *lies = lie_and_walk(wx, wy, &far, &winds);
	snprintf(buf, n, "%s,%s%s", lies, dist[far], winds ? WINDS : "");
	return buf;
}

/* Where ProtoMan waits, for L's words: the lie of his pink mark, how far
 * the walk to him is, and whether it winds; NULL while no duel waits. */
static const char *rival_where(void) {
	static char buf[96];
	int wx, wy;
	return duel_waiting(&wx, &wy) ? spot_where(wx, wy, buf, sizeof buf) : NULL;
}

/* MegaMan's words on the layer's guardian, on arriving: who he is and how
 * he fights once they have fought him, in any run: before that MegaMan has
 * no battle data on the copy, only a strong signal, and naming him or
 * reciting his moves would spend the first fight's discovery (and how
 * could he know?); what he always knows is the net's own grammar, the
 * yellow panels that light where an attack will land. Appended to `buf`
 * at `k`; the new length. */
static int guardian_words(char *buf, int k, int size) {
	#define ADD(...) (k += snprintf(buf + k, k < size ? (size_t)(size - k) : 0, __VA_ARGS__))
	int navi = D.objs.guardian.navi;
	const char *tip = guardian_tip(navi);
	if (guardian_known(navi)) {
		ADD("|@M %s waits at the end!|", guardian(navi)->name);
		if (tip) ADD("@M We've got battle data on him!|@M %s|", tip);
		else ADD("@M Watch the yellow panels! Attacks land there!|");
	} else {
		/* (what a Navi on the net said, as hearsay) */
		if (guardian_heard()) ADD("|@M Word is,%s waits at the end!|@M We've got no battle data on him,Lan.|", guardian(navi)->name);
		else ADD("|@M A strong Navi's signal waits at the end...|@M We've got no battle data on it,Lan.|");
		ADD("@M Watch the yellow panels! Attacks land there!|");
	}
	/* (EraseCross on a Navi, which the setup has no room for: a playtester
	 * saw BlastMan's HP drain after a Vulcan, and only patch notes had
	 * said why) */
	if (flag_get(BN6_FLAG_ERASE_CROSS))
		ADD("@M In EraseCross,watch his HP for a 4!|@M Then a plain chip bugs him,Lan.|@M His HP drains for the rest of the fight!|");
	#undef ADD
	return k;
}

/* What the ScrtData carried are for, appended to `buf` at `k` once per
 * count (a playtester carried two and never learned; the next heard it on
 * every layer), and where their gate stands, which only a bystander's
 * rumour had said (a playtester holding three asked where it was); the
 * new length. */
static int scrt_note(char *buf, int k, int size) {
	if (run.fragments == D.fragments_told) return k;
	D.fragments_told = run.fragments;
	if (run.fragments <= 0) return k;
	static const char *const counts[] = { "one", "two" };
	char n[12];
	snprintf(n, sizeof n, "%d", run.fragments);
	return k + snprintf(buf + k, k < size ? (size_t)(size - k) : 0, "@M We've got %s ScrtData,Lan!|@M %s|", run.fragments <= 2 ? counts[run.fragments - 1] : n,
		run.fragments < 3 ? "Three open the Secret Area's golden gate.|@M It's in the Undernet!"
		                  : "The Secret Area's golden gate will open for us!|@M It's in the Undernet,through a dark warp.");
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
			heal ? "@M Let's visit the Recovery Mr.Prog!|@M He's %s,%s%s.|@M The arrow turns green and leads there!|"
			     : "@M The Net Dealer sells MiniEnrg,Lan!|@M He's %s,%s%s.|", hw, near_far[hf], wind);
		break;
	}
	return k;
}

/* The area's viruses that fight in ways BN6 never explains, where the act
 * or a side layer begins, appended to `buf` at `k`; the new length. Only
 * once they have been battled, in any run: the first meeting is theirs to
 * show; and as a may, which the area's battles can bring, not will (a
 * playtester met none of "StarFish here again" in a whole act). (A playtester's Thunder healed a ScarCrow to full, two DarkMechs
 * took 460 HP before he knew, and a StarFish's bubbles ate three Cannons,
 * a WideSht and a Navi chip's fire while it took 350.) */
static int family_words(char *buf, int k, int size) {
	static const struct { int family; const char *words; } warn[] = {
		{ FAMILY_SCARCROW, "@M We might meet ScarCrows again!|@M Their lightning heals them. So do Elec chips!|@M Hit them hard with anything but Elec!|" },
		{ FAMILY_DARKMECH, "@M We might meet DarkMechs again!|@M They warp right beside us to slash.|@M Keep moving,and strike as they appear!|" },
		{ FAMILY_STARFISH, "@M We might meet StarFish again!|@M Their bubbles soak up our shots.|@M Touch one,and it traps us!|"
			"@M A chip that drops from above gets past them!|" },
	};
	/* (none where the area's battles are another game's: L warned of
	 * StarFish in End Area, whose battle was BN5's Whirlies, session 65) */
	if (encounter_guest) return k;
	uint32_t fams = loot_families_here(run.depth, run.biome);
	for (unsigned i = 0; i < sizeof warn / sizeof *warn; ++i)
		if (fams & (1u << warn[i].family) && profile_family_fought(warn[i].family))
			k += snprintf(buf + k, k < size ? (size_t)(size - k) : 0, "%s", warn[i].words);
	return k;
}

/* What MegaMan senses on the layer (`here`, n of them), appended to `buf`
 * at `k`, then the rival, whose call has said why (docs/RIVAL.md), apart:
 * in the list, "ProtoMan, waiting for our duel and an official gate" read
 * as waiting for the gate too (a playtester's). His race named a race, as
 * Chaud's call does: "our duel" read as a fight a playtester skipped
 * (session 62). The new length. */
static int sense_words(char *buf, int k, int size, const char *const *here, int n, bool duel) {
	#define ADD(...) (k += snprintf(buf + k, k < size ? (size_t)(size - k) : 0, __VA_ARGS__))
	/* (in boxes that keep to their three lines, as BN6's never page on: a
	 * box closes where its next name would pass them, the next "And ...") */
	for (int i = 0, len = 0, box = 0; i < n; ++i) {
		bool first = !len, last = i == n - 1 || len + (int)(strlen(here[i]) + strlen(here[i + 1])) > 36;
		ADD("%s%s%s", first ? (box ? "@M And " : "@M I sense ") : last ? " and " : ",", here[i], !last ? "" : box ? "!|" : " here!|");
		len = last ? 0 : len + (int)strlen(here[i]) + 1;
		box += last;
	}
	const char *what = layer_objs_duel_rung == 2 ? "our netbattle" : "our race against his time";
	if (duel) ADD(n ? "@M And ProtoMan's waiting for %s!|" : "@M ProtoMan's waiting for %s here!|", what);
	#undef ADD
	return k;
}

/* L's word on a program left off the board, appended to `buf` at `k`; the
 * new length. Said on every layer until placed, unless MegaMan has said it
 * on this layer already (a playtester heard it on CONTINUE, then again in
 * L's first words); one that cannot fit, once a board (a playtester's
 * SuprArmr could not share the 4x4 board with Custom1). */
static int off_board_note(char *buf, int k, int size) {
	#define ADD(...) (k += snprintf(buf + k, k < size ? (size_t)(size - k) : 0, __VA_ARGS__))
	int offv;
	const char *off = run_won_here() || D.off_told ? NULL : program_off_board(&offv);
	int k0 = k;
	if (off && !fits_beside_placed(offv)) {
		const char *w = no_room_words(*off ? off : "That program", offv);
		if (w) ADD("%s|", w);
	} else if (off && *off && off_board_explained(offv)) {
		if (fits_as_it_stands(offv)) ADD("@M Lan,%s is still off our board!|", off);
	} else if (off && *off) {
		ADD("@M Lan,%s isn't on our NaviCust board yet!|@M In the PET,go to MegaMan,then NaviCust!|@M %s|", off, navicust_turn_words(offv));
		off_board_explain(offv);
	} else if (off) ADD("@M Lan,a program isn't on our NaviCust board yet!|@M In the PET,go to MegaMan,then NaviCust!|");
	if (k > k0) D.off_told = true;
	#undef ADD
	return k;
}

/* The map's violet marks L has not explained yet, explained (once a
 * profile), appended to `buf` at `k`; the new length. And its counters,
 * the first time they show over L's words: they count what MegaMan knows,
 * so "2/2" is not read as the layer's all. */
static int mark_lessons(char *buf, int k, int size, int fresh) {
	#define ADD(...) (k += snprintf(buf + k, k < size ? (size_t)(size - k) : 0, __VA_ARGS__))
	if (!(profile.marks_taught & MARK_COUNTS) && counts_any()) {
		ADD("@M The crystals up top count our Mystery Data!|@M What we've taken,out of what we've seen or sensed.|"
			"@M There could be more out there!|");
		fresh |= MARK_COUNTS;
	}
	if (fresh & MARK_SERVER) ADD("@M A strong virus signal,marked violet on the map!|@M Its Server offers a hard battle for a good chip!|");
	if (fresh & MARK_WARP) ADD("@M A dark warp,marked violet on the map!|@M It leads to the Undernet.|@M Tougher viruses in there,but richer data!|");
	if (fresh & MARK_GATE) ADD("@M The Secret Area's golden gate,marked violet on the map!|");
	if (fresh & MARK_NAVI_GATE)
		ADD("@M A gate sealed with %s's code!|@M It's marked violet on the map.|@M His code opens it for good. His SP waits inside!|",
			guardian(D.objs.gate_navi)->name);
	if (fresh & MARK_VAULT) ADD("@M A collector's vault,marked violet on the map!|@M A big enough Library opens it. Rare chips inside!|");
	if (fresh) { profile.marks_taught |= (uint8_t)fresh; profile_save(); }
	#undef ADD
	return k;
}

/* The layer's set pieces MegaMan senses (issue #48), a bit each; not an
 * invisible path, whose navi's hint is its cue. */
enum { SENSED_PURPLE = 1, SENSED_RUSH = 2, SENSED_TELEPORT = 4, SENSED_ARROWS = 8, SENSED_OBSTACLE = 16, SENSED_CUBE = 32, SENSED_SKULL = 64,
	SENSED_NUMBER = 128 };

static int pieces_sensed(void) {
	int m = (layer.ngaps ? SENSED_RUSH : 0) | (layer.nteleports ? SENSED_TELEPORT : 0) | (layer.nlanes ? SENSED_ARROWS : 0);
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].type == OBJ_MYSTERY && layer.obj[i].param == MD_PURPLE) m |= SENSED_PURPLE;
	for (int i = 0; i < layer.nblocks; ++i) {
		int kind = layer.block[i].kind;
		m |= kind < BLOCK_KINDS ? SENSED_OBSTACLE : kind == BLOCK_SKULL ? SENSED_SKULL : kind == BLOCK_NUMBER ? SENSED_NUMBER : SENSED_CUBE;
	}
	return m;
}

/* What each is called among what MegaMan senses, once he has explained it,
 * and his words the first time a profile meets it: the lock and its key,
 * as the bone panels' were (BN6 says nothing of RushFood at its bones). */
static const struct { int bit; const char *name, *lesson; } piece_talk[] = {
	{ SENSED_PURPLE, "purple Mystery Data", "@M Purple Mystery Data,locked tight!|@M An Unlocker opens it. A Net Dealer might sell one!|" },
	{ SENSED_RUSH, "bone panels", "@M Bone panels by a gap!|@M Rush can bridge it,if we've got RushFood!|" },
	{ SENSED_TELEPORT, "teleport pads", "@M Teleport pads! Step on one,and we beam to the other!|" },
	{ SENSED_ARROWS, "arrow panels", "@M Arrow panels! They only carry us the way they point.|" },
	{ SENSED_OBSTACLE, "a Link Navi's obstacle", "@M Something's blocking a walkway!|@M A Link Navi could clear it. Or the right Cross!|" },
	{ SENSED_CUBE, "a security cube", "@M A security cube!|@M It wants a P-Code someone here knows. Or a toll!|" },
	{ SENSED_SKULL, "a skull door", "@M A skull door! Only a WWW-ID gets us past.|" },
	{ SENSED_NUMBER, "a number door", "@M A number door!|@M We'll find its answer by counting something here.|" },
};

/* The `known` pieces' names added to here[] (n of them, max at most); the
 * new count. */
static int piece_names(int known, const char **here, int n, int max) {
	for (unsigned i = 0; i < sizeof piece_talk / sizeof *piece_talk && n < max; ++i)
		if (known & piece_talk[i].bit) here[n++] = piece_talk[i].name;
	return n;
}

/* The `fresh` pieces explained, appended to `buf` at `k`, and noted as
 * taught; the new length. */
static int piece_lessons(char *buf, int k, int size, int fresh) {
	for (unsigned i = 0; i < sizeof piece_talk / sizeof *piece_talk; ++i)
		if (fresh & piece_talk[i].bit) k += snprintf(buf + k, k < size ? (size_t)(size - k) : 0, "%s", piece_talk[i].lesson);
	if (fresh) { profile.pieces_taught |= (uint8_t)fresh; profile_save(); }
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
 * security cube"), or NULL: the counters count data MegaMan senses there,
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
			: b->kind == BLOCK_NUMBER ? "behind the number door" : "behind the security cube";
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
	const char *what = b->kind < BLOCK_KINDS ? "obstacle" : b->kind == BLOCK_SKULL || b->kind == BLOCK_NUMBER ? "door" : "cube";
	char where[96];
	int wx, wy;
	netmap_world(b->x, b->y, &wx, &wy);
	return k + snprintf(buf + k, k < size ? (size_t)(size - k) : 0, "@M The %s's %s!|@M It's the violet mark on the map.|", what,
		spot_where(wx, wy, where, sizeof where));
}

/* The data MegaMan senses but has not seen, said where it lies (two at
 * most), and the locks it is behind, appended to `buf` at `k`; the new
 * length. */
static int prize_note(char *buf, int k, int size) {
	const char *where[2];
	int n = 0, lock[2] = { -1, -1 };
	for (int i = 0; i < D.objs.nmd && n < 2; ++i) {
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
	bool shop, heal, programs, trader, bugtrader, challenge, warp, gate, navi_gate, vault, duel, official, flame;
	bool any;   /* anything L's second words would name */
} Here;

/* The flame of darkness still burning on the layer (docs/META.md): L
 * names it and where, and the map marks it, an Event; a playtester walked
 * two layers of ACDC Area past one, L naming every service but it, the
 * map marking only ProtoMan (session 66). */
bool flame_here(void) { return D.objs.dark_flame_obj >= 0 && !flag_get(LAYER_DARK_TAKEN_FLAG); }

static int flame_words(char *buf, int k, int size) {
	if (!flame_here()) return k;
	static const char *const near_far[3] = { "close by", "a ways off", "far off" };
	const NetObj *o = &layer.obj[D.objs.dark_flame_obj];
	int wx, wy, far;
	bool winds;
	netmap_world((int)o->x, (int)o->y, &wx, &wy);
	const char *lies = lie_and_walk(wx, wy, &far, &winds);
	return k + snprintf(buf + k, k < size ? (size_t)(size - k) : 0, "@M The flame's %s,%s%s!|@M It's the violet mark on the map.|",
		lies, near_far[far], winds ? WINDS : "");
}

static void here_scan(Here *h) {
	memset(h, 0, sizeof *h);
	for (int i = 0; i < layer.nobj; ++i) {
		int t = layer.obj[i].type;
		h->duel |= t == OBJ_DUEL && !layer_objs_duel_later;
		h->official |= t == OBJ_OFFICIAL;
		h->shop |= t == OBJ_SHOP;
		h->heal |= t == OBJ_HEAL && !heal_spent();
		h->programs |= t == OBJ_PROGRAMS;
		h->trader |= t == OBJ_TRADER;
		h->bugtrader |= t == OBJ_BUGTRADER;
		h->challenge |= t == OBJ_CHALLENGE;
		h->warp |= t == OBJ_UNDERNET;
		h->gate |= t == OBJ_SECRET_GATE;
		h->navi_gate |= t == OBJ_NAVI_GATE;
		h->vault |= t == OBJ_VAULT;
	}
	h->flame = flame_here();
	h->any = h->duel || h->official || h->flame || h->shop || h->heal || h->programs || h->trader || h->bugtrader || h->challenge || h->warp ||
		h->gate || h->navi_gate || h->vault || pieces_sensed() || run.fragments != D.fragments_told || rival_where() ||
		run.side_kind != LAYER_NORMAL || layer_in_act(run.depth) == 0;
}

/* L's first words on a layer: where they are and what guards it,
 * appended to `buf` at `k`; the new length. */
static int first_words(char *buf, int k, int size) {
	#define ADD(...) (k += snprintf(buf + k, k < size ? (size_t)(size - k) : 0, __VA_ARGS__))
	const char *area = guardian_area_in_text(run.biome, run.side_kind);
	ADD("@M Layer %d,Lan. We're in %s!%s", run.depth, area, encounter_guest ? "|@M Battles here run the older Net's way." : "");
	if (D.objs.guardian.navi && !boss_beaten()) k = guardian_words(buf, k, size);
	/* (not after the act's arrival words, which spoke of him; a
	 * CONTINUE does not say them again, and there he is spoken of) */
	else if (!D.objs.guardian.navi && run.side_kind == LAYER_NORMAL && !D.guardian_named) {
		int navi = run_guardian(run.biome);
		if (guardian_known(navi)) ADD("|@M %s's copy guards this area!|", guardian(navi)->name);
		else if (guardian_heard()) ADD("|@M Word is,%s guards this area!|", guardian(navi)->name);
		else ADD("|@M A strong Navi's signal waits deeper in...|");
	}
	else ADD("|");
	#undef ADD
	return k;
}

/* What L names among what is here: the services, then the map's violet
 * marks once explained (`fresh` those not yet), then an official gate;
 * their number. */
static int here_names(const Here *h, const char **here, int *fresh) {
	int n = 0;
	if (h->shop) here[n++] = "a Net Dealer";
	if (h->heal) here[n++] = "a Recovery Mr.Prog";
	if (h->programs) here[n++] = "a NaviCust program shop";
	if (h->trader) here[n++] = "a Chip Trader";
	if (h->bugtrader) here[n++] = "a BugFrag Trader";
	int marks = (h->challenge ? MARK_SERVER : 0) | (h->warp ? MARK_WARP : 0) | (h->gate ? MARK_GATE : 0) | (h->navi_gate ? MARK_NAVI_GATE : 0) |
		(h->vault ? MARK_VAULT : 0);
	int known = marks & profile.marks_taught;
	*fresh = marks & ~profile.marks_taught;
	if (known & MARK_SERVER) here[n++] = "a strong virus signal";
	if (known & MARK_WARP) here[n++] = "a dark warp";
	if (known & MARK_GATE) here[n++] = "the golden gate";
	static char sealed[48];
	if (known & MARK_NAVI_GATE) { snprintf(sealed, sizeof sealed, "a gate with %s's code", guardian(D.objs.gate_navi)->name); here[n++] = sealed; }
	if (known & MARK_VAULT) here[n++] = "a collector's vault";
	if (h->flame && n < 8) here[n++] = "a flame of darkness";
	if (h->official && n < 8) here[n++] = "an official gate";
	return n;
}

/* L's second words on a layer: the area's battlefields and viruses, the
 * ScrtData carried, then what is here in one breath (a playtester paged
 * eight boxes on arriving in act 3): the services (the map marks a trader
 * as a shop, and L had said nothing of one), the map's violet marks, in
 * full until explained (a playtester stood beside one and never found out
 * what it was), named after that, the set pieces, the rival, a program off
 * the board and the map's tip; appended to `buf` at `k`, the new length. */
static int more_words(char *buf, int k, int size, const Here *h) {
	#define ADD(...) (k += snprintf(buf + k, k < size ? (size_t)(size - k) : 0, __VA_ARGS__))
	/* (the area's battlefields, on its first layer: a playtester froze
	 * on the Aquarium's ice, 140 to 80 HP, and nothing had said so) */
	if (run.biome == BIOME_AQUARIUM_COMP && run.side_kind == LAYER_NORMAL && layer_in_act(run.depth) == 0)
		ADD("@M Brr... The battlefields here are icy,Lan.|@M An Aqua hit on ice freezes us!|@M Keep off the ice when viruses shoot water.|");
	/* (and a homepage's conveyors and ice: a conveyor carried a
	 * playtester off the row he stepped into, every time, and the ice
	 * froze him twice in the next act) */
	if (run.biome == BIOME_HOMEPAGE && run.side_kind == LAYER_NORMAL && layer_in_act(run.depth) == 0)
		ADD("@M The battlefields here have conveyors and ice!|@M The arrows carry us along.|@M And an Aqua hit on ice freezes us. Careful!|");
	if (run.side_kind != LAYER_NORMAL || layer_in_act(run.depth) == 0) k = family_words(buf, k, size);
	k = scrt_note(buf, k, size);
	const char *here[16];
	int fresh, n = here_names(h, here, &fresh);
	int pieces = pieces_sensed();
	n = piece_names(pieces & profile.pieces_taught, here, n, 16);
	k = sense_words(buf, k, size, here, n, h->duel);
	k = flame_words(buf, k, size);
	k = mark_lessons(buf, k, size, fresh);
	k = piece_lessons(buf, k, size, pieces & ~profile.pieces_taught);
	k = prize_note(buf, k, size);
	/* (where the rival waits, and his mark: the map showed him as the
	 * official gate's violet, and a playtester's session ran out at
	 * the gate, alone, looking for him) */
	const char *rw = rival_where();
	if (rw) ADD("@M ProtoMan's %s!|@M He's the pink mark on the map.|", rw);
	k = off_board_note(buf, k, size);
	/* (the map's tip on the run's first layers, until the map has been
	 * held: a playtester who used it heard it again every run) */
	if (run.depth <= 2 && !map_was_held()) ADD("@M Hold SELECT for a map of where we've been!|");
	#undef ADD
	return k;
}

/* What MegaMan says when L is pressed: where they are, what is ahead. */
const char *status_words(void) {
	static char buf[1400];
	int k = 0;
	#define ADD(...) (k += snprintf(buf + k, k < (int)sizeof buf ? sizeof buf - (size_t)k : 0, __VA_ARGS__))
	if (D.town) {
		int far;
		const char *way = town_way(&far);
		/* all of it the first time, then only the way (a box each) */
		if (!D.port_told) ADD("@M The port's %s,by the %s!|@M Stand next to it,press R,and jack me in!", way, town_info()->landmark);
		else ADD("@M The port's %s,Lan!", way);
		D.port_told = true;
		return buf;
	}
	/* where they are and what guards it, then the heal while hurt and the
	 * way on; what else the layer holds at the next L (a playtester paged
	 * twelve and thirteen boxes on arriving, and asked for the guardian,
	 * the heal and the way first, session 63); then only the way on */
	D.layer_told |= flag_get(LAYER_TOLD_FLAG);
	bool told = D.layer_told, more = told && !D.more_told;
	if (!told) k = first_words(buf, k, (int)sizeof buf);
	Here h;
	here_scan(&h);
	if (more) {
		k = more_words(buf, k, (int)sizeof buf, &h);
		D.more_told = true;
	}
	if (!told) {
		D.layer_told = true;
		flag_set(LAYER_TOLD_FLAG);
	}
	bool hurt = hurt_now();
	if (hurt && (h.heal || h.shop)) k = heal_note(buf, k, (int)sizeof buf, h.heal);
	/* (and after that, where ProtoMan waits, while he does) */
	const char *rival = told && !more ? rival_where() : NULL;
	if (rival) ADD("@M ProtoMan's waiting %s!|", rival);
	k = goal_words(buf, k, (int)sizeof buf, hurt && h.heal, told);
	if (!told && h.any) ADD("|@M There's more here,Lan. Press L again!");
	#undef ADD
	goal_way();   /* (the arrow's way, which the words that follow start) */
	return buf;
}

/* The last stop before a guardian's arena: the Net Dealer and a heal stand
 * in the room before it or one beside it (net_gen.c), off the arrow's
 * line, and a playtester walked past both to SpoutMan at 120 of 140 HP
 * with 1150 zenny unspent. Stepping into the room before the arena,
 * MegaMan names those not yet used, once, and which way each is. */
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

#define NO_RUNNING "Once we're in,there's no running!"
void last_stop(int cx, int cy) {
	if (D.last_stop_told || layer.ante < 0 || !D.objs.guardian.navi || boss_beaten() || boss_fighting()) return;
	const Room *a = &layer.rooms[layer.ante];
	if (cx < a->x || cy < a->y || cx >= a->x + a->w || cy >= a->y + a->h) return;
	if (emu_read8(BN6_CHATBOX) || talk_busy() || cinema_busy() || D.warping || D.map_shown) return;
	D.last_stop_told = true;
	int hp = emu_read16(BN6_NAVI_HP), max = emu_read16(BN6_NAVI_MAX_HP);
	const char *dealer = NULL, *heal = NULL;
	static char dway[80], hway[80];
	for (int i = 0; i < layer.nobj; ++i) {
		const NetObj *o = &layer.obj[i];
		int wx, wy;
		netmap_world((int)o->x, (int)o->y, &wx, &wy);
		if (o->type == OBJ_SHOP && !dealer && !flag_get(LAYER_DEALER_TOLD_FLAG)) dealer = service_where(wx, wy, dway, sizeof dway);
		if (o->type == OBJ_HEAL && !heal && !flag_get(LAYER_HEAL_TOLD_FLAG) && hp < max) heal = service_where(wx, wy, hway, sizeof hway);
	}
	if (!dealer && !heal) return;
	static char buf[400];
	int k = guardian_known(D.objs.guardian.navi) || guardian_heard()
		? snprintf(buf, sizeof buf, "@M %s's arena is just ahead,Lan!|@M ", guardian(D.objs.guardian.navi)->name)
		: snprintf(buf, sizeof buf, "@M The guardian's arena is just ahead,Lan!|@M ");
	/* (and no running once in, as from BN6's story bosses: a playtester
	 * at 1 HP tried L and R, which only BN6's random battles answer) */
	if (dealer && heal) snprintf(buf + k, sizeof buf - (size_t)k, "Want to get ready first?|@M The Net Dealer's %s.|@M A Recovery Mr.Prog's %s.|@M %s", dealer, heal, NO_RUNNING);
	else if (dealer) snprintf(buf + k, sizeof buf - (size_t)k, "Want to get ready first?|@M The Net Dealer's %s.|@M %s", dealer, NO_RUNNING);
	else snprintf(buf + k, sizeof buf - (size_t)k, "Want to heal up first?|@M A Recovery Mr.Prog's %s.|@M %s", heal, NO_RUNNING);
	talk_start(buf, FACE_MEGAMAN);
}
