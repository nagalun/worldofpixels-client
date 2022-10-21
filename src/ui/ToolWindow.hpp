#pragma once

#include <util/emsc/ui/Button.hpp>
#include <util/emsc/ui/Window.hpp>
#include <util/NonCopyable.hpp>

#include <vector>

class ToolManager;

class ToolWindow : public eui::Window, NonCopyable {
	ToolManager& tm;
	std::vector<eui::Button> toolBtns;

public:
	ToolWindow(ToolManager&);
	ToolWindow(ToolWindow&&) noexcept;
	const ToolWindow& operator=(ToolWindow&&) noexcept;

	void buildWindow();
};

