#pragma once

#include <cstdint>

#include "util/emsc/ui/Window.hpp"

#include "ui/misc/TabbedView.hpp"

#include "ui/settings/GeneralSettings.hpp"
#include "ui/settings/ThemeSettings.hpp"
#include "ui/settings/KeybindSettings.hpp"
#include "ui/settings/SocialSettings.hpp"
#include "ui/settings/WorldSettings.hpp"

class SettingsWindow : public eui::Window {
	TabbedView<GeneralSettings, ThemeSettings, KeybindSettings, SocialSettings, WorldSettings> tabs;

public:
	SettingsWindow();
};
