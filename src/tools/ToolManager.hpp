#pragma once

#include <cstdint>
#include <tuple>
#include <functional>

#include <tools/Tool.hpp>
#include <tools/impl/PencilTool.hpp>
#include <tools/impl/MoveTool.hpp>

class World;

class ToolManager {
public:
	using ToolsTuple = std::tuple<PencilTool, MoveTool>;

private:
	Tool* selectedTool;
	World& w;
	std::function<void(Tool* old, Tool* cur)> onSelectionChanged;
	const bool local; // is this instance from local player or remote?
	ToolsTuple tools;

public:
	ToolManager(World&, InputAdapter&); // local ctor
	ToolManager(World&); // remote ctor

	bool isLocal() const;

	World& getWorld();

	template<typename Fn>
	constexpr void forEachTool(Fn cb);

	template<typename T>
	constexpr T getTool();

	Tool* getSelectedTool();
	template<typename T>
	void selectTool();
	void selectTool(Tool*);

	void setOnSelectionChanged(std::function<void(Tool* old, Tool* cur)>);
};

#include "ToolManager.tpp"
