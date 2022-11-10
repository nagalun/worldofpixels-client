#include "ColorWidget.hpp"

#include <cstdio>

#include "tools/providers/ColorProvider.hpp"
#include "util/misc.hpp"
#include "util/color.hpp"
#include "util/byteswap.hpp"
#include "util/explints.hpp"
#include "util/emsc/dom.hpp"

ColorWidget::ColorWidget(ColorProvider& clr)
: clr(clr),
  primaryClr(std::bind(&ColorWidget::updatePrimaryClr, this)),
  secondaryClr(std::bind(&ColorWidget::updateSecondaryClr, this)),
  paletteBtn("palette", "Palettes", false),
  swapBtn("clr-swap", "Swap colors", false, std::bind(&ColorWidget::swapColors, this)) {

	addClass("eui-wg");
	addClass("owop-colors");

	primaryClr.setProperty("title", "Primary color");
	secondaryClr.setProperty("title", "Secondary color");

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

	eui_root_css_property_set("--clr-pri", svprintf("#%08X", cssPriClr));
	eui_root_css_property_set("--clr-sec", svprintf("#%08X", cssSecClr));

	primaryClr.setColor(pClr);
	secondaryClr.setColor(sClr);
}

void ColorWidget::setPaletteTglFn(std::function<void(void)> cb) {
	paletteBtn.setCb(std::move(cb));
}

void ColorWidget::updatePrimaryClr() {
	clr.setPrimaryColor(primaryClr.getColor());
}

void ColorWidget::updateSecondaryClr() {
	clr.setSecondaryColor(secondaryClr.getColor());
}

bool ColorWidget::swapColors() {
	clr.swapColors();
	return false;
}
