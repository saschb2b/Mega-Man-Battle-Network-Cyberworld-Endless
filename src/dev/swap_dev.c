/* The swap step (swap_dev.h). */
#include "swap_dev.h"

#include <stdio.h>
#include <string.h>

#include "battle_swap.h"
#include "director.h"

bool swap_dev_step(const char *spec) {
	SwapForm forms[SWAP_FORMS] = { { 0, 0 } };
	int n = 0;
	char buf[64];
	snprintf(buf, sizeof buf, "%s", spec);
	for (char *t = strtok(buf, " /,"); t; t = strtok(NULL, " /,")) {
		unsigned id = 0;
		int hp = 0;
		if (n >= SWAP_FORMS || sscanf(t, "%x:%d", &id, &hp) < 1) {
			printf("swap: %s is no form (ID:HP, the id in hex, at most %d forms)\n", t, SWAP_FORMS);
			return false;
		}
		forms[n++] = (SwapForm){ (int)id, hp };
	}
	/* (the last plan's end said, and its hooks off before the next) */
	SwapState was = battle_swap_state();
	if (was.forms) printf("swap: the last plan: form %d of %d, %d gone, the loop's phase %02x as it ended\n", was.form + 1, was.forms, was.gone, was.result);
	battle_swap_disarm();
	if (n < 2 || !battle_swap_arm(forms, n)) {
		printf("swap: %s cannot be fought (2 to %d of BN6's Navis)\n", spec, SWAP_FORMS);
		return false;
	}
	char foes[16];
	snprintf(foes, sizeof foes, "%03X@52", (unsigned)forms[0].id);
	printf("swap: %d forms armed, the battle of %s due\n", n, foes);
	return director_dev_foes(foes);
}
