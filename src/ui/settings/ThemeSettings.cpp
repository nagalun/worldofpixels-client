#include "ui/settings/ThemeSettings.hpp"
#include "ThemeManager.hpp"

#include <functional>

using namespace std::string_view_literals;

ThemeSettings::Theme::Theme(std::string_view _theme, TmTheme* themeObj)
: theme(_theme),
  infoName("h1") {
	onSelect = createHandler("click", std::bind(&Theme::select, this), false);

	preview.addClass("item-theme-pv");
	infoBox.addClass("item-theme-info");
	infoName.addClass("info-theme-name");
	infoDesc.addClass("info-theme-desc");

	if (themeObj) {
		infoName.setProperty("textContent", themeObj->name);
		infoDesc.setProperty("textContent", themeObj->description);
	} else {
		infoName.setProperty("textContent", theme);
		infoDesc.setProperty("textContent", "Click to load (for now)");
	}

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
  theme(std::move(o.theme)),
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

	auto& tm = ThemeManager::get();
	buildThemeList();

	skThemeListCh = tm.onThemeListChanged.connect([this] (ThemeManager&) {
		buildThemeList();
	});

	skThemeLoad = tm.onThemeLoaded.connect([this] (TmTheme& th) {
		fillThemeInfo(th);
	});

	skSelectedThemeCh = Settings::get().selectedTheme.connect([this] (auto newTheme) {
		updateActive(newTheme);
	});

	addClass("settings-themes");
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

void ThemeSettings::fillThemeInfo(TmTheme& th) {
	for (auto& t : themeList) {
		if (t.theme == th.keyName) {
			t.infoName.setProperty("textContent", th.name);
			t.infoDesc.setProperty("textContent", th.description);
			break;
		}
	}
}

void ThemeSettings::buildThemeList() {
	themeList.clear(); // quite wasteful of elements
	const auto& sel = Settings::get().selectedTheme.get();
	auto& tm = ThemeManager::get();
	for (auto s : tm.getAvailableThemes()) {
		auto& item = themeList.emplace_back(s, tm.getTheme(s));
		item.appendTo(*this);

		if (s == sel) {
			item.addClass("active");
		}
	}
}
