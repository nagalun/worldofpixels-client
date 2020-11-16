#pragma once

#include <util/color.hpp>
#include <util/explints.hpp>
#include <uvias/User.hpp>
#include <world/Chunk.hpp>
#include <world/Cursor.hpp>
#include <world/SelfCursor.hpp>
#include <string>
#include <memory>
#include <utility>
#include <queue>
#include <unordered_map>

#include <InputManager.hpp>
#include <Renderer.hpp>
#include <ui/ToolWindow.hpp>


class alignas(32) World {
public:
	using Pos = i32;

	static constexpr Chunk::Pos border = std::numeric_limits<Pos>::max() / Chunk::size;
	static constexpr sz_t maxNameLength = 24;

private:
	const std::string name;
	RGB_u bgClr;
	Renderer r;

	std::queue<std::pair<Chunk::Pos, Chunk::Pos>> chunkLoaderQueue;
	std::unordered_map<Chunk::Key, Chunk> chunks;
	std::unordered_map<Cursor::Id, Cursor> cursors; // visible cursors only

	std::optional<User::Id> owner;
	SelfCursor me;
	u32 cursorCount;

	ToolWindow toolWin;
	InputAdapter& aWorld;

	ImAction iPrintCoords;
	ImAction iRoundCoords;
	ImAction iCamUp;
	ImAction iCamDown;
	ImAction iCamLeft;
	ImAction iCamRight;
	ImAction iCamZoomIn;
	ImAction iCamZoomOut;
	ImAction iCamZoomWh;
	ImAction iCamPanWh;
	ImAction iCamPanMo;
	ImAction iCamTouch;

	bool drawingRestricted;

public:
	World(InputAdapter& base, std::string name, std::unique_ptr<SelfCursor>,
			RGB_u bgClr, bool restricted, std::optional<User::Id> owner);

	void setCursorCount(u32);
	void tick();

	bool unloadChunks(sz_t amount = 1);
	bool freeMemory(bool tryHarder = false);

	sz_t getMaxLoadedChunks() const;
	const Camera& getCamera() const;
	const std::unordered_map<Chunk::Key, Chunk>& getChunkMap() const;
	Chunk& getOrLoadChunk(Chunk::Pos, Chunk::Pos);

	RGB_u getBackgroundColor() const;
	const char * getChunkUrl(Chunk::Pos, Chunk::Pos);
	void signalChunkLoaded(Chunk&);
	void signalChunkUnloaded(Chunk&);

private:
	void loadMissingChunksTick();
};
