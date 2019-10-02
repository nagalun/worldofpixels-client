#include "Client.hpp"

#include <emscripten/html5.h>

#include <memory>

#include "jstext.hpp"

static std::unique_ptr<Client> cl;

int main() {
	js_txt_init();
	emscripten_set_beforeunload_callback(nullptr, [] (int, const void *, void *) -> const char * {
		cl = nullptr;
		return nullptr;
	});

	cl = std::make_unique<Client>();

	cl->open("main");
}
