#include "WebGlContext.hpp"

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cmath>

#include <emscripten.h>
#include <emscripten/html5.h>

#include "Settings.hpp"

/* used to reinitialize the canvas element on context losses and destruction */
//extern "C" void reset_element(const char * target, std::size_t len);
EM_JS(void, reset_element, (const char * target, std::size_t len), {
	var targetSelector = UTF8ToString(target, len);
	var el = document.querySelector(targetSelector);
	if (el) {
		el.replaceWith(el.cloneNode());
	}
});

EM_JS(void, ctx_give_up, (void), {
	alert("Sorry, OWOP can't recover from a WebGL failure. Try reloading the page.");
});

namespace gl {

WebGlContext::WebGlContext(const char * targetCanvas, const char * targetSizeElem, bool forceWebgl1)
: ctxInfo(0),
  targetCanvas(targetCanvas),
  targetSizeElem(targetSizeElem),
  sizeCache{-1, -1},
  dipSizeCache{-1., -1.},
  realSizeCache{-1, -1},
  dprCache(-1.0),
  renderLoopSet(false),
  renderPaused(false) {
	activateRenderingContext(forceWebgl1);
}

WebGlContext::~WebGlContext() {
	if (ctxInfo > 0) {
		stopRenderLoop();
		destroyRenderingContext();
	}

	if (targetCanvas) { // flashes white before fade in completes
		reset_element(targetCanvas, std::strlen(targetCanvas));
	}
}

WebGlContext::WebGlContext(WebGlContext&& other)
: GlContext(std::move(other)),
  ctxInfo(std::exchange(other.ctxInfo, 0)),
  targetCanvas(std::exchange(other.targetCanvas, nullptr)),
  targetSizeElem(std::exchange(other.targetSizeElem, nullptr)),
  sizeCache(std::exchange(other.sizeCache, {-1, -1})),
  dipSizeCache(std::exchange(other.dipSizeCache, {-1., -1.})),
  realSizeCache(std::exchange(other.realSizeCache, {-1, -1})),
  dprCache(std::exchange(other.dprCache, -1)),
  renderLoopSet(other.renderLoopSet),
  renderPaused(other.renderPaused) {
	if (ctxInfo > 0) {
		emscripten_set_webglcontextlost_callback(targetCanvas, this, true, emEvent);
		emscripten_set_webglcontextrestored_callback(targetCanvas, this, true, emEvent);
		emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, onEvtResize);
		sk.setCb([this] (auto) {
			onEvtResize(0, nullptr, this);
		});
	}
}

WebGlContext& WebGlContext::operator=(WebGlContext&& other) {
	destroyRenderingContext();

	GlContext::operator=(std::move(other));

	ctxInfo = std::exchange(other.ctxInfo, 0);
	targetCanvas = std::exchange(other.targetCanvas, nullptr);
	targetSizeElem = std::exchange(other.targetSizeElem, nullptr);
	sizeCache = std::exchange(other.sizeCache, {-1, -1});
	dipSizeCache = std::exchange(other.dipSizeCache, {-1., -1.});
	realSizeCache = std::exchange(other.realSizeCache, {-1, -1});
	dprCache = std::exchange(other.dprCache, -1);
	renderLoopSet = other.renderLoopSet;
	renderPaused = other.renderPaused;

	if (ctxInfo > 0) {
		emscripten_set_webglcontextlost_callback(targetCanvas, this, true, emEvent);
		emscripten_set_webglcontextrestored_callback(targetCanvas, this, true, emEvent);
		emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, onEvtResize);
		sk.setCb([this] (auto) {
			onEvtResize(0, nullptr, this);
		});
	}

	return *this;
}

bool WebGlContext::resize(int w, int h) {
	return resize(w, h, w, h);
}

bool WebGlContext::resize(int drawingWidth, int drawingHeight, double elemWidth, double elemHeight) {
	bool ok = emscripten_set_canvas_element_size(targetCanvas, drawingWidth, drawingHeight) >= 0;
	emscripten_set_element_css_size(targetCanvas, elemWidth, elemHeight);

	sizeCache.w = -1;

	if (ctxInfo != 0 && resizeCb) {
		resizeCb();
	}

	return ok;
}

GlContext::Size<int> WebGlContext::getSize() const {
	if (sizeCache.w < 0) {
		int * w = &sizeCache.w;
		int * h = &sizeCache.h;

		emscripten_webgl_get_drawing_buffer_size(ctxInfo, w, h);
	}

	return sizeCache;
}

// aka. css pixels
GlContext::Size<double> WebGlContext::getDipSize() const {
	if (dipSizeCache.w < 0) {
		GlContext::Size<double> dipSize;
		GlContext::Size<int> realSize = getRealSize();
		double w = realSize.w;
		double h = realSize.h;
		double dpr = getDpr();

		w /= dpr;
		h /= dpr;

		dipSize.w = w;
		dipSize.h = h;
		dipSizeCache = dipSize;
	}

	return dipSizeCache;
}

// aka. screen pixels
GlContext::Size<int> WebGlContext::getRealSize() const {
	if (realSizeCache.w < 0) {
		GlContext::Size<int> realSize;
		double w;
		double h;
		double dpr = getDpr();
		emscripten_get_element_css_size(targetSizeElem, &w, &h);

		w = std::round(w * dpr);
		h = std::round(h * dpr);

		realSize.w = w;
		realSize.h = h;
		realSizeCache = realSize;
	}

	return realSizeCache;
}

void WebGlContext::setTitle(const char* name) {
	emscripten_set_window_title(name);
}

bool WebGlContext::activateRenderingContext(bool forceWebgl1) {
	if ((forceWebgl1 || !activateRenderingContextAs(false))
			&& !activateRenderingContextAs(true)) {
		// WebGL not supported...?
		std::fprintf(stderr, "[WebGlContext] Couldn't create context. Bad GPU drivers?\n");
		return false;
	}

	return true;
}

bool WebGlContext::activateRenderingContextAs(bool webgl1) {
	destroyRenderingContext();

	EmscriptenWebGLContextAttributes attr;
	emscripten_webgl_init_context_attributes(&attr);

	attr.enableExtensionsByDefault = false;
	attr.alpha = false;
	attr.antialias = false;
	attr.majorVersion = webgl1 ? 1 : 2;

	auto s = getRealSize();
	auto sdip = getDipSize();
	resize(s.w, s.h, sdip.w, sdip.h);

	ctxInfo = emscripten_webgl_create_context(targetCanvas, &attr);
	if (ctxInfo <= 0) {
		std::fprintf(stderr, "[WebGlContext] WebGL%c context creation failed: %i\n", webgl1 ? '1' : '2', ctxInfo);
		return false;
	}

	int ok = 0;
	if (webgl1) {
		ok |= emscripten_webgl_enable_ANGLE_instanced_arrays(ctxInfo) == EM_TRUE;
		//ok |= (emscripten_webgl_enable_WEBGL_multi_draw(ctxInfo) == EM_TRUE) << 1;
		ok |= (emscripten_webgl_enable_OES_vertex_array_object(ctxInfo) == EM_TRUE) << 1;
	}

	std::printf("[WebGlContext] CtxInfo: %d, extensions: %d\n", ctxInfo, ok);

	if (EMSCRIPTEN_RESULT err = emscripten_webgl_make_context_current(ctxInfo)) {
		std::fprintf(stderr, "[WebGlContext] Context activation failed: %i\n", err);
		destroyRenderingContext();
		return false;
	}

	emscripten_set_webglcontextlost_callback(targetCanvas, this, true, emEvent);
	emscripten_set_webglcontextrestored_callback(targetCanvas, this, true, emEvent);
	emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, onEvtResize);
	sk = Settings::get().nativeRes.connect([this] (auto) {
		onEvtResize(0, nullptr, this);
	});

	std::printf("[WebGlContext] Created WebGL%c rendering context\n", webgl1 ? '1' : '2');

	return true;
}

void WebGlContext::destroyRenderingContext() {
	if (ctxInfo > 0) {
		auto currCtx = emscripten_webgl_get_current_context();
		if (currCtx == ctxInfo) {
			emscripten_webgl_make_context_current(0);
		}

		emscripten_set_webglcontextlost_callback(targetCanvas, nullptr, true, nullptr);
		emscripten_set_webglcontextrestored_callback(targetCanvas, nullptr, true, nullptr);
		emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, true, nullptr);
		sk.disconnect();

		if (EMSCRIPTEN_RESULT err = emscripten_webgl_destroy_context(ctxInfo)) {
			std::fprintf(stderr, "[WebGlContext] Context destruction failed: %i\n", err);
		}

		ctxInfo = 0;
	}
}

void WebGlContext::giveUp() {
	ctx_give_up();
	std::exit(1);
}

void WebGlContext::startRenderLoop(em_arg_callback_func f, void * user) {
	if (renderLoopSet) {
		stopRenderLoop();
	}

	emscripten_set_main_loop_arg(f, user, 0, false);
	renderLoopSet = true;
	renderPaused = false;
}

void WebGlContext::stopRenderLoop() {
	emscripten_cancel_main_loop();
	renderLoopSet = false;
}

int WebGlContext::emEvent(int eventType, const void *, void * r) {
	WebGlContext& ctx = *static_cast<WebGlContext *>(r);

	switch (eventType) {
	case EMSCRIPTEN_EVENT_WEBGLCONTEXTLOST:
		std::printf("[WebGlContext] Context lost!\n");
		if (ctx.lostCb) { ctx.lostCb(); }
		ctx.destroyRenderingContext();
		reset_element(ctx.targetCanvas, std::strlen(ctx.targetCanvas));
		return true;
		break;

	case EMSCRIPTEN_EVENT_WEBGLCONTEXTRESTORED:
		std::printf("[WebGlContext] Context restored?\n");
		if (ctx.restoredCb) { ctx.restoredCb(); }
		break;
	}

	return false;
}

EM_BOOL WebGlContext::onEvtResize(int, const EmscriptenUiEvent *, void * ctx) {
	WebGlContext& c = *static_cast<WebGlContext *>(ctx);
	c.dipSizeCache.w = -1;
	c.realSizeCache.w = -1;
	c.dprCache = -1.0;
	auto sdip = c.getDipSize();
	auto s = c.getRealSize();
	c.resize(s.w, s.h, sdip.w, sdip.h);
	return false;
}

bool WebGlContext::ok() const {
	return ctxInfo > 0;
}

bool WebGlContext::pauseRendering() {
	if (!renderLoopSet) {
		return false;
	}

	if (!renderPaused) {
		renderPaused = true;
		emscripten_pause_main_loop();
	}

	return true;
}

bool WebGlContext::resumeRendering() {
	if (!renderLoopSet) {
		return false;
	}

	if (renderPaused) {
		renderPaused = false;
		emscripten_resume_main_loop();
	}

	return true;
}

double WebGlContext::getTime() const {
	return emscripten_get_now();
}

double WebGlContext::getDpr() const {
	if (!Settings::get().nativeRes) {
		return 1.0;
	}

	if (dprCache < 0.0) {
		dprCache = emscripten_get_device_pixel_ratio();
	}

	return dprCache;
}


} /* namespace gl */
