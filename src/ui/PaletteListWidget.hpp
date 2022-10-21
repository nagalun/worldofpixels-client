#pragma once

#include <cstdint>
#include <vector>

#include "util/emsc/ui/Object.hpp"
#include "util/color.hpp"

#include "ui/PaletteItem.hpp"

class ColorProvider;

class PaletteListWidget : public eui::Object {
	ColorProvider& clr;
	eui::Object worldPaletteContainer;
	eui::Object worldPaletteSummary;
	eui::Object userPaletteContainer;
	eui::Object userPaletteSummary;
	std::vector<PaletteItem> worldPalettes;
	std::vector<PaletteItem> userPalettes;
	bool limitedPalettes;

public:
	PaletteListWidget(ColorProvider&);

	void setPalettes(std::vector<std::vector<RGB_u>> world, std::vector<std::vector<RGB_u>> user);
};

