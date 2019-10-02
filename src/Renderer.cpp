#include "Renderer.hpp"

#include <cstdio>
#include <cmath>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "World.hpp"
#include "explints.hpp"

// TODO: use resume/pauseRendering()

Renderer::Renderer(World& w)
: w(w),
  ctxInfo(0),
  vpWidth(1),
  vpHeight(1) {
	if (!activateRenderingContext()) {
		std::abort();
	}

	startRenderLoop();
	std::printf("[Renderer] Initialized\n");
}

Renderer::~Renderer() {
	stopRenderLoop();
	destroyRenderingContext();
	std::printf("[Renderer] Destroyed\n");
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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	pauseRendering();
}

bool Renderer::setupView() {
	glm::vec3    eye{0.f, 0.f,  1.5f};
	glm::vec3 center{0.f, 0.f, -5.0f};
	glm::vec3     up{0.f, 1.f,  0.0f};

	view = glm::lookAt(eye, center, up);
	return true;
}

bool Renderer::setupProjection() {
	float ratio = static_cast<float>(vpWidth) / vpHeight;

	glViewport(0, 0, vpWidth, vpHeight);
	// left, right, bottom, top, near, far
	projection = glm::frustum(-ratio, ratio, -1.f, 1.f, 1.f, 10.f);

	return true;
}

bool Renderer::setupRenderingContext() {
	glClearColor(.5f, .5f, .5f, .5f);
	setupView();
	setupProjection();
	return true;
}

void Renderer::doRender(void * r) {
	static_cast<Renderer *>(r)->render();
}
