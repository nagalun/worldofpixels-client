#include "ThemeManager.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <string_view>
#include <vector>

#include <json/jute.h>

#include "Settings.hpp"
#include "util/PngImage.hpp"
#include "util/async.hpp"
#include "util/color.hpp"
#include "util/emsc/dom.hpp"
#include "util/emsc/request.hpp"
#include "util/emsc/ui/BlobUrl.hpp"
#include "util/fast_blur.hpp"
#include "util/misc.hpp"

const Theme::ToolStateInfo& Theme::ToolVisualInfo::getState(std::uint8_t st) const {
	return st < states.size() ? states[st] : states[0];
}

void Theme::setEnabled(bool s) {
	if (s && !enabled) {
		if (extraCss.has_value()) {
			extraCss->setDisabled(false);
		}
	} else if (!s && enabled) {
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
	--tex-isz-tools: %2$dpx;
	--tex-isz-ui: %3$dpx;
})",
		keyName.c_str(), toolSize, iconSize
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

		std::uint32_t nrows = iToolset.getHeight() / toolSize;

		int iShadowBlur = shadowBlur;
		std::int32_t leftPad = 0;
		std::int32_t topPad = 0;
		std::size_t maxToolSize = toolSize;

		if (toolShadows != Theme::ShadowType::NONE) {
			maxToolSize += std::max(std::abs(shadowOffsX), std::abs(shadowOffsY)) + iShadowBlur * 2;
			leftPad = std::max(iShadowBlur - shadowOffsX, 0);
			topPad = std::max(iShadowBlur - shadowOffsY, 0);
		}

		// make sure all the shadowed tools are going to fit in the toolset
		PngImage iFinalToolset(tools.size() * maxToolSize, nrows * maxToolSize, {.rgb = 0});
		PngImage iTool(maxToolSize, maxToolSize, {.rgb = 0});
		PngImage iToolTmp(maxToolSize, maxToolSize, {.rgb = 0});
		std::vector<std::uint8_t> fileBuf;

		for (std::uint32_t t = 0; t < tools.size(); t++) {
			auto& tool = tools[t];
			std::uint32_t col = tool.column;
			for (std::uint32_t row = tool.firstAsUiOnly ? 1 : 0; row < nrows; row++) {

				iTool.allocate(maxToolSize, maxToolSize, {.rgb = 0});
				iTool.paste(leftPad, topPad, iToolset, false, col * toolSize, row * toolSize, toolSize, toolSize);
				if (iTool.isFullyTransparent()) {
					// stop on the first blank tool row
					break;
				}

				if (toolShadows != Theme::ShadowType::NONE) {
					iTool.applyTransform([this, &iTool](std::uint32_t x, std::uint32_t y) -> RGB_u {
						RGB_u src = shadowColor;
						RGB_u cur = iTool.getPixel(x, y);
						float sAf = src.c.a / 255.f;
						float dAf = cur.c.a / 255.f;
						float fAf = dAf * sAf; // blend using multiply, not source-over
						std::uint8_t fA = std::round(std::min(fAf, 1.f) * 255.f);
						return {{src.c.r, src.c.g, src.c.b, fA}};
					});

					float sigma = shadowBlur;
					iTool.move(shadowOffsX, shadowOffsY); // apply shadow offset
					fast_gaussian_blur_3(
						iTool.getData(), iToolTmp.getData(), iTool.getWidth(), iTool.getHeight(), iTool.getChannels(),
						sigma / 2.f
					);
					// now overlay the tool on top
					iTool.paste(leftPad, topPad, iToolset, true, col * toolSize, row * toolSize, toolSize, toolSize);
				}

				// remove empty space from the image
				auto [xoff, yoff] = iTool.fitToContent();
				//auto [xoff, yoff] = std::make_pair(0,0);

				std::uint16_t fxAltasX = col * maxToolSize;
				std::uint16_t fxAtlasY = row * maxToolSize;
				std::uint16_t fxAtlasW = iTool.getWidth();
				std::uint16_t fxAtlasH = iTool.getHeight();

				// calculate new hotspot
				std::uint16_t fxHotspotX = std::max(leftPad + tool.hotspotX + xoff, 0);
				std::uint16_t fxHotspotY = std::max(topPad + tool.hotspotY + yoff, 0);

				// paste the resulting tool texture on the toolset, to use as atlas for rendering later
				iFinalToolset.paste(fxAltasX, fxAtlasY, iTool, false);

				iTool.writeFileOnMem(fileBuf);

				auto& img = tool.states.emplace_back(ToolStateInfo{
					eui::BlobUrl::fromBuf(fileBuf.data(), fileBuf.size(), "image/png"),
					fxHotspotX,
					fxHotspotY,
					fxAltasX,
					fxAtlasY,
					fxAtlasW,
					fxAtlasH
				});

				int state = tool.firstAsUiOnly ? row - 1 : row;
				// use the first tool state as fallback on unknown states
				std::string stateRule(state == 0 ? "[data-tool-st]" : svprintf(R"([data-tool-st="%d"])", state));
				// use as cursor when selected
				generatedStyle.insertRuleBack(svprintf(
					R"(
[data-theme="%s"][data-tool="%s"]%s #input {
	cursor: url("%s") %d %d, auto;
})",
					keyName.c_str(), tool.name.c_str(), stateRule.c_str(), img.fxTexUrl.get().data(),
					img.fxHotspotX, img.fxHotspotY
				));
			}
		}

		fxToolAtlas = std::move(iFinalToolset);
	}

	std::string cssCurUrls;
	for (const auto& tool : tools) {
		// since these rules will be for UI, if the texture has specified that the first row is for ui only
		// we don't generate the following states.
		std::size_t numRows = tool.states.size();
		std::size_t numRowsUi = tool.firstAsUiOnly ? !tool.states.empty() : numRows;

		for (std::uint32_t row = 0; row < numRowsUi; row++) {
			std::string stateRule(row == 0 ? "[data-tool-st]" : svprintf(R"([data-tool-st="%d"])", row));
			generatedStyle.insertRuleBack(svprintf(
				R"(
[data-theme="%s"] .tool[data-tool="%s"]%s::after {
	background-position: %dpx %dpx;
})",
				keyName.c_str(), tool.name.c_str(), stateRule.c_str(), -tool.column * toolSize, -row * toolSize
			));
		}

		for (std::uint32_t row = 0; row < numRows; row++) {
			cssCurUrls += "url(\"";
			cssCurUrls.append(tool.states[row].fxTexUrl.get());
			cssCurUrls += "\") ";
		}
	}

	// preload cursor textures to avoid incorrectly showing default cursor in some edge cases
	generatedStyle.insertRuleBack(svprintf(
		R"(
[data-theme="%s"] #game-data::before {
	content: %s;
})",
		keyName.c_str(), cssCurUrls.c_str()
	));

	// TODO: enable depending on user choice
	loadExtraCss();
	generatedStyle.setDisabled(false);
	co_return true;
}

void Theme::loadExtraCss() {
	if (!hasCss || extraCss.has_value()) {
		return;
	}

	extraCss.emplace(svprintf("/theme/%s/style.css", keyName.data()));
	extraCss->setDisabled(!enabled);
	extraCss->appendToHead();
}

Theme::ToolVisualInfo* Theme::getToolInfo(std::string_view tname) {
	auto it = std::find_if(tools.begin(), tools.end(), [tname] (const ToolVisualInfo& tinfo) {
		return tinfo.name == tname;
	});
	return it != tools.end() ? &*it : nullptr;
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

	availableThemes.emplace_back("default");

	auto j(jute::parser::parse(std::string_view(data.get(), data.get() + len)));
	if (j.is_array()) {
		for (const auto& v : j) {
			if (!v.is_string()) {
				continue;
			}

			auto str = v.as_string();
			if (str != "default") {
				availableThemes.emplace_back(str);
			}
		}
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
		auto j(jute::parser::parse(std::string_view(data.get(), data.get() + len)));

		if (!j.is_object()) {
			co_return nullptr;
		}

		// TODO: crash-proof this (without exceptions it's painful)
		// TODO: check input strings css safety (prevent css injection)
		// TODO: check if a theme is currently being loaded

		auto uiIcons = j["ui"];
		auto toolDefs = j["tools"]["definitions"];
		auto shadow = j["tools"]["shadow_opts"];

		using ST = Theme::ShadowType;
		Theme t{
			.keyName{keyName},
			.name{j["theme_name"].as_string("Unknown")},
			.description{j["theme_desc"].as_string("A cool theme.")},
			.iconSize = j["icon_size_px"].as_int<std::uint16_t>(32),
			.toolSize = j["tool_size_px"].as_int<std::uint16_t>(j["icon_size_px"].as_int<std::uint16_t>(32)),
			.hasCss = j["has_css"].as_bool(false),
			.toolShadows = shadow["enable"].as_bool(true)
		                       ? (shadow["allow_growing_tex"].as_bool(false) ? ST::RESIZING_RENDER : ST::RENDER)
		                       : ST::NONE,
			.shadowBlur = shadow["blur"].as_int<std::uint8_t>(1),
			.shadowOffsX = shadow["offset_x"].as_int<std::int8_t>(1),
			.shadowOffsY = shadow["offset_y"].as_int<std::int8_t>(1),
			.shadowColor = color_from_css_hex(shadow["color"].as_string("#0000007F"))};

		for (auto [it, col] = std::tuple{toolDefs.begin(), std::uint16_t{0}}; it != toolDefs.end(); it++, col++) {
			auto& tool = *it;
			if (!tool["name"].is_string()) {
				continue;
			}

			t.tools.emplace_back(Theme::ToolVisualInfo{
				.name{tool["name"].as_string()},
				.column = col,
				.hotspotX = tool["hotspot"][0].as_int<std::uint16_t>(0),
				.hotspotY = tool["hotspot"][1].as_int<std::uint16_t>(0),
				.firstAsUiOnly = tool["first_as_ui_only"].as_bool(false)});
		}

		for (auto [it, row] = std::tuple{uiIcons.begin(), std::uint16_t{0}}; it != uiIcons.end(); it++, row++) {
			if (!it->is_array()) {
				continue;
			}

			for (auto [itCol, col] = std::tuple{it->begin(), std::uint16_t{0}}; itCol != it->end(); itCol++, col++) {
				if (!itCol->is_string()) {
					continue;
				}

				t.icons.emplace_back(Theme::UiIconInfo{.name = itCol->as_string(), .row = row, .column = col});
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

Theme* ThemeManager::getCurrentTheme() {
	return currentTheme;
}
