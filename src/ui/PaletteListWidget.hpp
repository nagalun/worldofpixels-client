#pragma once

#include <cstdint>
#include <vector>

#include <util/emsc/ui/Object.hpp>

#include <ui/PaletteItem.hpp>

class PaletteListWidget : public eui::Object {
	eui::Object worldPaletteContainer;
	eui::Object worldPaletteSummary;
	eui::Object userPaletteContainer;
	eui::Object userPaletteSummary;
	std::vector<PaletteItem> worldPalettes;
	std::vector<PaletteItem> userPalettes;
	bool limitedPalettes;

public:
	PaletteListWidget();
};

