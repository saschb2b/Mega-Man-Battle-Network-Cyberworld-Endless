#include "online_socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "online_stream.h"
#include "protocol.h"
#include "negotiation.h"
#include "rom.h"

#if !defined(_WIN32) && !defined(CW_IOS)
static uint8_t incoming[ONLINE_MAX_MESSAGE * 2 + 16];
static size_t buffered;

static void base64(const uint8_t *bytes, size_t size, char *out) {
	static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	for (size_t i = 0; i < size; i += 3) {
		uint32_t word = (uint32_t)bytes[i] << 16;
		if (i + 1 < size) word |= (uint32_t)bytes[i + 1] << 8;
		if (i + 2 < size) word |= bytes[i + 2];
		*out++ = alphabet[word >> 18];
		*out++ = alphabet[(word >> 12) & 63];
		*out++ = i + 1 < size ? alphabet[(word >> 6) & 63] : '=';
		*out++ = i + 2 < size ? alphabet[word & 63] : '=';
	}
	*out = '\0';
}

static bool url_host(const char *url, char *host, size_t size) {
	const char *start = NULL;
	if (!strncmp(url, "ws://", 5)) start = url + 5;
	if (!strncmp(url, "wss://", 6)) start = url + 6;
	if (!start) return false;
	const char *path = strchr(start, '/');
	if (!path || strcmp(path, "/presence") || path == start || (size_t)(path - start) >= size) return false;
	for (const char *p = start; p < path; ++p)
		if ((unsigned char)*p <= 32 || (unsigned char)*p >= 127 || *p == '@' || *p == '\\' || *p == '#') return false;
	memcpy(host, start, (size_t)(path - start));
	host[path - start] = '\0';
	return strlen(url) < 500;
}

static bool handshake_request(const char *host, char *expected, const OnlineNegotiation *selection) {
	uint8_t random[16], digest[20];
	char key[25], combined[80], hex[41], request[768];
	if (!online_stream_random(random, sizeof random)) return false;
	base64(random, sizeof random, key);
	snprintf(combined, sizeof combined, "%s258EAFA5-E914-47DA-95CA-C5AB0DC85B11", key);
	sha1_hex((const uint8_t *)combined, strlen(combined), hex);
	for (size_t i = 0; i < sizeof digest; ++i) {
		char pair[3] = { hex[i * 2], hex[i * 2 + 1], 0 };
		digest[i] = (uint8_t)strtoul(pair, NULL, 16);
	}
	base64(digest, sizeof digest, expected);
	int length = snprintf(request, sizeof request,
	                      "GET /presence HTTP/1.1\r\nHost: %s\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n"
	                      "Sec-WebSocket-Key: %s\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Protocol: %s\r\n\r\n",
	                      host, key, online_negotiation_offer(selection));
	return length > 0 && (size_t)length < sizeof request && online_stream_send((const uint8_t *)request, (size_t)length);
}

static bool header_value(const char *response, const char *name, const char *value) {
	size_t key_size = strlen(name), value_size = strlen(value);
	for (const char *line = strstr(response, "\r\n"); line; line = strstr(line, "\r\n")) {
		line += 2;
		if (SDL_strncasecmp(line, name, key_size) || line[key_size] != ':') continue;
		const char *field = line + key_size + 1;
		while (*field == ' ' || *field == '\t') ++field;
		return !strncmp(field, value, value_size) && !strncmp(field + value_size, "\r\n", 2);
	}
	return false;
}

static bool handshake_response(const char *expected, SDL_atomic_t *stop, OnlineNegotiation *selection) {
	char response[4096];
	size_t used = 0;
	uint32_t started = SDL_GetTicks();
	while (!SDL_AtomicGet(stop) && SDL_GetTicks() - started < 5000 && used < sizeof response - 1) {
		int got = online_stream_receive((uint8_t *)response + used, sizeof response - used - 1);
		if (got < 0) return false;
		used += (size_t)got;
		response[used] = '\0';
		if (strstr(response, "\r\n\r\n"))
			return !strncmp(response, "HTTP/1.1 101 ", 13) &&
			       header_value(response, "Sec-WebSocket-Accept", expected) &&
			       online_negotiation_accept(selection,
			           header_value(response, "Sec-WebSocket-Protocol", ONLINE_SUBPROTOCOL) ? ONLINE_SUBPROTOCOL :
			           header_value(response, "Sec-WebSocket-Protocol", ONLINE_TRADE_SUBPROTOCOL) ? ONLINE_TRADE_SUBPROTOCOL : NULL) &&
			       header_value(response, "Upgrade", "websocket") && header_value(response, "Connection", "Upgrade");
		SDL_Delay(2);
	}
	return false;
}

bool online_socket_open(const char *url, SDL_atomic_t *stop, OnlineNegotiation *selection, char *error, size_t size) {
	char host[256], expected[29];
	buffered = 0;
	if (!url_host(url, host, sizeof host)) {
		SDL_strlcpy(error, "Use ws(s)://host[:port]/presence", size);
		return false;
	}
	if (!online_stream_open(url, stop, error, size)) return false;
	if (!handshake_request(host, expected, selection) || !handshake_response(expected, stop, selection)) {
		SDL_strlcpy(error, "WebSocket handshake failed", size);
		return false;
	}
	return true;
}

static bool frame_send(const uint8_t *data, size_t size, uint8_t opcode) {
	uint8_t frame[ONLINE_MAX_MESSAGE + 8], mask[4];
	if (size > ONLINE_MAX_MESSAGE || !online_stream_random(mask, sizeof mask)) return false;
	frame[0] = (uint8_t)(0x80 | opcode);
	frame[1] = (uint8_t)(0x80 | (size < 126 ? size : 126));
	size_t offset = size < 126 ? 2 : 4;
	if (offset == 4) { frame[2] = (uint8_t)(size >> 8); frame[3] = (uint8_t)size; }
	memcpy(frame + offset, mask, sizeof mask);
	offset += sizeof mask;
	for (size_t i = 0; i < size; ++i) frame[offset + i] = data[i] ^ mask[i % 4];
	return online_stream_send(frame, offset + size);
}

bool online_socket_send(const uint8_t *data, size_t size) { return frame_send(data, size, 2); }

static int take_frame(uint8_t *data, size_t size) {
	if (buffered < 2) return 0;
	uint8_t opcode = incoming[0] & 15;
	size_t length = incoming[1] & 127, offset = 2;
	if (!(incoming[0] & 0x80) || (incoming[0] & 0x70) || (incoming[1] & 0x80) || length == 127) return -1;
	if (opcode != 2 && opcode != 8 && opcode != 9 && opcode != 10) return -1;
	if (opcode >= 8 && length > 125) return -1;
	if (length == 126) {
		if (buffered < 4) return 0;
		length = (size_t)incoming[2] * 256 + incoming[3];
		offset = 4;
		if (length < 126) return -1;
	}
	if (length > size || length > ONLINE_MAX_MESSAGE) return -1;
	if (buffered < offset + length) return 0;
	if (opcode == 8 || (opcode == 2 && !length)) return -1;
	if (opcode == 9 && !frame_send(incoming + offset, length, 10)) return -1;
	memcpy(data, incoming + offset, length);
	buffered -= offset + length;
	memmove(incoming, incoming + offset + length, buffered);
	return opcode == 2 ? (int)length : 0;
}

int online_socket_receive(uint8_t *data, size_t size) {
	int complete = take_frame(data, size);
	if (complete) return complete;
	if (buffered == sizeof incoming) return -1;
	int got = online_stream_receive(incoming + buffered, sizeof incoming - buffered);
	if (got < 0) return -1;
	buffered += (size_t)got;
	return take_frame(data, size);
}

void online_socket_close(void) {
	online_stream_close();
	buffered = 0;
}
#endif
