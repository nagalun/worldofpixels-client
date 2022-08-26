#include <world/World.hpp>

#include <array>
#include <algorithm>
#include <cstdio>
#include <cmath>

#include <InputManager.hpp>
#include <Camera.hpp>

#include <util/gl/GlContext.hpp>
#include <util/emsc/dom.hpp>
#include <util/byteswap.hpp>
#include <util/explints.hpp>
#include <util/misc.hpp>

// "'#89ABCDEF'"
constexpr std::size_t bufSz = 11;

World::World(InputAdapter& base, std::string name, std::unique_ptr<SelfCursor::Builder> me,
		RGB_u bgClr, bool restricted, std::optional<User::Id> owner)
: name(std::move(name)),
  bgClr(bgClr),
  r(*this),
  chunkLoaderQueue({std::pair<Chunk::Pos, Chunk::Pos>{0, 0}}),
  owner(std::move(owner)),
  me(me->build(*this)),
  aWorld(base.mkAdapter("World")),
  toolMan(*this, aWorld),
  toolWin(toolMan),
  posUi(this->me.getX(), this->me.getY(), r.getZoom()),
  iMoveCursor(aWorld, "Move cursor", T_ONMOVE),
  iPrintCoords(aWorld, "Print Coordinates"),
  iRoundCoords(aWorld, "Round Coordinates"),
  tickNum(0),
  drawingRestricted(restricted) {

	u32 cssBgClr = bswap_32(bgClr.rgb);
	eui_root_css_property_set("--bg-clr", svprintf<bufSz>("#%08X", cssBgClr));

	iMoveCursor.setCb([this] (ImAction::Event& ev, const InputInfo& ii) {
		recalculateCursorPosition(ii);
	});

	iPrintCoords.setDefaultKeybind("P");
	iPrintCoords.setCb([this] (auto&, const auto&) {
		std::printf("[World] Coords: %f, %f\n", r.getX(), r.getY());
	});

	iRoundCoords.setDefaultKeybind("O");
	iRoundCoords.setCb([this] (auto&, const auto&) {
		r.setPos(std::round(r.getX()), std::round(r.getY()));
	});

	std::puts("[World] Created");

	//loadMissingChunksTick();
}

void World::setCursorCount(u32 worldCount, u32 globalCount) {
	pCntUi.setCounts(worldCount, globalCount);
}

void World::tick() {
	++tickNum;

	if (!(tickNum % (20 * 10))) {
		sz_t n = unloadFarChunks();
		if (n > 0) {
			std::printf("[World] Unloaded %lu chunks.\n", n);
		}
	}
}

sz_t World::unloadChunks(sz_t targetAmount) { /* pretty bad worst case perf, optimize! */
	sz_t origAmount = targetAmount;
	bool doingWork = false;
	// allocating memory may not be safe right now
	std::array<decltype(chunks)::const_iterator, 8> toUnload;

	do {
		toUnload.fill(chunks.cend());

		for (auto it = chunks.cbegin(); it != chunks.cend(); ++it) {
			if (it->second.shouldUnload() && !r.isChunkVisible(it->second)) {
				if (toUnload[0] != chunks.cend()) {
					float distTop = getDistanceToChunk(toUnload[0]->second);
					float distCur = getDistanceToChunk(it->second);
					if (distCur < distTop) {
						continue;
					}
				}

				std::move_backward(toUnload.begin(), toUnload.end() - 1, toUnload.end());
				toUnload[0] = it;
			}
		}

		doingWork = false;
		for (auto it : toUnload) {
			if (targetAmount == 0) {
				break;
			}

			if (it != chunks.cend()) {
				doingWork = true;
				chunks.erase(it);
				--targetAmount;
			}
		}
	} while (targetAmount != 0 && doingWork);

	return origAmount - targetAmount;
}

sz_t World::unloadNonVisibleNonReadyChunks() {
	sz_t unloaded = 0;

	for (auto it = chunks.cbegin(); it != chunks.cend(); ) {
		const Chunk& c = it->second;

		if (c.shouldUnload() && !r.isChunkVisible(c, 256.f) && !c.isReady()) {
			it = chunks.erase(it);
			unloaded++;
		} else {
			++it;
		}
	}

	return unloaded;
}

sz_t World::unloadFarChunks() {
	sz_t unloaded = 0;

	for (auto it = chunks.cbegin(); it != chunks.cend(); ) {
		const Chunk& c = it->second;

		if (c.shouldUnload() && !r.isChunkVisible(c, 256.f) && (!c.isReady() || getDistanceToChunk(c) > 20.f)) {
			it = chunks.erase(it);
			unloaded++;
		} else {
			++it;
		}
	}

	return unloaded;
}

sz_t World::unloadAllChunks() {
	auto s = chunks.size();
	chunks.clear();
	return s;
}

bool World::freeMemory(bool tryHarder) {
	if (unloadFarChunks()) {
		return true;
	}

	for (auto it = chunks.begin(); it != chunks.end(); ++it) {
		if (it->second.shouldUnload() // we could be trying to set a pixel on that chunk
				&& it->second.getGlState().freeMemory()) {
			return true;
		}
	}

	if (unloadChunks()) {
		return true;
	}

	if (tryHarder) {
		for (auto it = chunks.begin(); it != chunks.end(); ++it) {
			std::printf("[World] Chunk %ld (%d, %d) %c %c\n", std::distance(chunks.begin(), it), it->second.getX(), it->second.getY(), it->second.isReady() ? '1' : '0', it->second.shouldUnload() ? '1' : '0');
		}
	}

	return false;
}

sz_t World::getMaxLoadedChunks() const {
	sz_t mv = r.getMaxVisibleChunks();
	return std::max(std::min(mv * 8, 128ul), mv);
}

Camera& World::getCamera() {
	return r;
}

const Camera& World::getCamera() const {
	return r;
}

SelfCursor& World::getCursor() {
	return me;
}

const SelfCursor& World::getCursor() const {
	return me;
}

const std::unordered_map<Chunk::Key, Chunk>& World::getChunkMap() const {
	return chunks;
}

Chunk& World::getOrLoadChunk(Chunk::Pos x, Chunk::Pos y) {
	auto it = chunks.try_emplace(Chunk::key(x, y), x, y, *this);
	if (it.second) { // if a chunk was emplaced
		r.queueRerender();
		unloadNonVisibleNonReadyChunks();

		sz_t ml = getMaxLoadedChunks();
		if (chunks.size() >= Renderer::maxLoadedChunks) {
			it.first->second.preventUnloading(true);
			if (!unloadChunks(chunks.size() - Renderer::maxLoadedChunks + 1)) {
				std::printf("[World] Can't keep loaded chunks (%ld) under limit (%ld)\n", chunks.size(), ml);
			}

			it.first->second.preventUnloading(false);
		}
	}

	return it.first->second;
}

const std::string& World::getName() const {
	return name;
}

RGB_u World::getBackgroundColor() const {
	return bgClr;
}

Renderer& World::getRenderer() {
	return r;
}

const char * World::getChunkUrl(Chunk::Pos x, Chunk::Pos y) {
	// /api/worlds/<name>/view/<x>/<y>
	// i32 = -2147483648 (11 chars)
	static char urlBuf[12 + World::maxNameLength + 6 + 11 + 1 + 11 + 1] = {0};
	std::sprintf(urlBuf, "/api/worlds/%s/view/%i/%i", name.c_str(), x, y);
	return urlBuf;
}

void World::signalChunkUpdated(Chunk * c) {
	r.chunkToUpdate(c);
}

void World::signalChunkUnloaded(Chunk * c) {
	r.chunkUnloaded(c);
}

void World::loadMissingChunksTick() {
	if (chunkLoaderQueue.empty()) {
		std::puts("Chunkloaderqueue empty");
		i32 cx = std::floor(r.getX() / Chunk::size);
		i32 cy = std::floor(r.getY() / Chunk::size);
		chunkLoaderQueue.emplace(cx, cy);
	}

	const auto queueIfUnloaded = [this] (Chunk::Pos x, Chunk::Pos y) {
		/*if (!r.isChunkVisible(x, y)) {
			return;
		}*/

		auto search = chunks.find(Chunk::key(x, y));
		if (search == chunks.end()) {
			chunkLoaderQueue.emplace(x, y);
		}
	};

	auto [x, y] = chunkLoaderQueue.front();
	chunkLoaderQueue.pop();

	auto search = chunks.find(Chunk::key(x, y));
	if (search == chunks.end()) {
		getOrLoadChunk(x, y);
		queueIfUnloaded(x - 1, y);
		queueIfUnloaded(x + 1, y);
		queueIfUnloaded(x, y - 1);
		queueIfUnloaded(x, y + 1);
	}
}

float World::getDistanceToChunk(const Chunk& c) const {
	float dx = std::abs(r.getX() / Chunk::size - c.getX());
	float dy = std::abs(r.getY() / Chunk::size - c.getY());

	return dx + dy;
}

Chunk * World::getChunkAt(World::Pos x, World::Pos y) {
	auto it = chunks.find(Chunk::key(x >> Chunk::posShift, y >> Chunk::posShift));
	if (it != chunks.end()) {
		return &it->second;
	}

	return nullptr;
}

const Chunk * World::getChunkAt(World::Pos x, World::Pos y) const {
	auto it = chunks.find(Chunk::key(x >> Chunk::posShift, y >> Chunk::posShift));
	if (it != chunks.end()) {
		return &it->second;
	}

	return nullptr;
}

RGB_u World::getPixel(World::Pos x, World::Pos y) const {
	const Chunk * c = getChunkAt(x, y);

	if (c) {
		return c->getPixel(x, y);
	}

	return {{0, 0, 0, 0}};
}

bool World::setPixel(World::Pos x, World::Pos y, RGB_u clr, bool alphaBlending) {
	Chunk * c = getChunkAt(x, y);

	if (c) {
		return c->setPixel(x, y, clr, alphaBlending);
	}

	return false;
}

void World::updateUi() {
	posUi.paint();
	pCntUi.paint();
}

void World::recalculateCursorPosition() {
	recalculateCursorPosition(aWorld.getInputManager().getLastInputInfo());
}

void World::recalculateCursorPosition(const InputInfo& ii) {
	if (ii.getNumActivePointers() == 0) {
		return;
	}

	float wx, wy;
	float mx = ii.getMidX();
	float my = ii.getMidY();
	r.getWorldPosFromScreenPos(mx, my, &wx, &wy);
	getCursor().setPos(wx, wy);

	const Camera& cam = getCamera();
	posUi.setPos(std::floor(wx), std::floor(wy), cam.getX(), cam.getY(), cam.getZoom());
}
