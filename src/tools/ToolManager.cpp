#include "ToolManager.hpp"

#include <utility>

#include <util/misc.hpp>
#include <InputManager.hpp>

ToolManager::ToolManager(World& w, InputAdapter& ia)
: selectedTool(nullptr),
  w(w),
  local(true),
  providers(cartesian_make_tuple<ProvidersTuple>(std::forward_as_tuple(*this, ia.mkAdapter("Tools")))),
  tools(cartesian_make_tuple<ToolsTuple>(std::forward_as_tuple(*this, ia.mkAdapter("Tools")))) {
	selectTool<MoveTool>();
}

ToolManager::ToolManager(World& w)
: selectedTool(nullptr),
  w(w),
  local(false),
  providers(cartesian_make_tuple<ProvidersTuple>(*this)),
  tools(cartesian_make_tuple<ToolsTuple>(*this)) { }

bool ToolManager::isLocal() const {
	return local;
}

World& ToolManager::getWorld() {
	return w;
}

Tool* ToolManager::getSelectedTool() {
	return selectedTool;
}

void ToolManager::setOnSelectionChanged(std::function<void(Tool* old, Tool* cur)> cb) {
	onSelectionChanged = std::move(cb);
}

void ToolManager::selectTool(Tool* tool) {
	if (tool == selectedTool) {
		return;
	}

	Tool* oldTool = selectedTool;
	selectedTool = tool;

	if (oldTool != nullptr) {
		oldTool->onSelectionChanged(false);
	}

	if (selectedTool != nullptr) {
		selectedTool->onSelectionChanged(true);
	}

	if (onSelectionChanged) {
		onSelectionChanged(oldTool, selectedTool);
	}
}
