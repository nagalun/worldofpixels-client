#pragma once

#include <utility>
#include <vector>
#include <string_view>
#include <optional>

#include <glm/ext/matrix_float4x4.hpp>

#include "Settings.hpp"
#include "util/emsc/gl/WebGlContext.hpp"
#include "util/explints.hpp"
#include "util/NonCopyable.hpp"

#include "Camera.hpp"
#include "world/Chunk.hpp"
#include "gl/ChunkRendererGlState.hpp"
#include "gl/ChunkUpdaterGlState.hpp"

class World;

class Renderer : public Camera, NonCopyable {
public:
	static constexpr sz_t vramMaxLimit = 512 * 1000 * 1000; // 512 MB
	static constexpr sz_t maxLoadedChunks = vramMaxLimit / (
			Chunk::size * Chunk::size * Chunk::pxTexNumChannels
			+ Chunk::pc * Chunk::pc * sizeof(Chunk::ProtGid));

private:
	enum RenderType : u8 {
		R_NONE = 0,
		R_WORLD = 1,
		R_UI = 2
	};

	World& w;
	gl::WebGlContext ctx;
	std::optional<ChunkRendererGlState> cRendererGl;
	std::optional<ChunkUpdaterGlState> cUpdaterGl;
	glm::mat4 view; // view matrix
	glm::mat4 projection;
	decltype(Settings::showGrid)::SlotKey skShowGridCh;
	float lastRenderTime;
	u8 pendingRenderType;
	u8 contextFailureCount;
	u16 frameNum;

	std::vector<Chunk *> chunksToUpdate;

public:
	Renderer(World&);
	~Renderer();

	sz_t getMaxVisibleChunks() const;
	const gl::GlContext& getGlContext() const;

	void loadMissingChunks();
	bool isChunkVisible(Chunk::Pos x, Chunk::Pos y, float extraPxMargin = 0.f) const;
	bool isChunkVisible(const Chunk&, float extraPxMargin = 0.f) const;
	void chunkToUpdate(Chunk *);
	void chunkUnloaded(Chunk *);
	void queueUiUpdate();
	void queueRerender();
	static void queueUiUpdateSt();
	static void queueRerenderSt();

	double getScreenDpr() const override;
	void getScreenSize(double *w, double *h) const override;
	void setPos(float, float) override;
	void setZoom(float) override;
	void setZoom(float z, float ox, float oy) override;
	void translate(float, float) override;
	void setMomentum(float, float) override;

private:
	void recalculateCursorPosition() const override;

	void render();
	u8 preRenderUpdates(float now, float dt);
	bool renderWorld(float now, float dt);
	bool renderUi(float now, float dt);

	bool setupView();
	bool setupProjection();
	bool setupRenderingContext();
	bool setupRenderingCallbacks();

	bool resizeRenderingContext();
	void destroyGlState();
	bool resetGlState(bool unloadChunks = true);

	void delayedGlReset();

	static void doRender(void *);
	static void doDelayedGlReset(void *);
};
