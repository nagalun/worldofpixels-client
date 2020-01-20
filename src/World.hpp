#pragma once

#include <string>
#include <memory>
#include <utility>
#include <queue>
#include <unordered_map>

#include "Chunk.hpp"
#include "Cursor.hpp"
#include "SelfCursor.hpp"
#include "Renderer.hpp"
#include "User.hpp"

#include "explints.hpp"
#include "color.hpp"

class InputAdapter;
class ImAction;

class World {
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
	std::unique_ptr<SelfCursor> me; // can't really be null
	std::optional<User::Id> owner;
	u32 cursorCount;
	InputAdapter& aWorld;

	std::shared_ptr<ImAction> iPrintCoords;
	std::shared_ptr<ImAction> iRoundCoords;
	std::shared_ptr<ImAction> iCamUp;
	std::shared_ptr<ImAction> iCamDown;
	std::shared_ptr<ImAction> iCamLeft;
	std::shared_ptr<ImAction> iCamRight;
	std::shared_ptr<ImAction> iCamZoomIn;
	std::shared_ptr<ImAction> iCamZoomOut;
	std::shared_ptr<ImAction> iCamZoomWh;
	std::shared_ptr<ImAction> iCamPanWh;
	std::shared_ptr<ImAction> iCamPanMo;

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
