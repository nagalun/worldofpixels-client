#pragma once

#include <cstdint>
#include <util/emsc/ui/AutoStacking.hpp>
#include <util/emsc/ui/EventHandle.hpp>
#include <util/emsc/ui/Button.hpp>

#include <ui/ColorPicker.hpp>

class ColorProvider;

class ColorWidget : public eui::Object {
	ColorProvider& clr;
	ColorPicker primaryClr;
	ColorPicker secondaryClr;
	eui::Button paletteBtn;
	eui::Button swapBtn;

public:
	ColorWidget(ColorProvider&);

	void update();

private:
	void updatePrimaryClr();
	void updateSecondaryClr();
	bool swapColors();
	bool togglePaletteUi();
};

