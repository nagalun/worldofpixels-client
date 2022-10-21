#include "Client.hpp"

#include <emscripten.h>
#include <emscripten/html5.h>

#include <cstdlib>
#include <cstdio>
#include <memory>
#include <exception>
#include <new>

#include "util/preproc.hpp"
#include "JsApiProxy.hpp"

#ifndef OWOP_VERSION
#	define OWOP_VERSION unknown
#endif

#if __has_feature(address_sanitizer)
extern "C" const char *__asan_default_options() {
	return "leak_check_at_exit=false:quarantine_size_mb=32:allocator_may_return_null=true";
}
#endif

static std::unique_ptr<Client> cl;

int main(int argc, char * argv[]) {
	JsApiProxy& api = JsApiProxy::getInstance();

	std::printf("[main] Compiled on " __DATE__ " @ " __TIME__ "\n");
	std::printf("[main] Version: " TOSTRING(OWOP_VERSION) "\n");

	emscripten_set_beforeunload_callback(nullptr, [] (int, const void *, void *) -> const char * {
		cl = nullptr;
		return nullptr;
	});

//	std::atexit([] {
//		std::printf("[main] std::atexit\n");
//		cl = nullptr;
//	});

	std::set_new_handler([] {
		std::puts("OOM detected");
		if (!cl->freeMemory()) {
			std::terminate();
		}
	});

	cl = std::make_unique<Client>(api);

	cl->open(argc >= 2 ? argv[1] : "ws://localhost:13375", argc >= 3 ? argv[2] : "main");
}
