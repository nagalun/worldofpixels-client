#include "ColorWidget.hpp"

#include <charconv>
#include <cstdio>

#include <tools/providers/ColorProvider.hpp>
#include <util/misc.hpp>
#include <util/color.hpp>
#include <util/explints.hpp>
#include <util/byteswap.hpp>
#include <util/emsc/dom.hpp>

// "#89ABCDEF" + null
constexpr std::size_t bufSz = 9 + 1;

ColorWidget::ColorWidget(ColorProvider& clr)
: clr(clr),
  primaryClr("input"),
  secondaryClr("input"),
  paletteBtn([this] { return true; }),
  swapBtn([this] { this->clr.swapColors(); return true; }),
  onPriColorChange(primaryClr.createHandler("change", std::bind(&ColorWidget::updatePrimaryClr, this))),
  onSecColorChange(secondaryClr.createHandler("change", std::bind(&ColorWidget::updateSecondaryClr, this))) {
	addClass("eui-wg");
	addClass("owop-colors");

	primaryClr.setProperty("type", "color");
	secondaryClr.setProperty("type", "color");

	primaryClr.setProperty("title", "Primary color");
	secondaryClr.setProperty("title", "Secondary color");
	paletteBtn.setProperty("title", "Palette");
	swapBtn.setProperty("title", "Swap colors");

	primaryClr.addClass("primary-clr");
	secondaryClr.addClass("secondary-clr");
	paletteBtn.addClass("palette-btn");
	swapBtn.addClass("swap-clr-btn");

	primaryClr.appendTo(*this);
	paletteBtn.appendTo(*this);
	swapBtn.appendTo(*this);
	secondaryClr.appendTo(*this);
}

void ColorWidget::update() {
	auto pClr = clr.getPrimaryColor();
	auto sClr = clr.getSecondaryColor();

	u32 cssPriClr = bswap_32(pClr.rgb);
	u32 cssSecClr = bswap_32(sClr.rgb);

	eui_root_css_property_set("--clr-pri", svprintf<bufSz>("#%08X", cssPriClr));
	eui_root_css_property_set("--clr-sec", svprintf<bufSz>("#%08X", cssSecClr));

	primaryClr.setProperty("value", svprintf<bufSz>("#%06X", (cssPriClr >> 8) & 0xFFFFFF));
	secondaryClr.setProperty("value", svprintf<bufSz>("#%06X", (cssSecClr >> 8) & 0xFFFFFF));
}

bool ColorWidget::updatePrimaryClr() {
	auto s = primaryClr.getProperty("value");

	RGB_u newClr;

	// + 1 to skip '#'
	auto res = std::from_chars<u32>(s.data() + 1, s.data() + s.size(), newClr.rgb, 16);
	if (res.ec == std::errc::invalid_argument || res.ec == std::errc::result_out_of_range) {
		return false;
	}

	newClr.rgb = bswap_32(newClr.rgb) >> 8;
	newClr.c.a = 255; // to be made adjustable

	clr.setPrimaryColor(newClr);

	return false;
}

bool ColorWidget::updateSecondaryClr() {
	auto s = secondaryClr.getProperty("value");

	RGB_u newClr;

	// + 1 to skip '#'
	auto res = std::from_chars<u32>(s.data() + 1, s.data() + s.size(), newClr.rgb, 16);
	if (res.ec == std::errc::invalid_argument || res.ec == std::errc::result_out_of_range) {
		return false;
	}

	newClr.rgb = bswap_32(newClr.rgb) >> 8;
	newClr.c.a = 255; // to be made adjustable

	clr.setSecondaryColor(newClr);

	return false;
}
