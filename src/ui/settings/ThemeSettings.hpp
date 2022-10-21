#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "util/emsc/ui/Object.hpp"
#include "util/emsc/ui/EventHandle.hpp"
#include "Settings.hpp"
#include "util/NonCopyable.hpp"

class ThemeSettings : public eui::Object, NonCopyable {
	class Theme : public eui::Object {
	public:
		const std::string_view theme;

	private:
		eui::Object preview;
		eui::Object infoBox;
		eui::Object infoName;
		eui::Object infoDesc;
		eui::EventHandle onSelect;

	public:
		Theme(std::string_view);
		Theme(Theme&&) noexcept;

		bool select();
	};

	std::vector<Theme> themeList;
	decltype(Settings::selectedTheme)::SlotKey onSelectedThemeCh;

public:
	ThemeSettings();

	std::string_view getTabName() const;

private:
	void updateActive(const std::string& newTheme);
};
