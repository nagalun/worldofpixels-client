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
#include <optional>

#include <InputManager.hpp>
#include <Renderer.hpp>
#include <tools/ToolManager.hpp>
#include <ui/ToolWindow.hpp>
#include <ui/PositionWidget.hpp>
#include <ui/PlayerCountWidget.hpp>


class alignas(32) World {
public:
	// this is absolute pixel pos
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

	InputAdapter& aWorld;
	ToolManager toolMan;
	ToolWindow toolWin;
	PositionWidget posUi;
	PlayerCountWidget pCntUi;

	ImAction iMoveCursor;
	ImAction iPrintCoords;
	ImAction iRoundCoords;

	u16 tickNum;
	bool drawingRestricted;

public:
	World(InputAdapter& base, std::string name, std::unique_ptr<SelfCursor>,
			RGB_u bgClr, bool restricted, std::optional<User::Id> owner);

	void setCursorCount(u32 worldCount, u32 globalCount);
	void tick();

	sz_t unloadChunks(sz_t amount = 8);
	sz_t unloadFarChunks();
	sz_t unloadAllChunks();
	bool freeMemory(bool tryHarder = false);

	sz_t getMaxLoadedChunks() const;

	Renderer& getRenderer();
	Camera& getCamera();
	const Camera& getCamera() const;
	SelfCursor& getCursor();
	const SelfCursor& getCursor() const;
	void recalculateCursorPosition();
	void recalculateCursorPosition(const InputInfo&);

	const std::unordered_map<Chunk::Key, Chunk>& getChunkMap() const;
	Chunk& getOrLoadChunk(Chunk::Pos, Chunk::Pos);
	Chunk * getChunkAt(World::Pos, World::Pos);
	const Chunk * getChunkAt(World::Pos, World::Pos) const;

	RGB_u getPixel(World::Pos, World::Pos) const;
	bool setPixel(World::Pos, World::Pos, RGB_u);

	RGB_u getBackgroundColor() const;
	const char * getChunkUrl(Chunk::Pos, Chunk::Pos);
	void signalChunkUpdated(Chunk *);
	void signalChunkUnloaded(Chunk *);

private:
	float getDistanceToChunk(const Chunk&) const;
	void loadMissingChunksTick();
};
