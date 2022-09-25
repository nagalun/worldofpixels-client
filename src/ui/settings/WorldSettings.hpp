#pragma once

#include <cstdint>
#include <string_view>

#include <util/emsc/ui/Object.hpp>

#include <ui/misc/TabbedView.hpp>

#include <ui/settings/world/WorldChatSettings.hpp>
#include <ui/settings/world/WorldRolesSettings.hpp>
#include <ui/settings/world/WorldAudioSettings.hpp>
#include <ui/settings/world/WorldThemeSettings.hpp>
#include <ui/settings/world/WorldSecuritySettings.hpp>

class WorldSettings: public eui::Object {
	TabbedView<WorldChatSettings, WorldRolesSettings, WorldAudioSettings, WorldThemeSettings, WorldSecuritySettings> tabs;

public:
	WorldSettings();

	std::string_view getTabName() const;
};

