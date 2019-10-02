#include "Renderer.hpp"

#include <emscripten.h>
#include <emscripten/html5.h>

#include <cstdio>

static const char * targetCanvas = "#world";

static int emEvent(int eventType, const void *, void * r) {
	std::printf("[Renderer] Context event fired: %i\n", eventType);
	return false;
}

void Renderer::resumeRendering() {
	emscripten_resume_main_loop();
}

void Renderer::pauseRendering() {
	emscripten_pause_main_loop();
}

void Renderer::getViewportSize() {
	emscripten_get_canvas_element_size(targetCanvas, &vpWidth, &vpHeight);
	std::printf("[Renderer] Viewport size: %ix%i\n", vpWidth, vpHeight);
}

bool Renderer::activateRenderingContext() {
	EmscriptenWebGLContextAttributes attr;
	emscripten_webgl_init_context_attributes(&attr);

	attr.alpha = false;
	attr.antialias = false;

	ctxInfo = emscripten_webgl_create_context(targetCanvas, &attr);
	if (ctxInfo < 0) {
		std::fprintf(stderr, "[Renderer] Context creation failed: %i\n", ctxInfo);
		return false;
	}

	if (EMSCRIPTEN_RESULT err = emscripten_webgl_make_context_current(ctxInfo)) {
		std::fprintf(stderr, "[Renderer] Context activation failed: %i\n", err);
		return false;
	}

	emscripten_set_webglcontextlost_callback(targetCanvas, this, true, emEvent);
	emscripten_set_webglcontextrestored_callback(targetCanvas, this, true, emEvent);
	getViewportSize();

	return setupRenderingContext();
}

void Renderer::destroyRenderingContext() {
	if (ctxInfo > 0) {
		if (EMSCRIPTEN_RESULT err = emscripten_webgl_destroy_context(ctxInfo)) {
			std::fprintf(stderr, "[Renderer] Context destruction failed: %i\n", err);
		}
	}
}

void Renderer::startRenderLoop() {
	emscripten_set_main_loop_arg(Renderer::doRender, this, 0, false);
}

void Renderer::stopRenderLoop() {
	emscripten_cancel_main_loop();
}
