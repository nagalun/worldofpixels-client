#include <ui/ToolWindow.hpp>

static eui::Object mkToolsTitle() {
	eui::Object title("img");
	title.setProperty("src", "/img/ui/default/tools_text.png");
	title.setProperty("alt", "Tools");
	//title.setProperty("draggable", "false");

	return title;
}

ToolWindow::ToolWindow()
: Window({mkToolsTitle()}) {
	addClass("owop-tools");
}

