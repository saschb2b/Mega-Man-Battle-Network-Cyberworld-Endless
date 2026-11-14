/* start_net.h. */
#include "start_net.h"

#include "analytics_net.h"
#include "devtools.h"
#include "network_prefs.h"
#include "transfer.h"

void start_net(bool headless, bool plain) {
	analytics_net_start(headless, plain);
	transfer_link_set(online_transfer_link(dev.online ? dev.online : NETWORK_DEFAULT_URL));
}
