#include "World.hpp"

#include <cstdio>

World::World(std::string name, std::unique_ptr<SelfCursor> me, RGB_u bgClr, bool restricted, std::optional<User::Id> owner)
: name(std::move(name)),
  bgClr(bgClr),
  r(*this),
  me(std::move(me)),
  owner(std::move(owner)),
  cursorCount(1),
  drawingRestricted(restricted) {
	std::puts("[World] Created");

	r.loadMissingChunks();
}

void World::setCursorCount(u32 c) {
	cursorCount = c;
}

bool World::freeMemory(bool tryHarder) {
	//auto chosenOne = chunks.end();
	for (auto it = chunks.begin(); it != chunks.end(); ++it) {
		if (it->second.shouldUnload() && !r.isChunkVisible(it->second)) {
			chunks.erase(it);
			return true;
		}
	}

	return false;
}


Chunk& World::getOrLoadChunk(Chunk::Pos x, Chunk::Pos y) {
	return chunks.try_emplace(Chunk::key(x, y), x, y, *this).first->second;
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
