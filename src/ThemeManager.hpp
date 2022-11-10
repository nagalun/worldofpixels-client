#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "Settings.hpp"
#include "util/NonCopyable.hpp"
#include "util/Signal.hpp"
#include "util/async.hpp"
#include "util/color.hpp"
#include "util/emsc/ui/BlobUrl.hpp"
#include "util/emsc/ui/Stylesheet.hpp"

struct Theme {
	enum class ShadowType : std::uint8_t { NONE, RENDER, RESIZING_RENDER };

	struct ToolVisualInfo {
		std::vector<eui::BlobUrl> cursorUrl; // urls to every processed tool state
		std::string name;
		std::uint16_t column;
		std::uint16_t hotspotX;
		std::uint16_t hotspotY;
		// processed atlas info (shadowed)
		// std::uint16_t procHotspotX;
		// std::uint16_t procHotspotY;
		// std::uint16_t procAtlasX;
		// std::uint16_t procAtlasY;
		// std::uint16_t procAtlasW;
		// std::uint16_t procAtlasH;
		bool firstAsUiOnly;
	};

	struct UiIconInfo {
		std::string name;
		std::uint16_t row;
		std::uint16_t column;
	};

	std::string keyName;
	std::string name;
	std::string description;
	std::uint16_t iconSize;
	bool hasCss;
	ShadowType toolShadows;
	std::uint8_t shadowBlur;
	std::int8_t shadowOffsX;
	std::int8_t shadowOffsY;
	bool enabled = false;
	RGB_u shadowColor;

	eui::Stylesheet generatedStyle;
	std::optional<eui::Stylesheet> extraCss;

	std::vector<ToolVisualInfo> tools;
	std::vector<UiIconInfo> icons;

	eui::BlobUrl toolAtlasUrl;

	void setEnabled(bool s);
	Async<bool> generateStyle();
	void loadExtraCss();
};

class ThemeManager : NonCopyable {
	decltype(Settings::selectedTheme)::SlotKey skSelectedThemeCh;
	std::vector<std::string> availableThemes;
	std::vector<Theme> loadedThemes;
	Theme* currentTheme;
	bool ready;

	ThemeManager();

public:
	static ThemeManager& get();

	const std::vector<std::string>& getAvailableThemes() const;
	bool isThemeAvailable(std::string_view) const;
	bool isThemeSelected(std::string_view) const;
	Async<bool> switchTheme(std::string_view);
	Async<Theme*> getOrLoadTheme(std::string_view);
	Theme* getTheme(std::string_view);

	Signal<void(ThemeManager&)> onThemeListChanged;
	Signal<void(Theme&)> onThemeLoaded;

private:
	Async<> loadThemeList();
	Async<> loadBuiltinThemeList();
	Async<> loadUserThemeList();

	Async<bool> loadCurrentTheme(bool resetIfNotAvailable);

};
