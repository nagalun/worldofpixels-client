#include "Renderer.hpp"

#include <emscripten.h>
#include <emscripten/html5.h>

#include <cstdio>

static const char * targetCanvas = "#world";

EM_JS(void, get_window_size, (int * width, int * height), {
	Module['HEAP32'][width / Int32Array.BYTES_PER_ELEMENT] = window.innerWidth;
	Module['HEAP32'][height / Int32Array.BYTES_PER_ELEMENT] = window.innerHeight;
});

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

bool Renderer::resizeCanvas(int w, int h) {
	return emscripten_set_canvas_element_size(targetCanvas, w, h) == EMSCRIPTEN_RESULT_SUCCESS;
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

	const char * windowTarget = nullptr;
	windowTarget += 2; // look in __findEventTarget

	emscripten_set_webglcontextlost_callback(targetCanvas, this, true, emEvent);
	emscripten_set_webglcontextrestored_callback(targetCanvas, this, true, emEvent);
	emscripten_set_resize_callback(windowTarget, this, true, +[] (int, const EmscriptenUiEvent * ev, void * rp) -> EM_BOOL {
		Renderer& r = *static_cast<Renderer *>(rp);
		r.resizeCanvas(ev->windowInnerWidth, ev->windowInnerHeight);
		r.resizeRenderingContext();
		return true;
	});

	int scrWidth;
	int scrHeight;

	get_window_size(&scrWidth, &scrHeight);
	resizeCanvas(scrWidth, scrHeight);

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
