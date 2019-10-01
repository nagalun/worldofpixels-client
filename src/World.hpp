#pragma once

#include <string>
#include <unordered_map>

#include "Chunk.hpp"
#include "Cursor.hpp"
#include "Renderer.hpp"

#include "color.hpp"

class World {
public:
	using Pos = i32;

	static constexpr Chunk::Pos border = std::numeric_limits<Pos>::max() / Chunk::size;

private:
	const std::string name;
	std::unordered_map<Chunk::Key, Chunk> chunks;
	std::unordered_map<Cursor::Id, Cursor> cursors; // visible cursors only
	Cursor me;
	Renderer r;
	RGB_u bgClr;

public:
	World(std::string name);

	RGB_u getBackgroundColor() const;
	const char * getChunkUrl(Chunk::Pos, Chunk::Pos);
	void signalChunkLoaded(Chunk&);
};
