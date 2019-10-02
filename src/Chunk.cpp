#include "Chunk.hpp"

#include <cstdio>

#include <emscripten.h>

#include "rle.hpp"

#include "World.hpp"

static_assert((Chunk::size & (Chunk::size - 1)) == 0,
	"Chunk::size must be a power of 2");

static_assert((Chunk::protectionAreaSize & (Chunk::protectionAreaSize - 1)) == 0,
	"Chunk::protectionAreaSize must be a power of 2");

static_assert((Chunk::size % Chunk::protectionAreaSize) == 0,
	"Chunk::size must be divisible by Chunk::protectionAreaSize");

static_assert((Chunk::pc & (Chunk::pc - 1)) == 0,
	"size / protectionAreaSize must result in a power of 2");

static void destroyWget(void * e) {
	if (e) {
		emscripten_async_wget2_abort(reinterpret_cast<int>(e));
		//std::printf("Cancelled chunk request\n");
	}
}

Chunk::Chunk(Pos x, Pos y, World& w)
: w(w),
  x(x),
  y(y),
  loaderRequest(nullptr, destroyWget),
  canUnload(false),
  protectionsLoaded(false) { // to check if I need to 0-fill the protection array
	bool readerCalled = false;

	data.setChunkReader("woPp", [this] (u8 * d, sz_t size) {
		protectionsLoaded = rle::decompress(d, size, protectionData.data(), protectionData.size());
		return true;
	});

	loaderRequest.reset(reinterpret_cast<void *>(emscripten_async_wget2_data(
			w.getChunkUrl(x, y), "GET", nullptr, this, true,
			Chunk::loadCompleted, Chunk::loadFailed, nullptr)));

	preventUnloading(false);

	std::printf("[Chunk] Created (%i, %i)\n", x, y);
}

bool Chunk::setPixel(u16 x, u16 y, RGB_u clr) {
	x &= Chunk::size - 1;
	y &= Chunk::size - 1;

	if (data.getPixel(x, y).rgb != clr.rgb) {
		data.setPixel(x, y, clr);
		// TODO: send setPixel packet
		return true;
	}

	return false;
}

void Chunk::setProtectionGid(ProtPos x, ProtPos y, u32 gid) {
	x &= Chunk::pc - 1;
	y &= Chunk::pc - 1;

	protectionData[y * Chunk::pc + x] = gid;
	// TODO: send setProtection packet
}

u32 Chunk::getProtectionGid(ProtPos x, ProtPos y) const {
	x &= Chunk::pc - 1;
	y &= Chunk::pc - 1;

	return protectionData[y * Chunk::pc + x];
}

const u8 * Chunk::getData() const {
	return data.getData();
}

bool Chunk::isReady() const {
	// if no request pending, it means the chunk has been loaded (or failed to)
	return !loaderRequest;
}

bool Chunk::shouldUnload() const {
	return canUnload;
}

void Chunk::preventUnloading(bool state) {
	canUnload = !state;
}

Chunk::Key Chunk::key(Chunk::Pos x, Chunk::Pos y) {
	union {
		struct {
			Chunk::Pos x;
			Chunk::Pos y;
		};
		Chunk::Key xy;
	} k = {x, y};
	return k.xy;
}

void Chunk::loadCompleted(unsigned, void * e, void * buf, unsigned len) {
	Chunk& c = *static_cast<Chunk *>(e);
	if (len) {
		c.data.readFileOnMem(static_cast<u8 *>(buf), len);
	} else { // 204
		c.data.allocate(Chunk::size, Chunk::size, c.w.getBackgroundColor());
	}

	if (!c.protectionsLoaded) {
		c.protectionData.fill(0);
	}

	c.preventUnloading(false);
	c.loaderRequest = nullptr;

	c.w.signalChunkLoaded(c);

	std::printf("[Chunk] Loaded (%i, %i)\n", c.x, c.y);
}

void Chunk::loadFailed(unsigned, void * e, int code, const char * err) {
	Chunk& c = *static_cast<Chunk *>(e);
	// what do?
	//c.data.allocate(Chunk::size, Chunk::size, c.w.getBackgroundColor());
	c.preventUnloading(false);
	c.protectionData.fill(0);

	std::printf("[Chunk] Load failed (%i, %i): %s\n", c.x, c.y, err);

	c.loaderRequest = nullptr;
}
