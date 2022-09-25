#pragma once

#include <util/emsc/ui/Button.hpp>
#include <util/emsc/ui/Window.hpp>

#include <vector>

class ToolManager;

class ToolWindow : public eui::Window {
	ToolManager& tm;
	std::vector<eui::Button> toolBtns;

public:
	ToolWindow(ToolManager&);

	void buildWindow();
};

