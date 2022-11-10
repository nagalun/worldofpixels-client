#include "ToolWindow.hpp"

#include "tools/ToolManager.hpp"
#include "util/emsc/audio.hpp"
#include "util/emsc/dom.hpp"
#include "util/stringparser.hpp"
#include <string_view>

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

		updatePointerCursor();
	});

	updatePointerCursor();
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

void ToolWindow::updatePointerCursor() {
	Tool * t = tm.getSelectedTool();
	std::string_view k{"data-tool"};
	std::string_view v = t ? t->getName() : "";
	std::string_view kSt{"data-tool-st"};
	std::string_view vSt = t ? toString(t->getToolVisualState()) : "";
	eui_root_attr_set(k.data(), k.size(), v.data(), v.size());
	eui_root_attr_set(kSt.data(), kSt.size(), vSt.data(), vSt.size());
}
