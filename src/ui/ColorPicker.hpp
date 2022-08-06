#pragma once

#include <cstdint>
#include <functional>

#include <util/emsc/ui/Object.hpp>
#include <util/emsc/ui/EventHandle.hpp>
#include <util/color.hpp>

class ColorPicker : public eui::Object {
	eui::Object input;
	std::function<void(RGB_u)> cb;
	eui::EventHandle onColorChange;
	RGB_u color;

public:
	ColorPicker(std::function<void(RGB_u)> cb);

	void setColor(RGB_u);
	RGB_u getColor() const;

private:
	bool colorChanged();
	RGB_u readColor() const;
};

