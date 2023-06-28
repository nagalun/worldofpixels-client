#pragma once

#include "tools/ToolManager.hpp"
#include "util/emsc/ui/Button.hpp"
#include "util/emsc/ui/Window.hpp"
#include "util/NonCopyable.hpp"

#include <vector>

class ToolStates;
class Tool;

class ToolWindow : public eui::Window, NonCopyable {
	ToolManager& tm;
	decltype(ToolManager::onLocalStateChanged)::SlotKey toolChSk;
	std::vector<eui::Button> toolBtns;

public:
	ToolWindow(ToolManager&);
	ToolWindow(ToolWindow&&) noexcept;
	const ToolWindow& operator=(ToolWindow&&) noexcept;

	void buildWindow();
	void updatePointerCursor(ToolStates& ts, Tool * t);
};
