#pragma once

#include <utility>
#include <vector>

#include <glm/ext/matrix_float4x4.hpp>

#include "Camera.hpp"
#include "Chunk.hpp"

class World;

class Renderer : public Camera {
	World& w;
	int ctxInfo;
	int vpWidth;
	int vpHeight;
	glm::mat4 view; // view matrix
	glm::mat4 projection;
	glm::mat4 model;

public:
	Renderer(World&);

	Renderer(const Renderer &) = delete;
	Renderer(Renderer&&) = delete;

	~Renderer();

	void resumeRendering();
	void pauseRendering();

	void loadMissingChunks();

private:
	void render();

	void getViewportSize();

	bool setupView();
	bool setupProjection();
	bool setupRenderingContext();

	bool activateRenderingContext();
	void destroyRenderingContext();

	void startRenderLoop();
	void stopRenderLoop();

	static void doRender(void *);
};
