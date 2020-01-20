#include "World.hpp"

#include "InputManager.hpp"

#include <cstdio>
#include <cmath>

World::World(InputAdapter& base, std::string name, std::unique_ptr<SelfCursor> me,
		RGB_u bgClr, bool restricted, std::optional<User::Id> owner)
: name(std::move(name)),
  bgClr(bgClr),
  r(*this),
  chunkLoaderQueue({std::pair<Chunk::Pos, Chunk::Pos>{0, 0}}),
  me(std::move(me)),
  owner(std::move(owner)),
  cursorCount(1),
  aWorld(base.mkAdapter("World")),
  drawingRestricted(restricted) {

	iPrintCoords = aWorld.add("Print Coordinates", {{"P"}}, T_ONPRESS, [this] (const auto&, auto, const auto&) {
		std::printf("[World] Coords: %f, %f\n", r.getX(), r.getY());
	});

	iRoundCoords = aWorld.add("Round Coordinates", {{"O"}}, T_ONPRESS, [this] (const auto&, auto, const auto&) {
		r.setPos(std::round(r.getX()), std::round(r.getY()));
	});

	iCamUp = aWorld.add("Camera ↑", {{"ARROWUP"}}, T_ONHOLD, [this] (const auto&, auto, const auto&) {
		r.translate(0.f, -3.f / (r.getZoom() / 16.f));
	});

	iCamDown = aWorld.add("Camera ↓", {{"ARROWDOWN"}}, T_ONHOLD, [this] (const auto&, auto, const auto&) {
		r.translate(0.f, 3.f / (r.getZoom() / 16.f));
	});

	iCamLeft = aWorld.add("Camera ←", {{"ARROWLEFT"}}, T_ONHOLD, [this] (const auto&, auto, const auto&) {
		r.translate(-3.f / (r.getZoom() / 16.f), 0.f);
	});

	iCamRight = aWorld.add("Camera →", {{"ARROWRIGHT"}}, T_ONHOLD, [this] (const auto&, auto, const auto&) {
		r.translate(3.f / (r.getZoom() / 16.f), 0.f);
	});

	iCamZoomIn = aWorld.add("Zoom +", {{"CONTROL", "+"}}, T_ONPRESS, [this] (const auto&, auto, const auto&) {
		float nz = r.getZoom() * 2.f;
		r.setZoom(nz >= 32.f ? 32.f : nz);
	});

	iCamZoomOut = aWorld.add("Zoom -", {{"CONTROL", "-"}}, T_ONPRESS, [this] (const auto&, auto, const auto&) {
		float nz = r.getZoom() / 2.f;
		r.setZoom(nz);
	});

	iCamZoomWh = aWorld.add("Zoom Wheel", {{"CONTROL"}, M_WHEEL}, T_ONPRESS, [this] (const auto&, auto, const auto& ii) {
		double d = ii.getWheelDy();
		if (d == 0.0) {
			return;
		}

		float nz = std::min(32.f, std::max(1.f, r.getZoom() - (d > 0.0 ? 1.f : -1.f)));
		r.setZoom(nz);
	});

	iCamPanWh = aWorld.add("Pan Camera Wheel", {{}, M_WHEEL}, T_ONPRESS, [this] (const auto&, auto, const auto& ii) {
		float z = r.getZoom();
		r.translate(ii.getWheelDx() / z, ii.getWheelDy() / z);
	});

	iCamPanMo = aWorld.add("Pan Camera Mouse", {{}, M_PRIMARY}, T_ONMOVE, [this] (const auto&, auto ev, const auto& ii) {
		r.translate(
			-(ii.getMouseX() - ii.getLastMouseX()) / r.getZoom(),
			-(ii.getMouseY() - ii.getLastMouseY()) / r.getZoom()
		);
	});

	std::puts("[World] Created");

	//loadMissingChunksTick();
}

void World::setCursorCount(u32 c) {
	cursorCount = c;
}

void World::tick() {
	//loadMissingChunksTick();
}

bool World::unloadChunks(sz_t amount) {
	// TODO: unload furthest ones first
	// https://stackoverflow.com/questions/14902876/indices-of-the-k-largest-elements-in-an-unsorted-length-n-array
	for (auto it = chunks.begin(); it != chunks.end(); ++it) {
		if (it->second.shouldUnload() && !r.isChunkVisible(it->second)) {
			it = chunks.erase(it);

			if (--amount == 0) {
				return true;
			}
		}
	}

	return false;
}

bool World::freeMemory(bool tryHarder) {
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
	return std::max(std::min(mv * 4, 128ul), mv);
}

const Camera& World::getCamera() const {
	return r;
}

const std::unordered_map<Chunk::Key, Chunk>& World::getChunkMap() const {
	return chunks;
}

Chunk& World::getOrLoadChunk(Chunk::Pos x, Chunk::Pos y) {
	auto it = chunks.try_emplace(Chunk::key(x, y), x, y, *this);
	if (it.second) {
		sz_t ml = getMaxLoadedChunks();
		if (chunks.size() > ml) {
			it.first->second.preventUnloading(true);
			if (!unloadChunks(chunks.size() - ml)) {
				std::printf("[World] Can't keep loaded chunks (%ld) under limit (%ld)\n", chunks.size(), ml);
			}

			it.first->second.preventUnloading(false);
		}
	}

	return it.first->second;
}

RGB_u World::getBackgroundColor() const {
	return bgClr;
}

const char * World::getChunkUrl(Chunk::Pos x, Chunk::Pos y) {
	// /api/worlds/<name>/view/<x>/<y>
	// i32 = -2147483648 (11 chars)
	static char urlBuf[12 + World::maxNameLength + 6 + 11 + 1 + 11 + 1] = {0};
	std::sprintf(urlBuf, "/api/worlds/%s/view/%i/%i", name.c_str(), x, y);
	return urlBuf;
}

void World::signalChunkLoaded(Chunk& c) {
	r.useChunk(c);
}

void World::signalChunkUnloaded(Chunk& c) {
	r.unuseChunk(c);
}

void World::loadMissingChunksTick() {
	if (chunkLoaderQueue.empty()) {
		std::puts("Chunkloaderqueue empty");
		i32 cx = std::floor(r.getX() / Chunk::size);
		i32 cy = std::floor(r.getY() / Chunk::size);
		chunkLoaderQueue.emplace(cx, cy);
	}

	auto [x, y] = chunkLoaderQueue.front();
	chunkLoaderQueue.pop();

	const auto queueIfUnloaded = [this] (Chunk::Pos x, Chunk::Pos y) {
		/*if (!r.isChunkVisible(x, y)) {
			return;
		}*/

		auto search = chunks.find(Chunk::key(x, y));
		if (search == chunks.end()) {
			chunkLoaderQueue.emplace(x, y);
		}
	};

	auto search = chunks.find(Chunk::key(x, y));
	if (search == chunks.end()) {
		getOrLoadChunk(x, y);
		queueIfUnloaded(x - 1, y);
		queueIfUnloaded(x + 1, y);
		queueIfUnloaded(x, y - 1);
		queueIfUnloaded(x, y + 1);
	}
}
