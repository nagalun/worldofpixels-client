#pragma once

#include <cstdint>
#include <util/emsc/ui/AutoStacking.hpp>
#include <util/emsc/ui/EventHandle.hpp>
#include <util/emsc/ui/Button.hpp>

class ColorProvider;

class ColorWidget : public eui::Object {
	ColorProvider& clr;
	eui::Object primaryClr;
	eui::Object secondaryClr;
	eui::Button paletteBtn;
	eui::Button swapBtn;
	eui::EventHandle onPriColorChange;
	eui::EventHandle onSecColorChange;

public:
	ColorWidget(ColorProvider&);

	void update();

private:
	bool updatePrimaryClr();
	bool updateSecondaryClr();
};

