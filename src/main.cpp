#include "Client.hpp"

#include <emscripten.h>
#include <emscripten/html5.h>

#include <cstdlib>
#include <cstdio>
#include <memory>
#include <exception>
#include <new>

#include <JsApiProxy.hpp>

#ifndef OWOP_VERSION
#	define OWOP_VERSION "unknown"
#endif

#ifdef DEBUG
EM_JS(void, enable_auto_refresh_client, (void), {
	var ws = null;
	var to = null;
	function conn() {
		ws = new WebSocket("ws://" + location.hostname + ":9005");
		ws.onmessage = function(m) {
			console.log(m.data);
			clearTimeout(to);
			to = setTimeout(function() { location.reload(true); }, 1500);
		};
	}

	try { conn(); } catch (e) { }
});
#endif

static std::unique_ptr<Client> cl;

int main(int argc, char * argv[]) {
	JsApiProxy& api = JsApiProxy::getInstance();

#ifdef DEBUG
	enable_auto_refresh_client();
#endif

	std::printf("[main] Compiled on " __DATE__ " @ " __TIME__ "\n");
	std::printf("[main] Version: " OWOP_VERSION "\n");

	emscripten_set_beforeunload_callback(nullptr, [] (int, const void *, void *) -> const char * {
		cl = nullptr;
		return nullptr;
	});

	std::atexit([] {
		std::printf("[main] std::atexit\n");
		cl = nullptr;
	});

	std::set_new_handler([] {
		std::puts("OOM detected");
		if (!cl->freeMemory()) {
			std::terminate();
		}
	});

	cl = std::make_unique<Client>(api);

	cl->open(argc >= 2 ? argv[1] : "main");
}
