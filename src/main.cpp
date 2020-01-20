#include "Client.hpp"

#include <emscripten.h>
#include <emscripten/html5.h>

#include <memory>
#include <exception>
#include <new>

static std::unique_ptr<Client> cl;

int main() {
	// argless EM_ASM causes warnings/errors with -Wall
	EM_ASM({window['OWOP'] = Module.api = {}}, 0);

#ifdef DEBUG
	EM_ASM({Module.api['module'] = Module}, 0);
#endif

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

	cl->open("main");
}
