#include "network_prefs.h"

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
EM_JS(int, identity_random, (uint8_t *out), {
	if (!globalThis.crypto || !globalThis.crypto.getRandomValues) return 0;
	const bytes = new Uint8Array(16);
	globalThis.crypto.getRandomValues(bytes);
	HEAPU8.set(bytes, out);
	return 1;
});
#elif defined(_WIN32)
#include <windows.h>
#elif defined(__3DS__)
#include <3ds.h>
#else
#include <stdio.h>
#endif

bool network_identity(uint8_t out[NETWORK_ID_BYTES]) {
#if defined(__EMSCRIPTEN__)
	return identity_random(out) != 0;
#elif defined(_WIN32)
	typedef BOOLEAN (WINAPI *RandomBytes)(PVOID, ULONG);
	HMODULE library = LoadLibraryW(L"advapi32.dll");
	if (!library) return false;
	RandomBytes random_bytes = (RandomBytes)GetProcAddress(library, "SystemFunction036");
	bool ok = random_bytes && random_bytes(out, NETWORK_ID_BYTES);
	FreeLibrary(library);
	return ok;
#elif defined(__3DS__)
	if (R_FAILED(psInit())) return false;
	bool ok = R_SUCCEEDED(PS_GenerateRandomBytes(out, NETWORK_ID_BYTES));
	psExit();
	return ok;
#else
	FILE *file = fopen("/dev/urandom", "rb");
	if (!file) return false;
	bool ok = fread(out, NETWORK_ID_BYTES, 1, file) == 1;
	fclose(file);
	return ok;
#endif
}
