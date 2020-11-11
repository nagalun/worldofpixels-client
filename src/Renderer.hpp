#pragma once

#include <util/explints.hpp>
#include <world/Chunk.hpp>
#include <utility>
#include <vector>
#include <string_view>

#include <glm/ext/matrix_float4x4.hpp>

#include <Camera.hpp>

class World;

class Renderer : public Camera {
	World& w;
	int ctxInfo;
	int vpWidth;
	int vpHeight;
	glm::mat4 view; // view matrix
	glm::mat4 projection;
	u32 chunkProgram;
	u32 tbuf;
	u32 chunkUnifView;
	u32 chunkUnifProj;
	u32 chunkUnifZoom;
	u32 chunkUnifChunkSize;
	u32 chunkUnifTex;
	u32 chunkUnifOffset;
	u32 fxProgram;
	u32 cursorsProgram;

public:
	Renderer(World&);

	Renderer(const Renderer &) = delete;
	Renderer(Renderer&&) = delete;

	~Renderer();

	sz_t getMaxVisibleChunks() const;

	void resumeRendering();
	void pauseRendering();

	void loadMissingChunks();
	bool isChunkVisible(Chunk::Pos x, Chunk::Pos y) const;
	bool isChunkVisible(const Chunk&) const;
	void useChunk(Chunk&);
	void unuseChunk(Chunk&);

	void setPos(float, float);
	void setZoom(float);
	void translate(float, float);

private:
	void render();

	void updateUnifViewMatrix();
	void updateUnifProjMatrix();
	void updateUnifZoom();
	void getViewportSize();

	u32 buildProgram(std::string_view vertexShader, std::string_view fragmentShader);

	bool setupView();
	bool setupProjection();
	bool setupShaders();
	bool setupRenderingContext();

	bool resizeCanvas(int w, int h);
	bool resizeRenderingContext();
	bool activateRenderingContext(bool webgl1 = false);
	void destroyRenderingContext();

	void startRenderLoop();
	void stopRenderLoop();

	static void doRender(void *);
};
