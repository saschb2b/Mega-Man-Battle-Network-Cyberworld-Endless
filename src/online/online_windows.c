#include "online_socket.h"

#ifdef _WIN32
#include <windows.h>
#include <winhttp.h>
#include <stdio.h>
#include <string.h>

#include "protocol.h"

/* WinHTTP's asynchronous WebSocket API leaves both directions outstanding.
 * Its callbacks copy only bounded metadata; the SDL worker decodes messages. */
static struct {
	void *library;
	__typeof__(&WinHttpOpen) open;
	__typeof__(&WinHttpSetTimeouts) timeouts;
	__typeof__(&WinHttpSetOption) option;
	__typeof__(&WinHttpSetStatusCallback) callback;
	__typeof__(&WinHttpCrackUrl) crack;
	__typeof__(&WinHttpConnect) connect;
	__typeof__(&WinHttpOpenRequest) request;
	__typeof__(&WinHttpSendRequest) send;
	__typeof__(&WinHttpReceiveResponse) response;
	__typeof__(&WinHttpQueryHeaders) query;
	__typeof__(&WinHttpCloseHandle) close;
	__typeof__(&WinHttpWebSocketCompleteUpgrade) upgrade;
	__typeof__(&WinHttpWebSocketSend) ws_send;
	__typeof__(&WinHttpWebSocketReceive) ws_receive;
	__typeof__(&WinHttpWebSocketClose) ws_close;         /* (optional: the closing handshake before the handles go) */
} http;

static struct {
	HINTERNET session, connection, request, socket;
	SDL_sem *event;
	SDL_atomic_t *stop;
	SDL_atomic_t failed, request_sent, headers, read_ready, write_done, handles, closed;
	uint8_t received[ONLINE_MAX_MESSAGE], sending[ONLINE_MAX_MESSAGE];
	DWORD received_size;
	WINHTTP_WEB_SOCKET_BUFFER_TYPE received_type;
} win;

static bool load_http(void) {
	if (http.library) return true;
	http.library = SDL_LoadObject("winhttp.dll");
	if (!http.library) return false;
	http.open = (__typeof__(http.open))SDL_LoadFunction(http.library, "WinHttpOpen");
	http.timeouts = (__typeof__(http.timeouts))SDL_LoadFunction(http.library, "WinHttpSetTimeouts");
	http.option = (__typeof__(http.option))SDL_LoadFunction(http.library, "WinHttpSetOption");
	http.callback = (__typeof__(http.callback))SDL_LoadFunction(http.library, "WinHttpSetStatusCallback");
	http.crack = (__typeof__(http.crack))SDL_LoadFunction(http.library, "WinHttpCrackUrl");
	http.connect = (__typeof__(http.connect))SDL_LoadFunction(http.library, "WinHttpConnect");
	http.request = (__typeof__(http.request))SDL_LoadFunction(http.library, "WinHttpOpenRequest");
	http.send = (__typeof__(http.send))SDL_LoadFunction(http.library, "WinHttpSendRequest");
	http.response = (__typeof__(http.response))SDL_LoadFunction(http.library, "WinHttpReceiveResponse");
	http.query = (__typeof__(http.query))SDL_LoadFunction(http.library, "WinHttpQueryHeaders");
	http.close = (__typeof__(http.close))SDL_LoadFunction(http.library, "WinHttpCloseHandle");
	http.upgrade = (__typeof__(http.upgrade))SDL_LoadFunction(http.library, "WinHttpWebSocketCompleteUpgrade");
	http.ws_send = (__typeof__(http.ws_send))SDL_LoadFunction(http.library, "WinHttpWebSocketSend");
	http.ws_receive = (__typeof__(http.ws_receive))SDL_LoadFunction(http.library, "WinHttpWebSocketReceive");
	http.ws_close = (__typeof__(http.ws_close))SDL_LoadFunction(http.library, "WinHttpWebSocketClose");
	if (http.open && http.timeouts && http.option && http.callback && http.crack && http.connect && http.request &&
	    http.send && http.response && http.query && http.close && http.upgrade && http.ws_send && http.ws_receive) return true;
	SDL_UnloadObject(http.library);
	memset(&http, 0, sizeof http);
	return false;
}

static void CALLBACK completed(HINTERNET handle, DWORD_PTR context, DWORD status, LPVOID detail, DWORD size) {
	(void)handle; (void)context;
	switch (status) {
	case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE: SDL_AtomicSet(&win.request_sent, 1); break;
	case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE: SDL_AtomicSet(&win.headers, 1); break;
	case WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE: SDL_AtomicSet(&win.write_done, 1); break;
	case WINHTTP_CALLBACK_STATUS_CLOSE_COMPLETE: SDL_AtomicSet(&win.closed, 1); break;
	case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
		if (size == sizeof(WINHTTP_WEB_SOCKET_STATUS)) {
			const WINHTTP_WEB_SOCKET_STATUS *read = detail;
			win.received_size = read->dwBytesTransferred;
			win.received_type = read->eBufferType;
			SDL_AtomicSet(&win.read_ready, 1);
		} else SDL_AtomicSet(&win.failed, 1);
		break;
	case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
	case WINHTTP_CALLBACK_STATUS_SECURE_FAILURE: SDL_AtomicSet(&win.failed, 1); break;
	default: break;
	}
	if (win.event) SDL_SemPost(win.event);
	/* HANDLE_CLOSING is the final callback for each handle. No semaphore or
	 * shared state is touched after the last decrement. */
	if (status == WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING) SDL_AtomicAdd(&win.handles, -1);
}

static bool wait_for(SDL_atomic_t *flag, uint32_t milliseconds) {
	uint32_t started = SDL_GetTicks();
	while (!SDL_AtomicGet(flag)) {
		if (SDL_AtomicGet(&win.failed) || SDL_AtomicGet(win.stop) || SDL_GetTicks() - started >= milliseconds) return false;
		SDL_SemWaitTimeout(win.event, 10);
	}
	return !SDL_AtomicGet(&win.failed);
}

static HINTERNET counted(HINTERNET handle) {
	if (handle) SDL_AtomicAdd(&win.handles, 1);
	return handle;
}

static bool request_url(const char *url, wchar_t *host, wchar_t *path, INTERNET_PORT *port, bool *secure) {
	*secure = !strncmp(url, "wss://", 6);
	if (!*secure && strncmp(url, "ws://", 5)) return false;
	char converted[512];
	int length = snprintf(converted, sizeof converted, "%s%s", *secure ? "https://" : "http://", url + (*secure ? 6 : 5));
	if (length < 0 || (size_t)length >= sizeof converted) return false;
	wchar_t address[512];
	if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, converted, -1, address, 512)) return false;
	URL_COMPONENTS parts = { .dwStructSize = sizeof parts, .lpszHostName = host, .dwHostNameLength = 256,
	                        .lpszUrlPath = path, .dwUrlPathLength = 128, .dwUserNameLength = (DWORD)-1,
	                        .dwPasswordLength = (DWORD)-1, .dwExtraInfoLength = (DWORD)-1 };
	if (!http.crack(address, 0, 0, &parts) || parts.dwUserNameLength || parts.dwPasswordLength || parts.dwExtraInfoLength ||
	    !parts.dwHostNameLength || wcscmp(path, L"/presence")) return false;
	*port = parts.nPort;
	return true;
}

static bool start_read(void) {
	SDL_AtomicSet(&win.read_ready, 0);
	DWORD result = http.ws_receive(win.socket, win.received, sizeof win.received, NULL, NULL);
	if (result == NO_ERROR || result == ERROR_IO_PENDING) return true;
	SDL_AtomicSet(&win.failed, 1);
	return false;
}

static bool upgrade_request(void) {
	wchar_t headers[160], protocol[96], actual[96];
	char selected[96];
	if (!MultiByteToWideChar(CP_UTF8, 0, ONLINE_SUBPROTOCOL, -1, protocol, 96)) return false;
	if (swprintf(headers, 160, L"Sec-WebSocket-Protocol: %ls\r\n", protocol) < 0) return false;
	if (!http.option(win.request, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, NULL, 0)) return false;
	DWORD redirect = WINHTTP_OPTION_REDIRECT_POLICY_NEVER;
	if (!http.option(win.request, WINHTTP_OPTION_REDIRECT_POLICY, &redirect, sizeof redirect)) return false;
	BOOL sent = http.send(win.request, headers, (DWORD)-1, WINHTTP_NO_REQUEST_DATA, 0, 0, (DWORD_PTR)&win);
	if ((!sent && GetLastError() != ERROR_IO_PENDING) || !wait_for(&win.request_sent, 7000)) return false;
	BOOL response = http.response(win.request, NULL);
	if ((!response && GetLastError() != ERROR_IO_PENDING) || !wait_for(&win.headers, 7000)) return false;
	DWORD status = 0, size = sizeof status;
	if (!http.query(win.request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
	                WINHTTP_HEADER_NAME_BY_INDEX, &status, &size, WINHTTP_NO_HEADER_INDEX) || status != 101) return false;
	size = sizeof actual;
	if (!http.query(win.request, WINHTTP_QUERY_CUSTOM, L"Sec-WebSocket-Protocol", actual, &size, WINHTTP_NO_HEADER_INDEX) ||
	    !WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, actual, -1, selected, sizeof selected, NULL, NULL) ||
	    strcmp(selected, ONLINE_SUBPROTOCOL)) return false;
	win.socket = counted(http.upgrade(win.request, (DWORD_PTR)&win));
	if (!win.socket) return false;
	http.close(win.request);
	win.request = NULL;
	return start_read();
}

bool online_socket_open(const char *url, SDL_atomic_t *stop, char *error, size_t size) {
	if (SDL_AtomicGet(&win.handles)) {
		SDL_strlcpy(error, "Previous Windows connection is still closing", size);
		return false;
	}
	if (win.event) SDL_DestroySemaphore(win.event);
	memset(&win, 0, sizeof win);
	win.stop = stop;
	if (!load_http() || !(win.event = SDL_CreateSemaphore(0))) {
		SDL_strlcpy(error, "Online needs Windows 8 or newer with WinHTTP", size);
		return false;
	}
	bool secure;
	INTERNET_PORT port;
	wchar_t host[256], path[128];
	if (!request_url(url, host, path, &port, &secure)) {
		SDL_strlcpy(error, "Use ws(s)://host[:port]/presence", size);
		return false;
	}
	win.session = http.open(L"Cyberworld Endless", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
	                        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, WINHTTP_FLAG_ASYNC);
	if (win.session) {
		if (http.callback(win.session, completed, WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS, 0) == WINHTTP_INVALID_STATUS_CALLBACK) {
			http.close(win.session); win.session = NULL;
		} else counted(win.session);
	}
	if (win.session && http.timeouts(win.session, 5000, 5000, 2000, 7000))
		win.connection = counted(http.connect(win.session, host, port, 0));
	if (win.connection) win.request = counted(http.request(win.connection, L"GET", path, NULL,
	                                  WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, secure ? WINHTTP_FLAG_SECURE : 0));
	if (win.request && upgrade_request()) return true;
	SDL_strlcpy(error, "Windows WebSocket connection or TLS verification failed", size);
	return false;
}

bool online_socket_send(const uint8_t *data, size_t size) {
	if (!win.socket || !size || size > sizeof win.sending || SDL_AtomicGet(&win.failed)) return false;
	memcpy(win.sending, data, size);
	SDL_AtomicSet(&win.write_done, 0);
	DWORD result = http.ws_send(win.socket, WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE, win.sending, (DWORD)size);
	return (result == NO_ERROR || result == ERROR_IO_PENDING) && wait_for(&win.write_done, 2000);
}

int online_socket_receive(uint8_t *data, size_t size) {
	if (SDL_AtomicGet(&win.failed)) return -1;
	if (!SDL_AtomicGet(&win.read_ready)) return 0;
	if (win.received_type == WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE) return -1;
	/* (a text message, an empty one, or one past the wire's bound, which
	 * WinHTTP hands over as a fragment: the server broke the protocol, as
	 * online_socket.c says of the same frames, and is never tried again) */
	if (win.received_type != WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE || !win.received_size || win.received_size > size) return -2;
	int count = (int)win.received_size;
	memcpy(data, win.received, (size_t)count);
	start_read();
	return count;
}

void online_socket_close(void) {
	if (!http.close) return;
	/* (closing the handles alone resets the connection, and Windows drops
	 * what the server has not read yet, a goodbye just written among it:
	 * the closing handshake first, which waits for the server's own close
	 * frame, so everything before it was read; a short wait, as the game
	 * is leaving) */
	if (win.socket && http.ws_close && !SDL_AtomicGet(&win.failed)) {
		SDL_AtomicSet(&win.closed, 0);
		DWORD result = http.ws_close(win.socket, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, NULL, 0);
		uint32_t started = SDL_GetTicks();
		while ((result == NO_ERROR || result == ERROR_IO_PENDING) && !SDL_AtomicGet(&win.closed) &&
		       !SDL_AtomicGet(&win.failed) && SDL_GetTicks() - started < 1000)
			SDL_SemWaitTimeout(win.event, 10);
	}
	if (win.socket) http.close(win.socket);
	if (win.request) http.close(win.request);
	if (win.connection) http.close(win.connection);
	if (win.session) http.close(win.session);
	win.socket = win.request = win.connection = win.session = NULL;
	uint32_t started = SDL_GetTicks();
	while (SDL_AtomicGet(&win.handles) && SDL_GetTicks() - started < 2000) SDL_SemWaitTimeout(win.event, 10);
	if (!SDL_AtomicGet(&win.handles)) {
		if (win.event) SDL_DestroySemaphore(win.event);
		memset(&win, 0, sizeof win);
	}
}
#endif
