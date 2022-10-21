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

class World;
class InputAdapter;

class ToolManager {
public:
	// Providers manage shared context between tools, and they can also have keybinds
	using ProvidersTuple = std::tuple<ColorProvider>;
	using ToolsTuple = std::tuple<PencilTool, MoveTool, ZoomTool, PipetteTool>;

private:
	Tool* selectedTool;
	World& w;
	std::function<void(Tool* old, Tool* cur)> onSelectionChanged;
	const bool local; // is this instance from local player or remote?
	ProvidersTuple providers;
	ToolsTuple tools;

public:
	ToolManager(World&, InputAdapter&); // local ctor
	ToolManager(World&); // remote ctor

	bool isLocal() const;

	World& getWorld();

	template<typename Fn>
	constexpr void forEachTool(Fn cb);

	template<typename T>
	constexpr T& get();

	Tool* getSelectedTool();
	template<typename T>
	void selectTool();
	void selectTool(Tool*);

	void setOnSelectionChanged(std::function<void(Tool* old, Tool* cur)>);
};

#include "ToolManager.tpp" // IWYU pragma: keep
