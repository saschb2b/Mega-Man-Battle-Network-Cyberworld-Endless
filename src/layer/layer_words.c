/* What the layer's navis say of what stands on it, and MegaMan at what he
 * finds there (docs/voice/rules.md): the Net Dealer, the NaviCust vendor,
 * ProtoMan's terms, a bystander's rumor and an invisible path's hint, a
 * ScrtData and a Spin. Their scripts are layer_objs.c's. */
#include "layer_words.h"

#include <stdio.h>
#include <string.h>

#include "data.h"
#include "guardians.h"
#include "meta.h"
#include "netmap.h"
#include "pacing.h"
#include "save.h"
#include "super_lines.h"
#include "xguardian.h"

/* ProtoMan's words where his netbattle waits for a later act: where, as
 * the net goes ("the third act" was the game's word, not his). */
const char *netbattle_later_words(void) {
	return pacing_act(run.depth) == 0
		? "Enough racing.|Our next duel is a NetBattle. You and me.|Not here. I'll wait past the next two guardians."
		: "Enough racing.|Our next duel is a NetBattle. You and me.|Not here. I'll wait past the next guardian.";
}

/* The Net Dealer's word on the act's guardian, `navi`, by his list
 * (`stock`, n of them): the element he can't stand (of either wheel:
 * TenguMan's Sword, issue #39) and the pick of it, first on the list; for
 * one weak to none, the hardest hit, and the viruses' weakness besides
 * where the list has a chip of it (an element-less guardian's act is
 * answered for its viruses). ElementMan's element changes as he fights,
 * so none answers him for long. One tip, a seller's, in one box (his
 * list's first row is the pick; how many he brought, the list's): how a
 * chip lands and a Navi's armor are strategy, the BBS's (the voice sweep,
 * docs/voice/sweep/net.md). A box for the viruses' weakness besides. */
static void dealer_word(char *word, size_t n, const DealerTalk *d) {
	int navi = d->navi, counter = d->counter, nstock = d->nstock;
	const ShopItem *stock = d->stock;
	const char *name = guardian(navi)->name;
	bool weak = counter > 0 && guardian_weakness(navi) > 0, listed = false;
	for (int i = 0; i < nstock; ++i)
		if (stock[i].kind == 2 && counter > 0 && chip_hits_with(stock[i].id) == counter && (i == 0 || !weak)) listed = true;
	if (weak && listed) {
		snprintf(word, n, "Word is,%s can't stand %s! My pick's first!", name, elem_name(counter));
		return;
	}
	/* (a weakness he found no chip of, as a few layers' rolls of Cursor
	 * chips came to: the hardest hit first, shop_dealer_stock) */
	if (weak) {
		snprintf(word, n, "Word is,%s can't stand %s! But I've got none!", name, elem_name(counter));
		return;
	}
	int k = navi == GUARDIAN_ELEMENTMAN ? snprintf(word, n, "Word is,%s keeps changing his element! Hit hard!", name)
		: snprintf(word, n, "Word is,%s has no weak element. My pick hits hard!", name);
	if (listed && k > 0 && (size_t)k < n)
		snprintf(word + k, n - (size_t)k, "|The viruses here hate %s chips! I've got one!", elem_name(counter));
}

/* (the keys he stocks for a lock on this layer, said: what each opens,
 * issues #41, #14, #47; and Rush's gap, how wide: as many RushFood as its
 * panels call him, BN6's own rule, unsaid; a playtester would have bought
 * one of three, session 64, and the width says how many. A key for deeper
 * in goes unsaid: the lock says its own key at A, and R the key's use,
 * session 76's seven boxes) */
static void dealer_keys(char *hello, size_t n, const DealerTalk *d) {
	const ShopItem *stock = d->stock;
	int nstock = d->nstock;
	for (int i = 0; i < nstock; ++i) {
		size_t k = strlen(hello);
		if (stock[i].kind != 1 || k >= n) continue;
		if (stock[i].id == SUB_UNLOCKER && d->purple)
			snprintf(hello + k, n - k, "|Got an Unlocker for the purple data here!");
		else if (stock[i].id == ITEM_RUSH_FOOD && layer.ngaps && layer.gap[0].len > 1)
			snprintf(hello + k, n - k, "|A gap here,%d panels wide! RushFood's on my list!", layer.gap[0].len);
		else if (stock[i].id == ITEM_RUSH_FOOD && layer.ngaps)
			snprintf(hello + k, n - k, "|A gap here! Rush bridges it for a RushFood!");
		else if (stock[i].id == ITEM_WWW_ID && d->skull)
			snprintf(hello + k, n - k, "|A WWW-ID opens the skull doors here!");
	}
}

/* A Spin found, and that it stays: the first also how they are found, one
 * deeper in each dive. What a Spin does, and L and R, are BN6's own,
 * unsaid (the voice sweep, docs/voice/sweep/net.md). */
int spin_words(TextArchive *text, int colour, bool first) {
	static char words[400];
	const char *c = meta_spin_name(colour);
	int held = 0;
	for (int k = 0; k < 6; ++k) held += meta_spins() >> k & 1;
	if (first)
		snprintf(words, sizeof words, "Lan,look! A Spin for %s programs!|And it stays with us,in every dive!|"
			"Each dive hides one more,deeper in.|A color we don't have yet!", c);
	else if (held >= 5) snprintf(words, sizeof words, "A Spin for %s programs,Lan!|That's all six!|In every dive!", c);
	else snprintf(words, sizeof words, "A Spin for %s programs,Lan!|In every dive from here on!", c);
	return ta_say(text, FACE_MEGAMAN, words);
}

/* The Net Dealer's words before his list: one box (his greeting, or his
 * word on the act's guardian), on the run's first act his codes besides,
 * and a box for each key he stocks for a lock on this layer; his line when
 * he is visited again, and sold out. (A box each made seven and eight
 * before his list, sessions 73 and 76: "Ask me again deeper in", the count
 * he brought, the keys for deeper in and a greeting his list's own box
 * says went, docs/voice/rules.md's "one short line about what's
 * special".) */
ShopWords dealer_words(const DealerTalk *d) {
	static char hello[400];
	char word[260] = "";
	int navi = d->navi;
	/* (nor does he deny the bystanders' rumor: a playtester heard it
	 * two platforms before his "No word yet"; with no word yet, his
	 * greeting alone) */
	/* (a super boss never met: no word on the net, only its fear; docs/
	 * docs/game/bosses/super-bosses.md) */
	if (super_boss(navi) && !d->tells)
		snprintf(word, sizeof word, "Something's waiting at the end of %s...|Even I don't go near it!",
			guardian_area_in_text(run.biome, run.side_kind));
	else if (navi > 0 && d->tells)
		dealer_word(word, sizeof word, d);
	/* (that his chips come in the folder's codes, loot_fit_code, and past
	 * act 1 the finds' too, their chips as they roll: loot.c's lean_find,
	 * the rule said once where the dealer half says it,
	 * docs/game/core-loop.md) */
	const char *lead = run.depth <= 3 && run.codes[0] && !(d->again && d->tells) ? "Chips in your codes! Mystery Data's too,deeper in!" : NULL;
	/* (the run's last layer: the last stop, a fact, said beside his word;
	 * it has no "from here", which a playtester heard there) */
	bool last = run.depth > 3 && run.side_kind == LAYER_NORMAL && run.mode == RUN_SHORT && run_short_last(run.depth);
	if (last) lead = "The bottom of the Net,MegaMan! My last stop,and yours!";
	if (!lead && !*word) lead = run.depth <= 3 ? "Need chips? Take a look!" : "Still at it,MegaMan? Stock up!";
	snprintf(hello, sizeof hello, "%s%s%s", lead ? lead : "", lead && *word ? "|" : "", word);
	/* (met in this act already: the pick, in a line) */
	if (d->again && d->tells && !last)
		snprintf(hello, sizeof hello, "Back again! My pick for %s is first!", guardian(navi)->name);
	dealer_keys(hello, sizeof hello, d);
	return (ShopWords){ hello, "Back for more? Take a look!", "Sold out,MegaMan!|You bought every chip I had!" };
}

/* The NaviCust vendor's greeting, naming the programs MegaMan has used in
 * earlier runs that lead his list (`names`, n of them) */
ShopWords vendor_words(char names[][16], int n) {
	static char hello[300];
	char again[140] = "";
	if (n == 1) snprintf(again, sizeof again, "|I hear you've used %s before!|So I brought it along!", names[0]);
	else if (n == 2)
		snprintf(again, sizeof again, "|I hear you've used %s and %s!|So I brought them along!", names[0], names[1]);
	snprintf(hello, sizeof hello, "NaviCust programs,for BugFrags!|Fresh from my workbench!%s", again);
	return (ShopWords){ hello, "More programs? Take a look!", "Sold out!|Everything I brought is yours!" };
}

/* ProtoMan's terms for a race (docs/game/rival.md), into `terms`: the time to
 * beat (`frames`), as the results screen shows a DeleteTime (seconds and
 * hundredths, cut), and the rung's rule; the squad (`foes` viruses) and the
 * stake said before the choice (a playtester took it at 100 HP, nothing
 * saying it was a real fight; and the two are old rivals, not strangers);
 * the first duel (`met` none before) says who he is. To heal first goes
 * unsaid, and the stake is noted (the voice sweep,
 * docs/voice/sweep/net.md). */
void duel_terms(char *terms, size_t n, int met, int foes, int frames, int rung) {
	static const char *const count[] = { "", "a lone virus", "a pair of viruses", "three viruses", "four viruses" };
	int f = frames, sec = f / 60, cs = (f % 60) * 100 / 60;
	int nf = foes >= 1 && foes <= 4 ? foes : 3;
	snprintf(terms, n, "%sI busted %s here in %d:%02d.%02d.|Beat that%s.|@M Real viruses,Lan!|@M If they delete us,the dive's over.",
		met ? "Back again,MegaMan? Chaud's watching.|" :
		"MegaMan... So it's you,diving the Endless Net.|The Nest copies Navis,they say.|I'm no copy.|Chaud wants to see your skill.|",
		count[nf], sec / 60, sec % 60, cs, rung == 1 ? " without taking a hit" : "");
}

/* The net's word on the act's guardian `navi`, a bystander's: who he is
 * and a rumor */
const char *rumor_words(int navi) {
	static char rumor[200];
	/* (a super boss's: what the net feels of him, never his name) */
	if (super_boss(navi)) return super_rumor(navi);
	snprintf(rumor, sizeof rumor, "Did you hear?|A copy of %s guards the end of %s!|Word is,%s", guardian(navi)->name,
		guardian_area_in_text(run.biome, LAYER_NORMAL), guardian_rumor(navi));
	return rumor;
}

/* A bystander's hint at an invisible path (issue #46) */
const char *hinter_words(void) {
	return "See that lonely pad out in the void?|I saw a Navi walk right out to it!|Over nothing at all!";
}

/* What a ScrtData is for, said as the run's `held`th is picked up (a
 * playtester was told a layer later, then on every layer after), and where
 * its gate stands, which a playtester holding three asked */
const char *fragment_words(int held) {
	static const char *const found[3] = {
		"Lan,look! A ScrtData!|Three open the golden gate to the Secret Area.|It stands in the Undernet's copy.|A dark warp leads there.|Let's find two more!",
		"Our second ScrtData!|One more,and the Secret Area's gate opens!|It's in the Undernet's copy!",
		"That's three ScrtData,Lan!|Now the Secret Area's golden gate opens!|It's in the Undernet's copy.|The next dark warp leads there!",
	};
	return found[held < 3 ? held : 2];
}
