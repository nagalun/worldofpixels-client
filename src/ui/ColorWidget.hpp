#pragma once

#include <cstdint>

#include "tools/providers/ColorProvider.hpp"
#include "ui/misc/UiButton.hpp"
#include "ui/ColorPicker.hpp"
#include "util/NonCopyable.hpp"

class ToolManager;

class ColorWidget : public eui::Object, NonCopyable {
	ToolManager& tm;
	ColorProvider::State& clr;
	ColorPicker primaryClr;
	ColorPicker secondaryClr;
	UiButton paletteBtn;
	UiButton swapBtn;

public:
	ColorWidget(ToolManager& tm, ColorProvider::State&);

	void update();
	void setPaletteTglFn(std::function<void(void)>);

private:
	void updatePrimaryClr();
	void updateSecondaryClr();
	bool swapColors();
};
