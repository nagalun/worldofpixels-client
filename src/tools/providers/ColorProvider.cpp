#include "ColorProvider.hpp"

#include <utility>

#include <ui/ColorWidget.hpp>
#include <InputManager.hpp>
#include <world/World.hpp>
#include <tools/ToolManager.hpp>

struct ColorProvider::LocalContext {
	ColorWidget cw;
	PaletteListWidget paletteWdg;
	ImAction iSwapColors;

	LocalContext(ColorProvider& clr, ToolManager& tm, InputAdapter& ia)
	: cw(clr),
	  paletteWdg(clr),
	  iSwapColors(ia, "Swap Colors", T_ONPRESS) {
		iSwapColors.setDefaultKeybind("X");

		cw.setPaletteTglFn([this] {
			paletteWdg.tglClass("hide");
		});

		auto& llui = tm.getWorld().getLlCornerUi();
		auto& fstCol = llui.template get<0>();
		auto& sndCol = llui.template get<1>();
		cw.appendTo(fstCol);
		paletteWdg.appendTo(sndCol);
	}
};

// local ctor
ColorProvider::ColorProvider(std::tuple<ToolManager&, InputAdapter&> params)
: lctx(std::make_unique<LocalContext>(*this, std::get<0>(params), std::get<1>(params))),
  primaryColor{{0, 0, 0, 255}},
  secondaryColor{{255, 255, 255, 255}} {

	lctx->iSwapColors.setCb([this] (auto&, const auto&) {
		swapColors();
	});

	lctx->cw.update();
}

// remote ctor
ColorProvider::ColorProvider(ToolManager& tm)
: primaryColor{{0, 0, 0, 255}},
  secondaryColor{{255, 255, 255, 255}} { }

ColorProvider::~ColorProvider() { }


RGB_u ColorProvider::getPrimaryColor() const {
	return primaryColor;
}

RGB_u ColorProvider::getSecondaryColor() const {
	return secondaryColor;
}

void ColorProvider::swapColors() {
	std::swap(primaryColor, secondaryColor);
	if (lctx) {
		lctx->cw.update();
	}
}

void ColorProvider::setPrimaryColor(RGB_u clr) {
	primaryColor = clr;
	if (lctx) {
		lctx->cw.update();
	}
}

void ColorProvider::setSecondaryColor(RGB_u clr) {
	secondaryColor = clr;
	if (lctx) {
		lctx->cw.update();
	}
}
