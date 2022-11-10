#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "util/Signal.hpp"
#include "util/emsc/ui/Object.hpp"
#include "util/emsc/ui/EventHandle.hpp"
#include "Settings.hpp"
#include "util/NonCopyable.hpp"

class ThemeManager;
using TmTheme = struct Theme; // ThemeManager's theme struct

class ThemeSettings : public eui::Object, NonCopyable {
	struct Theme : public eui::Object {
		std::string theme;

		eui::Object preview;
		eui::Object infoBox;
		eui::Object infoName;
		eui::Object infoDesc;
		eui::EventHandle onSelect;

		Theme(std::string_view, TmTheme*);
		Theme(Theme&&) noexcept;

		bool select();
	};

	std::vector<Theme> themeList;
	decltype(Settings::selectedTheme)::SlotKey skSelectedThemeCh;
	Signal<void(ThemeManager&)>::SlotKey skThemeListCh;
	Signal<void(TmTheme&)>::SlotKey skThemeLoad;

public:
	ThemeSettings();

	std::string_view getTabName() const;

private:
	void buildThemeList();
	void fillThemeInfo(TmTheme&);
	void updateActive(const std::string& newTheme);
};
