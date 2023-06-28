#include "ToolWindow.hpp"

#include "tools/Tool.hpp"
#include "tools/ToolManager.hpp"
#include "util/emsc/audio.hpp"
#include "util/emsc/dom.hpp"
#include "util/stringparser.hpp"
#include "util/misc.hpp"
#include <string_view>

ToolWindow::ToolWindow(ToolManager& _tm)
: Window(true, false),
  tm(_tm) {
	addClass("owop-tools");

	toolChSk = tm.onLocalStateChanged.connect([this] (ToolStates& ts, Tool* updatedTool) {
		Tool* newTool = tm.getSelectedTool();
		tm.forEachTool([&] (int i, Tool& t) {
			auto& btn = toolBtns[i];
			if (!newTool || *newTool != t) {
				btn.delClass("active");
			}

			if (newTool && *newTool == t) {
				btn.addClass("active");
			}

			btn.setProperty("dataset.tool", t.getToolVisualName(ts));
			std::uint16_t vs = t.getToolVisualState(ts);
			btn.setProperty("dataset.toolSt", svprintf("%d", vs));
		});

		updatePointerCursor(ts, newTool);
	});

	updatePointerCursor(tm.getLocalState(), tm.getSelectedTool());
	buildWindow();
	bringUp();
	move(3, 30);
}

void ToolWindow::buildWindow() {
	auto& cont = Window::getContent();
	Tool * selectedTool = tm.getSelectedTool();
	ToolStates& ts = tm.getLocalState();

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
		btn.setProperty("dataset.tool", t.getToolVisualName(ts));
		std::uint16_t vs = t.getToolVisualState(ts);
		btn.setProperty("dataset.toolSt", svprintf("%d", vs));

		btn.appendTo(cont);
	});
}

void ToolWindow::updatePointerCursor(ToolStates& ts, Tool * t) {
	std::string_view k{"data-tool"};
	std::string_view v = t ? t->getToolVisualName(ts) : "";
	std::string_view kSt{"data-tool-st"};
	std::string_view vSt = t ? toString(t->getToolVisualState(ts)) : "";
	eui_root_attr_set(k.data(), k.size(), v.data(), v.size());
	eui_root_attr_set(kSt.data(), kSt.size(), vSt.data(), vSt.size());
}
