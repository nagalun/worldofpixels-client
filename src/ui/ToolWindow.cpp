#include "ToolWindow.hpp"

#include <tools/ToolManager.hpp>
#include <util/emsc/audio.hpp>

ToolWindow::ToolWindow(ToolManager& tm)
: Window(true, false),
  tm(tm) {
	addClass("owop-tools");

	tm.setOnSelectionChanged([this] (Tool * oldTool, Tool * newTool) {
		this->tm.forEachTool([&] (int i, Tool& t) {
			if (oldTool && *oldTool == t) {
				toolBtns[i].delClass("active");
			}

			if (newTool && *newTool == t) {
				toolBtns[i].addClass("active");
			}
		});
	});

	buildWindow();
	bringUp();
	move(3, 30);
}


void ToolWindow::buildWindow() {
	auto& cont = Window::getContent();
	Tool * selectedTool = tm.getSelectedTool();

	toolBtns.clear();
	tm.forEachTool([&] (int i, Tool& t) {
		auto& btn = toolBtns.emplace_back([this, &t] {
			tm.selectTool(&t);
			playAudioId("a-btn");
		});

		btn.addClass("tool");

		if (selectedTool && *selectedTool == t) {
			btn.addClass("active");
		}

		btn.setProperty("title", t.getName());
		btn.setProperty("dataset.tool", t.getName());

		btn.appendTo(cont);
	});
}
