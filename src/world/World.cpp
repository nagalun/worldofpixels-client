#include "world/World.hpp"

#include <array>
#include <algorithm>
#include <cassert>
#include <functional>
#include <optional>
#include <cstdio>
#include <cmath>

#include "InputManager.hpp"
#include "Camera.hpp"
#include "Client.hpp"
#include "PacketDefinitions.hpp"

#include "util/emsc/dom.hpp"
#include "util/byteswap.hpp"
#include "util/explints.hpp"
#include "util/misc.hpp"
#include "world/Chunk.hpp"

World::World(Client& cl, InputAdapter& base, std::string name, std::unique_ptr<SelfCursor::Builder> _me,
		RGB_u bgClr, bool restricted, std::optional<User::Id> owner)
: cl(cl),
  name(std::move(name)),
  bgClr(bgClr),
  r(*this),
  currentAreaSyncSeq(0),
  expectedAreaSyncSeq(0),
  owner(std::move(owner)),
  me(_me->build(*this)),
  aWorld(base.mkAdapter("World")),
  toolMan(*this, me.getToolStates(), aWorld),
  toolWin(toolMan),
  posUi(me.getX(), me.getY(), r.getZoom()),
  settingsBtn("settings", "Settings"),
  helpBtn("help", "Help"),
  iMoveCursor(aWorld, "Move cursor", T_ONENTER | T_ONPRESS | T_ONMOVE | T_ONWHEEL | T_ONLEAVE | T_OPT_ALWAYS),
  tickNum(0),
  drawingRestricted(restricted) {

	toolMan.updateState(me.getToolStates(), _me->getTid(), _me->getTid());

	toolChSk = toolMan.onLocalStateChanged.connect([this] (ToolStates&, Tool*) {
		me.markNeedsSend();
	});

	u32 cssBgClr = bswap_32(bgClr.rgb);
	eui_root_css_property_set("--bg-clr", svprintf("#%08X", cssBgClr));

	iMoveCursor.setDefaultKeybind(Keybind::ANY_PTR_BTN);
	iMoveCursor.setCb([this] (ImAction::Event& ev, const InputInfo& ii) {
		recalculateCursorPosition(ii);
	});

	llcorner.addClass("owop-llui");
	llcorner.appendToMainContainer();

	settingsBtn.setCb([this] {
		sw.moveToCenter(true, true);
		sw.toggle();
	});

	helpBtn.setCb([this] {
		hw.moveToCenter(true, true);
		hw.toggle();
	});

	settingsBtn.appendTo(llcorner.get<0>());
	helpBtn.appendTo(llcorner.get<0>());

	std::puts("[World] Created");

	loadMissingChunksTick();
}

void World::setCursorCount(u32 worldCount, u32 globalCount) {
	pCntUi.setCounts(worldCount, globalCount);
}

void World::tick() {
	++tickNum;

	getCursor().tick();

	// every ~250ms
	if (!(tickNum % 5)) {
		loadMissingChunksTick();
	}

	// every 10 seconds
	if (!(tickNum % (20 * 10))) {
		sz_t n = unloadFarChunks();
		if (n > 0) {
			std::printf("[World] Unloaded %lu chunks.\n", n);
		}
	}
}

sz_t World::unloadChunks(sz_t targetAmount) {
	sz_t origAmount = targetAmount;
	bool doingWork = false;

	using cit_t = decltype(chunks)::const_iterator;
	// allocating memory may not be safe right now
	std::array<cit_t, 32> toUnload;

	// necessary so that partial_sort_copy gives me iterators
	struct it_wrap : cit_t {
		it_wrap(cit_t it) : cit_t(it) { }
		using value_type = cit_t;
		value_type& operator*() { return *this; }
	};

	do {
		auto end = std::partial_sort_copy(
				it_wrap{chunks.cbegin()}, it_wrap{chunks.cend()},
				toUnload.begin(), toUnload.begin() + std::min(targetAmount, toUnload.size()),
				[this] (const cit_t& a, const cit_t& b) {
					bool unlA = a->second.shouldUnload();
					bool unlB = b->second.shouldUnload();
					bool visA = r.isChunkVisible(a->second);
					bool visB = r.isChunkVisible(b->second);

					// order of unloading: non-visible far to closest, visible far to closest
					// non-unloadable chunks go last
					if (unlA && !unlB) {
						return true;
					} else if (!unlA && unlB) {
						return false;
					} else if (visA && !visB) {
						return false;
					} else if (!visA && visB) {
						return true;
					}

					float dstA = getDistanceToChunk(a->second);
					float dstB = getDistanceToChunk(b->second);
					return dstA > dstB;
				});

		doingWork = end == toUnload.end();
		for (auto it = toUnload.begin(); it != end; ++it) {
			if (!(*it)->second.shouldUnload()) {
				doingWork = false;
				break;
			}
			chunks.erase(*it);
			--targetAmount;
		}
	} while (targetAmount != 0 && doingWork);

	return origAmount - targetAmount;
}

sz_t World::unloadNonSubscribedChunks() {
	return unloadChunksPred([this] (const Chunk& c) {
		auto pos = c.getUpdArea();
		return c.shouldUnload() && !isSubscribedToUpdateArea(pos);
	});
}

sz_t World::unloadNonVisibleNonReadyChunks() {
	return unloadChunksPred([this] (const Chunk& c) {
		return c.shouldUnload() && !r.isChunkVisible(c, 256.f) && !c.isReady();
	});
}

sz_t World::unloadFarChunks() {
	return unloadChunksPred([this] (const Chunk& c) {
		return c.shouldUnload() && !r.isChunkVisible(c, 256.f) && (!c.isReady() || getDistanceToChunk(c) > 20.f);
	});
}

sz_t World::unloadAllChunks() {
	return unloadChunksPred([] (const Chunk& c) {
		return c.shouldUnload();
	});
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
	return std::min(std::max(std::min(mv * 8, 128ul), mv), Renderer::maxLoadedChunks);
}

Box<eui::Object, eui::Object>& World::getLlCornerUi() {
	return llcorner;
}

Client& World::getClient() {
	return cl;
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

ToolManager& World::getToolManager() {
	return toolMan;
}

const std::vector<Cursor>& World::getCursors() const {
	return cursors;
}

const std::unordered_map<Chunk::Key, Chunk>& World::getChunkMap() const {
	return chunks;
}

Chunk * World::getChunk(Chunk::Pos x, Chunk::Pos y) {
	auto it = chunks.find(Chunk::key(x, y));
	if (it != chunks.end()) {
		return &it->second;
	}

	return nullptr;
}

Chunk& World::getOrMkChunk(Chunk::Pos x, Chunk::Pos y) {
	auto it = chunks.try_emplace(Chunk::key(x, y), x, y, *this);
	if (it.second) { // if a chunk was emplaced
		r.queueRerender();
		unloadNonVisibleNonReadyChunks();

		sz_t ml = getMaxLoadedChunks();
		if (chunks.size() >= ml) {
			it.first->second.preventUnloading(true);
			if (!unloadChunks(chunks.size() - ml + 1)) {
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
	// i32 = -2147483648 (11 chars)
	constexpr const char fmt[] = "/api/worlds/view?n=%s&x=%i&y=%i";
	static char urlBuf[sizeof(fmt) + World::maxNameLength + 11 + 11] = {0};
	std::sprintf(urlBuf, fmt, name.c_str(), x, y);
	return urlBuf;
}

void World::signalChunkLoaded(Chunk * c) {
	r.chunkToUpdate(c);

	// a chunk just got loaded, instead of waiting another tick to start another request,
	// check now to maintain the 4 concurrent chunk loads.
	// TODO: in case the max limit of chunks is reached, avoid a fast load/unload loop
	loadMissingChunksTick(false);
}

void World::signalChunkUpdated(Chunk * c) {
	r.chunkToUpdate(c);
}

void World::signalChunkUnloaded(Chunk * c) {
	r.chunkUnloaded(c);
}

void World::loadMissingChunksTick(bool allowSubscribes) {
	static std::vector<Chunk*> sorted;
	sorted.clear();
	sorted.reserve(r.getMaxVisibleChunks());

	if (!r.getGlContext().ok()) {
		// skip loading chunks if the rendering context isn't valid
		return;
	}

	int numLoading = 0;

	iterateScreenTiles(Chunk::size, [&, this] (twoi32 pos) {
		Chunk& c = getOrMkChunk(pos.c.x, pos.c.y);
		if (c.isReady()) {
			return;
		}

		if (c.isLoading()) {
			++numLoading;
			return;
		}

		auto it = std::lower_bound(sorted.begin(), sorted.end(), &c, [this] (const Chunk* c2, const Chunk* c1) {
			return getDistanceToChunk(*c1) > getDistanceToChunk(*c2);
		});

		sorted.emplace(it, &c);
	});

	bool needsSubscribe = false;
	for (auto it = sorted.begin(); it != sorted.end() && numLoading < 4; ++it) {
		if (!std::binary_search(subscribedUpdateAreas.begin(), subscribedUpdateAreas.end(), (*it)->getUpdArea())) {
			// if the update area is not confirmed to be subscribed don't load the chunk
			needsSubscribe = true;
			continue;
		}

		numLoading += (*it)->tryLoad();
	}

	if (allowSubscribes && needsSubscribe) {
		subscribeToUpdateAreas();
	}
}

float World::getDistanceToChunk(const Chunk& c) const {
	float dx = std::abs(r.getX() / Chunk::size - c.getX());
	float dy = std::abs(r.getY() / Chunk::size - c.getY());

	return dx + dy;
}

Chunk * World::getChunkAtPx(World::Pos x, World::Pos y) {
	auto it = chunks.find(Chunk::key(x >> Chunk::posShift, y >> Chunk::posShift));
	if (it != chunks.end()) {
		return &it->second;
	}

	return nullptr;
}

const Chunk * World::getChunkAtPx(World::Pos x, World::Pos y) const {
	auto it = chunks.find(Chunk::key(x >> Chunk::posShift, y >> Chunk::posShift));
	if (it != chunks.end()) {
		return &it->second;
	}

	return nullptr;
}

RGB_u World::getPixel(World::Pos x, World::Pos y) const {
	const Chunk * c = getChunkAtPx(x, y);

	if (c) {
		return c->getPixel(x, y);
	}

	return {{0, 0, 0, 0}};
}

bool World::setPixel(World::Pos x, World::Pos y, RGB_u clr, bool alphaBlending) {
	Chunk * c = getChunkAtPx(x, y);

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
	float wx, wy;
	float mx = ii.getMidX();
	float my = ii.getMidY();
	r.getWorldPosFromScreenPos(mx, my, &wx, &wy);

	SelfCursor& cur = getCursor();
	const Camera& cam = getCamera();

	cur.setPos(wx, wy);
	posUi.setPos(cur.getX(), cur.getY(), cam.getX(), cam.getY(), cam.getZoom());
}

template<typename Func>
sz_t World::unloadChunksPred(Func f) {
	sz_t unloaded = 0;

	for (auto it = chunks.cbegin(); it != chunks.cend(); ) {
		const Chunk& c = it->second;

		if (f(c)) {
			it = chunks.erase(it);
			unloaded++;
		} else {
			++it;
		}
	}

	return unloaded;
}


template<typename Func>
void World::iterateScreenTiles(i32 tileSize, Func f) {
	double w, h;
	r.getScreenSize(&w, &h);

	float hVpWidth = w / 2.f / r.getZoom();
	float hVpHeight = h / 2.f / r.getZoom();
	float tlx = std::floor((r.getX() - hVpWidth) / tileSize);
	float tly = std::floor((r.getY() - hVpHeight) / tileSize);
	float brx = std::floor((r.getX() + hVpWidth) / tileSize);
	float bry = std::floor((r.getY() + hVpHeight) / tileSize);

	for (; tly <= bry; tly += 1.f) {
		for (float tlx2 = tlx; tlx2 <= brx; tlx2 += 1.f) {
			twoi32 pos = mk_twoi32(tlx2, tly);
			f(pos);
		}
	}
}

void World::handleUpdates(net::DAbsUpdAreaPos uaX, net::DAbsUpdAreaPos uaY, net::VPlayersHide hides, net::VPlayersShow shows, net::VPlayersUpdate updates) {
	const auto lowerBoundById = [this] (Cursor::Id id) {
		return std::lower_bound(cursors.begin(), cursors.end(), id, [] (const Cursor& c, Cursor::Id id) {
			return id < c.getId();
		});
	};

	const auto findById = [&, this] (Cursor::Id id) {
		auto it = lowerBoundById(id);
		if (it != cursors.end() && it->getId() != id) {
			it = cursors.end();
		}

		return it;
	};

	bool needsRender = false;

	for (auto pid : hides) {
		auto it = findById(pid);
		if (it != cursors.end()) {
			auto curUArea = it->getUpdArea();
			if (curUArea.c.x != uaX || curUArea.c.y != uaY) {
				// see below explanation for details. in this case, delete is being received last.
				continue;
			}
			needsRender |= true; // r.isPlayerVisible(*it)
			//std::printf("del %llu\n", pid.get());
			cursors.erase(it);
		}
	}

	for (const auto& [pid, relX, relY, step, tid, tstate] : updates) {
		auto it = findById(pid);
		assert((it != cursors.end() && "updated a non existent cursor!"));
		auto curUArea = it->getUpdArea();
		if (curUArea.c.x != uaX || curUArea.c.y != uaY) {
			// this can happen because order of received updates for each update region is not guaranteed, so
			// if a cursor crosses an update area, a final relative update is sent on the original UA
			// so the clients know it went outside, along with the cursor data in the "shows" array on the new UA.
			// any of them could be received first, so if this condition is true, the final update
			// on the old UA was received last.
			continue;
		}
		//if (!isSubscribedToUpdateArea(it->getUpdArea())) {
		//	std::printf("[ERR] ");
		//}
		//std::printf("upd %llu, abspos_b %d, %d", pid.get(), it->getX(), it->getY());
		needsRender |= it->updateRel(relX, relY, step, toolMan, tid, tstate);
		auto uare = it->getUpdArea();
		//std::printf(" abspos_a %d, %d, relpos, %lld, %lld, ua %d, %d", it->getX(), it->getY(), relX.get(), relY.get(), uare.c.x, uare.c.y);
		if (!isSubscribedToUpdateArea(uare)) {
			// if the player now lies outside of the subscribed update areas we won't receive any more updates from it
			// so, forget the player
			// TODO: despawn after moving animation finishes
			//std::printf(" & del");
			cursors.erase(it);
		}
		//std::printf("\n");
	}

	for (const auto& [uid, plUpd] : shows) {
		const auto& [pid, absX, absY, step, tid, tstate] = plUpd;
		auto it = lowerBoundById(pid);
		if (it != cursors.end() && it->getId() == pid) {
			// can happen when crossing update regions.
			// setting the absolute pos shouldn't be necessary but just in case there's error
			//std::printf("newE %llu, pos %lld, %lld\n", pid.get(), absX.get(), absY.get());
			needsRender |= it->update(absX, absY, step, toolMan, tid, tstate);
		} else {
			//std::printf("new %llu, pos %lld, %lld\n", pid.get(), absX.get(), absY.get());
			cursors.emplace(it, cl.getUser(uid), pid, absX, absY, step, toolMan, tid, tstate);
			needsRender |= true;
		}
	}

	if (needsRender) {
		r.queueRerender();
	}
}

void World::setSubscribedUpdateAreas(u8 arseq, std::vector<twoi32> areas) {
	currentAreaSyncSeq = arseq;
	if (arseq != expectedAreaSyncSeq) {
		// ignore because it's not the most updated list we'll receive
		return;
	}

	subscribedUpdateAreas = std::move(areas);
	// we unload chunks that fall out of the subscribed areas because we are not going to
	// receive pixel updates from there anymore.
	unloadNonSubscribedChunks();

	// to make the reaction time as fast as possible we'll check the loadable chunks right now
	// but don't subscribe new areas to avoid fast and noisy loops
	loadMissingChunksTick(false);
}

void World::subscribeToUpdateAreas() {
	if (currentAreaSyncSeq != expectedAreaSyncSeq) {
		// wait until we're synchronized to subscribe to other areas
		return;
	}

	iterateScreenTiles(World::updateAreaSize, [this] (twoi32 pos) {
		auto dist = getDistance2dSq(pos, getCursor().getUpdArea());
		if (dist >= updateAreasMaxDist || subscribedUpdateAreas.size() >= updateAreasMaxNum) {
			// skip if too far from cursor (extremely big screen?)
			// or the max num of subscribed areas is reached (TODO: implement client-sided unsubscription)
			return;
		}

		auto it = std::lower_bound(subscribedUpdateAreas.begin(), subscribedUpdateAreas.end(), pos);
		if (it == subscribedUpdateAreas.end() || *it != pos) {
			//subscribedUpdateAreas.emplace(it, pos);
			++expectedAreaSyncSeq;
			std::printf("subscribing to %d, %d\n", pos.c.x, pos.c.y);
			cl.send(SSubscribeArea::toBuffer(pos.c.x, pos.c.y, true));
		}
	});
}

bool World::isSubscribedToUpdateArea(twoi32 pos) {
	return std::binary_search(subscribedUpdateAreas.begin(), subscribedUpdateAreas.end(), pos);
}

twoi32 World::updAreaOf(World::Pos x, World::Pos y) {
	float uAreaSzf = updateAreaSize;
	return mk_twoi32(std::floor(x / uAreaSzf), std::floor(y / uAreaSzf));
}

twoi32 World::updAreaOfChunk(Chunk::Pos x, Chunk::Pos y) {
	std::int32_t uAreaSz = updateAreaSize / Chunk::size;
	float uAreaSzf = uAreaSz;
	return mk_twoi32(std::floor(x / uAreaSzf), std::floor(y / uAreaSzf));
}
