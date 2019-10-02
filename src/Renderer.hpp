#pragma once

#include <utility>
#include <vector>

#include "Camera.hpp"
#include "Chunk.hpp"

class World;

class Renderer : public Camera {
	World& w;
	int ctxInfo;
	int vpWidth;
	int vpHeight;

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

	bool activateRenderingContext();
	void destroyRenderingContext();
	void startRenderLoop();
	void stopRenderLoop();

	static void doRender(void *);
	static int emEvent(int, const void *, void *);
};
