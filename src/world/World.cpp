#include <world/World.hpp>
#include <InputManager.hpp>
#include <Camera.hpp>

#include <array>
#include <algorithm>
#include <cstdio>
#include <cmath>

World::World(InputAdapter& base, std::string name, std::unique_ptr<SelfCursor> me,
		RGB_u bgClr, bool restricted, std::optional<User::Id> owner)
: name(std::move(name)),
  bgClr(bgClr),
  r(*this),
  chunkLoaderQueue({std::pair<Chunk::Pos, Chunk::Pos>{0, 0}}),
  owner(std::move(owner)),
  me(std::move(*me)),
  cursorCount(1),
  aWorld(base.mkAdapter("World")),
  iPrintCoords(aWorld, "Print Coordinates"),
  iRoundCoords(aWorld, "Round Coordinates"),
  iCamUp(aWorld, "Camera ↑", T_ONHOLD),
  iCamDown(aWorld, "Camera ↓", T_ONHOLD),
  iCamLeft(aWorld, "Camera ←", T_ONHOLD),
  iCamRight(aWorld, "Camera →", T_ONHOLD),
  iCamZoomIn(aWorld, "Zoom +"),
  iCamZoomOut(aWorld, "Zoom -"),
  iCamZoomWh(aWorld, "Zoom Wheel", T_ONWHEEL),
  iCamPanWh(aWorld, "Pan Camera Wheel", T_ONWHEEL),
  iCamPanMo(aWorld, "Pan Camera Mouse", T_ONPRESS | T_ONMOVE),
  iCamTouch(aWorld, "Camera Touch Control", T_ONPRESS | T_ONMOVE),
  tickNum(0),
  drawingRestricted(restricted) {

	iPrintCoords.setDefaultKeybind("P");
	iPrintCoords.setCb([this] (auto&, const auto&) {
		std::printf("[World] Coords: %f, %f\n", r.getX(), r.getY());
	});

	iRoundCoords.setDefaultKeybind("O");
	iRoundCoords.setCb([this] (auto&, const auto&) {
		r.setPos(std::round(r.getX()), std::round(r.getY()));
	});

	iCamUp.setDefaultKeybind("ARROWUP");
	iCamUp.setCb([this] (auto&, const auto&) {
		r.translate(0.f, -3.f / (r.getZoom() / 16.f));
	});

	iCamDown.setDefaultKeybind("ARROWDOWN");
	iCamDown.setCb([this] (auto&, const auto&) {
		r.translate(0.f, 3.f / (r.getZoom() / 16.f));
	});

	iCamLeft.setDefaultKeybind("ARROWLEFT");
	iCamLeft.setCb([this] (auto&, const auto&) {
		r.translate(-3.f / (r.getZoom() / 16.f), 0.f);
	});

	iCamRight.setDefaultKeybind("ARROWRIGHT");
	iCamRight.setCb([this] (auto&, const auto&) {
		r.translate(3.f / (r.getZoom() / 16.f), 0.f);
	});

	iCamZoomIn.setDefaultKeybind({M_CTRL, "+"});
	iCamZoomIn.setCb([this] (auto&, const auto&) {
		float nz = r.getZoom() * 2.f;
		r.setZoom(nz >= 32.f ? 32.f : nz);
	});

	iCamZoomOut.setDefaultKeybind({M_CTRL, "-"});
	iCamZoomOut.setCb([this] (auto&, const auto&) {
		float nz = r.getZoom() / 2.f;
		r.setZoom(nz);
	});

	iCamZoomWh.setCb([this] (auto& ev, const auto& ii) {
		if (!(ii.getModifiers() & M_CTRL)) {
			ev.reject();
			return;
		}

		double d = ii.getWheelDy();
		if (d == 0.0) {
			return;
		}

		float nz = std::min(32.f, std::max(1.f, r.getZoom() - (d > 0.0 ? 1.f : -1.f)));
		r.setZoom(nz);
	});

	iCamPanWh.setCb([this] (auto& ev, const auto& ii) {
		if (ii.getModifiers() & M_CTRL) {
			ev.reject();
			return;
		}

		float z = r.getZoom();
		r.translate(ii.getWheelDx() / z, ii.getWheelDy() / z);
	});

	iCamPanMo.setDefaultKeybind(P_MMIDDLE);
	iCamPanMo.setCb([this] (auto& ev, const auto& ii) {
		r.translate(
			-ii.getDx() / r.getZoom(),
			-ii.getDy() / r.getZoom()
		);
	});

	iCamTouch.setDefaultKeybind(P_MPRIMARY);
	iCamTouch.setCb([
		this,
		lastDist{0.0}
	] (auto& ev, const auto& ii) mutable {
		const auto& p = ii.getActivePointers();
		if (p.size() != 2) {
			ev.reject();
			return;
		}

		double dist = std::sqrt(std::pow(p[1]->getX() - p[0]->getX(), 2) + std::pow(p[1]->getY() - p[0]->getY(), 2));
		if (ev.getActivationType() == T_ONPRESS) {
			lastDist = dist;
			return;
		}

		int lastMidX = (p[1]->getLastX() + p[0]->getLastX()) / 2;
		int lastMidY = (p[1]->getLastY() + p[0]->getLastY()) / 2;
		int midX = (p[1]->getX() + p[0]->getX()) / 2;
		int midY = (p[1]->getY() + p[0]->getY()) / 2;

		int midDx = midX - lastMidX;
		int midDy = midY - lastMidY;

		r.setZoom(std::max(1.f, r.getZoom() * (float)(dist / lastDist)));
		r.translate(
			-midDx / r.getZoom(),
			-midDy / r.getZoom()
		);
		
		lastDist = dist;
	});

	std::puts("[World] Created");

	//loadMissingChunksTick();
}

void World::setCursorCount(u32 c) {
	cursorCount = c;
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

sz_t World::unloadFarChunks() {
	sz_t unloaded = 0;

	for (auto it = chunks.cbegin(); it != chunks.cend(); ) {
		const Chunk& c = it->second;

		if (c.shouldUnload() && !r.isChunkVisible(c) && getDistanceToChunk(c) > 20.f) {
			it = chunks.erase(it);
			unloaded++;
		} else {
			++it;
		}
	}

	return unloaded;
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
	std::sprintf(urlBuf, /*/api*/"/worlds/%s/view/%i/%i", name.c_str(), x, y);
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

bool World::setPixel(World::Pos x, World::Pos y, RGB_u clr) {
	Chunk * c = getChunkAt(x, y);

	if (c) {
		return c->setPixel(x, y, clr);
	}

	return false;
}
