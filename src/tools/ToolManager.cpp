#include "ToolManager.hpp"

#include <utility>

// this should be in utils
template <typename Tuple, std::size_t ...I, typename... Args>
static Tuple do_cartesian_make_tuple(std::index_sequence<I...>, Args&&... args) {
	if constexpr (sizeof... (Args) == 0) {
		return Tuple{};
	} else if constexpr (sizeof... (Args) == 1) {
		// dumb
		return Tuple{(I, std::get<0>(std::forward_as_tuple(std::forward<Args>(args)...)))...};
	} else {
		return Tuple{std::tuple_element_t<I, Tuple>{std::forward<Args>(args)...}...};
	}
}

template <typename Tuple, typename ...Args>
static Tuple cartesian_make_tuple(Args&&... args) {
    return do_cartesian_make_tuple<Tuple>(std::make_index_sequence<std::tuple_size_v<Tuple>>(), std::forward<Args>(args)...);
}

ToolManager::ToolManager(World& w, InputAdapter& ia)
: selectedTool(nullptr),
  w(w),
  local(true),
  tools(cartesian_make_tuple<ToolsTuple>(std::forward_as_tuple(*this, ia.mkAdapter("Tools")))) {
	selectTool<MoveTool>();
}

ToolManager::ToolManager(World& w)
: selectedTool(nullptr),
  w(w),
  local(false),
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
