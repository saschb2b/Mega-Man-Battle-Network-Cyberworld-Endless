/* start_net.h. */
#include "start_net.h"

#include "analytics_net.h"
#include "cloud.h"
#include "devtools.h"
#include "discord.h"
#include "discord_game.h"
#include "dive_looks.h"
#include "dive_people.h"
#include "game.h"
#include "network_prefs.h"
#include "saves_cloud.h"
#include "transfer.h"

void start_net(bool headless, bool plain) {
	analytics_net_start(headless, plain);
	discord_start((plain && !headless) || dev.discord);
	frame_watch = discord_game_frame;
	quit_watch = discord_game_quit;
	const char *url = dev.online ? dev.online : NETWORK_DEFAULT_URL;
	transfer_link_set(online_transfer_link(url));
	cloud_link_set(online_cloud_link(url));
	saves_cloud_start();
	/* (a dive's layers with little room for the divers: dive_people.h) */
	if (dev.dive_room > 0) dive_people_dev_room(dev.dive_room);
	if (dev.dive_looks > 0) dive_looks_dev_room(dev.dive_looks);
}
