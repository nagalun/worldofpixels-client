#include "Renderer.hpp"

#include <algorithm>
#include <exception>
#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <util/explints.hpp>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <Client.hpp>
#include <world/World.hpp>
#include <gl/data/ChunkShader.hpp>

Renderer::Renderer(World& w)
: w(w),
  ctx("#world"),
  view(1.0f),
  projection(1.0f),
  lastRenderTime(ctx.getTime() / 1000.f),
  fixViewportOnNextFrame(false),
  contextFailureCount(0),
  frameNum(0) {
	if (!ctx.ok()) {
		std::printf("[Renderer] ctx.ok() == false\n");
		Client::setStatus(R"(
			<p>Failed to init WebGL.</p>
			<p>No GPU acceleration seems to be available.</p>
			<p>Try reloading the page or updating video drivers.</p>
		)");
		std::exit(1);
	}

	resetGlState();
	setupRenderingCallbacks();
	ctx.startRenderLoop(Renderer::doRender, this);

	std::printf("[Renderer] Initialized\n");
}

Renderer::~Renderer() {
	std::printf("[Renderer] Destroyed\n");
}

sz_t Renderer::getMaxVisibleChunks() const {
	auto s = ctx.getSize();
	return (s.w / Chunk::size + 2) * (s.h / Chunk::size + 2);
}

const gl::GlContext& Renderer::getGlContext() const {
	return ctx;
}

void Renderer::loadMissingChunks() {
	auto s = ctx.getSize();

	float hVpWidth = s.w / 2.f / getZoom();
	float hVpHeight = s.h / 2.f / getZoom();
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

bool Renderer::isChunkVisible(Chunk::Pos px, Chunk::Pos py) const {
	auto s = ctx.getSize();

	float x = static_cast<float>(px) * Chunk::size;
	float y = static_cast<float>(py) * Chunk::size;
	float czoom = getZoom();
	float tlcx = getX() - static_cast<float>(s.w) / 2.f / czoom;
	float tlcy = getY() - static_cast<float>(s.h) / 2.f / czoom;

	return x + Chunk::size > tlcx && y + Chunk::size > tlcy &&
	       x <= tlcx + s.w / czoom && y <= tlcy + s.h / czoom;
}

bool Renderer::isChunkVisible(const Chunk& c) const {
	return isChunkVisible(c.getX(), c.getY());
}


void Renderer::chunkToUpdate(Chunk * c) {
	if (std::find(chunksToUpdate.begin(), chunksToUpdate.end(), c) == chunksToUpdate.end()) {
		chunksToUpdate.emplace_back(c);
	}

	ctx.resumeRendering();
}

void Renderer::chunkUnloaded(Chunk * c) {
	auto it = std::find(chunksToUpdate.begin(), chunksToUpdate.end(), c);
	if (it != chunksToUpdate.end()) {
		chunksToUpdate.erase(it);
	}

	ctx.resumeRendering();
}

void Renderer::setFixViewportOnNextFrame() {
	fixViewportOnNextFrame = true;
}

void Renderer::setPos(float x, float y) {
	Camera::setPos(x, y);
	setupView();
}

void Renderer::setZoom(float z) {
	Camera::setZoom(z);
	setupProjection();
	std::printf("[Renderer] Zoom: %f\n", getZoom());
}

void Renderer::translate(float dx, float dy) {
	setPos(getX() + dx, getY() + dy);
}

void Renderer::render() {
	using LoadState = ChunkGlState::LoadState;

	auto s = ctx.getSize();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float now = ctx.getTime() / 1000.f;
	++frameNum;

	float czoom = getZoom();
	float hVpWidth = s.w / 2.f / czoom;
	float hVpHeight = s.h / 2.f / czoom;
	float tlx = std::floor((getX() - hVpWidth) / Chunk::size);
	float tly = std::floor((getY() - hVpHeight) / Chunk::size);
	float brx = std::floor((getX() + hVpWidth) / Chunk::size);
	float bry = std::floor((getY() + hVpHeight) / Chunk::size);

	bool shouldKeepRendering = false;

//	for (int i = -32; i < 32; i++) {
//		for (int j = -32; j < 32; j++) {
//			RGB_u clr = {{0, 0, 0, 0}};
//			clr.rgb = u32(std::sqrt(std::pow(j, 2) + std::pow(i, 2)) * frameNum);
//			clr.c.b = clr.c.r;
//			clr.c.r = 0;
//			clr.c.g *= 0.4;
//			clr.c.a = 255;
//			w.setPixel(j, i, clr);
//		}
//	}

	bool glstActive = false;
	for (auto ch : chunksToUpdate) {
		ChunkGlState& cgl = ch->getGlState();
		glstActive |= cgl.renderUpdates(*cUpdaterGl, glstActive);
	}

	chunksToUpdate.clear();

	if (glstActive) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, s.w, s.h);
	} else if (fixViewportOnNextFrame) {
		glViewport(0, 0, s.w, s.h);
		fixViewportOnNextFrame = false;
	}
	
	cRendererGl->use();

	TexturedChunkProgram& tcp = cRendererGl->getTexChunkProg();
	EmptyChunkProgram& ecp = cRendererGl->getEmptyChunkProg();
	LoadingChunkProgram& lcp = cRendererGl->getLoadChunkProg();
	LoadState progInUse = LoadState::ERROR;

	const auto& chunks = w.getChunkMap();
	sz_t loadCount = 0;
	for (; tly <= bry; tly += 1.f) {
		for (float tlx2 = tlx; tlx2 <= brx; tlx2 += 1.f) {
			auto search = chunks.find(Chunk::key(tlx2, tly));

			if (search == chunks.end()) {
				if (++loadCount < 6) {
					w.getOrLoadChunk(tlx2, tly);
				}

				continue;
			} else if (!search->second.isReady()) {
				++loadCount;
			}

			const ChunkGlState& glst = search->second.getGlState();
			switch (glst.getLoadState()) {
				case LoadState::TEXTURED:
					if (progInUse != LoadState::TEXTURED) {
						tcp.use();
						tcp.setUZoom(getZoom());
						tcp.setUMats(projection, view);
						progInUse = LoadState::TEXTURED;
					}

					glActiveTexture(GL_TEXTURE1);
					glst.getProtGlTex().use(GL_TEXTURE_2D);
					glActiveTexture(GL_TEXTURE0);
					glst.getPixelGlTex().use(GL_TEXTURE_2D);

					tcp.setUOffset({tlx2 * Chunk::size, tly * Chunk::size});
					glDrawArrays(GL_TRIANGLES, 0, cRendererGl->vertexCount());
					break;

				case LoadState::EMPTY:
					if (progInUse != LoadState::EMPTY) {
						ecp.use();
						ecp.setUZoom(getZoom());
						ecp.setUMats(projection, view);
						progInUse = LoadState::EMPTY;
					}

					ecp.setUOffset({tlx2 * Chunk::size, tly * Chunk::size});
					glDrawArrays(GL_TRIANGLES, 0, cRendererGl->vertexCount());
					break;

				case LoadState::ERROR:
				case LoadState::LOADING:
					if (progInUse != LoadState::LOADING) {
						lcp.use();
						lcp.setUZoom(getZoom());
						lcp.setUMats(projection, view);
						lcp.setUTime(now);
						progInUse = LoadState::LOADING;
					}

					lcp.setUOffset({tlx2 * Chunk::size, tly * Chunk::size});
					glDrawArrays(GL_TRIANGLES, 0, cRendererGl->vertexCount());
					shouldKeepRendering = true; // keep rendering animation
					break;

				default:
					break;
			}
		}
	}

	if (!shouldKeepRendering) {
		ctx.pauseRendering();
	}

	lastRenderTime = now;
}

bool Renderer::setupView() {
	view = glm::translate(glm::mat4(1.0f), glm::vec3(-getX(), -getY(), 0.f));

	//view = glm::lookAt(eye, center, up);
	ctx.resumeRendering();
	return true;
}

bool Renderer::setupProjection() {
	auto s = ctx.getSize();
	float vpHalfW = static_cast<float>(s.w) / 2.0f;
	float vpHalfH = static_cast<float>(s.h) / 2.0f;

	float vpLeft = -std::floor(vpHalfW) / getZoom();
	float vpRight = std::ceil(vpHalfW) / getZoom();
	float vpBottom = std::floor(vpHalfH) / getZoom();
	float vpTop = -std::ceil(vpHalfH) / getZoom();

	// left, right, bottom, top, near, far
	projection = glm::ortho(
			vpLeft, vpRight, vpBottom, vpTop,
			0.5f, 1.5f);

	projection = glm::scale(projection, glm::vec3(1.f, 1.f, -1.f));

	//std::puts(glm::to_string(projection).c_str());
	ctx.resumeRendering();

	return true;
}

bool Renderer::resizeRenderingContext() {
	auto s = ctx.getSize();

	glViewport(0, 0, s.w, s.h);
	setupProjection();

	std::printf("[Renderer] Viewport size: %ix%i (Chunks: %zu)\n", s.w, s.h, getMaxVisibleChunks());
	return true;
}

bool Renderer::setupRenderingContext() {
	resizeRenderingContext();

	RGB_u bgClr = w.getBackgroundColor();
	glClearColor(bgClr.c.r / 255.f, bgClr.c.g / 255.f, bgClr.c.b / 255.f, 0.5f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	setupView();

	return true;
}

bool Renderer::setupRenderingCallbacks() {
	ctx.onLost([this] {
		destroyGlState();
		ctx.startRenderLoop(Renderer::doDelayedGlReset, this);
	});

//	ctx.onRestored([this] {
//		resetGlState();
//		ctx.startRenderLoop(Renderer::doRender, this);
//	});

	ctx.onResize([this] {
		resizeRenderingContext();
	});

	return true;
}

void Renderer::destroyGlState() {
	cRendererGl = std::nullopt;
	cUpdaterGl = std::nullopt;
	w.unloadAllChunks();
}

bool Renderer::resetGlState() {
	w.unloadAllChunks();
	cRendererGl = ChunkRendererGlState{};
	cUpdaterGl = ChunkUpdaterGlState{};

	setupRenderingContext();

	return true;
}

void Renderer::delayedGlReset() {
	if (!ctx.ok()) {
		if (ctx.getTime() / 1000.f - lastRenderTime > 1.f && !ctx.activateRenderingContext(contextFailureCount > 4)) {
			std::printf("[Renderer] Couldn't recreate the context. (%d)\n", contextFailureCount);
			lastRenderTime = ctx.getTime() / 1000.f;
			if (++contextFailureCount >= 8) {
				std::printf("[Renderer] Giving up after 8 tries.");
				ctx.stopRenderLoop();
				ctx.giveUp();
			}
		}

		return;
	}

	if (!resetGlState()) {
		std::printf("[Renderer] Couldn't reset the WebGL state.\n");
		return;
	}

	ctx.startRenderLoop(Renderer::doRender, this);
}

void Renderer::doRender(void * r) {
	static_cast<Renderer *>(r)->render();
}

void Renderer::doDelayedGlReset(void * r) {
	static_cast<Renderer *>(r)->delayedGlReset();
}
