#include "JsApiProxy.hpp"

#include <memory>
#include <cstdio>

#include <emscripten.h>

#include <util/explints.hpp>

#include <Client.hpp>
#include <world/World.hpp>

EM_JS(void, create_api_structure, (void), {
	var unsign = function(str) {
		return function() {
			return Module[str].apply(null, arguments) >>> 0;
		};
	};

	window['OWOP'] = Module.api = {
		'world': {
			getPixel: unsign('_owop_api_get_pixel'),
			setPixel: Module['_owop_api_set_pixel']
		},
		'client': {},
		'chat': {},
		'player': {},
		'camera': {}
	};
});

extern "C" {

EMSCRIPTEN_KEEPALIVE
bool owop_api_set_pixel(World::Pos x, World::Pos y, u32 clrU32) {
	Client * c = JsApiProxy::getInstance().getClient();
	if (!c) {
		return false;
	}

	World * w = c->getWorld();
	if (!w) {
		return false;
	}

	RGB_u clr;
	clr.rgb = clrU32;
	//std::printf("clr: %i,%i,%i,%i\n", clr.c.r, clr.c.g, clr.c.b, clr.c.a);
	return w->setPixel(x, y, clr);
}

EMSCRIPTEN_KEEPALIVE
u32 owop_api_get_pixel(World::Pos x, World::Pos y) {
	Client * c = JsApiProxy::getInstance().getClient();
	if (!c) {
		return 0;
	}

	World * w = c->getWorld();
	if (!w) {
		return 0;
	}

	return w->getPixel(x, y).rgb;
}

}

JsApiProxy::JsApiProxy()
: cli(nullptr) {
	create_api_structure();

#ifdef DEBUG
	// argless EM_ASM causes warnings/errors with -Wall
	EM_ASM({Module.api['module'] = Module}, 0);
	std::printf("[JsApiProxy] DEBUG build, defining OWOP.module\n");
#endif
}

JsApiProxy& JsApiProxy::getInstance() {
	static JsApiProxy instance;

	return instance;
}

void JsApiProxy::setClientInstance(Client * c) {
	cli = c;
}

Client * JsApiProxy::getClient() {
	return cli;
}
