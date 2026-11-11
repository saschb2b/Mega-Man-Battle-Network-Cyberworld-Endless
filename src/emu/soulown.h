/* Each Soul's own in BN6's engine (issue #82, the Souls track's S3-S5;
 * docs/multirom/souls.md): its code moved (soulcode.h), its look
 * (soullook.h), Chaos Unison's (soulchaos.h), its abilities (soulpower.h,
 * BN4's Souls' among them: soulbn4.h), passives and chips (soulpassive.h,
 * soulchips.h), ColonelSoul's button (soulcolonel.h) and BN5's mood
 * (soulmood.h), installed in that order once UNITE stands
 * (soulunite_install). */
#ifndef CW_SOULOWN_H
#define CW_SOULOWN_H

#include <stdbool.h>

/* Each part installed, all or none of each, with the spec's breaks
 * (soulunite.h's SOULBREAK_*): false where one is not placed whole (the
 * Souls then unite as the proxy) */
bool soulown_install(int soulbreak);

#endif
