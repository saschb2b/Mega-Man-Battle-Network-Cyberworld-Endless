#include "online_trade_text.h"

const char trade_panel_title[] = "Trade with";
const char trade_panel_give[] = "YOU GIVE  1";
const char trade_panel_receive[] = "YOU GET  1";
const char trade_panel_code[] = "CODE";
const char trade_panel_preview[] = "UI PREVIEW - TRADING DISABLED";
const char *const trade_panel_status[TRADE_PANEL_PHASES][2] = {
	{ "Swap these spare chips?", "Your equipped folders stay the same." },
	{ "Preparing your swap...", "Your run is on hold." },
	{ "Waiting for the other Navi...", "Your run is on hold." },
	{ "Saving your swapped Pack...", "Your run is on hold." },
	{ "Trade pending. Your run is on hold.", "Reconnect to finish this trade." },
	{ "Your swap is saved.", "The new chip is in your Pack." },
	{ "Trade cancelled.", "Your Pack is unchanged." },
	{ "Could not prepare this trade.", "Your Pack is unchanged." },
};
const char *const trade_panel_help[TRADE_PANEL_PHASES] = {
	"A: Confirm   B: Cancel", "Please wait...", "B: Ask to cancel", "Please wait...",
	"A: Retry   B: Leave Square", "A/B: Close", "A/B: Close", "A/B: Close",
};
