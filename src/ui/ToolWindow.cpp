#include "ToolWindow.hpp"

#include <tools/ToolManager.hpp>
#include <util/emsc/audio.hpp>

#include <emscripten/html5.h>


// assume only one ToolWindow will be active at a time
static ToolWindow * tw = nullptr;

ToolWindow::ToolWindow(ToolManager& tm)
: Window(),
  tm(tm) {
	addClass("owop-tools");
	tw = this;

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
}


void ToolWindow::buildWindow() {
	auto& cont = Window::getContent();
	Tool * selectedTool = tm.getSelectedTool();

	toolBtns.clear();
	tm.forEachTool([&] (int i, Tool& t) {
		Object btn("button");
		btn.addClass("tool");

		if (selectedTool && *selectedTool == t) {
			btn.addClass("active");
		}

		btn.setProperty("title", t.getName());
		btn.setProperty("dataset.tool", t.getName());

		btn.appendTo(cont);

		// give vector index as user data (so not actually a pointer) to identify what tool was clicked
		emscripten_set_click_callback(btn.getSelector().data(), reinterpret_cast<void*>(i), false, ToolWindow::selectToolEvent);

		toolBtns.emplace_back(std::move(btn));
	});
}

int ToolWindow::selectToolEvent(int type, const EmscriptenMouseEvent *ev, void *data) {
	int idx = reinterpret_cast<int>(data);

	// lazy way
	tw->tm.forEachTool([idx] (int i, Tool& t) {
		if (i == idx) {
			tw->tm.selectTool(&t);
		}
	});

	playAudioId("a-btn");
	return true;
}
