#include "Client.hpp"

#include <emscripten.h>
#include <emscripten/html5.h>

#include <cstdio>
#include <memory>
#include <exception>
#include <new>

#ifndef OWOP_VERSION
#	define OWOP_VERSION "unknown"
#endif

#ifdef DEBUG
EM_JS(void, enable_auto_refresh_client, (void), {
	var ws = null;
	function conn() {
		ws = new WebSocket("ws://" + location.hostname + ":9005");
		ws.onmessage = function(m) {
			console.log(m.data);
			location.reload(true);
		};
	}

	conn();
});
#endif

static std::unique_ptr<Client> cl;

int main(int argc, char * argv[]) {
	// argless EM_ASM causes warnings/errors with -Wall
	EM_ASM({window['OWOP'] = Module.api = {}}, 0);

#ifdef DEBUG
	EM_ASM({Module.api['module'] = Module}, 0);
	std::printf("[main] DEBUG build, defining OWOP.module\n");

	enable_auto_refresh_client();
#endif

	std::printf("[main] Compiled on " __DATE__ " @ " __TIME__ "\n");
	std::printf("[main] Version: " OWOP_VERSION "\n");

	emscripten_set_beforeunload_callback(nullptr, [] (int, const void *, void *) -> const char * {
		cl = nullptr;
		return nullptr;
	});

	std::set_new_handler([] {
		std::puts("OOM detected");
		if (!cl->freeMemory()) {
			std::terminate();
		}
	});

	cl = std::make_unique<Client>();

	cl->open(argc >= 2 ? argv[1] : "main");
}
