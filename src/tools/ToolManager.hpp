#pragma once

#include <cstdint>
#include <tuple>
#include <functional>

#include "tools/Tool.hpp"

#include "tools/providers/ColorProvider.hpp"

#include "tools/impl/PencilTool.hpp"
#include "tools/impl/MoveTool.hpp"
#include "tools/impl/ZoomTool.hpp"
#include "tools/impl/PipetteTool.hpp"
#include "util/Signal.hpp"

class World;
class InputAdapter;

class ToolManager {
public:
	// Providers manage shared context between tools, and they can also have keybinds
	using ProvidersTuple = std::tuple<ColorProvider>;
	using ToolsTuple = std::tuple<PencilTool, MoveTool, ZoomTool, PipetteTool>;

	using StateTuple = std::tuple<
			ColorProvider::State,
			PencilTool::State,
			MoveTool::State,
			ZoomTool::State,
			PipetteTool::State>;

	// cur could be a non currently selected tool.
	Signal<void(ToolStates&, Tool* cur)> onLocalStateChanged;

private:
	World& w;
	ToolStates& localState;
	ProvidersTuple providers;
	ToolsTuple tools;

public:
	ToolManager(World&, ToolStates& localState, InputAdapter&);

	World& getWorld();

	template<typename Fn>
	constexpr void forEachTool(Fn cb);

	// Gets a tool or provider
	template<typename T>
	constexpr T& get();

	ToolStates& getLocalState();
	Tool* getByNetId(std::uint8_t tid);

	// From local state
	Tool* getSelectedTool();
	// From specified state
	Tool* getSelectedTool(const ToolStates& st);

	std::uint64_t getState(const ToolStates&);
	bool updateState(ToolStates&, std::uint8_t newTid, std::uint64_t newState);

	template<typename T>
	void selectTool();
	void selectTool(Tool*);

	template<typename T>
	void emitLocalStateChanged();
};

#include "ToolManager.tpp" // IWYU pragma: keep
