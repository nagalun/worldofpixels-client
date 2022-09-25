#include <ui/settings/SettingsWindow.hpp>

SettingsWindow::SettingsWindow() {
	tabs.appendTo(getContent());
	setTitle("Settings");
}

