#include "Renderer.hpp"

#include <emscripten.h>
#include <emscripten/html5.h>
#include <cstdio>
#include <cmath>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "World.hpp"
#include "explints.hpp"

// TODO: use resume/pauseRendering()
static const char * targetCanvas = "#world";

Renderer::Renderer(World& w)
: w(w),
  ctxInfo(0),
  vpWidth(1),
  vpHeight(1) {
	if (!activateRenderingContext()) {
		std::abort();
	}

	getViewportSize();
	startRenderLoop();
	std::printf("[Renderer] Initialized\n");
}

Renderer::~Renderer() {
	stopRenderLoop();
	destroyRenderingContext();
	std::printf("[Renderer] Destroyed\n");
}

void Renderer::resumeRendering() {
	emscripten_resume_main_loop();
}

void Renderer::pauseRendering() {
	emscripten_pause_main_loop();
}

void Renderer::loadMissingChunks() {
	float hVpWidth = vpWidth / 2.f;
	float hVpHeight = vpHeight / 2.f;
	float tlx = std::floor((getX() - hVpWidth) / Chunk::size);
	float tly = std::floor((getY() - hVpHeight) / Chunk::size);
	float brx = std::floor((getX() + hVpWidth) / Chunk::size);
	float bry = std::floor((getY() + hVpHeight) / Chunk::size);

	for (; tly <= bry; tly += 1.f) {
		for (float tlx2 = tlx; tlx2 <= brx; tlx2 += 1.f) {
			w.getOrLoadChunk(tlx2, tly);
		}
	}
}

void Renderer::render() {
	pauseRendering();
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

	emscripten_set_webglcontextlost_callback(targetCanvas, this, true, Renderer::emEvent);
	emscripten_set_webglcontextrestored_callback(targetCanvas, this, true, Renderer::emEvent);

	return true;
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

void Renderer::doRender(void * r) {
	static_cast<Renderer *>(r)->render();
}

int Renderer::emEvent(int eventType, const void *, void * r) {
	std::printf("[Renderer] Context event fired: %i\n", eventType);
	return false;
}
