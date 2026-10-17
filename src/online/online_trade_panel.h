/* Both exact chip cards stay visible through confirmation and recovery. */
#ifndef CW_ONLINE_TRADE_PANEL_H
#define CW_ONLINE_TRADE_PANEL_H

#include <stdint.h>

typedef enum {
	TRADE_PANEL_REVIEW, TRADE_PANEL_PREPARING, TRADE_PANEL_WAITING,
	TRADE_PANEL_INSTALLING, TRADE_PANEL_RECOVERING, TRADE_PANEL_COMPLETE,
	TRADE_PANEL_CANCELLED, TRADE_PANEL_FAILED, TRADE_PANEL_PHASES
} OnlineTradePanelPhase;

typedef struct {
	uint16_t give, receive;
	char peer[17];
	OnlineTradePanelPhase phase;
} OnlineTradePanel;

/* Read-only presentation of validated, owned data. This function neither
 * accepts an offer nor changes a transaction or emulator state. */
void online_trade_panel_draw(const OnlineTradePanel *panel);

#endif
