#include "ColorWidget.hpp"

#include <cstdio>

#include <tools/providers/ColorProvider.hpp>
#include <util/misc.hpp>
#include <util/color.hpp>
#include <util/byteswap.hpp>
#include <util/explints.hpp>
#include <util/emsc/dom.hpp>

// "'#89ABCDEF'"
constexpr std::size_t bufSz = 11;

ColorWidget::ColorWidget(ColorProvider& clr)
: clr(clr),
  primaryClr(std::bind(&ColorWidget::updatePrimaryClr, this)),
  secondaryClr(std::bind(&ColorWidget::updateSecondaryClr, this)),
  paletteBtn(std::bind(&ColorWidget::togglePaletteUi, this)),
  swapBtn(std::bind(&ColorWidget::swapColors, this)),
  paletteWdg(clr),
  paletteWdgShown(true) {
	addClass("owop-clr-ui");

	pickerContainer.addClass("eui-wg");
	pickerContainer.addClass("owop-colors");

	primaryClr.setProperty("title", "Primary color");
	secondaryClr.setProperty("title", "Secondary color");
	paletteBtn.setProperty("title", "Palettes");
	swapBtn.setProperty("title", "Swap colors");

	primaryClr.addClass("primary-clr");
	secondaryClr.addClass("secondary-clr");
	paletteBtn.addClass("palette-btn");
	swapBtn.addClass("swap-clr-btn");

	primaryClr.appendTo(pickerContainer);
	paletteBtn.appendTo(pickerContainer);
	swapBtn.appendTo(pickerContainer);
	secondaryClr.appendTo(pickerContainer);

	pickerContainer.appendTo(*this);
	paletteWdg.appendTo(*this);

	appendToMainContainer();
}

void ColorWidget::update() {
	auto pClr = clr.getPrimaryColor();
	auto sClr = clr.getSecondaryColor();

	u32 cssPriClr = bswap_32(pClr.rgb);
	u32 cssSecClr = bswap_32(sClr.rgb);

	eui_root_css_property_set("--clr-pri", svprintf<bufSz>("#%08X", cssPriClr));
	eui_root_css_property_set("--clr-sec", svprintf<bufSz>("#%08X", cssSecClr));

	primaryClr.setColor(pClr);
	secondaryClr.setColor(sClr);
}

void ColorWidget::updatePrimaryClr() {
	clr.setPrimaryColor(primaryClr.getColor());
}

void ColorWidget::updateSecondaryClr() {
	clr.setSecondaryColor(secondaryClr.getColor());
}

bool ColorWidget::swapColors() {
	clr.swapColors();
	return true;
}

bool ColorWidget::togglePaletteUi() {
	if (!paletteWdgShown) {
		paletteWdg.appendTo(*this);
	} else {
		paletteWdg.remove();
	}

	paletteWdgShown = !paletteWdgShown;

	return true;
}
