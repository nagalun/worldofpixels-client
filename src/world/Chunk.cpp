#include "Chunk.hpp"

#include <cstdio>
#include <cmath>

#include <util/BufferHelper.hpp>
#include <util/rle.hpp>
#include <world/World.hpp>
#include <Camera.hpp>

#include <util/emsc/request.hpp>
#include <util/explints.hpp>

static void destroyWget(void * e) {
	cancel_async_request(reinterpret_cast<int>(e) - 1);
}

Chunk::Chunk(Pos x, Pos y, World& w)
: w(w),
  x(x),
  y(y),
  loaderRequest(nullptr, destroyWget),
  canUnload(false),
  downscaling(std::min(std::max(1.f, std::floor(1.f / w.getCamera().getZoom())), 16.f)) {

	// terrible use of unique_ptr. 1 + needed because request id can be 0
	loaderRequest.reset(reinterpret_cast<void *>(1 + async_request(
			w.getChunkUrl(x, y), "GET", nullptr, this, true,
			Chunk::loadCompleted, Chunk::loadFailed, nullptr)));

	//std::printf("[Chunk] Created (%i, %i)\n", x, y);

	preventUnloading(false);
}

Chunk::~Chunk() {
	w.signalChunkUnloaded(this);
}

Chunk::Pos Chunk::getX() const {
	return x;
}

Chunk::Pos Chunk::getY() const {
	return y;
}

bool Chunk::setPixel(u16 pxX, u16 pxY, RGB_u clr) {
	// pushing to the vectors could allocate...
	preventUnloading(true);
	pxX &= Chunk::size - 1;
	pxY &= Chunk::size - 1;

	//std::printf("ch setpixel %i, %i\n", this->x, this->y);

	glst.queueSetPixel(pxX, pxY, clr);
	w.signalChunkUpdated(this);
	preventUnloading(false);
	return true;
}

RGB_u Chunk::getPixel(u16 pxX, u16 pxY) const {
	pxX &= Chunk::size - 1;
	pxY &= Chunk::size - 1;

	return glst.getPixel(w.getRenderer(), pxX, pxY);
}

void Chunk::setProtectionGid(ProtPos protX, ProtPos protY, u32 gid) {
	preventUnloading(true);
	protX &= Chunk::pc - 1;
	protY &= Chunk::pc - 1;

	protectionData[protY * Chunk::pc + protX] = gid;
	glst.queueSetProtectionGid(protX, protY, gid);
	w.signalChunkUpdated(this);
	preventUnloading(false);
}

u32 Chunk::getProtectionGid(ProtPos protX, ProtPos protY) const {
	protX &= Chunk::pc - 1;
	protY &= Chunk::pc - 1;

	return protectionData[protY * Chunk::pc + protX];
}

ChunkGlState& Chunk::getGlState() {
	return glst;
}

const ChunkGlState& Chunk::getGlState() const {
	return glst;
}

bool Chunk::isReady() const {
	// if no request pending, it means the chunk has been loaded (or failed to)
	return !loaderRequest;
}

bool Chunk::shouldUnload() const {
	// request aborting doesn't always work
	return canUnload;
}

void Chunk::preventUnloading(bool state) {
	canUnload = !state;
}



void Chunk::loadCompleted(unsigned, void * e, void * buf, unsigned len) {
	Chunk& c = *static_cast<Chunk *>(e);
	int loadStatus = 0;
	c.preventUnloading(true); // this is necessary because the OOM handler could be called

	// so since i can't easily check http.status, I quickly check if the file
	// received looks like a real png file.
	const u8 * filebuf = static_cast<const u8 *>(buf);
	if (len > 4 && buf::readLE<u32>(filebuf) == 0x474E5089) {
		PngImage data;
		// to check if I need to 0-fill the protection array
		bool protectionsLoaded = false;
		data.setChunkReader("woPp", [&protectionsLoaded, &c] (u8 * d, sz_t size) {
			protectionsLoaded = rle::decompress(d, size, c.protectionData.data(), c.protectionData.size());
			return true;
		});

		data.readFileOnMem(static_cast<u8 *>(buf), len);
		if (c.downscaling > 1) {
			data.nearestDownscale(c.downscaling);
		}

		if (!protectionsLoaded) {
			c.protectionData.fill(0);
		}

		if (!c.glst.loadTextures(std::move(data), c.protectionData)) {
			c.glst.loadError();
			loadStatus = 1;
		}

	} else { // 204, or other 2xx code
		c.glst.loadEmpty();
		loadStatus = 2;
	}

	c.loaderRequest = nullptr;
	c.w.signalChunkUpdated(&c);

	const char * status = "ed";
	switch (loadStatus) {
		case 1:
			status = "ing failed";
			break;
		case 2:
			status = "ed empty";
			break;
	}

	std::printf("[Chunk] Load%s (%i, %i) [%u]\n", status, c.x, c.y, c.downscaling);

	c.preventUnloading(false);
}

void Chunk::loadFailed(unsigned, void * e, int code, const char * err) {
	Chunk& c = *static_cast<Chunk *>(e);
	c.preventUnloading(true);
	c.protectionData.fill(0);
	c.glst.loadError();

	c.w.signalChunkUpdated(&c);

	c.loaderRequest = nullptr;
	std::printf("[Chunk] Load request failed (%i, %i), (%i): %s\n", c.x, c.y, code, err);

	c.preventUnloading(false);
}
