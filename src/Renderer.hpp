#pragma once

#include <utility>
#include <vector>
#include <string_view>
#include <optional>

#include <glm/ext/matrix_float4x4.hpp>

#include <util/emsc/gl/WebGlContext.hpp>
#include <util/explints.hpp>

#include <Camera.hpp>
#include <world/Chunk.hpp>
#include <gl/BackgroundGlState.hpp>
#include <gl/ChunkRendererGlState.hpp>
#include <gl/ChunkUpdaterGlState.hpp>

class World;

class Renderer : public Camera {
public:
	static constexpr sz_t vramMaxLimit = 512 * 1000 * 1000; // 512 MB
	static constexpr sz_t maxLoadedChunks = vramMaxLimit / (
			Chunk::size * Chunk::size * Chunk::pxTexNumChannels
			+ Chunk::pc * Chunk::pc * sizeof(Chunk::ProtGid));

private:
	World& w;
	gl::WebGlContext ctx;
	std::optional<ChunkRendererGlState> cRendererGl;
	std::optional<ChunkUpdaterGlState> cUpdaterGl;
	glm::mat4 view; // view matrix
	glm::mat4 projection;
	float lastRenderTime;
	bool fixViewportOnNextFrame;
	u8 contextFailureCount;
	u16 frameNum;

	std::vector<Chunk *> chunksToUpdate;

public:
	Renderer(World&);

	Renderer(const Renderer &) = delete;
	Renderer(Renderer&&) = delete;

	~Renderer();

	sz_t getMaxVisibleChunks() const;
	const gl::GlContext& getGlContext() const;

	void loadMissingChunks();
	bool isChunkVisible(Chunk::Pos x, Chunk::Pos y) const;
	bool isChunkVisible(const Chunk&) const;
	void chunkToUpdate(Chunk *);
	void chunkUnloaded(Chunk *);
	void setFixViewportOnNextFrame();

	void setPos(float, float) override;
	void setZoom(float) override;
	void translate(float, float) override;

private:
	void render();

	bool setupView();
	bool setupProjection();
	bool setupRenderingContext();
	bool setupRenderingCallbacks();

	bool resizeRenderingContext();
	void destroyGlState();
	bool resetGlState();

	void delayedGlReset();

	static void doRender(void *);
	static void doDelayedGlReset(void *);
};
