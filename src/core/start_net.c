/* start_net.h. */
#include "start_net.h"

#include "analytics_net.h"
#include "cloud.h"
#include "devtools.h"
#include "discord.h"
#include "discord_game.h"
#include "game.h"
#include "network_prefs.h"
#include "saves_cloud.h"
#include "transfer.h"

void start_net(bool headless, bool plain) {
	analytics_net_start(headless, plain);
	discord_start(plain && !headless);
	frame_watch = discord_game_frame;
	quit_watch = discord_game_quit;
	const char *url = dev.online ? dev.online : NETWORK_DEFAULT_URL;
	transfer_link_set(online_transfer_link(url));
	cloud_link_set(online_cloud_link(url));
	saves_cloud_start();
}
