#pragma once

#include <string>
#include <unordered_map>

#include "Chunk.hpp"
#include "Cursor.hpp"
#include "SelfCursor.hpp"
#include "Renderer.hpp"
#include "User.hpp"

#include "color.hpp"

class World {
public:
	using Pos = i32;

	static constexpr Chunk::Pos border = std::numeric_limits<Pos>::max() / Chunk::size;
	static constexpr sz_t maxNameLength = 24;

private:
	const std::string name;
	std::unordered_map<Chunk::Key, Chunk> chunks;
	std::unordered_map<Cursor::Id, Cursor> cursors; // visible cursors only
	std::unique_ptr<SelfCursor> me; // can't really be null
	std::optional<User::Id> owner;
	Renderer r;
	RGB_u bgClr;
	u32 cursorCount;
	bool drawingRestricted;

public:
	World(std::string name, std::unique_ptr<SelfCursor>, RGB_u bgClr, bool restricted, std::optional<User::Id>);

	void setCursorCount(u32);

	Chunk& getOrLoadChunk(Chunk::Pos, Chunk::Pos);

	RGB_u getBackgroundColor() const;
	const char * getChunkUrl(Chunk::Pos, Chunk::Pos);
	void signalChunkLoaded(Chunk&);
};
