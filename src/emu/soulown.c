/* Each Soul's own in BN6's engine (soulown.h). */
#include "soulown.h"

#include "soulchaos.h"
#include "soulchips.h"
#include "soulcode.h"
#include "soulcolonel.h"
#include "soullook.h"
#include "soulmood.h"
#include "soulpassive.h"
#include "soulpower.h"
#include "soulunite.h"

bool soulown_install(int soulbreak) {
	return soulcode_install(&soulcode_bn5) && soullook_install(SOULBREAK_LOOK(soulbreak)) &&
		soulchaos_install(SOULBREAK_POWER(soulbreak)) && soulpower_install(SOULBREAK_POWER(soulbreak)) &&
		soulpassive_install(SOULBREAK_PASSIVE(soulbreak)) && soulchips_install(SOULBREAK_CHIPS(soulbreak)) &&
		soulcolonel_install(SOULBREAK_PASSIVE(soulbreak)) && soulmood_install(SOULBREAK_MOOD(soulbreak));
}
