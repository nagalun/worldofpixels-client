#pragma once

#include <vector>

#include <util/PngImage.hpp>
#include <util/gl/Framebuffer.hpp>
#include <util/gl/Texture.hpp>
#include <util/color.hpp>

#include <world/ChunkConstants.hpp>
#include <gl/ChunkUpdaterGlState.hpp>

class Renderer;

class ChunkGlState {
public:
	enum class LoadState {
		UNLOADED,
		LOADING,
		EMPTY,
		TEXTURED,
		ERROR
	};

	using PxUpdate = ChunkUpdaterGlState::PxUpdate;
	using ProtUpdate = ChunkUpdaterGlState::ProtUpdate;

private:
	gl::Texture pixelTex;
	gl::Texture protTex;

	// pixel and protection updates to be applied the next frame
	std::vector<PxUpdate> pendingPxUpdates;
	std::vector<ProtUpdate> pendingProtUpdates;

	mutable PngImage textureCache;
	LoadState ls;

public:
	ChunkGlState();

	bool loadEmpty();
	bool loadTextures(PngImage&&, const ChunkConstants::ProtTexture&);
	bool loadError();

	// Tries to free ram (not vram)
	bool freeMemory();

	LoadState getLoadState() const;
	const gl::Texture& getPixelGlTex() const;
	const gl::Texture& getProtGlTex() const;

	RGB_u getPixel(u16 x, u16 y) const;
	void queueSetPixel(u16 x, u16 y, RGB_u rgba);
	void queueSetPixelWithBlending(u16 x, u16 y, RGB_u rgba);
	void queueSetProtectionGid(u16 x, u16 y, ChunkConstants::ProtGid gid);

	/* returns true if the gl state is activated, else glstActive */
	bool renderUpdates(ChunkUpdaterGlState&, bool glstActive);

private:
	void initAndUsePixelTex();
	void initAndUseProtTex();
	void loadEmptyTextures();

	void readTexToCache() const;
};
