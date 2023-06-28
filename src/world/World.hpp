#pragma once

#include <cmath>
#include <string>
#include <memory>
#include <utility>
#include <queue>
#include <unordered_map>
#include <optional>

#include "util/color.hpp"
#include "util/explints.hpp"
#include "util/misc.hpp"
#include "util/NonCopyable.hpp"
#include "util/emsc/ui/Object.hpp"
#include "uvias/User.hpp"
#include "world/Chunk.hpp"
#include "world/Cursor.hpp"
#include "world/SelfCursor.hpp"
#include "tools/ToolManager.hpp"
#include "InputManager.hpp"
#include "Renderer.hpp"
#include "PacketDefinitions.hpp"

#include "ui/misc/UiButton.hpp"
#include "ui/misc/Box.hpp"
#include "ui/ToolWindow.hpp"
#include "ui/PositionWidget.hpp"
#include "ui/PlayerCountWidget.hpp"
#include "ui/HelpWindow.hpp"
#include "ui/settings/SettingsWindow.hpp"

class Client;

class World : NonCopyable {
public:
	// this is absolute pixel pos
	using Pos = i32;

	static constexpr Chunk::Pos border = 0xFFFFFF / Chunk::size; // 16777215
	static constexpr sz_t maxNameLength = 24;

	// in pixels, how big the update regions are
	static constexpr i32 updateAreaSize = 2048;
	static constexpr u32 updateAreasMaxNum = 20;
	static constexpr u32 updateAreasMaxDist = 4;

	// expected world update frequency in ms
	static constexpr float updateRateMs = 50.f;

private:
	Client& cl;
	const std::string name;
	RGB_u bgClr;
	Renderer r;

	std::unordered_map<Chunk::Key, Chunk> chunks;
	std::vector<Cursor> cursors; // visible cursors only, sorted by pid
	std::vector<twoi32> subscribedUpdateAreas;
	u8 currentAreaSyncSeq;
	u8 expectedAreaSyncSeq;

	std::optional<User::Id> owner;
	SelfCursor me;

	/* clr picker column, palette window column */
	Box<eui::Object, eui::Object> llcorner;

	InputAdapter& aWorld;
	ToolManager toolMan;
	ToolWindow toolWin;
	PositionWidget posUi;
	PlayerCountWidget pCntUi;
	SettingsWindow sw;
	HelpWindow hw;
	UiButton settingsBtn;
	UiButton helpBtn;

	ImAction iMoveCursor;

	u16 tickNum;
	bool drawingRestricted;

	decltype(ToolManager::onLocalStateChanged)::SlotKey toolChSk;

public:
	World(Client&, InputAdapter& base, std::string name, std::unique_ptr<SelfCursor::Builder>,
			RGB_u bgClr, bool restricted, std::optional<User::Id> owner);

	void setCursorCount(u32 worldCount, u32 globalCount);
	void tick();

	sz_t unloadChunks(sz_t amount = 8);
	sz_t unloadFarChunks();
	sz_t unloadNonSubscribedChunks();
	sz_t unloadNonVisibleNonReadyChunks();
	sz_t unloadAllChunks();
	bool freeMemory(bool tryHarder = false);

	sz_t getMaxLoadedChunks() const;

	Box<eui::Object, eui::Object>& getLlCornerUi();

	Client& getClient();
	Renderer& getRenderer();
	Camera& getCamera();
	const Camera& getCamera() const;
	SelfCursor& getCursor();
	const SelfCursor& getCursor() const;
	ToolManager& getToolManager();
	void recalculateCursorPosition();
	void recalculateCursorPosition(const InputInfo&);

	const std::vector<Cursor>& getCursors() const;
	const std::unordered_map<Chunk::Key, Chunk>& getChunkMap() const;
	Chunk * getChunk(Chunk::Pos, Chunk::Pos);
	Chunk& getOrMkChunk(Chunk::Pos, Chunk::Pos);
	Chunk * getChunkAtPx(World::Pos, World::Pos);
	const Chunk * getChunkAtPx(World::Pos, World::Pos) const;

	RGB_u getPixel(World::Pos, World::Pos) const;
	bool setPixel(World::Pos, World::Pos, RGB_u, bool alphaBlending = false);

	const std::string& getName() const;
	RGB_u getBackgroundColor() const;
	const char * getChunkUrl(Chunk::Pos, Chunk::Pos);
	void signalChunkLoaded(Chunk *);
	void signalChunkUpdated(Chunk *);
	void signalChunkUnloaded(Chunk *);

	void updateUi();

	void handleUpdates(net::DAbsUpdAreaPos x, net::DAbsUpdAreaPos y, net::VPlayersHide, net::VPlayersShow, net::VPlayersUpdate);
	void setSubscribedUpdateAreas(u8 arseq, std::vector<twoi32> areas);
	bool isSubscribedToUpdateArea(twoi32 pos);

	static twoi32 updAreaOf(World::Pos x, World::Pos y);
	static twoi32 updAreaOfChunk(Chunk::Pos x, Chunk::Pos y);

private:
	template<typename Func>
	sz_t unloadChunksPred(Func f);

	template<typename Func>
	void iterateScreenTiles(i32 tileSize, Func f);

	float getDistanceToChunk(const Chunk&) const;
	void loadMissingChunksTick(bool allowSubscribes = true);
	void subscribeToUpdateAreas();
};
