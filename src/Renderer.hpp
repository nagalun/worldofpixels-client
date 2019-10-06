#pragma once

#include <utility>
#include <vector>
#include <string_view>

#include <glm/ext/matrix_float4x4.hpp>

#include "explints.hpp"
#include "IdSys.hpp"

#include "Camera.hpp"
#include "Chunk.hpp"

class World;

class Renderer : public Camera {
	World& w;
	// texUnit 0 will be reserved for temporary gl uses
	IdSys<u32> texUnits;
	int ctxInfo;
	int vpWidth;
	int vpHeight;
	glm::mat4 view; // view matrix
	glm::mat4 projection;
	glm::mat4 projectionInv; // inverse
	u32 chunkProgram;
	u32 tbuf;
	u32 chunkUnifView;
	u32 chunkUnifProj;
	u32 chunkUnifProjInv;
	u32 chunkUnifTex;
	u32 chunkUnifOffset;
	u32 fxProgram;
	u32 cursorsProgram;

public:
	Renderer(World&);

	Renderer(const Renderer &) = delete;
	Renderer(Renderer&&) = delete;

	~Renderer();

	void resumeRendering();
	void pauseRendering();

	void loadMissingChunks();
	bool isChunkVisible(Chunk&);
	void useChunk(Chunk&);
	void unuseChunk(Chunk&);

private:
	void render();

	void updateUnifViewMatrix();
	void updateUnifProjMatrix();
	void getViewportSize();

	u32 buildProgram(std::string_view vertexShader, std::string_view fragmentShader);

	bool setupView();
	bool setupProjection();
	bool setupShaders();
	bool setupRenderingContext();

	bool activateRenderingContext();
	void destroyRenderingContext();

	void startRenderLoop();
	void stopRenderLoop();

	static void doRender(void *);
};
