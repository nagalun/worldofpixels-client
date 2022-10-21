#include "ui/settings/ThemeSettings.hpp"

#include <functional>

using namespace std::string_view_literals;

ThemeSettings::Theme::Theme(std::string_view _theme)
: theme(_theme),
  infoName("h1") {
	onSelect = createHandler("click", std::bind(&Theme::select, this), false);

	preview.addClass("item-theme-pv");
	infoBox.addClass("item-theme-info");
	infoName.addClass("info-theme-name");
	infoDesc.addClass("info-theme-desc");

	infoName.setProperty("textContent", theme);
	infoDesc.setProperty("textContent", "theme description goes here");

	infoName.appendTo(infoBox);
	infoDesc.appendTo(infoBox);

	preview.appendTo(*this);
	infoBox.appendTo(*this);

	addClass("item-theme");
	addClass("eui-themed");
	setAttribute("data-theme", theme);
}

ThemeSettings::Theme::Theme(Theme&& o) noexcept
: eui::Object(std::move(o)),
  theme(o.theme),
  preview(std::move(o.preview)),
  infoBox(std::move(o.infoBox)),
  infoName(std::move(o.infoName)),
  infoDesc(std::move(o.infoDesc)),
  onSelect(std::move(o.onSelect)) {
	onSelect.setCb(std::bind(&Theme::select, this));
}

bool ThemeSettings::Theme::select() {
	Settings::get().selectedTheme = theme;
	return false;
}

ThemeSettings::ThemeSettings() {

	for (auto s : {"default"sv, "easter"sv, "halloween"sv, "newyear"sv}) {
		themeList.emplace_back(s).appendTo(*this);
	}

	onSelectedThemeCh = Settings::get().selectedTheme.connect([this] (auto newTheme) {
		updateActive(newTheme);
	});

	addClass("settings-themes");
	updateActive(Settings::get().selectedTheme);
}

std::string_view ThemeSettings::getTabName() const {
	return "Themes";
}

void ThemeSettings::updateActive(const std::string& newTheme) {
	for (auto& t : themeList) {
		if (t.theme == newTheme) {
			t.addClass("active");
		} else {
			t.delClass("active");
		}
	}
}
