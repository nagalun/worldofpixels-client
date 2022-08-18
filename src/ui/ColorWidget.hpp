#pragma once

#include <cstdint>
#include <util/emsc/ui/AutoStacking.hpp>
#include <util/emsc/ui/EventHandle.hpp>
#include <util/emsc/ui/Button.hpp>

#include <ui/ColorPicker.hpp>
#include <ui/PaletteListWidget.hpp>

class ColorProvider;

class ColorWidget : public eui::Object {
	ColorProvider& clr;
	eui::Object pickerContainer;
	ColorPicker primaryClr;
	ColorPicker secondaryClr;
	eui::Button paletteBtn;
	eui::Button swapBtn;
	PaletteListWidget paletteWdg;

public:
	ColorWidget(ColorProvider&);

	void update();

private:
	void updatePrimaryClr();
	void updateSecondaryClr();
	bool swapColors();
	bool togglePaletteUi();
};

