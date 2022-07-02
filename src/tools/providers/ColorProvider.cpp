#include "ColorProvider.hpp"

#include <utility>

#include <InputManager.hpp>

struct ColorProvider::LocalContext {
	ImAction iSwapColors;

	LocalContext(InputAdapter& ia)
	: iSwapColors(ia, "Swap Colors", T_ONPRESS) {
		//iSwapColors.setDefaultKeybind("X");
	}
};

// local ctor
ColorProvider::ColorProvider(std::tuple<ToolManager&, InputAdapter&> params)
: lctx(std::make_unique<LocalContext>(std::get<1>(params))),
  primaryColor{{0, 0, 0, 255}},
  secondaryColor{{255, 255, 255, 255}} {

	lctx->iSwapColors.setCb([this] (auto&, const auto&) {
		swapColors();
	});
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
}

void ColorProvider::setPrimaryColor(RGB_u clr) {
	primaryColor = clr;
}

void ColorProvider::setSecondaryColor(RGB_u clr) {
	secondaryColor = clr;
}
