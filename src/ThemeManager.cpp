#include "ThemeManager.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <vector>

#include "Settings.hpp"
#include "util/PngImage.hpp"
#include "util/async.hpp"
#include "util/color.hpp"
#include "util/emsc/dom.hpp"
#include "util/emsc/request.hpp"
#include "util/emsc/ui/BlobUrl.hpp"
#include "util/fast_blur.hpp"
#include "util/misc.hpp"

void Theme::setEnabled(bool s) {
	if (s && !enabled) {
		// generatedStyle.appendToHead();
		if (extraCss.has_value()) {
			extraCss->setDisabled(false);
		}
	} else if (!s && enabled) {
		// generatedStyle.remove();
		if (extraCss.has_value()) {
			extraCss->setDisabled(true);
		}
	}
	enabled = s;
}

Async<bool> Theme::generateStyle() {
	generatedStyle.setDisabled(true);
	generatedStyle.appendToHead();
	generatedStyle.deleteAllRules();
	generatedStyle.insertRuleBack(svprintf(
		R"(
[data-theme="%1$s"] {
	--tex-url-tools: url("/theme/%1$s/tools.png");
	--tex-url-tools-text: url("/theme/%1$s/tools_text.png");
	--tex-url-border: url("/theme/%1$s/border.png");
	--tex-url-border-active: url("/theme/%1$s/border_active.png");
	--tex-url-ui: url("/theme/%1$s/ui.png");
})",
		keyName.c_str()
	));

	for (const auto& icon : icons) {
		generatedStyle.insertRuleBack(svprintf(
			R"(
[data-theme="%s"] .owop-ui[data-i="%s"]::after {
	background-position: %dpx %dpx;
})",
			keyName.c_str(), icon.name.c_str(), -icon.column * iconSize, -icon.row * iconSize
		));
	}

	for (const auto& tool : tools) {
		generatedStyle.insertRuleBack(svprintf(
			R"(
[data-theme="%s"] .tool[data-tool="%s"]::after {
	background-position: %dpx %dpx;
})",
			keyName.c_str(), tool.name.c_str(), -tool.column * iconSize, 0
		));
	}

	{ // PROCESS TOOL ATLAS
		PngImage iToolset;
		{
			auto url = svprintf("/theme/%s/tools.png", keyName.data());
			auto [data, len, err] = co_await async_request(url.data());
			if (err) {
				std::printf("[ThemeManager] Couldn't load theme: %s returned error %d\n", url.data(), err);
				co_return false;
			}

			iToolset.readFileOnMem(reinterpret_cast<std::uint8_t*>(data.get()), len);
		}

		std::uint32_t nrows = iToolset.getHeight() / iconSize;

		PngImage iTool(iconSize, iconSize, {.rgb = 0});
		PngImage iToolTmp(iconSize, iconSize, {.rgb = 0});
		std::vector<std::uint8_t> fileBuf;

		for (std::uint32_t t = 0; t < tools.size(); t++) {
			auto& tool = tools[t];
			std::uint32_t col = tool.column;
			for (std::uint32_t row = tool.firstAsUiOnly ? 1 : 0; row < nrows; row++) {
				iTool.paste(0, 0, iToolset, false, col * iconSize, row * iconSize, iconSize, iconSize);
				if (iTool.isFullyTransparent()) {
					// stop on the first blank tool row
					break;
				}

				if (toolShadows != Theme::ShadowType::NONE) { /* TODO: resizing_render not yet implemented */
					iTool.applyTransform([this, &iTool](std::uint32_t x, std::uint32_t y) -> RGB_u {
						RGB_u src = shadowColor;
						RGB_u cur = iTool.getPixel(x, y);
						float sAf = src.c.a / 255.f;
						float dAf = cur.c.a / 255.f;
						float fAf = dAf * sAf; // blend using multiply, not source-over
						std::uint8_t fA = std::round(std::min(fAf, 1.f) * 255.f);
						return {{src.c.r, src.c.g, src.c.b, fA}};
					});

					iTool.move(shadowOffsX, shadowOffsY); // apply shadow offset
					fast_gaussian_blur_3(
						iTool.getData(), iToolTmp.getData(), iTool.getWidth(), iTool.getHeight(), iTool.getChannels(),
						shadowBlur
					);
					// now overlay the tool on top
					iTool.paste(0, 0, iToolset, true, col * iconSize, row * iconSize, iconSize, iconSize);
					// paste the resulting shadowed tool on the toolset, to use as atlas for rendering later
					// if shadows are not enabled it'd be the same so no need to do it always
					iToolset.paste(col * iconSize, row * iconSize, iTool, false);
				}

				fileBuf.clear();
				iTool.writeFileOnMem(fileBuf);
				auto& img =
					tool.cursorUrl.emplace_back(eui::BlobUrl::fromBuf(fileBuf.data(), fileBuf.size(), "image/png"));
				int state = tool.firstAsUiOnly ? row - 1 : row;
				// use the first tool state as fallback on unknown states
				std::string stateRule(state == 0 ? "[data-tool-st]" : svprintf(R"([data-tool-st="%d"])", state));
				// use as cursor when selected
				generatedStyle.insertRuleBack(svprintf(
					R"(
[data-theme="%s"][data-tool="%s"]%s #input {
	cursor: url("%s") %d %d, auto;
})",
					keyName.c_str(), tool.name.c_str(), stateRule.c_str(), img.get().data(), tool.hotspotX,
					tool.hotspotY
				));
			}
		}

		fileBuf.clear();
		iToolset.writeFileOnMem(fileBuf);
		toolAtlasUrl = eui::BlobUrl::fromBuf(fileBuf.data(), fileBuf.size(), "image/png");
	}

	generatedStyle.setDisabled(false);
	co_return true;
}

void Theme::loadExtraCss() {
	if (!hasCss || extraCss.has_value()) {
		return;
	}

	extraCss.emplace(svprintf("/theme/%s/style.css", keyName.data()));
	if (enabled) {
		extraCss->appendToHead();
	}
}

ThemeManager::ThemeManager()
: currentTheme(nullptr),
  ready(false) {
	skSelectedThemeCh = Settings::get().selectedTheme.connect([this] (auto newTheme) {
		loadCurrentTheme(true);
	});

	loadThemeList();
}

ThemeManager& ThemeManager::get() {
	static ThemeManager tm;
	return tm;
}

const std::vector<std::string>& ThemeManager::getAvailableThemes() const {
	return availableThemes;
}

bool ThemeManager::isThemeAvailable(std::string_view t) const {
	return std::find(availableThemes.begin(), availableThemes.end(), t) != availableThemes.end();
}

bool ThemeManager::isThemeSelected(std::string_view t) const {
	return currentTheme && currentTheme->keyName == t;
}

Async<bool> ThemeManager::switchTheme(std::string_view key) {
	if (isThemeSelected(key)) { // if it's already loaded ignore
		co_return true;
	}

	Theme* t = co_await getOrLoadTheme(key);
	if (!t) {
		co_return false;
	}

	if (currentTheme) {
		currentTheme->setEnabled(false);
	}

	if (t) {
		currentTheme = t;
		currentTheme->setEnabled(true);
	}

	std::string_view prop{"data-theme"};
	eui_root_attr_set(prop.data(), prop.size(), currentTheme->keyName.c_str(), currentTheme->keyName.size());

	co_return true;
}


Async<> ThemeManager::loadThemeList() {
	availableThemes.clear();
	co_await loadUserThemeList();
	co_await loadBuiltinThemeList();
	onThemeListChanged(*this);
	co_await loadCurrentTheme(true);
}

Async<> ThemeManager::loadBuiltinThemeList() {
	auto [data, len, err] = co_await async_request("/theme/builtin.json");

	auto j(nlohmann::json::parse(data.get(), data.get() + len, nullptr, false));
	if (!j.is_discarded() && j.is_array()) {
		for (const auto& v : j) {
			if (!v.is_string()) {
				continue;
			}

			availableThemes.emplace_back(v.get<std::string>());
		}
	}

	if (availableThemes.empty()) {
		availableThemes.emplace_back("default");
	}
}

Async<> ThemeManager::loadUserThemeList() {
	co_return;
}

Async<bool> ThemeManager::loadCurrentTheme(bool resetIfNotAvailable) {
	auto& keyName = Settings::get().selectedTheme;

	Theme* t = co_await getOrLoadTheme(keyName.get());
	if (!t && resetIfNotAvailable && availableThemes.size() != 0) {
		keyName.setBlocked(true);
		keyName = availableThemes[0];
		keyName.setBlocked(false);
	}

	co_return co_await switchTheme(keyName.get());
}

Async<Theme*> ThemeManager::getOrLoadTheme(std::string_view keyName) {

	if (Theme* t = getTheme(keyName)) {
		co_return t;
	}

	if (!isThemeAvailable(keyName)) {
		co_return nullptr;
	}

	Theme* resultingTheme = nullptr;
	{
		auto [data, len, err] = co_await async_request(svprintf("/theme/%s/theme.json", keyName.data()).data());
		auto j(nlohmann::json::parse(data.get(), data.get() + len, nullptr, false));

		if (j.is_discarded() || !j.is_object()) {
			co_return nullptr;
		}

		// TODO: crash-proof this (without exceptions it's painful)
		// TODO: check input strings css safety (prevent css injection)
		// TODO: check if a theme is currently being loaded
		auto empty = nlohmann::json::object();
		auto emptyArr = nlohmann::json::array();

		auto uiIcons = j.contains("ui") && j["ui"].is_array() ? j["ui"] : emptyArr;
		auto tools = j.contains("tools") && j["tools"].is_object() ? j["tools"] : empty;
		auto toolDefs =
			tools.contains("definitions") && tools["definitions"].is_array() ? tools["definitions"] : emptyArr;
		auto shadow = tools.contains("shadow_opts") && tools["shadow_opts"].is_object() ? tools["shadow_opts"] : empty;

		using ST = Theme::ShadowType;
		Theme t{
			.keyName{keyName},
			.name{j.value<std::string>("theme_name", "Unknown")},
			.description{j.value<std::string>("theme_desc", "A cool theme.")},
			.iconSize = j.value<std::uint16_t>("icon_size_px", 32),
			.hasCss = j.value<bool>("has_css", false),
			.toolShadows = shadow.value<bool>("enable", true)
		                       ? (shadow.value<bool>("allow_growing_tex", false) ? ST::RESIZING_RENDER : ST::RENDER)
		                       : ST::NONE,
			.shadowBlur = shadow.value<std::uint8_t>("blur", 1),
			.shadowOffsX = shadow.value<std::int8_t>("offset_x", 1),
			.shadowOffsY = shadow.value<std::int8_t>("offset_y", 1),
			.shadowColor = read_css_hex_color(shadow.value<std::string_view>("color", "#0000007F"))};

		for (auto [it, col] = std::tuple{toolDefs.begin(), std::uint16_t{0}}; it != toolDefs.end(); it++, col++) {
			auto& tool = *it;
			if (!tool.contains("name") || !tool["name"].is_string()) {
				continue;
			}

			std::uint16_t hx = 0;
			std::uint16_t hy = 0;
			bool firstUi = false;
			if (tool.contains("hotspot") && tool["hotspot"].is_array() && tool["hotspot"].size() == 2) {
				auto hs = tool["hotspot"];
				if (hs[0].is_number_unsigned()) hx = hs[0].get<std::uint16_t>();
				if (hs[1].is_number_unsigned()) hy = hs[1].get<std::uint16_t>();
			}

			if (tool.contains("first_as_ui_only") && tool["first_as_ui_only"].is_boolean()) {
				firstUi = tool["first_as_ui_only"].get<bool>();
			}

			t.tools.emplace_back(Theme::ToolVisualInfo{
				.name{tool["name"].get<std::string>()},
				.column = col,
				.hotspotX = hx,
				.hotspotY = hy,
				.firstAsUiOnly = firstUi});
		}

		for (auto [it, row] = std::tuple{uiIcons.begin(), std::uint16_t{0}}; it != uiIcons.end(); it++, row++) {
			if (!it->is_array()) {
				continue;
			}

			for (auto [itCol, col] = std::tuple{it->begin(), std::uint16_t{0}}; itCol != it->end(); itCol++, col++) {
				if (!itCol->is_string()) {
					continue;
				}

				t.icons.emplace_back(Theme::UiIconInfo{.name = itCol->get<std::string>(), .row = row, .column = col});
			}
		}

		std::string currThemeKey;
		// if the vector is going to be resized, the pointer to the currentTheme will change
		if (currentTheme && loadedThemes.capacity() == loadedThemes.size()) {
			currThemeKey = currentTheme->keyName;
		}

		resultingTheme = &loadedThemes.emplace_back(std::move(t));

		if (!currThemeKey.empty()) {
			currentTheme = getTheme(currThemeKey);
		}
	}

	co_await resultingTheme->generateStyle();
	onThemeLoaded(*resultingTheme);
	co_return resultingTheme;
}

Theme* ThemeManager::getTheme(std::string_view t) {
	auto it = std::find_if(loadedThemes.begin(), loadedThemes.end(), [t] (const Theme& theme) {
		return theme.keyName == t;
	});

	return it != loadedThemes.end() ? &*it : nullptr;
}
