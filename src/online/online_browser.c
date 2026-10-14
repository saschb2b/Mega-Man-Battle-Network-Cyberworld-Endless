#include "online_browser.h"

#include <string.h>

#include "protocol.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/websocket.h>

static struct {
	EMSCRIPTEN_WEBSOCKET_T socket;
	OnlineBrowserMessage received;
	OnlineBrowserClosed closed;
	uint8_t hello[52 + ONLINE_NAME_BYTES];
	size_t hello_size;
} browser;

bool online_browser_send(const uint8_t *data, size_t size) {
	uint8_t copy[ONLINE_MAX_MESSAGE];
	size_t buffered = 0;
	if (size > sizeof copy || !browser.socket ||
	    emscripten_websocket_get_buffered_amount(browser.socket, &buffered) != EMSCRIPTEN_RESULT_SUCCESS || buffered > 8192) return false;
	memcpy(copy, data, size);
	return emscripten_websocket_send_binary(browser.socket, copy, (uint32_t)size) == EMSCRIPTEN_RESULT_SUCCESS;
}

static bool opened(int type, const EmscriptenWebSocketOpenEvent *event, void *user) {
	(void)type; (void)user;
	if (event->socket != browser.socket) return true;
	char protocol[64];
	if (emscripten_websocket_get_protocol(browser.socket, protocol, sizeof protocol) != EMSCRIPTEN_RESULT_SUCCESS ||
	    strcmp(protocol, ONLINE_SUBPROTOCOL) || !online_browser_send(browser.hello, browser.hello_size)) browser.closed();
	return true;
}

static bool received(int type, const EmscriptenWebSocketMessageEvent *event, void *user) {
	(void)type; (void)user;
	if (event->socket != browser.socket) return true;
	if (event->isText || !event->numBytes || event->numBytes > ONLINE_MAX_MESSAGE) browser.closed();
	else browser.received(event->data, event->numBytes);
	return true;
}

static bool closed(int type, const EmscriptenWebSocketCloseEvent *event, void *user) {
	(void)type; (void)user;
	if (event->socket == browser.socket) browser.closed();
	return true;
}

static bool failed(int type, const EmscriptenWebSocketErrorEvent *event, void *user) {
	(void)type; (void)user;
	if (event->socket == browser.socket) browser.closed();
	return true;
}

bool online_browser_open(const char *url, const uint8_t *hello, size_t size,
                         OnlineBrowserMessage on_message, OnlineBrowserClosed on_close) {
	if (!emscripten_websocket_is_supported() || size > sizeof browser.hello) return false;
	if (strncmp(url, "ws://", 5) && strncmp(url, "wss://", 6)) return false;
	EmscriptenWebSocketCreateAttributes attributes = { .url = url, .protocols = ONLINE_SUBPROTOCOL, .createOnMainThread = true };
	browser.socket = emscripten_websocket_new(&attributes);
	if (browser.socket <= 0) { browser.socket = 0; return false; }
	memcpy(browser.hello, hello, size);
	browser.hello_size = size;
	browser.received = on_message;
	browser.closed = on_close;
	emscripten_websocket_set_onopen_callback(browser.socket, NULL, opened);
	emscripten_websocket_set_onmessage_callback(browser.socket, NULL, received);
	emscripten_websocket_set_onerror_callback(browser.socket, NULL, failed);
	emscripten_websocket_set_onclose_callback(browser.socket, NULL, closed);
	return true;
}

void online_browser_close(void) {
	if (browser.socket > 0) {
		EMSCRIPTEN_WEBSOCKET_T socket = browser.socket;
		browser.socket = 0;
		emscripten_websocket_close(socket, 1000, "Leaving square");
		emscripten_websocket_delete(socket);
	}
	memset(&browser, 0, sizeof browser);
}

#endif
