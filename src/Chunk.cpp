#include "Chunk.hpp"

#include <emscripten/fetch.h>

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

static void destroyFetch(emscripten_fetch_t * e) {
	emscripten_fetch_close(e);
}

Chunk::Chunk(Pos x, Pos y, World& w)
: w(w),
  x(x),
  y(y),
  loaderRequest(nullptr, destroyFetch),
  canUnload(false),
  protectionsLoaded(false) { // to check if I need to 0-fill the protection array
	bool readerCalled = false;

	data.setChunkReader("woPp", [this] (u8 * d, sz_t size) {
		protectionsLoaded = rle::decompress(d, size, protectionData.data(), protectionData.size());
		return true;
	});

	emscripten_fetch_attr_t attr;
	emscripten_fetch_attr_init(&attr);
	strcpy(attr.requestMethod, "GET");

	attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
	attr.onsuccess = Chunk::loadCompleted;
	attr.onerror = Chunk::loadFailed;
	attr.userData = this;

	loaderRequest.reset(emscripten_fetch(&attr, w.getChunkUrl(x, y)));

	preventUnloading(false);
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

void Chunk::loadCompleted(emscripten_fetch_t * e) {
	Chunk& c = *static_cast<Chunk *>(e->userData);
	if (e->status == 200) {
		const u8 * buf = reinterpret_cast<const u8 *>(e->data);
		// disgusting const cast
		c.data.readFileOnMem(const_cast<u8 *>(buf), e->numBytes);
	} else { // 204
		c.data.allocate(Chunk::size, Chunk::size, c.w.getBackgroundColor());
	}

	if (!c.protectionsLoaded) {
		c.protectionData.fill(0);
	}

	c.preventUnloading(false);
	c.loaderRequest = nullptr;

	c.w.signalChunkLoaded(c);
}

void Chunk::loadFailed(emscripten_fetch_t * e) {
	Chunk& c = *static_cast<Chunk *>(e->userData);
	// what do?
	//c.data.allocate(Chunk::size, Chunk::size, c.w.getBackgroundColor());
	c.preventUnloading(false);
	c.protectionData.fill(0);
	c.loaderRequest = nullptr;
}
