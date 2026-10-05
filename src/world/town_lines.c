/* The towns' people and what is said there (docs/VOICE.md): each town's
 * folk, where they stand and what they say, its checks' words and
 * MegaMan's on arriving, and Lan's and MegaMan's as a run begins. Its plan
 * and map, town.c. */
#include "town_lines.h"

#include <stdio.h>

#include "save.h"
#include "town.h"

/* Its people: at the statue, the shop, the school gate, the bus stop, the
 * houses, the closed road to the Expo. Some have lines of the run's at
 * home (town_words.c); at each visit they stand elsewhere (town_folk.c). */
static const Folk central_folk[] = {
	{ 108, -68, FACE_SW, 5, 0x36, "Did you hear?|The Net under town copies every battle!" },
	{ 100, -52, 0, 7, 0x11, "*wag,wag* Woof! Woof!!" },   /* (the robot dog: one animation) */
	/* (the plaza's Mr.Prog calls the Net's news: town_words.c) */
	{ 60, -96, FACE_SE, 7, 0x0F, "NET NEWS! NET NEWS!|A NEW NET OPENED UNDER TOWN!" },
	{ 44, -20, FACE_NE, 5, 0x2E, "I jacked in yesterday. Today the paths were all new!|Wow... It really does go on forever!" },
	{ -172, -4, FACE_NW, 5, 0x2C, "Ooh! AsterLand got new chips in!|I could look at 'em all day..." },
	{ -108, -36, FACE_NE, 5, 0x2B, "My dad parks here every Sunday!|AsterLand's the best!" },
	{ 130, 150, FACE_NE, 5, 0x2F, "The Academy kids swear a GigaChip's under the LevBus!|One's waited a week for it to drive off!" },
	{ 84, -180, FACE_SE, 5, 0x34, "Hey,Lan! No class today!|You diving into the Endless Net too?" },
	{ -164, 196, FACE_NE, 5, 0x38, "Heading out,Lan?|Be careful on the Net,OK?" },
	{ 18, 290, FACE_SW, 5, 0x39, "My,my... Aren't the flowers lovely here?" },
	{ -146, -184, FACE_SE, 5, 0x3A, "Oh dear... The road to the Expo Site is closed.|And I so wanted to see the pavilions!" },
	{ 132, 180, FACE_SE, 5, 0x2D, "Phew! Long day at the lab...|Nothing beats a nice walk!", 10 },
	{ -12, -164, FACE_NE, 5, 0x31, "Ack! I'm late for the Academy's NetBattle club!|Everyone's hunting for Program Advances!|"
		"Three chips in the right order make a new one!", 10 },
};

/* What its checks (triggers 0xF0 + n, in front of each thing) say: the
 * houses (Lan's, the pink one, the orange one, the gray pair), the flower
 * bed, the bus stop, Aster Land's door, the Expo gates' signs and road, the
 * Academy's gate, the statue, Aster Land's window. */
static const char *const central_checks[16] = {
	"@L Home sweet home! Mom's making curry tonight!|@M Yum! Let's be back for dinner,Lan!",
	"A pink house. The curtains are drawn.",
	"Someone is watering the plants on the roof terrace.",
	"Two gray houses,side by side. It's quiet in there.",
	"The flowers are in full bloom.",
	"The LevBus stop.|\"Next bus: ACDC Town\"|No GigaChip under here. Just a gum wrapper.",
	"@M AsterLand! We'll shop later,Lan. The Net's waiting!",
	"EXPO\nThe sign lists the pavilions on show.",
	"Cyber Academy. The gate is closed for the day.",
	"A statue of a blue bird.|It looks ready to fly off.",
	"The road to the Expo Site. It's closed off today.",
	"EXPO\nA map of the site. It's huge!",
	"Chips and PETs line the shelves in the window.",
	NULL, NULL, NULL,
};

/* Its people: kids in the park, the chip shop, the Metroline, the
 * mansion, the houses, the promenade. */
static const Folk acdc_folk[] = {
	{ -196, -20, FACE_SE, 5, 0x2B, "Meet you at the squirrel!|Last one there's a Mettaur!" },
	{ -180, -4, FACE_NW, 5, 0x34, "Lan! All the way from Central Town?|The squirrel's port goes to the Endless Net too!" },
	{ -204, -4, FACE_NE, 5, 0x37, "Squirrel! Squirrel!" },
	{ -180, -44, FACE_SE, 7, 0x0F, "WELCOME! I'M THE PARK'S PORT GUIDE!|PRESS R BY THE SQUIRREL TO JACK IN!" },
	{ -188, -92, FACE_NE, 5, 0x2D, "Higsby's got rare chips... but those prices!|They say his rarest never leave the back room!" },
	{ 4, -132, FACE_SW, 5, 0x30, "The Metroline goes right to Central Town. So handy!" },
	{ 252, -28, FACE_SW, 5, 0x2E, "That's the Ayanokoji mansion!|They say there's a whole garden inside!" },
	{ 100, 36, FACE_SE, 5, 0x36, "Nobody lives in that house anymore...|But somebody still waters the flowers." },
	{ 60, 164, FACE_SW, 5, 0x39, "Mr.Famous says he busted a virus pack with no chips!|Nobody's seen him do it,though! Hahaha!" },
	{ 124, -84, FACE_SW, 5, 0x38, "The boy here NetBattles day and night...|So noisy! Geez..." },
	{ -60, 164, FACE_NE, 5, 0x2C, "Every morning I walk the promenade...|Then I dive a few layers!", 12 },
};

static const char *const acdc_checks[16] = {
	"@L Our old house... Feels like only yesterday.|@M Lan,we had so many adventures here...",
	"The hedge is neatly trimmed.",
	"Mayl's house. Piano music drifts out the window.|@M Lan,Mayl's practicing again!",
	"The squirrel statue! Its port leads into the Endless Net.",
	"Higsby's chip shop.|\"Rare chips in stock!\"",
	"A blue house. The mailbox says \"Oyama.\"|@L I bet Dex is NetBattling again...",
	"A tall wall runs around the Ayanokoji mansion.",
	"The Ayanokoji mansion. The gate is shut tight.",
	"A Chip Trader. It's out of order today.",
	NULL, NULL, NULL, NULL, NULL, NULL, NULL,
};

/* Its people, on the plaza's height (they stand at 0): by the fountain,
 * the fish shop, the way down to the aquarium. */
static const Folk seaside_folk[] = {
	{ -60, -44, FACE_SW, 5, 0x31, "The mermaid fountain has a port,you know.|Press R beside it to jack in!" },
	{ -44, -108, FACE_SW, 5, 0x36, "They say the mermaid's port goes to a new Net!|Ahh,the sea air! I wanna dive in!" },
	{ -92, -156, FACE_NW, 5, 0x2C, "Fish sticks,fresh from the sea!|I buy a dozen every Sunday!" },
	{ -132, -150, FACE_NW, 5, 0x2E, "The Aquarium's Net copied itself overnight!|Grandpa says it's Dr.Wily...|He says that about everything!" },
	{ -140, -60, FACE_NE, 5, 0x39, "I come here to watch the boats...|And that whale never gets old!" },
	{ -60, -20, FACE_SE, 5, 0x30, "Lan! You took the LevBus to Seaside?|Good luck down there!" },
	{ -108, -44, FACE_NE, 5, 0x2F, "The mermaid looks out over the sea...|Lovely,isn't she?", 10 },
};

/* What its checks say: the fountain (0), the fish shop (2, 3); the others
 * say nothing (1 and 7 round the plaza's east corner, 4 to 6 up on the
 * station's walkway). */
static const char *const seaside_checks[16] = {
	"A mermaid over the fountain,gazing out to sea.|Her port leads into the Endless Net.",
	NULL,
	"The fish shop.|\"FISH STICKS! Fresh every morning!\"",
	"Fish of every color swim in the shop's window.",
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
};

/* Its people, on the ground at 0: by the knight statue, up the paths to
 * the flower shop and the Judge Tree, round the pond. */
static const Folk green_folk[] = {
	{ -60, -156, FACE_SE, 5, 0x31, "The knight statue has a port,you know.|Press R beside it to jack in!" },
	{ -20, -268, FACE_SW, 5, 0x36, "The flower shop's roses are in full bloom!|Take a peek before you dive in!" },
	{ -196, -172, FACE_SE, 5, 0x2C, "My friend says the right buttons shrink NaviCust programs!|But he won't tell me which ones! Hmph!" },
	{ -180, -236, FACE_NW, 5, 0x2E, "The JudgeTree was here long before the town.|They say its roots reach all the way into the Net!" },
	{ 68, -124, FACE_SW, 5, 0x30, "Lan! You took the LevBus to Green Town?|Good luck down there!" },
	{ -60, -204, FACE_SE, 5, 0x39, "I jacked in at the knight yesterday...|Today the paths were all new! It really is endless!" },
	{ -132, -108, FACE_NE, 5, 0x32, "Ahh... Green Town's air is so clean!|Even the Net feels fresher here!", 10 },
};

/* What its checks say: the stump's table of books (0, 4), the flower shop
 * (1) and its flower boxes (5), the stumps' stools (3, 6), the lily ponds
 * (7); 8, by the plaza's east arm, has nothing to see, and the knight's
 * own (2) lies under his pedestal, which keeps Lan from reaching it. */
static const char *const green_checks[16] = {
	"A table of books on a tree stump.|Someone left them out to read in the sun.",
	"The flower shop.|\"FRESH FLOWERS! Picked this morning!\"",
	NULL,
	"Stumps cut smooth for stools.|The whole town sits on its trees.",
	"A table of books on a tree stump.|Someone left them out to read in the sun.",
	"Flower boxes in rows. The whole plaza smells sweet.",
	"Stools round a stump table. A nice spot for lunch.",
	"Lilies float on the pond. A frog watches from a leaf.",
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
};

/* What Lan and MegaMan say as a run begins: on the first dive ever, Dad's
 * call about the Endless Net; after that, a word about the last one. */
const char *town_intro(const char *arrival) {
	static char buf[900];
	int k = 0;
	#define ADD(...) (k += snprintf(buf + k, k < (int)sizeof buf ? sizeof buf - (size_t)k : 0, __VA_ARGS__))
	if (arrival) ADD("%s", arrival);
	/* (the net's name is the Endless Net, but a short run goes to its Nest:
	 * "The Endless Net again" read odd to a playtester who chose Short) */
	const char *again = run.mode == RUN_SHORT ? "@L Down to the Nest again... I wonder what's new?"
		: "@L The Endless Net again... I wonder what's new?";
	if (!profile.seen_intro) {
		ADD("@D Lan,it's Dad. Got a minute?|"
			"@D A new Net just opened up under town.|"
			"@D Its paths change every time someone jacks in.|"
			"@D And it only goes down. They call it the Endless Net.|"
			"@M The Endless Net... Lan,that sounds like an adventure!|"
			"@D It's all copied data down there.|"
			"@D Nothing you find comes back out with you.|"
			"@D Something at the very bottom is copying it all.|"
			"@D We call it the Nest.|"
			"@D If MegaMan's deleted,my backup brings him home.|"
			"@D So dive as deep as you can,and send me your readings!|"
			"@L Leave it to us,Dad!|"
			"@M Let's jack in from your PC,Lan!");
	} else if (profile.nest_clears > 0 && profile.runs % 2) {
		/* (after a win, the ending's hook: "reached" undersold it to a
		 * playtester who had won) */
		if (profile.short_wins > 0)
			ADD("@D You two brought the Nest down,Lan.|@D But something below it is still awake...|"
				"@D The Net's changed again. Be careful!|@L Got it,Dad!");
		else
			ADD("@D Lan,you two reached the Nest before.|@D But it's all changed again. Be careful!|@L Got it,Dad!");
	} else if (town_after_abandon) {
		ADD("@L We never finished that last dive...|@M Then let's start a fresh one!|@M To the PC,Lan!");
	} else if (profile.runs == 0) {
		/* (the call heard, but no run over yet: no best to speak of) */
		ADD("%s|@M Let's find out,Lan!|@M To the PC!", again);
	} else {
		/* (the short net ends on its Nest: its goal, not a depth to beat) */
		bool nest_goal = run.mode == RUN_SHORT && profile.best_depth >= SHORT_LAYERS - 1;
		switch (profile.runs % 3) {
		case 0:
			if (nest_goal) ADD("@M Ready,Lan? The Nest is waiting!|@L This time we'll bring it down!");
			else ADD("@M Ready,Lan? Our best is layer %d!|@L This time we'll go even deeper!", profile.best_depth);
			break;
		case 1:
			if (nest_goal) ADD("@M Dad's backup got me home safe last time.|@L Alright! Let's reach the Nest today!");
			else ADD("@M Dad's backup got me home safe last time.|@L Alright! Let's beat layer %d today!", profile.best_depth);
			break;
		default: ADD("%s|@M Let's find out,Lan!|@M To the PC!", again); break;
		}
	}
	#undef ADD
	return buf;
}

#define FOLK(list) list, (int)(sizeof list / sizeof *list)

const TownLines town_lines[TOWN_LINES] = {
	{ FOLK(central_folk), central_checks, NULL },
	{ FOLK(acdc_folk), acdc_checks, "@M ACDC Town,Lan! The Metroline's so fast!|" },
	{ FOLK(seaside_folk), seaside_checks, "@M Seaside Town,Lan! Smell that sea air!|" },
	{ FOLK(green_folk), green_checks, "@M Green Town,Lan! Smell those flowers!|" },
};

_Static_assert(sizeof central_folk / sizeof *central_folk <= MAX_FOLK && sizeof acdc_folk / sizeof *acdc_folk <= MAX_FOLK &&
	sizeof seaside_folk / sizeof *seaside_folk <= MAX_FOLK && sizeof green_folk / sizeof *green_folk <= MAX_FOLK, "at most MAX_FOLK townsfolk");
