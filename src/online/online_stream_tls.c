#include "online_stream.h"

/* The stream over BSD sockets and mbedTLS, both linked in: the PS4's,
 * whose SDK has no libcurl (docker/mbedtls.sh). The transport above writes
 * the WebSocket's upgrade itself, as over the consoles' libcurl. */
#ifdef CW_PS4
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/ssl.h>

#include "analytics_roots.h"

static struct {
	mbedtls_net_context net;
	mbedtls_ssl_context ssl;
	mbedtls_ssl_config conf;
	mbedtls_x509_crt roots;
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context drbg;
	bool secure, tls;   /* (tls: its contexts made, to free) */
	SDL_atomic_t *stop;
} S;

/* The kernel's random bytes, getentropy's 256 at a time */
static bool kernel_random(uint8_t *out, size_t size) {
	for (size_t n; size; out += n, size -= n) {
		n = size < 256 ? size : 256;
		if (getentropy(out, n) != 0) return false;
	}
	return true;
}

/* (mbedTLS's entropy, as it is built without a platform source: docker/mbedtls.sh) */
static int kernel_entropy(void *data, unsigned char *out, size_t size, size_t *got) {
	(void)data;
	*got = kernel_random(out, size) ? size : 0;
	return *got ? 0 : MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
}

/* ws://host[:port]/path or wss://...: whether it is TLS, its host and port */
static bool url_parts(const char *url, char *host, size_t host_size, char *port, size_t port_size) {
	S.secure = !strncmp(url, "wss://", 6);
	if (!S.secure && strncmp(url, "ws://", 5)) return false;
	const char *h = url + (S.secure ? 6 : 5);
	size_t len = strcspn(h, ":/");
	if (!len || len >= host_size) return false;
	memcpy(host, h, len);
	host[len] = 0;
	if (h[len] != ':') {
		snprintf(port, port_size, "%s", S.secure ? "443" : "80");
		return true;
	}
	size_t digits = strcspn(h + len + 1, "/");
	if (!digits || digits >= port_size) return false;
	memcpy(port, h + len + 1, digits);
	port[digits] = 0;
	return true;
}

/* TLS over the connection: Let's Encrypt's roots the game carries, the
 * host's name checked, HTTP/1.1 offered for the upgrade */
static bool tls_start(const char *host, char *error, size_t size) {
	static const char *alpn[] = { "http/1.1", NULL };
	mbedtls_ssl_init(&S.ssl);
	mbedtls_ssl_config_init(&S.conf);
	mbedtls_x509_crt_init(&S.roots);
	mbedtls_entropy_init(&S.entropy);
	mbedtls_ctr_drbg_init(&S.drbg);
	S.tls = true;
	int r = mbedtls_entropy_add_source(&S.entropy, kernel_entropy, NULL, 32, MBEDTLS_ENTROPY_SOURCE_STRONG);
	if (!r) r = mbedtls_ctr_drbg_seed(&S.drbg, mbedtls_entropy_func, &S.entropy, (const unsigned char *)"cyberworld", 10);
	if (!r) r = mbedtls_x509_crt_parse(&S.roots, (const unsigned char *)analytics_roots, strlen(analytics_roots) + 1);
	if (!r) r = mbedtls_ssl_config_defaults(&S.conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
	if (!r) {
		mbedtls_ssl_conf_authmode(&S.conf, MBEDTLS_SSL_VERIFY_REQUIRED);
		mbedtls_ssl_conf_ca_chain(&S.conf, &S.roots, NULL);
		mbedtls_ssl_conf_rng(&S.conf, mbedtls_ctr_drbg_random, &S.drbg);
		r = mbedtls_ssl_conf_alpn_protocols(&S.conf, alpn);
	}
	if (!r) r = mbedtls_ssl_setup(&S.ssl, &S.conf);
	if (!r) r = mbedtls_ssl_set_hostname(&S.ssl, host);
	if (!r) {
		mbedtls_ssl_set_bio(&S.ssl, &S.net, mbedtls_net_send, mbedtls_net_recv, NULL);
		while ((r = mbedtls_ssl_handshake(&S.ssl)) == MBEDTLS_ERR_SSL_WANT_READ || r == MBEDTLS_ERR_SSL_WANT_WRITE)
			if (SDL_AtomicGet(S.stop)) break;
	}
	if (r) snprintf(error, size, "TLS failed: -0x%04x", (unsigned)-r);
	return !r;
}

bool online_stream_open(const char *url, SDL_atomic_t *stop, char *error, size_t size) {
	char host[256], port[8];
	S.stop = stop;
	if (!url_parts(url, host, sizeof host, port, sizeof port)) {
		snprintf(error, size, "Not a server's address: %s", url);
		return false;
	}
	mbedtls_net_init(&S.net);
	int r = mbedtls_net_connect(&S.net, host, port, MBEDTLS_NET_PROTO_TCP);
	if (r) {
		snprintf(error, size, "Connect failed: -0x%04x", (unsigned)-r);
		return false;
	}
	if (S.secure && !tls_start(host, error, size)) {
		online_stream_close();
		return false;
	}
	mbedtls_net_set_nonblock(&S.net);
	return true;
}

bool online_stream_send(const uint8_t *data, size_t size) {
	uint32_t started = SDL_GetTicks();
	while (size && !SDL_AtomicGet(S.stop)) {
		int r = S.secure ? mbedtls_ssl_write(&S.ssl, data, size) : mbedtls_net_send(&S.net, data, size);
		if (r == MBEDTLS_ERR_SSL_WANT_READ || r == MBEDTLS_ERR_SSL_WANT_WRITE) r = 0;
		if (r < 0) return false;
		data += r;
		size -= (size_t)r;
		if (SDL_GetTicks() - started >= 2000) return false;
		if (!r) SDL_Delay(2);
	}
	return size == 0;
}

int online_stream_receive(uint8_t *data, size_t size) {
	int r = S.secure ? mbedtls_ssl_read(&S.ssl, data, size) : mbedtls_net_recv(&S.net, data, size);
	if (r == MBEDTLS_ERR_SSL_WANT_READ || r == MBEDTLS_ERR_SSL_WANT_WRITE) return 0;
	return r > 0 ? r : -1;
}

bool online_stream_random(uint8_t *out, size_t size) { return kernel_random(out, size); }

void online_stream_close(void) {
	if (S.tls) {
		mbedtls_ssl_free(&S.ssl);
		mbedtls_ssl_config_free(&S.conf);
		mbedtls_x509_crt_free(&S.roots);
		mbedtls_ctr_drbg_free(&S.drbg);
		mbedtls_entropy_free(&S.entropy);
	}
	mbedtls_net_free(&S.net);
	S.tls = S.secure = false;
}
/* (the stream is not one the worker can wait on here: looked at every
 * millisecond, a key sent at the next look; online_socket.h) */
void online_stream_wait(uint32_t ms) { SDL_Delay(ms ? 1 : 0); }
void online_stream_wake(void) {}
#endif
