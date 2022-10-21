#pragma once

#include <cstdint>
#include <vector>

#include <util/emsc/ui/Object.hpp>
#include <util/emsc/ui/Button.hpp>
#include <util/color.hpp>

class ColorProvider;

class PaletteItem : public eui::Object {
public:
	class Color : public eui::Button {
		RGB_u color;

	public:
		template<typename Fn>
		Color(RGB_u, Fn onSelect);

		RGB_u getColor();
	};

private:
	ColorProvider& clrp;
	std::vector<Color> colors;

public:
	PaletteItem(ColorProvider&, std::vector<RGB_u>);
	PaletteItem(PaletteItem&&) noexcept;
};

