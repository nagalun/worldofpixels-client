#pragma once

#include <cstdint>
#include <util/emsc/ui/AutoStacking.hpp>
#include <util/emsc/ui/EventHandle.hpp>
#include <util/NonCopyable.hpp>

#include <ui/misc/UiButton.hpp>
#include <ui/ColorPicker.hpp>
#include <ui/PaletteListWidget.hpp>

class ColorProvider;

class ColorWidget : public eui::Object, NonCopyable {
	ColorProvider& clr;
	ColorPicker primaryClr;
	ColorPicker secondaryClr;
	UiButton paletteBtn;
	UiButton swapBtn;

public:
	ColorWidget(ColorProvider&);

	void update();
	void setPaletteTglFn(std::function<void(void)>);

private:
	void updatePrimaryClr();
	void updateSecondaryClr();
	bool swapColors();
};

