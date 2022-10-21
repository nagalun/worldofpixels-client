#include "JsApiProxy.hpp"

#include <memory>
#include <cstdio>

#include <emscripten.h>

#include "util/explints.hpp"

#include "Client.hpp"
#include "world/World.hpp"

EM_JS(void, create_api_structure, (void), {
	var f = function(fname) {
		return Module["_" + fname];
	};

	var uf = function unsign(fname) {
		var fn = f(fname);
		return function() {
			return fn.apply(null, arguments) >>> 0;
		};
	};

	var sf = function stringify(fname) {
		var fn = f(fname);
		return function() {
			var ptr = fn.apply(null, arguments);
			return ptr ? UTF8ToString(ptr) : null;
		};
	};

	window["OWOP"] = Module.api = {
		"events": {},
		"world": {
			"getName": sf("owop_api_get_world_name"),
			"getPixel": uf("owop_api_get_pixel"),
			"setPixel": f("owop_api_set_pixel"),
			get ["name"]() { return this["getName"](); }
		},
		"client": {
			"reconnect": f("owop_api_reconnect"),
			get ["ws"]() { return Module.JSWS.ws; }
		},
		"chat": {},
		"player": {},
		"camera": {
			"getX": f("owop_api_get_cam_x"),
			"getY": f("owop_api_get_cam_y"),
			"setPos": f("owop_api_set_cam_pos"),
			"getZoom": f("owop_api_get_cam_zoom"),
			"setZoom": f("owop_api_set_cam_zoom"),
			"setMomentum": f("owop_api_set_cam_momentum"),
			get ["x"]() { return this["getX"](); },
			set ["x"](x) { this["setPos"](x, this["y"]); return x; },
			get ["y"]() { return this["getY"](); },
			set ["y"](y) { this["setPos"](this["x"], y); return y; },
			get ["zoom"]() { return this["getZoom"](); },
			set ["zoom"](zoom) { return this["setZoom"](zoom); }
		}
	};
});

extern "C" {

/******
 * WORLD API
 ******/

EMSCRIPTEN_KEEPALIVE
const char * owop_api_get_world_name(void) {
	World * w = JsApiProxy::getWorld();
	return w ? w->getName().c_str() : nullptr;
}

EMSCRIPTEN_KEEPALIVE
bool owop_api_set_pixel(World::Pos x, World::Pos y, u32 clrU32) {
	World * w = JsApiProxy::getWorld();
	if (!w) {
		return false;
	}

	RGB_u clr;
	clr.rgb = clrU32;
	return w->setPixel(x, y, clr);
}

EMSCRIPTEN_KEEPALIVE
u32 owop_api_get_pixel(World::Pos x, World::Pos y) {
	World * w = JsApiProxy::getWorld();
	return w ? w->getPixel(x, y).rgb : 0;
}

/******
 * CLIENT API
 ******/

EMSCRIPTEN_KEEPALIVE
bool owop_api_reconnect(void) {
	Client * c = JsApiProxy::getClient();
	if (!c) {
		return false;
	}

	return c->reconnect();
}

/******
 * CAMERA API
 ******/

EMSCRIPTEN_KEEPALIVE
float owop_api_get_cam_x(void) {
	Camera * c = JsApiProxy::getCamera();
	return c ? c->getX() : 0.f;
}

EMSCRIPTEN_KEEPALIVE
float owop_api_get_cam_y(void) {
	Camera * c = JsApiProxy::getCamera();
	return c ? c->getY() : 0.f;
}

EMSCRIPTEN_KEEPALIVE
float owop_api_get_cam_zoom() {
	Camera * c = JsApiProxy::getCamera();
	return c ? c->getZoom() : 0.f;
}

EMSCRIPTEN_KEEPALIVE
void owop_api_set_cam_pos(float x, float y) {
	Camera * c = JsApiProxy::getCamera();
	if (c) {
		c->setPos(x, y);
	}
}

EMSCRIPTEN_KEEPALIVE
float owop_api_set_cam_zoom(float zoom) {
	Camera * c = JsApiProxy::getCamera();
	if (c) {
		c->setZoom(zoom);
		return c->getZoom();
	}

	return 0.f;
}

EMSCRIPTEN_KEEPALIVE
void owop_api_set_cam_momentum(float dx, float dy) {
	Camera * c = JsApiProxy::getCamera();
	if (c) {
		c->setMomentum(dx, dy);
	}
}

}

JsApiProxy::JsApiProxy()
: cli(nullptr) {
	create_api_structure();

#ifdef DEBUG
	// argless EM_ASM causes warnings/errors with -Wall
	EM_ASM({Module.api["module"] = Module}, 0);
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
	return getInstance().cli;
}

World * JsApiProxy::getWorld() {
	Client * c = getClient();
	return c ? c->getWorld() : nullptr;
}

Renderer * JsApiProxy::getRenderer() {
	World * w = getWorld();
	return w ? std::addressof(w->getRenderer()) : nullptr;
}

Camera * JsApiProxy::getCamera() {
	return getRenderer();
}
