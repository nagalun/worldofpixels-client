#pragma once

#include <util/emsc/ui/Object.hpp>
#include <util/emsc/ui/Window.hpp>

#include <vector>

class ToolManager;
struct EmscriptenMouseEvent;

class ToolWindow : public eui::Window {
	ToolManager& tm;
	std::vector<eui::Object> toolBtns;

public:
	ToolWindow(ToolManager&);

	void buildWindow();

private:
	static int selectToolEvent(int type, const EmscriptenMouseEvent *ev, void *data);
};

