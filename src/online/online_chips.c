/* online_chips.h. */
#include "online_chips.h"

#include "bn6.h"
#include "chips.h"
#include "protocol.h"

bool online_chip_known(unsigned id) {
	if (id < 1 || id > ONLINE_CHIP_LAST || !chips_record((int)id)) return false;
	bool bn6 = id <= ONLINE_BN6_CHIP_LAST && !BN6_CHIP_BLANK(id);
	return bn6 || chips_bn5((int)id) || chips_bn4((int)id);
}

bool online_chip_plays(unsigned id) { return online_chip_known(id) && !chips_sits_out((int)id); }
