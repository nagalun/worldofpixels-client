#include <util/emsc/gl/WebGlContext.hpp>

#include <cstdio>
#include <cmath>

#include <emscripten.h>
#include <emscripten/html5.h>

namespace gl {

WebGlContext::WebGlContext(const char * targetCanvas)
: targetCanvas(targetCanvas),
  sizeCache{-1, -1},
  renderLoopSet(false),
  renderPaused(false) {
	if (!activateRenderingContext(false)
			&& !activateRenderingContext(true)) {
		// WebGL not supported...?
		std::fprintf(stderr, "[WebGlContext] Couldn't create context. Bad GPU drivers?\n");
	}
}

WebGlContext::~WebGlContext() {
	stopRenderLoop();
	destroyRenderingContext();
}

WebGlContext::WebGlContext(WebGlContext&& other)
: GlContext(std::move(other)),
  ctxInfo(std::exchange(other.ctxInfo, 0)),
  targetCanvas(std::exchange(other.targetCanvas, nullptr)),
  renderLoopSet(other.renderLoopSet),
  renderPaused(other.renderPaused) { }

WebGlContext& WebGlContext::operator=(WebGlContext&& other) {
	destroyRenderingContext();

	GlContext::operator=(std::move(other));

	ctxInfo = std::exchange(other.ctxInfo, 0);
	targetCanvas = std::exchange(other.targetCanvas, nullptr);
	renderLoopSet = other.renderLoopSet;
	renderPaused = other.renderPaused;

	return *this;
}

bool WebGlContext::resize(int w, int h) {
	bool ok = emscripten_set_canvas_element_size(targetCanvas, w, h) >= 0;

	sizeCache.w = -1;

	if (resizeCb) {
		resizeCb();
	}

	return ok;
}

GlContext::Size WebGlContext::getSize() const {
	if (sizeCache.w < 0) {
		int * w = &sizeCache.w;
		int * h = &sizeCache.h;

		emscripten_webgl_get_drawing_buffer_size(ctxInfo, w, h);
	}

	return sizeCache;
}

void WebGlContext::setTitle(const char* name) {
	emscripten_set_window_title(name);
}

bool WebGlContext::activateRenderingContext(bool webgl1) {
	EmscriptenWebGLContextAttributes attr;
	emscripten_webgl_init_context_attributes(&attr);

	attr.enableExtensionsByDefault = false;
	attr.alpha = false;
	attr.antialias = false;
	if (!webgl1) {
		attr.majorVersion = 2;
	}

	ctxInfo = emscripten_webgl_create_context(targetCanvas, &attr);
	if (ctxInfo <= 0) {
		std::fprintf(stderr, "[WebGlContext] WebGL%c context creation failed: %i\n", webgl1 ? '1' : '2', ctxInfo);
		return false;
	}

	int ok = 0;
	ok |= emscripten_webgl_enable_ANGLE_instanced_arrays(ctxInfo) == EM_TRUE;
	ok |= (emscripten_webgl_enable_WEBGL_multi_draw(ctxInfo) == EM_TRUE) << 1;
	ok |= (emscripten_webgl_enable_OES_vertex_array_object(ctxInfo) == EM_TRUE) << 2;

	std::printf("[WebGlContext] CtxInfo: %d, extensions: %d\n", ctxInfo, ok);

	if (EMSCRIPTEN_RESULT err = emscripten_webgl_make_context_current(ctxInfo)) {
		std::fprintf(stderr, "[WebGlContext] Context activation failed: %i\n", err);
		destroyRenderingContext();
		return false;
	}

	emscripten_set_webglcontextlost_callback(targetCanvas, this, true, emEvent);
	emscripten_set_webglcontextrestored_callback(targetCanvas, this, true, emEvent);
	emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, +[] (int, const EmscriptenUiEvent * ev, void * ctx) -> EM_BOOL {
		WebGlContext& c = *static_cast<WebGlContext *>(ctx);

		double canvasWidth;
		double canvasHeight;
		emscripten_get_element_css_size(c.targetCanvas, &canvasWidth, &canvasHeight);

		c.resize(std::round(canvasWidth), std::round(canvasHeight));
		return false;
	});

	std::printf("[WebGlContext] Created WebGL%c rendering context\n", webgl1 ? '1' : '2');

	double canvasWidth;
	double canvasHeight;

	emscripten_get_element_css_size(targetCanvas, &canvasWidth, &canvasHeight);
	resize(std::round(canvasWidth), std::round(canvasHeight));

	return true;
}

void WebGlContext::destroyRenderingContext() {
	if (ctxInfo > 0) {
		emscripten_set_webglcontextlost_callback(targetCanvas, nullptr, true, nullptr);
		emscripten_set_webglcontextrestored_callback(targetCanvas, nullptr, true, nullptr);
		emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, true, nullptr);

		if (EMSCRIPTEN_RESULT err = emscripten_webgl_destroy_context(ctxInfo)) {
			std::fprintf(stderr, "[WebGlContext] Context destruction failed: %i\n", err);
		}

		ctxInfo = 0;
	}
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
		return true;
		break;

	case EMSCRIPTEN_EVENT_WEBGLCONTEXTRESTORED:
		std::printf("[WebGlContext] Context restored!\n");
		if (ctx.restoredCb) { ctx.restoredCb(); }
		break;
	}

	return false;
}

bool WebGlContext::ok() const {
	return ctxInfo != 0;
}

bool WebGlContext::pauseRendering() {
	if (!renderPaused) {
		renderPaused = true;
		emscripten_pause_main_loop();
	}

	return true;
}

bool WebGlContext::resumeRendering() {
	if (renderPaused) {
		renderPaused = false;
		emscripten_resume_main_loop();
	}

	return true;
}

double WebGlContext::getTime() const {
	return emscripten_get_now();
}

} /* namespace gl */
