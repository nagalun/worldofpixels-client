#include "ChunkGlState.hpp"

#include <cstdio>

#include "util/gl/Framebuffer.hpp"

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

using LoadState = ChunkGlState::LoadState;

ChunkGlState::ChunkGlState()
: pixelTex(nullptr),
  protTex(nullptr),
  ls(LoadState::LOADING) { }

bool ChunkGlState::loadEmpty() {
	ls = LoadState::EMPTY;
	return true;
}

bool ChunkGlState::loadTextures(PngImage&& pixelData, const ChunkConstants::ProtTexture& protData) {
	const u8 * pxDataPtr = pixelData.getData();

	if (!(pixelData.getWidth() == ChunkConstants::size
			&& pixelData.getHeight() == ChunkConstants::size) && pxDataPtr) {
		std::printf("[ChunkGlState] Invalid chunk image size: (%ix%i) != (%ix%i)\n",
				pixelData.getWidth(), pixelData.getHeight(), (int)ChunkConstants::size, (int)ChunkConstants::size);

		return false;
	} else if (!pxDataPtr) {
		std::printf("[ChunkGlState] pxDataPtr == nullptr\n");

		return false;
	}

	if (ChunkConstants::pxTexNumChannels != pixelData.getChannels()) {
		// shouldn't happen, loaded image is converted
	}

	GLint fmt = pixelData.getChannels() == 4 ? GL_RGBA : GL_RGB;

	initAndUsePixelTex();
	glTexImage2D(GL_TEXTURE_2D, 0, fmt,
			ChunkConstants::size, ChunkConstants::size,
			0, fmt, GL_UNSIGNED_BYTE, pixelData.getData());

	initAndUseProtTex();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
			ChunkConstants::pc, ChunkConstants::pc,
			0, GL_RGBA, GL_UNSIGNED_BYTE, protData.data());

	// makes little sense since it's more likely that the cache won't be needed
	// textureCache = std::move(pixelData);
	ls = LoadState::TEXTURED;
	return true;
}

bool ChunkGlState::loadError() {
	ls = LoadState::ERROR;
	return true;
}

LoadState ChunkGlState::getLoadState() const {
	return ls;
}

void ChunkGlState::queueSetPixel(u16 x, u16 y, RGB_u rgba) {
	pendingPxUpdates.emplace_back(PxUpdate{x, y, rgba});

	if (textureCache.getData()) {
		textureCache.setPixel(x, y, rgba);
	}
}

void ChunkGlState::queueSetPixelWithBlending(u16 x, u16 y, RGB_u rgba) {
	if (ls != LoadState::TEXTURED && ls != LoadState::EMPTY) {
		// blending can't be done with no texture
		return;
	}

	if (!textureCache.getData()) {
		readTexToCache();
	}

	textureCache.setPixel(x, y, rgba, true);
	pendingPxUpdates.emplace_back(PxUpdate{x, y, textureCache.getPixel(x, y)});
}

void ChunkGlState::queueSetProtectionGid(u16 x, u16 y, ChunkConstants::ProtGid gid) {
	pendingProtUpdates.emplace_back(ProtUpdate{x, y, gid});
}

const gl::Texture& ChunkGlState::getPixelGlTex() const {
	return pixelTex;
}

const gl::Texture& ChunkGlState::getProtGlTex() const {
	return protTex;
}

bool ChunkGlState::freeMemory() {
	sz_t pxVecCap = pendingPxUpdates.capacity();
	sz_t protVecCap = pendingProtUpdates.capacity();

	pendingPxUpdates.shrink_to_fit();
	pendingProtUpdates.shrink_to_fit();

	if (pendingPxUpdates.capacity() < pxVecCap
			|| pendingProtUpdates.capacity() < protVecCap) {
		return true;
	}

	if (textureCache.getData()) {
		textureCache.freeMem();
		return true;
	}

	return false;
}

bool ChunkGlState::renderUpdates(ChunkUpdaterGlState& glst, bool glstActive) {
	if (!pendingPxUpdates.empty() || !pendingProtUpdates.empty()) {
		switch (ls) {
			case LoadState::ERROR:
				// Drop the updates, they're of no use
				pendingPxUpdates.clear();
				pendingProtUpdates.clear();
				return glstActive;

			case LoadState::LOADING:
				// Defer, we're still loading. These updates may have been sent after
				// encoding the PNG server side, so they're needed to be up to date
				return glstActive;

			case LoadState::EMPTY:
				// Init textures to apply the updates
				loadEmptyTextures();
				break;

			default:
				break;
		}

		if (!glstActive) {
			glstActive = true;
			// colors in the pending updates vectors are pre-blended.
			glDisable(GL_BLEND);
			glst.use();
		}
	}

	if (!pendingPxUpdates.empty()) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pixelTex.get(), 0);
//		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
//			std::printf("Framebuffer not complete\n");
//		}

		glViewport(0, 0, ChunkConstants::size, ChunkConstants::size);
		glst.uploadPxData(pendingPxUpdates);
		glDrawArraysInstancedANGLE(GL_TRIANGLES, 0, 6, pendingPxUpdates.size());
		pendingPxUpdates.clear();
	}

	if (!pendingProtUpdates.empty()) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, protTex.get(), 0);
		glViewport(0, 0, ChunkConstants::pc, ChunkConstants::pc);
		glst.uploadProtData(pendingProtUpdates);
		glDrawArraysInstancedANGLE(GL_TRIANGLES, 0, 6, pendingProtUpdates.size());
		pendingProtUpdates.clear();
	}

	return glstActive;
}

void ChunkGlState::initAndUsePixelTex() {
	pixelTex = gl::Texture{};
	pixelTex.use(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void ChunkGlState::initAndUseProtTex() {
	protTex = gl::Texture{};
	protTex.use(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

RGB_u ChunkGlState::getPixel(u16 x, u16 y) const {
	if (ls != LoadState::TEXTURED && pendingPxUpdates.size() == 0) {
		return {{0, 0, 0, 0}};
	}

	if (!textureCache.getData()) {
		readTexToCache();
	}

	return textureCache.getPixel(x, y);
}

void ChunkGlState::loadEmptyTextures() {
	initAndUsePixelTex();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
			ChunkConstants::size, ChunkConstants::size,
			0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	initAndUseProtTex();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
			ChunkConstants::pc, ChunkConstants::pc,
			0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	ls = LoadState::TEXTURED;
}

void ChunkGlState::readTexToCache() const {
	glViewport(0, 0, ChunkConstants::size, ChunkConstants::size);

	// getChannels will return the num of channels of the last texture
	GLint fmt = textureCache.getChannels() == 4 ? GL_RGBA : GL_RGB;
	textureCache.allocate(ChunkConstants::size, ChunkConstants::size, RGB_u{{0, 0, 0, 0}}, textureCache.getChannels());

	if (ls == LoadState::TEXTURED) {
		gl::Framebuffer fb; // this is probably really bad. figure out some way to get a long lived framebuffer here
		fb.use(GL_FRAMEBUFFER);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pixelTex.get(), 0);
		glReadPixels(0, 0, ChunkConstants::size, ChunkConstants::size, fmt, GL_UNSIGNED_BYTE, static_cast<void *>(textureCache.getData()));
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// catch up with updates
	for (const auto& px : pendingPxUpdates) {
		textureCache.setPixel(px.x, px.y, px.rgba);
	}
}
