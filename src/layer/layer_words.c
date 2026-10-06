/* What the layer's navis say of what stands on it, and MegaMan at what he
 * finds there (docs/VOICE.md): the Net Dealer, the NaviCust vendor,
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
 * so none answers him for long. */
static void dealer_word(char *word, size_t n, const DealerTalk *d, const char *brought, const char *lands) {
	int navi = d->navi, counter = d->counter, nstock = d->nstock;
	const ShopItem *stock = d->stock;
	bool weak = counter > 0 && guardian_weakness(navi) > 0, listed = false;
	for (int i = 0; i < nstock; ++i)
		if (stock[i].kind == 2 && counter > 0 && chip_hits_with(stock[i].id) == counter && (i == 0 || !weak)) listed = true;
	if (weak && listed) {
		snprintf(word, n, "|Word is,%s can't stand %s chips!|My pick's first on the list.|%s%s", guardian(navi)->name,
			elem_name(counter), brought, lands);
		return;
	}
	/* (a weakness he found no chip of, as a few layers' rolls of Cursor
	 * chips came to: the hardest hit, shop_dealer_stock) */
	if (weak) {
		snprintf(word, n, "|Word is,%s can't stand %s chips!|But I couldn't find any. Sorry!|Hit hard,then! My pick's "
			"first on the list.|%s", guardian(navi)->name, elem_name(counter), brought);
		return;
	}
	/* (KnightMan's armor turns every blow but while he swings or leaps: a
	 * playtester learned it over three Custom screens, session 68) */
	bool knight = guardian_older(navi) && guardian_older_ai(navi) == BN5_NAVI_KNIGHTMAN;
	int k = snprintf(word, n, "|Word is,%s %s.|Hit hard%s! My pick's first on the list.|%s", guardian(navi)->name,
		navi == GUARDIAN_ELEMENTMAN ? "changes his element as he fights" :
		knight ? "has no weak element.|His armor blocks all,but not when he swings or leaps" : "has no weak element",
		knight ? ",then" : "", brought);
	if (listed && k > 0 && (size_t)k < n)
		snprintf(word + k, n - (size_t)k, "|The viruses here hate %s chips,though!|I've got one of those,too!",
			elem_name(counter));
}

/* (Rush's gap, and how many RushFood call him there: he comes for as many
 * as its panels and eats one, which only MegaMan said, at its bones; a
 * playtester would have bought one of three, session 64) */
static void dealer_rush(char *s, size_t n) {
	int hold = layer.ngaps ? layer.gap[0].len : shop_rush_need(run.depth);
	const char *where = layer.ngaps ? "on this layer" : "deeper in";
	/* (why as many as its panels, said: "He comes when you hold 2 RushFood.
	 * He eats one." read as a riddle, session 70) */
	if (hold > 1)
		snprintf(s, n, "|And there's a gap %s,%d panels wide!|Rush comes when you hold a RushFood|for each panel. He eats just one!|"
			"It's on my list,too!", where, hold);
	else snprintf(s, n, "|And there's a gap %s!|Rush bridges it for one RushFood.|It's on my list,too!", where);
}

/* (the keys he stocks, said: what each opens and where, issues #41, #14, #47) */
static void dealer_keys(char *hello, size_t n, const DealerTalk *d) {
	const ShopItem *stock = d->stock;
	int nstock = d->nstock;
	for (int i = 0; i < nstock; ++i) {
		size_t k = strlen(hello);
		if (stock[i].kind != 1 || k >= n) continue;
		if (stock[i].id == SUB_UNLOCKER)
			snprintf(hello + k, n - k, "|Word is,there's purple data locked %s!|An Unlocker opens it. It's on my list!",
				d->purple ? "on this layer" : "deeper in");
		else if (stock[i].id == ITEM_RUSH_FOOD)
			dealer_rush(hello + k, n - k);
		else if (stock[i].id == ITEM_WWW_ID)
			snprintf(hello + k, n - k, "|Skull doors %s only let WWW members by.|A WWW-ID opens every one!",
				d->skull ? "on this layer" : "deeper in the Undernet");
	}
}

/* What a Spin does, and that it stays: the first also how they are
 * found, one deeper in each dive. */
int spin_words(TextArchive *text, int colour, bool first) {
	static char words[400];
	const char *c = meta_spin_name(colour);
	int held = 0;
	for (int k = 0; k < 6; ++k) held += meta_spins() >> k & 1;
	if (first)
		snprintf(words, sizeof words, "Lan,look! A Spin for %s programs!|Now we can turn %s programs on the NaviCust!|"
			"Hold one,then press L or R.|And it stays with us,in every dive!|Each dive hides one more,deeper in.|"
			"A color we don't have yet!", c, c);
	else if (held >= 5)
		snprintf(words, sizeof words, "A Spin for %s programs,Lan!|That's all six! Every program turns now!|"
			"In every dive!", c);
	else
		snprintf(words, sizeof words, "A Spin for %s programs,Lan!|%c%s programs turn with L and R now!|"
			"In every dive from here on!", c, c[0] - 'a' + 'A', c + 1);
	return ta_say(text, FACE_MEGAMAN, words);
}

/* The Net Dealer's greeting and its word on the act's guardian and the
 * keys he stocks; his line when he is visited again, and sold out */
ShopWords dealer_words(const DealerTalk *d) {
	static char hello[720];
	char word[280] = "";
	int navi = d->navi, nstock = d->nstock;
	const ShopItem *stock = d->stock;
	const char *brought = nstock && stock[0].stock == 1 ? "It's my only one. Make it count!" : "I brought two. They go fast!";
	/* (how his pick lands, where it is not straight ahead: AquaNdl2 missed
	 * a hopping BlastMan two times in three) */
	const char *lands = nstock && stock[0].kind == 2 && chip_family(stock[0].id) == 50
		? "|Its needles drop a moment late.|So fire when he stops!" : "";
	/* (nor does he deny the bystanders' rumor: a playtester heard it
	 * two platforms before his "No word yet") */
	if (navi > 0 && !d->tells)
		snprintf(word, sizeof word, "|Nobody's come back from the end of %s!|There's talk on the Net... "
			"But I don't sell talk!|Ask me again deeper in!", guardian_area_in_text(run.biome, LAYER_NORMAL));
	else if (navi > 0)
		dealer_word(word, sizeof word, d, brought, lands);
	/* (and that his chips come in the folder's codes, loot_fit_code) */
	snprintf(hello, sizeof hello, "%s%s", run.depth <= 3
		? run.codes[0] ? "Welcome! Chips for sale!|In your Folder's codes,when I can!"
			: "Welcome! Chips for sale!"
		: run.side_kind == LAYER_NORMAL && run.mode == RUN_SHORT && run_short_last(run.depth)
			/* (the run's last layer has no "from here": a playtester heard it there) */
			? "The bottom of the Net,MegaMan!|Stock up! My last stop,and yours!"
			: "Still at it,MegaMan?|Stock up! It only gets tougher from here!", word);
	/* (met in this act already: the pick, in a line) */
	if (d->again && d->tells)
		snprintf(hello, sizeof hello, "Back again,MegaMan!|My pick for %s is first on the list!|%s", guardian(navi)->name, brought);
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
	snprintf(hello, sizeof hello, "NaviCust programs!|Fresh from my workbench!%s|Install them in the PET.|Pick MegaMan,then NaviCust!",
		again);
	return (ShopWords){ hello, "More programs? Take a look!", "Sold out!|Every program I brought is yours!" };
}

/* ProtoMan's terms for a race (docs/RIVAL.md), into `terms`: the time to
 * beat (`frames`), as the results screen shows a DeleteTime (seconds and
 * hundredths, cut), and the rung's rule; the squad (`foes` viruses) and the
 * stake said before the choice (a playtester took it at 100 HP, nothing
 * saying it was a real fight; and the two are old rivals, not strangers);
 * the first duel (`met` none before) says who he is */
void duel_terms(char *terms, size_t n, int met, int foes, int frames, int rung) {
	static const char *const count[] = { "", "a lone virus", "a pair of viruses", "three viruses", "four viruses" };
	int f = frames, sec = f / 60, cs = (f % 60) * 100 / 60;
	int nf = foes >= 1 && foes <= 4 ? foes : 3;
	snprintf(terms, n, "%sI busted %s here in %d:%02d.%02d.|Beat that%s.|"
		"@M Real viruses,Lan!|@M If they delete us,the dive's over.|@M Let's heal up first if we're hurt.",
		met ? "Back again,MegaMan? Chaud's watching.|" :
		"MegaMan... So it's you,diving the Endless Net.|The Nest copies Navis,they say.|I'm no copy.|Chaud wants to see your skill.|",
		count[nf], sec / 60, sec % 60, cs, rung == 1 ? " without taking a hit" : "");
}

/* The net's word on the act's guardian `navi`, a bystander's: who he is
 * and a rumor */
const char *rumor_words(int navi) {
	static char rumor[200];
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
